*  $Id: dispatchregs.asm,v 1.2 1992/01/27 00:18:14 johnw Exp $

	xdef	_dispatchRegs

* ULONG __saveds __asm dispatchRegs (register __a2 (*)(),register __a0 struct AppInfo *ai,register __d0 STRPTR,register __a1 struct TagItem *);

_dispatchRegs:
	move.l	a1,-(sp)		; struct TagItem *Attrs
	move.l	d0,-(sp)		; STRPTR CmdLine
	move.l	a0,-(sp)		; struct AppInfo *ai
	jsr	(a2)
	lea	12(sp),sp
	rts
