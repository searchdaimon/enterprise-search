
struct bs {
	int count;
	int max;
	void **t;
	#ifdef WITH_THREAD
		pthread_mutex_t mutex;
	#endif
};

void *bs_init(struct bs *s, int max);
int bs_add(struct bs *s, void *data);
int bs_get(struct bs *s, void **data);
