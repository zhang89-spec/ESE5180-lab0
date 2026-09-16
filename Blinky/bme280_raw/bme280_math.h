#ifndef BME280_MATH_H_
#define BME280_MATH_H_

#include <stdint.h>

struct bme280_temp_calibration {
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
};

/*
 * Convert a 20-bit raw BME280 temperature reading into
 * hundredths of a degree Celsius.
 *
 * Example: 2508 means 25.08 degrees Celsius.
 */
int bme280_compensate_temperature(
	const struct bme280_temp_calibration *calibration,
	int32_t adc_temperature,
	int32_t *temperature_centi_c);

#endif /* BME280_MATH_H_ */