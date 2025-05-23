// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

typedef void(*test_ubsan_fp)(void);

#define UBSAN_TEST(config, ...)	do {					\
		pr_info("%s " __VA_ARGS__ "%s(%s=%s)\n", __func__,	\
			sizeof(" " __VA_ARGS__) > 2 ? " " : "",		\
			#config, IS_ENABLED(config) ? "y" : "n");	\
	} while (0)

static void test_ubsan_divrem_overflow(void)
{
	volatile int val = 16;
	volatile int val2 = 0;

	UBSAN_TEST(CONFIG_UBSAN_DIV_ZERO);
	val /= val2;
}

static void test_ubsan_shift_out_of_bounds(void)
{
	volatile int neg = -1, wrap = 4;
	int val1 = 10;
	int val2 = INT_MAX;

	UBSAN_TEST(CONFIG_UBSAN_SHIFT, "negative exponent");
	val1 <<= neg;

	UBSAN_TEST(CONFIG_UBSAN_SHIFT, "left overflow");
	val2 <<= wrap;
}

static void test_ubsan_out_of_bounds(void)
{
	int i = 4, j = 4, k = -1;
	volatile struct {
		char above[4]; /* Protect surrounding memory. */
		int arr[4];
		char below[4]; /* Protect surrounding memory. */
	} data;

	OPTIMIZER_HIDE_VAR(i);
	OPTIMIZER_HIDE_VAR(j);
	OPTIMIZER_HIDE_VAR(k);

	UBSAN_TEST(CONFIG_UBSAN_BOUNDS, "above");
	data.arr[j] = i;

	UBSAN_TEST(CONFIG_UBSAN_BOUNDS, "below");
	data.arr[k] = i;
}

enum ubsan_test_enum {
	UBSAN_TEST_ZERO = 0,
	UBSAN_TEST_ONE,
	UBSAN_TEST_MAX,
};

static void test_ubsan_load_invalid_value(void)
{
	volatile char *dst, *src;
	bool val, val2, *ptr;
	enum ubsan_test_enum eval, eval2, *eptr;
	unsigned char c = 0xff;

	UBSAN_TEST(CONFIG_UBSAN_BOOL, "bool");
	dst = (char *)&val;
	src = &c;
	*dst = *src;

	ptr = &val2;
	val2 = val;

	UBSAN_TEST(CONFIG_UBSAN_ENUM, "enum");
	dst = (char *)&eval;
	src = &c;
	*dst = *src;

	eptr = &eval2;
	eval2 = eval;
}

static void test_ubsan_misaligned_access(void)
{
	volatile char arr[5] __aligned(4) = {1, 2, 3, 4, 5};
	volatile int *ptr, val = 6;

	UBSAN_TEST(CONFIG_UBSAN_ALIGNMENT);
	ptr = (int *)(arr + 1);
	*ptr = val;
}

static const test_ubsan_fp test_ubsan_array[] = {
	test_ubsan_shift_out_of_bounds,
	test_ubsan_out_of_bounds,
	test_ubsan_load_invalid_value,
	test_ubsan_misaligned_access,
};

/* Excluded because they Oops the module. */
static const test_ubsan_fp skip_ubsan_array[] = {
	test_ubsan_divrem_overflow,
};

static int __init test_ubsan_init(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(test_ubsan_array); i++)
		test_ubsan_array[i]();

	return 0;
}
module_init(test_ubsan_init);

static void __exit test_ubsan_exit(void)
{
	/* do nothing */
}
module_exit(test_ubsan_exit);

MODULE_AUTHOR("Jinbum Park <jinb.park7@gmail.com>");
MODULE_LICENSE("GPL v2");
