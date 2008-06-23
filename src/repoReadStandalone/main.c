#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <zlib.h>

#include "common.h"

#define RECORD_SEPARATOR "***"

void
usage(void)
{
	errx(1, "Usage: %s repository", "reporead");
}

/* Repository read functions */
/* Return values:
 *  0: Unrecoverable error
 *  1: All OK
 *  2: End of data
 *  3: Hopfully recoverable error
 */
int
repo_read_entry(FILE *fp, RepositoryHeader_t *header, char **data, size_t *datalen, char **image, size_t *imagelen)
{
	int n;
	char *zdata;
	char separator[strlen(RECORD_SEPARATOR)+1];
	size_t datasize;
	int error;

	*data = *image = zdata = NULL;
	error = 0;

	/* Read in header format */
	if ((n = fread(header, sizeof(*header), 1, fp)) != 1)
		return 2; /* No more data to read */
	
	/* Allocate room for data and image buffers */
	if (header->htmlSize == 0) {
		zdata = NULL;
	} else {
		zdata = malloc(header->htmlSize);
		if (zdata == NULL)
			goto err;
	}
	if (header->imageSize == 0) {
		*image = NULL;
	} else {
		*image = malloc(header->imageSize);
		if (*image == NULL)
			goto err;
	}

	/* Read in data */
	if (zdata != NULL) {
		if ((n = fread(zdata, header->htmlSize, 1, fp)) != 1) {
			goto err;
		}

	}
	/* Read in image */
	if (*image != NULL) {
		if ((n = fread(*image, header->imageSize, 1, fp)) != 1) {
			goto err;
		}
	}
	*imagelen = header->imageSize;

	/* Uncompress data */
	datasize = header->htmlSize * 8;
	*data = malloc(datasize+1);
	if (*data == NULL)
		goto err;
	if ((n = uncompress((Bytef *)*data, (uLongf *)&datasize, (Bytef *)zdata, header->htmlSize)) != 0) {
		warnx("data uncompress error: %d", n);
		error++;
	} else {
		//printf("datasize: %d\n", datasize);
		*datalen = datasize;
	}
	
	/* Get record separator */
	if ((n = fread(separator, strlen(RECORD_SEPARATOR), 1, fp)) != 1) {
		goto err;
	} else {
		/* Verify separator content */
		separator[strlen(RECORD_SEPARATOR)] = '\0';
		if (strcmp(separator, RECORD_SEPARATOR) != 0) {
			warnx("Wrong record separator: %x %x %x", separator[0], separator[1], separator[2]);
			error++;
		}
	}

	free(zdata);
	return error == 0 ? 1 : 3;
 err:
 	free(*data);
	free(*image);
 	free(zdata);
	return 0;
}

void
repo_dump(FILE *fp)
{
	int n;
	char *data, *image;
	size_t datalen, imagelen;
	RepositoryHeader_t header;
	struct in_addr in;

	while ((n = repo_read_entry(fp, &header, &data, &datalen, &image, &imagelen)) > 0) {
		if (n == 3) {
			goto next;
		} else if (n == 2) { /* No more data */
			break;
		}

		/* Print document information */
		printf("Document: %s\n", header.url);
		in.s_addr = header.IPAddress;
		printf("\taddress: %s\n", inet_ntoa(in));
		printf("\tresponse code: %d\n", header.response);

		/* Print HTML */
		printf("Html:\n%s\n", data);

 next:
		free(data);
		free(image);
	}
}

int
main(int argc, char **argv)
{
	FILE *fp;

	if (argc < 2)
		usage();

	fp = fopen(argv[1], "r");
	if (fp == NULL)
		err(1, "Unable to open repository file: %s", argv[1]);
	
	repo_dump(fp);
	fclose(fp);

	return 0;
}
