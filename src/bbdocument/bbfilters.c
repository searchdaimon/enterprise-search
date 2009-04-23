#include "bbfilters.h"
#include "../perlembed/perlembed.h"
#include <stdio.h>
#include <string.h>
#include "../common/boithohome.h"
#include "../common/ht.h"

#define LIBEXTRACTOR_PATH "/home/dagurval/bin/bin/extract"
//#define LIBEXTRACTOR_PATH "/home/dagurval/websearch/fakeextract/extract"

void run_filter_perlplugin(char *dst, size_t dst_size, struct fileFilterFormat *filter, struct hashtable **metahash) {
	char perlpath[PATH_MAX];	
	snprintf(perlpath, sizeof perlpath, "%smain.pm", filter->path);

	HV *perl_metahash = newHV();
	SV *perl_dst = newSVpv("", strlen(""));
	//AV *perl_extracted_files = newAV();
	HV *params = newHV();

	hv_store(params, "file", strlen("file"), sv_2mortal(newSVpv(filter->command, 0)), 0);
	hv_store(params, "metadata", strlen("metadata"), sv_2mortal(newRV((SV *) perl_metahash)), 0);
	hv_store(params, "data", strlen("data"),  sv_2mortal(newRV((SV *) perl_dst)), 0);
	//hv_store(params, "extracted_files", strlen("extracted_files"),  sv_2mortal(newRV((SV *) perl_extracted_files)), 0);

	#ifdef DEBUG
		printf("perl run: %s:dump(file=%s, metadata=%p)\n",perlpath,filter->command,perl_metahash);
	#endif

	if(!perl_embed_run(perlpath, "dump", params, NULL, NULL))
		errx(1, "Perlplugin error on '%s'", filter->command);

	STRLEN data_size;
	char *data = SvPV(perl_dst, data_size);


	// asuming data is a '\0'-terminated string
	strlcpy(dst, data, dst_size);

	if (metahash) {
		*metahash = create_hashtable(3, ht_stringhash, ht_stringcmp);
		perl_ht_to_ht(perl_metahash, *metahash);
	}

	// clean up
	hv_undef(perl_metahash);
	hv_undef(params);
	free(data); 

}

#ifdef USE_LIBEXTRACTOR

int in_list(const char *key, char **list) {
	int i;
	for (i = 0; list[i] != NULL; i++) {
		if (strcmp(list[i], key) == 0)
			return 1;
	}
	return 0;
}

void parse_libextractor_output(struct hashtable *dst, const char *output, char **whitelist) {
	char **lines;
	int n_lines = split(output, "\n", &lines);
	char key[MAX_ATTRIB_LEN];
	char val[MAX_ATTRIB_LEN];

	printf("parse_libextractor_output: n_lines %i\n",n_lines);
	
	int i, j;
	for (i = 0; i < n_lines; i++) {
		char *line = lines[i];
		char *remain = strchr(line, '-');
		if (remain == NULL)
			continue;
		int keylen = remain - line;
		remain += 2; // skip '- '
		
		keylen = keylen > MAX_ATTRIB_LEN ? MAX_ATTRIB_LEN : keylen;
		strlcpy(key, line, keylen);
		strlcpy(val, remain, sizeof val);

		if (!in_list(key, whitelist)) {
			//warnx("ignoring attr %s - %s\n", key, val);
			continue;
		}

		hashtable_insert(dst, strdup(key), strdup(val));
		//warnx("inserted attr %s - %s\n", key, val);
	}
	FreeSplitList(lines);
}

void add_libextractor_attr(struct hashtable **metadata, char *filepath, char **whitelist) {

	if (!metadata) {
		warnx("metadata not used, not running libextractor %d %s", __LINE__, __FILE__);
		return;
	}
	
	int buflen = 1024 * 1024;
	int retval;
	char *buf = malloc(sizeof(char) * buflen);
	char *args[] = { LIBEXTRACTOR_PATH, filepath, NULL };

	printf("add_libextractor_attr runing: %s %s\n",LIBEXTRACTOR_PATH,filepath);

	if (!exeoc_timeout(args, buf, &buflen, &retval, 120)) {
		warnx("exec_timeout failed %d %s\n", __LINE__, __FILE__);
		free(buf);
		return;
	}
	// incorrect retval ?
	/*if (retval) {
		warnx("%s executed with error %d\n", LIBEXTRACTOR_PATH, retval);
		free(buf);
		return;
	}*/
	parse_libextractor_output(*metadata, buf, whitelist);
	free(buf);
}
#endif


void run_filter_exeoc(char *dst, size_t dst_size, struct fileFilterFormat *fileFilter, struct hashtable **metahash) {
	#ifdef DEBUG
	printf("command: %s\n",(*fileFilter).command);
	#endif

	char **splitdata;
        int TokCount;


	//hvis vi skal lage en ny fil må vi slette den gamle
	//sletter den etterpå i steden. Men før vi kaller return
	//if (strcmp((*fileFilter).outputformat,"textfile") == 0) {
	//	unlink(filconvertetfile_out_txt);
	//}

	//her parser vi argumenter selv, og hver space blir en ny argyment, selv om vi 
	//bruker "a b", som ikke riktig blir to argumenter her, a og b
	//splitter på space får å lage en argc
	TokCount = split((*fileFilter).command, " ", &splitdata);
	//#ifdef DEBUG
	printf("splitet comand in %i, program is \"%s\"\n",TokCount,splitdata[0]);
	//#endif
	printf("running: %s\n",(*fileFilter).command);
	//sender med størelsen på buferen nå. Vil få størelsen på hva vi leste tilbake


	char *envpairpath = strdup("/tmp/converter-metadata-XXXXXX");
	char envpair[PATH_MAX];
	mktemp(envpairpath);
	sprintf(envpair, "SDMETAFILE=%s", envpairpath);
	free(envpairpath);
	envpairpath = envpair + strlen("SDMETAFILE=");
        char *shargs[] = { "/usr/bin/env", NULL, "/bin/sh", "-c", NULL, NULL, };
	shargs[1] = envpair;
        shargs[4] = fileFilter->command;

	int retval;
	int exeocbuflen = dst_size;

	if (!exeoc_timeout(shargs, dst, &exeocbuflen, &retval,120)) {

		printf("dident get any data from exeoc. But can be a filter that creates files, sow we wil continue\n");
		//kan ikke sette den til 0 da vi bruker den får å vite hvos stor bufferen er lengere nede
		//(*dst) = 0;
		dst[0] = '\0';
		//return 0;

	}

	if (metahash) {
		FILE *metafp;

		*metahash = create_hashtable(3, ht_stringhash, ht_stringcmp);

		if ((metafp = fopen(envpairpath, "r")) != NULL) {
			char *key, *value, line[2048];

			printf("Fooop\n");
			while (fgets(line, sizeof(line), metafp)) {
				char *p, *p2;

				/* Comment */
				if (line[0] == '#')
					continue;

				key = line;
				p = strchr(key, '=');
				if (p == NULL) {
					fprintf(stderr, "Invalid format on meta spec file: %s\n", line);
					continue;
				}
				p2 = p;
				while (isspace(*(p2-1)))
					p2--;
				*p2 = '\0';
				p++; /* Skip past = */
				while (isspace(*p))
					p++;
				value = p;
				while (isspace(*key))
					key++;

				if (value[strlen(value)-1] == '\n')
					value[strlen(value)-1] = '\0';
				printf("Got pair: %s = %s\n", key, value);
				hashtable_insert(*metahash, strdup(key), strdup(value));
			}
			fclose(metafp);
			unlink(envpairpath);
		} else {
			printf("Couldn't open %s\n", envpairpath);
		}
	}
}


