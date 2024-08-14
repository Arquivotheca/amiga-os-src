
* *****************************************************************************
* 
* lib.asm -- skeleton library support
* 
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
* 
* Date       Name               Description
* ---------  -----------        -----------------------------------------------
* 15-Jul-88  -RJ                Changed all files to work with new includes
* 
* *****************************************************************************


		INCLUDE "assembly.i"

		NOLIST
		INCLUDE "exec/types.i"
		INCLUDE "exec/nodes.i"
		INCLUDE "exec/lists.i"
		INCLUDE "exec/ports.i"
		INCLUDE "exec/libraries.i"

		INCLUDE "hardware/intbits.i"

		INCLUDE "janus/janusbase.i"
		LIST

		INCLUDE "asmsupp.i"



		XDEF	Open
		XDEF	Close
		XDEF	Expunge
		XDEF	Null

		XLIB	FreeMem
		XLIB	CloseLibrary
		XLIB	Remove
		XLIB	RemIntServer


Open:
		addq.w	#1,LIB_OPENCNT(a6)
		bclr	#LIBB_DELEXP,LIB_FLAGS(a6)

		move.l	a6,d0
		rts

Close:
		subq.w	#1,LIB_OPENCNT(a6)
		bne.s	Close_Return

		;------ check for delayed expunge
		btst	#LIBB_DELEXP,LIB_FLAGS(a6)
		beq.s	Close_Return

		;------ this is a delayed expunge
		bsr	Expunge
		bra.s	Close_end


Close_Return:
		CLEAR	d0
Close_end:
		rts




Expunge:
;------
;------ janus.library will never be expunged because the PC has no mechanism
;------ to restart it if it does not exist.
;------
;------ This code has been kept for historical purposes only, it has not
;------ been maintained throughout changes to janus.library.
;------ 
;		tst.w	LIB_OPENCNT(a6)
;		bne.s	Expunge_delayed
;
;		movem.l d2/a5/a6,-(sp)
;
;		;------ save seglist for later return
;		move.l	jb_SegList(a6),d2
;
;		;------ keep execbase in a6 so we needn't link to it.
;		move.l	jb_ExecBase(a6),a5
;		exg	a5,a6
;
;		move.l	jb_DOSBase(a5),a1
;		CALLSYS CloseLibrary
;
;		;------ remove us from the library list
;		move.l	a5,a1
;		CALLSYS Remove
;
;		;------ turn off the interrupt server
;		lea	jb_IntServer(a5),a1
;		moveq	#INTB_PORTS,d0
;		CALLSYS RemIntServer
;
;		;------ free the library memory
;		move.l	a5,a1
;		CLEAR	d0
;		move.w	LIB_NEGSIZE(a5),d0
;		sub.w	d0,a1
;		add.w	LIB_POSSIZE(a5),d0
;		CALLSYS FreeMem
;
;
;		;------ return the seg list
;		move.l	d2,d0
;		movem.l (sp)+,d2/a5/a6
;		bra.s	Expunge_end
;
Expunge_delayed:
		bset	#LIBB_DELEXP,LIB_FLAGS(a6)
		CLEAR	d0

Expunge_end:
		rts



Null:
		moveq	#0,d0
		rts


	end

