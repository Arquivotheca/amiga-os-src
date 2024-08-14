*******************************************************************************
*
*	$Id: BltClear.asm,v 42.0 93/06/16 11:12:06 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'                  * Data type definitions
	include 'exec/nodes.i'
	include 'exec/lists.i'
	include 'exec/interrupts.i'
	include 'exec/libraries.i'
	include 'hardware/custom.i'     * Amiga hardware registers
	include 'hardware/blit.i'               * Blitter constants
	include 'graphics/gfxbase.i'
	include 'submacs.i'                 * Subroutine/function macros
	include '/sane_names.i'           * sane names

	section	graphics
	xdef    _BltClear          * Define function entries as public symbols

	xref    _custom                     * extern struct AmigaRegs ioregs;
	xref    waitblitdone



	PAGE

******* graphics.library/BltClear *********************************************
* 
*   NAME   
* 
*	BltClear - Clear a block of memory words to zero.
* 
*   SYNOPSIS
*	BltClear( memBlock, bytecount, flags )
*		    a1         d0       d1
*
*	void BltClear( void *, ULONG, ULONG );
*
*   FUNCTION
*	For memory that is local and blitter accessible, the most
*	efficient way to clear a range of memory locations is
*       to use the system's most efficient data mover, the blitter.
*       This command accepts the starting location and count and clears
*       that block to zeros.
*
*   INPUTS
*	memBloc	- pointer to local memory to be cleared
*		  memBlock is assumed to be even.
*	flags	- set bit 0 to force function to wait until 
*		  the blit is done.
*		  set bit 1 to use row/bytesperrow.
*
*	bytecount - if (flags & 2) == 0 then
*				even number of bytes to clear.
*			else
*				low 16 bits is taken as number of bytes
*				per row and upper 16 bits taken as
*				number of rows.
*
*	This function is somewhat hardware dependent. In the rows/bytesperrow
*	mode (with the pre-ECS blitter) rows must be <- 1024. In bytecount mode 
*	multiple runs of the blitter may be used to clear all the memory.
*
*	Set bit 2 to use the upper 16 bits of the Flags as the data to fill 
*	memory with instead of 0 (V36).
*
*   RESULT
*	The block of memory is initialized.
* 
*   BUGS
* 
*   SEE ALSO
* 
******************************************************************************

*                               CLEAR BLITTER MEMORY BUFFER
_BltClear:

	move.l  d6,-(sp)                    * need a temporary register

	lea     _custom,a0                  * io = &ioregs;
	clr     d6                          * Can't use CLR on H/W registers

	OWNBLITTER

	WAITBLITDONE  a0                    * waitblitdone();

	move    d6,bltcon1(a0)          * so use move.w instead
	move    d6,bltmdd(a0)           * io->bltcon1 = io->bltmdd = 0;
	move    #DEST+ANBNC+ANBC+ABNC+ABC,bltcon0(a0)	* A->D
	btst	#2,d1
	if <>
		swap	d1
		move.w	d1,adata(a0)	; use upper word
		swap	d1
	else
		move.w	d6,adata(a0)	; use #0
	endif
	moveq	#-1,d6
	move.l	d6,fwmask(a0)		; get lwmask also
	move.l  a1,bltptd(a0)           * io->bltptd = (UBYTE *) a;

	btst    #1,d1                       * if (flags & 2)
	if <>
		if #$4000000 < d0.l
			bra.s too_big
		endif
		if #$80<d0.w
*			too many BytesPerRow
* use other BltClear mode
too_big:
			move.w	d0,d6				* d6 = BytesPerRow
			swap	d0					* d0 = # of Rows
			mulu	d6,d0				* total # of Bytes
			bra.s	long_way
		endif
		asr.l   #1,d0                       * d0 = size >> 1
		moveq   #$3F,d6
		and.l   d0,d6
	    asr.l   #8,d0
	    asr.l   #1,d0                   * d0 = size >> 10
	    and.w   #$FFC0,d0
	    or.w    d6,d0                   * d7 = ((size>>10)&0xFFC0)|((size>>1)&0x3F);
	    move.w  d0,bltsize(a0)      * start blit
	else
long_way:
*		asr.l   #1,d0                       * d0 = size >> 1
		lsr.l   #1,d0                       * shift unsigned -- bart
		moveq   #$3F,d6
		and.l   d0,d6
		sub.l   d6,d0                   * size -= t;

	    if d6<>#0.l                     * if (t)
		or      #$40,d6
		move    d6,bltsize(a0)  * io->bltsize = 0x40 | t;
	    endif

	    move.l  d0,d6
	    and.l   #$FFC0,d6               * t = size & 0xFFC0;

	    if <>                           * if (t)
		sub.l   d6,d0               * size = size - t;

		WAITBLITDONE   a0            * waitblitdone();

		move    d6,bltsize(a0)  * io->bltsize = t;
	    endif

	    swap d0                     * does a >>16 and test
	    if <>                       * if (size)
		clr.w   d6
		repeat
		    WAITBLITDONE  a0            * waitblitdone();
		    move    d6,bltsize(a0)  * io->bltsize = size = 0;
		    subq.w  #1,d0
		until =
	    endif
	endif

	btst    #0,d1                       * if (flags & 1)
	if <>
	    WAITBLITDONE    a0              * waitblitdone();
	endif

	DISOWNBLITTER                       * disownblitter(GB);

	move.l  (sp)+,d6
	rts                                 * Exit to caller

	end                                 * End of BltClear
