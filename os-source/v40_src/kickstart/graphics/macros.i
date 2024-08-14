*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: macros.i,v 39.5 92/04/03 16:51:20 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	macros.i,v $
*   Revision 39.5  92/04/03  16:51:20  chrisg
*    added local variable macros.
*   
*   Revision 39.4  91/11/15  10:55:46  chrisg
*    added DEFINES for UNLIKELY_WORD and BYTE. used to tag interleaved
*   bitmaps.
*   
*   Revision 39.3  91/10/30  15:47:22  chrisg
*    added macro to get vectors via a viewport with null checking.
*   
*   Revision 39.2  91/10/28  11:51:37  chrisg
*    added dbugw macro
*   
*   Revision 39.1  91/09/23  12:22:43  chrisg
*   rcsdiff graphnt.h
*   added new dbg macro
*   
*   Revision 39.0  91/08/21  17:11:38  chrisg
*   Bumped
*   
*   Revision 37.1  91/07/09  16:31:28  chrisg
*     added amzing print macro
*   
*   Revision 37.0  91/01/07  15:14:46  spence
*   initial switchover from V36
*   
*   Revision 33.3  90/07/27  16:30:10  bart
*   *** empty log message ***
*   
*   Revision 33.2  90/03/28  09:40:19  bart
*   *** empty log message ***
*   
*   Revision 33.1  89/05/02  09:31:48  bart
*   copyright 1989
*   
*   Revision 33.0  86/05/17  14:57:42  bart
*   added to rcs for updating
*   
*
*******************************************************************************

*------ library dispatch macros --------------------------------------
 
CALLGFX     MACRO
        jsr _LVO\1(a6)
        ENDM

JUMPGFX     MACRO
        jmp _LVO\1(a6)
        ENDM

JSRFAR		MACRO
			xref	_LVO\1
			jsr		_LVO\1(a6)
			ENDM
 
EXTERN_FUNC  MACRO
        XREF    _LVO\1
        ENDM

CALLEXEC	MACRO
		xref	_LVO\1
		move.l	a6,-(sp)
		move.l	4,a6
		jsr		_LVO\1(a6)
		move.l	(sp)+,a6
			ENDM

GFX_RASTPORT_1_2 equ 1

dbug	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.l \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	8(a7),a7
	bra.s	b\@
a\@: dc.b	'%lx ',0,0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

dbugw	macro	r
	xref	_kprintf
	movem.l	d0/d1/a0/a1,-(a7)
	move.w \1,-(a7)
	pea	a\@(pc)
	jsr	_kprintf
	lea	6(a7),a7
	bra.s	b\@
a\@: dc.b	'%x ',0
b\@:
	movem.l	(a7)+,d0/d1/a0/a1
	endm

print	macro	string
	xref	KPutStr
	movem.l	a0/a1/d0/d1,-(a7)
	lea	mystring\@(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0/a1/d0/d1
	bra.s	skip\@
mystring\@:
	dc.b	\1,0
	cnop	0,2
skip\@:
	endm

vp_to_vector	macro	vp_ptr,vec_ofs,default_vector
;			\1     \2      \3
	move.l	vp_ColorMap(\1),\1
	cmp	#0,\1
	beq.s	use_default\@
	tst.b	cm_Type(\1)
	beq.s	use_default\@
	move.l	cm_vpe(\1),\1
	cmp	#0,\1
	beq.s	use_default\@
	move.l	vpe_VecTable(\1),\1
	cmp	#0,\1
	beq.s	use_default\@
	move.l	\2(\1),\1
	bra.s	end_macro\@
use_default\@:
	lea	\3,\1
end_macro\@:
	endm

UNLIKELY_BYTE	equ	$f5
UNLIKELY_WORD	equ	$805c

WORDVAR	macro	vname
TEMP_SIZE	set	TEMP_SIZE+(TEMP_SIZE&1)	; even up
\1_w		set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+2
	endm

LONGVAR	macro	vname
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
\1_l		set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+4
	endm

BVAR	macro	vname
\1_b	set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+1
	endm

ARRAYVAR	macro	vname,size
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
\1	set	TEMP_SIZE
TEMP_SIZE	set	TEMP_SIZE+\2
	endm

ALLOCLOCALS	macro
; align temp_size and sub from sp
TEMP_SIZE	set	TEMP_SIZE+((4-(TEMP_SIZE&3))&3)	; lword align
	lea	-TEMP_SIZE(a7),a7
	endm

