#ifndef _SPELLING_H_
#define _SPELLING_H_

typedef struct {
	char inited;
	struct hashtable *words;
	struct hashtable *soundslike;
} spelling_t;

int train(spelling_t *s, const char *dict);
void untrain(spelling_t *s);
int correct_word(spelling_t *s, wchar_t *word);
char *check_word(spelling_t *s, char *word, int *found);

#endif /* _SPELLING_H_ */
