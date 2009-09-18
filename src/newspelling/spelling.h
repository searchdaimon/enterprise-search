#ifndef _SPELLING_H_
#define _SPELLING_H_

typedef struct {
	char inited;
	struct hashtable *words;
	struct hashtable *soundslike;
} spelling_t;

spelling_t *train(const char *dict);
void untrain(spelling_t *s);
int correct_word(const spelling_t *s, char *word);
char *check_word(const spelling_t *s, char *word, int *found);

#endif /* _SPELLING_H_ */
