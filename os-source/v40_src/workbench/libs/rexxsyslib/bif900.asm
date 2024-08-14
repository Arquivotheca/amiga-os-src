* == bif900.asm ========================================================
*
* Copyright (c) 1988-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Time and Date functions

* =============================     TIME     ===========================
* Retrieves the current system time.
* Usage: TIME([option])
BIFtime
         movea.l  a4,a0                ; environment
         bsr      SysTime              ; get time and date
         movem.l  SYSMINS(a4),d0/d1    ; load minutes and ticks

         lea      NFormat(pc),a0       ; 'Normal' format
         moveq    #24,d3               ; hour limit
         moveq    #60,d4               ; secs/min or mins/hour

         ; Decode the option character.

         tst.b    d7                   ; option given?
         beq.s    BTNormal             ; no

         bclr     #LOWERBIT,d5         ; UPPERcase option
         subi.b   #'C',d5              ; 'Civil'?
         beq.s    BTCivil              ; yes
         subq.b   #'E'-'C',d5          ; 'Elapsed'?
         beq.s    BTElapse             ; yes
         subq.b   #'H'-'E',d5          ; 'Hours'?
         beq.s    BTHour               ; yes
         subq.b   #'M'-'H',d5          ; 'Minutes'?
         beq.s    BTMin                ; yes
         subq.b   #'N'-'M',d5          ; 'Normal'?
         beq.s    BTNormal             ; yes
         subq.b   #'R'-'N',d5          ; 'Reset'?
         beq.s    BTReset              ; yes
         subq.b   #'S'-'R',d5          ; 'Seconds'?
         bne      BFErr18              ; no --invalid option

         ; 'Seconds' ... return seconds after midnight.

         divu     #TICKRATE,d1         ; lower: seconds
         move.w   d1,d3                ; ignore remainder
         mulu     d4,d0                ; seconds
         add.l    d3,d0                ; total seconds

         ; 'Minutes' ... return minutes after midnight.

BTMin    move.l   d0,d3                ; minutes
         bra.s    BIF901               ; numeric return

         ; 'Hours' ... return hours after midnight.

BTHour   divu     d4,d0                ; hours
         move.w   d0,d3                ; ignore remainder

BIF901   bra      BFNumber             ; numeric return

         ; 'Civil' option ... time in form HH:MMXX

BTCivil  moveq    #'A',d2              ; 'AM' suffix
         moveq    #12,d3               ; hour limit
         lea      CFormat(pc),a0       ; 'Civil' format

         ; 'Normal' (default) option ... convert time to form "HH:MM:SS"

BTNormal divu     #TICKRATE,d1         ; seconds

         divu     d4,d0                ; upper: minutes lower: hours
         cmp.w    d3,d0                ; hours <= limit?
         blt.s    2$                   ; ... less
         beq.s    1$                   ; ... equal
         sub.w    d3,d0                ; adjust hours

1$:      moveq    #'P',d2              ; 'PM' suffix

2$:      tst.w    d2                   ; civil time?
         beq.s    3$                   ; no
         move.w   d2,d1                ; suffix character
         tst.w    d0                   ; hours > 0?
         bne.s    3$                   ; yes
         move.w   d3,d0                ; ... recode hours

3$:      swap     d0                   ; upper: hours lower:minutes
         bra.s    BIF903

*         movea.l  a3,a0                ; work buffer
*         movea.l  sp,a1                ; buffer
*         moveq    #':',d0              ; separator
*         bsr      ThreePart            ; A0=scan
*         addq.w   #6,sp                ; restore stack
*
*         ; Check whether to append the suffix string ('AM' or 'PM').
*
*         tst.w    d2                   ; a suffix?
*         beq.s    BIF903               ; no
*
*         subq.w   #3,a0                ; ... back up
*         move.b   d2,(a0)+             ; 'A' or 'P'
*         move.b   #'M',(a0)+           ; 'M' 
*         bra.s    BIF903

         ; 'Reset' ... reset timer and return elapsed time.

BTReset  moveq    #-1,d2               ; 'update' flag

         ; 'Elapsed' ... return elapsed time.

BTElapse mulu     #TICKRATE*60,d0      ; D0=ticks
         add.l    d1,d0                ; add ticks

         ; Compute the elapsed time in ticks ...

         move.l   ev_Elapse(a4),d1     ; current timer
         bpl.s    1$                   ; ... already started
         move.l   d0,d1                ; initial value
         moveq    #-1,d2               ; set 'update' flag

1$:      tst.w    d2                   ; reset timer?
         beq.s    2$                   ; no
         move.l   d0,ev_Elapse(a4)     ; update timer

2$:      sub.l    d1,d0                ; elapsed ticks
         bcc.s    3$                   ; wrapped around?
         add.l    #TICKSDAY,d0         ; add offset

         ; Convert elapsed time to seconds in form "SSSSS.SS"

3$:      moveq    #TICKRATE,d1         ; ticks/second
         bsr      Ldivu                ; D0=seconds  D1=ticks

         lsl.w    #1,d1                ; convert to 1/100ths
         lea      EFormat(pc),a0       ; elapsed format

*         move.l   d1,d3                ; save ticks
*         movea.l  a3,a0                ; work area
*         moveq    #5,d1                ; digits
*         CALLSYS  CVi2a                ; A0=scan
*         move.b   #'.',(a0)+           ; decimal point
*
*         move.l   d3,d0                ; ticks=>D0
*         lsl.l    #1,d0                ; hundredths (X 100/TICKRATE)
*         moveq    #2,d1                ; decimal places
*         CALLSYS  CVi2az               ; A0=scan
*
*BIF903   move.l   a0,d3                ; end of string
*         sub.l    a3,d3                ; length
*         bra      BFNewstr             ; string return

         ; Format the data stream ...

BIF903   move.w   d1,-(sp)             ; push hundredths
         move.l   d0,-(sp)             ; push datum
         movea.l  sp,a1                ; data stream
         move.l   a3,d0                ; buffer
         bsr      FormatString         ; D0=length
         addq.w   #6,sp                ; restore stack
         move.l   d0,d3                ; final length
         bra      BFNewstr

         ; Format strings

CFormat  dc.b     '%d:%02d%cM',0       ; 'Civil' format
NFormat  dc.b     '%02d:%02d:%02d',0   ; 'Normal' format
EFormat  dc.b     '%ld.%02d',0         ; 'Elapsed' format
         CNOP     0,2

         STRUCTURE DateBlock,0
         UWORD    Month                ; month 1-12
         UWORD    Day                  ; day 1-31
         UWORD    Year                 ; 2-digit year 00-99
         UWORD    Julian               ; day in year
         ULONG    SysDays              ; system days
         LABEL    db_SIZEOF

* =============================     Date     ===========================
* Implements the DATE() function.
* USAGE: DATE([option],[date],[format])
BIFdate
         lea      32(a3),a2            ; dateblock pointer
         move.w   #365,d6              ; load days

         moveq    #'N',d4              ; default option
         tst.w    d0                   ; option argument?
         beq.s    1$                   ; no
         move.b   (a0),d4              ; load option

1$:      cmpi.b   #2,d7                ; date/format arguments?
         blt.s    BF9Date              ; no ... option only
         beq.s    BF9D01               ; date only

         ; Format argument specified ... check for 'I' or 'S'.

         tst.w    d2                   ; format supplied?
         beq.s    BF9D01               ; no
         bclr     #LOWERBIT,d5         ; UPPERcase
         subi.b   #'I',d5              ; 'Internal'?
         beq.s    BF9D01               ; yes
         subi.b   #'S'-'I',d5          ; 'Sorted'?
         bne      BFErr18              ; no ... error

         ; Convert from YYYYMMDD to internal (system) days.

         divu     #10000,d3            ; upper: mon/day lower: year
         move.w   d3,d2                ; load year
         subi.w   #1977,d2             ; offset from 1977
         bcs      BFErr18              ; ... error
         divu     #4,d2                ; upper: years lower: 4-year blocks
         move.w   d2,d1                ; save 4-year blocks

         ; Check for a leap day in the year.

         swap     d2                   ; years=>lower
         cmpi.w   #3,d2                ; leap year?
         seq      d0                   ; set flag
         ext.w    d0                   ; extend ... -1 or 0

         ; Compute the total days in the years.

         mulu     d6,d2                ; days in remainder years
         mulu     #1461,d1             ; days in 4-year blocks

         ; Calculate the cumulative days up to the month.

         clr.w    d3                   ; clear lower
         swap     d3                   ; month/days=>lower
         divu     #100,d3              ; upper: days lower: month
         subq.w   #2,d3                ; month index
         blt.s    3$                   ; no days
         beq.s    2$                   ; no leap day
         sub.w    d0,d2                ; ... add leap day

2$:      move.b   BF9Days(pc,d3.w),d5  ; load days
         add.w    d5,d2                ; accumulate
         dbf      d3,2$                ; loop back

3$:      clr.w    d3                   ; clear lower
         swap     d3                   ; days=>lower
         subq.w   #1,d3                ; day index
         add.l    d1,d3                ; days in 4-year blocks
         add.l    d2,d3                ; days in remainder years
         sub.l    d6,d3                ; offset to Jan 1, 1978
         bra.s    BF9D01

         ; Array of days in months 1-12

BF9Days  dc.b     31,28,31,30,31,30,31,31,30,31,30,31

         ; No date specified ... get the current system date.

BF9Date  movea.l  a4,a0                ; environment
         bsr      SysTime              ; get date/time
         move.l   SYSDAYS(a4),d3       ; days since Jan 1, 1978

         ; Construct the DateBlock in the temporary buffer area.

BF9D01   move.w   d4,d5                ; save option
         move.l   d3,SysDays(a2)       ; install days

         cmpi.l   #122*365+30,d3       ; outside range?
         bgt      BFErr18              ; yes

         move.w   d6,d2                ; days in 1977
         add.l    d3,d2                ; days since Jan 1, 1977
         moveq    #77,d3               ; base year = 1977
         divu     #1461,d2             ; 4-year block: 365+365+365+366
         lsl.w    #2,d2                ; years
         add.w    d2,d3                ; accumulate

         clr.w    d2
         swap     d2                   ; days in block
         divu     d6,d2                ; low=years high=days
         add.w    d2,d3                ; final year (maybe)

         suba.l   a0,a0                ; clear leap flag
         subq.w   #3,d2                ; leap year?
         bcs.s    2$                   ; no
         beq.s    1$                   ; yes ... normal

         ; Special case ... last day of leap year.

         subq.w   #1,d3                ; adjust year
         move.w   d6,d2                ; day 365
         swap     d2                   ; to high end

1$:      addq.w   #1,a0                ; set leap flag

2$:      swap     d2                   ; day in year (0-365)
         addq.w   #1,d2                ; recode
         move.w   d2,d4                ; save Julian day

         ; Compute the month and day ... A0 is leap day.

         moveq    #0,d0                ; clear days
         moveq    #0,d1                ; month index
         bra.s    4$                   ; jump in

3$:      sub.w    d0,d2                ; decrement days
         move.w   a0,d0                ; install leap day
         suba.l   a0,a0                ; clear leap day

4$:      add.b    BF9Days(pc,d1.w),d0  ; days in month
         addq.w   #1,d1                ; advance month
         cmp.w    d0,d2                ; days > month?
         bhi.s    3$                   ; ... loop back

         movem.w  d1/d2/d3/d4,(a2)     ; install Month/Day/Year/Julian
         movea.l  a3,a0                ; buffer area
         move.l   SysDays(a2),d0       ; load days
         addi.w   #1900,d3             ; offset year

         ; Decode the option ...

         bclr     #LOWERBIT,d5         ; UPPERcase
         subi.w   #'B',d5              ; >= 'B'?
         bcs.s    BDErr                ; no
         cmpi.w   #'W'-'B',d5          ; <= 'W'
         bhi.s    BDErr                ; no
         move.b   BDTable(pc,d5.w),d5  ; load offset
         jmp      BDTable(pc,d5.w)     ; branch to it

BDTable  dc.b     BDB-BDTable          ; 'Basedate'
         dc.b     BDC-BDTable          ; 'Century'
         dc.b     BDD-BDTable          ; 'Days'
         dc.b     BDE-BDTable          ; 'European'
         dc.b     BDErr-BDTable
         dc.b     BDErr-BDTable
         dc.b     BDErr-BDTable
         dc.b     BDI-BDTable          ; 'Internal'
         dc.b     BDJ-BDTable          ; 'Julian'
         dc.b     BDErr-BDTable
         dc.b     BDErr-BDTable
         dc.b     BDM-BDTable          ; 'Month'
         dc.b     BDN-BDTable          ; 'Normal'
         dc.b     BDO-BDTable          ; 'Ordered'
         dc.b     BDErr-BDTable
         dc.b     BDErr-BDTable
         dc.b     BDErr-BDTable
         dc.b     BDS-BDTable          ; 'Sorted'
         dc.b     BDErr-BDTable
         dc.b     BDU-BDTable          ; 'USA'
         dc.b     BDErr-BDTable
         dc.b     BDW-BDTable          ; 'Weekday'
         CNOP     0,2

BDErr    bra      BFErr18              ; invalid operand

         ; 'Basedate' ... days since Jan 1, 0001.

BDB      addi.l   #693960,d0           ; offset to Jan 1, 0001

         ; 'Century' ... days since Jan 1, 1900.

BDC      addi.l   #28490,d0            ; days to Jan 1, 1978
         bra.s    BDI

         ; 'Days' ... report days in year as DDD.

BDD      move.w   d4,d0                ; days in year

         ; 'Internal' ... report days since Jan 1, 1978.

BDI      moveq    #6,d1                ; max digits
         CALLSYS  CVi2a                ; D0=length
         bra.s    BDOut

         ; 'European' ... format as DD/MM/YY.

BDE      move.w   d3,-(sp)             ; push year
         move.w   d1,-(sp)             ; push month
         move.w   d2,-(sp)             ; push day
         bra.s    BDStdFmt

         ; 'Julian' ... format as YYDDD with leading zeroes.

BDJ      move.w   d3,d0                ; year
         moveq    #2,d1                ; 2 digits
         CALLSYS  CVi2az               ; A0=scan
         move.w   d4,d0                ; days in year
         moveq    #3,d1                ; three digits
         bra.s    BDN2

         ; 'Month' ... return current month.

BDM      bsr      GetMonth             ; D0=length A1=month
         movea.l  a1,a3                ; update buffer
         bra.s    BDOut

         ; 'Normal' ... format as DD MMM YYYY.

BDN      moveq    #' ',d5              ; separator
         move.w   d2,d0                ; day in month
         moveq    #2,d1                ; 2 digits
         CALLSYS  CVi2az               ; D0=length A0=scan
         move.b   d5,(a0)+             ; separator

         move.w   (a2),d1              ; load month
         bsr.s    GetMonth             ; D0=length A1=month (A0 preserved)
         move.b   (a1)+,(a0)+          ; 1st character
         move.b   (a1)+,(a0)+          ; 2nd
         move.b   (a1)+,(a0)+          ; 3rd
         move.b   d5,(a0)+             ; separator

         move.w   d3,d0                ; year
         moveq    #4,d1                ; 4-digits
BDN2     CALLSYS  CVi2az               ; A0=scan
         bra.s    BDLength

         ; 'Ordered' ... format as YY/MM/DD

BDO      move.w   d2,-(sp)             ; push day
         move.w   d1,-(sp)             ; push month
         move.w   d3,-(sp)             ; push year
         bra.s    BDStdFmt

         ; 'Sorted' ... format as YYYYMMDD

BDS      move.w   d2,-(sp)             ; push day
         move.w   d1,-(sp)             ; push month
         move.w   d3,-(sp)             ; push year

         moveq    #4,d1                ; initial length
         moveq    #0,d0                ; no separator
         bra.s    BDFormat

         ; 'Weekday' ... return day of week.

BDW      bsr.s    GetWeek              ; D0=length A1=weekday
         movea.l  a1,a3                ; update buffer
         bra.s    BDOut

         ; 'USA' ... format as MM/DD/YY

BDU      move.w   d3,-(sp)             ; push year
         move.w   d2,-(sp)             ; push day
         move.w   d1,-(sp)             ; push month

BDStdFmt moveq    #2,d1                ; initial length
         moveq    #'/',d0              ; default separator

         ; Format three words from the stack frame ... separator in D0.

BDFormat movea.l  sp,a1                ; data stream
         bsr.s    ThreePart            ; A0=scan
         addq.w   #6,sp                ; restore stack

BDLength move.l   a0,d0                ; end of string
         sub.l    a3,d0                ; length

BDOut    move.l   d0,d3                ; final length
         moveq    #0,d6                ; clear error
         bra      BFNewstr

* ==========================     ThreePart     =========================
* Formats three values from the specified data stream with a separator.
* Registers:   A0 -- scan pointer
*              A1 -- data stream
*              D0 -- separator
*              D1 -- initial length
* Return:      A0 -- scan pointer
ThreePart
         movem.l  d2/d3/a2,-(sp)
         movea.l  a1,a2
         move.b   d0,d2
         moveq    #2,d3                ; loop count
         bra.s    2$                   ; jump in

1$:      move.b   d2,(a0)+             ; a separator?
         bne.s    2$                   ; yes ...
         subq.w   #1,a0                ; back up

2$:      move.w   (a2)+,d0             ; pop value
         CALLSYS  CVi2az               ; D0=length
         moveq    #2,d1                ; default length
         dbf      d3,1$                ; loop back

         movem.l  (sp)+,d2/d3/a2
         rts

* ===========================     GetMonth     =========================
* Returns the name of the current month.  A0 is preserved.
* Registers:   D1 -- month
* Return:      D0 -- length
*              A1 -- pointer to name
GetMonth
         lea      (MNIndex-1)(pc,d1.w),a1
         bra.s    GW001

* ============================     GetWeek     =========================
* Returns the name of the current day of the week.  A0 is preserved.
* Registers:   D0 -- system days
* Return:      D0 -- length
*              A1 -- pointer to name
GetWeek
         divu     #7,d0                ; upper=day index lower=weeks
         swap     d0                   ; weekday index (0-6)
         lea      WKIndex(pc,d0.w),a1  ; load pointer

GW001    moveq    #0,d0                ; clear offset
         move.b   (a1),d0              ; load offset
         adda.w   d0,a1                ; selected entry
         move.b   (a1)+,d0             ; ... load length
         rts

         ; String constants

WKIndex  dc.b     WKSun-WKIndex-0,WKMon-WKIndex-1,WKTue-WKIndex-2
         dc.b     WKWed-WKIndex-3,WKThu-WKIndex-4,WKFri-WKIndex-5
         dc.b     WKSat-WKIndex-6

WKSun    dc.b     6,'Sunday'
WKMon    dc.b     6,'Monday'
WKTue    dc.b     7,'Tuesday'
WKWed    dc.b     9,'Wednesday'
WKThu    dc.b     8,'Thursday'
WKFri    dc.b     6,'Friday'
WKSat    dc.b     8,'Saturday'

MNIndex  dc.b     MNJan-MNIndex-0,MNFeb-MNIndex-1,MNMar-MNIndex-2
         dc.b     MNApr-MNIndex-3,MNMay-MNIndex-4,MNJun-MNIndex-5
         dc.b     MNJul-MNIndex-6,MNAug-MNIndex-7,MNSep-MNIndex-8
         dc.b     MNOct-MNIndex-9,MNNov-MNIndex-10,MNDec-MNIndex-11

MNJan    dc.b     7,'January'
MNFeb    dc.b     8,'February'
MNMar    dc.b     5,'March'
MNApr    dc.b     5,'April'
MNMay    dc.b     3,'May'
MNJun    dc.b     4,'June'
MNJul    dc.b     4,'July'
MNAug    dc.b     6,'August'
MNSep    dc.b     9,'September'
MNOct    dc.b     7,'October'
MNNov    dc.b     8,'November'
MNDec    dc.b     8,'December'
         CNOP     0,2
