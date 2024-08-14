	.XLIST
	.SALL

	PAGE	63,132
	TITLE JLINK.COM  - MSDOS Virtual Disk Support

EXTRN	PChar:near	; print char in al on DOS stdout
EXTRN	AtoI:near	; get number in ax from string in ds:si

INCLUDE	D:\PCJANUS\INCLUDE\JDISK.I
INCLUDE D:\PCJANUS\INCLUDE\PSP.I
INCLUDE D:\PCJANUS\INCLUDE\JFX.I
INCLUDE D:\PCJANUS\INCLUDE\JANUSINT.I
INCLUDE D:\PCJANUS\INCLUDE\MACROS.I
INCLUDE D:\PCJANUS\INCLUDE\BPB.I

	.LIST
;
; All new Code: Dieter Preiss 			Feb 87
;
; diag	= 1
cr	= 13
lf	= 10
VERSION = 0
J_JFT	equ	14
;
; Here we go ...
;

CODE	GROUP CSEG,DSEG
CSEG    segment para public 'code'
	ASSUME CS:CODE,DS:CODE,ES:CODE,SS:CODE

PSP	PSPdef <>

;	ORG	100H

Main	PROC Near
	jmp	los
;
; local data
;
ID	Ident	<>	       	; structure

IL	ILink	<>		; Link structure

curdrv	db	3		; start drive = C:
vdrive  db      0		; virtual drive to use
retcode db	0		; return code
;
; This byte has one bit per switch:
;
switch	db	0		; switches, see below
;
; Flag bits assigned to switches
;
SW_UNLINK	equ	1 shl 0	; /u - unlink
SW_RO		equ	1 shl 1	; /r - read only
SW_NOMSG	equ	1 shl 2 ; /n - no console message
SW_CREATE	equ	1 shl 3 ; /c - create volume
SW_DIAG		equ	1 shl 7	; /d - diag
;
; This characters define the switches:
;
Aflags  db	'u'	 	; ulink		bit 0
	db	'r'		; read only	bit 1
	db	'n'		; no message    bit 2
	db	'c'		; create	bit 3
	db	0		; <unused>	bit 4
	db	0		; <unused>	bit 5
	db	0		; <unused>	bit 6
	db	'd'		; diag		bit 7
;
; Each Switch can have a decimal parameter /s:dddd.
; If given, it is placed here:
;
Numtbl	label	word		; one word per flag
NT_Unlink	dw	0	; /u
NT_RO		dw	0	; /r
NT_NoMsg	dw	0	; /n
NT_Create	dw	07fffh	; /c max size, 32MB
		dw	0	; <unused>
		dw	0	; <unused>
		dw	0	; <unused>
NT_Diag		dw	0	; /d diag level
;
; And here we really start:
;
los:
	mov	ah,30h
	int	21h
	cmp	al,3			; major DOS version
	jb	wrongdos

	call	FindDriver
	jnc	chkline

	printf	<"JDISK.SYS not installed\n">
	jmp	badret

WrongDOS:
	printf	<"MSDOS 3.xx or higher required\n">
	jmp	badret

NotAvail:
	printf  <"AMIGA Service not available\n">
	jmp	badret

JErr:   sub	ah,ah
	printf	<"AMIGA Service failed, Error %d\n">,ax
	jmp	badret

chkline:
	cld
	mov	ah,J_GET_BASE		; we want The Base.
	mov	al,J_JFT		; our special service #
	mov	di,-1			; just in case ...
	int	JANUS			; call the guru
	cmp	al,J_JFT		; a change ?
	jne	chkl10
;
; If JANUS is there, he shall have the MSDOS INT 0bh
; vector to point directly to his service now ...
; So let's try again.
;
; Best regards to MSDOS 3.2 !
;
	int	JANUS 			; Try again
	cmp	al,J_JFT		; a change ?
	je	NotAvail		; give up if no change

chkl10:
	cmp	al,J_OK			; something wrong ?
	jne	JErr			; yes if ne
	cmp	di,-1			; reasonable offset ?
	je	NotAvail		; not if still -1 .
;
; Call succeeded, set up pointers:
;
	mov	word ptr IL.IL_JParam+2,es	; Param mem Segment
	mov	word ptr IL.IL_JParam,di	; & offset
	mov	word ptr IL.IL_JBuffer+2,dx	; Buffer Segment
	mov	ax,es:[di].adr_BufferAddr  	; use that offset
	mov	word ptr IL.IL_JBuffer,ax	; as Buffer Offset
	mov	IL.IL_Name,0			; Zero Terminate
;
; Set up command line pointer & count
;
	push	cs
	pop	es

	mov	si,offset PSP.CmdLin	; point there

	call	nxtarg
	jnz	chkdrv
	jmp	pstat			; end if zf

;
; Check for a DOS drive Letter followed by colon
;
chkdrv:
	cmp	byte ptr 1[si],':'	; colon expected
	jne	chkd99			; no drive if ne	
	lodsb				; get letter
	or	al,20h			; to lower
	sub	al,'a'-1		; make it bin
	mov	vdrive,al		; and save it
	call	nxtarg
	je	sw99
;
; Is next parm a switch ?
;
chkd99: cmp	byte ptr [si],'/'	; switch ?
	je	chksw			; yes if eq

;
; Move Filename to IL.IL_Name and JBuffer
;
	push	es

	mov	bx,offset IL.IL_Name	; name buffer of IL
	les	di,IL.IL_JBuffer	; name buffer for JOPEN

	mov	ah,' '			; end of name character
	lodsb				; look at 1st char
	cmp	al,'"'			; quote ?
	jne	lp00			; normal processing
	mov	ah,al			; else wait for another quote	

lp01:	cmp byte ptr [si],cr		; end of line ?
	je	lp99

	lodsb			  	; else get that byte

lp00:	cmp	al,ah		   	; end of name ?
	je	lp99       		; yes if eq

lp02:	stosb		      		; else store
	mov	[bx],al	     		; next
	inc     bx	     		; update
	jmp	lp01	    		; loop if more

lp99:   sub	al,al	       		; terminate strings
	stosb
	mov	[bx],al	

	pop	es
;
; Check for switches
;
chksw:
sw00:	cmp	byte ptr [si],cr	; end of line ?
	je	sw99			; yes if eq
	lodsb				; else get that char

	cmp	al,'/'			; switch ?
	jne	sw00			; no if ne

	lodsb				; switch found
	sub	bx,bx			; table offset
	mov	ah,1			; flag

sw20:	cmp	al,Aflags[bx]		; found ?
	je	sw10			; yes if eq
	inc	bx			; next char from table
	shl	ah,1			; next flag
	jnc	sw20			; not found if cf
	jmp	IllSwitch		; error

sw10:	or	switch,ah		; set that flag
	cmp     byte ptr [si],':'	; followed by number ?
	jne	sw00			; check for more
	inc	si			; skip colon

	call	AtoI			; get int number
					; returned in ax	
	shl	bx,1			; point to arg table
	mov	Numtbl[bx],ax		; save that number
	jmp	sw00			; check for next

sw99:
;
; So, what shall we really do now ?
;
	test	switch,SW_DIAG
	jz	doit
	printf	<"JDISK.SYS at Segment %X \n">,<ID.I_CS>
	inc	NT_Diag			; at least 1 ...
	
doit:	cmp	vdrive,0		; vdrive ?
	jnz     do10			; no if z

	cmp	IL.IL_Name,0		; amiga name given ?
	jne	synerr			; name but no dos drive !
	jmp	pstat			; print status only

;
; Anything else requires an AMIGA filename given:
;
;
; create & link ?
;
do10:	test	switch,SW_CREATE	; shall we create ?
	jz	do20			; no if z

	cmp	IL.IL_Name,0		; must have a name
	je	synerr			; error if not

	jmp	create			; go create

do20:	test	switch,SW_UNLINK	; shall we do that ?
	jz	do30			; no if z, assume link

	mov	ah,0dh			; Disk Reset (Flush)
	int	21h			; before we unlink

	mov	IL.IL_Command,IL_UnLink ; else unlink
	jmp	SendAndBye

;
; Link old assumed
;
do30: 	cmp	IL.IL_Name,0		; must have a name
	je	synerr

	mov	ax,ADR_FNCTN_OPEN_OLD	; open old
	call	open			; AMIGA file
	jc	openerr			; error on carry
	mov	IL.IL_Command,IL_Link
	jmp	SendAndBye
;
; various errors
;
openerr:
	printf	<"Open File %s failed">,<offset IL.IL_Name>
	jmp	badret

synerr:	printf	<"Syntax Error\n">
	jmp	badret

IllSwitch:
	printf	<"Illegal Switch specified: /%c\n">,ax
	jmp	badret
;
; now tell the driver
;
SendAndBye:
	call	SendPacket
	jc	senderr
;
; error check
;
chkerr:	mov	al,IL.IL_Status
	cmp	al,ILE_OK
	je	pstat
;
; We had a problem:
;
	printf  <"JLINK Error: ">

	cmp	al,ILE_LINKED
	jne	cke01

	mov	byte ptr es:[di].adr_Fnctn,ADR_FNCTN_CLOSE
	call	DoJanus

	printf	<"Drive is already linked, unlink first.\n">
	jmp	short cke99

cke01:	cmp	al,ILE_CLOSE
	jne	cke02
	printf	<"Close Error on virtual volume.\n">
	jmp	short	cke99

cke02:
	cmp	al,ILE_NOT_LINKED
	jne	cke03
	printf	<"Nothing linked.\n">
	jmp	short cke99

cke03:	sub	ah,ah
	printf	<"%d\n">,ax				; general nature

cke99:	mov	retcode,1

pstat:  test	switch,SW_NOMSG			; wants a speech ?
	jnz	bye				; no if nz
	call	PInfo				; print link info
	jmp	bye

senderr:
	cmp	al,0fh				; Illegal drive ?
	jne	sende9
	mov	al,vdrive
	add	al,'a'-1
	printf	<"Illegal DOS Drive %c: specified\n">,ax
	jmp	short badret

sende9:
	printf	<"Could not send a packet, error code = %X \n">,ax

badret:	mov	retcode,1

bye:	mov	ah,4ch				; terminate
	mov	al,retcode			; return code
	int	21h				; tschuess

;
; create a virtual volume
;
create:
	mov	ax,ADR_FNCTN_OPEN_OLD		; check whether
	call	Open				; file exists
	jc	cre10				; does not if c
;
; close again
;
	mov	es:[di].adr_Fnctn,ADR_FNCTN_CLOSE
	call	DoJanus
	test	switch,SW_NOMSG			; keep quiet ?
	jnz	cre10
	printf	<"File %s exists, continue ? [Y/N]: ">,<offset IL.IL_Name>
	mov	ah,1				; get char from keyboard
	int	21h
	cmp	al,'y'				; check reply
	je	cre10
	cmp	al,'Y'
	je	cre10
	jmp	pstat				; forget it & bye
cre10:
	mov	AX,ADR_FNCTN_OPEN_NEW
	call	Open				; open new file
	jnc	cre05
	jmp	openerr

cre05:	
	mov	IL.IL_Command,IL_Link		; link it
	call	SendPacket
	jc	cre33

	mov	al,IL.IL_Status			; problems ?
	cmp	al,ILE_OK
	je	cre15				; no
	cmp	al,ILE_LINKED
	je	cre14

cre13:	jmp	chkerr				; yes
cre33:	jmp	senderr

cre14:
	test	switch,SW_UNLINK		; unlink switch ?
	jz	cre13				; no if z
	and	switch, NOT SW_UNLINK		; once only ..
	mov	IL.IL_Command,IL_Unlink		; else unlink
	call	SendPacket
	jc	cre33

	mov	al,IL.IL_Status
	cmp	al,ILE_OK			; success ?
	jne	cre13				; failed if ne
	jmp	cre05				; try again
;
; Now calculate BPB & Root Dir Parameters
;
cre15:
;
; Get current date
;
	mov	ah,2ah
	int	21h 				; ask DOS

	sub	cx,1980				; year
	mov	ah,cl
	mov	cl,4
	shl	dh,cl				; month
	mov	al,dh
	shl	ax,1
	or	al,dl				; day
	mov	RDDate,ax

	mov	ah,2ch				; get time
	int	21h				; from DOS

	sub	ax,ax
	xchg	al,cl				; minutes
	shr	cx,1
	shr	cx,1
	or	ax,cx				; hours
	mov	cl,5	
	shl	ax,cl
	shr	dh,1
	or	al,dh				; seconds/2
	mov	RDTime,ax
;
; With max 4096 clusters, ClusterSecs = NumSects / 4096
;	= NumSects shr 12
;	rounded to next power of two
;
	mov	ax,NT_Create			; what size shall it be ?
;
; Check for correct size range
;
	cmp	ax,07fffh			; valid size ?
	jbe	cre20				; yes if below
	mov	ax,07fffh			; else limit
cre20:	cmp	ax,160				; minimum ?
	jae	cre30
	mov	ax,160
cre30:	shl	ax,1				; convert to NumSects
	mov	BPBsect.NumSecs,ax 		; and set up
	sub	dx,dx				; long div
	mov	bx,4096
	div	bx
	neg	dx 				; round
	adc	al,0				; if remainder > 0

	mov	ah,al
	mov	al,1

cre18:	cmp	al,ah 				; this power of two
	jae	cre17				; done if be
	shl	al,1				; else next
	jmp	cre18

cre17:	mov	BPBsect.ClusterSecs,al		; Sectors Per Cluster
;
; Number of FAT Entries = Num Sects / Cluster Sects
;
	sub	ah,ah

	test	switch,SW_DIAG
	jz	deb15
	
	printf	<"Sectors per Cluster: %u\n">,ax
deb15:

	mov	bx,ax
	mov	ax,BPBsect.NumSecs
	sub	dx,dx
	div	bx				; AX now NFATEntries
	neg	dx				; round
	adc	ax,0

	test	switch,SW_DIAG
	jz	deb10
	printf	<"Number of Clusters: %u\n">,ax
deb10:
;
; Fat size :
;	if NFat > 4085 Fat Size = NFat * 2 (bytes)
;	         else  FatSize  = NFat * 1.5 (bytes)
;
	cmp	ax,4085
	ja	cre42

	mov	bx,ax				; ax = ax times 1.5
	shr	bx,1				; for 12 bit FAT
	adc	ax,bx				; rounded

	mov	FATS+3,0			; free if 12 bit
	jmp	short cre44

cre42:	shl	ax,1				; 16 bit FAT

;
; FATSecs = NFAT / 512
;
cre44:
	test	switch,SW_DIAG
	jz	deb11

	printf	<"Length of FAT: %u bytes\n">,ax
deb11:

	test	ax,511				; round up
	jz	cre46
	add	ax,512
cre46:	mov	cl,9
	shr	ax,cl
	mov	BPBsect.FATSecs,ax    		; Number of FAT Sectors

	test	switch,SW_DIAG
	jz	deb12

	printf	<"Sectors per FAT: %u\n">,ax
deb12:
;
; Recalculate total number of sectors we can address:
;
						; Sector per FAT times
	mul	BPBsect.SectorLength		; SectorLength
						; = # of FAT bytes
	cmp	FATS+3,0			; 12 bit FAT ?
	jne	cre50				; no if ne

	sub	dx,dx				; long div
	shl	ax,1
	mov	bx,3				; else times 2/3
	div	bx
	jmp	short cre51			; 12 bit fat done

cre50:	shr	ax,1				; 16 bit FAT: times 1/2
						; number of clusters
cre51:	sub	ax,2				; two clusters taken
	mov	bl,BPBsect.ClusterSecs		; times sectors/cluster
	sub	bh,bh
	mul	bx
	mov	BPBsect.NumSecs,ax		; number of sectors

cre80:
	test	switch,SW_DIAG
	jz	deb13

	printf	<"Free Sectors: %u\n">,ax
deb13:
;
; Now write boot block with BPB, FAT1, FAT2 and root dir:
;
	sub	dx,dx				; rel block 0
	mov	bx,offset BPBSect		; data
	call    writesec			; write boot block
	jc	cre99				; failed on carry

	call	writefat	 		; write fat 1
	jc	cre99

	call	writefat			; then fat 2
	jc	cre99

	sub	ax,ax
	mov	word ptr FATS,ax  		; clean up
	mov	word ptr FATS+2,ax		; FAT Data

	mov	bx,offset RootDir		; write root dir
	call	writesec
	jc	cre99

	mov	cx,15
	call	writefat1			; empty dirs
	jc	cre99

	jmp	pstat

cre99:	mov	bl,vdrive
	add	bl,'a'
	xchg	bx,ax
	printf	<"Write Error %X during CREATE on drive %c:\n">,bx,ax
	mov	retcode,1
	jmp	pstat

;
; local aux routines
;
Open	proc near
;
; Open an AMIGA file
;
; AX = ADR_FNCTN_OPEN_OLD or ADR_FNCTN_OPEN_NEW
;
	les	di,IL.IL_JParam			; get param pointer
	cmp	ID.I_Init,0			; 1st time ?
	jne	opn1				; no if ne
		    				; else init AMIGA driver
	push	ax				; save fnctn code
	mov	es:[di].adr_Fnctn,ADR_FNCTN_INIT; send init request
	call	DoJanus				; no return status
	pop	ax				; recall fnctn code

opn1:	mov	es:[di].adr_Fnctn,AX		; that function
	call	DoJanus				; do it
	mov	ax,es:[di].adr_Err		; error code
	cmp	ax,ADR_ERR_OK			; ok ?
	stc					; assume not
	jne	opn99				; failed if ne

opn2:	mov	ax,es:[di].adr_File		; get file handle

	mov	IL.IL_Handle,ax			; store
	mov	IL.IL_Flag,0			; clear
	mov	IL.IL_Status,0			; ok
	clc					; good return

	test	switch,SW_RO			; wants Read Only?
	jz	opn99				; no if z

	or	IL.IL_Flag,ILF_RO		; else mark it
opn99:	ret
Open	endp
;
; Find the driver and read an info packet
; Expects ID structure defined
; Returns CF if no driver found, else NC
; leaves curdrv updated
;
FindDriver	proc near
 	mov	curdrv,3	; start at drive c:
FD001: 				; loop start
	mov	ah,44h		; IOCTL request
	mov	al,4		; get IOCTL packet
	mov	bl,curdrv	; that drive
	mov	cx,size Ident	; that many bytes
	mov	dx,offset ID	; there
	int	21h
	jnc	FD003		; nc = no error  
				; else error condition
	cmp	ax,0fh	        ; illegal drive ?
	jne	FD002		; continue if not
	stc			; else not found
	ret

FD002:	inc	curdrv		; next
	jmp	FD001		; loop
;
; Validate response we got
;
FD003: 	cmp     al,ID.I_Length	; Length of response ok ?
	jne     FD002		; no if ne
	cmp     ID.I_Marker,JD	; Identifier OK ?
	jne     FD002		; no if ne
	clc			; else found
	ret

FindDriver	endp
;
; Send an ILink Packet to JDISK.SYS
;
SendPacket	proc near
	mov	ah,44h		; IOCTL
 	mov	al,5		; Write
	mov	bl,vdrive	; drive
	mov	cx,SIZE ILink	; count
	mov	dx,offset IL	; address
	int	21h		; fire
	ret
SendPacket	endp			
;
; Send a request to AMIGA
; wait for completion
;
; return EQ if OK
; else error code in ah
;
DoJanus	proc	near		     
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
	mov	ah,al			; make a copy
	cmp	al,J_FINISHED
DJret:	pop	bx
	ret
DoJanus	endp
;
; next arg
; expects ds:si pointing to command line
; if current cha is space, falls into skpispc
; 	else skips everything that is non-space
;	and falls into skipspc
;
nxtarg	proc	near
	cmp	byte ptr [si],' '	; space ?
	je	skps10			; yes if eq
	cmp	byte ptr [si],cr	; end of command line ?
	je	skps99			; yes if eq
	inc	si			; else check next
	jmp	nxtarg			
nxtarg	endp
;
; Skip Spaces
; expects ds:si pointing to space
; returns ZF=1 if only spaces found
; 	else non-zero and si adjusted
;
skipspc	proc near
	cmp	byte ptr [si],cr	; something left ?
	jz	skps99			; return zf=1 if not

skps00:	cmp	byte ptr [si],' '	; pointing to space ?
	jne	skps99			; return zf=0
skps10:	inc	si			; else next
	jmp	skipspc

skps99:	ret
skipspc	endp	
;
; Print info
;
PInfo	proc near

	push	es

	printf  <"\nVDrive  Status  Linked to\n">
	printf  <"--------------------------------------------------\n">

	mov	cl,ID.I_Units
	sub	ch,ch			; number of units reported

	mov	es,ID.I_CS		; point to file tables
	mov	si,ID.I_FTbl		; es:[si]
	add	ID.I_BaseDrv,'a'	; make ascii

hloop:	mov	bx,es:[si]		; Point to 1st table
	add	si,2			; update

	printf	<"%c:">,<word ptr ID.I_BaseDrv>
	inc	ID.I_BaseDrv

	or	bx,bx			; anything linked ?
	jz	nolink			; no if z

	mov     al,es:[bx].IL_Flag	; get flag
	lea	bx,[bx].IL_Name		; then point to name
	test	al,ILF_RO		; status R/W ?
	jnz	ronly			; print R/O if nz
	printf	<"\tR/W\t">,bx
	jmp	pfile
ronly:	printf	<"\tR/O\t">,bx
	jmp	short pfile

pf1:	call	PChar	
pfile:	mov	al,es:[bx]
	inc	bx
	or	al,al
	jnz	pf1

nolink:	printf	<"\n">
	loop	hloop			; next vdrive

	pop	es
	ret				; done

PInfo	endp


Creaux	proc near
;
; local aux routines
;
writefat:
	mov	bx,offset FATS		; write 1st FAT Sector
	call	writesec
	jc	wf99

	mov	cx,BPBsect.FATSecs
	dec	cx			; one done alredy
	jz	wf99

writefat1:
	mov	bx,offset CleanS	; and more clean sectors
	call	writesec		; write remaining secs
	jc	wf99
	loop	writefat1
wf99:	ret


writesec:
	push	bx
	push	cx	
	push	dx
	mov	al,vdrive			; write boot sector
	dec	al
	mov	cx,1
	int	26h

	pop	dx				; throw away old flags

	pop	dx
	pop	cx
	pop	bx

	pushf
	inc	dx
	popf

	ret


Creaux	endp


Main	ENDP
;
; Constants to use with "Create"
;

BPBsect	BPB	<>
	db	"This is a virtual PC JANUS Disk made by "
RootDir	db	"Jdisk V1.0 " 		; virtual Volume Name
					; must be 11 chars long
	db	8			; attribute, 8=VolumeLabel
	db	10 dup (0)	        ; DOS reserved
RDTime	dw	0			; time of creation
RDDate	dw	0			; date of creation
					; remaining bytes 0
FATS	db	0fbh,0ffh,0ffh,0ffh	; 16 bit fat assumed
CleanS	db	512 dup (0)		; sector of zeros
	
;
; TheEnd - the last offset within this program
;
DSEG    segment para public 'data'
	ASSUME CS:CODE
TheEnd  db	?		; End of resident part
DSEG    ends
CSEG	ENDS

	END	Main


