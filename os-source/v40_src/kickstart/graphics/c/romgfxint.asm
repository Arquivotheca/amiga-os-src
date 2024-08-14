*******************************************************************************
*
*	$Id: romgfxint.asm,v 39.3 93/05/06 12:00:07 chrisg Exp $
*
*******************************************************************************

* c code calling other code through vector table
* changed to use pragmas! these pragmas will be automagically added to the protos file.


* GRAPHICS library calls
* graphics intermodule calls throught vector table

	section	graphics


    xdef    _ADDICRVECTOR
    xref    _LVOAddICRVector
_ADDICRVECTOR:
    move.l  a6,-(sp)
    move.l  8(sp),a6
    movem.l 12(sp),d0/a1
    jsr     _LVOAddICRVector(a6)
    move.l  (sp)+,a6
    rts
 
    xdef    _ABLEICR
    xref    _LVOAbleICR
_ABLEICR:
    move.l  a6,-(sp)
    move.l  8(sp),a6
    move.l  12(sp),d0
    jsr     _LVOAbleICR(a6)
    move.l  (sp)+,a6
    rts



	end
