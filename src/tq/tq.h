#ifndef _TQ__H_
#define _TQ__H_


#define TCOUNT 27
#define MAX_Q 12

struct tqFormat {
        int     count;
        int     have_data;

        void    *q[MAX_Q];

        pthread_mutex_t count_mutex;
        pthread_cond_t count_empty_cv;
        pthread_cond_t count_full_cv;
};


void tq_init(struct tqFormat *tqh);
int tq_get(struct tqFormat *tqh, void **data);
void tq_add(struct tqFormat *tqh, void *data);
void *tq_end(struct tqFormat *tqh);
void *tq_free(struct tqFormat *tqh);

#endif
