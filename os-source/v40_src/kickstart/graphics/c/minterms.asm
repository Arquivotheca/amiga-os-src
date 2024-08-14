*******************************************************************************
*
*	$Id: minterms.asm,v 39.1 91/11/18 11:44:25 chrisg Exp $
*
*******************************************************************************


    include 'exec/types.i'                  * Data type definitions
    include 'graphics/rastport.i'           * RastPort/BitMap structures
    include 'hardware/blit.i'               * Blitter constants

    section graphics
    xdef GenMinTerms			   

***** GenMinTerms *****************************************************
* 
*NAME   
* 
*   GenMinTerms - Generate minterms into RastPort structure
*
*
*SYNOPSIS
* 
*   GenMinTerms is called as follows:
*
*               A1 = Pointer to RastPort structure (w)
*               JSR  GenMinTerms
*
* 
*FUNCTION
* 
*   This function was
*   converted from C code with an associated assembly language interface.  The
*   C calling sequence was as follows:
*
*           genminterms(w)
*             register struct RastPort *w;
*           {
*           }
*
*   The assembly language interface called genminterms by pushing the data
*   contained within the following registers onto the stack:
*
*           w       = A1 
*
*
*INPUTS
*
*   Inputs to the GenMinTerms function are required in the following registers:
*
*               A1 = Pointer to the RastPort memory structure
*
*
*REGISTERS USED
*
*   The following registers are defined for use within BltClear
*
*       Register    C Variable Name     Byte Length     Restored For Exit
*
*         D0            minterm              1                No
*         D1            i                    2                No
*         D2            N/A                 N/A               Yes
*         D4            N/A                 N/A               Yes
*         D5          (w->DrawMode)          1                Yes
*         D6        (1&(w->BgPen>>i))        1                Yes
*         D7        (1&(w->FgPen>>1))        1                Yes
*
*         A0            minterm address ptr  4                No
*         A1            w                    4                No
*         A2            N/A                  4                Yes
*         A3            N/A                  4                Yes
*         A4            N/A                  4                Yes
*         A5            N/A                  4                Yes
*         A6            N/A                  4                Yes
*
* 
*RESULT
* 
* 
*EXAMPLE
*
*   Callng sequence shown above.  The table lookup procedure was implemented
*   in accordance with the following MinTerm information:
*
*
*            MinTerm Input Variables              Hex           Decision Table
*                                                Offsets        MinTerm Output
*
*                                               |      |
*     FGPEN   BGPEN    INV     XOR    JAM1/2    |      |
*_______________________________________________|______|________________________
*                                               |      |
*       0       0       0       0       0       |  00  |    NANBC+NABC+ANBC
*                                               |      |
*       0       0       0       0       1       |  01  |    NANBC+NABC
*                                               |      |
*       0       0       0       1       0       |  02  |    NANBC+NABC+ANBC+ABNC
*                                               |      |
*       0       0       0       1       1       |  03  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       0       0       1       0       0       |  04  |    NANBC+NABC+ABC
*                                               |      |
*       0       0       1       0       1       |  05  |    NANBC+NABC
*                                               |      |
*       0       0       1       1       0       |  06  |    NANBC+NABC+ANBNC+ABC
*                                               |      |
*       0       0       1       1       1       |  07  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       0       1       0       0       0       |  08  |    NANBC+NABC+ANBC
*                                               |      |
*       0       1       0       0       1       |  09  |    NANBC+NABC+ANBNC+ANBC
*                                               |      |
*       0       1       0       1       0       |  10  |    NANBC+NABC+ANBC+ABNC
*                                               |      |
*       0       1       0       1       1       |  11  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       0       1       1       0       0       |  12  |    NANBC+NABC+ABC
*                                               |      |
*       0       1       1       0       1       |  13  |    NANBC+NABC+ABNC+ABC
*                                               |      |
*       0       1       1       1       0       |  14  |    NANBC+NABC+ANBNC+ABC
*                                               |      |
*       0       1       1       1       1       |  15  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       1       0       0       0       0       |  16  |    NANBC+NABC+ANBC+ABNC+ABC
*                                               |      |
*       1       0       0       0       1       |  17  |    NANBC+NABC+ABNC+ABC
*                                               |      |
*       1       0       0       1       0       |  18  |    NANBC+NABC+ANBC+ABNC
*                                               |      |
*       1       0       0       1       1       |  19  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       1       0       1       0       0       |  20  |    NANBC+NABC+ABC+ANBNC+ANBC
*                                               |      |
*       1       0       1       0       1       |  21  |    NANBC+NABC+ANBNC+ANBC
*                                               |      |
*       1       0       1       1       0       |  22  |    NANBC+NABC+ANBNC+ABC
*                                               |      |
*       1       0       1       1       1       |  23  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       1       1       0       0       0       |  24  |    NANBC+NABC+ANBC+ABNC+ABC
*                                               |      |
*       1       1       0       0       1       |  25  |    NANBC+NABC+ANBNC+ANBC+ABNC+ABC
*                                               |      |
*       1       1       0       1       0       |  26  |    NANBC+NABC+ANBC+ABNC
*                                               |      |
*       1       1       0       1       1       |  27  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*       1       1       1       0       0       |  28  |    NANBC+NABC+ABC+ANBNC+ANBC
*                                               |      |
*       1       1       1       0       1       |  29  |    NANBC+NABC+ABNC+ABC+ANBNC+ANBC
*                                               |      |
*       1       1       1       1       0       |  30  |    NANBC+NABC+ANBNC+ABC
*                                               |      |
*       1       1       1       1       1       |  31  |    NANBC+NABC+ANBNC+ABNC
*                                               |      |
*
* 
*BUGS
* 
*   None known.
*
*
*SEE ALSO
* 
*   The graph.s,blit.s, types.s, submacs.s and graph.h include files.
*
* CHANGES:
*   6-20-88 REJ speed improvements
*           new version is somewhat larger, esp tables.
*   8-14-89 REJ's code had some bugs -- bart
*	    analyze, fix, optimise and include for v1.4 alpha 17 release
*
* 
******************************************************************************

    PAGE
*                                                 Define all minterms possible

*  NOTE:  Optimized for execution speed rather than memory size



*                               GENERATE MINTERMS FOR RastPort STRUCTURE
	xdef    _genminterms
; void __asm genminterms(register __a1 struct RastPort *rp);
GenMinTerms:
_genminterms:
	movem.w d5-d7,-(sp)             * save registers used

	lea rp_minterms+8(a1),a0        * set up destination

	move.b  rp_DrawMode(a1),d5      * d0.b = w->DrawMode
	andi.w  #$07,d5                 * d0.w = INV, XOR and JAM1/2 bits ONLY
	btst	#1,d5			* test for xormode
	bne.s   xormode             	* if xor, ignore pens

	move.b  rp_FgPen(a1),d7         * d7.b = w->FgPen
	moveq   #3,d1                   * start with msb
	btst    #0,d5               	* check jam bit
	bne.s   jam2mode            	* if jam2, respect bgpen
*
*   	JAM1 mode, ignore bgpen
*

jam1mode:
	lsr.w	 #2,d5			* prepare inv bit
loop2:  move.w   d5,d0           	* get inv bit
	add.b	d7,d7           	* get a bit from FgPen
	addx.w	d0,d0           	* move into index
	add.b	d7,d7           	* get a bit from FgPen
	addx.w	d0,d0           	* now we have index
	add.w	d0,d0	          	* adjust index for WORD fetch
	move.w  JAM1_Table(pc,d0.w),-(a0)  * w->minterms[i] = minterm;
	dbra d1,loop2
	bra.s exit

*
*   	JAM2, not xor, maybe INV
*
jam2mode:
	lsr.w	 #2,d5			* prepare inv bit
	move.b   rp_BgPen(a1),d6        * d6.b = w->BgPen

loop1: 	move.w   d5,d0                  * d0.w = INV bits
        add.b	d7,d7           	* get a bit from FgPen
        addx.w	d0,d0           	* move into index
        add.b	d6,d6           	* a bit from BgPen
        addx.w	d0,d0           	* move into index
        add.b	d7,d7           	* get a bit from FgPen
        addx.w	d0,d0           	* move into index
        add.b	d6,d6           	* a bit from BgPen
        addx.w	d0,d0           	* now we have index
	add.w	d0,d0           	* adjust index for WORD fetch
        move.w   JAM2_TABLE(pc,d0.w),-(a0)   * w->minterms[i] = minterm;
    	dbra     d1,loop1

exit:   move.w  (sp)+,d5
	move.w  (sp)+,d6                * movem.w (sp)+,d5-d7 without extend
    	move.w  (sp)+,d7

	rts                             * Exit to caller

xormode:
	andi	#$ef,ccr		* clear extend
	roxr.w  #2,d5			* roll thru extend in order to
	rol.w   #3,d5                   * eliminate xor bit, adjust LONG fetch
	move.l  XOR_Table(pc,d5.w),d0      * get the minterms
	move.l  d0,-(a0)		* and copy into
	move.l  d0,-(a0)		* the minterms
	bra.s   exit

* NOTE: word align tables for word and longword access !!!
	ds.w	0

JAM1_Table:
* NOTE: this table generates 2 bytes at a time!
* here is the one byte at a time version for reference:
*
*   dc.b    NANBC+NABC+ANBC                     * 00 - /FGPEN, /INV   
*   dc.b    NANBC+NABC+ANBC+ABNC+ABC            * 16 - FGPEN, /INV    
*   dc.b    NANBC+NABC+ABC                      * 04 - /FGPEN, INV    
*   dc.b    NANBC+NABC+ABC+ANBNC+ANBC           * 20 - FGPEN, INV     
*
* Ordering is <INV><FGPEN-HIGH><FGPEN-LOW>
*   the high and low refer to higher and lower numbered bitplanes, in the
*   table, the ordering is: <minterm-low><minterm-high>
*
    dc.b    NANBC+NABC+ANBC                     * 00 - /FGPEN, /INV   
    dc.b    NANBC+NABC+ANBC                     * 00 - /FGPEN, /INV   
    dc.b    NANBC+NABC+ANBC+ABNC+ABC            * 16 - FGPEN, /INV    
    dc.b    NANBC+NABC+ANBC                     * 00 - /FGPEN, /INV   

    dc.b    NANBC+NABC+ANBC                     * 00 - /FGPEN, /INV   
    dc.b    NANBC+NABC+ANBC+ABNC+ABC            * 16 - FGPEN, /INV    
    dc.b    NANBC+NABC+ANBC+ABNC+ABC            * 16 - FGPEN, /INV    
    dc.b    NANBC+NABC+ANBC+ABNC+ABC            * 16 - FGPEN, /INV    

    dc.b    NANBC+NABC+ABC                      * 04 - /FGPEN, INV    
    dc.b    NANBC+NABC+ABC                      * 04 - /FGPEN, INV    
    dc.b    NANBC+NABC+ABC+ANBNC+ANBC           * 20 - FGPEN, INV     
    dc.b    NANBC+NABC+ABC                      * 04 - /FGPEN, INV    

    dc.b    NANBC+NABC+ABC                      * 04 - /FGPEN, INV    
    dc.b    NANBC+NABC+ABC+ANBNC+ANBC           * 20 - FGPEN, INV     
    dc.b    NANBC+NABC+ABC+ANBNC+ANBC           * 20 - FGPEN, INV     
    dc.b    NANBC+NABC+ABC+ANBNC+ANBC           * 20 - FGPEN, INV     

	ds.l	0
XOR_Table:
*
* This table is read a longword at a time.
*
    dc.b    NANBC+NABC+ANBC+ABNC                * 02    /INV XOR JAM1
    dc.b    NANBC+NABC+ANBC+ABNC                * 02    /INV XOR JAM1
    dc.b    NANBC+NABC+ANBC+ABNC                * 02    /INV XOR JAM1
    dc.b    NANBC+NABC+ANBC+ABNC                * 02    /INV XOR JAM1

    dc.b    NANBC+NABC+ANBNC+ABNC               * 03    /INV XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 03    /INV XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 03    /INV XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 03    /INV XOR JAM2

    dc.b    NANBC+NABC+ANBNC+ABC                * 06    INV  XOR JAM1
    dc.b    NANBC+NABC+ANBNC+ABC                * 06    INV  XOR JAM1
    dc.b    NANBC+NABC+ANBNC+ABC                * 06    INV  XOR JAM1
    dc.b    NANBC+NABC+ANBNC+ABC                * 06    INV  XOR JAM1

    dc.b    NANBC+NABC+ANBNC+ABNC               * 07    INV  XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 07    INV  XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 07    INV  XOR JAM2
    dc.b    NANBC+NABC+ANBNC+ABNC               * 07    INV  XOR JAM2

JAM2_TABLE:
*
* JAM2 is in effect.
*
* Ordering is <INV><FGPEN-HIGH><BGPEN-HIGH><FGPEN-LOW><BGPEN_LOW>
*   the high and low refer to higher and lower numbered bitplanes, in the
*   table, the ordering is: <minterm-low><minterm-high>
*
* NOTE: this table generates 2 bytes at a time!
* here is the one byte at a time version for reference: 
*
*   dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
*   dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
*   dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
*   dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
*
*   dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
*   dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
*   dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
*   dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
*
    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN

    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN

    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN

    dc.b    NANBC+NABC                          * 01 /INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 09 /INV, /FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 17 /INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC+ABNC+ABC      * 25 /INV, FGPEN, BGPEN

    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN

    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 

    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN

    dc.b    NANBC+NABC                          * 05 INV, /FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC                 * 13 INV, /FGPEN, BGPEN 
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ANBNC+ANBC               * 21 INV, FGPEN, /BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN
    dc.b    NANBC+NABC+ABNC+ABC+ANBNC+ANBC      * 29 INV, FGPEN, BGPEN

    end                                 * End of SetRast

