/*
 * $Id: targets.h,v 1.1 2007/06/06 22:04:08 dagurval Exp $
 */
#ifndef __TARGETS__
#define __TARGETS__

typedef struct target_list {
	unsigned int	maskformat;
	int		nleft;

	/* These 4 are used for the '/mask' style of specifying target net */
	u_int32_t	netmask;
	struct in_addr	start;
	struct in_addr	end;
	struct in_addr	current_addr;

	/* These 3 are used for "Free Form" target specifications */
	u_int8_t	addresses[4][256];
	unsigned int	current_idx[4];
	u_int8_t	last[4];
} TARGET_LIST;

char *target_list_err (void);
int init_target_list (struct target_list *targets, char *arg);
int next_target (struct target_list *targets, struct in_addr *target);

#endif
