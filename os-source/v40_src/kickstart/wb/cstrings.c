/*
 * $Id: cstrings.c,v 38.1 91/06/24 11:34:14 mks Exp $
 *
 * $Log:	cstrings.c,v $
 * Revision 38.1  91/06/24  11:34:14  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "startup.h"
#include "global.h"
#include "support.h"

char *scopy(s)
char *s;
{
char *mem;
	mem = ALLOCVEC(strlen(s) + 1, MEMF_PUBLIC);
	if (mem) strcpy(mem, s);
	return(mem);
}

/*
    suffix checks to see if suf is the suffix of s.
    If so, returns pointer to suffix in s, else returns zero.
    Made case insensitive by David Berezowski on Jan 19/1990
    by changing 'strcmp' to 'ctricmp'.
*/
char *suffix(s, suf)
char *s;
char *suf;
{
	long slen;
	long suflen;
	char *tail;

	slen = strlen(s);
	suflen = strlen(suf);
	if (suflen > slen) return(NULL);
	else {
		tail = s + slen - suflen;
		if (stricmp(tail, suf)) return(NULL);
		else return(tail);
	}
}
