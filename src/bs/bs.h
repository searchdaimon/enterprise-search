
struct bs {
	int count;
	int max;
	void **t;
};

void *bs_init(struct bs *s, int max);
int bs_add(struct bs *s, void *data);
int bs_get(struct bs *s, void **data);
