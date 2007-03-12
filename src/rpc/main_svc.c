
#include <stdio.h>
#include <errno.h>

#include "main.h"
#include "suggest.h"
#include "../suggest/suggest.h"

bool_t
get_best_results_1_svc(char *arg1, numbest_res *result,  struct svc_req *rqstp)
{
	namelist nl, *nlp;
	struct suggest_input **si;

	printf("Looking up: %s\n", arg1);
	si = suggest_find_prefix(suggest_data, arg1);
	printf("Looking up: %s\n", arg1);
	if (si == NULL) {
		result->_errno = errno;
		return 1;
	}
	nlp = &(result->numbest_res_u.list);
	for (; *si != NULL; si++) {
		nl = malloc(sizeof(namenode));
		*nlp = nl;
		if (nl == NULL) {
			/* XXX: memleak? */
			result->_errno = errno;
			return 1;
		}
		nl->name = (*si)->word;
		//nl->frequency = (*si)->frequency;
		nlp = &nl->next;
	}
	*nlp = NULL;
	result->_errno = 0;


	return 1;
}

int
suggest_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	return 1;
}

