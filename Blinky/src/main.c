/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <stdint.h>
#include <zephyr/logging/log.h>
#include "bme280_raw.h"

#if defined(CONFIG_SUM_PRINT)
#include "sum_printk.h"
#elif defined(CONFIG_SUM_LOG)
#include "sum_log.h"
#endif

LOG_MODULE_REGISTER(blinky_app, LOG_LEVEL_INF);

#define TEMPERATURE_INTERVAL_MS 2000

#define POLL_INTERVAL_MS 20

/* Custom LED alias from the application overlay. */
#define LED5180_NODE DT_ALIAS(led5180)

/* Default board alias for Button 1 / SW1. */
#define BUTTON_NODE DT_ALIAS(button5180)

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED5180_NODE, gpios);

static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int main(void)
{
	const int input_a = 7;
	const int input_b = -2;
	int ret;
	int sum_result;
	bool led_state = false;
	bool previous_pressed = false;

#if defined(CONFIG_SUM_PRINT)
	sum_result = sum_printk(input_a, input_b);
#elif defined(CONFIG_SUM_LOG)
	sum_result = sum_log(input_a, input_b);
#endif

	// Initialize the BME280 sensor
	int64_t next_temperature_ms = 0;
	
	if (sum_result != 5) {
		return 0;
	}

	if (!gpio_is_ready_dt(&led) ||
	    !gpio_is_ready_dt(&button)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	// Initialize the BME280 sensor
	ret = bme280_raw_init();
	if (ret < 0) {
		LOG_ERR("BME280 initialization failed: %d", ret);
		return 0;
	}

	while (1) {
		int pressed = gpio_pin_get_dt(&button);

		if (pressed < 0) {
			return 0;
		}

		/* Toggle the LED only once for each button press. */
		if (pressed && !previous_pressed) {
			ret = gpio_pin_toggle_dt(&led);
			if (ret < 0) {
				return 0;
			}

			led_state = !led_state;
			printf("LED state: %s\n",
			       led_state ? "ON" : "OFF");
		}

		previous_pressed = pressed;

		// Read and log the temperature from the BME280 sensor every TEMPERATURE_INTERVAL_MS milliseconds
		int64_t now = k_uptime_get();
		if (now >= next_temperature_ms) {
			int32_t temperature;
			int32_t magnitude;

			ret = bme280_raw_read_temperature(&temperature);
			if (ret < 0) {
				LOG_ERR("Temperature read failed: %d", ret);
			} else {
				magnitude = temperature < 0
						? -temperature
						: temperature;

				LOG_INF("Temperature: %s%d.%02d C",
					temperature < 0 ? "-" : "",
					magnitude / 100,
					magnitude % 100);
			}

			next_temperature_ms = now + TEMPERATURE_INTERVAL_MS;
		}

		k_msleep(POLL_INTERVAL_MS);
	}

	return 0;
}
