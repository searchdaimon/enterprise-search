#include <stdio.h>
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../common/boithohome.h"
#include <assert.h>
#include <err.h>
#include <string.h>
#include "perlembed.h"


int perl_opt_cache;

/**
  * embed functions
  */


int _chararray_size(char **a) {
	if (a == NULL) return 0;
	int i = 0;
	while (a[i] != NULL) i++;
	return i;
}
		

void perl_embed_init(char **incl_path, int cache_perl_files) {
	static int is_initialized = 0;
	if (is_initialized) {
		fprintf(stderr, "perl_embed_init has already been initialized, ignoring call.\n");
		return;
	}
	is_initialized = 1;

	int incl_n = _chararray_size(incl_path);
	perl_opt_cache = cache_perl_files;
	char sdlib_arg[512];
	snprintf(sdlib_arg, sizeof sdlib_arg, "-Mblib=%s", bfile(SD_CRAWL_LIB_PATH));

        //char *perl_argv[] = { "", blib,  "-I", bfile("crawlers/Modules/"), bfile2("perl/persistent.pl"), NULL };
	char *perl_argv[incl_n ? incl_n + 4 : 3];
	int perl_argc = 0;

	perl_argv[perl_argc++] = "";
	if (incl_n) {
		perl_argv[perl_argc++] = "-I";
		int i;
		for (i = 0; i < incl_n; i++)
			perl_argv[perl_argc++] = incl_path[i];
	}
	perl_argv[perl_argc++] = sdlib_arg;
	perl_argv[perl_argc++] = bfile(PERSISTENT_PATH);
	perl_argv[perl_argc] = NULL;

	
	extern char **environ;
        PERL_SYS_INIT3(&argc, &argv, &environ);
        my_perl = perl_alloc();
        perl_construct(my_perl);

	//int j = 0;
	//while (perl_argv[j++] != NULL)
		//printf("perl argument %s\n", perl_argv[j]);

        perl_parse(my_perl, xs_init, perl_argc, perl_argv, NULL);
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
}

void perl_embed_clean() {
        perl_destruct(my_perl);
        perl_free(my_perl);
        PERL_SYS_TERM();
}

/**
 * file_path    - full path to perl file to load
 * func_name    - function to run within loaded file
 * func_params  - hashref to send to function
 * obj_name     - class name, when run_func is a class method (NULL if not used)
 * obj_attr     - hashref obj_name is blessed with (NULL if not used)
 */
int perl_embed_run(char *file_path, char *func_name, HV *func_params, char *obj_name, HV *obj_attr) {
	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);

	//filnavnet
	XPUSHs(sv_2mortal(newSVpv(file_path, 0) ));

	//mappen, for å inkludere
	//XPUSHs(sv_2mortal(newSVpv(collection->crawlLibInfo->resourcepath, 0) ));

	XPUSHs(sv_2mortal(newSViv(perl_opt_cache))); 
	XPUSHs(sv_2mortal(newSVpv(func_name, 0)));
	XPUSHs(sv_2mortal(newRV((SV *) func_params)));
	if (obj_name != NULL)
		XPUSHs(sv_2mortal(newSVpv(obj_name, 0)));
	if (obj_attr != NULL)
		XPUSHs(sv_2mortal(newRV((SV *) obj_attr)));

	PUTBACK;

	int retn = call_pv("Embed::Persistent::eval_file2", G_SCALAR | G_EVAL);
	//antar at rutiner som ikke returnerer noe mislykkes. Dette kan for eks skje hvis vi kaller die, eller ikke trenger retur koden
	int retv = 0;

	SPAGAIN; //refresh stack pointer
	if (SvTRUE(ERRSV)) {
		fprintf(stderr, "Perl preprocessor error: %s\n", SvPV_nolen(ERRSV));
	}
	else if (retn == 1) {
		//pop the return value, as a int
		retv = POPi;
	}
	else {
		fprintf(stderr, "perlfunc returned %i values, expected 0 or 1. Ignored.\n", retn);
	}

	FREETMPS;
	LEAVE;

	return retv;
}

/**
 * helper functions 
 */

void ht_to_perl_ht(HV *perl_ht, struct hashtable *params) {
    if (!hashtable_count(params)) return;

    struct hashtable_itr *itr;
    itr = hashtable_iterator(params);


    do {
        char *param = hashtable_iterator_key(itr);
        char *value = hashtable_iterator_value(itr);
        
        // check if key already exists
        if (hv_exists(perl_ht, param, strlen(param))) {
            fprintf(stderr, "Parameter '%s' is already defined. Ignoring.\n", param);
            continue;
        }

        hv_store(perl_ht, param, strlen(param),
            sv_2mortal(newSVpv(value, 0)), 0);
        
    } while (hashtable_iterator_advance(itr));
    free(itr);
}

void perl_ht_to_ht(HV *perl_ht, struct hashtable *ht) {
	HE *he;
	STRLEN len;
	I32 len2;
	
	hv_iterinit(perl_ht);
	while ((he = hv_iternext(perl_ht))) {
		SV *val = hv_iterval(perl_ht, he);
		
		char *val_str = strdup(SvPV(val, len));
		char *key_str = strdup(hv_iterkey(he, &len2));

		if (hashtable_search(ht, key_str)) {
			warnx("key '%s' already exists in ht, ignoring", key_str);
			continue;
		}
		hashtable_insert(ht, key_str, val_str);
	}
}


int perl_ht_add_str(HV *ht, char *key, char *val) {
	
	// warn & ignore if key exists
	if (hv_exists(ht, key, strlen(key))) {
		fprintf(stderr, "perl ht key '%s' exists, ignoring", key);
		return 0;
	}
	hv_store(ht, key, strlen(key), sv_2mortal(newSVpv(val, 0)), 0);
	return 1;
}


