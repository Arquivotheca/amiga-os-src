	.XLIST

	PAGE	62,132
	TITLE JDISK.SYS MSDOS Device Driver
	.SALL
INCLUDE	..\INCLUDE\JDISK.I
INCLUDE ..\INCLUDE\JFX.I
INCLUDE ..\INCLUDE\JANUSINT.I
INCLUDE ..\INCLUDE\BPB.I

	.LIST

;
; JDISK.SYS - MSDOS Driver for a virtual PC Drive
;
; All new Code: Dieter Preiss 			Feb 87
;
;diag	= 1
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
	sub	ah,ah
	mov	si,ax
	shl	si,1		; word pointer in si

	mov	bl,es:[di].R_Command ; what shall we do ??
	cmp	bl,12		; that much commands we know
	ja	Commerr		; too high for us if ae
	sub	bh,bh		; expand
	shl	bx,1		; to word offset
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

ifdef diag
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

ifdef diag
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
ifdef diag
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
ifdef diag
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

ioct10:	mov	ax,CS:FCBTbl[bx]	; set up pointer
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
	jmp	short iodone

ioc1:
	cmp	[si].IL_Command,IL_UnLink
	je	ioc2
	mov	al,ILE_COMERR
	jmp	short iodone

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
	stosb   			; save length
	mov	ax,256*'D'+'J'          ; our name
	stosw
	mov	al,VERSION		; Version
	stosb
	mov	al,basedrv		; base drive
	stosb
	mov	al,ndrives		; Number of Units we know
	stosb
	mov	ax, offset FTbl		; and the File Handles,
	stosw
	mov	ax,cs			; and segment
	stosw 
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

	les	di,CS:[si].IL_JParam

	mov	es:[di].adr_Fnctn,ADR_FNCTN_CLOSE
	mov	ax,[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	DoJanus

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
	
	les	di,CS:[si].IL_JParam

	mov	es:[di].adr_Fnctn,ADR_FNCTN_READ

	mov	ax,CS:[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	jrw_setOffs

readloop:

	call	jrw_setCount
;
; Call the famous Janus service
;
	call	DoJanus
	mov	al,E_General
	jne	rddone				; Janus Err if ne

	cmp	es:[di].adr_Err,ADR_ERR_OK	; File err if ne
	mov	al,E_ReadFault
	jne	rddone

	cmp	bx,es:[di].adr_Count_l		; Count wrong if ne
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
	lds	si,CS:[si].IL_JBuffer	; there it is
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
DiskWrite:

	push    es
	push	di
	push	bp

	sub	bp,bp

	les	di,CS:[si].IL_JParam

	mov	es:[di].adr_Fnctn,ADR_FNCTN_WRITE
	mov	ax,cs:[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	jrw_setOffs

writeloop:
	call	jrw_setCount
;
; move that block to write buffer
;
	push	cx
	push	es
	push	di

	mov	cx,bx
	les	di,CS:[si].IL_JBuffer		; there it is

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
	call	DoJanus
	mov	al,E_General
	jne	wrdone				; Janus Err if ne

	cmp	es:[di].adr_Err,ADR_ERR_OK	; File err if ne
	mov	al,E_WriteFault
	jne	wrdone

	cmp	bx,es:[di].adr_Count_l		; Count wrong if ne
	mov	al,E_CRC
	jne	wrdone

	add	bp,cx				; add sects xferred
		     	
	or	cx,cx
	jnz	writeloop

	sub	ax,ax

wrdone: 
	mov	cx,bp
	pop	bp
	pop	di
	pop	es
	ret

;
; Calculate byte count
;
jrw_setCount:

	mov	bx,cx				; save a copy
	cmp	bx,16				; max count
	jb	jrw10
	mov	bx,16

jrw10:	sub	cx,bx				; sectors left
	xchg	bl,bh				; shl 8
	shl	bx,1				; times 512 total

	mov	es:[di].adr_Count_l,bx		; set up count
 	mov	es:[di].adr_Count_h,0

	ret
;
; Calculate byte offset of virtual disk file
;
jrw_setOffs:
	mov	es:[di].adr_Offset_l,0		; set up offet
	mov	es:[di].adr_Offset_h,0
	mov	byte ptr es:[di].adr_Offset_l+1,bl
	mov	byte ptr es:[di].adr_Offset_h,bh
	shl	es:[di].adr_Offset_l,1
	rcl	es:[di].adr_Offset_h,1
	ret
;
; Send a FILE IO Request to Janus
;	
DoJanus:
	push	ax
	push	bx

	mov	ah,J_CALL_AMIGA
	mov	al,J_JFT
	mov	bx,0ffffh
	int	JANUS
	cmp	al,J_FINISHED
	je	DJret
	cmp	al,J_PENDING
	jne	DJret
	mov	ah,J_WAIT_AMIGA
	mov	al,J_JFT
	int	JANUS
	cmp	al,J_FINISHED

DJret: 	pop	bx
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
	push	bx
	sub	bx,bx
	mov	ah,0eh
	int	10h
	pop	bx
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
endif			; diag

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