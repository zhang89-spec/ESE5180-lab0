#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "bme280_math.h"
#include "bme280_raw.h"

LOG_MODULE_REGISTER(bme280_raw, LOG_LEVEL_INF);

#define BME280_NODE DT_NODELABEL(bme280_5180)

#if !DT_NODE_HAS_STATUS(BME280_NODE, okay)
#error "The bme280_5180 devicetree node is missing or disabled"
#endif

BUILD_ASSERT(DT_REG_ADDR(BME280_NODE) == 0x77,
	     "BME280 must use I2C address 0x77");

#define BME280_REG_CALIB_T1  0x88
#define BME280_REG_CHIP_ID   0xD0
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_TEMP_MSB  0xFA

#define BME280_CHIP_ID       0x60

/*
 * Temperature oversampling x1, pressure skipped, normal mode:
 *
 * osrs_t = 001
 * osrs_p = 000
 * mode   = 11
 *
 * 0010 0011 = 0x23
 */
#define BME280_CTRL_MEAS_VALUE 0x23

static const struct i2c_dt_spec bme280 =
	I2C_DT_SPEC_GET(BME280_NODE);

static struct bme280_temp_calibration temp_calibration;
static bool initialized;

static uint16_t decode_u16_le(const uint8_t *data)
{
	return (uint16_t)data[0] |
	       ((uint16_t)data[1] << 8);
}

static int16_t decode_s16_le(const uint8_t *data)
{
	return (int16_t)decode_u16_le(data);
}

static int bme280_read_register(uint8_t reg, uint8_t *value)
{
	return i2c_write_read_dt(&bme280, &reg, sizeof(reg),
				 value, sizeof(*value));
}

static int bme280_write_register(uint8_t reg, uint8_t value)
{
	uint8_t message[] = {reg, value};

	return i2c_write_dt(&bme280, message, sizeof(message));
}

int bme280_raw_init(void)
{
	uint8_t chip_id;
	uint8_t calibration[6];
	int ret;

	if (!device_is_ready(bme280.bus)) {
		LOG_ERR("I2C controller is not ready");
		return -ENODEV;
	}

	/* Allow the sensor to complete startup and NVM calibration copy. */
	k_msleep(10);

	ret = bme280_read_register(BME280_REG_CHIP_ID, &chip_id);
	if (ret < 0) {
		LOG_ERR("Unable to read chip ID: %d", ret);
		return ret;
	}

	if (chip_id != BME280_CHIP_ID) {
		LOG_ERR("Unexpected chip ID: 0x%02x", chip_id);
		return -ENODEV;
	}

	ret = i2c_burst_read_dt(&bme280, BME280_REG_CALIB_T1,
				calibration, sizeof(calibration));
	if (ret < 0) {
		LOG_ERR("Unable to read temperature calibration: %d", ret);
		return ret;
	}

	temp_calibration.dig_t1 = decode_u16_le(&calibration[0]);
	temp_calibration.dig_t2 = decode_s16_le(&calibration[2]);
	temp_calibration.dig_t3 = decode_s16_le(&calibration[4]);

	ret = bme280_write_register(BME280_REG_CTRL_MEAS,
				    BME280_CTRL_MEAS_VALUE);
	if (ret < 0) {
		LOG_ERR("Unable to configure CTRL_MEAS: %d", ret);
		return ret;
	}

	k_msleep(10);
	initialized = true;

	LOG_INF("BME280 detected at I2C address 0x%02x",
		bme280.addr);

	return 0;
}

int bme280_raw_read_temperature(int32_t *temperature_centi_c)
{
	uint8_t raw[3];
	int32_t adc_t;
	int ret;

	if (!initialized || temperature_centi_c == NULL) {
		return -EINVAL;
	}

	ret = i2c_burst_read_dt(&bme280, BME280_REG_TEMP_MSB,
				raw, sizeof(raw));
	if (ret < 0) {
		return ret;
	}

	/* TEMP_MSB, TEMP_LSB and the upper four bits of TEMP_XLSB. */
	adc_t = ((int32_t)raw[0] << 12) |
		((int32_t)raw[1] << 4) |
		((int32_t)raw[2] >> 4);

	return bme280_compensate_temperature(&temp_calibration,
					     adc_t,
					     temperature_centi_c);
}
