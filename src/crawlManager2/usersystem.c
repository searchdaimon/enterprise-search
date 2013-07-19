#include <stdio.h>
#include <EXTERN.h>
#include <perl.h>

#include "usersystem.h"
#include "../perlembed/perlembed.h"
#include "../crawl/crawl.h"

#define PERL_LIST_GROUPS "PerlUserSystem::list_groups"
#define PERL_LIST_USERS "PerlUserSystem::list_users"
#define PERL_GET_NAME "PerlUserSystem::_internal_get_name"
#define PERL_AUTH_USER "PerlUserSystem::authenticate"


int us_authenticate_perl(usersystem_data_t *data, char *user, char *password) {

	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	HV *params = newHV();
	int ret;	
	struct us_cargsF cargs;


        HV *obj_attr = newHV();
        hv_store(obj_attr, "ptr", strlen("ptr"), sv_2mortal(newSViv(PTR2IV(&cargs))), 0);


	ht_to_perl_ht(params, data->parameters);

	hv_store(params, "Enduser", strlen("Enduser"), sv_2mortal(newSVpv(user, 0)), 0);
	hv_store(params, "Endpassword", strlen("Endpassword"), sv_2mortal(newSVpv(password, 0)), 0);


	ret = perl_embed_run(usp->perlpath, PERL_AUTH_USER, params, "PerlUserSystem", obj_attr, NULL, 0);

	if (ret == -1) {
		warnx("perl error");
		return 0;
	}

	return ret;
}

int us_listUsers_perl(usersystem_data_t *data, char ***users, int *n_users) {

	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	HV *params = newHV();
	struct us_cargsF cargs;


        HV *obj_attr = newHV();
        hv_store(obj_attr, "ptr", strlen("ptr"), sv_2mortal(newSViv(PTR2IV(&cargs))), 0);
	
	ht_to_perl_ht(params, data->parameters);

	if (perl_embed_run_arr(usp->perlpath, PERL_LIST_USERS, params, "PerlUserSystem", obj_attr, NULL, 0, users, n_users) == -1) {
		warnx("perl error");
		*n_users = 0;
		*users = NULL;
		return 0;
	}

	printf(" n_users=%d\n", *n_users);

	return 1;

}


int us_listGroupsForUser_perl(usersystem_data_t *data, const char *user, char ***groups, int *n_groups) {

	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	HV *params = newHV();
	struct us_cargsF cargs;


        HV *obj_attr = newHV();
        hv_store(obj_attr, "ptr", strlen("ptr"), sv_2mortal(newSViv(PTR2IV(&cargs))), 0);
	
	ht_to_perl_ht(params, data->parameters);
	hv_store(params, "Enduser", strlen("Enduser"), sv_2mortal(newSVpv(user, 0)), 0);

	if (perl_embed_run_arr(usp->perlpath, PERL_LIST_GROUPS, params, "PerlUserSystem", obj_attr, NULL, 0, groups, n_groups) == -1) {
		warnx("perl error");
		*n_groups = 0;
		*groups = NULL;
		return 0;
	}

	printf(" n_groups=%d\n", *n_groups);

	return 1;
}

char *us_getName_perl(void *data) {

	usersystem_perl_t *usp = data;
	char *name;
	HV *params = newHV();
	SV *perl_name = newSVpv("", strlen(""));
	STRLEN data_size;

	hv_store(params, "_internal_name", strlen("_internal_name"),  sv_2mortal(newRV((SV *) perl_name)), 0);

	if (!perl_embed_run(usp->perlpath, PERL_GET_NAME, params, NULL, NULL, NULL, 0))
		name = "error";
	else
		name = SvPV(perl_name, data_size);

	return strdup(name);
}
