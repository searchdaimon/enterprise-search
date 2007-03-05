/*
 * Eirik A. Nygaard
 * February 2007
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "suggest.h"
#include "suffixtree.h"


int
suggest_add_item(struct suggest_data *sd, char *word, int freq)
{
	struct suggest_input *si;

	si = malloc(sizeof *si);
	if (si == NULL)
		return -1;

	si->word = strdup(word);
	si->frequency = freq;

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
	free(sd);
}

int
suggest_read_frequency(struct suggest_data *sd, char *wordlist)
{
	FILE *fp;
	int freq, linenum;
	char word[1024];

	if ((fp = fopen(wordlist, "r")) == NULL)
		return -1;

	linenum = 1;
	while (!feof(fp)) {
		if (fscanf(fp, "%s %d\n", word, &freq) != 2) {
			fprintf(stderr, "Unable to read parse line %d in %s\n", linenum, wordlist);
			continue;
		}

		suggest_add_item(sd, word, freq);

		linenum++;
		if ((linenum % 100000) == 0) {
			printf("100000 new word!\n");
		}
	}
		
	printf("Collected: %d\n", linenum);
	fclose(fp);

	return 0;
}

void
suggest_most_used(struct suggest_data *sd)
{
	suffixtree_most_used(&sd->tree);
}

struct suggest_input **
suggest_find_prefix(struct suggest_data *sd, char *prefix)
{
	return suffixtree_find_suffix(&sd->tree, prefix);
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
	struct suggest_input **suffixtree_find_suffix(struct suffixtree *, char *);


	if (sd == NULL) {
		fprintf(stderr, "Could not initialize suggest\n");
		return 1;
	}
	printf("%d\n", suggest_read_frequency(sd, "UnikeTermerMedForekomst.ENG"));
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
	si = suggest_find_prefix(sd, "individ");
	
	//printf("si... %x\n", *si);
#if 1
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

	return 0;
}

#endif
