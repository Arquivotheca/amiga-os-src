*******************************************************************************
*
*	$Id: misc.asm,v 39.38 93/05/06 12:11:31 spence Exp $
*
*******************************************************************************

    include	'exec/types.i'
    include	'exec/nodes.i'
    include	'exec/lists.i'
    include	'exec/interrupts.i'
    include	'exec/libraries.i'
    include 	'exec/ables.i'
    include	'utility/tagitem.i'
    include	'/gfxbase.i'
    include	'/vp_internal.i'
    include	'/gfx.i'
    include	'/view.i'

    include	'hardware/blit.i'
    include	'hardware/intbits.i'
    include	'hardware/dmabits.i'
    include	'/cia8520.i'
    include	'hardware/custom.i'
    include	'/sane_names.i'
    include	'hardware/custom.i'
    include	'/macros.i'
    include	'/a/submacs.i'
    include	'/rastport.i'
    include	'/clip.i'
    include	'/display.i'

	xref	_LVOGfxSpare1,_LVOReplyMsg,_LVODisownBlitter
	xref	_LVODoHookClipRects
	xref	_LVOGetBitMapAttr

	section	graphics
DEFSTOP equ     0
SYSTRAP equ     $1e
SUPERVISOR equ  -60
IC_COLDRESET    equ 1

*MOLLYFIX	equ	1
ANUTHABADAGNUS	equ	1
DEBUGCOLOR  equ 0
PRINTF	equ 0
*HEDLEY	equ 0

    xref    _custom
    xref    _ciab,_ciaa
    xref    _intena
    xref    _intreq
    xref    _intreqr
    xref    _dmacon
    xref    _dmaconr
    xref    _vposr
    xref    _vhposr

    xref    _cpwrup
    xref    _dflt_clrs
    xref    _DisownBlitter
    xref    _disownnodec
	xref	waitblitdone

    xref    _LVOAbleICR
    xref    _LVOSetICR
    xref    _Debug

    xdef    _get_denise_id
    xdef    _OpenGfx
    xdef    _CloseGfx
    xdef    _ExpungeGfx
    xdef    _ExtFuncGfx,_AlwaysReturnsZero
    xdef    _FindMSBSet
    xdef    _GetTagDataUser
    xdef    _IsAUCopIns

	xref	_LVOOpenLibrary,_LVOInstallLayerHook
	xref	_LVOSortLayerCR,_LVOInitResident,_LVOFindResident

_OpenGfx:
	tst.l	gb_LayersBase(a6)
	bne.s	_ExtFuncGfx
	move.l	a6,-(a7)
	move.l	gb_ExecBase(a6),a6
	lea	lname(pc),a1
	jsr	_LVOFindResident(a6)	; this will init layers, which will poke its base into gb_lbase
	move.l	d0,a1
	moveq	#0,d1
	jsr	_LVOInitResident(a6)
	move.l	(a7)+,a6
_ExtFuncGfx:
	move.l  a6,d0
	rts

_CloseGfx:
_ExpungeGfx:
_AlwaysReturnsZero:
	moveq	#0,d0		* this library is currently never removed
	rts

*	xdef	_FetchGBase
*_FetchGBase:
*	move.l	a6,d0
*	rts

    xdef    _vbasm
_vbasm:
	lea.l	_custom,a0
	movem.l	d2/d4,-(sp)
	addq.l	#1,gb_VBCounter(a1)
	move.l	gb_current_monitor(a1),d2
	beq.s	no_moni
	move.l	d2,a6
	move.w	ms_total_rows(a6),vtotal(a0)

* Check if beamcon0 should be changed from a LoadView(). Some applications
* (and bootmenu) write directly to beamcon0, so we don't want to overwrite them.

	move.w	ms_BeamCon0(a6),d4
	btst.b	#3,gb_system_bplcon0+1(a1)	; lpen set?
	beq.s	no_lpen
	btst.b	#GFXB_HR_AGNUS,gb_ChipRevBits0(a1)
	beq.s	no_beamcon0			; can't set beamcon0 on <ECS
	or.w	#$2000,d4			; set LPENDIS so we can read vposr
	bra.s	do_beamcon0
no_lpen:
	btst.b	#BMB_CHANGE,gb_Bugs(a1)
	beq.s	no_beamcon0
do_beamcon0:
	move.w	d4,beamcon0(a0)
	bclr.b	#BMB_CHANGE,gb_Bugs(a1)
no_beamcon0:
	btst.b	#MSB_REQUEST_SPECIAL,ms_Flags+1(a6)
	lea	gb_MonitorVBlank(a1),a6
	move.l	asi_Start(a6),d2	; vbstrt and vbstop
no_moni:
	move.l	gb_ExecBase(a1),a6	; For DISABLE/ENABLE macros
	btst	#2,gb_Modes+1(a1)	; lace?
	if <>
		moveq	#0,d1
* if LPEN, and not HR_AGNUS, don't check vposr.
		move.b	gb_system_bplcon0+1(a1),d1
		or.b	gb_ChipRevBits0(a1),d1
		and.b	#($8|GFXF_HR_AGNUS),d1
		cmp.b	#$8,d1
		beq.s	just_doit
		move.w	gb_TopLine(a1),d1
		DISABLE					; blit ints fucking this up.
		move.l	vposr(a0),d0 	; too far?
		and.l	#$007FF00,d0
		cmp.l	d1,d0
		if <
*			long or short frame?
just_doit:
			tst.b	vposr(a0) 
			if >= 
				move.l	d2,vbstrt(a0)	; write VBSTRT, VBSTOP
				move.l gb_SHFlist(a1),cop2ptr(a0)
			else
				IFD	VBLANK_BUG
				btst.b	#BMB_BUG_VBLANK,gb_Bugs(a1)
				beq.s	1$
				add.l	#$10000,d2
1$:				ENDC
				move.l	d2,vbstrt(a0)	; write VBSTRT, VBSTOP
				move.l  gb_LOFlist(a1),cop2ptr(a0)
			endif
		endif
		ENABLE
	else
		IFD	VBLANK_BUG
		btst.b	#BMB_BUG_VBLANK,gb_Bugs(a1)
		beq.s	1$
		add.l	#$10000,d2
1$:		ENDC

* If the display mode just changed from Lace to non-laced, then the 
* mode may be stuck in the ShortFrame mode. So make sure it's Long.
* However, don't do this if the system_beamcon0 LACE bit is set.

		DISABLE
		btst.b	#2,gb_system_bplcon0+1(a1)
		bne.s	not_special	; we really want it LACEd
		move.w	vposr(a0),d0
		bmi.s	not_special	; already LOF
		move.w	vposr(a0),d0	; reduce errors by reading again
		or.w	#VPOSRLOF,d0
		move.w	d0,vposw(a0)
not_special:
		move.l	d2,vbstrt(a0)	; write VBSTRT, VBSTOP
		move.l  gb_LOFlist(a1),cop2ptr(a0)
		ENABLE
	endif

* If the lpen bit was set, rewrite beamcon0. We can't do the better test
* of bclr.l #13,d4, as d4 may be garbage. This happens during boot up, where
* there is no current_monitor.
	btst.b	#3,gb_system_bplcon0+1(a1)	; lpen set?
	beq.s	no_lpen_here
	btst.b	#GFXB_HR_AGNUS,gb_ChipRevBits0(a1)
	beq.s	no_lpen_here
	bclr.l	#13,d4
	move.w	d4,beamcon0(a0)
no_lpen_here:
	movem.l	(sp)+,d2/d4

	lea	_ciab,a6
	and.b	#$7f,ciacrb(a6)
	clr.l	d0
	move.b	d0,ciatodhi(a6)
	move.b	d0,ciatodmid(a6)
	move.b	vhposr(a0),ciatodlow(a6)	* start timer
	or.b	#$80,ciacrb(a6)			* back to alarm mode

***********************************************************************
* _bobvblank(gfxbase)
*              A1
*
* This function causes any pending beam synced blits to occur
* as soon as possible.  What happens is we add a beam synced
* blit, but sometime between the time it is added, and the time
* the blitnode is serviced, a VBLANK has occured.  Once true, we
* get caught up on all pending beam synced blits by marking that
* we should just ignore the request for beam syncing.
* 
* IF we don't get caught up, asyncronous beam synced blits can
* be added at a rate rapid enough that OwnBlitter() can hold
* off indefinitely.  Actually this is not a new problem, but we
* would make it worse.  Also its unlikely that someone asking for
* a beam synced blit means he wants it to happen many frames later.
* Infact historically the BOB code has done catch up code!
*
* NOTE -- VBLANK, and BLIT interrupts are equal CPU priority, so there is
* no race condition.  VBLANK can access the qblit list safely
* without causing any list processing problems in the BLIT interrupt.
*
***********************************************************************

	;-- check for empty qsbblit list; if so, bail

	move.l	gb_blthd(a1),d0
	beq.s	bobvb_emptylist

	;-- check if the current node is waiting for a timer
	;-- event.  Force one ASAP if so.  Also prevents
	;-- waiting forever if the monitor changed suddenly
	;-- such that the waiting position is invalid.

	move.l	d0,a0

	bclr	#GBFLAGSB_TIMER,gb_Flags+1(a1)	;atomic flag clear/test
	beq.s	bobvb_clearlist

	;-- Don't bother disabling.  If the timer goes off later
	;-- the flag bit above is clear, so the blitter interrupt
	;-- handler won't get a false ring.  Even if it did,
	;-- we are protected within the interrupt code.  Whenever
	;-- it does go off (and it really should have gone off
	;-- already), we will disable further ALRM interrupts.

	move.w  #BITSET+INTF_BLIT,_intena	;not needed, but doesn't hurt
	move.w  #BITSET+INTF_BLIT,_intreq

	;-- bitclear high bit of bn_beamsync value for any nodes
	;-- on qblit list; the BLIT interrupt will therefore not
	;-- bother sending off any timer waits for these nodes.

bobvb_clearlist:
	bclr	#7,bn_beamsync(a0)		;clear high bit of word
	move.l	(a0),d0				;bn_n(a0) (CC affected)
	move.l	d0,a0				;does not affect CC
	bne.s	bobvb_clearlist

bobvb_emptylist:

*	check for those Wait_TOF 
	clr.b	gb_VBlank(a1)
	move.l	gb_ExecBase(a1),a6		* get ExecBase
	move.l  d2,-(sp)
	lea	gb_DBList(a1),a0			; prev node
	move.l	(a0),d2
	beq.s	no_dbscan
	move.l	a1,-(a7)

scan_next:
	move.l	d2,a1
	sub.l	#1,dbi_Count1(a1)		; decrement frame counter
	bne.s	no_wakeup
	
	move.l	a0,-(a7)
	add.l	#dbi_SafeMessage,a1
	jsr	_LVOReplyMsg(a6)
	move.l	(a7)+,a0
	move.l	d2,a1
	move.l	(a1),d2				; prev->next=cur->next
	move.l	d2,(a0)
	bne.s	scan_next
	bra.s	done_dbscan
no_wakeup:
	move.l	d2,a0
	move.l	(a0),d2
	bne.s	scan_next
	
done_dbscan:
	move.l	(a7)+,a1

no_dbscan:
	move.l	gb_TOF_WaitQ(a1),a1
	move.l	(a1),d2					* get Succ
	if <>
		repeat
			moveq	#$0,d0			* now stores bit # in LN_PRI!
			move.b	LN_PRI(a1),d0
			bset	d0,d0
			move.l	LN_NAME(a1),a1
			JSRFAR	Signal
			move.l	d2,a1			* recall next ptr
			move.l	(a1),d2
		until =
	endif
; now, let's scan our db list, decrementing frame counters and deleting and singalling expired nodes.

	move.l	(sp)+,d2
	clr.l	d0
*	returns with ccr set to zero
	rts

	xdef	_BLTBYTES,bltbytes

; int __asm BLTBYTES(register __d0 UWORD xmin,register __d1 UWORD xmax);
_BLTBYTES:
bltbytes:
	add.w	#16,d1
	and.w	#$FFF0,d1
	and.w	#$FFF0,d0
	sub.w	d0,d1
	asr.w	#3,d1		* convert to bytes per row
	ext.l	d1
	move.l	d1,d0		* return in d0
	rts

    xdef    _timasm
    xref    _AbleICR
***********************************************************************
* _timasm(gfxbase)
*           A1
*
* Cause a blitter interrupt when beamsync timer expires; of note,
* this is a level 6 interrupt, so it is important to return ASAP
* to minimize impact on lower priority interrupts (in particular,
* serial interrupts).
*
* A6 - scratchable
***********************************************************************

_timasm:

	;-- disable CIAB ALARM interrupts

	move.l	gb_cia(a1),a6
	move.l	a1,-(sp)
	moveq	#CIAICRF_ALRM,d0
	jsr	_LVOAbleICR(a6)
	move.l	(sp)+,a1

	;-- if the timer bit is not set in in gb_Flags, then this
	;-- is a false alarm, or one that has already been handled
	;-- by the vertical blank server.

	bclr	#GBFLAGSB_TIMER,gb_Flags+1(a1)	;atomic flag clear/test
	beq.s	falsealarm

	;-- make a blitter interrupt

	move.w  #BITSET+INTF_BLIT,_intena	;not needed, but doesn't hurt
	move.w  #BITSET+INTF_BLIT,_intreq

falsealarm:
	rts


***********************************************************************
* _bltasm(gfxbase,_custom)
*           A1      A0
*
* Service next blit node on queue.  Special case handling for
* beamsync'ed nodes -- may start beam synced timer, and exit.
*
* D0/D1/A0/A1/A5/A6 - scratchable
*
* NOTE - VBLANK, and BLITTER interrupts run at the same priority, so
* there is no race condition regarding VBLANK clearing the high bit
* of bn_beamsync.
***********************************************************************


	;-- The LASTBLIT flag means the node is done, and has started
	;-- a blit.  Once this is done we can call their cleanup routine
	;-- and remove this node from the qblit list.  Waiting for the
	;-- interrupt is much beter than busy waiting in the interrupt

bltasm_lastblit:

	;-- If more to do, then just exit; will call back node again
	;-- next blitter interrupt.  If not more to do, wait for
	;-- next interrupt indicating last blit is done.

	bne.s	bltasm_busy

	bset	#GBFLAGSB_LASTBLIT,gb_Flags+1(a6)

	;-- This code is intended to fix anyone who does not start
	;-- a blit!! ... make sure we actually get a blitter interrupt!
	;-- If they started a really short blit which is already done,
	;-- thats ok too.  The important thing is we need to get back
	;-- to our blitter interrupt handler.

bltasm_busy:
	tst.b	_dmaconr			;bad agnus??

	btst	#DMAB_BLTDONE-8,_dmaconr
	bne.s	bltasm_continue

	move.w	#BITSET!INTF_BLIT,_intreq

bltasm_continue:
	rts

_bltasm::
	;-- DO NOT MODIFY A0 without making sure that it points to _custom
	;-- when calling the bltnode function!

	move.w	#INTF_BLIT,intreq(a0)	;clear blitter interrupt

	;-- screen interrupts when QBlit is not active

	move.l	gb_blthd(a1),d0
	beq.s	bltasm_continue

	btst	#QBOWNERn,gb_Flags+1(a1)
	beq.s	bltasm_continue

	move.l	a1,a6
	move.l	d0,a1

	;-- Make sure blitter is truly done.  If not, allow
	;-- any pending VBLANK to be serviced.  If no pending
	;-- VBLANK, wait for this blit to finish.  In actual
	;-- use this case is rarely going to be true.  An
	;-- example of when it might be true is someone calling
	;-- DisownBlitter after having done more than one blit.
	;-- DisownBlitter might not have set INTB_BLIT in intreq,
	;-- but would have set the flag in intena.

	tst.b	dmaconr(a0)			;bad agnus??

bltasm_waitblit:

	btst	#DMAB_BLTDONE-8,dmaconr(a0)
	beq.s	bltasm_nextblit

	;-- fake access to ciaa PRA gets CPU off the bus for a short while

	tst.b	_ciaa
	tst.b	_ciaa

	;-- check for VBLANK pending; blitter is in busy state

	move.w	intreqr(a0),d0
	and.w	#INTF_VERTB,d0
	beq.s	bltasm_waitblit
	bra.s	bltasm_busy

bltasm_nextblit:

	tst.b	dmaconr(a0)			;bad agnus??

	;-- Check if this node had previously indicated it is done.
	;-- If so, do cleanup routine (?) and remove it

	bclr	#GBFLAGSB_LASTBLIT,gb_Flags+1(a6)
	bne.s	bltasm_cleanup

	;-- determine if current node is waiting for a preferred beam pos

	move.w	bn_beamsync(a1),d0
	bpl.s	bltasm_immediate

	;-- if so, clear marker (first time through - will not be true
	;-- again for subsequent calls)

	bclr	#15,d0
	move.w	d0,bn_beamsync(a1)

	;-- check if we need to start timer, or blit immediately

	move.l	vposr(a0),d1
	lsr.l	#08,d1
	and.w	#$07FF,d1

	;-- if beam position is > waiting position, start beam timer
	;-- else just start blit now

	cmp.w	d0,d1
	bcs.s	start_timer		;beam < desired position?

	;-- call back current blitnode (_custom is already in A0)

bltasm_immediate:
	move.l	bn_function(a1),a5
	jsr	(a5)

	bra	bltasm_lastblit

	;-- **********************************************************

bltasm_cleanup:		

	;-- This node is done; cleanup routine specified??

	btst	#CLEANMEn,bn_stat(a1)
	beq.s	bltasm_nocleanup

	move.l	bn_cleanup(a1),a5
	jsr	(a5)

	move.l	gb_blthd(a6),a1		;refresh bltnode pointer

bltasm_nocleanup:

	;-- Remove blitnode, and DisownBlitter if this is the last
	;-- qblit node.  We are responsible for clearing the QBOWNER
	;-- flag, and disabling blitter interrupt.s

	
	move.l	(a1),gb_blthd(a6)	;Remove()

	bne.s	bltasm_qbactive

	and.w	#(~(QBOWNER)),gb_Flags(a6)
	move.w	#INTF_BLIT,_intena

	jmp	_LVODisownBlitter(a6)	;rts

bltasm_qbactive:
		
	;-- Give any waiters a break, but if no waiters, just cause another
	;-- blit interrupt.  If waiters, then DisownBlitter such that
	;-- the waiter is Signaled.  Once the waiter (if any) calls
	;-- DisownBlitter(), the QBLIT code is given ownership again.
	;
	;-- Because we are running from an interrupt, neither DISABLE or
	;-- FORBID is needed.

	tst.w	gb_BlitLock(a6)
	beq.s	bltasm_nextqblit	;if 0, QBLIT is only owner

	clr.l	gb_blthd(a6)

	and.w	#(~(QBOWNER)),gb_Flags(a6)
	move.w	#INTF_BLIT,_intena

	jsr	_LVODisownBlitter(a6)	;preserves all registers

	;-- restore gb_blthd() for next call to DisownBlitter

	move.l	(a1),gb_blthd(a6)
	addq.w	#1,gb_BlitLock(a6)	;restore my owner count
	rts

	;-- Cause a blitter interrupt.  
	;-- We could loop back, but we want to give any pending VBLANK
	;-- a chance to run.  This code is also smaller (though
	;-- slower than) loop back code.

bltasm_nextqblit:

	move.w	#BITSET!INTF_BLIT,_intreq
	rts
		

***********************************************************************
* start_timer(gfxbase,bltnode,beampos)
*                A6      A1     D0
*
* Start beam sync timer, and bail out; interrupt will wake us up
*
* NOTE - this function is only called from the blitter interrupt
* code now, so there is no race condition with VBLANK resetting the
* counters.  Blitter interrupts and VBLANK run at the same priority.
***********************************************************************
start_timer:

	;-- if genloc, then this counter is halfassed

	btst	#1,gb_system_bplcon0+1(a6)	;isn't there an EQUATE?
	beq.s	start_tnogenlock

	lsr.w	#1,d0				;divide by 2

start_tnogenlock:
	lea	_ciab,a0
	move.l	a6,a1

	movem.l	d0/a0/a1,-(sp)

	moveq	#-1,d0

	;-- We want to clear any pending CIA interrupt; presumably
	;-- the last CIA-B ALRM interrupt has triggered, and is
	;-- therefore also disabled.  But if not, thats ok too.
	;
	;-- The important thing to note is that even if the CIA
	;-- interrupt is disabled, it can still be pending.  So
	;-- what we need to do is clear pending in a state where
	;-- we do not get a false ring due to some previous value
	;-- in the ALRM.  To do this, we set the ALRM value to
	;-- something ridiculous.  We will also set the full
	;-- 24 bit ALRM value here, even though that it should
	;-- not be needed either (but it doesn't hurt to play it safe)

	move.b	#00,ciatodhi(a0)		;not really needed
	move.b	d0,ciatodmid(a0)
	move.b	d0,ciatodlow(a0)
		
	;-- clear pending CIA interrupt (if any?) 

	move.l	gb_cia(a6),a6
	moveq	#CIAICRF_ALRM,d0
	jsr	_LVOSetICR(a6)

	;-- mark that this node is waiting for a beam sync alarm

	movem.l	(sp)+,d0/a0/a1

	bset	#GBFLAGSB_TIMER,gb_Flags+1(a1)

	;-- ******************************************************
	;-- * What happens if the beam is already at, or passed us
	;-- * by the time we do the new counter load??  I know I'll
	;-- * take care of the problem by next VBLANK, but ideally
	;-- * would we check for this case in DISABLE, and try to
	;-- * compensate (e.g., beam postion +1, or +2??)
	;-- ******************************************************

	;-- load new beam counter value

	ror.w	#8,d0
	move.b	d0,ciatodmid(a0)		;hold
	rol.w	#8,d0
	move.b	d0,ciatodlow(a0)		;and latch value

	;-- enable CIAB TOD interrupts
		
	move.l	#(CIAICRF_SETCLR!CIAICRF_ALRM),d0
	jsr	_LVOAbleICR(a6)
	rts

    xdef    _IDivS_ceiling
*   if there is a residue bump up the quotient
*   this probably only works for positive quotients
; int __asm IDivS_ceiling(register __d0 int a,register __d1 int b);
_IDivS_ceiling:
    divs    d1,d0
    move.l  d0,d1
    ext.l   d0
    swap    d1
    if  d1.w
	addq    #1,d0
    endif
    rts

    xdef    _IDivS_round
*   rounded signed division
; int __asm IDivS_round(register __d0 int a, register __d1 int b);
_IDivS_round:
    add.l   d0,d0       * pre multiply by 2
    divs    d1,d0   * now divide it
    if  >
	add.w   #1,d0
    endif
    ext.l   d0
    asr.l   #1,d0           * now drop the round bit if unneeded
    rts



	xdef	_rotate
; UWORD __asm rotate(register __d0 UWORD data, register __d1 UWORD count);
*	rotates a UWORD right by n
*	rotate(data,n)
_rotate
	ror.w	d1,d0
	rts


	include	'exec/ables.i'
	xref	_intena

	INCLUDE "utility/hooks.i"
	xdef	_hookEntry

_hookEntry:
 	movem.l	a5/a6,-(sp)		; save library base
 	move.l	h_Data(a0),a6		; restore context
	move.l	h_SubEntry(a0),a5 ; fetch C entry point ...
	jsr	(a5)			; ... and call it
 	movem.l	(sp)+,a5/a6		; restore library base
	rts				; and get the hell outa here...!


my_name
*	dc.b    'The Amiga Wizards bring this power to you',10,0

    data
	xdef	gfx_end
gfx_end:    dc.b    0       * this will go somewhere at the end
	cnop	0,2

	section	graphics2
**************************************************************************

* this code added Oct 23 1991 for 2.05 A300 build - better detection of
* non-ECS denise (esp. on A1000).

_intenar	equ	$dff01c
_intenaw	equ	$dff09a
_deniseid equ	$dff07c

_get_denise_id:
	move.w	d2,-(sp)
	move.l	$4,a1
	DISABLE	a1,NOFETCH

	move.w	_intenar,-(sp)
	move.w	#$0009,_intenaw
	move.w	_deniseid,a0
	move.w	#16,d1
	moveq	#0,d0
1$:	tst.w	_intenar
	move.w	_deniseid,d0
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	tst.w	d5
	move.w	d0,d2
	and.w	#$000f,d2
	cmp.w	#$0009,d2
	beq.s	no_id
	cmp.w	a0,d0
	bne.s	no_id
	dbra	d1,1$
	and.w	#$ff,d0
bye:	move.w	(sp)+,d1
	bset	#15,d1
	move.w	d1,_intenaw
	not.w	d1
	move.w	d1,_intenaw
	ENABLE	a1,NOFETCH
	move.w	(sp)+,d2
	rts

no_id:	moveq	#-1,d0
	bra.s	bye

; int __asm rectXrect(register __a0 struct Rectangle *l,register __a1 struct Rectangle *cr);
	xdef	_rectXrect,_crectXrect
_crectXrect:
	movem.l	4(a7),a0/a1
_rectXrect:
;    if ( (cr->MaxX < l->MinX) ||
;	 (cr->MinX > l->MaxX) ||
;	 (cr->MaxY < l->MinY) ||
;	 (cr->MinY > l->MaxY) ) return(FALSE);
;   else return(TRUE);
	moveq	#0,d0
	move.w	(a0)+,d1	; d1=minx
	cmp.w	ra_MaxX(a1),d1
	bgt.s	no_int
	move.w	(a0)+,d1	; d1=miny
	cmp.w	ra_MaxY(a1),d1
	bgt.s	no_int
	move.w	(a0)+,d1	; d1=maxx
	cmp.w	ra_MinX(a1),d1
	blt.s	no_int
	move.w	(a0)+,d1	; d1=maxy
	cmp.w	ra_MinY(a1),d1
	blt.s	no_int
	moveq	#-1,d0
no_int:	rts



*
*  result = DelayAndRead(address,number)
*
*	UWORD = DelayAndRead(UWORD *,ULONG)
*					(only 16 bits are used in the number)
*
*	This function reads the address given the number of times given
*	and returns the value of the last read.  This is usefull for doing
*	delays with respect to the custom chips.
*
*	The value returned is the next read.  So if you pass in a 1, it will
*	read it once and then read it again and return that value...
*
		XDEF	_DelayAndRead
_DelayAndRead:	move.l	4(sp),a0	; Get address...
		move.l	8(sp),d1	; Get count...
		moveq.l	#0,d0		; Clear d0...
DAR_Loop:	move.w	(a0),d0		; Read...
		dbra	d1,DAR_Loop	; Do it for count...
		rts			; We be done.


; void __asm GetRPBounds(register __a1 struct RastPort *rp,register __a0 struct Rectangle *rect);
_GetRPBounds::
	movem.l	a4/a5/d2-d7,-(sp)
	move.l	rp_Layer(a1),d0
	bne.s	yeslayer
yes_bm:	move.l	a0,a5			; save rectangle ptr
	clr.l	(a0)			; minx=miny=0
	move.l	rp_BitMap(a1),a0
	move.l	a0,a4
	moveq	#BMA_WIDTH,d1
	jsr	_LVOGetBitMapAttr(a6)
	subq.l	#1,d0
	move.w	d0,ra_MaxX(a5)
	move.l	a4,a0
	moveq	#BMA_HEIGHT,d1
	jsr	_LVOGetBitMapAttr(a6)
	subq	#1,d0
	move.w	d0,ra_MaxY(a5)
	movem.l	(sp)+,a4/a5/d2-d7
	rts
yeslayer:
	move.l	d0,a5			; layer
	tst.l	lr_SuperBitMap(a5)
	beq.s	not_super
	move.l	d0,a1
	add.l	#lr_SuperBitMap-rp_BitMap,a1
	bra.s	yes_bm

not_super:
	LOCKLAYER
	move.w	#32767,d0		; d0=minx
	move.w	d0,d1			; d1=miny
	move.w	#-32767,d2		; d2=maxx
	move.w	d2,d3			; d3=maxy
	move.l	lr_ClipRect(a5),a4
cr_lp:	cmp.w	#0,a4
	beq.s	no_more_crs
	tst.l	cr_lobs(a4)		; lobs?
	beq.s	got_cr
	tst.l	cr_BitMap(a4)		; if lobs & (bm=0), obscured.
	beq.s	next_cr
got_cr:
	movem.w	cr_MinX(a4),d4/d5/d6/d7	; fetch minx/miny/maxx/maxy
	sub.w	lr_MinX(a5),d4
	sub.w	lr_MinX(a5),d6
	add.w	lr_Scroll_X(a5),d4
	add.w	lr_Scroll_X(a5),d6

	sub.w	lr_MinY(a5),d5
	sub.w	lr_MinY(a5),d7
	add.w	lr_Scroll_Y(a5),d5
	add.w	lr_Scroll_Y(a5),d7

	cmp.w	d4,d0			; if minx>cr_minx, minx=cr_minx
	ble.s	no_updminx
	move.w	d4,d0
no_updminx:
	cmp.w	d6,d2			; maxx < cr_maxx?
	bge.s	no_updmaxx
	move.w	d6,d2
no_updmaxx:
	cmp.w	d5,d1
	ble.s	no_updminy
	move.w	d5,d1
no_updminy:
	cmp.w	d7,d3
	bge.s	no_updmaxy
	move.w	d7,d3
no_updmaxy:
next_cr:
	move.l	cr_Next(a4),a4
	bra.s	cr_lp
no_more_crs
	movem.w	d0-d3,(a0)
	UNLOCKLAYER
	movem.l	(sp)+,a4/a5/d2-d7
	rts

; UWORD * __asm CopyWords(register __a0 UWORD *source, register __a1 UWORD *dest, register __d0 int nwords);
_CopyWords::
	subq.l	#1,d0
1$:	move.w	(a0)+,(a1)+
	dbra	d0,1$
	move.l	a1,d0
	rts

; UBYTE __asm FindMSBSet(register __d1 UBYTE thisbyte);
_FindMSBSet:
	move.b	d1,d0
	beq.s	2$
	moveq	#7,d0
1$:
	asl.b	#1,d1
	dbcs	d0,1$
2$:
	rts

; ULONG __asm GetTagDataUser(register __d0 ULONG tagValue, register __d1 ULONG def, register __a0 struct TagItem *tagList);
	xref	_LVOGetTagData

_GetTagDataUser:
	or.l	#TAG_USER,d0
	move.l	a6,-(sp)
	move.l	gb_UtilBase(a6),a6
	jsr	_LVOGetTagData(a6)
	move.l	(sp)+,a6
	rts

; BOOL __asm IsAUCopIns(register __a0 struct ViewPort *vp, register __a1 struct CopList *cl);

_IsAUCopIns:
	moveq	#-1,d0		; result = TRUE
	move.l	vp_UCopIns(a0),a0	; we know this in non-NULL, as it is checked
					; before calling in mrgcop().
1$:
	cmp.l	ucl_FirstCopList(a0),a1
	beq.s	IsAUCopIns.
	move.l	ucl_Next(a0),d0
	move.l	d0,a0
	bne.s	1$
	moveq	#0,d0		; FALSE

IsAUCopIns.:
	rts

lname	dc.b	'layers.library',0

    end     stupid metacomco assembler
