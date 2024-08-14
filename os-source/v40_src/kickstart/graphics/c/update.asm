*******************************************************************************
*
*	$Id: update.asm,v 39.16 93/04/21 18:35:43 spence Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/interrupts.i'
    include 'exec/libraries.i'
    include '/monitor.i'
    include '/vp_internal.i'
    include '/copper.i'
    include '/gfx.i'
    include '/gfxbase.i'
    include '/view.i'
    include '/display.i'
    include 'hardware/intbits.i'
    include 'hardware/dmabits.i'
    include 'hardware/custom.i'

    section	graphics

    xdef _update_top_color
    xref _new_mode
    xref _check_genlock
    xref _get_bplcon3
    xref _get_bplcon2

*   downcoded from c 03.20.89 -- bart
*   update_top_color(mspc)    -- modifies copinit variables for current display
*   needs various hardcoded bit fields redefined as equ's in include files
*   does not return any specific value in d0
* NOTE this routine must be called while ActiViewCPRSemaphore is held!!!!

_update_top_color:

    move.l	gb_ActiView(a6),d0
    beq		utc_rts				; no view
    move.l	d0,a1
    move.l	v_ViewPort(a1),d1
    beq		utc_novp			; no viewport

utc_save:
    movem.l	a2/d0-d5,-(sp)	; save registers
    move.l	d1,a0
    move.l	a0,a2				; bestvp in a2
    move.w	#10000,d3			; bestdy in d3
    move.l	a0,(sp)				; preset stack

utc_loop:
    btst.b	#5,vp_Modes(a0)			; visible?
    bne.s	utc_next
    move.w	vp_DyOffset(a0),d2		; tempdy in d2
    jsr		_new_mode			; control modes in d0
    move.l	d0,d4				; tempmd in d4
    btst	#2,d0				; interlace?
    beq.s	utc_chksdbl
    asr.w	#1,d2
    bra.s	utc_noshift
utc_chksdbl:
    btst	#3,d0				; doublescan?
    beq.s	utc_noshift
    asl.w	#1,d2

utc_noshift:
    cmp.w	d2,d3	
    ble.s	utc_next			; tempdy <= bestdy

utc_newbest:

    move.l	(sp),a2				; new bestvp
    move.w	d2,d3				; new bestdy
    move.l	d4,d5				; new bestmd

utc_next:
    move.l	(sp),a0				; lastvp
    move.l	vp_Next(a0),a0			; nextvp
    move.l	a0,(sp)				; preset stack
    bne.s	utc_loop			; more to check...

utc_getcm:

    clr.l	4(sp)				; clear second stack arg
    move.l	vp_ColorMap(a2),(sp)		; cm arg to check_genlock
    beq		utc_nocm

utc_cm:

    jsr		_check_genlock			; gen in d0

    move.l	(sp),a0				; cm
    move.l	cm_ColorTable(a0),a0		; cm->colortable
    move.w	(a0),d1				; cm->colortable[0]

    btst	#5,d5				; superhires?
    beq.s	utc_notsuper
    btst.b	#GFXB_AA_MLISA,gb_ChipRevBits0(a6)
    bne.s	utc_notsuper
    bsr		fix_super

utc_notsuper:

    move.w	#color,d2
    swap	d2
    move.w	d0,d2
    or.w	d1,d2		
    move.l	gb_copinit(a6),a1
    lea		copinit_diagstrt+4(a1),a1
    move.l	#((bplcon3<<16)|$0c00),d3
    move.l	(sp),a0				; colormap
    movem.l	d0/d1/a1,-(sp)
    jsr		_get_bplcon3			; get border genloc stuff
    or.w	d0,d3
    movem.l	(sp)+,d0/d1/a1
    move.l	32(sp),d4			; mspec
    beq.s	1$
    move.l	d4,a0
    btst.b	#3,ms_BeamCon0+1(a0)		; BLANKEN
    beq.s	1$
    or.b	#$1,d3				; EXTBLNKEN
1$:
    move.l	d3,(a1)+			; bank 0
    move.l	d2,(a1)+
    move.l	(sp),a0				; colormap
    move.b	cm_Type(a0),d2
    cmp.b	#COLORMAP_TYPE_V39,d2
    bne.s	utc_notsuperlow			; use the same bits
    btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
    beq.s	utc_notsuperlow
    move.l	cm_LowColorBits(a0),a0
    move.w	(a0),d1
    btst	#5,d5
    beq.s	utc_notsuperlow

    bsr		fix_super

utc_notsuperlow:
    move.w	#color,d2
    ; do we want a COLOUR0, BPLCON3 or two NOPs?
    btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
    bne.s	1$
    swap	d3
    move.w	#$1fe,d3
    move.w	#$1fe,d2
    swap	d3
1$:
    swap	d2
    move.w	d0,d2
    or.w	d1,d2
    or.w	#$200,d3			; bank 0 loct 1
    move.l	d3,(a1)+
    move.l	d2,(a1)+

utc_mspc:

    move.l	32(sp),d0			
    beq		utc_delay			; delay diwstrt...

    move.l	gb_copinit(a6),a2
*    btst.b	#1,gb_ChipRevBits0(a6)		; ecs denise?
*    beq		utc_exit			; nothing more to do...

    move.l	d0,a1				; a1 == mspc			
						; d1 == bplcon0
						; d2 == bplcon2
						; d3 == bplcon3
utc_bplcon0:

    move.w	#bplcon0,d4
    swap	d4
    move.w	gb_system_bplcon0(a6),d4
    move.l	gb_ActiView(a6),a0
    move.w	v_Modes(a0),d0
    and.w	#4,d0				; view interlaced?
    or.w	d0,d4
    btst	#5,d5				; vp superhires?
    beq.s	utc_nosuperbpl
    or.w	#$40,d4				; set super in bplcon0
utc_nosuperbpl:
    move.w	d5,d0
    and.w	#V_HAM+V_DUALPF+V_HIRES,d0	; misc bits
    or.w	d0,d4				; basic bplcon0
    btst.b	#1,gb_ChipRevBits0(a6)		; ecs denise?
    bne.s	utc_bplcon2
    move.l	d4,copinit_diagstrt(a2)		; stuf bplcon0 in init list
    bra		utc_exit			; nothing more to do.

utc_bplcon2:
    or.w	#1,d4				; set ECSENA - we will be writing
    						; to bplcon3
    move.l	(sp),a0
    moveq.l	#0,d2
    move.w	#bplcon2,d2
    swap	d2
    jsr		_get_bplcon2
    move.w	d0,d2
    move.l	d4,d1

    moveq.l	#0,d0
    move.w	d5,d0
    and.w	#V_PFBA,d0			; playfield 2 priority?
    or.w	d0,d2				; basic bplcon2

utc_cmtype:

    move.b	cm_Type(a0),d0			; test version
    beq.s	utc_nobdtrans			; && skip V1.2

utc_cmflags:

    move.b	cm_Flags(a0),d0
    btst	#0,d0
    beq.s	utc_nomaptrans
    or.w	#$400,d2			; set zdcten

utc_nomaptrans:

    btst	#1,d0
    beq.s	utc_nobdtrans
    or.w	#$800,d2			; set zdbpen

    swap	d0
    move.b	cm_TransparenyPlane(a0),d0
    moveq	#12,d4
    lsl.w	d4,d0
    or.w	d0,d2				; set zdbplanes
    swap	d0
	
utc_nobdtrans:

    moveq	#0,d3
    move.w	#bplcon3,d3
    btst.b	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
    bne.s	1$
    move.w	#$1fe,d3			; NOP for ECS and A
1$:
    swap	d3				; basic bplcon3
    move.w	ms_BeamCon0(a1),d4
    btst	#3,d4				; csblank?
    beq.s	utc_nocsblank
    or.w	#1,d1				; use bplcon3...
    or.w	#1,d3				; and extblnken...

utc_nocsblank:

	move.w	gb_SpriteFMode(a6),d0
	btst.b	#MSB_DOUBLE_SPRITES,ms_Flags+1(a1)
	beq.s	1$
	or.w	#FMF_SSCAN2,d0
1$:	move.w	d0,copinit_fm0+2(a2)
    move.l	d1,copinit_diagstrt(a2)		; stuf bplcon0 in init list
    move.l	d2,copinit_bplcon2(a2)		; stuf bplcon2 in init list
    jsr		_get_bplcon3				; d0=bplcon3
    or.w	d0,d3
    move.l	d3,copinit_diagstrt+20(a2)	; stuf bplcon3 in init list
utc_bpldone:

    bra.s	utc_delay	

utc_nocm:

    moveq.l 	#0,d2
    move.w	#bplpt,d2
    swap	d2
    move.l	gb_copinit(a6),a0
    lea		copinit_diagstrt+4(a0),a1
    move.l	d2,(a1)+
    move.l	d2,(a1)+
    move.l	d2,(a1)+
    move.l	d2,(a1)		; kill top color in init list

utc_delay:

* restore the blank line in copinit. This may have been overriden by the
* no_viewport fix below. BUT, not for 'A' machines. 

    btst.b	#GFXB_HR_DENISE,gb_ChipRevBits0(a6)
    beq.s		utc_exit
    move.w	#diwstrt,d0
    swap	d0
    move.w	#$0181,d0
    move.w	#diwstop,d1
    swap	d1
    move.w	#$0281,d1
    move.l	gb_copinit(a6),a0
    move.l	d0,copinit_diwstart(a0)		; stuf diwstrt in init list
    move.l	d1,copinit_diwstart+4(a0)	; stuf diwstop in init list

utc_exit:

    movem.l	(sp)+,a2/d0-d5	; restore registers

utc_rts:

	btst	#GFXB_AA_LISA,gb_ChipRevBits0(a6)
	bne.s	no_hackhack
; set the extra color00 writes to nops so as to not fool
; copper list pokers. The bplcon3 instructions in AA copinit should be NOPs from
; above.
	move.l	gb_copinit(a6),a0
	move.w	#$1fe,16+copinit_diagstrt(a0)

no_hackhack:
    rts

utc_novp:
	
*    view with no viewport" are there old style copper lists???
*    when such old style copper lists are loaded we can sniff them out
*    by looking at first bplcon0 having a depth of zero, or by the lack
*    of finding a diwhigh: the latter is preferable, but the first is faster
*
*    (in the interests of space and speed, check code removed for 37.2 release)
*
*    btst.b	#1,gb_ChipRevBits0(a6)		; ecs denise?
*    beq.s	utc_rts				; nothing more to do...
*
*    move.l	v_LOFCprList(a1),d0		; must check for old style lists
*    beq.s	utc_rts
*    move.l	d0,a0
*    move.w	crl_MaxCount(a0),d1
*    beq.s	utc_rts
*    move.l	crl_start(a0),d0
*    beq.s	utc_rts
*    move.l	d0,a0
*    bra.s	1$
*2$  move.l	(a0)+,d0		
*    and.l	#$FFFF7010,d0
*    cmp.l	#$01000000,d0
*    beq.s	3$
*1$  dbra	d1,2$
*    bra.s 	utc_rts
*
*   found old style lists, may kludge display window open for showanim
*

3$  move.l	gb_copinit(a6),a0
    lea.l	copinit_diwstart(a0),a0
    move.w	#diwstrt,(a0)+
    move.w	#$14ff,(a0)+
    move.w	#diwhigh,(a0)+
    move.w	#$0000,(a0)
    bra.s 	utc_rts

fix_super:
    btst	#GFXB_AA_LISA,gb_ChipRevBits0(a6)   
    bne.s	fix_super.		; don't fudge the colours for AA
    and.w	#$ccc,d1
    move.w	d1,d2	
    lsr.w	#2,d2
    or.w	d2,d1				; funny hardware

    move.w	d0,d2	
    lsr.w	#1,d2
    or.w	d2,d0		
fix_super.:
    rts

    end   
