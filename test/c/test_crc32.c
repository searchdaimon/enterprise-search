/* Unit tests for src/common/crc32.c (standard CRC-32, same as PKZIP/zlib). */
#include "tap.h"
#include "common/crc32.h"

int main(void)
{
	is_uint(crc32boitho(""), 0x00000000u, "crc32 of the empty string is 0");
	is_uint(crc32boitho("123456789"), 0xCBF43926u, "crc32 check value for \"123456789\"");
	is_uint(crc32boitho("The quick brown fox jumps over the lazy dog"), 0x414FA339u, "crc32 of a pangram matches zlib");
	is_uint(crc32boithonl("123456789", 9), 0xCBF43926u, "crc32boithonl agrees with crc32boitho");
	is_uint(crc32boithonl("123456789xxx", 9), 0xCBF43926u, "crc32boithonl honours the length");
	ok(crc32boitho("a") != crc32boitho("b"), "different inputs hash differently");
	ok(crc32boitho("abc") == crc32boitho("abc"), "hashing is deterministic");
	return done_testing();
}
