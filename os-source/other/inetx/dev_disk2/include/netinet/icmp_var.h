#ifndef NETINET_ICMP_VAR_H
#define NETINET_ICMP_VAR_H
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)icmp_var.h	7.4 (Berkeley) 6/29/88
 */

/*
 * Variables related to this implementation
 * of the internet control message protocol.
 */
struct	icmpstat {
/* statistics related to icmp packets generated */
	long	icps_error;		/* # of calls to icmp_error */
	long	icps_oldshort;		/* no error 'cuz old ip too short */
	long	icps_oldicmp;		/* no error 'cuz old was icmp */
	long	icps_outhist[ICMP_MAXTYPE + 1];
/* statistics related to input messages processed */
 	long	icps_badcode;		/* icmp_code out of range */
	long	icps_tooshort;		/* packet < ICMP_MINLEN */
	long	icps_checksum;		/* bad checksum */
	long	icps_badlen;		/* calculated bound mismatch */
	long	icps_reflect;		/* number of responses */
	long	icps_inhist[ICMP_MAXTYPE + 1];
};

#ifdef KERNEL
struct	icmpstat icmpstat;
#endif
#endif
