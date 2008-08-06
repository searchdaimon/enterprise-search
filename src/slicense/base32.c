#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static const char *base_32 = "ABCDEFGHJKLMNOPQRSTVWXYZ23456789";

#define XX 255

static const unsigned char index_32[256] = {
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,24,25, 26,27,28,29, 30,31,XX,XX, XX,XX,XX,XX,
    XX, 0, 1, 2,  3, 4, 5, 6,  7, XX, 8,9, 10,11,12,13,
    14,15,16,17, 18,XX,19,20, 21,22,23,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,

    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};

char
get_bits(unsigned int bitnum, const char *a)
{
	char c;
	int i, rest;

	c = 0;
	i = bitnum / 8;
	rest = bitnum % 8;

	switch (rest) {
		case 0: case 1: case 2: case 3:
			c = (a[i] >> (3-rest)) & 0x1f;
			break;
		case 4:
			c = (a[i] & 0xF) << 1;
			c |= (a[i+1] & (0x1 << 7)) >> 7;
			break;
		case 5:
			c = (a[i] & 0x7) << 2;
			c |= (a[i+1] & (0x3 << 6)) >> 6;
			break;
		case 6:
			c = (a[i] & 0x3) << 3;
			c |= (a[i+1] & (0x7 << 5)) >> 5;
			break;
		case 7:
			c = (a[i] & 0x1) << 4;
			c |= (a[i+1] & (0xF << 4)) >> 4;
			break;
	}

	return c;
}

int
base32_encode(char *in, char **out, size_t len)
{
	int i;
	unsigned int bitnum;
	char *s;

	assert(((len * 8) % 5) == 0);

	s = malloc((len * 8) + 1); // XXX Way too much

	bitnum = 0;
	i = 0;
	while (bitnum / 8 < len) {
		char b = get_bits(bitnum, in);
		//printf("%c", base_32[b]);
		s[i++] = base_32[(int)b];
		bitnum += 5;
	}
	s[i] = '\0';
	//puts("");

	*out = s;

	return i;
}

void
set_bits(unsigned int bitnum, char *a, char bits)
{
	char c;
	int i, rest;

	c = 0;
	i = bitnum / 8;
	rest = bitnum % 8;

	switch (rest) {
		case 0: case 1: case 2: case 3:
			a[i] |= bits << (3-rest);
			break;
		case 4:
			a[i] |= (bits >> 1) & 0xF;
			a[i+1] = (bits & 0x1) << 7;
			break;
		case 5:
			a[i] |= (bits >> 2) & 0x7;
			a[i+1] = (bits & 0x3) << 6;
			break;
		case 6:
			a[i] |= (bits >> 3) & 0x3;
			a[i+1] = (bits & 0x7) << 5;
			break;
		case 7:
			a[i] |= (bits >> 4) & 0x1;
			a[i+1] = (bits & 0xF) << 4;
			break;
	}
}



int
base32_decode(char *in, char **out, size_t len)
{
	int i;
	unsigned int bitnum;
	char *buf;

	buf = malloc(len * 8); // XXX: Way too big

	bitnum = 0;
	for (i = 0; len > i; i++) {
		set_bits(bitnum, buf, index_32[(int)in[i]]);
		bitnum += 5;
	}
	
#if 0
	for (i = 0; i < bitnum/8; i++)
		printf("0x%X ", buf[i]&0xff);
	puts("");
#endif

	*out = buf;

	return bitnum / 8;
}

#ifdef TEST_BASE32
int
main(int argc, char **argv)
{
	char *str = "\xf3\xf1\xc2\x53\x84";
	char *out;
	int i;

	printf("Starting with: ");
	for (i = 0; i < 5; i++)
		printf("0x%X ", str[i] & 0xff);
	puts("");
	base32_encode(str, &out, 5);
	base32_decode("8Q26EW6E", &out, 8);

	return 0;
}
#endif
