#ifndef _COMM_H_
#define _COMM_H_

/* Communication commands */
#define	COMM_HELP	0x00000001 /* I need to phone home for help, aka: init */
#define	COMM_GETCONN	0x00000002 /* I need to phone home for help, aka: init */
       	            	           /* Port and addr argument */
#define COMM_OK		0x00000003 /* Verify sent command, ready to go */

#endif /* _COMM_H_ */
