	.XLIST

	PAGE	62,132
	TITLE JDISK.SYS MSDOS Device Driver
	.SALL

;diagx	equ	1

;dbug	equ	1
;diag	equ	1
;serdeb equ 1

include		janus\services.inc
include		janus\dosserv.inc
INCLUDE		..\INCLUDE\JDISK.I
INCLUDE		..\INCLUDE\BPB.I

	.LIST

;
; JDISK.SYS - MSDOS Driver for a virtual PC Drive
;
; All new Code: Dieter Preiss 			Feb 87
;
cr	= 13
lf	= 10
VERSION = 1
;
; Macros
;
prints	macro	string
local	stradr

DSEG    segment para public 'data'
	ASSUME CS:CODE,DS:CODE
stradr	db	string,0
DSEG    ends

  	push	si
	mov	si,offset CODE:stradr
	call	PStrng
	pop	si
   	endm
	
printw	macro	hw
	push	ax
	mov	ax,hw
	call	OutHexW
	pop	ax
	endm

printb	macro	hw
	push	ax
	mov	al,hw
	call	OutHexB
	pop	ax
	endm
;
; Error Codes MSDOS understands:
;
E_WriteProtect	equ	0
E_WrongUnit	equ	1
E_DriveNotReady	equ	2
E_IllCommand	equ	3
E_CRC		equ	4
E_BadStruct	equ	5
E_Seek		equ	6
E_UnknownMedia	equ	7
E_SecNotFound	equ	8
E_OutOfPaper	equ	9
E_WriteFault	equ	10
E_ReadFault	equ	11
E_General	equ	12
;
; Bits of Status Word (Request.R_Status):
;
S_Error		equ	1 shl 15
S_Busy		equ	1 shl 9
S_Done		equ	1 shl 8
;
; Offsets of a Request Header
;
Request	STRUC
R_Length	DB	?
R_Unit		DB      ?
R_Command	DB      ?
R_Status 	DW      ?
R_Resrvd	DB      8 DUP (?)
Request	ENDS

InitReq	STRUC
		DB	13 DUP (?)
IR_Units	DB	?
IR_EndAddr	DD	?
IR_BPB		DD      ?
IR_DriveNum     DB      ?
InitReq	ENDS

MedReq	STRUC
		DB	13 DUP (?)
MR_MedDescr	DB	?
MR_Return	DB	?
MR_VolID	DD	?
MedReq	ENDS

BPBReq	STRUC
		DB	13 DUP (?)
BR_MedDescr     DB      ?
BR_TFA		DD	?
BR_BPBTbl	DD	?
BPBReq	ENDS

IOReq	STRUC
		DB	13 DUP (?)
IOR_MedDescr	DB	?
IOR_TFA		DD	?
IOR_Count	DW	?
IOR_StartSect	DW	?
IOR_VolID	DD	?
IOReq	ENDS

J_JFT	equ	16

	PAGE
;
; Here we go ...
;

CODE	GROUP CSEG,DSEG
CSEG    segment para public 'code'
	ASSUME CS:CODE,DS:NOTHING,ES:NOTHING,SS:NOTHING

;
; Device Header Block
;
Header	DW	-1,-1		; Filled later by MSDOS
	DW	6000H		; Block Device, Non-IBM Format,
				; IOCTL supported
	DW	Strategy	; Pointer to Strategy Entry
	DW	DevInt		; Pointer to Interrupt Entry
Units	DB	4		; Max of 4 Units
;
; Command Table
;
ComTbl	DW	Init		;  0, Init Device
	DW	MediaCheck	;  1
	DW	BuildBPB	;  2
	DW	IOCTL_I		;  3, IOCTL Input
	DW	Read		;  4
	DW	Commerr		;  5, Char Devs Only: Non-D Read
	DW	Commerr		;  6, Char Devs Only: Input Status
	DW	Commerr		;  7, Char Devs Only: Input Flush
	DW	Write		;  8
	DW	WriteVerify	;  9, Write & Verify
	DW	Commerr		; 10, Char Devs Only: Output Status
	DW	Commerr		; 11, Char Devs Only: Output Flush
	DW	IOCTL_O		; 12, IOCTL Output
;
; BPB
;
BBlk0	BPB	<>		; drop it here for drive 0
BBlk1	BPB	<>		; drop it here for drive 1
BBlk2	BPB	<>		; drop it here for drive 2
BBlk3	BPB	<>		; drop it here for drive 3

;
; BPB Pointers for each logical Unit (Used by Init)
;
BPBPtr	DW	offset BBlk0.SectorLength	; Unit 0
	DW	offset BBlk1.SectorLength	; Unit 1
	DW	offset BBlk2.SectorLength	; Unit 2
	DW	offset BBlk3.SectorLength	; Unit 3
;
; Media Check Status for each drive
;
MchkTbl DB	-1
	DB	-1
	DB	-1
	DB	-1
;
; File Handles for each drive
;
FTbl	DW	0
	DW	0
	DW	0
	DW	0

FCBTbl  DW	offset	CODE:IL0
	DW	offset	CODE:IL1
	DW	offset	CODE:IL2
	dw	offset	CODE:IL3
;
; Scratch Variables
;
ptrsav		dd	?	; Request Block Pointer during Strategy
stack  		dd      ?	; Stack Save during Devint
basedrv		db	?	; our first drive number
ndrives 	db      ?       ; number of drives we know
curdrv  	db      ?       ; current drive number
AInit		db	0	; 1 if AMIGA has been initialized

sd_ds		dd	0	;dosserv servicedata struct
ds_rq		dd	0	;dosserv req struct
ds_buf_ptr	dd	0	;ptr to dosserv buffer
ds_buf_len	dw	0	;length of dosserv buffer
secs_per_buf	dw	0	;# of sectors (512 bytes) that will fit
link_count	db	0	;# of linked files

		PAGE
;
; ------------------- Entries ----------------------------
;
; Save Request Block pointer
;
Strategy	PROC FAR
	mov	word ptr cs:ptrsav,bx
	mov	word ptr cs:ptrsav+2,es
	ret
Strategy	ENDP
;
; This is the general Main Entry:
;
main	proc far
Devint:	pushf
	cld				;our blks go forward only
        mov     word ptr cs:stack,sp
	mov     word ptr cs:stack+2,ss
	cli
	push    cs
	pop     ss
	mov	sp,offset CODE:TheEnd-2
	sti
 	push	ax		; save all regs
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es

	push    cs
	pop	ds
	assume  ds:CODE


;
; Dispatcher
;
	les	di,ptrsav	; get strategy pointer

	mov	al,es:[di].R_Unit ; current unit

	mov	curdrv,al	; save
ifdef diagx
	push	ax
	prints <"drive ">
	add     al,'a'
	add	al,cs:basedrv
	call    PChar
	pop	ax
endif
	sub	ah,ah
	mov	si,ax
	shl	si,1		; word pointer in si

	mov	bl,es:[di].R_Command ; what shall we do ??
ifdef diagx
	push	ax
	prints <", command ">
	mov	al,bl
	call	OutHexB
	prints  <cr,lf>
	pop	ax
endif
	cmp	bl,12		; that much commands we know
	ja	Commerr		; too high for us if ae
	sub	bh,bh		; expand
	shl	bx,1		; to word offset
ifdef dbug
prints <"(">
endif
	jmp     ComTbl[bx]	; bye bye
				; The called entry now finds:
				; DS ok
				; ES:DI pointing to RequestBlock
				; current drive in .AL
				; All regs on stack
;
;------------ returns -------------------------------------------
;
BadDrive:
	mov	al,E_WrongUnit
	jmp	short Error

Commerr:			; we are here since we got an
   	mov	al,E_IllCommand ; illegal command
;
; Error return, error code in .AL
;
Error:
	mov     ah,S_Error shr 8
	mov     es:[di].R_Status,ax; let DOS know.
				; fall into exit.
;
; General Exit (no error bit set)
;
Exit: 	or      es:[di].R_Status,S_Done
ifdef dbug
prints <")">
endif

	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	cli
	mov	sp,word ptr cs:stack
	mov     ss,word ptr cs:stack+2
	sti
	popf
	ret
;
; ------------ Now the particular services:
;
;
; Initialize this driver, called once only from DOS
;
Init:
	mov     al,es:[di].IR_DriveNum
	mov	basedrv,al

ifdef diagx
	prints <"JDISK.SYS Rev 0.10",cr,lf,"Drives = ">
	add     al,'a'
	call    PChar
	prints  <": to ">
	add	al,3
	call    PChar
	prints  <":",cr,lf,"*** D I A G  Version ***",cr,lf>
endif
	mov	es:[di].IR_Units,4
	mov	ndrives,4
	mov     word ptr es:[di].IR_EndAddr,offset CODE:TheEnd
	mov     word ptr es:[di].IR_EndAddr+2,cs
	mov     word ptr es:[di].IR_BPB+2,cs
	mov	word ptr es:[di].IR_BPB,offset BPBPtr
	jmp	Exit
;
; Write Request
;
WriteVerify:
Write:

ifdef diagx
prints <"Write called, Count = ">
printw <es:[di].IOR_Count>
prints <", StartSec = ">
printw <es:[di].IOR_StartSect>
prints <", Vdrive = ">
printb <es:[di].R_Unit>
prints <cr,lf>
endif
	mov	si,FTbl[si]
	test	[si].IL_Flag,ILF_RO	; read only ?
	jnz	Wprot			; yes if nz

	sub	cx,cx			; pessimistic
	xchg	cx,es:[di].IOR_Count	; Sector Count
	or	si,si			; valid pointer ?
	jz	wrerr			; no if z
	push	ds			; else
       	lds	dx,es:[di].IOR_TFA	; Transfer Address
	mov	bx,es:[di].IOR_StartSect; Start Sector
	call    DiskWrite		; Do the read
	pop	ds
	mov     es:[di].IOR_Count,cx	; update count
	or	al,al			; OK ?
	jnz	wrerr			; failed if cf

	jmp	Exit

wrerr:	
ifdef diagx
prints	<"Write Error: ">
printb	al
prints	<cr,lf>
endif	
	jmp	Error


Wprot:	mov	al,E_WriteProtect
	jmp	Error
;
; Read Request
;
Read:
	mov	si,FTbl[SI]		; to si
	sub	cx,cx			; pessimistic
	xchg	cx,es:[di].IOR_Count	; Sector Count
	or	si,si			; valid pointer ?
	jz	rderr			; no if z
	push	ds			; else
       	lds	dx,es:[di].IOR_TFA	; Transfer Address
	mov	bx,es:[di].IOR_StartSect; Start Sector
	call    DiskRead		; Do the read
	pop	ds
	mov     es:[di].IOR_Count,cx	; update count
	or	al,al			; OK ?
	jnz	rderr			; failed if cf

	jmp	Exit

rderr:	
ifdef diagx
prints	<"Read Error: ">
printb	al
prints	<cr,lf>
endif	
	jmp	Error
;
; Media Check Request
;
MediaCheck:

     	mov     bx,ax
	mov     al,MchkTbl[BX]
	mov	es:[di].MR_Return,al	; last status
	cmp	FTbl[SI],0		; something linked ?
	jz      mc1			; no if z
	mov	MchkTbl[bx],1		; no change next time
mc1:	jmp	Exit
;
; Build BPB Request
;
BuildBPB:

	cmp	FTbl[si],0 		; point to current File Handle
	jne	getbpb			; yes if ne

Bdnr:	mov	al,E_DriveNotReady
	jmp	Error			; else error
NotDOS:	mov	al,E_UnknownMedia	; not a DOS volume
	jmp	Error

getbpb:	mov	bx,0			; start sector
	mov	cx,1			; count
	push	si			; drive table offset
	lds	dx,es:[di].BR_TFA	; TFA ptr
	mov	si,cs:FTbl[si]		; FCB ptr
	call    DiskRead		; read 1st sector
	pop	bx			; was si
	or	al,al			; read ok ?
	jnz	Bdnr			; no if nz
	push	es			; save requset pointer
	push    di
	lds	si,es:[di].BR_TFA	; ds:si -> sector just read
	push	cs			; es = cs
	pop	es
	mov	di,cs:BPBPtr[bx]	; where is the bpb ?
	sub	di,0bh			; es:di -> cs:bpb

	mov	cx,4			; the first 8 bytes
	repe	cmpsw			; must fit
	jne	gtbp1			; NotDOS Volume if ne

	mov	cx, (size BPB)-8	; count, 8 bytes done alreay
	rep     movsb			; copy the rest

gtbp1:	pop	di			; restore request pointer
	pop	es

	jne	NotDOS

	mov	ax,cs:BPBPtr[bx]	; return a valid BPB Pointer
	mov	word ptr es:[di].BR_BPBTbl,ax
	mov     word ptr es:[di].BR_BPBTbl+2,cs
	jmp	Exit			; good exit
;
; IOCTL Request
;	
IOCTL_O:
	lds	si,es:[di].IOR_TFA

	mov	bx,ax
	mov	cs:MchkTbl[bx],-1		; medium changed

	shl	bx,1

	cmp	[si].IL_Command,IL_Link
	jne	ioc1
					; link request
	cmp	CS:Ftbl[bx],0		; file in use ?
	jz	ioct10			; no if z
	mov	al,ILE_LINKED		; else error
	jmp	short iodone

ioct10:	call	try_open_dosserv
	jz	ioct10a
	mov	al,ILE_NO_SERVICE
	jmp	short iodone

ioct10a:
	mov	ax,CS:FCBTbl[bx]	; set up pointer
	mov	CS:FTbl[bx],ax
	push	es
	push	di
	push	cs
	pop	es
	mov	di,CS:FTbl[bx]		; copy file info structure
	mov	cx,size ILink
	rep	movsb
	pop	di
	pop	es
	mov	al,ILE_OK
	mov	CS:AInit,1			; we had a link ...

	jmp	short iodone1

ioc1:
	cmp	[si].IL_Command,IL_UnLink
	je	ioc2
	mov	al,ILE_COMERR
	jmp	short iodone1

ioc2:	push	cs				; unlink request
	pop	ds

	mov	al,ILE_NOT_LINKED
	cmp	Ftbl[bx],0			; File linked ?
	je	iodone				; no if eq
	
	sub	si,si
	xchg	Ftbl[bx],si			; mark not linked
	call	DiskClose			; and close it

	mov	al,ILE_OK			; assume ok
	jz	iodone	 			; is it ok ?

	mov	al,ILE_CLOSE			; no if we are here
iodone:
	push	ax
	call	try_close_dosserv
	pop	ax

iodone1:	
	lds	si,es:[di].IOR_TFA
	mov	[si].IL_Status,al		; return a status
	jmp	Exit

; 
; We are asked to send an Info Message
;
IOCTL_I:				; Send Info Message
	mov	ax,SIZE Ident
	
	cmp     ax,es:[di].IOR_Count	; That much space provided ?
	ja	sizerr			; error if not
	mov	es:[di].IOR_Count,ax	; update bytes xferred
	push    es			; save request pointer
	push	di
	les	di,es:[di].IOR_TFA	; point to target buffer
	mov	ax,JD			; our name
	stosw
	mov	ax, offset FTbl		; and the File Handles,
	stosw
	mov	ax,cs			; and segment
	stosw 
	mov	al,SIZE Ident
	stosb   			; save length
	mov	al,VERSION		; Version
	stosb
	mov	al,basedrv		; base drive
	stosb
	mov	al,ndrives		; Number of Units we know
	stosb
	mov	al,AInit		; init status
	stosb
	pop	di			; and the request pointer
	pop	es    
	jmp	Exit			; done

sizerr: mov	al,E_BadStruct
	jmp	Error
main	endp

aux	proc    near
;
; --------- AUX Routines
; Note that they are local and NEAR
;
;
;
; DiskClose
;
; Close Virtual Disk File
;
; Expects FCB pointer in SI
;
DiskClose:
	push	es
	push	di

	call	lock_dosserv
	mov 	es:DOSServReq.dsr_Function[di],DSR_FUNC_CLOSE
ifdef dbug
prints <"C">
endif
	call	DoJanus
	call	unlock_dosserv
	xor	ax,ax		;indicate ok
	
	pop	di
	pop	es
	ret


;
; Read Sectors from disk
;
; Expects:	Pointer to File Block in CS:[si]
;		Sector Count in	CX
;		Buffer Address in DS:DX
;		Start Sector in BX
;
; Returns:	Actual Count in CX
;		Error Status in AX
;
DiskRead:

	push    es
	push	di
	push	bp
	sub	bp,bp
	
	call	lock_dosserv

	clc				;flag a read
	call	jrw_setOffs
	mov	al,E_Seek
	jc	rddone

	mov 	es:DOSServReq.dsr_Function[di],DSR_FUNC_READ

readloop:

	call	jrw_setCount
;
; Call the famous Janus service
;
ifdef dbug
prints <"R">
endif
	call	DoJanus
	mov	al,E_General
	jne	rddone				; Janus Err if ne

	cmp	es:DOSServReq.dsr_Err[di],0	; File err if ne
	mov	al,E_ReadFault
	jne	rddone

	cmp	es:DOSServReq.dsr_Arg3_l[di],bx	;got right amount?
	mov	al,E_CRC
	jne	rddone
;
; move that block
;
	add	bp,cx				; count down

	push	es				; preserve
	push	ds
	push	si
	push	cx
 
	push	ds				; there we want it
	pop	es

	xchg	di,dx
	lds	si,CS:ds_buf_ptr		; there it is
	mov	cx,bx  				; count
	shr	cx,1 				; always even
	rep	movsw				; more efficient
	xchg	di,dx				; pointer updated

	pop	cx				; restore
	pop	si
	pop	ds
	pop	es

	or	cx,cx				; sectors left ?
	jnz	readloop
	sub	ax,ax

rddone: mov	cx,bp				; actual count left

	call	unlock_dosserv
	or	ax,ax
	
	pop	bp				; orig. count
	pop	di
	pop	es
	ret
	
;
;
; Write Sectors to disk
;
; Expects:	Pointer to [FCB] in CS:[si]
;		Sector Count in	CX
;		Buffer Address in DS:DX
;		Start Sector in BX
;
; Returns:	Actual Count in CX
;		Error Status in AX
;
;		performs magic seek extension if necessary.  returns errors
;		if it can't.
DiskWrite:

	push    es
	push	di
	push	bp

	sub	bp,bp

	call	lock_dosserv

	stc				;flag a write
	call	jrw_setOffs
	mov	al,E_Seek
	jc	wrdone

	mov 	es:DOSServReq.dsr_Function[di],DSR_FUNC_WRITE

writeloop:
	call	jrw_setCount
;
; move that block to write buffer
;
	push	cx
	push	es
	push	di

	mov	cx,bx
	les	di,CS:ds_buf_ptr		; there it is

	xchg	si,dx				; source buffer
	shr	cx,1 				; always even
	rep	movsw				; more efficient
	xchg	si,dx				; pointer updated

	pop	di
	pop	es
	pop	cx				; restore  count

;
; Call the famous Janus service
;
ifdef dbug
prints <"W">
endif
	call	DoJanus
	mov	al,E_General
	jne	wrdone				; Janus Err if ne

	cmp	es:DOSServReq.dsr_Err[di],0	; File err if ne
	mov	al,E_WriteFault
	jne	wrdone

	cmp	es:DOSServReq.dsr_Arg3_l[di],bx	;got right amount?
	mov	al,E_CRC
	jne	wrdone

	add	bp,cx				; add sects xferred
		     	
	or	cx,cx
	jnz	writeloop

	sub	ax,ax

wrdone: 
	mov	cx,bp
	call	unlock_dosserv
	or	ax,ax
	pop	bp
	pop	di
	pop	es
	ret

;
; Calculate byte count
;
jrw_setCount:

	mov	bx,cx				; save a copy
	cmp	cs:secs_per_buf,bx		; max count
	jnc	jrw10
	mov	bx,cs:secs_per_buf

jrw10:	sub	cx,bx				; sectors left
	xchg	bl,bh				; shl 8
	shl	bx,1				; times 512 total

	mov	es:DOSServReq.dsr_Arg2_l[di],bx
	mov  	es:DOSServReq.dsr_Arg2_h[di],0

	ret

;
; seek to given offset (BX) in jlink file (DS:SI) using dosserv (ES:DI).
; if offset exceeds size of file, extend file up to that offset.
; if this is a read, extend the file by the number of blocks to be read.
; seek to the desired offset in the file.
;
; C set for write, C clear for read
; BX = block number to seek to
; CX = number of blocks about to be read/written (only used if a read)
; ES:DI = ptr to dosserv req block
; DS:SI = ptr to ILink structure
;
; all regs preserved except AX

jrw_setOffs:
	mov	ax,bx			;new len = read/write start
	jc	extendit		;C set means write.

	;it's a read.  we must make sure the read will not start at, start
	;past, or cross the EOF point.

	add	ax,cx			;new len = read start + read size

extendit:

	;0123456789
	;xxxxx		file is 5 blocks long
	;   ww		write at 3, len 2 - filesize stays the same
	;    ww		write at 4, len 2 - will extend naturally
	;     ww	write at 5, len 2 - will extend naturally
	;     *ww	write at 6, len 2 - force extend by 1, reseek
	;   rr		read at 3, len 2 doesn't cross EOF, filesize stays same
	;    r*		read at 4, len 2 - crosses EOF, force extend by 1, reseek
	;     **	read at 5, len 2 - at EOF, force extend by 2, reseek
	;     ***	read at 6, len 2 - past EOF, force extend by 3, reseek
	;
	; w = normal write
	; r = normal read
	; * = forced extension (blocks of zeros)

	;trying to do one of the following:
	;	write starting past the EOF point
	;	read starting past the EOF point
	;	read starting AT the EOF point
	;	read across the EOF point
	;trying to read/write vacuum.  we have to extend the
	;file up to this point (for reads or writes), and if it's a read, we
	;continue to extend by the number of blocks he wanted to read.

	mov 	es:DOSServReq.dsr_Function[di],DSR_FUNC_SEEK_EXTEND

	;newsize = endsec# * 256
	mov	byte ptr es:DOSServReq.dsr_Arg3_l+0[di],0
	mov 	byte ptr es:DOSServReq.dsr_Arg3_l+1[di],al
	mov 	byte ptr es:DOSServReq.dsr_Arg3_h+0[di],ah
	mov	byte ptr es:DOSServReq.dsr_Arg3_h+1[di],0

	;newsize = (endsec# * 256) * 2
	shl	es:DOSServReq.dsr_Arg3_l[di],1
	rcl 	es:DOSServReq.dsr_Arg3_h[di],1

	;offset = sector# * 256
	mov	byte ptr es:DOSServReq.dsr_Arg2_l+0[di],0
	mov 	byte ptr es:DOSServReq.dsr_Arg2_l+1[di],bl
	mov 	byte ptr es:DOSServReq.dsr_Arg2_h+0[di],bh
	mov	byte ptr es:DOSServReq.dsr_Arg2_h+1[di],0

	;offset = (sector# * 256) * 2
	shl	es:DOSServReq.dsr_Arg2_l[di],1
	rcl 	es:DOSServReq.dsr_Arg2_h[di],1

ifdef dbug
prints <"S">
endif
	call	DoJanus
	jnz	ret_err
	cmp	es:DOSServReq.dsr_Err[di],0	; File err if ne
	jz	ret_noerr

ret_err:	;return with C
	stc
	jmp	ret_either

ret_noerr:	;return with NC
	clc

ret_either:
	ret

;*************************************************************************
;*
;*	try_open_dosserv - if the number of linked files is 1, open
;*			   dosserv.
;*
;*************************************************************************
try_open_dosserv:
ifdef diagx
prints	<"open_dosserv: ">
prints	<cr,lf>
endif	
	cmp	link_count,0
	jz	open_it
	jmp	quit_open

open_it:	
	push	es
	push	si
	push	di
	push	dx
	push	cx
	push	ax
	
	push	ds
	mov	ax,DOSSERV_APPLICATION_ID shr 16
	mov	DS,ax
	mov	SI,DOSSERV_APPLICATION_ID and 0FFFFH
	mov	CX,DOSSERV_LOCAL_ID
	xor	ax,ax
	mov	ES,ax
	mov	DI,ax
	mov	AX,JFUNC_GETSERVICE*100h + GETS_WAIT+GETS_ALOAD_A
	int	JFUNC_JINT
	pop	ds
	cmp	al,JSERV_OK
	jz	got_dosserv

ifdef diagx
prints	<"open_dosserv: failed">
prints	<cr,lf>
endif	
	jmp	no_dosserv		;return NZ if failed

got_dosserv:
	;lock and unlock the servicedata structure (waits until fully initted)
	mov	AH,JFUNC_LOCKSERVICEDATA
;	mov	DI,word ptr cs:sd_ds
	int	JFUNC_JINT
	mov	AH,JFUNC_UNLOCKSERVICEDATA
;	mov	DI,word ptr cs:sd_ds
	int	JFUNC_JINT

	;es:di -> ServiceData - save in ds_sd
	mov	word ptr cs:sd_ds,di
	mov	word ptr cs:sd_ds+2,es

	;get DOSServReq struct ptr from ServiceData struct
	les	di,es:ServiceData.sd_PCMemPtr[di]
	mov	word ptr cs:ds_rq,di
	mov	word ptr cs:ds_rq+2,es

	;get buffer seg, offset, and length from DOSServReq struct
	mov	ax,es:DOSServReq.dsr_Buffer_Off[di]
	mov	word ptr cs:ds_buf_ptr,ax
	mov	ax,es:DOSServReq.dsr_Buffer_Seg[di]
	mov	word ptr cs:ds_buf_ptr+2,ax
	mov	ax,es:DOSServReq.dsr_Buffer_Size[di]
	mov	cs:ds_buf_len,ax

	;calculate how many sectors the buffer will hold
	xor	dx,dx
	mov	cx,512
	div	cx		;ax = dx:ax / cx, dx = remainder
	or	ah,ah		;result exceeds 255?
	jnz	toobig		;oops.
	cmp	al,128		;>127?
	jc	use_it
toobig:
	mov	al,127		;yes!  must fit in a byte!
use_it:
	xor	ah,ah
	mov	cs:secs_per_buf,ax
	
	inc	cs:link_count	;yay!
ifdef diagx
prints	<"open_dosserv: ok">
prints	<cr,lf>
endif	

	xor	ax,ax		;return Z if succeeded

no_dosserv:
	pop	ax
	pop	cx
	pop	dx
	pop	di
	pop	si
	pop	es

quit_open:
	ret

;*************************************************************************
;*
;*	try_close_dosserv - if the number of linked files is 0, close
;*			    dosserv.
;*
;*************************************************************************
try_close_dosserv:
	cmp	cs:link_count,0
	jz	quit_close
	dec	cs:link_count
	jnz	quit_close

	push	es
	push	di
	les	di,sd_ds
	mov	ah,JFUNC_RELEASESERVICE
	int	JFUNC_JINT
	pop	di
	pop	es
	
quit_close:
	ret

;*************************************************************************
;*
;*	lock_dosserv		make DOSServReq our very own
;*
;*************************************************************************
lock_dosserv:
	push	ax
	les	di,cs:ds_rq
	push	di
	lea	di,es:DOSServReq.dsr_Lock[di]
	mov	ah,JFUNC_LOCK
ifdef dbug
prints <"L">
endif
	int	JFUNC_JINT
ifdef dbug
prints <"l">
endif
	pop	di
	mov	ax,word ptr cs:IL_Handle[si]
	mov	es:DOSServReq.dsr_Arg1_l[di],ax
	mov	ax,word ptr cs:IL_Handle+2[si]
	mov	es:DOSServReq.dsr_Arg1_h[di],ax
	pop	ax
	ret

	
;*************************************************************************
;*
;*	unlock_dosserv		free DOSServReq
;*
;*************************************************************************
unlock_dosserv:
	push	ax
	push	di
	lea	di,es:DOSServReq.dsr_Lock[di]
	mov	ah,JFUNC_UNLOCK
ifdef dbug
prints <"U">
endif
	int	JFUNC_JINT
ifdef dbug
prints <"u">
endif
	pop	di
	pop	ax
	ret

;
; Send a FILE IO Request to Janus
;
; returns Z if ok, NZ if failed
;
DoJanus:
	push	ax
	push	es
	push	di

ifdef diagx
prints <"djpre: function = ">
printw <es:DOSServReq.dsr_Function[di]>
prints <", arg1 = ">
printw <es:DOSServReq.dsr_Arg1_h[di]>
printw <es:DOSServReq.dsr_Arg1_l[di]>
prints <", arg2 = ">
printw <es:DOSServReq.dsr_Arg2_h[di]>
printw <es:DOSServReq.dsr_Arg2_l[di]>
prints <", arg3 = ">
printw <es:DOSServReq.dsr_Arg3_h[di]>
printw <es:DOSServReq.dsr_Arg3_l[di]>
prints <", ds_rq = ">
printw <es>
prints <":">
printw <di>
endif

	mov	AH,JFUNC_CALLSERVICE	;call a service
	les	di,cs:sd_ds		;(dosserv) - es used later on
ifdef diagx
prints <", sd_ds = ">
printw <es>
prints <":">
printw <di>
endif
ifdef dbug
prints <"D">
endif
	int	JFUNC_JINT
ifdef dbug
prints <"d">
endif
ifdef diagx
prints <", retval = ">
printb <al>
prints <cr,lf>
endif

	cmp	al,JSERV_OK		;did it work?
;	jnz	do_error		;no.
	jz	dook
	jmp	do_error
dook:

	;wait for service to complete
waitloop:
	test	word ptr es:ServiceData.sd_Flags[di],SERVICE_PCWAIT
	sti
	jnz	waitloop
ifdef dbug
prints <"!">
endif

ifdef diagx
	pop	di
	pop	es	
prints <"djpst: function = ">
printw <es:DOSServReq.dsr_Function[di]>
prints <", arg1 = ">
printw <es:DOSServReq.dsr_Arg1_h[di]>
printw <es:DOSServReq.dsr_Arg1_l[di]>
prints <", arg2 = ">
printw <es:DOSServReq.dsr_Arg2_h[di]>
printw <es:DOSServReq.dsr_Arg2_l[di]>
prints <", arg3 = ">
printw <es:DOSServReq.dsr_Arg3_h[di]>
printw <es:DOSServReq.dsr_Arg3_l[di]>
prints <cr,lf>
	push	es
	push	di
endif

	xor	ax,ax

;	les	di,cs:ds_rq		;now check for an error in the call
;	mov	ax,es:DOSServReq.dsr_Err[di]
;	or	ax,ax
do_error:
	pop	di
	pop	es
	pop	ax

	ret



ifdef diag
; Print Hex word in .AX
; destroys .AX
;
OutHexW:
	xchg	al,ah
	call	OutHexB
	xchg	al,ah
;
; Print hex Byte in .AL
; destroys .AL
;
OutHexB:
	push	ax
	shr	al,1
	shr	al,1
	shr	al,1
	shr	al,1
	call    outh1
	pop     ax
	and     al,0fh
outh1:	add	al,'0'
	cmp	al,'9'
	jbe	PChar
	add	al,7
;
; Print Char in .AL
;
PChar:	push	ax
ifdef serdeb
	push	dx
	xor	dx,dx
	mov	ah,1
	int	14h
	pop	dx
else
	push	bx
	xor	bx,bx
	mov	ah,0eh
	int	10h
	pop	bx
endif
	pop	ax
	ret
;
; Print ASCIIZ String Pointed to by DS:SI
; Bumps SI
;
PStrng: push    ax
	push    ds
	push    cs
	pop	ds
	jmp     short pstr2
pstr1:	call	PChar
pstr2:	lodsb
	or	al,al
	jnz	pstr1
	pop	ds
	pop     ax
	ret
;
; Print Hex data pointed to by cs:si
; count in dx
; destroys cs:si and dx
;
Pblock:	push 	ax
	push 	di
	push 	cx
	mov	cl,es:[di]
	sub     ch,ch
pb001:	mov     al,' '
	call    PChar
	mov	al,es:[di]
	inc	di
	call    OutHexB
	loop    pb001
	pop	cx
	pop	di
	pop     ax
	ret
endif			; diagx

aux	endp

;
; Stack storage & TheEnd:
;
DSEG    segment para public 'data'

IL0	ILink	<>
IL1	ILink   <>
IL2	ILink	<>
IL3	ILink	<>

	db	256 dup (?)	; local stack space
	EVEN 			; in case we are on a 80268
TheEnd  db	?		; End of resident part, stack top
DSEG    ends
CSEG	ENDS

	END






	
