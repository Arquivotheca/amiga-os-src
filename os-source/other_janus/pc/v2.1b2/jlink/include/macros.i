printf	macro	string,p1,p2,p3,p4,p5,p6,p7,p8,p9
local	stradr,count

	ifndef  CPrintf
	EXTRN	CPrintf:near
	endif

	count = 2
DSEG    segment para public 'data'
	ASSUME CS:CODE,DS:CODE
stradr	db	string,0

DSEG	ENDS
	push	ax

	irp	pa,<<p9>,<p8>,<p7>,<p6>,<p5>,<p4>,<p3>,<p2>,<p1>>
	ifnb	<pa>
	mov	ax,pa
	push	ax
	count	= count +2
	endif
	endm
	
	mov	ax,offset CODE:stradr
	push	ax
	call	CPrintf
	add	sp,count
	pop	ax
	endm
		
prints	macro	string
local	stradr

	ifndef	PStrng
	EXTRN	PStrng:near
	endif

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

	ifndef OutHexW
	EXTRN	OutHexW:near
	endif

	push	ax
	mov	ax,hw
	call	OutHexW
	pop	ax
	endm

printb	macro	hw
	ifndef OutHexB
	EXTRN	OutHexB:near
	endif

	push	ax
	mov	al,hw
	call	OutHexB
	pop	ax
	endm
	
	
