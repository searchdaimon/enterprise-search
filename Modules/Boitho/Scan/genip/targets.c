/*
 * $Id: targets.c,v 1.1 2007/06/06 22:04:08 dagurval Exp $
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "targets.h"

#define	MAX_ERR_LEN	128

char	errbuf[MAX_ERR_LEN] = "";

char *
target_list_err (void) {
	return(errbuf);
}

int
init_target_list (struct target_list *targets, char *xarg) {
	char		*hostspec;
	char		*target_net, *s, *r;
	char		*field[5];
	int		i, j, k, namedhost = 0;
	int		start, end;
	struct hostent	*lookup;
	unsigned long	l;

	memset(targets, 0, sizeof(*targets));
	hostspec = strdup(xarg);

	target_net = strtok_r(hostspec, "/", &r);
	s = strtok_r(NULL, "", &r);
	targets->netmask = s ? atoi(s) : 32;
	if ((int)targets->netmask < 0 || targets->netmask > 32) {
		strncpy(errbuf, "Mask out of range", MAX_ERR_LEN);
		return(0);
	}

	for (i = 0; *(hostspec + i); i++)
		if (isupper((int) *(hostspec +i)) || islower((int) *(hostspec +i))) {
			namedhost = 1;
			break;
		}

	if (targets->netmask != 32 || namedhost) {
		targets->maskformat = 1;
		if (!inet_aton(target_net, &(targets->start))) {
			if ((lookup = gethostbyname(target_net)))
				memcpy(&(targets->start), lookup->h_addr_list[0], sizeof(struct in_addr));
			else {
				strncpy(errbuf, "Failed to resolve address", MAX_ERR_LEN);
				return(0);
			}
		};

		l = ntohl(targets->start.s_addr);
		targets->start.s_addr = l & (unsigned long)(0 - (1<<(32 - targets->netmask)));
		targets->end.s_addr = l | (unsigned long)((1<<(32 - targets->netmask)) - 1);
		targets->current_addr = targets->start;

		if (targets->start.s_addr <= targets->end.s_addr) {
			targets->nleft = targets->end.s_addr - targets->start.s_addr + 1;
			return(1);
		}

		strncpy(errbuf, "Unrecognised target specification", MAX_ERR_LEN);
		return(0);
	} else {

		targets->maskformat = 0;
		r = hostspec;
		i = 0;
		field[0] = field[1] = field[2] = field[3] = field[4] = NULL;
		field[0] = r = hostspec;

		while (*++r) {
			if (*r == '.' && ++i < 4) {
				*r = '\0';
				field[i] = r + 1;
			} else if (*r == '[') {
				*r = '\0';
			field[i]++;
			} else if (*r == ']') {
			} else if (*r != '*' && *r != ',' && *r != '-' && !isdigit((int)*r)) {
				strncpy(errbuf, "Illegal character in host specification", MAX_ERR_LEN);
				return(0);
			}
		}
		if (i != 3) {
			strncpy(errbuf, "Not enough octects", MAX_ERR_LEN);
			return(0);
		}

		for (i = 0; i < 4; i++) {
			j = 0;
			while ((s = strchr(field[i], ','))) {
				*s = '\0';
				if (*field[i] == '*') {
					start = 0; end = 255;
				} else if(*field[i] == '-') {
					start = 0;
					if (!field[i] + 1) end = 255;
					else end = atoi(field[i]+1);
				} else {
					start = end = atoi(field[i]);
					if ((r = strchr(field[i], '-')) && *(r+1))
						end = atoi(r+1);
					else if (r && !*(r+1)) end = 255;
				}

				if (start < 0 || start > end) {
					strncpy(errbuf, "Illegal range", MAX_ERR_LEN);
					return(0);
				}

				for (k = start; k <= end; k++)
					targets->addresses[i][j++] = k;
				field[i] = s + 1;
			}

			if (*field[i] == '*') {
				start = 0;
				end = 255;
			} else if (*field[i] == '-') {
				start = 0;
				if (!field[i] + 1) end = 255;
				else end = atoi(field[i]+1);
			} else {
				start = end = atoi(field[i]);
				if ((r = strchr(field[i], '-')) && *(r+1)) end = atoi(r+1);
					else if (r && !*(r+1)) end = 255;
			}

			if (start < 0 || start > end) {
				strncpy(errbuf, "Illegal range", MAX_ERR_LEN);
				return(0);
			}
			if (j + (end - start) > 255) {
				strncpy(errbuf, "Range too large", MAX_ERR_LEN);
				return(0);
			}

			for (k = start; k <= end; k++)
				targets->addresses[i][j++] = k;
			targets->last[i] = j - 1;
		}
	}

	memset(&targets->current_idx, 0, sizeof(targets->current_idx));
	targets->nleft = (targets->last[0] + 1) * (targets->last[1] + 1) *
		(targets->last[2] + 1) * (targets->last[3] + 1);

	return(1);
}

int
next_target (struct target_list *targets, struct in_addr *target) {
	int		i;

	if (targets->nleft <= 0)
		return(0);

	if (targets->maskformat) {
		if (targets->current_addr.s_addr <= targets->end.s_addr) {
			target->s_addr = htonl(targets->current_addr.s_addr++);
		} else {
			strncpy(errbuf, "Bad targets structure", MAX_ERR_LEN);
			return(0);
		}
	} else {

		target->s_addr = htonl(targets->addresses[0][targets->current_idx[0]] << 24 |
		    targets->addresses[1][targets->current_idx[1]] << 16 |
		    targets->addresses[2][targets->current_idx[2]] << 8 |
		    targets->addresses[3][targets->current_idx[3]]);

		for (i = 3; i >= 0; i--) {
			if (targets->current_idx[i] < targets->last[i]) {
				targets->current_idx[i]++;
				break;
			} else {
				targets->current_idx[i] = 0;
			}
		}

		if (i == -1) {
			targets->current_idx[0] = targets->last[0];
			targets->current_idx[1] = targets->last[1];
			targets->current_idx[2] = targets->last[2];
			targets->current_idx[3] = targets->last[4] + 1;
		}

	}

	targets->nleft--;

	return(1);
}
