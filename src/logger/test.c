
#include "logger.h"

int
main(int argc, char **argv)
{
	bblog_init("test");

	bblog(DEBUG, "debug");
	bblog(INFO, "info");
	bblog(WARN, "warn");
	bblog(ERROR, "error %d", 5);

	//while (1) sleep(5);

	bblog_destroy();
	return 0;
}
