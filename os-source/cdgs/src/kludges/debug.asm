**
**	$Id: debug.asm,v 1.1 92/11/20 15:32:55 jerryh Exp $
**
**	Conditional debugging code
**
**	(C) Copyright 1991-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

	INCLUDE	"debug.i"

	IFNE	DEBUG_DETAIL
	XDEF	kprint_macro

kprint_macro:

		IFND	KPrintF
		XREF	KPrintF
		ENDC

	movem.l	d0-d1/a6,-(sp)
	jsr	KPrintF
	movem.l	(sp)+,d0-d1/a6
	rts


	ENDC

