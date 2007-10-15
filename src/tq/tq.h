#ifndef _TQ__H_
#define _TQ__H_


//#define tq_MAX_Q 120

struct tqFormat {
        int     count;
        int     have_data;

        //void    *q[tq_MAX_Q];
        void    **q;

        pthread_mutex_t count_mutex;
        pthread_cond_t count_empty_cv;
        pthread_cond_t count_full_cv;

	int qlen;
};

void tq_init(struct tqFormat *tqh, int qlen);
int tq_get(struct tqFormat *tqh, void **data);
void tq_add(struct tqFormat *tqh, void *data);
void *tq_end(struct tqFormat *tqh);
void *tq_free(struct tqFormat *tqh);

#endif
