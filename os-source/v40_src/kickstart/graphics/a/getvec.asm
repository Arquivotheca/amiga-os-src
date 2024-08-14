*******************************************************************************
*
* $Id: getvec.asm,v 39.2 92/03/24 10:33:46 spence Exp $
*
*******************************************************************************

	section graphics

	include "/gfxbase.i"
	include "/view.i"
	include "/displayinfo.i"

	xdef	_GetVecTable
	xref	_VecLists

***i*** graphics.library/GetVecTable *******************************************
*
*   NAME
*	GetVecTable -- Find the VecTable for a given ViewPort (V39)
*
*   SYNOPSIS
*	VecTable = GetVecTable(vp, vinfo);
*	d0                     a0  a1
*
*	struct VecTable *GetVecTable(struct VecTable *, struct VecInfo *);
*
*   FUNCTION
*	To get the best VecTable for a given ViewPort.
*	The VecTable comes from the vp->ColorMap->ViewPortExtra->Vec.
*	If any pointer in the chain is NULL, then return the
*	vinfo->Vec.
*	If this is NULL, return a default depending on the GB->ChipRevBits0
*
*   INPUTS
*	vp    = viewport. If NULL, use vinfo.
*       vinfo = VecInfo. Can be NULL.
*
*   RESULT
*	VecTable = VecTable to use.
*
*   BUGS
*
*   SEE ALSO
*	<graphics/displayinfo.h> "vp_internal.h"
*
********************************************************************************

; struct VecTable * __asm GetVecTable(register __a0 struct ViewPort *vp, register __a1 struct VecInfo *vinfo);

; try for the ViewPort->ColorMap->ViewPortExtra->VecTable.
; If that is NULL, try for the VecInfo->Vec.
; If that is NULL, use the default.

_GetVecTable:
	moveq	#0,d1
	move.l	a0,d0			; check ViewPort
	beq.s	chk_vinfo
	move.w	vp_Modes(a0),d0
	and.w	#V_DUALPF,d0
	bne.s	is_dpf			; if dualpf, don't rely on vpe.
	move.l	vp_ColorMap(a0),d0	; check vp->Colormap
	beq.s	use_vl_dflt
	move.l	d0,a0
	tst.b	cm_Type(a0)		; check cm->Type
	beq.s	use_vl_dflt
	move.l	cm_vpe(a0),d0		; check cm->vpe
	beq.s	use_vl_dflt
	move.l	d0,a0
	move.l	vpe_VecTable(a0),d0	; check vpe->VecTable
	bne.s	gotvectable		; aha! got it.
chk_vinfo:
	move.l	a1,d0			; damn. Try through the VecInfo
	beq.s	use_vl_dflt
	move.l	vec_Vec(a1),d0		; check vec->Vec
	bne.s	gotvectable		; got it.
use_vl_dflt:
	lea	_VecLists,a0		; get the default list
	lea	0(a0,d1),a0		; use default AA VecList
	move.l	(a0),d0
	move.b	gb_ChipRevBits0(a6),d1
	btst.b	#GFXB_AA_LISA,d1
	bne.s	gotvectable		; is in fact AA
	move.l	8(a0),d0		; use ECS
	btst.b	#GFXB_HR_DENISE,d1
	bne.s	gotvectable		; is in fact ECS
	move.l	16(a0),d0		; use A
gotvectable:
	rts

is_dpf:
	moveq	#4,d1			; offset in VecLists for DualPlayfield
	bra.s	chk_vinfo

	end
