#include <zephyr/logging/log.h>

#include "sum_log.h"

LOG_MODULE_REGISTER(sum_log, LOG_LEVEL_DBG);

int sum_log(int a, int b)
{
	const int inputs[] = {a, b};
	int result = a + b;

	LOG_DBG("Starting sum calculation");

	if ((a < 0) || (b < 0)) {
		LOG_WRN("At least one input is negative");
	}

	LOG_HEXDUMP_INF(inputs, sizeof(inputs), "Input values");
	LOG_INF("Logger: %d + %d = %d", a, b, result);

	return result;
}
