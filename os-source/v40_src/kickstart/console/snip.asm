**
**	$Id: snip.asm,v 36.8 90/11/20 18:45:42 darren Exp $
**
**      console.device snip manipulation commands
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"


**	Exports

	XDEF	GetConSnip
	XDEF	SetConSnip
	XDEF	AddConSnipHook
	XDEF	RemConSnipHook


**	Imports

	XLVO	AddTail			; Exec
	XLVO	AllocVec		;
	XLVO	FreeVec			;
	XLVO	ObtainSemaphore		;
	XLVO	ReleaseSemaphore	;
	XLVO	Remove			;


*****i* console.device/GetConSnip ************************************
*
*   NAME
*	GetConSnip -- Get a copy of the current console snip (V36)
*
*   SYNOPSIS
*	snip = GetConSnip();
*	D0
*
*	char *GetConSnip( void );
*
*   FUNCTION
*	This command allocates a buffer for the current console snip
*	using exec's AllocVec function, fills the buffer with the
*	current snip, and returns the pointer to the buffer.  The
*	snip is null terminated.  It is the responsibility of the
*	caller to FreeVec the buffer when it is no longer in use.
*	Note that there is no linkage between the snip returned and
*	the actual console snip that prevents the latter from
*	changing after this call returns.
*
*   RESULT
*	snip -- the data the console device would use for a paste.
*	     A null result indicates either there is no snip or
*	     there was no memory to allocate a copy of it.
*
**********************************************************************
GetConSnip:
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore
		move.l	cd_SelectionSnip(a6),d0
		beq.s	gcsRelease
		move.l	d0,a0
		moveq	#0,d0
		move.w	snip_Length(a0),d0
		addq.l	#1,d0
		moveq	#0,d1
		LINKEXE	AllocVec
		tst.l	d0
		beq.s	gcsRelease

		move.l	d0,a1
		move.l	cd_SelectionSnip(a6),a0
		move.w	snip_Length(a0),d1
		addq.l	#snip_Data,a0
		bra.s	gcsCopyDBF
gcsCopyLoop:
		move.b	(a0)+,(a1)+
gcsCopyDBF:
		dbf	d1,gcsCopyLoop
		clr.b	(a1)

gcsRelease:
		move.l	d0,-(a7)
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		move.l	(a7)+,d0
		rts


*****i* console.device/SetConSnip ************************************
*
*   NAME
*	SetConSnip -- Set the current console snip (V36)
*
*   SYNOPSIS
*	success = SetConSnip(snip);
*	D0                   A0
*
*	BOOL SetConSnip( char * );
*
*   FUNCTION
*	This command copies the provided null terminated snip buffer
*	into the console's private snip.  The snip is null terminated.
*	Note that there is no linkage between the snip provided and
*	the actual console snip -- changing the former after this call
*	has no effect on the latter.
*
*   INPUT
*	snip -- a null terminated snip whose length is <= 65535.
*
*   RESULT
*	success -- usually true.  false if out of memory.
*
**********************************************************************
SetConSnip:
		movem.l	d2/a2,-(a7)
		move.l	a0,a2
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		;-- release old snip
		move.l	cd_SelectionSnip(a6),d0
		beq.s	scsSizeSnip
		move.l	d0,a1
		subq.w	#1,snip_Access(a1)
		bpl.s	scsSizeSnip
		LINKEXE	FreeVec

		;-- determine snip length
scsSizeSnip:
		move.l	a2,a0
		moveq	#-1,d2
scsSnipLen:
		tst.b	(a0)+
		dbeq	d2,scsSnipLen
		not.l	d2

		;-- acquire new snip
		move.l	d2,d0
		addq.l	#snip_Data,d0
		moveq	#0,d1
		LINKEXE	AllocVec
		move.l	d0,cd_SelectionSnip(a6)
		bne.s	scsCopySnip
		moveq	#0,d2
		bra.s	scsRelease

scsCopySnip:
		move.l	d0,a1
		clr.w	snip_Access(a1)
		move.w	d2,snip_Length(a1)
		moveq	#1,d2
		addq.l	#snip_Data,a1
scsCopyLoop:
		move.b	(a2)+,d0
		beq.s	scsRelease
		move.b	d0,(a1)+
		bra.s	scsCopyLoop

scsRelease:
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		move.l	d2,d0
		movem.l	(a7)+,d2/a2
		rts


*****i* console.device/AddConSnipHook ********************************
*
*   NAME
*	AddConSnipHook -- Install a console snip hook
*
*   SYNOPSIS
*	AddConSnipHook(hook)
*	               A0
*
*	void AddConSnipHook( struct Hook * );
*
*   FUNCTION
*	This command installs a hook that is called from the console
*	task to inform an application when a selection is available.
*	This notification occurs after the selection is complete, it
*	does not occur while the selection is in progress.  The hook
*	code may not in turn initiate any console functions or
*	commands, but the hook data is available while the hook itself
*	is called.
*
*   INPUT
*	hook - a standard hook.  Note that the h_MinNode is used by
*	    the console device, so must not be used by the hook owner.
*
*   HOOK ENVIRONMENT
*	hook message - a SnipHookMsg, as defined in devices/console.h
*	    shm_Type - 0, indicating that the message has the
*		following fields:
*	    shm_SnipLen - the length of the snip (not including the
*		null termination)
*	    shm_Snip - the null terminated snip.
*	hook object - the IO_UNIT associated with the source of the
*		selection.
*
*   NOTES
*	Intended for PRIVATE use only - generally only 1 hook should
*	be installed; there is a maximum of 255 hooks allowed - once
*	again though due to poor arbitration, the hook functions
*	are intended for private use only.  The addition of a
*	counter has been added to act as both a FLAG, and some
*	minimal protection without actually intending to serve
*	as an error condition.  Assuming we call this function
*	more then once in the future, the counter will suffice.
*
*	The only intended use of this function is to install a
*	program which copies text snips to the clipboard.device.
*	This utility will also have a read feature which CON:
*	will use.  Once a ConSnipHook is installed, the
*	internal pasting (into the circular buffer) is disabled.
*	This preserves V36 behavior, but also allows us to start
*	using the clipboard.device.
*
**********************************************************************
AddConSnipHook:
		move.l	a0,-(a7)
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		move.l	(a7)+,a1
		lea	cd_SelectionHooks(a6),a0
		LINKEXE	AddTail

		addq.b	#1,cd_Hooks(a6)

		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		rts


*****i* console.device/RemConSnipHook ********************************
*
*   NAME
*	RemConSnipHook -- Remove a console snip hook
*
*   SYNOPSIS
*	RemConSnipHook(hook)
*	               A0
*
*	void RemConSnipHook( struct Hook * );
*
*   FUNCTION
*	This command removes a hook installed by AddConSnipHook.
*	The hook will not be called after this function returns.
*
*   INPUT
*	hook - the hook passed to AddConSnipHook.
*
**********************************************************************
RemConSnipHook:
		move.l	a0,-(a7)
		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		move.l	(a7)+,a1
		LINKEXE	Remove

		subq.b	#1,cd_Hooks(a6)

		lea	cd_SelectionSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore
		rts

	END
