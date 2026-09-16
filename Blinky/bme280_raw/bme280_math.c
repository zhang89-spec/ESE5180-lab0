#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "bme280_math.h"

int bme280_compensate_temperature(
	const struct bme280_temp_calibration *calibration,
	int32_t adc_temperature,
	int32_t *temperature_centi_c)
{
	int64_t var1;
	int64_t var2;
	int64_t delta;
	int64_t t_fine;

	if (calibration == NULL || temperature_centi_c == NULL) {
		return -EINVAL;
	}

	/* BME280 temperature ADC result is 20 bits. */
	if (adc_temperature < 0 || adc_temperature > 0xFFFFF) {
		return -EINVAL;
	}

	var1 = (((((int64_t)adc_temperature >> 3) -
		  ((int64_t)calibration->dig_t1 << 1))) *
		 calibration->dig_t2) >> 11;

	delta = ((int64_t)adc_temperature >> 4) -
		calibration->dig_t1;

	var2 = (((delta * delta) >> 12) *
		calibration->dig_t3) >> 14;

	t_fine = var1 + var2;

	*temperature_centi_c =
		(int32_t)((t_fine * 5 + 128) >> 8);

	return 0;
}