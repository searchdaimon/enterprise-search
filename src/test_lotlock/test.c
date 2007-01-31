       #include <stdio.h>
       #include <sys/time.h>
       #include <sys/types.h>
       #include <unistd.h>

       int
       main(void)
       {
           fd_set rfds;
           struct timeval tv;
           int retval;

	FILE *LOTLOCK;

	LOTLOCK = fopen("test.loc","r+");

           /* Watch stdin (fd 0) to see when it has input. */
           FD_ZERO(&rfds);
           FD_SET(fileno(LOTLOCK), &rfds);
           /* Wait up to five seconds. */
           tv.tv_sec = 5;
           tv.tv_usec = 0;

           retval = select(fileno(LOTLOCK) +1, NULL, &rfds, NULL, &tv);
           /* Don't rely on the value of tv now! */

           if (retval)
               printf("Data is available now. retval: %i\n",retval);
               /* FD_ISSET(0, &rfds) will be true. */
           else
               printf("No data within five seconds.\n");

	fclose(LOTLOCK);
	
           exit(0);
       }

