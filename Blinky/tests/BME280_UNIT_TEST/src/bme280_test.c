#include <errno.h>
#include <stdint.h>

#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "bme280_math.h"

#define BME280_NODE DT_NODELABEL(bme280_5180)

BUILD_ASSERT(DT_NODE_HAS_STATUS(BME280_NODE, okay),
	     "bme280_5180 must exist and be enabled");
BUILD_ASSERT(DT_REG_ADDR(BME280_NODE) == 0x77,
	     "BME280 address must be 0x77");

ZTEST(bme280_test_suite, test_devicetree_setup)
{
	zassert_true(DT_NODE_HAS_STATUS(BME280_NODE, okay),
		     "BME280 node is not enabled");
	zassert_equal(DT_REG_ADDR(BME280_NODE), 0x77,
		      "Expected address 0x77");
}

ZTEST(bme280_test_suite, test_temperature_compensation)
{
	const struct bme280_temp_calibration calibration = {
		.dig_t1 = 27504,
		.dig_t2 = 26435,
		.dig_t3 = -1000,
	};
	int32_t temperature;
	int ret;

	ret = bme280_compensate_temperature(&calibration, 519888,
					     &temperature);

	zassert_equal(ret, 0, "Compensation returned %d", ret);
	zassert_equal(temperature, 2508,
		      "Expected 25.08 C, got %d centi-C", temperature);
	zassert_true(temperature >= -4000 && temperature <= 8500,
		     "Temperature is outside the sane range");
}

ZTEST(bme280_test_suite, test_invalid_arguments)
{
	const struct bme280_temp_calibration calibration = {
		.dig_t1 = 27504,
		.dig_t2 = 26435,
		.dig_t3 = -1000,
	};
	int32_t temperature;

	zassert_equal(bme280_compensate_temperature(NULL, 519888,
						    &temperature),
		      -EINVAL, "NULL calibration should be rejected");
	zassert_equal(bme280_compensate_temperature(&calibration, 519888,
						    NULL),
		      -EINVAL, "NULL output should be rejected");
	zassert_equal(bme280_compensate_temperature(&calibration, 0x100000,
						    &temperature),
		      -EINVAL, "ADC values larger than 20 bits should be rejected");
}

ZTEST_SUITE(bme280_test_suite, NULL, NULL, NULL, NULL, NULL);
