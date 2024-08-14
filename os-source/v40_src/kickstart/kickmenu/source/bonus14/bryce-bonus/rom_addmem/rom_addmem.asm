**
**	rom_addmem - A tiny module that adds A3000 memory to the memory list.
**	Works under V1.3 or V1.4; is intended for the "Bonus ROM".
**
**	This code has a hard cap of 16MB fast memory.
**


*************************************************************************
*									*
*   Copyright 1984,85,88,89,90 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************


DEBUG		EQU 0
DEBUG_DETAIL	EQU 0


		INCLUDE "exec/types.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/resident.i"
		INCLUDE "exec/memory.i"
		IFNE	DEBUG
		INCLUDE "exec/macros.i"
		ENDC



;;;;		dc.w	$1111
;;;;		jmp	$00fc0002



;[A6=ExecBase, A2=ExpansionBase]
addmemresidentTag:
		DC.W	RTC_MATCHWORD
		DC.L	addmemresidentTag
		DC.L	addmemEndMarker
		DC.B	RTF_COLDSTART
		DC.B	36		; version
		DC.B	0		; no type
		DC.B	115		;TODO 115	      ; priority
		DC.L	IDString
		DC.L	IDString
		DC.L	addthemem



RAM_END 	EQU $08000000-(512*1024)
RAM_STEPSIZE	EQU 256*1024
RAM_PRIORITY	EQU 20
RAM_LOWBOUND	EQU $07000000+RAM_STEPSIZE  ;Cap search at 16MB



addthemem:	move.l	d2,-(sp)
	       ;move.l	4,a6

		move.l	#RAM_END,d0		;D0=RAM_END
		move.l	d0,a0

tm_MunchMoreMem:
		suba.l	#RAM_STEPSIZE,a0
		cmpa.l	#RAM_LOWBOUND-RAM_STEPSIZE,a0
		beq	tm_EndRange
		PRINTF	11,<'  t:$%lx'>,a0
		move.l	(a0),d1
		not.l	d1
		not.l	(a0)
		move.l	$10,a1			;magic (confuse bus)
		cmp.l	(a0),d1
		bne.s	tm_EndMem
		not.l	d1
		not.l	(a0)
		move.l	$10,a1			;magic (confuse bus)
		cmp.l	(a0),d1
		beq.s	tm_MunchMoreMem
tm_EndMem:

		;[D0=RAM_END]
		adda.l	#RAM_STEPSIZE,a0	;A0=base
		lea.l	memname(pc),a1          ;A1=name
		sub.l	a0,d0			;D0=size (size=end-base)
		beq   tm_NoMem
		move.l	#MEMF_PUBLIC!MEMF_FAST!MEMF_LOCAL,d1	;D1=flags
		moveq	#RAM_PRIORITY,d2		;D2=pri

		PRINTF	10,<'Adding %ld($%lx) bytes at %lx'>,d0,d0,a0
		JSRLIB	AddMemList	;size, attributes, pri, base, name
					; d0   d1	   d2	a0    a1
		PRINTF	10,<'After AddMemList (%lx)'>,d0

tm_NoMem:	move.l	(sp)+,d2
		rts

tm_EndRange:
		PRINTF	10,<'End range (%lx)'>,a0
		bra	tm_EndMem

IDString:	dc.b	'rom_addmem',10,0
memname:	dc.b	'32-bit fast memory',0
		CNOP	0,4

addmemEndMarker:


	END
