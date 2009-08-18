#include "../common/define.h"
#include "../cgi-util/cgi-util.h"
#include "../base64/base64.h"
#include "../common/bstr.h"
#include <err.h>
#include "cgihandler.h"
#include "library.h"


int _lookup_lang(const char *lang) {
	// TODO: lookup table
	if (strcmp(lang, "no") == 0)
		return LANG_NBO;
	if (strcmp(lang, "nb") == 0)
		return LANG_NBO;
	if (strcmp(lang, "nn") == 0)
		return LANG_NBO;
	
	if (strcmp(lang, "en") == 0)
		return LANG_ENG;
	if (strcmp(lang, "en_us") == 0)
		return LANG_ENG;

	return LANG_UNKNOWN;
	
}

void dispatcher_cgi_init(void) {
	int res = cgi_init();
	if (res != CGIERR_NONE)
		errx(1, "cgi init error %d: %s", res, cgi_strerror(res));
}

int cgi_access_type(char *remoteaddr, char *correct_bbkey) {
	if (strcmp(remoteaddr, "127.0.0.1") == 0)
		return ACCESS_TYPE_FULL;

	const char *bbkey = cgi_getentrystr("bbkey");
	warnx("correct: %s, provided: %s\n", correct_bbkey, bbkey);

	/*extern char **environ;
	char **next = environ;
	while (*next) {
		warnx("environ %s", *next);
		next++;
	}*/

	// Limited access if no key, but user is logged in
	if (bbkey == NULL) {
		if (getenv("REDIRECT_REMOTE_USER") != NULL) {
			return ACCESS_TYPE_LIMITED;
		}
		
		warn("no key, no logged in user");
		return ACCESS_TYPE_NONE;
	}


	if (strcmp(bbkey, correct_bbkey) == 0)
		return ACCESS_TYPE_FULL;

	return ACCESS_TYPE_NONE;
}

void cgi_set_defaults(struct QueryDataForamt *qdata) {
#ifdef BLACK_BOKS
	qdata->anonymous = 0;
#endif
	char *user = getenv("REDIRECT_REMOTE_USER");
	if (user == NULL)
		qdata->search_user[0] = '\0';
	else
		strscpy(qdata->search_user, user, sizeof(qdata->search_user));

	qdata->subname[0] = '\0';
	qdata->orderby[0] = '\0';
	qdata->filterOn = 1;
	qdata->opensearch = 0;
	qdata->version = 2.0;
	qdata->navmenucfg[0] = '\0';
	qdata->MaxsHits = DefultMaxsHits;
	qdata->lang = LANG_UNKNOWN;
	

	// legacy
	qdata->AmazonAssociateTag[0] = '\0';
	qdata->AmazonSubscriptionId[0] = '\0';
	qdata->rankUrl[0] = '\0';
}

// parameters that are handled the
// same way for both limited and full access
void cgi_fetch_common(struct QueryDataForamt *qdata, int *noDocType) {
	if (cgi_getentrystr("query") == NULL)
		die(2,"","No query provided.");
	
	strscpy(qdata->query, cgi_getentrystr("query"), sizeof(qdata->query));

        *noDocType = (cgi_getentrystr("noDoctype") == NULL) ? 1 : 0;

	int tmpint;
	qdata->start = ((tmpint = cgi_getentryint("start")) != 0) 
		? tmpint
		: 1;


	if ((tmpint = cgi_getentryint("opensearch")) != 0)
		qdata->opensearch = cgi_getentryint("opensearch");

	const char *tmpstr;
	if ((tmpstr = cgi_getentrystr("lang")) != NULL) {
		int lang = _lookup_lang(tmpstr);
		if (lang) qdata->lang = lang;
	}
}



void cgi_fetch_limited(struct QueryDataForamt *qdata, char *remoteaddr) {
	
	// Assuming remote address is from logged in user (API access)
	strscpy(qdata->userip, remoteaddr, sizeof qdata->userip);

	char *tmpstr;
	if ((tmpstr = getenv("HTTP_ACCEPT_LANGUAGE")) != NULL)
		strscpy(qdata->HTTP_ACCEPT_LANGUAGE, tmpstr, sizeof(qdata->HTTP_ACCEPT_LANGUAGE));
	else {
		qdata->HTTP_ACCEPT_LANGUAGE[0] = '\0';
		warnx("HTTP_ACCEPT_LANGUAGE not available");
	}
	if ((tmpstr = getenv("HTTP_USER_AGENT")) != NULL)
		strscpy(qdata->HTTP_USER_AGENT, tmpstr, sizeof(qdata->HTTP_USER_AGENT));
	else {
		qdata->HTTP_USER_AGENT[0] = '\0';
		warnx("HTTP_USER_AGENT not available");
	}
	qdata->HTTP_REFERER[0] = '\0';
	qdata->version = 2.1;

	

}


void _cgistr_to_str(char *dst, char *cgi_key, size_t dst_len) {
	const char *tmpstr;
	if ((tmpstr = cgi_getentrystr(cgi_key)) != NULL)
		strscpy(dst, tmpstr, dst_len);
	else {
		dst[0] = '\0';
		warnx("%s not provided", cgi_key);
	}
}

void cgi_fetch_full(struct QueryDataForamt *qdata) {
	const char *tmpstr;

#ifdef TFSO_HACK
	if (cgi_getentrystr("search_bruker") == NULL) {
                char *p;

                strscpy(qdata->subname,cgi_getentrystr("subname"),sizeof(qdata->subname) -1);
                if (strncmp(qdata->subname, "email-", 6) == 0) {
                        strlcpy(qdata->search_user, qdata->subname+6, sizeof(qdata->search_user));
                } else {
                        strlcpy(qdata->search_user, qdata->subname, sizeof(qdata->search_user));
                }

		//fjerner eventuelt -body
                p = strrchr(qdata->search_user, '-');
                if (p != NULL) {
                        if (strcmp(p, "-body") == 0)
                                *p = '\0';
                }

                p = strrchr(qdata->subname, '-');
                if (p != NULL) {
                        if (strcmp(p, "-body") == 0)
                                *p = '\0';
                }
	
		warnx("tfso hack search_user \"%s\"\n",qdata->search_user);
		warnx("tfso hack subname \"%s\"\n",qdata->subname);
	}
#else
	if (0) {

	}	
#endif
	else {

	#ifdef BLACK_BOKS
		if (cgi_getentryint("anonymous") != 0) {
			qdata->anonymous = 1;
			strscpy(qdata->search_user, ANONYMOUS_USER, sizeof(qdata->search_user));
		}
		if (!qdata->anonymous) {
		#endif
			if ((tmpstr = cgi_getentrystr("search_bruker")) != NULL) {
				strscpy(qdata->search_user, tmpstr, sizeof(qdata->search_user));
			}
			if (qdata->search_user[0] == '\0')
				errx(1, "search_bruker missing");
		#ifdef BLACK_BOKS
		}
		#endif
		if ((tmpstr = cgi_getentrystr("subname")) != NULL) {
			/* 25.05.09: eirik
			 * Subname has changed it's meaning.
			 * If subname is non-empty we only want these collection.
			 * Empty(strlen(str) == 0) is everyone
			 */
			strscpy(qdata->subname, tmpstr, sizeof(qdata->subname));
		}
	}
	// Assuming remote address is *not* from remote user,
	// but from our client (webclient) or similar.
	if ((tmpstr = cgi_getentrystr("userip")) != NULL) {
		strscpy(qdata->userip, tmpstr, sizeof(qdata->userip));
	}
	else {
		warnx("User IP not provided.");
		qdata->userip[0] = '\0';
	}

	if ((tmpstr = cgi_getentrystr("orderby")) != NULL)
		strscpy(qdata->orderby, tmpstr, sizeof(qdata->orderby));

	int tmpint;
	if (tmpint = cgi_getentryint("filter")) 
		qdata->filterOn = tmpint;

	_cgistr_to_str(qdata->HTTP_ACCEPT_LANGUAGE, "HTTP_ACCEPT_LANGUAGE", sizeof qdata->HTTP_ACCEPT_LANGUAGE);
	_cgistr_to_str(qdata->HTTP_USER_AGENT, "HTTP_USER_AGENT", sizeof qdata->HTTP_USER_AGENT);
	_cgistr_to_str(qdata->HTTP_REFERER, "HTTP_REFERER", sizeof qdata->HTTP_REFERER);


	if (cgi_getentrydouble("version") != 0) 
		qdata->version = cgi_getentrydouble("version");

	if ((tmpstr = cgi_getentrystr("navmenucfg")) != NULL) {
		base64_decode(qdata->navmenucfg, tmpstr, sizeof qdata->navmenucfg);
	}
	else {
		warnx("navmenucfg not provided");
		qdata->navmenucfg[0] = '\0';
	}

	if ((tmpint = cgi_getentryint("maxhits")) != 0) 
		qdata->MaxsHits = tmpint;
}

