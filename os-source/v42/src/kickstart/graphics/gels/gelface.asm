*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gelface.asm,v 37.0 91/01/07 15:28:25 spence Exp $
*
*	$Locker:  $
*
*	$Log:	gelface.asm,v $
*   Revision 37.0  91/01/07  15:28:25  spence
*   initial switchover from V36
*   
*   Revision 33.2  90/07/27  16:38:47  bart
*   id
*   
*   Revision 33.1  90/03/28  09:20:30  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:23:01  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*********************************************************************
* realworld (application program) calls             		    *
*   --  library RAM-based synchronizer routine (rjlib.s), which     *
*       loads the registers with the correct arguments, then calls  *
*   --  dogfn, which vectors to                                     *
*   --  THESE interface routines, which push registers then call    *
*   --  the real code                                               *
*********************************************************************


	section	graphics
    xdef    _AddBob
    xref    __AddBob
*********************************************************************
*                                                                   *
*   _AddBob(bob, RPort, GfxBase)                                    *
*           a0   a1     a6                                          *
*                                                                   *
*********************************************************************
_AddBob:
    move.l  a1,-(sp)        * pointer to RPort
    move.l  a0,-(sp)        * pointer to Bob structure
    jsr __AddBob            * call C routine
    addq.l #8,sp           * remove arguments from stack
    rts

    xdef    _AddVSprite
    xref    __AddVSprite
*********************************************************************
*                                                                   *
*   _AddVSprite(VSprite, RPort, GfxBase)                            *
*               a0    a1     a6                                     *
*                                                                   *
*********************************************************************
_AddVSprite:
    move.l  a1,-(sp)        * pointer to RPort
    move.l  a0,-(sp)        * pointer to VSprite structure
    jsr __AddVSprite        * call C routine
    addq.l #8,sp           * remove arguments from stack
    rts

    xdef    _DoCollision
    xref    __DoCollision
*********************************************************************
*                                                                   *
*   _DoCollision(RPort, GfxBase)                                    *
*                a1     a6                                          *
*                                                                   *
*********************************************************************
_DoCollision:
    move.l  a1,-(sp)        * pointer to RPort
    jsr __DoCollision       * call C routine
    addq.l  #4,sp           * remove argument from stack
    rts

    xref    __DrawGList
    xdef    _DrawGList
*********************************************************************
*                                                                   *
*   _DrawGList(RPort, VPort, GfxBase)                               *
*              a1     a0     a6                                     *
*                                                                   *
*********************************************************************
_DrawGList:
    move.l  a0,-(sp)        * pointer to ViewPort
    move.l  a1,-(sp)        * pointer to RastPort
    jsr __DrawGList         * call C routine
    addq.l #8,sp           * remove arguments from stack
    rts

    xdef    _InitGels
    xref    __InitGels
*********************************************************************
*                                                                   *
*   _InitGels(head, tail, GelsInfo, GfxBase)                        *
*             a0    a1    a2        a6                              *
*                                                                   *
*********************************************************************
_InitGels:
    move.l  a2,-(sp)        * pointer to GelsInfo
    move.l  a1,-(sp)        * pointer to dummy tail
    move.l  a0,-(sp)        * pointer to dummy head
    jsr __InitGels          * call C routine
    lea 12(sp),sp           * remove arguments from stack
    rts

    xdef    _InitMasks
    xref    __InitMasks
*********************************************************************
*                                                                   *
*   _InitMasks(VSprite)                                             *
*             a0                                                    *
*                                                                   *
*********************************************************************
_InitMasks:
    move.l  a0,-(sp)        * pointer to VSprite
    jsr __InitMasks         * call C routine
    addq.l  #4,sp           * remove argument from stack
    rts

    xdef    _RemIBob
    xref    __XRemIBob
*********************************************************************
*                                                                   *
*   _RemIBob(bob, RPort, VPort, GfxBase)                            *
*            a0   a1     a2     a6                                  *
*                                                                   *
*********************************************************************
_RemIBob:
    move.l  a2,-(sp)        * pointer to ViewPort
    move.l  a1,-(sp)        * pointer to RastPort
    move.l  a0,-(sp)        * pointer to bob
    jsr __XRemIBob          * call C routine which calls __RemIBob and cleans up
    lea 12(sp),sp           * remove arguments from stack
    rts

    xref    __RemVSprite
    xdef    _RemVSprite
*********************************************************************
*                                                                   *
*   _RemVSprite(VSprite)                                            *
*               a0                                                  *
*                                                                   *
*********************************************************************
_RemVSprite:
    move.l  a0,-(sp)        * pointer to VSprite structure
    jsr __RemVSprite        * call C routine
    addq.l  #4,sp           * remove arguments from stack
    rts

    xref    __SetCollision
    xdef    _SetCollision
*********************************************************************
*                                                                   *
*   _SetCollision(type, routine, GelsInfo, GfxBase)                 *
*                 d0    a0   a1       a6                            *
*                                                                   *
*********************************************************************
_SetCollision:
    move.l  a1,-(sp)        * pointer to GelsInfo
    move.l  a0,-(sp)        * pointer to collision routine
    move.l  d0,-(sp)        * collision type
    jsr __SetCollision      * call C routine
    lea 12(sp),sp           * remove arguments from stack
    rts

    xdef    _SortGList
    xref    __SortGList
*********************************************************************
*                                                                   *
*   _SortGList(RPort, GfxBase)                                      *
*              a1     a6                                            *
*                                                                   *
*********************************************************************
_SortGList:
    move.l  a1,-(sp)        * pointer to RastPort
    jsr __SortGList         * call C routine
    addq.l  #4,sp           * remove argument from stack
    rts

    xdef    _AddAnimOb
    xref    __AddAnimOb
*********************************************************************
*                                                                   *
*   _AddAnimOb(obj, key, RPort, GfxBase)                            *
*              a0   a1   a2     a6                                  *
*                                                                   *
*********************************************************************
_AddAnimOb:
    move.l  a2,-(sp)        * pointer to RastPort
    move.l  a1,-(sp)        * pointer to animKey
    move.l  a0,-(sp)        * pointer to the object to be added
    jsr __AddAnimOb         * call C routine
    lea 12(sp),sp           * remove arguments from stack
    rts

    xdef    _Animate
    xref    __Animate
*********************************************************************
*                                                                   *
*   _Animate(key, port, GfxBase)                                    *
*            a0   a1    a6                                          *
*                                                                   *
*********************************************************************
_Animate:
    move.l  a1,-(sp)        * pointer to RastPort
    move.l  a0,-(sp)        * pointer to animation key
    jsr __Animate           * call C routine
    addq.l #8,sp           * remove arguments from stack
    rts

    xdef    _GetGBuffers
    xref    __GetGBuffers
*********************************************************************
*                                                                   *
*   _GetGBuffers(obj, port, dbuf)                                   *
*                a0   a1    d0                                      *
*                                                                   *
*********************************************************************
_GetGBuffers:
    move.l  d0,-(sp)        * double-buffer boolean
    move.l  a1,-(sp)        * pointer to RastPort
    move.l  a0,-(sp)        * pointer to animation object
    jsr __GetGBuffers       * call C routine
    lea 12(sp),sp           * remove arguments from stack
    rts

    xdef    _InitGMasks
    xref    __InitGMasks
*********************************************************************
*                                                                   *
*   _InitGMasks(obj)                                                *
*               a0                                                  *
*                                                                   *
*********************************************************************
_InitGMasks:
    move.l  a0,-(sp)        * pointer to animation object
    jsr __InitGMasks        * call C routine
    addq.l  #4,sp           * remove argument from stack
    rts

    xdef    _FreeGBuffers
    xref    _freegbuffers
*********************************************************************
*                                                                   *
*   _FreeGBuffers(obj, port, dbuf)                                   *
*                a0   a1    d0                                      *
*                                                                   *
*********************************************************************
_FreeGBuffers:
    move.l  d0,-(sp)        * double-buffer boolean
    move.l  a1,-(sp)        * pointer to RastPort
    move.l  a0,-(sp)        * pointer to animation object
    jsr _freegbuffers       * call C routine
    lea 12(sp),sp           * remove arguments from stack
    rts

	end
