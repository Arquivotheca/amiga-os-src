********************************************************************************
*
*       $Id: animcontrol.a,v 1.1 93/03/17 16:47:49 peter Exp $
*
********************************************************************************

        opt p=68020

BPLCON3 equ     $dff106
COLOUR0 equ     $dff180

CMOVE   MACRO   ;reg,value
        movem.l d0/d1,-(sp)
        move.l  a2,a1                   ; a2 -> UCopList
        move.l  \1,d0
        move.l  \2,d1
        jsr     _LVOCMove(a6)
        move.l  a2,a1
        jsr     _LVOCBump(a6)
        movem.l (sp)+,d0/d1
        ENDM


        xdef    _LibAnimationControlA
        xref    _LVOGetRGB32
        xref    _LVONextTagItem
        xref    _LVOUDivMod32
        xref    _LVOAllocMem,_LVOFreeMem
        xref    _LVOUCopperListInit,_LVOCMove,_LVOCWait,_LVOCBump
        xref    _LVOObtainSemaphore,_LVOReleaseSemaphore
        xref    _GfxBase

        incdir  "include:"
        include 'exec/types.i'
        include 'exec/memory.i'
        include "animate.i"
        include 'utility/tagitem.i'
        include 'graphics/copper.i'
        include 'graphics/gfxbase.i'
        include 'graphics/view.i'

******* amiga.lib/LibAnimationControlA ****************************************
*
*   NAME
*       LibAnimationControlA -- Controls often-used animation tricks (V40)
*       LibAnimationControl -- varargs stub for AnimationControlA()
*
*   SYNOPSIS
*       Error = LibAnimationControlA(View, ViewPort, TagItems)
*       d0                           a0    a1        a2
*
*       ULONG LibAnimationControlA(struct View *, struct ViewPort *,
*                                  struct TagItem *);
*
*       Error = LibAnimaionControl(View, ViewPort, Tag1, ...)
*
*       ULONG LibAnimationControl(struct View *, struct ViewPort *, ULONG, ...);
*
*   FUNCTION
*       To control various animation and other effects usually used by games in
*       an OS friendly and future compatible manner.
*
*   INPUTS
*       View - The View
*       ViewPort - The ViewPort
*       TagItems - A pointer to an array of TagItems.
*
*   RESULT
*       Error - -1 if operation was succesful, else an error number.
*
*   TAGS
*       ACTAG_ColorRange (struct ColorRange *) - A pointer to an initialised
*             ColorRange structure, for smooth 12 bit (or 24 bit in AA machines)
*             colour fades.
*
*   NOTES
*       This is currently an amiga.lib stub, but will be implemented as a
*       graphics.library LVO in the future.
*
*       For ACTAG_ColorRange to work correctly, the ViewPort needs to have been
*       made already with MakeVPort().
*
*   BUGS
*
********************************************************************************

    STRUCTURE   StackStuff,0
        ULONG   stk_TagPtr
        ULONG   stk_View
        ULONG   stk_ViewPort
    LABEL       FRAME_SIZE

    STRUCTURE   CorPrivate,24
        ULONG   Wait
        ULONG   con3_hi
        ULONG   pen_hi
        ULONG   con3_lo
        ULONG   pen_lo
        ULONG   Red32_orig
        ULONG   Green32_orig
        ULONG   Blue32_orig
        ULONG   Spare
    LABEL       CorPrivate_SIZE

_LibAnimationControlA:
        movem.l d2-d7/a2-a6,-(sp)
;       move.l  _GfxBase,a6

* Use NextTagItem() to iterate through the taglist
        move.l  sp,a5
        sub.l   #FRAME_SIZE,sp
        move.l  a2,(a5)
        movem.l a0-a1,stk_View(a5)      ; save View and ViewPort
ac_loop:
        move.l  a6,d6                   ; save GfxBase
        move.l  gb_UtilBase(a6),a6
        move.l  a5,a0                   ; struct TagItem **
        jsr     _LVONextTagItem(a6)
        move.l  d6,a6
        tst.l   d0
        beq     Tags_Done
        move.l  d0,a0
        move.l  ti_Tag(a0),d0

        and.l   #~$80000000,d0
        subq.l  #1,d0
        bne.s   ac_loop

*********************************************************************************
*                                                                               *
*                                  ColorRange                                   *
*                                                                               *
*********************************************************************************


Do_ColorRange:
        move.l  ti_Data(a0),a4          ; a4 points to the array of ColorRange structures
        btst.b  #CORB_ANIMATE,cor_Flags+3(a4)
        beq     Init_ColorRange

        ; ****************************************************************
        ;
        ; here we animate a pre-built colour range by just modifying the
        ; copperlists.
        ;
        ; ****************************************************************

        move.l  a6,d7
        move.l  gb_ActiViewCprSemaphore(a6),a3
        move.l  a3,a0
        move.l  gb_ExecBase(a6),a6
        jsr     _LVOObtainSemaphore(a6)
        move.l  d7,a6

        move.l  stk_ViewPort(a5),a2
        move.w  vp_Modes(a2),d0
        and.w   #V_VP_HIDE,d0
        bne     colour_change_interm    ; VP is hidden.
        move.l  vp_ClrIns(a2),a2
        movem.l a4/a2,-(sp)
        move.l  cl_CopSStart(a2),d0
        beq.s   1$                      ; no copper list?
        move.l  d0,a2                   ; here is the first WAIT in the colour list.
        moveq   #-1,d2                  ; flag to disable updating the cache
        bsr.s   do_change_colours
        movem.l (sp),a4/a2
1$:
        move.l  cl_CopLStart(a2),d0
        beq.s   2$
        move.l  d0,a2
        moveq   #0,d2
        bsr.s   do_change_colours
2$:
        movem.l (sp)+,a4/a2
        bra     colour_change_interm

do_change_colours:
        ; now we know we are changing a AA copperlist

        moveq   #-2,d4                  ; FFFF FFFE terminator
        moveq   #12,d6                  ; to add to the copper pointer for each colour change
change_colours:
        cmp.l   #-1,cor_Pen(a4)
        beq.s   done_colour_anim
        btst.b  #CORB_MODIFY,cor_Flags+3(a4)
        beq.s   no_colour_change1
        move.l  con3_hi(a4),d5
        move.w  pen_hi(a4),d7

2$:     ; find a MOVE BPLCON3
        move.l  (a2)+,d3
        cmp.l   d3,d4
        beq.s   done_colour_anim                ; end of copperlist
        cmp.l   d3,d5
        bne.s   2$
        ; check it's the right one
        move.w  (a2),d3
        cmp.w   d3,d7
        bne.s   2$

3$:
        ; calculate the Hi colour bits
        move.w  cor_Red(a4),d0
        lsr.w   #4,d0
        move.b  cor_Green(a4),d0
        move.b  cor_Blue(a4),d1
        lsr.b   #4,d1
        bfins   d1,d0{28:4}
        move.w  d0,2(a2)                ; into the copperlist
        tst.w   d2
        bne.s   4$                      ; don't update the cache for the SHF
        move.w  d0,pen_hi+2(a4)         ; into the cache

4$:
        ; calculate the Lo colour bits
        move.w  cor_Red(a4),d0
        and.w   #$f00,d0
        move.b  cor_Green(a4),d0
        asl.b   #4,d0
        move.b  cor_Blue(a4),d1
        bfins   d1,d0{28:4}
        move.w  d0,10(a2)
        tst.w   d2
        bne.s   no_colour_change
        move.w  d0,pen_lo+2(a4)

no_colour_change:
        add.l   d6,a2
        lea     cor_SIZEOF(a4),a4
        bra.s   change_colours
no_colour_change1:
        addq.l  #4,a2
        bra.s   no_colour_change

done_colour_anim:
        rts

        ; ******************************************************************
        ;
        ; This code will update the intermediate copper list in
        ; vp->ClrIns
        ;
        ; ******************************************************************

colour_change_interm:
        ; now update the intermediate copperlists
        move.l  stk_ViewPort(a5),a1
        move.l  vp_ColorMap(a1),d0
        beq     no_colour_load
        move.l  d0,a0
        btst.b  #CMAB_NO_INTERMED_UPDATE,cm_AuxFlags(a0)
        bne.s   no_colour_load
        move.l  vp_ClrIns(a1),a0        ; for now, let's assume there are no
                                        ; other ClrIns. This is a bug, as ther
                                        ; may be real ClrIns in the list, and we
                                        ; may be looking at the wrong ones.
        move.l  cl_CopIns(a0),a0
8$:
        move.l  cor_Pen(a4),d0
        addq.l  #1,d0
        beq.s   no_colour_load
        btst.b  #0,ci_OpCode+1(a0)
        bne.s   6$                      ; a WAIT
        btst.b  #CORB_MODIFY,cor_Flags+3(a4)
        beq.s   9$
        lea     ci_SIZEOF(a0),a0        ; skip the bplcon3
        move.l  pen_hi(a4),ci_DestAddr(a0)
        lea     ci_SIZEOF*2(a0),a0      ; skip the PenHi and Con3
        move.l  pen_lo(a4),ci_DestAddr(a0)
7$:
        lea     cor_SIZEOF(a4),a4
6$:
        lea     ci_SIZEOF(a0),a0        ; skip the PenLo
        bra.s   8$
9$:
        lea     ci_SIZEOF*4(a0),a0
        lea     cor_SIZEOF(a4),a4
        bra.s   8$

no_colour_load:
        move.l  a3,a0
        move.l  a6,d7
        move.l  gb_ExecBase(a6),a6
        jsr     _LVOReleaseSemaphore(a6)
        move.l  d7,a6
        bra     ac_loop


        ; ***************************************************************
        ;
        ; This code initialises vp->ClrIns for the first time through
        ;
        ; ***************************************************************

Init_ColorRange:
        ; how many copper instructions do we need?
        move.l  a4,a0
        moveq   #0,d2                   ; d2 will count the colours
        move.l  d2,d3                   ; d3 will count the WAITs
        move.w  cor_WAIT_Y(a0),d7
        addq.w  #1,d7
2$
        move.l  cor_Pen(a0),d0
        addq.l  #1,d0
        beq.s   1$                      ; that's the count
        addq.l  #1,d2
        move.w  cor_WAIT_Y(a0),d1
        cmp.w   d1,d7
        beq.s   3$
        move.w  d1,d7
        addq.w  #1,d3
3$:
        lea     cor_SIZEOF(a0),a0
        bra.s   2$
1$:
        tst.w   d2
        beq     ac_loop                 ; early sanity check
        ; for AA, need 2 colour instructions, and 2 bplcon3 instructions per line
        asl.w   #2,d2
4$:
        add.w   d3,d2                   ; add the WAITs
        addq.w  #1,d2                   ; terminator

        ; allocate a CopList structure. Do this by building a UserCopperList,
        ; call UCopListInit(), and use the UCopList->FirstCopList;
        moveq   #ucl_SIZEOF,d0
        move.l  #(MEMF_CLEAR|MEMF_PUBLIC),d1
        move.l  gb_ExecBase(a6),a6
        jsr     _LVOAllocMem(a6)
        move.l  d6,a6
        tst.l   d0
        beq     FAILURE_1
        move.l  d0,a0
        move.l  d0,a2
        move.l  d2,d0
        jsr     _LVOUCopperListInit(a6)
        tst.l   d0
        beq     FAILURE_2
        move.l  d0,a3                   ; a3 -> CopList

        ; put the CopList in the ClrIns
        move.l  stk_ViewPort(a5),a1     ; get the ViewPort
        move.l  vp_ClrIns(a1),cl_Next(a3)
        move.l  a3,vp_ClrIns(a1)


        ; Look through the DspIns for the last bplcon3 value.
        moveq   #0,d2                   ; store bplcon3 here
        move.l  stk_ViewPort(a5),a0
        move.l  vp_DspIns(a0),a0
        move.w  cl_Count(a0),d7
        move.l  cl_CopIns(a0),a0
        bra.s   get_bplcon3.
get_bplcon3:
        move.w  (a0)+,d0                ; opcode
        move.l  (a0)+,d1                ; address and data
        cmp.b   #0,d0
        bne.s   get_bplcon3.
        swap    d1
        cmp.w   #(BPLCON3&$fff),d1
        bne.s   get_bplcon3.            ; not a MOVE
        swap    d1
        move.w  d1,d2                   ; store the bplcon3
get_bplcon3.:
        dbra.s  d7,get_bplcon3
        move.w  cor_WAIT_Y(a4),d7
        subq.w  #1,d7                   ; prime d7 for the WAIT flag
        and.w   #%0001110111111111,d2   ; save the bits of bplcon3 we don't want to touch
        move.w  d2,d4

        ; walk through the array list.

loop_array1:
        moveq   #0,d5           ; 0 = at start of line list, $80000000 = in a line, $8000 = restoring
loop_array:
        ; get the current RGB32 value of the pen
        moveq   #0,d0
        move.l  cor_Pen(a4),d0
        cmp.l   #-1,d0
        beq     end_colours
        btst.b  #CORB_RESTORE,cor_Flags+3(a4)
        beq.s   1$
        move.l  stk_ViewPort(a5),a0
        move.l  vp_ColorMap(a0),a0
        moveq   #1,d1
        lea     Red32_orig(a4),a1
        jsr     _LVOGetRGB32(a6)

1$:
        ; build the private copper list cache
        move.l  #COLOUR0,a3
        move.w  #(BPLCON3&$1fe),d6
        swap    d6
        move.w  cor_Pen+2(a4),d6
        and.w   #31,d6
        add.w   d6,d6
        add.w   d6,a3                   ; a3 = colour register address
        move.w  cor_Pen+2(a4),d6        ; calculate the colour bank number
        and.w   #~31,d6
        asl.w   #8,d6
        or.w    d4,d6                   ; store together
        move.l  d6,con3_hi(a4)
        or.w    #$0200,d6
        move.l  d6,con3_lo(a4)
        and.l   #$ffff,d6

build_colours:
        bset.l  #31,d5
        move.w  cor_WAIT_Y(a4),d0
        cmp.w   d0,d7
        beq.s   1$
        bclr.l  #31,d5
        move.w  cor_WAIT_Y(a4),d7       ; the line number
1$:
        moveq   #-2,d0
        swap    d0
        move.w  d7,d0                   ; build the WAIT
        asl.w   #8,d0
        or.w    #1,d0
        swap    d0
        move.l  d0,Wait(a4)
100$:
        ; build the colour registers
        move.b  cor_Blue(a4),d0
        move.b  d0,d2
        swap    d2
        move.b  d0,d2
        move.w  cor_Red(a4),d0
        move.b  cor_Green(a4),d1
        tst.w   d5
        bpl.s   10$
        move.b  Blue32_orig(a4),d0
        move.b  d0,d2
        swap    d2
        move.b  d0,d2
        move.w  Red32_orig(a4),d0
        move.b  Green32_orig(a4),d1
10$:
        moveq   #0,d3
        move.w  d0,d3
        and.w   #$0f00,d3
        move.b  d1,d3
        asl.b   #4,d3
        and.b   #$f,d2
        or.b    d2,d3
        swap    d3
        move.w  d0,d3
        lsr.w   #4,d3
        move.b  d1,d3
        and.b   #$f0,d3
        swap    d2
        lsr.b   #4,d2
        or.b    d2,d3
        move.w  d3,d0
        swap    d0
        move.w  a3,d0
        and.w   #$01fe,d0
        swap    d0
        move.w  d0,d2
        tst.w   d5
        bmi.s   101$
        move.l  d0,pen_hi(a4)
101$:
        move.l  d3,d0
        move.w  a3,d0
        and.w   #$01fe,d0
        swap    d0
        move.w  d0,d1
        tst.w   d5
        bmi.s   102$
        move.l  d0,pen_lo(a4)
102$:

        ; now build the copper list
        tst.l   d5
        bmi.s   3$
        move.l  a2,a1
        moveq   #0,d0
        move.w  d7,d0
        move.l  d1,-(sp)
        moveq   #0,d1
        jsr     _LVOCWait(a6)
        move.l  a2,a1
        jsr     _LVOCBump(a6)
        move.l  (sp)+,d1
        bclr.l  #31,d5
3$:
        and.w   #~$0200,d6              ; Hi bits
        CMOVE   #BPLCON3,d6

        CMOVE   a3,d2                   ; Hi Colours
        or.w    #$0200,d6               ; Lo bits
        CMOVE   #BPLCON3,d6
        CMOVE   a3,d1                   ; Lo colours

        ; done that one. Do we need to restore the pen?
        tst.w   d5
        bmi.s   next_colour
        btst.b  #CORB_RESTORE,cor_Flags+3(a4)
        beq.s   next_colour
        move.w  #$8000,d5
        addq.w  #1,d7
        bra     build_colours
next_colour:
        sub.w   d5,d5
        lea     cor_SIZEOF(a4),a4
        bra     loop_array

end_colours:
        move.w  #10000,d0
        move.l  #255,d1
        move.l  a2,a1
        jsr     _LVOCWait(a6)           ; CEND
        move.l  a2,a1
        jsr     _LVOCBump(a6)

        ; Free the UserCopList
        move.l  a2,a1
        moveq   #ucl_SIZEOF,d0
        move.l  a6,d6
        move.l  gb_ExecBase(a6),a6
        jsr     _LVOFreeMem(a6)
        move.l  d6,a6

        bra     ac_loop

Do_ScrollLines:
Do_RepeatLines:
Tags_Done:
        add.l   #FRAME_SIZE,sp
        moveq   #-1,d0
_AnimationControlA.:
        movem.l (sp)+,d2-d7/a2-a6
        rts

FAILURE_2:
        ; free the UCopList
        move.l  a2,a1
        moveq   #ucl_SIZEOF,d0
        move.l  gb_ExecBase(a6),a6
        jsr     _LVOFreeMem(a6)
FAILURE_1:
        ; not enough memory
        moveq   #AC_ERR_NoMem,d0
        bra.s   _AnimationControlA.

        end
