*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: blisslicer.asm,v 39.2 93/04/01 15:48:12 spence Exp $
*
*	$Locker:  $
*
*	$Log:	blisslicer.asm,v $
*   Revision 39.2  93/04/01  15:48:12  spence
*   Removed Bart's interleaved BitMap processing. We really want
*   BytesPerRow after all, not bitmap width.
*   
*   Revision 39.1  92/02/21  19:51:03  bart
*   process interleaved bitmaps for V39
*   
*   Revision 39.0  91/08/21  17:33:41  chrisg
*   Bumped
*   
*   Revision 37.0  91/01/07  15:28:14  spence
*   initial switchover from V36
*   
*   Revision 33.2  90/07/27  16:36:20  bart
*   id
*   
*   Revision 33.1  90/03/28  09:26:27  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:22:25  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'graphics/gfx.i'
	INCLUDE 'graphics/rastport.i'
	INCLUDE 'graphics/clip.i'
	INCLUDE	'gelsinternal.i'

STACK_OFF	EQU	8*4
ARG0_OFF	EQU	2*4
ARG1_OFF	EQU	3*4
ARG2_OFF	EQU	4*4
ARG3_OFF	EQU	5*4
ARG4_OFF	EQU	6*4
ARG5_OFF	EQU	7*4
ARG6_OFF	EQU	8*4

	CODE

	XREF	_blisser
	XREF	_resetRects

	XDEF	_blisslicer

_blisslicer:

	LINK	A5,#-(gR_SIZEOF+gR_SIZEOF)	* worksrc, workdest

*	layered rastport ?

	MOVE.L	ARG2_OFF(A5),A0			* RPort in A0
	MOVE.L	rp_Layer(A0),D0			* CW in D0
	BNE	is_layer			* CW != 0 ?

no_layer:

	LEA	4+ARG6_OFF(A5),A1		* more effective pushes

	MOVE.L	D0,-(SP)			* push args
	MOVE.L	-(A1),-(SP)			* beamsync
	MOVE.L	-(A1),-(SP)			* type
	MOVE.L	-(A1),-(SP)			* mint
	OR.B	rp_Mask(A0),D0			* was MOVE.B rp_Mask(A2),D0 
	MOVE.L	D0,-(SP)			* RPort->Mask
	MOVE.L	-(A1),-(SP)			* s
	MOVE.L	rp_BitMap(A0),-(SP)		* RPort->BitMap
	MOVE.L	ARG1_OFF(A5),-(SP)		* dest
	MOVE.L	ARG0_OFF(A5),-(SP)		* src
	MOVE.L	rp_GelsInfo(A0),-(SP)		* RPort->GelsInfo

	JSR	_blisser			* bliss it
	
	UNLK	A5				* pop args
	RTS

is_layer:

	MOVEM.L	D2-D6/A2-A4,-(SP)		* save registers on stack

	MOVE.L	A0,A2				* RPort in A2
	CLR.L	D2				
	OR.B	rp_Mask(A2),D2			* RPort->Mask in D2
	MOVE.L	rp_GelsInfo(A2),A3		* GI in A3

	MOVE.B	rp_Flags+1(A2),gi_Flags(A3)	* GI_Flags =
	ANDI.B	#RPF_DBUFFER,gi_Flags(A3)	* RPort->Flags & DBUFFER

	MOVE.L	D0,A4				* CW (== RPort->Layer) in A4

	MOVE.L	ARG0_OFF(A5),A0			* src
	MOVE.L	ARG1_OFF(A5),A1			* dest

	MOVE.W  lr_MinX(A4),D0
	ADD.W	D0,gR_rX(A1)			* dest->rX += CW->bounds.MinX
	MOVE.W  lr_Scroll_X(A4),D0
	SUB.W	D0,gR_rX(A1)			* dest->rX -= CW->Scroll_X
	MOVE.W  lr_MinY(A4),D0
	ADD.W	D0,gR_rY(A1)			* dest->rY += CW->bounds.MinY
	MOVE.W  lr_Scroll_Y(A4),D0
	SUB.W	D0,gR_rY(A1)			* dest->rY -= CW->Scroll_Y

	MOVE.L	lr_ClipRect(A4),A4		* CR in A4

*	bliss all regular cliprects

	MOVE.W	gR_rX(A1),D3			* dest->rX in D3

	MOVE.W	D3,D4				* copy dest->rX
	ADD.W	gR_rW(A0),D4			* dest->rX + src->rW
	SUBQ.W	#1,D4				* dest->rX + src->rW - 1 in D4

	MOVE.W	gR_rY(A1),D5			* dest->rY in D5

	MOVE.W	D5,D6				* copy dest->rY
	ADD.W	gR_rH(A0),D6			* dest->rY + src->rH
	SUBQ.W	#1,D6				* dest->rY + src->rH -1 in D6

start_clip_loop:

	MOVE.L	A4,D0				* another cliprect to process ?
	BEQ	end_clip_loop

*	test ClipRect intersection with current bliss bounds

	CMP.W	cr_MaxX(A4),D3			* dest->rX : cr->MaxX 
	BGT	no_intersect			* dest->rX > cr->MaxX

	CMP.W	cr_MinX(A4),D4			* dest->rX+src->rW-1 : cr->MinX
	BLT	no_intersect			* dest->rX+src->rW-1 < cr->MinX

	CMP.W	cr_MaxY(A4),D5			* dest->rY : cr->MaxY 
	BGT	no_intersect			* dest->rY > cr->MaxY

	CMP.W	cr_MinY(A4),D6			* dest->rY+src->rH-1 : cr->MinY
	BLT	no_intersect			* dest->rY+src->rH-1 < cr->MinY

*	ok, these two intersect, so reset the gelRects and bliss it

*	MOVE.L	gR_rX(A0),gR_rX+STACK_OFF(SP)		* worksrc.rX,worksrc.rY
*	MOVE.L	gR_rW(A0),gR_rW+STACK_OFF(SP)		* worksrc.rW,worksrc.rH
*	MOVE.W	gR_rRealWW(A0),gR_rRealWW+STACK_OFF(SP)	* worksrc.rRealWW
*	MOVE.L	gR_rAddr(A0),gR_rAddr+STACK_OFF(SP)	* worksrc.rAddr

*	MOVE.L	gR_rX(A1),gR_rX+gR_SIZEOF+STACK_OFF(SP)	* workdst.rX,workdst.rY
*	MOVE.L	gR_rW(A1),gR_rW+gR_SIZEOF+STACK_OFF(SP)	* workdst.rW,workdst.rH
*	MOVE.W	gR_rRealWW(A1),gR_rRealWW+gR_SIZEOF+STACK_OFF(SP)
*	MOVE.L	gR_rAddr(A1),gR_rAddr+gR_SIZEOF+STACK_OFF(SP) * workdst.rAddr

	MOVE.L	(A0)+,gR_rX+STACK_OFF(SP)		* worksrc.rX,worksrc.rY
	MOVE.L	(A0)+,gR_rW+STACK_OFF(SP)		* worksrc.rW,worksrc.rH
	MOVE.W	(A0)+,gR_rRealWW+STACK_OFF(SP)	* worksrc.rRealWW
	MOVE.L	(A0)+,gR_rAddr+STACK_OFF(SP)	* worksrc.rAddr

	MOVE.L	(A1)+,gR_rX+gR_SIZEOF+STACK_OFF(SP)	* workdst.rX,workdst.rY
	MOVE.L	(A1)+,gR_rW+gR_SIZEOF+STACK_OFF(SP)	* workdst.rW,workdst.rH
	MOVE.W	(A1)+,gR_rRealWW+gR_SIZEOF+STACK_OFF(SP)
	MOVE.L	(A1)+,gR_rAddr+gR_SIZEOF+STACK_OFF(SP) * workdst.rAddr

*						* push args to resetRects
	MOVE.L	SP,A1				* save sp for PEA's
	MOVE.L 	A4,-(SP)			* CR
	PEA	gR_SIZEOF+STACK_OFF(A1)	* &workdest
	PEA	STACK_OFF(A1)			* &worksrc

	JSR	_resetRects			* clippedwidth in (word) D0

	LEA	12(SP),SP			* restore stack pointer

*	if this layer is obscured, fetch the alternate BitMap	

	TST.L	cr_lobs(A4)
	BEQ.S	not_obscured

	TST.L	cr_BitMap(A4)
	BEQ.S	no_bitmap
	MOVE.L	cr_BitMap(A4),A0

*	adjust dest gelRect x,y coordinates for special BitMap	

	MOVE.W	cr_MinX(A4),D1
	ANDI.W	#$FFF0,D1
	SUB.W	D1,gR_rX+gR_SIZEOF+STACK_OFF(SP) * workdest.rX

	MOVE.W	cr_MinY(A4),D1
	SUB.W	D1,gR_rY+gR_SIZEOF+STACK_OFF(SP) * workdest.rY

	MOVE.W	bm_BytesPerRow(A0),D1
	ASR.W	#1,D1

	MOVE.W	D1,gR_rRealWW+gR_SIZEOF+STACK_OFF(SP) * workdest.rRealWW
	
	BRA.S	is_bitmap

not_obscured:

	TST.L	rp_BitMap(A2)
	BEQ	no_bitmap
	MOVE.L	rp_BitMap(A2),A0

is_bitmap:

*						* push args to blisser
*						* BMap in A0
	MOVE.L	SP,A1				* save sp for PEA's
	EXT.L	D0				* sign extend clippedwidth
	MOVE.L	D0,-(SP)			* clippedwidth
	MOVE.L  ARG6_OFF(A5),-(SP)              * beamsync
	MOVE.L  ARG5_OFF(A5),-(SP)              * type
	MOVE.L  ARG4_OFF(A5),-(SP)              * mint
	MOVE.L  D2,-(SP)                        * RPort->Mask
	MOVE.L  ARG3_OFF(A5),-(SP)              * s
	MOVE.L  A0,-(SP)			* BMap
	PEA	gR_SIZEOF+STACK_OFF(A1)		* &workdest
	PEA	STACK_OFF(A1)			* &worksrc
	MOVE.L	A3,-(SP)			* GI (GelsInfo)

	JSR	_blisser			* do this bliss

	LEA	40(SP),SP

no_bitmap:	

*	restore A0,A1 for loop

	MOVE.L	ARG0_OFF(A5),A0			* restore src
	MOVE.L	ARG1_OFF(A5),A1			* restore dest

no_intersect:

	MOVE.L	cr_Next(A4),A4
	BRA	start_clip_loop

end_clip_loop:

*	now bliss super-cliprects, if any

	MOVE.L  rp_Layer(A2),A2                 * CW in A2
	TST.L	lr_SuperBitMap(A2)		* test if SuperBitMap layer
	BEQ	return

	MOVE.W  lr_MinX(A2),D0
	SUB.W   D0,gR_rX(A1)			* dest->rX -= CW->bounds.MinX
	MOVE.W  lr_Scroll_X(A2),D0
	ADD.W   D0,gR_rX(A1)                    * dest->rX += CW->Scroll_X
	MOVE.W  lr_MinY(A2),D0
	SUB.W   D0,gR_rY(A1)                    * dest->rY -= CW->bounds.MinY
	MOVE.W  lr_Scroll_Y(A2),D0
	ADD.W   D0,gR_rY(A1)                    * dest->rY += CW->Scroll_Y

	MOVE.L	lr_SuperClipRect(A2),A4		* CR in A4
	MOVE.L	lr_SuperBitMap(A2),A2		* CW->SuperBitMap in A2

*	bliss all super cliprects

	MOVE.W	gR_rX(A1),D3			* dest->rX in D3

	MOVE.W	D3,D4				* copy dest->rX
	ADD.W	gR_rW(A0),D4			* dest->rX + src->rW
	SUBQ.W	#1,D4				* dest->rX + src->rW - 1 in D4

	MOVE.W	gR_rY(A1),D5			* dest->rY in D5

	MOVE.W	D5,D6				* copy dest->rY
	ADD.W	gR_rH(A0),D6			* dest->rY + src->rH
	SUBQ.W	#1,D6				* dest->rY + src->rH -1 in D6

super_clip_loop:

	MOVE.L	A4,D0				* another cliprect to process ?
	BEQ	super_end_clip_loop

*	test ClipRect intersection with current bliss bounds

	CMP.W	cr_MaxX(A4),D3			* dest->rX : cr->MaxX 
	BGT	super_no_intersect		* dest->rX > cr->MaxX

	CMP.W	cr_MinX(A4),D4			* dest->rX+src->rW-1 : cr->MinX
	BLT	super_no_intersect		* dest->rX+src->rW-1 < cr->MinX

	CMP.W	cr_MaxY(A4),D5			* dest->rY : cr->MaxY 
	BGT	super_no_intersect		* dest->rY > cr->MaxY

	CMP.W	cr_MinY(A4),D6			* dest->rY+src->rH-1 : cr->MinY
	BLT	super_no_intersect		* dest->rY+src->rH-1 < cr->MinY

*	ok, these two intersect, so reset the gelRects and bliss it

*	MOVE.L	gR_rX(A0),gR_rX+STACK_OFF(SP)		* worksrc.rX,worksrc.rY
*	MOVE.L	gR_rW(A0),gR_rW+STACK_OFF(SP)		* worksrc.rW,worksrc.rH
*	MOVE.W	gR_rRealWW(A0),gR_rRealWW+STACK_OFF(SP)	* worksrc.rRealWW
*	MOVE.L	gR_rAddr(A0),gR_rAddr+STACK_OFF(SP)	* worksrc.rAddr

	MOVE.L	(A0)+,gR_rX+STACK_OFF(SP)		* worksrc.rX,worksrc.rY
	MOVE.L	(A0)+,gR_rW+STACK_OFF(SP)		* worksrc.rW,worksrc.rH
	MOVE.W	(A0)+,gR_rRealWW+STACK_OFF(SP)	* worksrc.rRealWW
	MOVE.L	(A0)+,gR_rAddr+STACK_OFF(SP)	* worksrc.rAddr

	MOVE.L	gR_rX(A1),gR_rX+gR_SIZEOF+STACK_OFF(SP)	* workdst.rX,workdst.rY
	MOVE.L	gR_rW(A1),gR_rW+gR_SIZEOF+STACK_OFF(SP)	* workdst.rW,workdst.rH

	MOVE.W	bm_BytesPerRow(A2),D0			* CW->SuperBitMap->
	ASR.W	#1,D0					* BytesPerRow >> 1

	MOVE.W	D0,gR_rRealWW+gR_SIZEOF+STACK_OFF(SP)	*   ==> workdst.rRealWW
	MOVE.L	gR_rAddr(A1),gR_rAddr+gR_SIZEOF+STACK_OFF(SP) * workdst.rAddr

*						* push args to resetRects
	MOVE.L	SP,A1				* save sp for PEA's
	MOVE.L 	A4,-(SP)			* CR
	PEA	gR_SIZEOF+STACK_OFF(A1)		* &workdest
	PEA	STACK_OFF(A1)			* &worksrc

	JSR	_resetRects			* clippedwidth in (word) D0

	LEA	12(SP),SP			* restore stack pointer

*						* push args to blisser
	MOVE.L	SP,A1				* save sp for PEA's
	EXT.L	D0				* sign extend clippedwidth
	MOVE.L	D0,-(SP)			* clippedwidth
	MOVE.L  ARG6_OFF(A5),-(SP)              * beamsync
	MOVE.L  ARG5_OFF(A5),-(SP)              * type
	MOVE.L  ARG4_OFF(A5),-(SP)              * mint
	MOVE.L  D2,-(SP)                        * RPort->Mask
	MOVE.L  ARG3_OFF(A5),-(SP)              * s
	MOVE.L  A2,-(SP)			* CW->SuperBitMap
	PEA	gR_SIZEOF+STACK_OFF(A1)		* &workdest
	PEA	STACK_OFF(A1)			* &worksrc
	MOVE.L	A3,-(SP)			* GI (GelsInfo)

	JSR	_blisser			* do this bliss

	LEA	40(SP),SP

*	restore A0,A1 for loop

	MOVE.L	ARG0_OFF(A5),A0			* restore src
	MOVE.L	ARG1_OFF(A5),A1			* restore dest

super_no_intersect:

	MOVE.L	cr_Next(A4),A4
	BRA	super_clip_loop

super_end_clip_loop:

*	done -- clean up and return

return:

	MOVEM.L	(SP)+,D2-D6/A2-A4	*restore registers
	UNLK	A5

	RTS

	END
