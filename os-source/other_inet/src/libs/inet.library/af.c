/*
 * Copyright (c) 1983, 1986 Regents of the University of California.
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
 *	@(#)af.c	7.4 (Berkeley) 6/27/88
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/af.h>

#include "proto.h"

/*
 * Address family support routines
 */

#define	AFNULL 		{ null_hash,	null_netmatch }

#ifdef INET
	#define	AFINET 	{ inet_hash,	inet_netmatch }
#else
	#define	AFINET	AFNULL
#endif

#ifdef NS
	#define	AFNS 	{ ns_hash,	ns_netmatch }
#else
	#define	AFNS	AFNULL
#endif

struct afswitch afswitch[AF_MAX] = {
	AFNULL,	AFNULL,	AFINET,	AFINET,	AFNULL,
	AFNULL,	AFNS,	AFNULL,	AFNULL,	AFNULL,
	AFNULL, AFNULL, AFNULL, AFNULL, AFNULL,
	AFNULL, AFNULL,					/* through 16 */
};

void null_init(void)
{
	register struct afswitch *af;

	for (af = afswitch; af < &afswitch[AF_MAX]; af++)
		if (af->af_hash == (int (*)())NULL) {
			af->af_hash = null_hash;
			af->af_netmatch = null_netmatch;
		}
}

/*ARGSUSED*/
void null_hash(addr, hp)
	struct sockaddr *addr;
	struct afhash *hp;
{
	hp->afh_nethash = hp->afh_hosthash = 0;
}

/*ARGSUSED*/
int null_netmatch(a1, a2)
	struct sockaddr *a1, *a2;
{
	return (0);
}
