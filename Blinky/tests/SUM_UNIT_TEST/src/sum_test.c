#include <zephyr/ztest.h>

#include "sum_log.h"

ZTEST(sum_log_test_suite, test_sum_log_basic)
{
	int result = sum_log(2, 3);

	zassert_equal(result, 5,
		      "Expected 2 + 3 to equal 5, but got %d",
		      result);
}

ZTEST(sum_log_test_suite, test_sum_log_negative)
{
	int result = sum_log(-5, 2);

	zassert_equal(result, -3,
		      "Expected -5 + 2 to equal -3, but got %d",
		      result);
}

ZTEST(sum_log_test_suite, test_sum_log_zero)
{
	int result = sum_log(0, 0);

	zassert_equal(result, 0,
		      "Expected 0 + 0 to equal 0, but got %d",
		      result);
}

ZTEST_SUITE(sum_log_test_suite, NULL, NULL, NULL, NULL, NULL);