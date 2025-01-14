*******************************************************************************
*
*	$Id: changevpbitmap.asm,v 39.15 93/02/16 12:22:15 chrisg Exp $
*
*******************************************************************************

	include	'/gfxbase.i'
	include	'/monitor.i'
	include	'/view.i'
	include	'exec/memory.i'
	include	'exec/ables.i'
	include	'exec/execbase.i'
	include	'hardware/custom.i'
	include	'/macros.i'

	SECTION	graphics
	
	OPTIMON

	xref	_LVOFreeSignal,_LVOAllocSignal,_LVOFreeMem,_LVOAllocMem
	xref	_LVOReplyMsg

******* graphics.library/AllocDBufInfo ****************************************
*
*   NAME
*       AllocDBufInfo -- Allocate structure for multi-buffered animation (V39)
*
*   SYNOPSIS
*       AllocDBufInfo(vp)
*		      a0
*
*	struct DBufInfo * AllocDBufInfo(struct ViewPort *)
*
*   FUNCTION
*	Allocates a structure which is used by the ChangeVPBitMap()
*	routine.
*
*   INPUTS
*       vp  =  A pointer to a ViewPort structure.
*
*   BUGS
*
*   NOTES
*	Returns 0 if there is no memory available or if the display mode
*	of the viewport does not support double-buffering.
*
*	The only fields of the DBufInfo structure which can be used by application
*	programs are the dbi_SafeMessage, dbi_DispMessage, dbi_UserData1 and 
*	dbi_UserData2 fields.
*
*	dbi_SafeMessage and dbi_DispMessage are standard exec message structures
*	which may be used for synchronizing your animation with the screen update.
*
*	dbi_SafeMessage is a message which is replied to when it is safe to write to
*	the old BitMap (the one which was installed when you called ChangeVPBitMap).
*
*	dbi_DispMessage is replied to when it is safe to call ChangeVPBitMap again
*	and be certain that the new frame has been seen at least once.
*
*	The dbi_UserData1 and dbi_UserData2 fields, which are stored after each
*	message, are for your application to stuff any data into that it may need
*	to examine when looking at the reply coming into the ReplyPort for either
*	of the embedded Message structures.
*
*	DBufInfo structures MUST be allocated with this function. The size of
*	the structure will grow in future releases. 
*
*	The following fragment shows proper double buffering synchronization:
*
*	int SafeToChange=TRUE, SafeToWrite=TRUE, CurBuffer=1;
*	struct MsgPort *ports[2];    /* reply ports for DispMessage and SafeMessage */
*	struct BitMap *BmPtrs[2];
*	struct DBufInfo *myDBI;
*
*	... allocate bitmap pointers, DBufInfo, set up viewports, etc.
*
*	myDBI->dbi_SafeMessage.mn_ReplyPort=ports[0];
*	myDBI->dbi_DispMessage.mn_ReplyPort=ports[1];
*	while (! done)
*	{
*	    if (! SafeToWrite)
*		while(! GetMsg(ports[0])) Wait(1l<<(ports[0]->mp_SigBit));
*	    SafeToWrite=TRUE;
*
*	    ... render to bitmap # CurBuffer.
*
*	    if (! SafeToChange)
*		while(! GetMsg(ports[1])) Wait(1l<<(ports[1]->mp_SigBit));
*	    SafeToChange=TRUE;
*	    WaitBlit();         /* be sure rendering has finished */
*	    ChangeVPBitMap(vp,BmPtrs[CurBuffer],myDBI);
*	    SafeToChange=FALSE;
*	    SafeToWrite=FALSE;
*	    CurBuffer ^=1;	/* toggle current buffer */
*	}
*       if (! SafeToChange)	/* cleanup pending messages */
*	    while(! GetMsg(ports[1])) Wait(1l<<(ports[1]->mp_SigBit));
*       if (! SafeToWrite)	/* cleanup */
*	    while(! GetMsg(ports[0])) Wait(1l<<(ports[0]->mp_SigBit));
*
*   SEE ALSO
*	FreeDBufInfo() ChangeVPBitMap()
*
*********************************************************************************

	xdef	_AllocDBufInfo

_AllocDBufInfo:
	move.l	a6,-(a7)
	move.l	gb_ExecBase(a6),a6
	move.l	#dbi_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
1$:	move.l	(a7)+,a6
	rts

******* graphics.library/FreeDBufInfo ****************************************
*
*   NAME
*       FreeDBufInfo -- free information for multi-buffered animation (V39)
*
*   SYNOPSIS
*       FreeDBufInfo(db)
*	             a1
*
*	void FreeDBufInfo(struct DBufInfo *)
*
*   FUNCTION
*	Frees a structure obtained from AllocDBufInfo
*
*   INPUTS
*       db  =  A pointer to a DBufInfo.
*
*   BUGS
*
*   NOTES
*	FreeDBufInfo(NULL) is a no-op.
*
*   SEE ALSO
*	AllocDBufInfo() ChangeVPBitMap()
*
*********************************************************************************

	xdef	_FreeDBufInfo

_FreeDBufInfo:
	move.l	a1,d0
	beq.s	1$
	movem.l	a6,-(a7)
	move.l	gb_ExecBase(a6),a6
	move.l	#dbi_SIZEOF,d0
	jsr	_LVOFreeMem(a6)
	movem.l	(a7)+,a6
1$:	rts

	xdef	_ChangeVPBitMap
******* graphics.library/ChangeVPBitMap ****************************************
*
*   NAME
*       ChangeVPBitMap -- change display memory address for multi-buffered
*			  animation (V39)
*
*   SYNOPSIS
*       ChangeVPBitMap(vp,bm,db)
*	               a0 a1 a2
*
*	void ChangeVPBitMap(struct ViewPort *, struct BitMap *, struct DBufInfo *);
*
*   FUNCTION
*	Changes the area of display memory which will be displayed in a
*	viewport. This can be used to implement double (or triple)
*	buffering, a method of achieving smooth animation.
*
*   INPUTS
*	vp  =  a pointer to a viewport
*       bm  = a pointer to a BitMap structure. This BitMap structure must be
*	      of the same layout as the one attached to the viewport (same
*	      depth, alignment, and BytesPerRow).
*	db  =  A pointer to a DBufInfo.
*	
*   BUGS
*
*   NOTES
*	This will set the vp->RasInfo->BitMap field to the bm pointer which is
*	passed.
*
*	When using the synchronization features, you MUST carefully insure that
*	all messages have been replied to before calling FreeDBufInfo or
*	calling ChangeVPBitMap with the same DBufInfo.
*	
*
*   SEE ALSO
*	AllocDBufInfo() AllocBitMap()
*
*********************************************************************************

	xref	_vposr,_vhposr,_intena
	xref	_LVOObtainSemaphore,_LVOReleaseSemaphore

_ChangeVPBitMap:
	movem.l	d2-d7/a2-a6,-(a7)
	move.l	a0,a3	; a3=vp
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	a6,a5
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOObtainSemaphore(a6)		; obtainsemaphore better not touch my fucking registers.
; now, we need to get registers a5=lof a4=shf a0=dspins

no_hide:
	move.l	a5,a6
	move.l	gb_SpecialCounter(a6),d0
	cmp.l	dbi_MatchLong(a2),d0
	beq	cache_hit			; cache is valid
	move.l	vp_DspIns(a3),d0
	beq	no_dspins
	move.l	d0,a4				; a4=dspins
	move.w	vp_Modes(a3),d0
	and.w	#V_VP_HIDE,d0
	beq.s	no_hidden
	sub.l	a5,a5
	sub.l	a4,a4
	sub.l	a0,a0
	bra.s	synced_hw_2
no_hidden:
	move.l	vp_DspIns(a3),a0
	move.l	cl_CopIns(a0),a0		; a0=intermediate copper list ptr
; now, let's skip all three copper list pointers until we hit the bitplane ptr loads
	moveq	#3,d1
	move.w	#$fff,d3
	cmp	#0,a0
	beq.s	synced_intermed
	move.l	vp_ColorMap(a3),d0	; get colormap
	beq.s	sync_intermed
	move.l	d0,a5
	btst	#CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a5)
	beq.s	sync_intermed
	sub.l	a0,a0
	bra.s	synced_intermed
sync_intermed:
	move.w	ci_OpCode(a0),d0
	and.w	d1,d0
	ifne	COPPER_MOVE
		fail copper_move not zero
	endc
	bne.s	is_wait_ins
	move.w	ci_DestAddr(a0),d2		; d2=reg ofs
	and.w	d3,d2
	cmp.w	#$e0,d2				; is it bpl1pth?
	beq.s	synced_intermed
is_wait_ins:
	lea	ci_SIZEOF(a0),a0
	bra.s	sync_intermed
synced_intermed:
; now, sync hw copper lists
	move.l	cl_CopLStart(a4),a5		; a5=lof
	move.l	cl_CopSStart(a4),a4		; a4=shf
	cmp	#0,a4
	beq.s	synced_hw_1
1$:	cmp.w	#bplpt,(a4)
	lea	4(a4),a4
	bne.s	1$
	lea	-4(a4),a4
synced_hw_1:
	cmp	#0,a5
	beq.s	synced_hw_2
1$:	cmp.w	#bplpt,(a5)
	lea	4(a5),a5
	bne.s	1$
	lea	-4(a5),a5
synced_hw_2:
; now, we're pointing at the copper list
; a0=intermediate copper list
; a4=hw 1 shf
; a5=hw 2 lof
; a2=dbufinfo
; a3=vp
; a1=change to bitmap
; a6=gfxbase
	movem.l	a0/a4/a5,dbi_CopPtr1(a2)
	move.l	a3,-(sp)
	move.l	a1,a3
	xref	_new_mode
	jsr	_new_mode
	move.l	a3,a1
	move.l	(sp)+,a3
	move	#30000,d1					; set if sync not needed.
	btst	#13-8,vp_Modes(a3)		; VP_HIDE?
	bne.s	no_sync_needed
	move.l	gb_ActiView(a6),a0
	cmp	#0,a0
	beq.s	no_sync_needed
	move.w	vp_DyOffset(a3),d1
	btst	#2,d0				; LACE?
	beq.s	no_lace1
	asr.w	#1,d1
no_lace1:
	btst	#3,d0				; doublescan?
	beq.s	no_dbscan2
	add.w	d1,d1
no_dbscan2:
	add.w	v_DyOffset(a0),d1

no_sync_needed:
	move.w	vp_DHeight(a3),d2
	btst	#3,d0				; double scan?
	beq.s	no_dbscan
	add.w	d2,d2
no_dbscan:
	add.w	d1,d2
	move.l	gb_SpecialCounter(a6),dbi_MatchLong(a2)
	move.l	gb_current_monitor(a6),a0
	move.w	d1,d0
	sub.w	ms_min_row(a0),d0
	bpl.s	1$
; if top-min_row <0
;    bot+=(top-min_row)
;    top=min_row
	add.w	d0,d2
	sub.w	d0,d1
1$:
	move.w	d1,dbi_BeamPos1(a2)
	move.w	d2,dbi_BeamPos2(a2)

cache_hit:
; first, let's avoid the beam.
	move.l	gb_ExecBase(a6),a0
; let's set d3 to the line to avoid
	move.w	dbi_BeamPos1(a2),d3
loopagain:
	DISABLE	a0,NOFETCH			; disable ints while examining beam position.
bad_count:
	move.w	_vposr,d0
	move.w	_vhposr,d2
	move.w	_vposr,d1

	cmp.w	d0,d1
	bne.s	bad_count
	lsr.w	#1,d1
	roxr.w	#1,d2
	lsr.w	#1,d1
	roxr.w	#1,d2
	lsr.w	#1,d1
	roxr.w	#1,d2
	lsr.w	#5,d2
	btst	#GFXB_HR_AGNUS,gb_ChipRevBits0(a6)
	bne.s	smart_agnus
	and.w	#511,d2				; mask out pre-ECS bits
smart_agnus:
	sub.w	d3,d2
	bge.s	good_count
	cmp.w	#-5,d2
	blt.s	good_count
	ENABLE	a0,NOFETCH
	bra.s	loopagain

good_count:
	movem.l	dbi_CopPtr1(a2),a0/a4/a5
	moveq	#0,d5
	move.w	vp_Modes(a3),d0
	btst	#13,d0				; VP_HIDE?
	beq.s	no_hide1
	moveq	#0,d2
	bra	done_pokes
no_hide1:
	btst	#8+3,d0				; HAM?
	beq.s	1$
	moveq	#2*4,d5				; ham8 bitplane fudge
1$:	move.l	vp_RasInfo(a3),a3
	move.l	ri_BitMap(a3),d0
	move.l	a1,ri_BitMap(a3)
	move.l	d0,a3
	moveq	#0,d0
	move.b	bm_Depth(a1),d0
	cmp.b	#8,d0
	beq.s	2$
	moveq	#0,d5
2$:
	lea	bm_Planes(a3),a3
	lea	bm_Planes(a1),a1
	moveq	#$1c,d4
	bra.s	end_loop
pl_loop:
	move.l	a5,d1
	beq.s	no_1st
; now, look at which register this is
	move.w	4(a5),d1	; bplxptl
	sub.w	d5,d1
	and.w	d4,d1
	move.w	2(a5),d6
	swap	d6
	move.w	6(a5),d6	; d6=generated pointer
	sub.l	0(a3,d1.w),d6	; d6=generated-old
	add.l	0(a1,d1.w),d6	; d6=new+(gen-old)
	move.w	d6,6(a5)
	swap	d6
	move.w	d6,2(a5)
	lea	8(a5),a5
no_1st:
	move.l	a4,d1
	beq.s	no_2nd
	move.w	4(a4),d1	; bplxptl
	sub.w	d5,d1
	and.w	d4,d1
	move.w	2(a4),d6
	swap	d6
	move.w	6(a4),d6	; d6=generated pointer
	sub.l	0(a3,d1.w),d6	; d6=generated-old
	add.l	0(a1,d1.w),d6	; d6=new+(gen-old)
	move.w	d6,6(a4)
	swap	d6
	move.w	d6,2(a4)
	lea	8(a4),a4
no_2nd:
end_loop:
	dbra	d0,pl_loop

	move.l	a0,d1
	beq.s	no_intermed
	move.w	#$fff,d7
it_lp:	move.w	ci_DestAddr+ci_SIZEOF(a0),d1
	and.w	d7,d1
	cmp.w	#bplpt,d1
	blo.s	no_intermed
	cmp.w	#bplpt+8*4,d1
	bhs.s	no_intermed

	sub.w	d5,d1
	and.w	d4,d1
	
	move.w	ci_DestData(a0),d6
	swap	d6
	move.w	ci_DestData+ci_SIZEOF(a0),d6
	sub.l	0(a3,d1.w),d6
	add.l	0(a1,d1.w),d6
	move.w	d6,ci_DestData+ci_SIZEOF(a0)
	swap	d6
	move.w	d6,ci_DestData(a0)
	lea	ci_SIZEOF*2(a0),a0
	bra.s	it_lp
no_intermed:

done_pokes:
; now, we need to signal queue the db node if the replyports are set.
;
;
;    d2 neg	---------------------------------	safe2write      safe2change
;		|other vp			|	  now		  nextvb
;		|				|
;    d2=0	---------------------------------
;		|the vp				|	  nextvb	  2ndvb
;		|				|
;		|				|
;    d2=bpos2	---------------------------------
;		|other vp			|	  now		  2ndvb
;		|				|
;		---------------------------------
;

	move.l	a6,a5
	move.l	gb_ExecBase(a6),a6

	tst.l	dbi_SafeMessage+MN_REPLYPORT(a2)
	beq.s	no_safesig
; sig now if d3 <0 or >bpos2, else queue 1 vb ahead.
	tst.w	d2
	bmi.s	signow
	cmp.w	dbi_BeamPos2(a2),d2
	blt.s	signext
signow:	lea	dbi_SafeMessage(a2),a1
	jsr	_LVOReplyMsg(a6)
	bra.s	no_safesig
signext:
	move.l	#1,dbi_Count1(a2)
	move.l	gb_DBList(a5),dbi_Link1(a2)
	move.l	a2,gb_DBList(a5)
no_safesig:
	tst.l	dbi_DispMessage+MN_REPLYPORT(a2)
	beq.s	no_chgsig
	moveq	#1,d3
	tst.w	d2
	bmi.s	chgnextvb
	addq.l	#1,d3
chgnextvb:
	lea	dbi_Link2(a2),a2
	move.l	d3,dbi_Count1(a2)
	move.l	gb_DBList(a5),dbi_Link1(a2)
	move.l	a2,gb_DBList(a5)
no_chgsig:
	ENABLE
	move.l	a5,a6
no_dspins:
	movem.l	(a7)+,d2-d7/a2-a6
	move.l	a6,d0
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	gb_ExecBase(a6),a6
	jsr	_LVOReleaseSemaphore(a6)
	move.l	d0,a6
	rts




	

	end
