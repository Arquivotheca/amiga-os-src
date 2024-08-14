*******************************************************************************
*
*	$Id: ClearMem.asm,v 39.0 91/08/21 17:24:38 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                      * Data type definitions

	section	graphics
    xdef    ClearMem
    xdef    _clearmem

_clearmem:
    move.l  4(sp),a0
    move.l  8(sp),d0
*   fall into rest of code

ClearMem:
* only trash d0/d1/a0
    move.w  d0,d1
    and.w   #$FFFC,d1
    sub.w   d1,d0
    asr.w   #2,d1
    sub.w   #1,d1
    if >=
clearlong:
		clr.l   (a0)+
		dbf     d1,clearlong
    endif
    if d0
		subq.b  #1,d0
clearbyte:
		clr.b   (a0)+
		dbf     d0,clearbyte
    endif
    rts
    
	end
