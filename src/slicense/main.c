#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <ctype.h>

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "base32.h"

#define SERIAL_FILE "serial"
#define SERIAL_LEN 1

#define SALT '\xa1'

char *
sha(char *s, size_t len)
{
	char *md;

	md = malloc(SHA_DIGEST_LENGTH);
	SHA1((void*)s, len, (void*)md);

	return md;
}

char *
base64(const unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;
	char *buf;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	buf = malloc(bptr->length);
	memcpy(buf, bptr->data, bptr->length-1);
	buf[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buf;
}

char *
unbase64(unsigned char *input, int length)
{
	BIO *b64, *bmem;
	char *buf;

	buf = malloc(length);
	memset(buf, '\0', length);

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);

	BIO_read(bmem, buf, length);
	BIO_free_all(bmem);

	return buf;
}

void
inc_uiarr(unsigned int *a, size_t n)
{
	size_t i;

	for (i = n-1; i >= 0; i--) {
		a[i]++;
		if (a[i] != 0) // Overflow?
			break;
		if (i == 0)
			err(1, "Serial has become too big!");
	}
}

unsigned int *
get_serial(size_t *len)
{
	FILE *fp;
	unsigned int num[SERIAL_LEN], *p;

	fp = fopen(SERIAL_FILE, "r");
	if (fp == NULL)
		err(1, "fopen(%s)", SERIAL_FILE);
	*len = fread(num,  sizeof(unsigned int), SERIAL_LEN, fp);
	if (*len != SERIAL_LEN)
		err(1, "fread(serial): %d", *len);
	fclose(fp);
	
	p = malloc(sizeof(num));
	memcpy(p, num, (*len) * sizeof(unsigned int));
	return p;
}

void
inc_serial(void)
{
	unsigned int *a;
	size_t len;
	FILE *fp;

	a = get_serial(&len);
	inc_uiarr(a, len);
	fp = fopen(SERIAL_FILE, "w");
	if (fp == NULL)
		err(1, "fopen(SERIAL_FILE)");
	if (fwrite(a, sizeof(unsigned int), SERIAL_LEN, fp) != SERIAL_LEN)
		err(1, "fwrite()");
	fclose(fp);
}

/*
 * Make a serial that has a maximum of n users.
 */
char *
make_license(unsigned short int users)
{
	size_t seriallen;
	unsigned int *serial;
	char s[1024], f[1024];
	int i, j;
	unsigned short int *iptr;
	char *h, *hb64;

	serial = get_serial(&seriallen);
	inc_serial();
	seriallen *= sizeof(unsigned int);
	memcpy(s, serial, seriallen);
	iptr = (unsigned short int *)&s[seriallen];
	*iptr = users;

	s[6] = SALT;
	h = sha(s, seriallen+2+1);

	j = 0;
	f[j++] = h[11];
	for (i = 0; i < seriallen+2; i++) {
		f[j++] = s[i];
		f[j] = h[j];
		j++;
	}
	f[j++] = h[15];
	f[j++] = h[17];
	f[j] = '\0';

	base32_encode(f, &hb64, j);//(seriallen+4)*2);

	return hb64;
}

void
normalize_key(char *k)
{
	int i, j;

	for (i = 0, j = 0; k[i] != '\0'; i++) {
		if (k[i] == '-')
			continue;
		k[j++] = toupper(k[i]);
	}
	k[j] = '\0';
}

void
get_licenseinfo(char *s)
{
	size_t len;
	char *buf;
	int i;
	int j;
	char hash[SHA_DIGEST_LENGTH], *h;
	char data[32];
	unsigned short int *users;
	unsigned int *serial;
	int check_indexes[] = { 2, 4, 6, 8, 10, 12, 11, 15, 17, -1, };

	normalize_key(s);
	len = base32_decode(s, &buf, strlen(s));
	j = 0;
	hash[11] = buf[0];
	for (i = 1; j < 6; i++) {
		data[j++] = buf[i++];
		hash[i] = buf[i];
	}
	hash[15] = buf[i++];
	hash[17] = buf[i++];
	data[6] = SALT;
	h = sha(data, 4+2+1);
	for (i = 0; check_indexes[i] != -1; i++) {
		int idx = check_indexes[i];

		if (hash[idx] != h[idx])
			errx(1, "Invalid license key");
	}

	serial = (unsigned int *)data;
	users = (unsigned short int *)(data+4);

	printf("Serial: %d number of licensed users: %d\n", *serial, *users);
}

char *
human_readable_key(char *k)
{
	int i;
	int j;
	char *b;

	b = malloc(strlen(k) + 4 + 1);

	for (i = 0, j = 0; k[i] != '\0'; i++) {
		if (i > 0 && i % 5 == 0) {
			b[j++] = '-';
		}
		b[j++] = k[i];
	}
	b[j] = '\0';

	free(k);
	return b;
}

#ifdef TESTMAIN
int
main(int argc, char **argv)
{
	char *key;

	key = make_license(1000);
	key = human_readable_key(key);
	printf("We have a key: %s\n", key);
	get_licenseinfo(key);
	free(key);

	return 0;
}
#endif
