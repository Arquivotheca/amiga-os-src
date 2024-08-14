#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/var.h>
#include <string.h>
#include <stddef.h>

#include "global.h"


 /*	Expand any macros in str */

int expand(gv,to,mc)
    struct Global *gv;
    UBYTE *to;
    LONG mc;
{
    struct DosLibrary *DOSBase=gv->DOSBase;
    struct Library *UtilityBase=gv->UtilityBase;
    UBYTE *from=gv->buffer;
    UBYTE buf[40]; /* note: 32 chars is limit for variable names */
    UBYTE temp[128];
    char  *rp;
    char  *p;
    char  *q;
    char *brace;
    char *bracend;
    int maxlen = mc;
    int i,ch;
    char lastrp=0;

    if((strlen(to))>=mc)return(-1); /* too long, do nothing */

    strcpy(from, to);

    rp = from;
    p = to;

/* add * escape here */

    while (*rp) {
	if (*rp != '$') {
	    if(lastrp == '*') {
		lastrp=0; /* two stars escape into one star */
	    }
	    else lastrp=*rp;
	    *p++ = *rp++;
	    (maxlen)--;
	}
	/* it is a $ */
	else if (lastrp == '*') {
	    lastrp=0;
	    p--;	/* lose the * */
	    *p++ = *rp++;
	    (maxlen)--;
	}
	else {
	    q = buf;
	    brace = rp;
	    bracend = rp;
	    i=0;
	    if (*++rp == '{') {
		while (*++rp && *rp != '}' && (i++ < 32 )) *q++ = *rp;
	    	bracend=rp+1;
	    	if (*rp) rp++;   /* if not at end of string continue */
	    }
	    else if (!*rp) {
		*p++ = '$';
		break;
	    }
	    else if(*rp == '$')*q++ = *rp++;
	    else {
		i=0;
		while ( (i++ < 32) && 
		    ( (((ch = ToUpper(*rp)) >= '0') && (ch <= '9')) || 
		    ((ch >= 'A') && (ch <='Z')) || 
		    (ch >160))) *q++ = *rp++;
	    	bracend=rp;
	    }
	    *q = '\0';

	    if (GetVar(buf,temp,127,0) < 0 ) {
		for(; brace<bracend; brace++)*p++ = *brace;  
	    }
	    else {
		for(q=temp; *q && (*q != '\n')&& maxlen;q++,maxlen--)
			*p++ = *q;
	    }
	}
	if (maxlen <= 0) {
	    to[mc-1]=0; /* null terminate the line */
	    return(-1);
	}
    }
    *p = '\0';
    return(strlen(to));
}
