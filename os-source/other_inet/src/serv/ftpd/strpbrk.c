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
 */

#include <sys/param.h>
#ifndef UCHAR_MAX
#define UCHAR_MAX 0xFF
#endif

char *
strpbrk(str, chars)
	register char *str, *chars;
{
	static u_char charmap[UCHAR_MAX];
	register int ch;
	register char *p;

	for (p = chars; ch = *p++;)
		charmap[ch] = 1;
	p = NULL;
	for (; ch = *str; ++str)
		if (charmap[ch]) {
			p = str;
			break;
		}
	while (ch = *chars++)
		charmap[ch] = 0;
	return(p);
}
