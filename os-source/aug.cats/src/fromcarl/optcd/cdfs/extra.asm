; Copyright (C) 1985 by Manx Software Systems, Inc.
; :ts=8
;
; (KMY 2708)
; Taken from Manx 3.6a lib source
; - Changed _SysBase to SysBase
; - added INCLUDE "stddefs.i"
; - merged all into one set
; - removed need for strlen

	INCLUDE "stddefs.i"

	public	SysBase

	public	_Allocate
_Allocate
	move.l	4(sp),a0
	move.l	8(sp),d0
	move.l	SysBase,a6
	jmp	-186(a6)


	public	_Deallocate
_Deallocate
	movem.l	4(sp),a0/a1
	move.l	12(sp),d0
	move.l	SysBase,a6
	jmp	-192(a6)


	public	_InitSemaphore
_InitSemaphore
	move.l	4(sp),a0
	move.l	SysBase,a6
	jmp	-558(a6)


	public	_AttemptSemaphore
_AttemptSemaphore
	move.l	4(sp),a0
	move.l	SysBase,a6
	jmp	-576(a6)


	public	_ReleaseSemaphore
_ReleaseSemaphore
	move.l	4(sp),a0
	move.l	SysBase,a6
	jmp	-570(a6)
