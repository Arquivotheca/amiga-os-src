/* -----------------------------------------------------------------------
 * types.h   telnet
 *
 * $Locker:  $
 *
 * $Id: types.h,v 1.1 91/01/15 18:03:36 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inetx/src/telnet/RCS/types.h,v 1.1 91/01/15 18:03:36 bj Exp $
 *
 * $Log:	types.h,v $
 * Revision 1.1  91/01/15  18:03:36  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
 * Copyright (c) 1988 Regents of the University of California.
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
 *	@(#)types.h	1.3 (Berkeley) 6/29/88
 */

typedef struct {
    char *modedescriptions;
    char modetype;
} Modelist;

extern Modelist modelist[];

typedef struct {
    int
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	gotDM;			/* when did we last see a data mark */
} Clocks;

extern Clocks clocks;
