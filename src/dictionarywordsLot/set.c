/*
 * June, 2007
 *
 * Eirik A. Nygaard
 */

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "set.h"

#define SET_START_SIZE 2

int
set_init(set *s)
{
	s->size = 0;
	s->len = SET_START_SIZE;
	if ((s->set = malloc(sizeof(char *) * s->len)) == NULL)
		return 0;

	return 1;
}

void
set_free(set *s)
{
	free(s->set);
}

void
set_free_all(set *s)
{
	int i;
	char *p;

	SET_FOREACH(i, s, p) {
		free(p);
	}
	set_free(s);
}

int
_set_find(set *s, char *str)
{
	int low, high, cur;

	if (s->size == 0)
		return -1;
	low = 0;
	high = s->size-1;
	while (low <= high) {
		int ret;
		cur = (high+low)/2;
		assert(cur >= 0 && cur < s->size);

		ret = strcmp(s->set[cur], str);
		if (ret == 0)
			return cur;
		else if (ret > 0)
			high = cur-1;
		else /* ret < 0 */
			low = cur+1;
	}

	return -1;
}

/*
 * Returns true if a str is in the set
 */
int
set_exists(set *s, char *str)
{
	return (_set_find(s, str) != -1);
}

int
set_grow(set *s)
{
	int origlen;

	origlen = s->len;
	s->len *= 2;
	if ((s->set = realloc(s->set, s->len * sizeof(char *))) == NULL) {
		s->len = origlen;
		return 0;
	}
	return 1;
}

int
set_add(set *s, char *str)
{
	int i;
	char *p;

	if (s->size == s->len) {
		if (!set_grow(s))
			return 0;
	}

	if (set_exists(s, str))
		return 2;

	/* XXX: Fix the linear search */
	SET_FOREACH(i, s, p) {
		if (strcmp(s->set[i], str) > 0)
			break;
	}

	if (s->size - i > 0) {
		memmove(s->set+(i+1), s->set+i, sizeof(char *) * (s->size - i));
	}
	s->set[i] = str;
	s->size++;
	
	return 1;
}

int
set_intersects(set *s1, set *s2)
{
	int i1, i2;

	i1 = 0;
	i2 = 0;

	while (i1 < s1->size && i2 < s2->size) {
		int ret = strcmp(s1->set[i1], s2->set[i2]);

		if (ret == 0)
			return 1;
		else if (ret < 0)
			i1++;
		else
			i2++;
	}
	
	return 0;
}

int
set_intersect(set *s1, set *s2, set *s)
{
	int i1, i2;

	i1 = 0;
	i2 = 0;

	while (i1 < s1->size && i2 < s2->size) {
		int ret = strcmp(s1->set[i1], s2->set[i2]);

		if (ret == 0) {
			if (!set_add(s, s1->set[i1]))
				return 0;
			i1++;
			i2++;
		} else if (ret < 0) {
			i1++;
		} else {
			i2++;
		}
	}

	return 1;
}


set *
set_clone(set *orig)
{
	set *new;
	set_iterator si;
	char *p;

	new = malloc(sizeof(*new));
	if (new == NULL)
		return NULL;
	if (!set_init(new)) {
		free(new);
		return NULL;
	}

	SET_FOREACH(si, orig, p) {
		set_add(new, strdup(p));
	}

	return new;
}

char *
set_to_string(set *s, char *sep)
{
	set_iterator si;
	char *str = NULL, *p, *endp;
	size_t len, seplen;

	len = 0;
	seplen = strlen(sep);

	SET_FOREACH(si, s, p) {
		if (str == NULL) {
			str = strdup(p);
			len = strlen(p);
		} else {
			size_t tmplen;

			tmplen = len;
			len += seplen;
			len += strlen(p);
			str = realloc(str, len+1);
			endp = str + tmplen;
			strcpy(endp, sep);
			strcpy(endp+seplen, p);
		}
	}
	if (str == NULL)
		return strdup("");

	return str;
}

#ifdef TEST_SET

int
main(int argc, char **argv)
{
	set s;
	set s2;
	set s3;
	char *p;
	int i;

	if (!set_init(&s))
		err(1, "init_set(s)");
	if (!set_init(&s2))
		err(1, "init_set(s2)");
	if (!set_init(&s3))
		err(1, "init_set(s3)");


#define add_str(str, s) printf("Adding: %s\n", str); set_add(&(s), str); printf("Set size: %d\n", (s).size);
	add_str("a", s);
	add_str("b", s);
	add_str("a", s);
	add_str("c", s);
	add_str("c", s);
	add_str("b", s);
	add_str("a", s);

	add_str("d", s2);
	add_str("f", s2);
	add_str("a", s2);
	add_str("e", s2);

	printf("foo\n");
	SET_FOREACH(i, &s, p) {
		printf("1> %s\n", p);
	}
	SET_FOREACH(i, &s2, p) {
		printf("2> %s\n", p);
	}

	printf("Intersects: %d\n", set_intersects(&s, &s2));

	if (!set_intersect(&s, &s2, &s3)) {
		printf("Unable to intersect...\n");
	} else {
		SET_FOREACH(i, &s3, p) {
			printf("3> %s\n", p);
		}
	}

	set_free(&s);
	set_free(&s2);
	set_free(&s3);
	
	return 0;
}

#endif /* TEST_SET */
