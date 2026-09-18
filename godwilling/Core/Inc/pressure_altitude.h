#ifndef PRESSURE_ALTITUDE_H
#define PRESSURE_ALTITUDE_H

#include <math.h>

// Barometric formula (valid for the troposphere, <11km):
//   h = 44330 * (1 - (P / P0)^(1/5.255))
//
// pressure_pa: current sensor reading, Pascals
// reference_pa: pressure at your zero-altitude reference, Pascals
//   - Use true sea-level pressure (~101325 Pa, or the day's QNH) if you
//     want altitude above sea level.
//   - Use the pressure measured on the pad before launch if you want
//     altitude above ground level (AGL) — this is what you want for
//     apogee detection, since it's immune to day-to-day weather drift
//     and needs no external QNH lookup.
//
// Returns altitude in meters relative to reference_pa.
static inline float pressure_to_altitude_m(float pressure_pa, float reference_pa) {
    return 44330.0f * (1.0f - powf(pressure_pa / reference_pa, 1.0f / 5.255f));
}

#endif // PRESSURE_ALTITUDE_H
