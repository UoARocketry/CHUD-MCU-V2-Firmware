#ifndef DEAD_RECKONING_H
#define DEAD_RECKONING_H

// Call once per new accelerometer sample.
// x, y, z: world-frame, gravity-removed acceleration in m/s^2.
void dr_update(float x, float y, float z);

// Read back the current position/velocity estimate.
void dr_get_position(float *x, float *y, float *z);
void dr_get_velocity(float *vx, float *vy, float *vz);

// Correct accumulated drift with an absolute fix (GPS, baro, vision, etc.)
void dr_correct_position(float x, float y, float z);

void dr_reset(void);

#endif // DEAD_RECKONING_H