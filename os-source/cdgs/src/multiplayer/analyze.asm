

        OPT     p=68020

        INCLUDE "/cdda/defs.i"
        INCLUDE "/cdda/cd.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "include:exec/libraries.i"

*
************************************************************************
***
***  External References
***
************************************************************************

        XDEF    _Analyze

        XREF    PutChar
        XREF    PutHex
        XREF    Print

PRINT           MACRO
                XREF    Print

                save    a0
                lea     90\1(pc),a0
                jsr     Print
                bra     95\1
90\1              dc.b    \2,10,0
                ds.w    0
95\1             
                restore a0
                ENDM

 STRUCTURE AnalData,0

    APTR   AD_Buff
    UWORD  AD_BuffSize
    UBYTE  AD_AvgCount1
    UBYTE  AD_AvgCount2
    UBYTE  AD_Amplitude
    UBYTE  AD_Pad
    LABEL  AD_SIZE

 STRUCTURE AnalPack,0

    STRUCT AP_List,LH_SIZE
    STRUCT AP_CDXL,CDXL_SIZE*2
    APTR   AP_SampleData
    APTR   AP_AnalBuff;
    APTR   AP_SampleReady;
    STRUCT AP_AnalData,AD_SIZE*12
    LABEL  AP_SIZE

 STRUCTURE PlayerLibrary,0

    STRUCT PL_PlayerLib,LIB_SIZE
    STRUCT PL_AnalPack,AP_SIZE
    STRUCT PL_Analysis,12
    LABEL  PL_SIZE

NUM_FREQ        equ 11
PEEK_SHIFT      equ 2
PEEK_DELTA      equ 1
SIGNAL_DIVIDE   equ 3
MAX_DROP        equ 3

******* player.library/Analyze() **************************************
*
*   NAME
*       Analyze - Analyze audio data and return spectrum analysis
*
*   SYNOPSIS
*       Analysis = Analyze()
*       D0
*
*       UBYTE *Analyze(void)
*
*   FUNCTION
*       Perform spectrum analysis on audio data
*
*   RESULTS
*       NULL = Operation failed (do not own library), otherwise
*              pointer to an 11 byte array.
*
*   NOTES
*       The 11 byte array returned contains level information
*       for the following frequencies:
*
*   SEE ALSO
*
***********************************************************************

;   Check for owner of library!!!

_Analyze
            movem.l d2-d7/a2-a5,-(sp)

            lea     PL_AnalPack(a6),a4                  ; A4 = AnalPack *

            tst.l   AP_SampleData(a4)                   ; Analyze data allocated?
            beq     Exit

*********** Create first 16Khz frequency array

Khz16
            clr.w   d7                                  ; D7 = Freq index

            move.l  AP_SampleReady(a4),a0               ; A0 = Source Data
            move.l  a0,a5                               ; A5 = Source Data

            move.l  a0,d0                               ; Have we a buffer yet?
            beq     Exit

            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            clr.w   d2                                  ; D2 = Source index
            clr.w   d3                                  ; D3 = Destination index
            move.b  AD_AvgCount1(a3),d6                 ; D6 = AvgCount
1$
            clr.l   d4                                  ; D4 = Sum
            clr.w   d5                                  ; D5 = SumCount
2$
            move.b  0(a0,d2.w),d0                       ; Average left and right channel
            extb.l  d0
            move.b  2(a0,d2.w),d1
            extb.l  d1
            add.l   d0,d1
            ror.w   #1,d1
            extb.l  d1

            add.l   d1,d4                               ; Add N samples together
            add.w   #4,d2
            add.w   #1,d5
            cmp.b   d6,d5
            bne     2$

            cmp.b   AD_AvgCount1(a3),d6                 ; Switch counts
            beq     3$
            move.b  AD_AvgCount1(a3),d6
            bra     4$
3$          move.b  AD_AvgCount2(a3),d6
4$
            divs    d5,d4                               ; Find average point of sample
            move.b  d4,0(a1,d3.w)
            add.w   #1,d3
            move.w  AD_BuffSize(a3),d0
            cmp.w   d0,d3
            bhs     5$
            sub.w   #4,d2
            bra     1$
5$
            cmp.l   AP_SampleReady(a4),a5               ; If the buffer was modified, do it again
            bne     Khz16

            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  a1,a0                               ; A0 = Previous frequency data buffer
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            add.w   #AD_SIZE,d7                         ; Next Frequency range


*********** Do the other "Normal" frequency arrays

WaveFormLoop

0$
            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  a1,a0                               ; A0 = Previous frequency data buffer
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            clr.w   d2                                  ; D2 = Source index
            clr.w   d3                                  ; D3 = Destination index
            move.b  AD_AvgCount1(a3),d6                 ; D6 = AvgCount
1$
            clr.l   d4                                  ; D4 = Sum
            clr.w   d5                                  ; D5 = SumCount
2$
            move.b  0(a0,d2.w),d1                       ; Get a data point
            extb.l  d1

            add.l   d1,d4                               ; Add N samples together
            add.w   #1,d2
            add.w   #1,d5
            cmp.b   d6,d5
            bne     2$

            cmp.b   AD_AvgCount1(a3),d6                 ; Switch counts
            beq     3$
            move.b  AD_AvgCount1(a3),d6
            bra     4$
3$          move.b  AD_AvgCount2(a3),d6
4$
            divs    d5,d4                               ; Find average point of sample
            move.b  d4,0(a1,d3.w)
            add.w   #1,d3
            move.w  AD_BuffSize(a3),d0
            cmp.w   d0,d3
            bhs     5$
            sub.w   #1,d2
            bra     1$
5$
            add.w   #AD_SIZE,d7                         ; Next Frequency range
            cmp.w   #AD_SIZE*NUM_FREQ,d7
            bne     0$


*********** Analyze the data

AnalyzeWaves
            lea     PL_Analysis(a6),a2                  ; A2 = Analysis *

            clr.w   d7                                  ; D7 = Frequency index
FreqLoop
            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer
            move.l  a1,a0                               ; A0 = Peek storage buffer (reuse frequency buffer)
            move.w  AD_BuffSize(a3),d6                  ; D6 = Buffer size
            sub.w   #1,d6

            move.b  #$7f,d2                             ; D2 = Last value = max
            clr.b   d3                                  ; D3 = Decending
            clr.l   d4                                  ; D4 = Accumulation of peeks
            clr.w   d5                                  ; D5 = Number of peeks
WaveLoop
            move.b  (a1)+,d1                            ; D1 = Delta between adjacent values
            extb.l  d1
            move.b  (a1),d0
            extb.l  d0
            sub.l   d0,d1
            bpl     1$                                  ; - Convert to absolute value
            move.w  d1,d0
            clr.w   d1
            sub.w   d0,d1
1$
            cmp.b   d2,d1                               ; Are the values ascending?
            bgt     2$

            tst.b   d3                                  ; No, has the wave peeked?
            beq     3$

            extb.l  d2                                  ; Yes, store the peek
            move.b  d2,0(a0,d5.w)
            addq.w  #1,d5

            clr.b   d3                                  ; We are now decending
            bra     3$
2$
            move.b  #1,d3                               ; We are ascending
3$
            move.b  d1,d2                               ; D2 = New last value

            subq.w  #1,d6                               ; Next position
            bne     WaveLoop

*********** Get highest reasonable peek

4$
            clr.b   d6                                  ; Any peeks at all?
            tst.w   d5
            beq     9$

            move.w  d5,d2                               ; D5 = Number of peeks in source array
            lsr.w   #PEEK_SHIFT,d2                      ; D2 = Number of peeks required to be a valid peek

            clr.w   d1                                  ; D1 = Index into peek array
            clr.b   d3                                  ; D3 = Highest peek
5$
            move.b  0(a0,d1.w),d0                       ; Find the peek peek value
            cmp.b   d3,d0
            blo     6$
            move.b  d0,d3
6$          addq.w  #1,d1
            cmp.w   d5,d1
            blo     5$

            move.b  d3,d6                               ; See if there are enough peeks close to this peek
            sub.b   #PEEK_DELTA,d3
            bpl     0$
            clr.b   d3
0$          clr.w   d1                                  ; D1 = Index into peek array
            clr.w   d4                                  ; D4 = Peek count
7$
            move.b  0(a0,d1.w),d0                       ; Find the peek peek value
            cmp.b   d3,d0
            blo     8$
            move.b  d3,0(a0,d1.w)
            addq.w  #1,d4
8$          addq.w  #1,d1
            cmp.w   d5,d1
            blo     7$

            cmp.w   d2,d4                               ; Are there enough peeks in range?  If not, do it again
            blo     4$
9$
            and.l   #$ff,d6
            divu    #SIGNAL_DIVIDE,d6
            cmp.b   #9,d6
            bls     10$
            move.b  #9,d6
10$
            tst.w   d7                                  ; Don't allow frequency curve to drop too fast
            beq     11$
            lea     AP_AnalData-AD_SIZE(a4,d7.w),a0
            move.b  AD_Amplitude(a0),d1
            sub.b   #MAX_DROP,d1
            cmp.b   d1,d6
            bge     11$
            move.b  d1,d6
11$
            move.b  d6,AD_Amplitude(a3)                 ; Store peek amplitude
            move.b  d6,(a2)+

            add.w   #AD_SIZE,d7                         ; Next Frequency range
            cmp.w   #AD_SIZE*NUM_FREQ,d7
            bne     FreqLoop

Exit
            lea     PL_Analysis(a6),a2                  ; A2 = Analysis *
            move.l  a2,d0

            movem.l (sp)+,d2-d7/a2-a5
            rts




