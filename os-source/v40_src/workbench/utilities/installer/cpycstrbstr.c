/*****************************************************
 *		Parsec Soft Systems string functions		 *
 *****************************************************/

#include "exec/types.h"
#include "string.h"
#include "functions.h"
#include "xfunctions.h"

#define CTOB(cptr)    ((long)(cptr) >> 2)

char *CpyCstrBstr(char *c,char *b)	/* b is a real pointer, we return BPTR */
{	b[0] = strlen(c);
	CopyMem(c,b+1,b[0]);
	return (char *)CTOB(b);
}

