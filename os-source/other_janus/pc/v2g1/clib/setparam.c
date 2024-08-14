/****************************************************************************
 *
 * FUNCTION	SetParam();
 *
 * SYNOPSIS	UBYTE SetParam(Service,ParmOff);
 *
 * INPUT		UBYTE Service		;1st generation service number
 *				UWORD ParmOff		;Offset for parameter mem
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE SetParam(Service,ParmOff)
UBYTE Service;
UWORD ParmOff;
{
	union REGS r;

	r.h.ah = 0x04;
	r.h.al = Service;
	r.x.bx = ParmOff;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}

