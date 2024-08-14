/****************************************************************************
 *
 * FUNCTION	FreeJanusMem();
 *
 * SYNOPSIS	UBYTE FreeJanusMem(Offset,Type);
 *
 * INPUT	 	UWORD	Offset		;Offset returned by AllocJanusMem
 *				UBYTE	Type			;Type of memory to free MEMB_PARAMETER or
 *																		MEMB_BUFFER 
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK, Doesn't return if type,offset is wrong!
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

#define LINT_ARGS

UBYTE FreeJanusMem(Offset,Type)
UWORD Offset;
UBYTE Type;
{
	union REGS r;

	r.h.ah = 0x03;
	r.	h.al = Type;
	r.x.bx = Offset;

	int86(JFUNC_JINT,&r,&r);

	return(r.h.al);
}
