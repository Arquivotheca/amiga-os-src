/****************************************************************************
 *
 * FUNCTION	GetBase();
 *
 * SYNOPSIS	UBYTE GetBase(Service,ParmSeg,ParmOff,BuffSeg);
 *
 * INPUT		UBYTE Service		;1st generation service number
 *				UWORD *ParmSeg		;Address of UWORD to receive Segment
 *				UWORD *ParmOff		;Address of UWORD to recieve ParmOffset
 *				UWORD *BuffSeg		;Address of UWORD to recieve BuffOffset
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK, JSERV_NOJANUSBASE
 *
 *				ParmSeg, ParmOff, and BuffOff are updated if no error.
 *				ParmOffset = -1 if not defined.
 *
 ***************************************************************************/
#include <dos.h>
#include <janus\janus.h>

UBYTE GetBase(Service,ParmSeg,ParmOff,BuffSeg)
UBYTE Service;
UWORD *ParmSeg,*ParmOff,*BuffSeg;
{
	union REGS r;
	struct SREGS sr;

	r.h.al = Service;
	r.h.ah = 0x01;

	int86x(JFUNC_JINT,&r,&r,&sr);
	segread(&sr);

	*ParmSeg = sr.es;
	*ParmOff = r.x.di;
	*BuffSeg = r.x.dx;

	return(r.h.al);

}
