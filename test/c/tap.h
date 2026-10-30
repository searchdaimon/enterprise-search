/*
 * tap.h - a minimal TAP (Test Anything Protocol) harness for C unit tests.
 *
 * Each test program emits TAP on stdout, so `prove` can run the compiled
 * binaries side by side with the Perl tests and summarise them together.
 * The process exit status is the number of failed assertions (capped at 254),
 * which is also what prove uses to decide pass/fail.
 *
 * Usage:
 *     #include "tap.h"
 *     int main(void) {
 *         ok(1 + 1 == 2, "arithmetic works");
 *         is_str(strdup("a"), "a", "strdup copies");
 *         is_int(strlen("abc"), 3, "strlen");
 *         return done_testing();
 *     }
 */
#ifndef SD_TAP_H
#define SD_TAP_H

#include <stdio.h>
#include <string.h>

static int tap_ran = 0;
static int tap_failed = 0;

static int tap_report(int pass, const char *name, const char *file, int line)
{
	tap_ran++;
	if (pass) {
		printf("ok %d - %s\n", tap_ran, name);
	} else {
		tap_failed++;
		printf("not ok %d - %s\n", tap_ran, name);
		printf("#   at %s line %d\n", file, line);
	}
	return pass;
}

#define ok(cond, name) tap_report((cond) ? 1 : 0, name, __FILE__, __LINE__)

#define is_int(got, expected, name) do { \
	long long tap_g = (long long)(got), tap_e = (long long)(expected); \
	if (!tap_report(tap_g == tap_e, name, __FILE__, __LINE__)) \
		printf("#   got:      %lld\n#   expected: %lld\n", tap_g, tap_e); \
} while (0)

#define is_uint(got, expected, name) do { \
	unsigned long long tap_g = (unsigned long long)(got), tap_e = (unsigned long long)(expected); \
	if (!tap_report(tap_g == tap_e, name, __FILE__, __LINE__)) \
		printf("#   got:      0x%llx\n#   expected: 0x%llx\n", tap_g, tap_e); \
} while (0)

#define is_str(got, expected, name) do { \
	const char *tap_g = (got), *tap_e = (expected); \
	int tap_pass = (tap_g != NULL && tap_e != NULL) ? strcmp(tap_g, tap_e) == 0 : tap_g == tap_e; \
	if (!tap_report(tap_pass, name, __FILE__, __LINE__)) \
		printf("#   got:      \"%s\"\n#   expected: \"%s\"\n", tap_g ? tap_g : "(null)", tap_e ? tap_e : "(null)"); \
} while (0)

#define diag(...) do { printf("# "); printf(__VA_ARGS__); printf("\n"); } while (0)

static int done_testing(void)
{
	printf("1..%d\n", tap_ran);
	if (tap_failed)
		printf("# Looks like you failed %d test%s of %d.\n", tap_failed, tap_failed == 1 ? "" : "s", tap_ran);
	return tap_failed > 254 ? 254 : tap_failed;
}

#endif /* SD_TAP_H */
