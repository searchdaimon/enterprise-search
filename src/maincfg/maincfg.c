#include "../common/boithohome.h"
#include <libconfig.h>

#define cfg_main "config/main.conf"

struct config_t maincfgopen() {

	struct config_t cfg;

	/* Initialize the configuration */
        config_init(&cfg);

	/* Load the file */
        #ifdef DEBUG
	        printf("loading [%s]..\n",bfile(cfg_main));
        #endif

        if (!config_read_file(&cfg, bfile(cfg_main))) {
                printf("[%s]failed: %s at line %i\n",bfile(cfg_main),config_error_text(&cfg),config_error_line(&cfg));
        	exit(1);
        }

	return cfg;
}


int maincfg_get_int(const config_t *cfg, const char *val) {

	config_setting_t *cfgarray;

        if ( (cfgarray = config_lookup(cfg, val) ) == NULL) {
		//printf("can't load \"usecashe\" from config\n");
                //exit(1);
        }

	return config_setting_get_int(cfgarray);
}
