/****************************************************************************
 *
 * FUNCTION	GetBase();
 *
 * SYNOPSIS	UBYTE GetBase(Service,ParmSeg,ParmOff,BuffSeg);
 *
 * INPUT		UBYTE Service		;1st generation service number
 *				UWORD *ParmSeg		;Address of UWORD to receive Segment
 *				UWORD *ParmOff		;Address of UWORD to recieve Offset
 *				UWORD *BuffSeg		;Address of UWORD to recieve Offset
 *
 * OUTPUT	returns Error code defined in  services.h
 *					JSERV_OK, JSERV_NOJANUSBASE
 *
 *				ParmSeg, ParmOff, and BuffOff are updated if no error.
 *
 ***************************************************************************/

