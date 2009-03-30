#ifndef _SEARCHKERNEL_MERGE_H_
#define _SEARCHKERNEL_MERGE_H_

struct iindexFormat;

void or_merge(struct iindexFormat **c, int *baselen, struct iindexFormat **a, int alen, struct iindexFormat **b, int blen);
void andNot_merge(struct iindexFormat **c, int *baselen, int *added,struct iindexFormat **a, int alen, struct iindexFormat **b, int blen);
void and_merge(struct iindexFormat *c, int *baselen, int originalLen, int *added, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen);
void andprox_merge(struct iindexFormat *c, int *baselen, int originalLen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen);
void frase_stopword(struct iindexFormat *c, int clen);
void frase_merge(struct iindexFormat *c, int *baselen,int Originallen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen);

#endif
