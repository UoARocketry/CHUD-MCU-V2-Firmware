#include <stdint.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    float pos[3]; // meters, world frame (x, y, z/altitude)
    float vel[3]; // m/s, world frame
    uint32_t last_time_ms;
    uint8_t initialized;
} dr_state_t;

static dr_state_t dr_state = {0};

// Milliseconds since boot, overflow-safe via unsigned wraparound subtraction.
static uint32_t dr_read_time_ms(void)
{
    return xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
}

// Returns elapsed time in seconds since the last call.
// First call after reset returns 0 to avoid a huge bogus dt.
static float dr_delta_time_s(void)
{
    uint32_t now = dr_read_time_ms();

    if (!dr_state.initialized)
    {
        dr_state.last_time_ms = now;
        dr_state.initialized = 1;
        return 0.0f;
    }

    // Unsigned subtraction wraps correctly even across tick-counter overflow.
    uint32_t dt_ms = now - dr_state.last_time_ms;
    dr_state.last_time_ms = now;

    // Guard against a stray zero/negative-looking or absurdly large dt
    // (e.g. after a debugger pause) which would otherwise corrupt the estimate.
    const float MAX_DT_S = 0.5f;
    float dt_s = dt_ms / 1000.0f;
    if (dt_s <= 0.0f || dt_s > MAX_DT_S)
    {
        return 0.0f;
    }
    return dt_s;
}

// Advances velocity and position by one step given world-frame,
// gravity-compensated acceleration (m/s^2) and elapsed time (s).
// Uses proper double integration:
//   pos += vel*dt + 0.5*accel*dt^2   (not just the 0.5*a*dt^2 term)
//   vel += accel*dt
static void dr_integrate(float accel[3], float dt_s)
{
    for (int i = 0; i < 3; i++)
    {
        dr_state.pos[i] += dr_state.vel[i] * dt_s + 0.5f * accel[i] * dt_s * dt_s;
        dr_state.vel[i] += accel[i] * dt_s;
    }
}

// Call once per new accelerometer sample.
// x, y, z: world-frame, gravity-removed acceleration in m/s^2.
void dr_update(float x, float y, float z)
{
    float dt_s = dr_delta_time_s();
    if (dt_s == 0.0f)
    {
        return; // first call, or a dt we rejected as bad
    }
    float accel[3] = {x, y, z};
    dr_integrate(accel, dt_s);
}

// Read back the current estimate.
void dr_get_position(float *x, float *y, float *z)
{
    *x = dr_state.pos[0];
    *y = dr_state.pos[1];
    *z = dr_state.pos[2];
}

void dr_get_velocity(float *vx, float *vy, float *vz)
{
    *vx = dr_state.vel[0];
    *vy = dr_state.vel[1];
    *vz = dr_state.vel[2];
}

// Call this whenever you get an absolute fix (GPS, barometer, vision, etc.)
// to correct accumulated drift. Optionally also zero/set velocity if you
// have a reliable velocity reference.
void dr_correct_position(float x, float y, float z)
{
    dr_state.pos[0] = x;
    dr_state.pos[1] = y;
    dr_state.pos[2] = z;
}

void dr_reset(void)
{
    dr_state.pos[0] = dr_state.pos[1] = dr_state.pos[2] = 0.0f;
    dr_state.vel[0] = dr_state.vel[1] = dr_state.vel[2] = 0.0f;
    dr_state.initialized = 0;
}