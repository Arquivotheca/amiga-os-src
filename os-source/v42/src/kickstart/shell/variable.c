#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/var.h>
#include <clib/macros.h>
#include <string.h>
#include <stddef.h>

#include "global.h"


 /*	Expand any macros in str */
/* modified to expand the buffer if required */
/* takes pointers to the buffer and size */
int expand(gv,to,size)
    struct Global *gv;
    UBYTE **to;
    LONG *size;
{
    struct Library *UtilityBase=gv->UtilityBase;
    UBYTE *from;
    LONG fromsize;
    UBYTE buf[40]; /* note: 32 chars is limit for variable names */
    UBYTE *temp;
    ULONG tempsize = 128;
    UBYTE  *q;
    UBYTE *brace;
    UBYTE *bracend;
    int i,ch;
    UBYTE lastfrom=0;
    LONG pos,newsize;

    // make sure from is large enough to hold the original string
    if (!extend(&(gv->buffer),&(gv->buffersize),strlen(*to)+1))
	return -1;

    if (!(temp = AllocMem(tempsize,MEMF_PUBLIC)))
	return -1;

    from = gv->buffer;
    fromsize = gv->buffersize;

    strcpy(from, *to);
    pos = 0;

/* add * escape here */

    while (*from) {
	if (*from != '$') {
	    if(lastfrom == '*') {
		lastfrom=0; /* two stars escape into one star */
	    }
	    else lastfrom=*from;
	    pos = addchar(pos,to,size,*from++);
	}
	/* it is a $ */
	else if (lastfrom == '*') {
	    lastfrom=0;
	    (*to)[pos-1] = *from++;	/* lose the * */
	    // no need for addchar, since it can't overflow
	}
	else {
	    q = buf;
	    brace = from;
	    bracend = from;
	    i=0;
	    if (*++from == '{') {
		while (*++from && *from != '}' && (i++ < 32 )) *q++ = *from;
	    	bracend=from+1;
	    	if (*from) from++;   /* if not at end of string continue */
	    }
	    else if (!*from) {
		pos = addchar(pos,to,size,'$');
		break;
	    }
	    else if(*from == '$')*q++ = *from++;
	    else {
		i=0;
		while ( (i++ < 32) && 
		    ( (((ch = ToUpper(*from)) >= '0') && (ch <= '9')) || 
		    ((ch >= 'A') && (ch <='Z')) || 
		    (ch >160))) *q++ = *from++;
	    	bracend=from;
	    }
	    *q = '\0';

	    if (MyGetVar(gv,buf,&temp,&tempsize,0) < 0 ) {
		for(; brace<bracend; brace++) 
			pos = addchar(pos,to,size,*brace);
	    }
	    else {
		// adding it in one go is much faster
		newsize = pos + strlen(temp);
		if (extend(to,size,newsize))
		{
			strcpy(&((*to)[pos]),temp);
			pos = newsize;
		}
	    }
	}
    }
    FreeMem(temp,tempsize);

    (*to)[pos] = '\0';
    return (int) strlen(*to);
}

/* force buffer to minimum of the new size - contents copied over */
int
extend (UBYTE **buf, LONG *size, LONG newsize)
{
	UBYTE *newbuf;

	if (newsize > *size)
	{
		newsize = MAX((*size)<<1,newsize);
		newbuf = AllocMem(newsize,MEMF_PUBLIC);
		if (!newbuf)
			return 0; // failure

		CopyMem(*buf,newbuf,*size);
		FreeMem(*buf,*size);
		*buf = newbuf;
		*size = newsize;
	}
	return 1; // success
}

LONG
addchar (LONG pos, UBYTE **buf, LONG *size, UBYTE ch)
{
	// make sure it's got enough space for the character and the null
	if (extend(buf,size,pos+1))
	{
		(*buf)[pos++] = ch;
	}
	// else fail to add char - should really ripple an error! FIX

	return pos;
}
