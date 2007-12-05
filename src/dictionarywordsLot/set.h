

#ifndef _SET_H_
#define _SET_H_

struct set {
	int size;
	int len;

	char **set;
};

typedef struct set set;
typedef int set_iterator;

#define SET_FOREACH(si, s, p) for (si = 0, p = si < (s)->size ? (s)->set[si] : NULL; \
                                   si < (s)->size; \
                                   si++, p = si < (s)->size ? (s)->set[si] : NULL)

int set_init(set *);
int set_exists(set *, char *);
int set_intersects(set *, set *);
int set_intersect(set *, set *, set *);
int set_add(set *, char *);
void set_free(set *);
void set_free_all(set *);


#endif /* _SET_H_ */
