
#include <aspell.h>

struct spelling {
	AspellSpeller *speller;
};

struct spelling_document {
	struct spelling *spelling;
	AspellDocumentChecker *checker;
};


struct spelling *spelling_init(char *);
void spelling_destroy(struct spelling *);
int spelling_correct(char *, struct spelling *);
char **spelling_suggestions(char *, struct spelling *);
void spelling_suggestions_destroy(char **);
struct spelling_document *spelling_document_init(char *);
char *spelling_document_line(struct spelling_document *, char *);
void spelling_document_destroy(struct spelling_document *);

