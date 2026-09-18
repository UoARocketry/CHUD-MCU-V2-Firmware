#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "dead_reckoning.h"   // dr_update, dr_get_position, dr_get_velocity, dr_reset

/* ------------------------------------------------------------------ */
/* Apogee detection                                                    */
/*                                                                     */
/* Method: watch vertical velocity (from dead reckoning). On a real    */
/* rocket flight, velocity is strongly positive during boost/coast,    */
/* crosses zero at apogee, then goes negative on the way down.         */
/*                                                                     */
/* A single negative sample is not trusted on its own — accelerometer  */
/* noise near the velocity-zero-crossing can cause a false trigger.    */
/* Instead we require N consecutive samples of negative/decreasing     */
/* vertical velocity before declaring apogee. This is the standard     */
/* debounced approach used in hobby/HPR altimeters.                    */
/* ------------------------------------------------------------------ */

#define APOGEE_CONFIRM_COUNT   5      // consecutive falling samples required
#define MIN_ALTITUDE_FOR_APOGEE_M 10.0f // ignore ground-level noise pre-launch

typedef struct {
    float last_altitude_m;
    uint8_t falling_count;
    bool armed;      // true once we've seen real upward motion (post-launch)
    bool apogee_detected;
    float apogee_altitude_m;
} apogee_state_t;

static apogee_state_t apogee = {0};

void apogee_reset(void) {
    apogee.last_altitude_m = 0.0f;
    apogee.falling_count = 0;
    apogee.armed = false;
    apogee.apogee_detected = false;
    apogee.apogee_altitude_m = 0.0f;
}

// Call once per dead-reckoning update. Returns true the instant apogee
// is confirmed (fires exactly once per flight until apogee_reset()).
bool apogee_check(void) {
    if (apogee.apogee_detected) {
        return false; // already fired, don't re-trigger
    }

    float x, y, z_vel;
    float px, py, altitude_m;
    dr_get_position(&px, &py, &altitude_m);
    dr_get_velocity(&x, &y, &z_vel);

    // Don't start looking for apogee until we've actually left the pad.
    if (!apogee.armed) {
        if (altitude_m > MIN_ALTITUDE_FOR_APOGEE_M && z_vel > 0.0f) {
            apogee.armed = true;
        }
        apogee.last_altitude_m = altitude_m;
        return false;
    }

    if (altitude_m < apogee.last_altitude_m || z_vel <= 0.0f) {
        apogee.falling_count++;
    } else {
        apogee.falling_count = 0; // reset on any real upward sample
    }

    apogee.last_altitude_m = altitude_m;

    if (apogee.falling_count >= APOGEE_CONFIRM_COUNT) {
        apogee.apogee_detected = true;
        apogee.apogee_altitude_m = altitude_m;
        return true;
    }

    return false;
}

float apogee_get_altitude(void) {
    return apogee.apogee_altitude_m;
}

bool apogee_has_occurred(void) {
    return apogee.apogee_detected;
}

/* ------------------------------------------------------------------ */
/* RTOS task                                                           */
/* ------------------------------------------------------------------ */

#define DR_TASK_PERIOD_MS   20   // 50 Hz update rate — adjust to your IMU's ODR

// Replace this with your actual IMU driver call. Must return
// world-frame, gravity-compensated acceleration in m/s^2.
// (If your IMU only gives body-frame readings, rotate them using your
// orientation estimate and subtract gravity before returning.)
extern void read_accel_world_frame(float *x, float *y, float *z);

// Records the tick time apogee was detected (i.e. when deployment
// would've fired) instead of actually triggering a pyro channel.
// Swap this out for the real deployment call once you're ready to
// go from logging-only to a live recovery sequence.
static uint32_t deploy_would_have_fired_at_ms = 0;
static bool deploy_logged = false;

// Replace this with your actual logging mechanism (UART printf,
// SD card write, flash log, etc). Left as a weak-ish extern hook
// so you can swap in the real thing without touching this file.
extern void log_event(const char *msg, uint32_t value);

static void deploy_recovery(void) {
    deploy_would_have_fired_at_ms = xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
    deploy_logged = true;
    log_event("APOGEE - would deploy recovery at t(ms)=", deploy_would_have_fired_at_ms);
}

uint32_t deploy_get_logged_time_ms(void) {
    return deploy_would_have_fired_at_ms;
}

bool deploy_was_logged(void) {
    return deploy_logged;
}

void vDeadReckoningTask(void *pvParameters) {
    (void) pvParameters;
    
    dr_reset();
    apogee_reset();

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period_ticks = pdMS_TO_TICKS(DR_TASK_PERIOD_MS);

    for (;;) {
        float ax, ay, az;
        read_accel_world_frame(&ax, &ay, &az);

        dr_update(ax, ay, az);

        if (apogee_check()) {
            deploy_recovery();
            // Task continues running (e.g. for descent tracking / logging)
            // but will not re-fire apogee detection until apogee_reset().
        }

        vTaskDelayUntil(&last_wake_time, period_ticks);
    }
}