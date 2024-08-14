

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
    UBYTE  AD_AvgCount
    UBYTE  AD_Amplitude
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

*********** Create first 16Khz frequency array

Khz16
            clr.w   d7                                  ; D7 = Freq index

            move.l  AP_SampleReady(a4),a0               ; A0 = Source Data
            move.w  #2352*2,d0
            move.l  #$122800,a1
0$          move.b  (a0)+,(a1)+
            subq.w  #1,d0
            bne     0$

            move.l  AP_SampleReady(a4),a0               ; A0 = Source Data
            move.l  a0,a5                               ; A5 = Source Data

            move.l  a0,d0                               ; Have we a buffer yet?
            beq     Exit

            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            clr.w   d2                                  ; D2 = Source index
            clr.w   d3                                  ; D3 = Destination index
            move.b  AD_AvgCount(a3),d6                  ; D6 = AvgCount

            move.l  #$120000,a2
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
            move.b  d1,(a2)+

            add.l   d1,d4                               ; Add N samples together
            add.w   #4,d2
            add.w   #1,d5
            cmp.b   d6,d5
            bne     2$

            divs    d5,d4                               ; Find average point of sample
            move.b  d4,0(a1,d3.w)

            move.b  d4,(a2)+

            add.w   #1,d3
            move.w  AD_BuffSize(a3),d0
            cmp.w   d0,d3
            bhs     3$
            sub.w   #4,d2
            bra     1$
3$
            cmp.l   AP_SampleReady(a4),a5               ; If the buffer was modified, do it again
            bne     Khz16

            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  a1,a0                               ; A0 = Previous frequency data buffer
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            move.l  #$122000,a2
            clr.w   d3
5$
            move.b  (a1)+,(a2)+
            add.w   #1,d3
            move.w  AD_BuffSize(a3),d0
            cmp.w   d0,d3
            bne     5$

 IFD 0
            add.w   #AD_SIZE,d7                         ; Next Frequency range


*********** Do the other "Normal" frequency arrays

WaveFormLoop

0$
            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  a1,a0                               ; A0 = Previous frequency data buffer
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer

            clr.w   d2                                  ; D2 = Source index
            clr.w   d3                                  ; D3 = Destination index
            move.b  AD_AvgCount(a3),d6                  ; D6 = AvgCount
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

            divs    d5,d4                               ; Find average point of sample
            move.b  d4,0(a1,d3.w)
            add.w   #1,d3
            move.w  AD_BuffSize(a3),d0
            cmp.w   d0,d3
            bhs     3$
            sub.w   #1,d2
            bra     1$
3$
            add.w   #AD_SIZE,d7                         ; Next Frequency range
            cmp.w   #AD_SIZE*8,d7
            bne     0$

 ENDC

*********** Analyze the data

AnalyzeWaves
            move.l  #$124000,a0

            lea     PL_Analysis(a6),a2                  ; A2 = Analysis *

            clr.w   d7
            clr.w   d3
0$
            lea     AP_AnalData(a4,d7.w),a3             ; A3 = AnalData array *
            move.l  AD_Buff(a3),a1                      ; A1 = Frequency data buffer
            move.w  AD_BuffSize(a3),d5                  ; D5 = Buffer size
            sub.w   #1,d5
            clr.b   d2                                  ; D2 = Peek
1$
            move.b  (a1)+,d1                            ; D1 = Delta between adjacent values
            extb.l  d1

            move.l  d1,(a0)+

            move.b  (a1),d0
            extb.l  d0

            move.l  d0,(a0)+

            sub.l   d0,d1

            move.l  d1,(a0)+

            bpl     2$                                  ; Convert to absolute value
            move.w  d1,d0
            clr.w   d1
            sub.w   d0,d1
2$

            move.l  d1,(a0)+

            cmp.b   d1,d2                               ; See if this new peek is higher than last
            bhs     3$
            move.b  d1,d2
3$
            subq.w  #1,d5
            bne     1$                                  ; Next position
4$
            move.l  #$11111111,(a0)+
            move.l  d2,(a0)+

            move.b  d2,AD_Amplitude(a3)                 ; Store peek amplitude
            move.b  d2,0(a2,d3.w)

            add.w   #1,d3                               ; Next Frequency range
            add.w   #AD_SIZE,d7
;            cmp.w   #AD_SIZE*8,d7
;            bne     0$

            cmp.l   AP_SampleReady(a4),a5               ; If the buffer was modified, do it again
            bne     Khz16

Exit
            lea     PL_Analysis(a6),a2                  ; A2 = Analysis *
            move.l  a2,d0

            movem.l (sp)+,d2-d7/a2-a5
            rts




