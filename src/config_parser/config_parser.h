
#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

int read_config( char *filename );
char *config_value( char *variable );
int config_value_int( char *variable );

#endif	// _CONFIG_PARSER_H_
