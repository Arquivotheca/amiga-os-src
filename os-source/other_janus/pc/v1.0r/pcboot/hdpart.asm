TITLE	FDISK  -  COPYRIGHT (C) 1986 Commodore Electronics Limited
PAGE	60,132
;******************************************************************************
;
;      HDPART : This routine looks for partitions on attached harddisks,
;		fills the amiga harddisk partition structure with found 
;		datas and adjusts pointer to first partition structure.
;
;
;    New code :  10-apr-86  TB
;    1.Update : 
;
;******************************************************************************

public		hdpart	       
 
;******************************************************************************

CSEG	SEGMENT PARA PUBLIC 'CODE'
	ASSUME	cs:cseg,ds:cseg,es:cseg,ss:nothing

include 	dskstruc.inc
include 	macros.inc
include 	vars_ext.inc
include 	mes.inc
include 	equ.inc

;******************************************************************************
;
; external utilities
;
extrn		pstrng:near
extrn		newline:near
extrn		outhxw:near

;******************************************************************************
;
;	Partition structures	    
;   
extrn		BootBlock:byte
extrn		HDPart1:byte
extrn		HDPart2:byte
extrn		HDPart3:byte
extrn		HDPart4:byte
extrn		PartMarker:word

;******************************************************************************
;
;	VARIABLES :  USED BY ADISK
;
extrn		Max_drive:byte
extrn		Max_head:byte
extrn		Max_cyl:word
extrn		Max_sec:byte 
extrn		CurDrive:byte
extrn		CurTable:word 
extrn		PartCount:byte
extrn		AM_HD:byte   

;******************************************************************************
;
;	PROGRAM STARTS HERE
;
;******************************************************************************
entry:
hdpart	proc	near 

	pushall       

	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	ax,offset Am_HD 	; prepare pointer
	mov	janus_part_base,ax
	mov	CurTable,ax	      
 
	CALL	DRIVE			; LOAD DRIVE PARAMETERS
	JNC	FD13			; ERROR HAPPENED ?
	mov	si,offset NoDriveMsg
	Jmp	PrintMes

FD13:
	CALL	L_PAT			; LOAD PARTITION TABLE
	JNC	FD11			; ERROR HAPPENED ?
	mov	si,offset LoadErrMsg
	Jmp	PrintMes    
 
FD11:
	CMP	PartMarker,0AA55H      
	JE	FD14
	mov	ax,0ee00h
	mov	si,offset NoPartMsg
	Jmp	PrintMes    
 
FD14:
	call	BuildTable

	inc	CurDrive
	mov	al,Max_drive
	cmp	al,CurDrive
	je	done
	jmp	fd13			; read partition table of second drive

PrintMes:
	if   (infolevel-20 ge 0)
	 xor  al,al
	 call outhxw
	 call pstrng
	 call newline
	endif

done:
	popall
	ret

hdpart	endp 


;******************************************************************************
;
;	DRIVE :  READ DRIVE PARAMETERS	    
;		 EXSPECTS CUR_DRIVE #
;		 SAVE ALL REGISTERS
;		 ERROR CODE IN AX AND SET CARRY IF ERROR
;
;******************************************************************************

DRIVE	PROC	NEAR			; READ DRIVE PARAMETERS

	PUSH	BX
	PUSH	CX
	PUSH	DX

	MOV	CX,3			; NUMBER OF RETRYS
DR1:
	PUSH	CX
	MOV	DL,CurDrive		; SET DRIVE NUMBER
	OR	DL,80H
	MOV	AH,08H			; SET MODE
	INT	DISK			; READ DISK PARAMETERS

	MOV	Max_drive,dl
	MOV	Max_head,dh
	MOV	BL,CL
	AND	BL,3FH			; MASK MAX_SEC
	MOV	Max_sec,bl
	MOV	BH,CL			; BUILD MAX_CYL
	MOV	CL,6
	SHR	BH,CL
	MOV	BL,CH
	INC	BX		  
	MOV	Max_cyl,bx		; SAVE IT
       
	POP	CX			; RESTORE COUNTER
	OR	AH,AH			; ERROR ? (INCLUDE RESET C-FLAG)
	JZ	DR2
	push	ax			; save error code in ax
	XOR	AX,AX
	INT	DISK			; RESET DISK
	pop	ax
	LOOP	DR1			; TRY IT AGAIN

	STC				; SET C-FLAG FOR ERROR
DR2:
	POP	DX
	POP	CX
	POP	BX
	RET

DRIVE	ENDP


;******************************************************************************
;
;	LOAD PARTITION TABLE INTO MEMORY
;
;	EXSPECTS CUR_DRIVE #
;	SAVES ALL REGISTERS
;	ERROR CODE IN AX AND SET CARRY IF ERROR
;
;******************************************************************************

L_PAT	PROC	NEAR

	PUSH	BX
	PUSH	CX
	PUSH	DX

	MOV	CX,10			; NUMBER OF RETRYS
L1:
	PUSH	CX
	MOV	DL,CurDrive		; SET DRIVE NUMBER
	OR	DL,80H

	MOV	BX,OFFSET BootBlock	; RESERVED MEMORY
	MOV	CX,0001H		; READ SECTOR 1 , CYLINDER 0
	SUB	DH,DH			; HEAD 0
	MOV	AL,1			; READ ONE SECTOR
	MOV	AH,02H			; SET MODE
	INT	DISK			; READ DISK SECTOR    

	POP	CX			; RESTORE COUNTER
	CLC
	OR	AH,AH			; ERROR ? (INCLUDE RESET C-FLAG)
	JZ	L2 

	push	ax			; save error code
	XOR	AX,AX
	INT	DISK			; RESET DISK AND
	pop	ax
	LOOP	L1			; TRY IT AGAIN

	STC				; SET C-FLAG FOR ERROR
L2: 
	POP	DX
	POP	CX
	POP	BX
	RET

L_PAT	ENDP



;******************************************************************************
;
;	SHOW :	PREPARES PARTITION PARAMETERS FOR PRINT OUT
;
;******************************************************************************

BuildTable	proc	near

	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DI
	PUSH	BP

	MOV	BX,OFFSET HDPart1
	mov	si,CurTable
	MOV	cx,MaxPart				; MAX. PARTITION NUMBER
SW2:
	PUSH	CX					; SAVE LOOP COUNTER
	CMP	BYTE PTR [BX+HDPart0.SysFlag],Amiga	; PART. TYPE ?
	JNE	SW3					; FOUND OTHER OS

	MOV	DH,BYTE PTR [BX+HDPart0.SecStart]	; BUILD BaseCylinder
	MOV	CL,6
	SHR	DH,CL
	MOV	DL,BYTE PTR [BX+HDPart0.CylStart]
	MOV	[si+dsk_partition.part_base_cyl],DX	; SAVE IT
 
	MOV	AH,BYTE PTR [BX+HDPart0.SecEnd] 	; BUILD END CYL.
	MOV	CL,6
	SHR	AH,CL
	MOV	AL,BYTE PTR [BX+HDPart0.CylEnd]
	MOV	[si+dsk_partition.part_end_cyl],ax	; SAVE IT
 
	xor	ah,ah					; build drive number
	mov	al,CurDrive
	or	al,80h
	mov	[si+dsk_partition.part_drvnum],ax
									    
	mov	al,Max_head				; build head number
	inc	al
	mov	[si+dsk_partition.part_numheads],ax

	mov	al,Max_sec				; build sector number
	mov	[si+dsk_partition.part_numsecs],ax

	cmp	si,offset Am_Hd 			; first table ?
	je	sw5					; yes !

	mov	ax,si					; build pointer to this
	sub	si,size dsk_partition			; partition structure  
	mov	[si+dsk_partition.part_next],ax 	; in the previous one
	xchg	ax,si

sw5:
	add	si,size dsk_partition			; si points to
							; next dsk struct.
SW3:
	add	BX,size HDPart0 			; BX points to 
							; next partition
	POP	CX					; RESTORE COUNTER
	LOOP	SW2
	mov	CurTable,si				; set pointer to
							; next partition
 
	POP	BP
	POP	DI
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	RET

BuildTable	endp


;******************************************************************************

CSEG	ENDS
	END ENTRY
