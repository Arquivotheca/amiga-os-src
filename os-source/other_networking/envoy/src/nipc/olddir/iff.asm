	INCLUDE	"exec/types.i"
	INCLUDE	"dos/dos.i"

	INCLUDE	"libraries/iffparse.i"
	INCLUDE	"nipcbase.i"

jsrlib	MACRO
	XREF	_LVO\1
	jsr	_LVO\1(a6)
	ENDM

ID_PREF	EQU	'PREF'

;---------------------------------------------------------------------
;
; BOOL OpenConfig(void)
;
; Opens	nipc.library's iff configuration file.
;
; Register Usage:
;
; d0-d1/a0-a1 Scratch
;
; d7 ULONG error
; d6 BOOL  cont
; a5 ContextNode *top
; a4 NICPBase

	XDEF	@OpenConfig

@OpenConfig:
	movem.l	d2-d7/a2-a6,-(sp)
	movea.l	a6,a4                           ; Save NIPCBase in a convenient place
	clr.l	nb_IFFParseBase(a4)             ; Clear pointers
	clr.l	iffHandle(a4)
	clr.l	iffStream(a4)
	movea.l	nb_DOSBase(a4),a6
	lea.l   ENVPrefName(pc),a0
	move.l	a0,d1
	move.l	#MODE_OLDFILE,d2
	jsrlib	Open                            ; Try opening ENV:envoy/nipc.prefs...
	move.l	d0,iffStream(a4)
	bne.b	1$
	lea.l	ENVARCPrefName(pc),a0
	move.l  a0,d1
	move.l	#MODE_OLDFILE,d2
	jsrlib	Open                            ; Try opening ENVARC:envoy/nipc.prefs...
	move.l	d0,iffStream(a4)
	beq	OC_Error

; Try opening iffparse.library

1$	movea.l	nb_SysBase(a4),a6
	lea.l	IFFParseName(pc),a1
	move.l	#37,d0
	jsrlib	OpenLibrary                     ; Try to open iffparse.library
	move.l	d0,nb_IFFParseBase(a4)
	beq.b	OC_Error
	movea.l	d0,a6

; Allocate an iffHandle

	jsrlib	AllocIFF
	move.l	d0,iffHandle(a4)
	beq.b	OC_Error

; Initialize the iffHandle for DOS streams

	movea.l	d0,a0
	jsrlib	InitIFFasDOS
	movea.l	iffHandle(a4),a0
	move.l	#IFFF_READ,d0
	jsrlib	OpenIFF
	tst.l	d0
	bne.b	OC_Error

	movea.l	iffHandle(a4),a3
	move.l	iffStream(a4),iff_Stream(a3)

	movea.l	a3,a0
	lea.l	NIPCProps(pc),a1
	moveq.l	#5,d0
	jsrlib	CollectionChunks
	tst.l	d0
	bne.b	OC_Error

ParseIt	movea.l	a3,a0
	move.l	#IFFPARSE_STEP,d0
	jsrlib	ParseIFF
	move.l	d0,d2
	beq.s	ParseIt

; See if we are	at the end of a	chunk

	cmp.l	#IFFERR_EOC,d0
	bne.s	1$

; See if we are	done with the FORM PREF

	movea.l	a3,a0
	jsrlib	CurrentChunk
	movea.l	d0,a2
	move.l	cn_Type(a2),d0
	cmp.l	#ID_PREF,d0
	bne.b	ParseIt
	move.l	cn_ID(a2),d0
	cmp.l	#ID_FORM,d0
	bne.b	ParseIt
1$	moveq.l	#1,d0

OC_Exit	movem.l	(sp)+,d2-d7/a2-a6
	rts

OC_Error:
	movem.l	(sp)+,d2-d7/a2-a6
	bsr.b	@CloseConfig
	moveq.l	#0,d0
	bra.b	OC_Exit


	XDEF	@CloseConfig

@CloseConfig:
	movem.l	d2-d7/a2-a6,-(sp)
	movea.l	a6,a4
	move.l	iffHandle(a4),d0
	beq.b	1$
	movea.l	d0,a3
	movea.l	d0,a0
	movea.l	nb_IFFParseBase(a4),a6
	jsrlib	CloseIFF
	movea.l	a3,a0
	jsrlib	FreeIFF
	movea.l	a6,a1
	movea.l	nb_SysBase(a4),a6
	jsrlib	CloseLibrary
	clr.l	nb_IFFParseBase(a4)
	clr.l	iffHandle(a4)

1$	move.l	iffStream(a4),d1
	beq.s	2$
	movea.l	nb_DOSBase(a4),a6
	jsrlib	Close
	clr.l	iffStream(a4)

2$	movem.l	(sp)+,d2-d7/a2-a6
	rts

	CNOP	0,4

NIPCProps	dc.b	'PREF','NDEV'
		dc.b	'PREF','NRRM'
		dc.b	'PREF','NLRM'
		dc.b	'PREF','NIRT'
		dc.b	'PREF','HOST'

ENVPrefName	dc.b	'ENV:envoy/nipc.prefs',0
ENVARCPrefName	dc.b	'ENVARC:envoy/nipc.prefs',0
IFFParseName	dc.b	'iffparse.library',0
