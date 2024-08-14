/*
 * Put up a requester.  Text may be up to 3 lines long.  Two buttons
 * may be specified.  proc returns 0 or 1 corresponding to a click
 * over one of the buttons.
 */

#include <stdarg.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>

struct IntuitionBase *IntuitionBase;
extern struct Library *SysBase;

struct EasyStruct es = {
    sizeof (struct EasyStruct),
    0,
    "Network Problem",
    NULL,
    NULL,
};

BOOL
requester(UBYTE *positive, UBYTE *negative, char *fmt, ...)
{
	UBYTE *args;
	BOOL rtn, openedintuition;
	char gadgets[32];
	ULONG iflags = 0;

	va_start(args,fmt);

	openedintuition = 0;
  	if(IntuitionBase == (struct IntuitionBase *)0L){
		IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
		if(IntuitionBase == 0){
			return (FALSE);
		}
		openedintuition++;
	}

	es.es_TextFormat = fmt;
	gadgets[0] = (char)0;

	if (positive) {
		strncat(gadgets,positive,15);
		if (negative) strcat(gadgets,"|");
	}
	if(negative)
		strncat(gadgets,negative,15);

	es.es_GadgetFormat = gadgets;

	rtn = EasyRequestArgs(NULL,&es,&iflags,args);

	va_end(args);

	if(openedintuition){
		CloseLibrary(IntuitionBase);
		IntuitionBase = (struct IntuitionBase *)NULL;
	}

	return ( rtn==1 ? TRUE : FALSE);
}

/**
main()
{
	long a = 37;
	long b = 147;
	if (requester("YES","no","a=%ld  b=%ld",a,b))
		requester("YES","NO","Right %s","martin");
}
**/
