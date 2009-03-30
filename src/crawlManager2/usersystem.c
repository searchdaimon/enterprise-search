#include <stdio.h>

#include "usersystem.h"
#include "../perlembed/perlembed.h"


/* XXX: Untested */
int
us_authenticate_perl(usersystem_data_t *data, char *user, char *password)
{
	usersystem_perl_t *usp = data->usc->usersystem.us_perl;
	HV *params = newHV();
	SV *in_user = newSVpv(user, strlen(user));
	SV *in_pass = newSVpv(password, strlen(password));
	int ret;
	
	ht_to_perl_ht(params, data->parameters);

	hv_store(params, "in_user", strlen("in_user"), sv_2mortal(newRV((SV *)in_user)), 0);
	hv_store(params, "in_pass", strlen("in_pass"), sv_2mortal(newRV((SV *)in_pass)), 0);
	if ((ret = perl_embed_run(usp->perlpath, "Usersystem::list_users", params, NULL, NULL)))
		errx(1, "perl error");

	printf("Ret: %d\n", ret);

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

	hv_store(params, "users", strlen("users"), sv_2mortal(newRV((SV *)av)), 0);
	if (!perl_embed_run(usp->perlpath, "Usersystem::list_users", params, NULL, NULL))
		errx(1, "perl error");

	if (av_len(av) == -1) /* No users */
		return 0;

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

			
	return 0;
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

	hv_store(params, "groups", strlen("groups"), sv_2mortal(newRV((SV *)av)), 0);
	hv_store(params, "in_username", strlen("in_username"), sv_2mortal(newRV((SV *)username)), 0);
	if (!perl_embed_run(usp->perlpath, "Usersystem::list_groups", params, NULL, NULL))
		errx(1, "perl error");

	if (av_len(av) == -1) /* No groups */
		return 0;

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

	hv_store(params, "name", strlen("name"),  sv_2mortal(newRV((SV *) perl_name)), 0);

	if (!perl_embed_run(usp->perlpath, "Usersystem::get_name", params, NULL, NULL))
		name = "error";
	else
		name = SvPV(perl_name, data_size);

	return strdup(name);
}
