#include <stdio.h>

#include "usersystem.h"
#include "../perlembed/perlembed.h"

#define PERL_LIST_GROUPS "PerlUserSystem::_internal_list_groups"
#define PERL_LIST_USERS "PerlUserSystem::_internal_list_users"
#define PERL_GET_NAME "PerlUserSystem::_internal_get_name"

/* XXX: Untested */
int
us_authenticate_perl(usersystem_data_t *data, char *user, char *password)
{
	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	HV *params = newHV();
	SV *in_user = newSVpv(user, strlen(user));
	SV *in_pass = newSVpv(password, strlen(password));
	SV *out_retval = newSViv(-1);
	
	ht_to_perl_ht(params, data->parameters);

	hv_store(params, "_in_user", strlen("_in_user"), sv_2mortal(newRV((SV *)in_user)), 0);
	hv_store(params, "_in_pass", strlen("_in_pass"), sv_2mortal(newRV((SV *)in_pass)), 0);
	hv_store(params, "_out_retval", strlen("_out_retval"), sv_2mortal(newRV((SV *)in_pass)), 0);
	if (!perl_embed_run(usp->perlpath, "Usersystem::_internal_authenticate", params, NULL, NULL)) {
		warnx("perl error");
		return 0;
	}
	int ret = SvIV(out_retval);
	if (ret == -1) {
		warnx("no return value from perl (auth)");
		return 0;
	}

	return ret;
}

int
us_listUsers_perl(usersystem_data_t *data, char ***users, int *n_users)
{
	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	AV *av = newAV();
	HV *params = newHV();
	int i;
	
	av_store( av, 0, &PL_sv_undef );
	ht_to_perl_ht(params, data->parameters);

	hv_store(params, "_internal_users", strlen("_internal_users"), sv_2mortal(newRV((SV *)av)), 0);
	if (!perl_embed_run(usp->perlpath, PERL_LIST_USERS, params, NULL, NULL)) {
		warnx("perl error");
		return 0;
	}

	if (av_len(av) == -1) { /* No users */
		*n_users = 0;
		*users = NULL;
		return 1;
	}

	*n_users = av_len(av)+1;
	*users = malloc(((*n_users)+1) * sizeof(char*));
	i = 0;
	while (av_len(av) != -1) {
		SV *user = av_pop(av);
		STRLEN data_size;
		char *suser;
		
		suser = SvPV(user, data_size);
		(*users)[i] = strdup(suser);
		i++;
	}
	(*users)[i] = NULL;

			
	return 1;
}


int
us_listGroupsForUser_perl(usersystem_data_t *data, const char *user, char ***groups, int *n_groups)
{
	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	AV *av = newAV();
	HV *params = newHV();
	SV *username = newSVpv(user, strlen(user));
	int i;
	
	av_store( av, 0, &PL_sv_undef );
	ht_to_perl_ht(params, data->parameters);

	hv_store(params, "_internal_groups", strlen("_internal_groups"), sv_2mortal(newRV((SV *)av)), 0);
	hv_store(params, "_in_username", strlen("_in_username"), sv_2mortal(newRV((SV *)username)), 0);
	if (!perl_embed_run(usp->perlpath, PERL_LIST_GROUPS, params, NULL, NULL)) {
		warnx("perl error");
		*n_groups = 0;
		*groups = NULL;
		return 0;
	}

	if (av_len(av) == -1) { /* No groups */
		*n_groups = 0;
		*groups = NULL;
		return 1;
	}

	*n_groups = av_len(av)+1;
	*groups = malloc(((*n_groups)+1) * sizeof(char*));
	i = 0;
	while (av_len(av) != -1) {
		SV *group = av_pop(av);
		STRLEN data_size;
		char *sgroup;
		
		sgroup = SvPV(group, data_size);
		(*groups)[i] = strdup(sgroup);
		i++;
	}
	(*groups)[i] = NULL;

	return 1;
}

char *
us_getName_perl(void *data)
{
	usersystem_perl_t *usp = data;
	char *name;
	HV *params = newHV();
	SV *perl_name = newSVpv("", strlen(""));
	STRLEN data_size;

	hv_store(params, "_internal_name", strlen("_internal_name"),  sv_2mortal(newRV((SV *) perl_name)), 0);

	if (!perl_embed_run(usp->perlpath, PERL_GET_NAME, params, NULL, NULL))
		name = "error";
	else
		name = SvPV(perl_name, data_size);

	return strdup(name);
}
