#ifndef BME280_RAW_H_
#define BME280_RAW_H_

#include <stdint.h>

/*
 * Initialize the BME280 and read its factory temperature
 * calibration values.
 */
int bme280_raw_init(void);

/*
 * Read compensated temperature.
 *
 * The output unit is 0.01 degrees Celsius:
 *     2345 means 23.45 degrees Celsius.
 */
int bme280_raw_read_temperature(int32_t *temperature_centi_c);

#endif /* BME280_RAW_H_ */