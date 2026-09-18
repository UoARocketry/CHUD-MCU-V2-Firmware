#ifndef BME280_H
#define BME280_H

#include "main.h"

// Define data structure matching your main.c usage
typedef struct {
    float temperature;
    float pressure;
} BME280_Data_t;

// Public function prototypes
void BME280_Init(void);
void BME280_Extract_Data(BME280_Data_t *data);

#endif /* BME280_H */
