*******************************************************************************
*
*	$Id: clip.asm,v 39.2 92/05/15 07:43:05 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/ports.i'
    include 'graphics/clip.i'

	section graphics

    xdef    getcode,_getcode

getcode:
; int __asm getcode(register __a2 struct ClipRect *cr,register __d0 WORD x, register __d1 WORD y);
_getcode:
*   return clipping values
*   enter with d0,d1 = coordinate
*               a2 points to cliprect
*   returns     d0  code value

; octants for MegaCoder:
;  1 2 3 
;  4 5 6
;  7 8 9

	cmp.w	cr_MinX(a2),d0
	blt.s	case_147
	cmp.w	cr_MaxX(a2),d0
	bgt.s	case_369

	cmp.w	cr_MinY(a2),d1
	blt.s	case_2
case_58:
	cmp.w	cr_MaxY(a2),d1
	bgt.s	case_8
	moveq	#0,d0			; case 5
	rts
case_8:
	moveq	#ISGRTRY,d0		; case 8
	rts

case_2:	moveq	#ISLESSY,d0		; case 2
	rts

case_147:
	cmp.w	cr_MinY(a2),d1
	bge.s	case_47
	moveq	#ISLESSY+ISLESSX,d0	; case 1
	rts
case_47:
	cmp.w	cr_MaxY(a2),d1
	bgt.s	case_7
	moveq	#ISLESSX,d0		; case 4
	rts
case_7:	moveq	#ISLESSX+ISGRTRY,d0	; case 7
	rts			
case_369:
	cmp.w	cr_MinY(a2),d1
	bge.s	case_69
	moveq	#ISLESSY+ISGRTRX,d0	; case 3
	rts
case_69:
	cmp.w	cr_MaxY(a2),d1
	bgt.s	case_9
	moveq	#ISGRTRX,d0		; case 6
	rts
case_9:	moveq	#ISGRTRX+ISGRTRY,d0	; case 9
	rts


	end
