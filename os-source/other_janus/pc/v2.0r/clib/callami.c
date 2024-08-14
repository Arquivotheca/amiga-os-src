/****************************************************************************
 *
 * FUNCTION	CallAmiga();
 *
 * SYNOPSIS	UBYTE CallAmiga(Service);
 *
 * INPUT		UBYTE Service		;1st generation service number
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK, JSERV_PENDING. JSERV_FINISHED
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE CallAmiga(Service)
UBYTE Service;
{
	union REGS r;

	r.h.ah = 0x07;
	r.h.al = Service;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}


