; Copyright (C) 1986,1987 by Manx Software Systems, Inc.
;
;:ts=8

;	libstart.a68 -- romtag for library

	include	'exec/types.i'
	include	'exec/resident.i'
	include	'exec/nodes.i'
	include	'exec/libraries.i'

MYVERSION	equ	0
MYPRI		equ	0

	cseg	; romtag must be in first hunk

	public	_myname
	public	_myid
	public	_myInitTab

	moveq	#0,d0		; don't let them run me
	rts
	public	_myRomTag
_myRomTag:
	dc.w	RTC_MATCHWORD
	dc.l	_myRomTag
	dc.l	endtag
	dc.b	RTF_AUTOINIT
	dc.b	MYVERSION
	dc.b	NT_LIBRARY
	dc.b	MYPRI
	dc.l	_myname
	dc.l	_myid
	dc.l	_myInitTab
endtag:
	dc.w	0		;to get things aligned to 4 byte boundary


;	For libraries:
;		library base in D0
;		segment list in A0
;		execbase in A6

;	Initial startup routine for Aztec C.

;	NOTE: code down to "start" must be placed at beginning of
;		all programs linked with Aztec Linker using small
;		code or small data.


	public	.begin
.begin
	public	_myInit
_myInit:
	movem.l	d0-d7/a0-a6,-(sp)	
	movem.l	d0/a0,-(sp)		;save library parameters
	bsr	_geta4			;get A4
	lea	__H1_end,a1
	lea	__H2_org,a0
	cmp.l	a1,a0			;check if BSS and DATA together
	bne	start			;no, don't have to clear
	move.w	#((__H2_end-__H2_org)/4)-1,d1
	bmi	start			;skip if no bss
	move.l	#0,d0
loop
	move.l	d0,(a1)+		;clear out memory
	dbra	d1,loop

start
	move.l	a6,_SysBase		;put where we can get it

	lea	dos_name,a1		;get name of dos library
	jsr	-408(a6)		;open the library any version
	move.l	d0,_DOSBase		;set it up
	bne	3$			;skip if okay

  	move.l  #$38007,d7		;AG_OpenLib | AO_DOSLib
	jsr     -108(a6)		;Alert
	bra	4$
3$
	jsr	__main			;call the startup stuff
4$
	add.w	#8,sp			;pop args
	movem.l	(sp)+,d0-d7/a0-a6
	rts				;and return

dos_name:
	dc.b	'dos.library',0

	public	_geta4
_geta4:
	far	data
	lea	__H1_org+32766,a4
	rts

	public	__main,__H0_org

	dseg

	public	_SysBase,_DOSBase
	public	__H1_org,__H1_end,__H2_org,__H2_end
	bss	_SysBase,4
	bss	_DOSBase,4

	end
