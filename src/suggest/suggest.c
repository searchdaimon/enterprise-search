/*
 * Eirik A. Nygaard
 * February 2007
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/bstr.h"

#include "suggest.h"
#include "suffixtree.h"
#include "acl.h"


int
suggest_add_item(struct suggest_data *sd, char *word, int freq, char *aclallow, char *acldeny)
{
	struct suggest_input *si;

	si = malloc(sizeof *si);
	if (si == NULL)
		return -1;

	si->word = strdup(word);
	if (si->word == NULL) {
		perror("strdup(word)");
		return -1;
	}
	si->frequency = freq;
#ifdef WITH_ACL
	si->aclallow = acl_parse_list(aclallow);
	if (si->aclallow == NULL) {
		perror("acl_parse_list(aclallow)");
		return -1;
	}
	si->acldeny = acl_parse_list(acldeny);
	if (si->acldeny == NULL) {
		perror("acl_parse_list(acldeny)");
		return -1;
	}
#endif

	suffixtree_insert(&(sd->tree), si);

	return 0;
}


struct suggest_data *
suggest_init(void)
{
	struct suggest_data *sg;

	if ((sg = malloc(sizeof(*sg))) == NULL) {
		return NULL;
	}
	suffixtree_init(&(sg->tree));

	return sg;
}

void
suggest_destroy(struct suggest_data *sd)
{
	suffixtree_destroy(&(sd->tree));
	free(sd);
}

void
suggest_destroy_si(struct suggest_input *si)
{
	if (si->aclallow)
		acl_destroy(si->aclallow);
	if (si->acldeny)
		acl_destroy(si->acldeny);
	free(si->word);
	free(si);
}

int
suggest_read_frequency(struct suggest_data *sd, char *wordlist)
{
	FILE *fp;
	int freq, linenum;
	int got;
	char word[1024];
	char aclallow[1024];
	char acldeny[1024];

	if ((fp = fopen(wordlist, "r")) == NULL)
		return -1;

	linenum = 1;
	got = 0;
	while (!feof(fp)) {
		int ret;
		size_t len;
		char *line = NULL;
		char **linedata;
		acldeny[0] = '\0';

		if (getline(&line, &len, fp) == -1) {
			if (line != NULL)
				free(line);
			continue;
		}
		if (line == NULL || line[0] == '\0') {
			free(line);
			continue;
		}
		if (line == NULL || split(line, " ", &linedata) < 2) {
	//	if ((ret = fscanf(fp, "%s %d %s %s\n", word, &freq, aclallow, acldeny)) < 3) {
			printf("'%s' %d\n", line, len);
			fprintf(stderr, "Unable to read parse line %d in %s\n", linenum, wordlist);
			linenum++;
			free(line);
			continue;
		}
		free(line);
		strcpy(word, linedata[0]);
		freq = atol(linedata[1]);
		if (linedata[2])
			strcpy(aclallow, linedata[2]);
		else
			strcpy(linedata[2], "");
		if (linedata[2] && linedata[3])
			strcpy(acldeny, linedata[3]);
		else
			strcpy(linedata[3], "");
		
		/* Free linedata */
		{
			int ifree;

			for(ifree = 0; linedata[ifree] != NULL; ifree++)
				free(linedata[ifree]);
			free(linedata);
		}
		//printf("%s <=> %d\nGot acl allow: %s\nGot acl deny: %s\n", word, freq, aclallow, acldeny);
		if (suggest_add_item(sd, word, freq, aclallow, acldeny) == -1) {
			perror("suggest_add_item()");
		}

		linenum++;
		got++;
		if ((linenum % 100000) == 0) {
			printf("100000 new word!\n");
		}
	}

	printf("Collected: %d\n", got);
	fclose(fp);

	return 0;
}

void
suggest_most_used(struct suggest_data *sd)
{
	suffixtree_most_used(&sd->tree);
}

struct suggest_input **
#ifdef WITH_ACL
suggest_find_prefix(struct suggest_data *sd, char *prefix, char *user)
#else
suggest_find_prefix(struct suggest_data *sd, char *prefix)
#endif
{
#ifndef WITH_ACL
	char *user = NULL;
#endif 
	return suffixtree_find_prefix(&sd->tree, prefix, user);
}


#ifdef TEST_SUGGEST

int
main(int argc, char **argv)
{
	struct suggest_data *sd = suggest_init();
	struct suggest_input **si;
	char *word = "logge"; //"heimdalsmunnen";
	FILE *fp;
	int i;
	struct suggest_input **suffixtree_find_prefix(struct suffixtree *, char *, char *);


	if (sd == NULL) {
		fprintf(stderr, "Could not initialize suggest\n");
		return 1;
	}
	printf("%d\n", suggest_read_frequency(sd, "/home/eirik/Boitho/boitho/websearch/lot/1/1/Exchangetest/dictionarywords"));
	//printf("%d\n", suggest_read_frequency(sd, "testinput.list"));
	//printf("%d\n", suggest_read_frequency(sd, "UnikeTermerMedForekomst.ENG"));
	//printf("%d\n", suggest_read_frequency(sd, "liten.list"));
	//printf("%d\n", suggest_read_frequency(sd, "wordlist.7"));
#if 0
	suffixtree_most_used(&(sd->tree), 5);

	printf("Looking for word: %s\n", word);
	for (i = 0; i < 100; i++)
		suffixtree_find_word(&(sd->tree), word);
#endif

	//printf("Word: %p\n", suffixtree_find_word(&(sd->tree), "typeface"));
	//printf("Word: %p\n", suffixtree_find_word(&(sd->tree), "laure"));
	//printf("Word: %p\n", suffixtree_find_word(&(sd->tree), "nezu"));
	//
	//printf("Word: %x\n", suffixtree_find_word(&(sd->tree), "individualsiaiaiai"));
	//printf("Word: %x\n", suffixtree_find_word(&(sd->tree), "individ"));
	//
	suggest_most_used(sd);
	//si = suggest_find_prefix(sd, "individ");
	
	//printf("si... %x\n", *si);
#if 0
	printf("Printing the most used word starting with 'individ'\n");
	for (; *si != NULL; si++) {
		printf("Something: %s / %d\n", (*si)->word, (*si)->frequency);
	}
#endif
	
#if 0
	{
		char s[1024];
		int foo;
		struct suggest_input *si;
		printf("doing shit\n");
		fp = fopen("UnikeTermerMedForekomst.ENG", "r");
		while (!feof(fp)) {
			if (fscanf(fp, "%s %d\n", s, &foo) != 2)
				continue;
			si = suffixtree_find_word(&(sd->tree), s);
			if (si == NULL) {
				printf("Could not find %s\n", s);
			}
		}
		fclose(fp);
	}
#endif

	//printf("Count %d\n", suffixtree_scan(&(sd->tree), 0));
	{
		struct suffixtree *sf = &(sd->tree);
		int a;

		printf("The most used words in the dictionary:\n");
		for (a=0; sf->best[a] != NULL; a++) {
			printf("%s <=> %d\n", sf->best[a]->word, sf->best[a]->frequency);
		}
	}

	do {
		char *buf;
		size_t len;
		unsigned int size = 1024;
		struct suggest_input *si2;

		buf = NULL;
		printf("Enter prefix: ");
		fflush(stdin);
		len = getline(&buf, &size, stdin);
		buf[len-1] = '\0';
#ifdef WITH_ACL
		si = suggest_find_prefix(sd, buf, "eirik");
#else
		si = suggest_find_prefix(sd, buf);
#endif
		if (si == NULL) {
			printf("No match for '%s'\n", buf);
			free(buf);
			continue;
		}
		else {
			printf("Printing the most used word starting with '%s'\n", buf);
			for (; *si != NULL; si++) {
				printf("Something: %s / %d\n", (*si)->word, (*si)->frequency);
			}
		}
		free(buf);
	} while(1);

	return 0;
}

#endif
