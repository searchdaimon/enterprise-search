
#include <stdio.h>
#include "suggest.h"


void
suggest_1(char *host, char *arg)
{
	CLIENT *clnt = NULL;
	enum clnt_stat retval_1;
	numbest_res result_1;
	char * get_best_results_1_arg;
	namelist nl;

#ifndef DEBUG
	clnt = clnt_create (host, SUGGEST, SUGGESTVERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif  /* DEBUG */

	retval_1 = get_best_results_1(arg, &result_1, clnt);

	if (retval_1 != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}
	else {
		for (nl = result_1.numbest_res_u.list;
				nl != NULL;
				nl = nl->next) {
			printf("%s\n", nl->name);
		}

	}


#ifndef DEBUG
	clnt_destroy (clnt);
#endif   /* DEBUG */
}



int
main(int argc, char **argv)
{
	int i;
	numbest_res res;

	if (argc < 3) {
		fprintf(stderr, "Not enough arguments: ./prog host prefix1 [prefix2 ...]\n");
		return 1;
	}

	for(i = 2; i < argc; i++) {
		printf("Checking: %s\n", argv[i]);

		suggest_1(argv[1], argv[i]);
	}

	return 0;
}
