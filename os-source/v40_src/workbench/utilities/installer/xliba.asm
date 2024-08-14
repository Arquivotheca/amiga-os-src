*****************************************************
*		Parsec Soft Systems xlib functions   		*
*		Copyright 1989 Parsec Soft Systems			*
*****************************************************

		section code

		include		'exec/types.i'
		include 	'exec/memory.i'
		include		'exec/lists.i'
		include		'exec/ports.i'
		include 	'graphics/gfx.i'
		include		'libraries/dosextens.i'
		include		'libraries/filehandler.i'
		include		'lat_macros.i'

;---------------------------------------------
;		a_to_x(str) - convert hex string to long

		xdef	_a_to_x
_a_to_x:
		move.l	4(sp),a0	; get string address
		move.l	a0,a1		; save a copy

		cmp.b	#'-',(a0)	; is first letter a minus sign?
		beq.s	5$			; yes, so increment needed
		cmp.b	#'+',(a0)	; is first letter a plus sign?
		bne.s	1$			; no, so no increment 
5$		addq.w	#1,a0		; increment

1$		moveq	#0,d0		; d0 = current value

2$		moveq	#0,d1		; set up for a long value
		move.b	(a0)+,d1	; get a letter
		sub.b	#'0',d1		; is it < '0'?
		bmi.s	3$			; yes, end of number

		cmp.b	#9,d1		; is it > '9'?
		bls.s	6$			; no, so have a number 0 to 9

		sub.b	#'A'-'0',d1	; is it < 'A'
		bmi.s	3$			; yes, end of number

		cmp.b	#5,d1		; is it > 'F'
		bls.s	7$			; no, so digit between A and F

		sub.b	#'a'-'A',d1	; is it < 'a'
		bmi.s	3$			; yes, end of number

		cmp.b	#5,d1		; is it > 'F'
		bhi.s	3$			; yes, so end of number
							; else digit between a and f

7$		add.l	#10,d1
6$		lsl.l	#4,d0		; 16 * d0
		add.l	d1,d0		; 16 * d0 + d1
		bra.s	2$

3$		cmp.b	#'-',(a1)	; was it negative?
		bne.s	4$
		neg.l	d0			; negate the value
4$		rts

;---------------------------------------------
;		Printf, Etc.

		xref	_DOSBase

MAXLINE		equ		128
FHPOS		equ		0
COUNTPOS	equ		4
DOSPOS		equ		6
TOTAL		equ		10
BUFPOS		equ		12
LOCSPACE	equ		14

put_char				; d0 = character, a3 = data
		tst.b	d0				; do not write NUL byte
		beq.s	1$

		moveq	#0,d1
		move.w	COUNTPOS(a3),d1		; get count
		move.b	d0,BUFPOS(a3,d1.w)	; put byte in buffer
		addq.w	#1,TOTAL(a3)		; increment printed count
		addq.w	#1,d1				; increment count
		cmp.w	#MAXLINE,d1
		bmi.s	2$

		tst.l	(a3)				; check file handle
		beq.s	4$					; if file handle NULL, skip writing

		SaveM	d2/d3/a0/a6			; save regs
		move.l	d1,d3				; length
		lea		BUFPOS(a3),a0		; get buffer address
		move.l	a0,d2
		move.l	(a3),d1				; file handle
		move.l	DOSPOS(a3),a6		; cached DOSBase pointer (not small code)
		Call	Write
		RestoreM	d2/d3/a0/a6
		tst.l	d0
		bmi.s	3$					; if error, handle it below
4$		moveq	#0,d1
2$		move.w	d1,COUNTPOS(a3)
1$		rts

3$		clr.l	(a3)				; zero cached file handle
		bra.s	4$

		DECLAREA VPrintf			; VPrintf(string,args)

		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		beq.s	fp_bye

		move.l	4+4(sp),a0			; format
		move.l	8+4(sp),a1			; args
		bra.s	Fprintf

VPrintf
		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		beq.s	fp_bye

		DECLAREL PrintF				; PrintF(string,args)

		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		bne.s	ok
fp_bye		move.l	(sp)+,a6
		rts

ok		move.l	4+4(sp),a0			; format
		lea		8+4(sp),a1			; args
		bra.s	Fprintf

		DECLAREA	VFprintf		; VFprintf(file,string,args)

		move.l	12(sp),a1			; args
		bra.s	fp_merge

		DECLAREL	Fprintf			; Fprintf(file,string,args)

		lea		12(sp),a1			; args
fp_merge
		move.l	4(sp),d0			; file
		move.l	8(sp),a0			; format
VFprintf
		move.l	a6,-(sp)
Fprintf
		movem.l	a2/a3,-(sp)
		sub.w	#MAXLINE+LOCSPACE,sp		; get local storage
		move.l	sp,a3
		move.l	d0,(a3)				; handle
		lea		put_char,a2			; format function
		clr.w	COUNTPOS(a3)		; no characters in buffer
		clr.w	TOTAL(a3)			; no characters printed
		move.l	_DOSBase,DOSPOS(a3)	; DOSBase

		CallEx	RawDoFmt			; format it

		moveq	#0,d1
		move.w	COUNTPOS(a3),d1		; get count
		beq.s	1$

		tst.l	(a3)				; check file handle
		beq.s	2$

		SaveM	d2/d3/a0			; save regs
		move.l	d1,d3				; length
		lea		BUFPOS(a3),a0		; get buffer address
		move.l	a0,d2
		move.l	(a3),d1				; file handle
		CallDos	Write
		RestoreM	d2/d3/a0
		tst.l	d0
		bmi.s	2$					; if error, handle below

1$		move.w	TOTAL(a3),d0		; return count printed
		ext.l	d0
		add.w	#MAXLINE+LOCSPACE,sp
		movem.l	(sp)+,a2/a3
		move.l	(sp)+,a6
		rts

2$		neg.w	TOTAL(a3)			
		bra.s	1$

;----------------------------------------------------------------
;	clamp(min,val,max) - force a longword to be between two other longword values

			section code

			xdef	_clamp
_clamp		movem.l	4(sp),d0/d1		; min/value

; find the higher of d0 and d1
			cmp.l	d0,d1			; d1 - d0  : if d1 > d0 then d0 = d1
			blt.s	1$				; 
			move.l	d1,d0
; find the lower of max (d1) and d0
1$			move.l	12(sp),d1		; max
			cmp.l	d0,d1			; d1 - d0
			bgt.s	2$
			move.l	d1,d0

2$			rts

;----------------------------------------------------------------
;	CpyBstrCstr

			DECLAREA	CpyBstrCstr

			move.l	4(sp),a0
			move.l	8(sp),a1
CpyBstrCstr
			adda.l	a0,a0			; BPTR -> CPTR
			adda.l	a0,a0
			moveq	#0,d0
			move.b	(a0),d0			; length
			exg.l	a0,a1			; make order right for str_ncpy
			addq.w	#1,a1			; string starts one above length
			move.l	a0,d1
			bra.s	2$

1$			move.b	(a1)+,(a0)+

		ifnd	BIGSTRING
2$			dbeq	d0,1$
			beq.s	3$
		endc
		ifd		BIGSTRING
			beq.s	3$
			subq.w	#1,d0
			bpl.s	2$
		endc
			clr.b	(a0)

3$			move.l	d1,d0
			rts

;---------------------------------------------------------
; Disktype -- returns the id_DiskType of a DOS device

			xdef		DiskType,_DiskType

; register usage:
;	d2		result
;	a2		address of device node task
;	a3		saved stack pointer

; stackframe: A StandardPacket followed by an InfoData structure.

SP_OFFSET	equ			(sp_SIZEOF+3)&~3
TEMP_SIZE	equ			SP_OFFSET+id_SIZEOF+8

_DiskType:	move.l		4(sp),a0
DiskType					; (a0: dev_node)

			movem.l		d2/a2-a3/a6,-(sp)		; save registers
			moveq		#0,d2					; default result (NULL) in d2

			move.l		dn_Task(a0),d0			; get task pointer into d0
			beq.s		99$						; return NULL if not a filesystem
			move.l		d0,a2					; device task address

			move.l		sp,a3					; save stack pointer in a3

			moveq.l		#TEMP_SIZE/4,d0			; size of needed memory in longwords
1$			clr.l		-(sp)					; clear a longword down
			dbra		d0,1$					; and loop till memory buf created

			move.l		sp,d0					; now get stack pointer in d0
			addq.l		#3,d0					; round to next highest longword
			and.w		#~3,d0
			move.l		d0,sp					; and move to stack pointer

			move.l		sp,a0					; packet address in a0
			moveq.l		#ACTION_DISK_INFO,d0	; ACTION_DISK_INFO
			jsr			InitSimplePacket		; init the packet (xlib func)

			move.l		sp,d0					; get address of temp data
			add.l		#SP_OFFSET,d0			; add offset of infodata struct
			lsr.l		#2,d0					; turn into bptr (ptr >> 2)
			move.l		d0,sp_Pkt+dp_Arg1(sp)	; and store in packet

			move.l		a2,a0					; port to send packet to
			move.l		sp,a1					; address of packet to send
			jsr			SendPacket				; send the packet (xlib func)
			tst.l		sp_Pkt+dp_Res1(sp)		; if there was a non-zero result
			beq.s		98$						; then
			move.l		SP_OFFSET+id_DiskType(sp),d2 ; get the result in d2

98$			move.l		a3,sp					; restore stack pointer
99$			move.l		d2,d0					; put result in d0
			movem.l		(sp)+,d2/a2-a3/a6		; restore registers
			rts

;---------------------------------------------------------
; FileOnly

		DECLAREA	FileOnly

		move.l	4(sp),a0				; get path/file name
FileOnly
		move.l	a0,a1					; second copy

3$		tst.b	(a0)+					; advance to end of string
		bne.s	3$

2$		cmp.l	a1,a0					; if back at start
		beq.s	1$						;	only a filename, done

		move.b	-(a0),d1				; get a character
		cmp.b	#':',d1					; is it a colon?
		beq.s	4$						;	yes, so found file start
		cmp.b	#'/',d1					; is it a slash?
		bne.s	2$						;	no, so filename continues

4$		move.l	a0,d0					; put location of ':' or '/'
		addq.l	#1,d0					; then point to filename start
		rts

1$		move.l	a0,d0
		rts

;---------------------------------------------------------
; GetFileDate


LNEW			macro
						; NEW	rn,size <,t>
						; where rn is a register
						; and t is CHIP,CLEAR or PUBLIC

			subq.w		#4,sp
			movem.l		a0/a1/a6/d0/d1,-(sp)
;			move.l		AbsExecBase,a6

			moveq		#0,d1

			move.l		#\2,d0
			CallEx		AllocMem
			move.l		d0,20(sp)
			movem.l		(sp)+,a0/a1/a6/d0/d1
			move.l		(sp)+,d0
			move.l		d0,\1
			endm

LDELETE		macro		; DELETE rn,size  (rn cannot be d0/a1/a6)

			xref		_LVOFreeMem
			movem.l		a0/a1/a6/d0/d1,-(sp)
			move.l		\1,d0
			beq.s		\@DEL1
			move.l		4,a6
			move.l		d0,a1
			move.l		#\2,d0
			jsr			_LVOFreeMem(a6)
\@DEL1		movem.l		(sp)+,a0/a1/a6/d0/d1

			endm


		xref	_DOSBase

		DECLAREL 	GetFileDate			; GetDateStamp(filename,ds_buffer)

		SaveM		a2/a3/a6/d2/d4

		moveq		#0,d4				; result = FALSE

		LNEW		a3,fib_SIZEOF		; get storage for FileInfoBlock
		tst.l		d0
		beq.s		scat1				; out of memory error

		move.l		4+20(sp),d1			; get Lock on file
		moveq		#ACCESS_READ,d2
		CallDos		Lock
		move.l		d0,a2
		tst.l		d0
		beq.s		scat2				; no such object

		move.l		a2,d1				; examine the object
		move.l		a3,d2
		Call		Examine
		tst.l		d0
		beq.s		scat3				; examine failed

		move.l		8+20(sp),a0			; copy datestamp into buffer
		lea			fib_DateStamp(a3),a1
		move.l		(a1)+,(a0)+
		move.l		(a1)+,(a0)+
		move.l		(a1)+,(a0)+
		moveq		#1,d4				; result = TRUE

scat3
		move.l		a2,d1				; free lock
		Call		UnLock
scat2
		LDELETE		a3,fib_SIZEOF		; free memory
scat1
		move.l		d4,d0				; return result
		RestoreM	a2/a3/a6/d2/d4
		rts

;----------------------------------------------------------
;	GetHead(list) - get (but doesn't remove) head from list

		DECLAREL	GetHead

		move.l	4(sp),a0
GetHead							; assembly entry point -- a0 = list
		move.l	LH_HEAD(a0),a0
		tst.l	LN_SUCC(a0)
		beq.s	1$
		move.l	a0,d0
		rts

1$		moveq	#0,d0
		rts

;----------------------------------------------------------
;  Fgets (handle,buffer,count) - stops at NEWLINE, ignores RETURN

		xref	_DOSBase

NEWLINE		equ		$0a
RETURN		equ		$0d

		DECLAREL Fgets

		movem.l	a2-a3/d2-d5,-(sp)
		move.l	4+24(sp),a3
		move.l	8+24(sp),a2
		move.l	12+24(sp),d5
fg_merge
		moveq	#0,d4
more
		move.l	a3,d1
		move.l	a2,d2
		moveq	#1,d3
		move.l	a6,-(sp)
		CallDos	Read
		move.l	(sp)+,a6
		tst.l	d0
		bmi.s	fg_error		; error condition
		beq.s	eof				; read zero, end of file

		cmp.b	#NEWLINE,(a2)
		beq.s	done			; a NEWLINE means we're done
		cmp.b	#RETURN,(a2)
		beq.s	more			; ignore RETURN
		addq.w	#1,a2

		addq.l	#1,d4
		cmp.l	d5,d4
		blt.s	more
done
		clr.b	(a2)
		move.l	d4,d0
		movem.l	(sp)+,a2-a3/d2-d5
		rts

eof		tst.l	d4
		bne.s	done
		moveq	#-1,d4
		bra.s	done
fg_error
		moveq	#-2,d4
		bra.s	done

*	Gets(buffer,count)

		DECLAREL	Gets

		move.l	a6,-(sp)
		CallDos	Input
		move.l	(sp)+,a6
		beq.s	nyet

		movem.l	a2-a3/d2-d5,-(sp)
		move.l	d0,a3
		move.l	4+24(sp),a2
		move.l	8+24(sp),d5
		bra.s	fg_merge
nyet
		moveq	#-1,d0
		rts

;----------------------------------------------------------
;	GetTail(list) - get (but doesn't remove) tail from list

		DECLAREA	GetTail

		move.l	4(sp),a0				; get list
GetTail
		move.l	LH_TAILPRED(a0),d0		; get last node on list
		cmpa.l	d0,a0					; if not pointing to list,
		bne.s	1$						;	list not empty so exit
		moveq	#0,d0					; list empty, set to NULL
1$		rts

;----------------------------------------------------------
; InitPacket(action,packet,port) (d0,a0,a1)

			DECLAREA InitPacket

			move.l	4(sp),d0
			move.l	8(sp),a0
			move.l	12(sp),a1
InitPacket
			move.l	a2,-(sp)
			move.b	#NT_MESSAGE,sp_Msg+LN_TYPE(a0)
			lea		sp_Pkt(a0),a2
			move.l	a2,sp_Msg+LN_NAME(a0)
			move.l	a1,sp_Msg+MN_REPLYPORT(a0)
			move.l	a1,sp_Pkt+dp_Port(a0)
			lea		sp_Msg(a0),a2
			move.l	a2,sp_Pkt+dp_Link(a0)
			move.l	d0,sp_Pkt+dp_Type(a0)
			move.l	(sp)+,a2
			rts

;----------------------------------------------------------
; InitSimplePacket

			DECLAREA InitSimplePacket

			move.l	4(sp),d0
			move.l	8(sp),a0
InitSimplePacket
			movem.l	d0/a0/a6,-(sp)
			suba.l	a1,a1
			CallEx	FindTask
			move.l	d0,a1
			lea		pr_MsgPort(a1),a1
			movem.l	(sp)+,d0/a0/a6
			bsr		InitPacket
			rts

;--------------------------------------------------------------
; MakeBitMap, UnMakeBitMap - Allocate space for bit map rasters

		xref	_GfxBase

* allocate the rasters for this bit map *

		DECLAREL 	MakeBitMap			; MakeBitMap(bm,depth,width,height)

		SaveM		a2-a3/a6/d2/d4-d7			; 24 bytes

		move.l		4+32(sp),d0			; if bm = NULL, error
		beq.s		mb_bye

		move.l		d0,a2				; save bm pointer
		move.l		8+32(sp),d4			; get depth

		move.l		a2,a0				; bm pointer
		move.l		d4,d0				; depth
		move.l		12+32(sp),d6		; width
		move.l		d6,d1
		move.l		16+32(sp),d7		; height
		move.l		d7,d2

		CallGfx		InitBitMap			; init that bitmap

		move.l		a2,a0				; bm pointer
		addq.w		#bm_Planes,a0		; bm_Planes = 8
		move.l		d4,d1				; depth
		bra.s		1$
2$
		clr.l		(a0)+
1$
		dbra		d1,2$

		move.l		a2,a3				; set-up to allocate planes
		addq.w		#bm_Planes,a3
		bra.s		3$
4$
		move.l		d6,d0				; allocate a plane
		move.l		d7,d1
		CallGfx		AllocRaster
		tst.l		d0					; if zero, error
		beq.s		mb_error
		move.l		d0,(a3)+			; else, copy into bitmap
3$
		dbra		d4,4$				; decrement depth count

		moveq		#1,d0
		bra.s		mb_bye

mb_error
		move.l		a2,-(sp)
		jsr			_UnMakeBitMap
		addq.w		#4,sp
		moveq		#0,d0
mb_bye
		RestoreM	a2-a3/a6/d2/d4-d7
		rts

		DECLAREL	UnMakeBitMap			; UnMakeBitMap(bitmap)

		SaveM		a2/a6/d4-d6				; 16 bytes

		move.l		4+20(sp),a2				; bitmap
		moveq		#0,d4
		move.w		bm_BytesPerRow(a2),d4	; bytes per row
		lsl.l		#3,d4					; pixels per row
		moveq		#0,d5		
		move.w		bm_Rows(a2),d5			; rows
		moveq		#0,d6
		move.b		bm_Depth(a2),d6			; depth

		clr.b		bm_Depth(a2)			; set depth = 0
		addq.w		#bm_Planes,a2			; adjust pointer to planes start
		bra.s		11$
12$
		move.l		(a2),d0
		beq.s		11$
		move.l		d0,a0
		move.l		d4,d0
		move.l		d5,d1
		CallGfx		FreeRaster
		clr.l		(a2)+
11$
		dbra		d6,12$

		RestoreM	a2/a6/d4-d6
		rts
		
;----------------------------------------------------------
;	NextNode(node) - get next node in list

		section	nextnode.asm,code

		include 'macros.i'
		include 'exec/nodes.i'

		DECLAREL	NextNode

		move.l	4(sp),a0				; get node
		move.l	LN_SUCC(a0),a1			; get next node
		move.l	a1,d0					; keep as result
		tst.l	LN_SUCC(a1)				; is next node's succ NULL?
		bne.s	1$						;	no, so node valid
		moveq	#0,d0					; was last node
1$		rts

xlib		macro	
			xref	_LVO\1
			endm

			xlib	Lock
			xlib	Examine
			xlib	UnLock
			xlib	ParentDir

; wildcard file requester -
;	- has volume / device / parent / subdirectory names
;	- shows filenotes (can be edited?)
;	- shows sizes
;	- shows permissions
;	- shows date (aaag)
;	- has room for additional buttons on the bottom - whatever you want
;		them to be (example - LOAD, SAVE, CANCEL, DELETE, LOCK,
;			LOAD TEXT, etc.
;		buttons can be toggles, multiple choice or single-action
;	- has wildcards for selectively showing files.
;

***************************************************************************
*
* length = ExpandPath(lock,buffer,maxlength);
*	derives the entire pathname of the given lock, and places the result
* 	into buffer. returns length of string (also null terminate).
*
***************************************************************************

; register usage
;	d3 = max string length
;	d4 = recursion count (number of directories)
;	d5 = 'no parent' flag
;	d6 = length of string
;	d7 = directory lock
;	a2 = buffer to store path string into
;	a5 = file info block
;	a6 = lib base pointer

			xdef	_ExpandPath,ExpandPath
			xref	_DOSBase

ExpandPath:				; (a0:Lock, a1:buffer, d0:length)
			movem.l	D2-D7/A2/A3/A5/A6,-(sp) ; save regs
			move.l	a0,d7
			move.l	a1,a2
			move.l	d0,d6
			bra.s	do_dir_text
_ExpandPath:
			movem.l	D2-D7/A2/A3/A5/A6,-(sp) ; save regs
			move.l	4+40(sp),D7		; put dir lock in d0
			move.l	8+40(sp),A2		; put buffer to use in a2
			move.l	12+40(sp),D6	; put length of string in D6
do_dir_text:
			subq.w	#1,d6			; subtract 1 from d6
			bmi.s	dir_error		; if no length, then error
			move.l	sp,A3			; save stack pointer
			move.l	_DOSBase,A6		; get dosbase
			sub		#fib_SIZEOF,sp ; Reserve space on stack for fib
			move.l	sp,D0			; get stack pointer in d0
			btst	#1,D0			; if on a non-long-word-aligned boundary
			beq.s	1$
			subq	#2,sp			; then align to long word
1$			move.l	sp,A5			; Set A5 pointing to fib

			moveq	#0,D4			; Set artificial limit of 20
									; directories to avoid endless loop
			move.l	d4,D3			; maximum string length = 80, start at 0

			clr.b	(A2)			; Set string to "null"
			bsr.s	ascend			; generate path
			clr.b	(A2)			; null terminate.

			move.l	A3,sp			; restore stack pointer
			move.l	D3,d0			; length of string.
			movem.l	(sp)+,D2-D7/A2/A3/A5/A6 ; restore regs
			rts						; return

dir_error	move.l	A3,sp			; restore stack pointer
			moveq	#0,d0			; length = 0, error.
			movem.l	(sp)+,D2-D7/A2/A3/A5/A6 ; restore regs
			rts

***************************************************************************
*
* ascend
*		Recursive routine to create path text.
*		Call with D7 = a valid child lock
*
ascend:
			cmp		#20,D4
			bge.s	dir_error			; Probable disk error looped

			move.l	D7,D1
			jsr		_LVOParentDir(A6)
			move.l	D0,D5
			beq.s	no_parent			; D5 is a flag "no parent" if 0

			addq	#1,D4				; Bump tally of parents
			move.l	D7,-(sp)			; Save the current directory
			move.l	D5,D7				; Now go fetch this one
			move.l	D5,-(sp)
			bsr.s	ascend				; Go up another level

			subq	#1,D4				; Reduce parents level

			move.l	(sp)+,D1			; Restore parent dir into D1
			move.l	(sp)+,D7			; Restore current direcory
			jsr		_LVOUnLock(A6)		; Free the extra ParentDir Lock.

			moveq	#1,D5

* no_parent
*		Here get the text for this directory and append it.
; DAVE - check for ram disk and other oddities.
no_parent:
			move.l	A5,D2				; d2 = address of file info block
			move.l	D7,D1				; d1 = lock
			jsr	_LVOExamine(A6)			; examine it.
			tst.l	D0					; if error
			beq.s	dir_error			; Bad error, couldn't Examine()

			tst.w	d4
			beq.s	npl0
			tst.l	fib_DirEntryType(A5) ; type of entry...
			ble.s	dir_error			; Parent of a dir not a dir. Hmm...

npl0		lea		fib_FileName(A5),A0	; Append this dir string 
npl1:		move.b	(A0)+,D0			; while *str++
			beq.s	npl2
			bsr.s	do_putch			; put to string.
			bra.s	npl1

npl2:		move.b	#':',D0				; if dir level = 0
			tst	D5
			beq.s	do_putch			; if ZERO, it's the base

			move.b	#'/',D0
			tst	D4
			bne.s	do_putch			; if NZ, it's got a parent

npl3:		rts


***************************************************************************
*
* do_putch
*		Append the character in D0 to the string at A2
*		Max length = <d3> characters.
*		
do_putch:	cmp.w	D6,D3
			bge.s	1$
			addq.w	#1,D3
			move.b	D0,(A2)+
1$			rts

;----------------------------------------------------------
;	Puts, Fputs

*	Fputs(handle,string)

		DECLAREA Fputs

		move.l	4(sp),d0
		move.l	8(sp),a1
Fputs
		lea		form,a0
		movem.l	d0/a0/a1,-(sp)
		jsr		_Fprintf			; Fprintf(handle,"%s\n",string)
		lea		12(sp),sp
		rts

*	Puts(string)

		DECLAREA	Puts

		move.l	4(sp),a1
Puts
		lea		form,a0
		movem.l	a0/a1,-(sp)
		jsr		_PrintF				; Printf("%s\n",string)
		addq.w	#8,sp
		rts

form
		dc.b	'%s',$0a,0
		ds.w	0

;----------------------------------------------------------
;	SendPacket

			DECLAREA SendPacket

			move.l	4(sp),a0
			move.l	8(sp),a1
SendPacket
			movem.l	a2/a6,-(sp)
			move.l	a1,-(sp)
			move.l	MN_REPLYPORT(a1),a2
			move.l	4,a6
			Call	PutMsg
			move.l	(sp),a1
			move.l	a2,a0
			Call	WaitPort
			move.l	(sp)+,a1
			Call	Remove
			movem.l	(sp)+,a2/a6
			rts

;----------------------------------------------------------
;		Sprintf(buffer,format,args)

		section	sprintf.asm,code

		include 'macros.i'

sput_char
		move.b	d0,(a3)+
		rts

		DECLAREA VSprintf				; VSprintf(buffer,format,args)

		move.l	12(sp),a1				; get args start
		bra.s	sp_merge

		DECLAREL Sprintf				; Sprintf(buffer,format,args)

		lea		12(sp),a1				; get args start
sp_merge
		movem.l	a2/a3/a6,-(sp)
		move.l	16(sp),a3				; store buffer location
		move.l	20(sp),a0				; get format string address
		lea		sput_char,a2			; routine address

		CallEx	RawDoFmt				; format it

		movem.l	(sp)+,a2/a3/a6
		move.l	4(sp),d0				; buffer start is return value
		rts

VSprintf								; (d0,a0,a1)
		movem.l	d0/a2/a3/a6,-(sp)
		move.l	d0,a3					; store buffer location
		lea		sput_char,a2			; routine address

		CallEx	RawDoFmt				; format it

		movem.l	(sp)+,d0/a2/a3/a6
		rts

;----------------------------------------------------------
; TackOn(oldpath,newname,maxlen) tack newname onto oldpath for maxlen characters.

			section	tackon.asm,code

			xdef		_TackOn,TackOn

_TackOn:	move.l		4(sp),a0			; old path
			move.l		8(sp),a1			; new path
			move.l		12(sp),d0			; max length

TackOn:		move.l		a2,-(sp)			; save a reg
			tst.w		d0
			bmi.s		NoEnd				; length is negative, invalid
			beq.s		NoEnd				; length is zero, invalid
			move.l		a0,a2				; save string pointer
			tst.b		(a0)				; if any characters in old path
			beq.s		CopyName			; old path is null string. just copy

FindEnd:	move.b		(a0)+,d1			; get last character.
			subq.w		#1,d0				; subtract 1 from length.
			beq.s		NoEnd				; string has no terminating null.
			tst.b		(a0)				; test if this character is end of string.
			bne.s		FindEnd				; if not, check next

			move.l		a0,a2				; save end of string.

			cmp.b		#'/',d1				; if last character was a slash
			beq.s		CopyName			; or
			cmp.b		#':',d1				; a colon
			beq.s		CopyName			; and
			tst.b		(a1)				; if newname is non-null
			beq.s		CopyEnd				; then
			move.b		#'/',(a0)+			; add a slash
			subq.w		#1,d0				; subtract 1 from length.
			beq.s		NoRoom				; string has no terminating null.
			
CopyName:	move.b		(a1)+,(a0)+			; move bytes
			beq.s		CopyEnd
			sub.w		#1,d0				; subtract 1 from length
			bne.s		CopyName			; if room left continue, else abort.

NoRoom:		clr.b		(a2)				; terminate the old string.
NoEnd:		moveq		#0,d0				; set return = FAILURE
			move.l		(sp)+,a2			; restore reg
			rts								; and return

CopyEnd:	moveq		#-1,d0				; set return = SUCCESS
			move.l		(sp)+,a2			; restore reg
			rts								; and return

;----------------------------------------------------------------
;	zero(start,length) -- zero a range of memory quickly (max size 1 << 21)

			section	zero.asm,code

			xdef	_zero,zero
_zero
			move.l	4(sp),a0
			move.l	8(sp),d0
zero								; assembly entry (a0 = start, d0 = length)
			tst.l	d0				; allow size of zero and just return
			beq.s	6$

			move.l	a0,d1			; need address in data register
			btst.l	#0,d1			; is this an odd address?
			beq.s	8$				; yes, so no initial byte to clear
			clr.b	(a0)+
			subq.l	#1,d0			; one less byte
			bne.s	8$				; was it one byte? no, continue
6$			rts

8$			add.l	d0,a0			; add in size
			bclr	#0,d0			; clear a final odd byte?
			beq.s	9$
			clr.b	-(a0)

*	At this point, size is even and address is even
9$			bclr	#1,d0			; test and reset bit 1
			beq		1$
			clr.w	-(a0)			; if set then clear 1 word

1$			bclr	#2,d0			; test and reset bit 2
			beq		2$
			clr.l	-(a0)			; if set then clear 1 longword

2$			bclr	#3,d0			; test and reset bit 3
			beq		3$
			clr.l	-(a0)			; if set then move 2 longwords
			clr.l	-(a0)

3$			bclr	#4,d0			; test and reset bit 4
			beq		4$
			clr.l	-(a0)			; if set then move 4 longwords
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)

; at this point we should of copied just enough data to clear bits
;	0-4 of size. This means that any further copies can be modulo 32.

4$
			lsr.l	#5,d0
			bra.s	7$
5$
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
			clr.l	-(a0)
7$			dbra	d0,5$
			rts

			END
