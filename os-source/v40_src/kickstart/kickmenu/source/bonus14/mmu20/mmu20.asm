**
**	bonus2 - A tiny rom module that holds the MMU tables (V1.4).
**



NUKEMEM 	EQU 0
SCRAMTEST	EQU 1



*************************************************************************
*									*
*   Copyright 1990 Commodore-Amiga, Inc.				*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************


		INCLUDE "exec/types.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/resident.i"
		INCLUDE "exec/memory.i"
		INCLUDE "exec/ables.i"
		INCLUDE "bonus20.i"
		INT_ABLES

		XREF	_LVOSupervisor
		XREF	_vhposr

ABSEXECBASE	equ 4

MAGIC_JUMP	EQU	$00f80002   ;Works for V1.3 or V1.4 roms

END_MEM 	EQU	$08000000
START_TEST	EQU	$07000000   ;start memory test here
END_TEST	EQU	$08000000-K_SIZE-K_SIZE
TEST_GRAN	EQU	1024*1024   ;[dependencies]
K_SIZE		EQU	256*1024
K_MEM		EQU	END_MEM-K_SIZE-K_SIZE


;----------------------------------------------------------------------------
;
;   POSITION INDEPENDENT CODE
;
;
StartCode:	dc.l	$0000feed,$c0edbabe	;Reserved for Exec coldstart
		nop				;Reserved Magik longword for
		nop				;by the KickMenu module


_EnableMMU:	bclr.b	#7,$00de0002		;clear A3000 cold powerup bit
		move.b	#00,$00de0000		;Disable A3000 BERR

		bsr	_SCRAM


		move.l	ABSEXECBASE,a6
		DISABLE
		lea.l	supermode(pc),a5
		JSR	_LVOSupervisor(a6)


supermode:
		*   Relocate table pointers (trashing the checksum).
		*   MMU table is built with all accessed and modified bits
		*   set (so checksum will not change).
		*
		lea.l	LevelA(pc),a0
		lea.l	mmu_CRP+4(pc),a1
		move.l	a0,(a1)

		lea.l	LevelB(pc),a1
		move.l	a1,d0
		or.w	#$a,d0		;accessed page table pointer
		move.l	d0,0*4(a0)

		lea.l	LevelBB(pc),a1
		move.l	a1,d0
		or.w	#$a,d0		;accessed page table pointer
		move.l	d0,7*4(a0)



		*
		*   Mark areas of no memory with invalid descriptors
		*   (data cache must be off).
		*   [This is a last moment hack to create more invalid
		*    memory... it misses a 256K block because of expansion
		*    library sizing needs]
		*
		move.l	#START_TEST,a0
		lea.l	LevelBB(pc),a1
		move.l	#$fffffffc,d2	;AND to invalidate page descriptor
		move.l	(a0),d0
		not.l	d0
		move.l	d0,(a0)
		move.l	$20,d1		;magic: confuse bus
		cmp.l	(a0),d0
		beq.s	huge_machine	;16MB of memory!
		bra.s	enter


more:		move.l	(a0),d0
		not.l	d0
		move.l	d0,(a0)
		move.l	$20,d1		;magic: confuse bus
		cmp.l	(a0),d0
		beq.s	found_memory
enter:		and.l	d2,(a1)+        ;mark as invalid
		and.l	d2,(a1)+        ;mark as invalid
		and.l	d2,(a1)+        ;mark as invalid
		and.l	d2,(a1)+        ;mark as invalid
		add.l	#TEST_GRAN,a0
		cmp.l	#END_TEST,a0
		bne.s	more


found_memory:	or.l	#$01,-16(a1)     ;re-enable start of last 1MB block
huge_machine:	not.l	(a0)            ;fix trashed location



		*
		*   Checksum ROM after messing with the page tables
		*
		bsr.s	kicksum



		*
		*   Turn on MMU.  This write-protects ourself
		*
		lea.l	mmu_CRP(pc),a0
		dc.w	$f010			; PMOVE (a0),CRP [with flush]
		dc.w	$4c00
		lea.l	mmu_TC(pc),a0
		dc.w	$f010			; PMOVE (a0),TC
		dc.w	$4000
		lea.l	mmu_TT0(pc),a0
		dc.w	$f010,$0800		; PMOVE (a0),TT0
		lea.l	mmu_TT1(pc),a0
		dc.w	$f010,$0c00		; PMOVE (a0),TT1
		suba.l	a0,a0
		dc.w	$4e7b,$8801		; movec.l   a0,vbr


		*
		*   WARNING: MAGIC STARTS HERE.  TALK TO BRYCE BEFORE
		*   EVEN _THINKING_ ABOUT CHANGING THIS REBOOT CODE!
		*
		lea.l	MAGIC_JUMP,a0	;Point to JMP instruction in V1.3 ROM
		nop			;sync instruction stream
		bra	MagicResetCode
;-------------- MagicResetCode ---------CHANGED FOR A GOOD REASON-----------

		CNOP	0,4	;IMPORTANT!  Longword align!  Do not change!
MagicResetCode: RESET		;all RAM goes away now!
		jmp	(a0)    ;Rely on prefetch to execute this instruction
;-------------- MagicResetCode ---------CHANGED FOR A GOOD REASON-----------




;
; Kicksum.asm	09/01/87 Bryce Nesbitt
;
; Simple program to do an in place fixup of a Kickstart image.
; Requires Kickstart running out of RAM, or Writeable Control
; Store with a write enable button.
;

ROMSTART	EQU K_MEM
ROMSIZE 	EQU K_SIZE+K_SIZE
PATCHLOC	EQU END_MEM-24	    ;Which longword to mangle

kicksum:	lea.l	ROMSTART,a0
		clr.l	PATCHLOC

		move.l	#ROMSIZE/4-1,d3
		moveq	#0,d0
		move.w	#0,CCR	;Clear extend bit

chksum		add.l	(a0)+,d0
		bcc.s	cd_nocarry
		addq.l	#1,d0
cd_nocarry:	dbra	d3,chksum
		sub.l	#$10000,d3
		bpl.s	chksum	    ;TOTAL+N=$FFFFFFFF (no carry, ever)

		not.l	d0
		move.l	d0,PATCHLOC
		rts



;---------------------------------page tables-------------------------------

mmu_CRP 	dc.l	$000f0002,$00000000 ;(LevelA-StartCode+K_MEM+K_SIZE)
mmu_TC		dc.l	$80f08630	    ;32K pages-8 bits-6 bits-3 bits
;
;   Great trick here.  TT0 is a Transparent Translation for _read only_.
;   This means reads translate with no table search time, and our precious
;   22 ATC entries are fully available for better uses.
;
;   We still get protection becasue WRITES are translated with the
;   page tables that map the same area!
;
mmu_TT0 	dc.l	$04038207	    ;64MB at $04000000-$07FFFFFF READ
mmu_TT1 	dc.l	$00000000



	CNOP 0,16		;page tables must be double long aligned.
	dc.b 'mmu',0,'mmu',0
	dc.b 'mmu',0,'mmu',0

LevelA	dc.l 0
	dc.l $01000019	 ;  1, 01000000
	dc.l $02000019	 ;  2, 02000000
	dc.l $03000019	 ;  3, 03000000
	dc.l $04000019	 ;  4, 04000000
	dc.l $05000019	 ;  5, 05000000
	dc.l $06000019	 ;  6, 06000000
	dc.l 0
	dc.l $08000019	 ;  1, 01000000
	dc.l $09000019	 ;  2, 02000000
	dc.l $0a000019	 ;  3, 03000000
	dc.l $0b000019	 ;  4, 04000000
	dc.l $0c000019	 ;  5, 05000000
	dc.l $0d000019	 ;  6, 06000000
	dc.l $0e000019	 ;  6, 06000000
	dc.l $0f000019	 ;  6, 06000000


	CNOP 0,16
LevelB	dc.l $00000059	;  0, 00000000
	dc.l $00040059	;  1, 00040000
	dc.l $00080059	;  2, 00080000
	dc.l $000c0059	;  3, 000c0000
	dc.l $00100059	;  4, 00100000
	dc.l $00140059	;  5, 00140000
	dc.l $00180059	;  6, 00180000
	dc.l $001c0059	;  7, 001c0000
	dc.l $00200019	;  8, 00200000
	dc.l $00240019	;  9, 00240000
	dc.l $00280019	; 10, 00280000
	dc.l $002c0019	; 11, 002c0000
	dc.l $00300019	; 12, 00300000
	dc.l $00340019	; 13, 00340000
	dc.l $00380019	; 14, 00380000
	dc.l $003c0019	; 15, 003c0000
	dc.l $00400019	; 16, 00400000
	dc.l $00440019	; 17, 00440000
	dc.l $00480019	; 18, 00480000
	dc.l $004c0019	; 19, 004c0000
	dc.l $00500019	; 20, 00500000
	dc.l $00540019	; 21, 00540000
	dc.l $00580019	; 22, 00580000
	dc.l $005c0019	; 23, 005c0000
	dc.l $00600019	; 24, 00600000
	dc.l $00640019	; 25, 00640000
	dc.l $00680019	; 26, 00680000
	dc.l $006c0019	; 27, 006c0000
	dc.l $00700019	; 28, 00700000
	dc.l $00740019	; 29, 00740000
	dc.l $00780019	; 30, 00780000
	dc.l $007c0019	; 31, 007c0000
	dc.l $00800019	; 32, 00800000
	dc.l $00840019	; 33, 00840000
	dc.l $00880019	; 34, 00880000
	dc.l $008c0019	; 35, 008c0000
	dc.l $00900019	; 36, 00900000
	dc.l $00940019	; 37, 00940000
	dc.l $00980019	; 38, 00980000
	dc.l $009c0019	; 39, 009c0000
	dc.l $00a00019	; 40, 00a00000
	dc.l $00a40019	; 41, 00a40000
	dc.l $00a80019	; 42, 00a80000
	dc.l $00ac0019	; 43, 00ac0000
	dc.l $00b00019	; 44, 00b00000
	dc.l $00b40019	; 45, 00b40000
	dc.l $00b80019	; 46, 00b80000
	dc.l $00bc0059	; 47, 00bc0000
	dc.l $00c00019	; 48, 00c00000
	dc.l $00c40019	; 49, 00c40000
	dc.l $00c80019	; 50, 00c80000
	dc.l $00cc0019	; 51, 00cc0000
	dc.l $00d00019	; 52, 00d00000
	dc.l $00d40019	; 53, 00d40000
	dc.l $00d80019	; 54, 00d80000
	dc.l $00dc0059	; 55, 00dc0000
	dc.l $00e00059	; 56, 00e00000
	dc.l $00e40059	; 57, 00e40000
	dc.l $00e80059	; 58, 00e80000
	dc.l $00ec0059	; 59, 00ec0000
	dc.l $00f00019	; 60, 00f00000
	dc.l $00f40019	; 61, 00f40000
       ;dc.l $00f80019	; 62, 00f80000
       ;dc.l $00fc0019	; 63, 00fc0000
	dc.l K_MEM!$1d		; 62, 00f80000	WRITE PROTECTED
	dc.l K_MEM+K_SIZE!$1d	; 63, 00fc0000	WRITE PROTECTED


		CNOP 0,16
LevelBB dc.l $07000019	 ;  0, 00000000
	dc.l $07040019	 ;  1, 00040000
	dc.l $07080019	 ;  2, 00080000
	dc.l $070c0019	 ;  3, 000c0000
	dc.l $07100019	 ;  4, 00100000
	dc.l $07140019	 ;  5, 00140000
	dc.l $07180019	 ;  6, 00180000
	dc.l $071c0019	 ;  7, 001c0000
	dc.l $07200019	 ;  8, 00200000
	dc.l $07240019	 ;  9, 00240000
	dc.l $07280019	 ; 10, 00280000
	dc.l $072c0019	 ; 11, 002c0000
	dc.l $07300019	 ; 12, 00300000
	dc.l $07340019	 ; 13, 00340000
	dc.l $07380019	 ; 14, 00380000
	dc.l $073c0019	 ; 15, 003c0000
	dc.l $07400019	 ; 16, 00400000
	dc.l $07440019	 ; 17, 00440000
	dc.l $07480019	 ; 18, 00480000
	dc.l $074c0019	 ; 19, 004c0000
	dc.l $07500019	 ; 20, 00500000
	dc.l $07540019	 ; 21, 00540000
	dc.l $07580019	 ; 22, 00580000
	dc.l $075c0019	 ; 23, 005c0000
	dc.l $07600019	 ; 24, 00600000
	dc.l $07640019	 ; 25, 00640000
	dc.l $07680019	 ; 26, 00680000
	dc.l $076c0019	 ; 27, 006c0000
	dc.l $07700019	 ; 28, 00700000
	dc.l $07740019	 ; 29, 00740000
	dc.l $07780019	 ; 30, 00780000
	dc.l $077c0019	 ; 31, 007c0000
	dc.l $07800019	 ; 32, 00800000
	dc.l $07840019	 ; 33, 00840000
	dc.l $07880019	 ; 34, 00880000
	dc.l $078c0019	 ; 35, 008c0000
	dc.l $07900019	 ; 36, 00900000
	dc.l $07940019	 ; 37, 00940000
	dc.l $07980019	 ; 38, 00980000
	dc.l $079c0019	 ; 39, 009c0000
	dc.l $07a00019	 ; 40, 00a00000
	dc.l $07a40019	 ; 41, 00a40000
	dc.l $07a80019	 ; 42, 00a80000
	dc.l $07ac0019	 ; 43, 00ac0000
	dc.l $07b00019	 ; 44, 00b00000
	dc.l $07b40019	 ; 45, 00b40000
	dc.l $07b80019	 ; 46, 00b80000
	dc.l $07bc0019	 ; 47, 00bc0000
	dc.l $07c00019	 ; 48, 00c00000
	dc.l $07c40019	 ; 49, 00c40000
	dc.l $07c80019	 ; 50, 00c80000
	dc.l $07cc0019	 ; 51, 00cc0000
	dc.l $07d00019	 ; 52, 00d00000
	dc.l $07d40019	 ; 53, 00d40000
	dc.l $07d80019	 ; 54, 00d80000
	dc.l $07dc0019	 ; 55, 00dc0000
	dc.l $07e00019	 ; 56, 00e00000
	dc.l $07e40019	 ; 57, 00e40000
	dc.l $07e80019	 ; 58, 00e80000
	dc.l $07ec0019	 ; 59, 00ec0000
	dc.l $07f00019	 ; 60, 00f00000
	dc.l $07f40019	 ; 61, 00f40000
	dc.l $07f8001d	 ; 62, 00f80000 WRITE PROTECTED
	dc.l $07fc001d	 ; 63, 00fc0000 WRITE PROTECTED
	CNOP	0,4
mmu_EndMarker:


	dc.b	'What secret messages?',0
	CNOP	0,4


		IFNE	NUKEMEM
**
**	rom_nukemem - A tiny module that removes memory that is used
**	for better purposes :-).
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


		INCLUDE "exec/types.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/resident.i"
		INCLUDE "exec/memory.i"

		XREF	_LVOAllocAbs

nukememresidentTag:
		DC.W	RTC_MATCHWORD
		DC.L	nukememresidentTag
		DC.L	nukememEndMarker
		DC.B	RTF_COLDSTART
		DC.B	36		; version
		DC.B	0		; no type
		DC.B	104		; priority
		DC.L	IDString
		DC.L	IDString
		DC.L	remthemem

KICK_SIZE	EQU 512*1024
RAM_END 	EQU $08000000

;KLUDGE_SIZE	 EQU 128*1024
;KLUDGE_START	 EQU $07000000

remthemem:	move.l	#KICK_SIZE,d0
		move.l	#RAM_END-KICK_SIZE,a1
		jsr	_LVOAllocAbs(a6)
	       ;move.l	#KLUDGE_SIZE,d0
	       ;move.l	#KLUDGE_START,a1
	       ;jsr	_LVOAllocAbs(a6)
		rts

IDString:	dc.b	'nukemem',10,0
		CNOP	0,4
nukememEndMarker:
		ENDC




		IFNE	SCRAMTEST
*
*   Check for SCRAM, and turn it on if present.
*
*   RESULTS
*	0   - ok
*	1   - Old Ramsey
*	2   - NO SCRAM
*
*   :TODO: Check bank-by-bank
*
		INCLUDE "exec/types.i"
		INCLUDE "exec/ables.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/memory.i"
		INCLUDE "v:include/internal/a3000_hardware.i"
		INT_ABLES

		XDEF	_SCRAM


ERR_OK		EQU 0
ERR_OLDRAMSEY	EQU 1
ERR_NOSCRAM	EQU 2

SAVEREGS	REG  d2/d3/d4/d5
MEMORY		EQUR a0
TEMP		EQUR a1
RAMCONTROL	EQUR a2

RAMEND		EQU $08000000


*****************************************************************************

_SCRAM: 	movem.l d2/d3/d4/d5/a2/a5/a6,-(sp)
		move.l	4,a6

		lea.l	FlushI(pc),a5
		JSRLIB	Supervisor	;(a5)

		move.l	#SCRAMtestE-SCRAMtest,d0
		moveq	#MEMF_CHIP,d1
		JSRLIB	AllocMem
		tst.l	d0
		beq.s	scram_Bail
		move.l	d0,a1
		move.l	d0,a5
		lea.l	SCRAMtest(pc),a0
		moveq	#((SCRAMtestE-SCRAMtest)/2)-1,d0
scram_Copy:	move.w	(a0)+,(a1)+
		dbra	d0,scram_Copy
		JSRLIB	Supervisor	;(a5)
		move.l	d0,-(sp)
		move.l	a5,a1
		move.l	#SCRAMtestE-SCRAMtest,d0
		JSRLIB	FreeMem
		move.l	(sp)+,d0
scram_Bail:	movem.l (sp)+,d2/d3/d4/d5/a2/a5/a6
		rts


;
;  For future compatiblity, Ramsey registers must be accessed in supervisor
;  mode.
;
SCRAMtest:	lea.l	RAMEND-(512*1024)-(4*4),MEMORY
		lea.l	A3000_RamControl,RAMCONTROL
		DISABLE
		moveq	#ERR_OLDRAMSEY,d1
		cmp.b	#ANCIENT_RAMSEY,A3000_RamseyVersion-A3000_RamControl(RAMCONTROL)
		beq.s	scram_TooOld

		    ;
		    ;	Cause the MMU to update the used bit before
		    ;	we "turn off" the memory.
		    ;
		    movem.l (MEMORY),SAVEREGS
		    movem.l SAVEREGS,(MEMORY)
		    ;
		    ;	Set PAGE mode (which makes the ram unusable if
		    ;	SCRAMs are not installed).
		    ;	Ramsey registers don't take effect until the next
		    ;	refresh event.	We spin on the register until it
		    ;	reads back what was written.
		    ;
		    move.b  (RAMCONTROL),d0
		    or.b    #RAMCF_BURST|RAMCF_PAGE,d0	    ;OR
		    move.b  d0,(RAMCONTROL)
		    and.b   #RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE,d0
scram_Spin1:	    move.b  (RAMCONTROL),d1
		    and.b   #RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE,d1
		    cmp.b   d0,d1
		    bne.s   scram_Spin1
		    nop 		; sync pipeline for advanced CPUs

		    move.l  MEMORY,TEMP
		    move.l  TEMP,(TEMP)+
		    move.l  TEMP,(TEMP)+
		    move.l  TEMP,(TEMP)+
		    move.l  TEMP,(TEMP)+

		    dc.w    $4e7a,$0002 ; movec.l CACR,d0
		    bset    #11,d0	; Set "Clear D cache"
		    dc.w    $4e7b,$0002 ; movec.l d0,CACR

		    moveq   #~(RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE),d0    ;!AND
		    moveq   #ERR_NOSCRAM,d1
		    cmpa.l  -(TEMP),TEMP
		    bne.s   scram_nogood
		    cmpa.l  -(TEMP),TEMP
		    bne.s   scram_nogood
		    cmpa.l  -(TEMP),TEMP
		    bne.s   scram_nogood
		    cmpa.l  -(TEMP),TEMP
		    bne.s   scram_nogood
		    moveq   #~(RAMCF_WRAP!RAMCF_PAGE),d0    ;!AND
		    moveq   #ERR_OK,d1

		    ;
		    ;	Set final settings.
		    ;
scram_nogood:	    and.b   (RAMCONTROL),d0
		    move.b  d0,(RAMCONTROL)
		    and.b   #RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE,d0
scram_Spin2:	    move.b   (RAMCONTROL),d1
		    and.b   #RAMCF_WRAP!RAMCF_BURST!RAMCF_PAGE,d1
		    cmp.b   d0,d1
		    bne.s   scram_Spin2
		    nop 		; sync pipeline for advanced CPUs

scram_good:	    movem.l SAVEREGS,(MEMORY)


scram_TooOld:	ENABLE
		move.l	d1,d0
		rte
SCRAMtestE:



FlushI: 	dc.w	$4e7a,$0002 ; movec.l CACR,d0
		bset	#3,d0	    ; Set "Clear I cache"
		dc.w	$4e7b,$0002 ; movec.l d0,CACR
		rte


		VERSTAG
		CNOP	0,2

		ENDC




		END
