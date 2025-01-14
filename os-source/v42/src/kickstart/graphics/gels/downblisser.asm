*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: downblisser.asm,v 42.0 93/06/16 11:19:28 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	downblisser.asm,v $
;; Revision 42.0  93/06/16  11:19:28  chrisg
;; initial
;; 
* Revision 42.0  1993/06/01  07:21:09  chrisg
* *** empty log message ***
*
*   Revision 37.3  92/07/01  10:42:17  chrisg
*   stub gone.
*   
*   Revision 37.2  92/06/11  14:30:46  chrisg
*   short branches.
*   
*   Revision 37.1  91/05/02  17:02:48  chrisg
*   killed ".." for lattice
*   
*   Revision 37.0  91/01/07  15:28:19  spence
*   initial switchover from V36
*   
*   Revision 33.5  90/12/06  13:50:18  bart
*   removed gfxallocmem.c and implemented better lo-mem error recovery -- bart
*   
*   Revision 33.4  90/07/27  16:36:42  bart
*   id
*   
*   Revision 33.3  90/03/28  09:26:33  bart
*   *** empty log message ***
*   
*   Revision 33.2  88/11/16  13:05:58  bart
*   *** empty log message ***
*   
*   Revision 33.1  88/11/16  12:59:07  bart
*   big blits
*   
*   Revision 33.0  86/05/17  15:22:41  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'graphics/gfx.i'
	INCLUDE 'graphics/rastport.i'
	INCLUDE 'graphics/clip.i'
	INCLUDE 'graphics/gels.i'
	INCLUDE 'hardware/blit.i'
	INCLUDE	'asbob.i'
	INCLUDE	'gelsinternal.i'
	INCLUDE	'/macros.i'


SHADOWPTR_OFF	EQU	1*4
SRCX_OFF	EQU	SHADOWPTR_OFF+1*2
WORKW_OFF	EQU	SRCX_OFF+1*2
WORKH_OFF	EQU	WORKW_OFF+1*2
DESTX_OFF	EQU	WORKH_OFF+1*2
SRCINCR_OFF	EQU	DESTX_OFF+1*4
SHIFT_OFF	EQU	SRCINCR_OFF+1*2
SBITSIZE_OFF	EQU	SHIFT_OFF+1*2
DBITSIZE_OFF	EQU	SBITSIZE_OFF+1*2
SRCOFF_OFF	EQU	DBITSIZE_OFF+1*4
DESTOFF_OFF	EQU	SRCOFF_OFF+1*4
WORKSPACE	EQU	DESTOFF_OFF+1*4

ARG0_OFF	EQU	2*4
ARG1_OFF	EQU	3*4
ARG2_OFF	EQU	4*4
ARG3_OFF	EQU	5*4
ARG4_OFF	EQU	6*4
ARG5_OFF	EQU	7*4
ARG6_OFF	EQU	8*4
ARG7_OFF	EQU	9*4
ARG8_OFF	EQU	10*4
ARG9_OFF	EQU	11*4

	CODE
	XREF	_allocblissobj
	XREF	_freelastblissobj
	XREF	_BlsDn
	XREF	_qBlissIt
	XREF	_LVOQBlit
	XREF	_LVOQBSBlit
	xref	_LVOAllocMem

	XDEF	_blisser

_blisser:

	LINK	A5,#-WORKSPACE

	MOVEM.L	D2-D7/A2-A4,-(SP)		* save registers on stack

	MOVE.L	ARG0_OFF(A5),-(SP)		* push args	
	JSR	_allocblissobj			* call allocation routine for bo
	ADDQ.L	#4,SP				* restore stack pointer
	TST.L	D0				* bo == NULL ?
	BEQ	return				* return FALSE

	MOVE.L	D0,A2				* bo in A2

	CMPI.W	#B2SWAP,ARG7_OFF+2(A5)		* type == B2SWAP ?
	BNE.S	no_swap
	MOVE.L	ARG1_OFF(A5),A0			* src in A0
	MOVE.L	ARG2_OFF(A5),A1			* dest in A1
	MOVE.L	gR_rW(A0),gR_rW(A1)		* dest->rW = src->rW
*						* dest->rH = src->rH
	MOVE.L	A0,ARG2_OFF(A5)			* dest <==> src
	MOVE.L	A1,ARG1_OFF(A5)			* src <==> dest

no_swap:

	CLR.L	D0
	MOVE.L	D0,A1				* null "name" arg in A1
	CALLEXEC	FindTask		* call exec routine
	MOVE.L	D0,WHOSENTME(A2)		* bo->whoSentMe = FindTask(0)

	MOVE.B	#CLEANME,BLITSTAT(A2)		* bo->blitStat = CLEANME
	MOVE.L	#_BlsDn,CLEANUP(A2)		* bo->cleanUp = BlsDn

	MOVE.L	ARG3_OFF(A5),A3			* bm in A3 
	MOVE.B	bm_Depth(A3),BLITCNT(A2)	* bo->blitCnt = bm->Depth

	MOVE.L	#_qBlissIt,BLITROUTINE(A2)	* bo->blitRoutine = qBlissIt

	CLR.L	D0
	MOVE.B	D0,BLISSINDEX(A2)		* bo->blissIndex = 0
	MOVE.B	D0,SRCINDEX(A2)			* bo->srcIndex = 0
	MOVE.B	D0,DUMMY(A2)			* bo->deBug = FALSE
	MOVE.L	D0,SHADOW(A2)			* bo->shadow = NULL

	MOVE.L	ARG4_OFF(A5),A4			* s in A4 
	MOVE.B	vs_PlanePick(A4),D0		* byte to word adjust
	MOVE.W	D0,PPICK(A2)			* bo->pPick = s->PlanePick
	MOVE.B	vs_PlaneOnOff(A4),D0		* byte to word adjust
	MOVE.W	D0,PONOFF(A2)			* bo->pOnOff = s->PlaneOnOff

	MOVE.W	ARG6_OFF+2(A5),MINTERM(A2)	* bo->minterm = mint
	MOVE.W	ARG7_OFF+2(A5),BLITTYPE(A2)	* bo->blitType = type
	MOVE.W	ARG8_OFF+2(A5),BEAMSYNC(A2)	* bo->beamSync = beamsync

	MOVE.W	ARG5_OFF+2(A5),D0		* word to byte adjust
	MOVE.B	D0,WRITEMASK(A2)		* bo->writeMask = mask

	MOVE.L	vs_VSBob(A4),A0			* s->VSBob in A0

	MOVE.L	bob_ImageShadow(A0),-SHADOWPTR_OFF(A5)	* shadowptr =
*							* s->VSBob->ImageShadow

	MOVE.L	ARG1_OFF(A5),A0			* src in A0
	MOVE.L	ARG2_OFF(A5),A1			* dest in A1

	MOVE.W	gR_rX(A0),D0
	MOVE.W	D0,-SRCX_OFF(A5)		* srcX = src->rX
	MOVE.W	D0,D6				* word copy of srcX in D6
	EXT.L	D0
	MOVE.L	D0,D2				* long copy of srcX in D2
	MOVE.L	D0,D3				* long copy of srcX in D3

	MOVE.W	gR_rX(A1),D1
	MOVE.W	D1,-DESTX_OFF(A5)		* destX = dest->rX
	MOVE.W	D1,D7				* word copy of destX in D7
	EXT.L	D1
	MOVE.L	D1,D4				* long copy of destX in D4
	MOVE.L	D1,D5				* long copy of destX in D5

	MOVE.W	gR_rW(A0),D0			* src->rW in D0
	MOVE.W	D0,-WORKW_OFF(A5)		* workW = src->rW
	EXT.L	D0

	MOVE.W	gR_rH(A0),D1			* src->rH
	SUBQ.W	#1,D1				* src->rH - 1 
	MOVE.W	D1,-WORKH_OFF(A5)		* workH = src->rH - 1

*	sBitSize = (((srcX + workW - 1) >> 4) - (srcX >> 4) + 1) << 4;

	ADD.L	D0,D2				* srcX + workW 
	SUBQ.L	#1,D2				* srcX + workW - 1
	ASR.L	#4,D2				* (srcX + workW - 1) >> 4
	ASR.L	#4,D3				* srcX >> 4
	SUB.L	D3,D2
	ADDQ.L	#1,D2
	ASL.W	#4,D2				* result is word

	MOVE.W	D2,-SBITSIZE_OFF(A5)

*	dBitSize = (((destX + workW - 1) >> 4) - (destX >> 4) + 1) << 4;

	ADD.L	D0,D4				* destX + workW 
	SUBQ.L	#1,D4				* destX + workW - 1
	ASR.L	#4,D4				* (destX + workW - 1) >> 4
	ASR.L	#4,D5				* destX >> 4
	SUB.L	D5,D4
	ADDQ.L	#1,D4
	ASL.W	#4,D4				* result is word

	MOVE.W	D4,-DBITSIZE_OFF(A5)

*						* srcIncr = 
*						* umuls(s->Width,s->Height) << 1

	MOVE.W	vs_Width(A4),D3			* s->Width
	MOVE.W	D3,D5				* copy of s->Width in D5
	MULU	vs_Height(A4),D3		* umuls(s->Width,s->Height)
	ASL.L	#1,D3				* result is long
	MOVE.L	D3,-SRCINCR_OFF(A5)		* D3 used below if clippedwidth

	ANDI.W	#$000F,D6
	ANDI.W	#$000F,D7
	SUB.W	D6,D7				* shift =
	MOVE.W	D7,-SHIFT_OFF(A5)		* (destX & 0xF) - (srcX & 0xF) 
	MOVE.W	D7,D6				* copy of shift in D6

	BLT	shift_left

* 	shift right or no shift

	MOVE.W	MINTERM(A2),D0		* mint	
	ANDI.W	#SRCA,D0		* mint & SRCA
	BEQ	no_srca

	ANDI.W	#$000F,D7		* shift & 0xF
	MOVEQ	#ASHIFTSHIFT,D0
	ASL.W	D0,D7			* (shift & 0xF) << ASHIFTSHIFT
	MOVE.W	D7,PBCN0(A2)		* bo->pbcn0 = (shift & 0xF)<<ASHIFTSHIFT

	MOVE.W	-SRCX_OFF(A5),D0	* srcX
	ANDI.W	#$000F,D0		* srcX & 0xF
	MOVEQ	#-1,D1			* 0xFFFF (in lower word ) of D1
	LSR.W	D0,D1			* 0xFFFF >> (srcX & 0xF)
	MOVE.W D1,FWM(A2)		* bo->fwm = 0xFFFF >> (srcX & 0xF)

	CMP.W	D2,D4			* sBitSize : dBitSize
	BLE	s_not_less

	CLR.W	LWM(A2)			* bo->lwm = 0

	MOVE.W	ARG9_OFF+2(A5),D4	* clippedwidth in D4	
	BEQ	not_clipped

	MOVE.L	D3,SHADOWSIZE(A2)	* bo->shadowSize = 
*					* umuls(s->Width, s->Height) << 1

	CLR.L	D0			* call AllocMem for wordptr
	MOVE.W	D3,D0			* byteSize in D0
	MOVEQ	#MEMF_CHIP,D1		* requirements in D1

	movem.l	a0/a1/a6,-(a7)
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)
	movem.l	(a7)+,a0/a1/a6

	TST.L	D0			* wordptr == NULL ?
	BNE.s	a_ok			* no, continue

	MOVE.L	ARG0_OFF(A5),-(SP)	* push args
	JSR	_freelastblissobj	* deallocate last bo
	ADDQ.L	#4,SP			* restore stack pointer

	BRA	return			* and abort operation

a_ok:	MOVE.L	D0,SHADOW(A2)		* bo->shadowptr = wordptr

	SUBQ.W	#1,D4			* clippedwidth - 1
	MOVE.W	D4,D3			* copy of (clippedwidth - 1) in D3
	ASR.W	#4,D3			* (clippedwidth -1) >> 4

*	clipword = s->Width - 1 - ((clippedwidth - 1) >> 4)

	SUBQ.W #1,D5			* s->Width -1
	SUB.W	D3,D5			* clipword in D5

*	clipmask = 0xFFFF << (((clippedwidth - 1) & 0xF) + 1)

	ANDI.W	#$000F,D4		* (clippedwidth -1) & 0xF
	ADDQ.W	#1,D4			* ((clippedwidth -1) & 0xF) + 1
	MOVEQ	#-1,D3			* 0xFFFF (in lower word ) of D3
	ASL.W	D4,D3			* clipmask in D3

	MOVE.L	-SHADOWPTR_OFF(A5),A0	* transfer shadowptr to A0
	MOVE.L	D0,A1			* transfer wordptr to A1

	MOVE.W	vs_Height(A4),D0
	EXT.L	D0
	BRA.S	itest

iloop:

	MOVE.W	D5,D1
	EXT.L	D1
	BRA.S	jtest

jloop:	MOVE.W	(A0)+,(A1)+		* *wordptr++ = *shadowptr++

jtest:	DBRA	D1,jloop

	MOVE.W	(A0)+,D1
	AND.W	D3,D1
	MOVE.W	D1,(A1)+		* *wordptr++ = (*shadowptr++) & clipmask

	MOVE.W	vs_Width(A4),D1
	SUB.W	D5,D1
	SUBQ.W	#1,D1
	EXT.L	D1
	BRA.S	ktest

kloop:	CLR.W	(A1)+			* *wordptr++ = 0; 
	ADDQ.L	#2,A0			* shadowptr++

ktest:	DBRA	D1,kloop

itest:	DBRA	D0,iloop

	MOVE.L	SHADOW(A2),-SHADOWPTR_OFF(A5)	* shadowptr = bo->shadow

not_clipped:

	BRA.s	after_s_not_less

s_not_less:

	MOVE.W	-SRCX_OFF(A5),D0	* srcX
	MOVEQ	#-1,D1			* 0xFFFF (in lower word ) of D1

	ADD.W	-WORKW_OFF(A5),D0
	ANDI.W	#$000F,D0
	EXT.L	D0
	BEQ.s	local1			* bo->lwm = 0xFFFF

	MOVEQ	#16,D3
	SUB.L	D0,D3
	ASL.W	D3,D1			* bo->lwm =
*					* 0xFFFF << (16-((srcX + workW) & 0xF))
local1

	MOVE.W	D1,LWM(A2)

after_s_not_less:

	BRA.S	after_srca

no_srca:

	CLR.W	PBCN0(A2)		* bo->pbcn0 = 0

	MOVEQ	#-1,D1			* 0xFFFF (in lower word ) of D1
	MOVE.W	-DESTX_OFF(A5),D0
	
	MOVE.W	D0,D3			* copy of destX in D3
	ANDI.W	#$000F,D3		* destX & 0xF
	LSR.W	D3,D1			* 0xFFFF >> (destX & 0xF)
	MOVE.W	D1,FWM(A2)		* bo->fwm = 0xFFFF >> (destX & 0xF)

	MOVEQ	#-1,D1			* 0xFFFF (in lower word ) of D1

	ADD.W	-WORKW_OFF(A5),D0
	ANDI.W	#$000F,D0
	EXT.L	D0
	BEQ.s	local2			* bo->lwm = 0xFFFF

	MOVEQ	#16,D3
	SUB.L	D0,D3
	ASL.W	D3,D1			* bo-lwm =
*					* 0xFFFF << (16-((destX + workW) & 0xF))

local2

	MOVE.W	D1,LWM(A2)

after_srca:

	ANDI.W	#$000F,D6		* shift & 0xF
	MOVEQ	#BSHIFTSHIFT,D0
	ASL.W	D0,D6			* (shift & 0xF) << BSHIFTSHIFT
	MOVE.W	D6,BCN1(A2)		* bo->bcn1 = (shift & 0xF)<<BSHIFTSHIFT

	MOVE.W	-DBITSIZE_OFF(A5),-SBITSIZE_OFF(A5)	* sBitSize = dBitSize

	MOVE.L	ARG1_OFF(A5),A0		* src in A0

	JSR	rastoff			* D1 <= rastoff(src)

	MOVE.W	-WORKH_OFF(A5),D0	* workH
	MULU	gR_rRealWW(A0),D0	* umuls(workH,(A0)->rRealWW)
	ADD.L	D0,D1			* rastoff(A0)+umuls(workH,(A0)->rRealWW)
	ASL.L	#1,D1			* << 1

	MOVE.L	D1,-SRCOFF_OFF(A5)	* assign result to srcOff

	MOVE.L	ARG2_OFF(A5),A0		* dest in A0

	JSR	rastoff			* D1 <= rastoff(dest)

	MOVE.W	-WORKH_OFF(A5),D0	* workH
	MULU	gR_rRealWW(A0),D0	* umuls(workH,(A0)->rRealWW)
	ADD.L	D0,D1			* rastoff(A0)+umuls(workH,(A0)->rRealWW)
	ASL.L	#1,D1			* << 1

	MOVE.L	D1,-DESTOFF_OFF(A5)	* assign result to destOff

	BRA.s	after_shift

shift_left:

* 	shift left

	NEG.W	D7			* shift = -shift
	MOVE.W	D7,-SHIFT_OFF(A5)	* re-save shift

	MOVE.W	D7,D1			* copy of shift in D1
	ANDI.W	#$000F,D1		* shift & 0xF
	MOVEQ	#ASHIFTSHIFT,D0
	ASL.W	D0,D1			* (shift & 0xF) << ASHIFTSHIFT
	MOVE.W	D1,PBCN0(A2)		* bo->pbcn0 = (shift & 0xF)<<ASHIFTSHIFT

	ANDI.W	#$000F,D7		* shift & 0xF
	MOVEQ	#BSHIFTSHIFT,D0
	ASL.W	D0,D7			* (shift & 0xF) << BSHIFTSHIFT
	ORI.W	#BLITREVERSE,D7		* | BLITREVERSE
	MOVE.W	D7,BCN1(A2)		* bo->bcn1 = 
*					* (shift & 0xF)<<BSHIFTSHIFT|BLITREVERSE

	MOVEQ	#-1,D1			* 0xFFFF in (lower word) of D1
	MOVE.W	-SRCX_OFF(A5),D0	* srcX
	ANDI.W	#$000F,D0		* srcX & 0xF
	LSR.W	D0,D1			* 0xFFFF >> (srcX & 0xF)
	MOVE.W	D1,LWM(A2)		* bo->lwm = 0xFFF >> (srcX & 0xF)

	MOVE.W	-SRCX_OFF(A5),D0	* srcX
	MOVEQ	#-1,D1			* 0xFFFF (in lower word ) of D1

	ADD.W	-WORKW_OFF(A5),D0
	ANDI.W	#$000F,D0
	EXT.L	D0
	BEQ.s	local3			* bo->fwm = 0xFFFF

	MOVEQ	#16,D3
	SUB.L	D0,D3
	ASL.W	D3,D1			* bo->fwm =
*					* 0xFFFF << (16-((srcX + workW) & 0xF))
local3

	MOVE.W	D1,FWM(A2)

	EXT.L	D2			* long-extend sBitSize
	ASR.L	#4,D2			* sBitSize >> 4
	SUBQ.L	#1,D2			* (sBitSize >> 4) - 1

	MOVE.L	ARG1_OFF(A5),A0		* src in A0
	JSR	rastoff			* D1 <= rastoff(src)
	ADD.L	D2,D1			* rastoff(src)+((sBitSize >> 4) - 1)
	ASL.L	#1,D1			* <<1
	MOVE.L	D1,-SRCOFF_OFF(A5)	* assign result to srcOff

	MOVE.L	ARG2_OFF(A5),A0		* dest in A0
	JSR	rastoff			* D1 <= rastoff(dest)
	ADD.L	D2,D1			* rastoff(dest)+((sBitSize >> 4) - 1)
	ASL.L	#1,D1			* <<1
	MOVE.L	D1,-DESTOFF_OFF(A5)	* assign result to destOff

after_shift:

	CMPI.W	#B2SWAP,ARG7_OFF+2(A5)	* type == B2SWAP ?
	BNE.S	not_b2swap

*	we're saving the background into the buffer

	MOVE.L	-DESTOFF_OFF(A5),D0	* destOff
*	ASR.L	#1,D0			* destOff >> 1
	ADD.L	-SHADOWPTR_OFF(A5),D0	* shadowptr + (destOff >> 1)
	MOVE.L	D0,ASRC(A2)		* bo->asrc = shadowptr + (destOff >> 1)
	
	MOVE.L	ARG2_OFF(A5),A0		* dest in A0
	MOVE.L	gR_rAddr(A0),D3		* use D3 for t
	ADD.L	-DESTOFF_OFF(A5),D3	* t(0) = dest->rAddr + destOff

	CLR.W	D0			* use D0 for i

mtest:	CMP.B	bm_Depth(A3),D0		* i : bm->Depth
	BGE.S	mstop			* i < bm->Depth ?

	MOVE.W	D0,D1			* copy of i	
	ASL.W	#2,D1			* i<<2 (pointer offset)

	MOVE.L	bm_Planes(A3,D1.W),D2	* bm->Planes[i]
	ADD.L	-SRCOFF_OFF(A5),D2	* bm->Planes[i] + srcOff
	MOVE.L	D2,SRCPTR(A2,D1.W)	* bo->srcPtr[i] = bm->Planes[i] + srcOff
*					* note initialization of t above
	MOVE.L	D3,DESTPTR(A2,D1.W)	* bo->destPtr[i] = dest->rAddr+destOff+t
	ADD.L	-SRCINCR_OFF(A5),D3	* t += srcIncr

	ADDQ.W	#1,D0			* i++
	BRA.S	mtest

mstop:	

	BRA.S	after_b2swap

not_b2swap:

	MOVE.L	-SRCOFF_OFF(A5),D0	* srcOff
*	ASR.L	#1,D0			* srcOff >> 1
	ADD.L	-SHADOWPTR_OFF(A5),D0	* shadowptr + (srcOff >> 1)
	MOVE.L	D0,ASRC(A2)		* bo->asrc = shadowptr + (srcOff >> 1)

	MOVE.L	ARG1_OFF(A5),A0		* src in A0
	MOVE.L	gR_rAddr(A0),D3		* use D3 for t
	ADD.L	-SRCOFF_OFF(A5),D3	* t(0) = src->rAddr + srcOff

	CLR.W	D0			* use D0 for i

ntest:	CMP.B	bm_Depth(A3),D0		* i : bm->Depth
	BGE.S	nstop			* i < bm->Depth ?

	MOVE.W	D0,D1			* copy of i	
	ASL.W	#2,D1			* i<<2 (pointer offset)

*					* note initialization of t above
	MOVE.L	D3,SRCPTR(A2,D1.W)	* bo->destPtr[i] = src->rAddr+src+t
	ADD.L	-SRCINCR_OFF(A5),D3	* t += srcIncr

	MOVE.L	bm_Planes(A3,D1.W),D2	* bm->Planes[i]
	ADD.L	-DESTOFF_OFF(A5),D2	* bm->Planes[i] + destOff
	MOVE.L	D2,DESTPTR(A2,D1.W)	* bo->destPtr[i] = bm->Planes[i]+destOff

	ADDQ.W	#1,D0			* i++
	BRA.S	ntest

nstop:	

after_b2swap:

*	bo->blitSize = blitsize(src->rH, sBitSize)

	MOVE.L	ARG1_OFF(A5),A0		* src in A0
	MOVE.W	gR_rH(A0),D0		* src->rH
	MOVE.W	D0,BLITSIZV(A2)		* bo->blitsizv = src->rH  - big blits!!!
	ASL.W	#HSIZEBITS,D0		* src->rH << HSIZEBITS
	MOVE.W	-SBITSIZE_OFF(A5),D1	* sBitSize
	LSR.W	#4,D1			* sBitSize >> 4
	MOVE.W	D1,BLITSIZH(A2)		* big blits!!!
	OR.W	D1,D0			* src->rH << HSIZEBITS | sBitSize >> 4
	MOVE.W	D0,BLITSIZE(A2)		* bo->blitsize =
*					* src->rH << HSIZEBITS | sBitSize >> 4

	MOVE.W	gR_rRealWW(A0),D0	* src->rRealWW
	ADD.W	D1,D0			* src->rRealWW + (sBitSize >> 4)
	NEG.W	D0			* -(src->rRealWW + (sBitSize >> 4))
	ASL.W	#1,D0			* (-(src->rRealWW + (sBitSize >> 4)))<<1
	MOVE.W	D0,BMDSRC(A2)		* bo->bmdsrc =
*					* (-(src->rRealWW + (sBitSize >> 4)))<<1

	MOVE.L	ARG2_OFF(A5),A0		* dest in A0

	MOVE.W	gR_rRealWW(A0),D0	* dest->rRealWW
	ADD.W	D1,D0			* dest->rRealWW + (sBitSize >> 4)
	NEG.W	D0			* -(dest->rRealWW + (sBitSize >> 4))
	ASL.W	#1,D0			*(-(dest->rRealWW + (sBitSize >> 4)))<<1
	MOVE.W	D0,BMDDST(A2)		* bo->bmddst =
*					*(-(dest->rRealWW + (sBitSize >> 4)))<<1
	
*	add this blissObj to gelsinfo list

	MOVE.L	ARG0_OFF(A5),A1		* gi in A1
	MOVE.L	gi_lastBlissObj(A1),D0	* botemp
	BEQ.S	nolast			* botemp == NULL ?

	MOVE.L	D0,A0			* botemp in A0
	MOVE.L	A2,NEXT(A0)		* botemp->Next = bo

nolast: MOVE.L	A2,gi_lastBlissObj(A1)	* gi->lastBlissObj = bo

	MOVE.B	gi_Flags(A1),D0		* gi->Flags
	MOVE.L	A2,A1			* bo in A1 for queue
	ANDI.B	#RPF_DBUFFER,D0		* gi->Flags & DBUFFER
	BEQ.s	qbs_it			* gi->Flags & DBUFFER ?

	JSR	_LVOQBlit(A6)		* queue the blit

	BRA.S	return

qbs_it:

	JSR	_LVOQBSBlit(A6)		* queue the blit

return:

	MOVEM.L	(SP)+,D2-D7/A2-A4	* restore registers
	UNLK	A5

	RTS

rastoff:

	MOVE.W	gR_rY(A0),D1		* (A0)->rY
	MULU	gR_rRealWW(A0),D1	* umuls((A0)->rY,(A0)->rRealWW) 
	MOVE.W	gR_rX(A0),D0		* (A0)->rX
	EXT.L	D0			* convert to long
	ASR.L	#4,D0			* (A0)->rX >> 4
	ADD.L	D0,D1			* rastoff(A0) in D1
	RTS

	END
