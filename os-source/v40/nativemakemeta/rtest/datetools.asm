	SECTION	DateTools

	XREF	UDivMod32S
	XREF	UMult32S

*$*	XDEF	CalcDateS
	XDEF	Amiga2DateS
	XDEF	Date2AmigaS
	XDEF	CheckDateS

*$*	XDEF	CalcDateH
	XDEF	_Amiga2DateH
	XDEF	_Date2AmigaH
	XDEF	_CheckDateH


	INCLUDE	"exec/types.i"
	INCLUDE	"date.i"

;
;	struct ClockData
;		{
;		UWORD	sec;
;		UWORD	min;
;		UWORD	hour;
;		UWORD	mday;
;		UWORD	month;
;		UWORD	year;
;		UWORD	wday;
;		};
;
;
;	/*
;	 * an interesting and useful macro I designed (the 'function' form
;	 * is much simpler, but for constants everthing is resolved at compile
;	 * time).
;	 */
;	#define CALC_DATE(YEAR,MONTH,DAY) \
;	  ( \
;	  ((YEAR)-1+((MONTH)+9)/12)*365L \
;	  +((YEAR)-1+((MONTH)+9)/12)/4 \
;	  -((YEAR)-1+((MONTH)+9)/12)/100 \
;	  +((YEAR)-1+((MONTH)+9)/12)/400 \
;	  +((((MONTH)+9)%12)*306+5)/10 \
;	  +(DAY) \
;	  -1 \
;	 )
;

*****i* utility.library/CalcDate *************************************
*
*   NAME
*	CalcDate -- Returns a relative day number based on date.  (V36)
*
*   SYNOPSIS
*	Index = CalcDate( Date )
*	D0                A0
*
*	ULONG CalcDate( struct ClockData * );
*
*   FUNCTION
*	Calculates the relative day number of the date specified relative
*	    to a date a long time ago.
*
*   INPUTS
*	Date -- pointer to a ClockData structure.
*
*   RESULTS
*	Index -- The number of days since a date a long time ago.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	ULONG __asm CalcDate(register __d0 ULONG year, register __d1 ULONG month, register __d2 ULONG day)
;		{
;		month += 9;
;		year = year-1 + (month/12);
;		month %= 12;
;		month = month*306 +5;
;
;		return (year*365) + (year/4) - (year/100) + (year/400)
;			+ (month/10) + day -1;
;		}

CalcDateS:
	MOVE.L    D2,-(A7)
	MOVE.L    D0,D2		; d2=year
	MOVEQ     #09,D0
	ADD.L     D1,D0		; d0=month+9
	MOVEQ     #12,D1
	JSR       UDivMod32S	; d1=(month+9)%12
	ADD.L     D0,D2		; d2=year+(month+9)/12
	MOVE.L    #306,D0
	JSR       UMult32S	; d0=((month+9)%12)*306
	ADDQ.L    #5,D0		; d0=((month+9)%12)*306+5
	MOVEQ     #10,D1
	JSR       UDivMod32S	; d0=(((month+9)%12)*306+5)/10
	MOVE.L    D0,-(SP)
	SUBQ.L    #1,D2		; d2=year+(month+9)/12 -1
	MOVE.L    D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ     #100,D1
	LSL.L     #2,D1
	JSR       UDivMod32S	; d0=(year+(month+9)/12-1)/400
	MOVE.L    D0,-(SP)
	MOVE.L    D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ     #100,D1
	JSR       UDivMod32S	; d0=(year+(month+9)/12-1)/100
	MOVE.L    D0,-(SP)
	MOVE.L    D2,D1		; d1=year+(month+9)/12-1
	LSR.L     #2,D2		; d2=(year+(month+9)/12-1)/4
	MOVE.L    #365,D0
	JSR       UMult32S	; d0=(year+(month+9)/12-1)*365
	ADD.L     D2,D0		; d0=d0 + (year+(month+9)/12-1)/4
	SUB.L     (SP)+,D0	; d0=d0 - (year+(month+9)/12-1)/100
	ADD.L     (SP)+,D0	; d0=d0 + (year+(month+9)/12-1)/400
	ADD.L     (SP)+,D0	; d0=d0 + (((month+9)%12)*306+5)/10
	MOVE.L    (A7)+,D2
	ADD.L     D2,D0		; d0=d0 + day
	SUBQ.L    #1,D0		; d0=d0 -1
	RTS

CalcDateH:
	MOVE.L    D2,-(A7)
	MOVE.L    D0,D2		; d2=year
	MOVEQ     #09,D0
	ADD.L     D1,D0		; d0=month+9
	MOVEQ     #12,D1
	divul.l	d1,d1:d0	; d1=(month+9)%12
	ADD.L     D0,D2		; d2=year+(month+9)/12
	MOVE.L    #306,D0
	mulu.l	d1,d0		; d0=((month+9)%12)*306
	ADDQ.L    #5,D0		; d0=((month+9)%12)*306+5
	MOVEQ     #10,D1
	divul.l	d1,d1:d0	; d0=(((month+9)%12)*306+5)/10
	MOVE.L    D0,-(SP)
	SUBQ.L    #1,D2		; d2=year+(month+9)/12 -1
	MOVE.L    D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ     #100,D1
	LSL.L     #2,D1
	divul.l	d1,d1:d0	; d0=(year+(month+9)/12-1)/400
	MOVE.L    D0,-(SP)
	MOVE.L    D2,D0		; d0=year+(month+9)/12 -1
	MOVEQ     #100,D1
	divul.l	d1,d1:d0	; d0=(year+(month+9)/12-1)/100
	MOVE.L    D0,-(SP)
	MOVE.L    D2,D1		; d1=year+(month+9)/12-1
	LSR.L     #2,D2		; d2=(year+(month+9)/12-1)/4
	MOVE.L    #365,D0
	mulu.l	d1,d0		; d0=(year+(month+9)/12-1)*365
	ADD.L     D2,D0		; d0=d0 + (year+(month+9)/12-1)/4
	SUB.L     (SP)+,D0	; d0=d0 - (year+(month+9)/12-1)/100
	ADD.L     (SP)+,D0	; d0=d0 + (year+(month+9)/12-1)/400
	ADD.L     (SP)+,D0	; d0=d0 + (((month+9)%12)*306+5)/10
	MOVE.L    (A7)+,D2
	ADD.L     D2,D0		; d0=d0 + day
	SUBQ.L    #1,D0		; d0=d0 -1
	RTS


******* utility.library/Amiga2Date ***********************************
*
*   NAME
*	Amiga2Date -- Calculate the date from a timestamp.  (V36)
*
*   SYNOPSIS
*	Amiga2Date( AmigaTime, Date )
*	            D0         A0
*
*	void Amiga2Date( ULONG, struct ClockData * );
*
*   FUNCTION
*	Fills in a ClockData structure with the date and time calculated
*	    from a ULONG containing the number of seconds from 01-Jan-1978
*	    to the date.
*
*   INPUTS
*	AmigaTime -- The number of seconds from 01-Jan-1978.
*
*   RESULTS
*	Date -- Filled in with the date/time specified by AmigaTime.
*
*   NOTES
*
*   SEE ALSO
*	CheckDate(), Date2Amiga()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	void __asm Amiga2Date(register __d0 ULONG amiga, register __a0 struct ClockData *cd)
;	  {
;	   LONG year;
;	   LONG month;
;	   LONG day;
;	   LONG hour;
;	   LONG minute;
;	   LONG second;
;	   LONG absday;
;	   LONG work;
;
;
;	   second  = amiga % 60;
;	   minute  = amiga / 60;
;	   hour    = minute / 60;
;	   minute %= 60;
;	   hour   %= 24;
;
;	   absday  = amiga / (60 * 60 * 24);
;	   absday += CALC_DATE(1978,1,1);      /* add the Amiga date offset */
;
;	   cd->wday  = (absday+3)%7
;
;	   /*
;	    * the following date logic took a great deal of time to develop and prove
;	    * so please give credit if you use it. That also includes the macro
;	    * in AmigArc.h.
;	    * NOTE: the following were split into separate lines for aestetic(sp?)
;	    * reasons as well as to prevent the (sometimes very stupid!) 'C'
;	    * optimizer from changing the math (implicit floor function is very
;	    * important!).
;	    */
;
;	   /* find year */
;	   work = absday;
;	   work -= (absday + 1) / 146097;   /* subtract quadcentury leap days */
;	   work += work / 36524;            /* add century leap days */
;	   work -= (work + 1) / 1461;       /* subtract all leap days */
;	   year = work / 365;
;
;	   /* find day of year */
;	   work = absday;
;	   work -= year * 365;
;	   work -= year / 4;
;	   work += year / 100;
;	   work -= year / 400;
;
;	   /* find month */
;	   month = work / 153;
;	   month *= 5;
;	   month += 10 * (work % 153) / 305;
;
;	   /* find day */
;	   day = 1 + work;
;	   day -= (long)((month * 306 + 5) / 10);
;
;	   /* adjust to Gregorian calendar */
;	   month += 2;
;	   year += month / 12;
;	   month %= 12;
;	   month++;
;
;	   cd->sec   = second;
;	   cd->min   = minute;
;	   cd->hour  = hour;
;	   cd->day   = day;
;	   cd->month = month;
;	   cd->year  = year;
;	  }

Amiga2DateS:
	MOVEM.L   D2-D3,-(A7)
	MOVEQ     #60,D1
	JSR       UDivMod32S
	MOVE.W    D1,sec(A0)	; d1=amiga%60
	MOVEQ     #60,D1	; d0=amiga/60
	JSR       UDivMod32S
	MOVE.W    D1,min(A0)	; d1=(amiga/60)%60
	MOVEQ     #24,D1	; d0=amiga/3600
	JSR       UDivMod32S
	MOVE.W    D1,hour(A0)	; d1=(amiga/3600)%24
	ADDI.L    #722390,D0	; d0=absday
	MOVE.L    D0,-(SP)
	MOVE.L    D0,D2
	ADDQ.L    #3,D0		; offset so sunday = 0
	MOVEQ.L   #7,d1
	JSR       UDivMod32S	; d1=dayofweek
	MOVE.W    d1,wday(A0)
	MOVE.L    D2,D0
	ADDQ.L    #1,D0
	MOVE.L    #146097,D1
	JSR       UDivMod32S	; d0=(absday+1)/146097
	SUB.L     D0,D2		; work=absday - (absday+1)/146097
	MOVE.L    D2,D0
	MOVE.L    #36524,D1
	JSR       UDivMod32S
	ADD.L     D0,D2		; work += work/36524
	MOVE.L    D2,D0
	ADDQ.L    #1,D0
	MOVE.L    #1461,D1
	JSR       UDivMod32S
	SUB.L     D0,D2		; work -= (work+1)/1461
	MOVE.L    D2,D0
	MOVE.L    #365,D1
	JSR       UDivMod32S

	MOVE.L    (SP)+,D2	; work = absday
	MOVE.L    D0,-(SP)	; save year
	MOVE.L    D0,D3
	MOVEQ     #100,D1
	LSL.L     #2,D1
	JSR       UDivMod32S	; d0=year/400
	SUB.L     D0,D2		; work-=year/400
	MOVE.L    D3,D0
	MOVEQ     #100,D1
	JSR       UDivMod32S	; d0=year/100
	ADD.L     D0,D2		; work+=year/100
	MOVE.L    D3,D0
	LSR.L     #2,D0
	SUB.L     D0,D2		; work-=year/4
	MOVE.L    D3,D0
	MOVE.L    #365,D1
	JSR       UMult32S	; d0=year*365
	SUB.L     D0,D2		; d2=work-=year*365

	MOVE.L    D2,D0
	MOVEQ     #102,D1
	NOT.B     D1
	JSR       UDivMod32S	; d1=work%153
	MOVEQ     #10,D0
	JSR       UMult32S	; d0=10*(work%153)
	MOVE.L    #305,D1
	JSR       UDivMod32S	; d0=10*(work%153)/305
	MOVE.L    D0,D3
	MOVE.L    D2,D0
	MOVEQ     #102,D1
	NOT.B     D1
	JSR       UDivMod32S	; d0=work/153
	MOVE.L    D0,D1
	LSL.L     #2,D1
	ADD.L     D0,D1		; d1=5*(work/153)
	ADD.L     D1,D3		; d2==work; d3==month

	MOVE.L    #306,D0
	MOVE.L    D3,D1
	JSR       UMult32S	; d0=month*306
	ADDQ.L    #5,D0		; d0=month*306+5
	MOVEQ     #10,D1
	JSR       UDivMod32S	; d0=(month*306+5)/10
	ADDQ.L    #1,D2
	SUB.L     D0,D2		; d2=day=1+work-(month*306+5)/10
	MOVE.W    D2,mday(A0)
	MOVE.L    D3,D0
	ADDQ.L    #2,D0
	MOVEQ     #12,D1
	JSR       UDivMod32S	; d0=(month+2)/12
	ADDQ.L    #1,D1		; d1=(month+2)%12 +1
	MOVE.W    D1,month(A0)
	ADD.L     (SP)+,D0	; year+=(month+2)/12
	MOVE.W    D0,year(A0)
	MOVEM.L   (A7)+,D2-D3
	RTS

_Amiga2DateH:
	MOVEM.L   D2-D3,-(A7)
	MOVEQ     #60,D1
	JSR       UDivMod32S
	MOVE.W    D1,sec(A0)	; d1=amiga%60
	MOVEQ     #60,D1	; d0=amiga/60
	divul.l	d1,d1:d0
	MOVE.W    D1,min(A0)	; d1=(amiga/60)%60
	MOVEQ     #24,D1	; d0=amiga/3600
	divul.l	d1,d1:d0
	MOVE.W    D1,hour(A0)	; d1=(amiga/3600)%24
	ADDI.L    #722390,D0	; d0=absday
	MOVE.L    D0,-(SP)
	MOVE.L    D0,D2
	ADDQ.L    #3,D0		; offset so sunday = 0
	MOVEQ.L   #7,d1
	divul.l	d1,d1:d0	; d1=dayofweek
	MOVE.W    d1,wday(A0)
	MOVE.L    D2,D0
	ADDQ.L    #1,D0
	MOVE.L    #146097,D1
	divul.l	d1,d1:d0	; d0=(absday+1)/146097
	SUB.L     D0,D2		; work=absday - (absday+1)/146097
	MOVE.L    D2,D0
	MOVE.L    #36524,D1
	divul.l	d1,d1:d0
	ADD.L     D0,D2		; work += work/36524
	MOVE.L    D2,D0
	ADDQ.L    #1,D0
	MOVE.L    #1461,D1
	divul.l	d1,d1:d0
	SUB.L     D0,D2		; work -= (work+1)/1461
	MOVE.L    D2,D0
	MOVE.L    #365,D1
	divul.l	d1,d1:d0

	MOVE.L    (SP)+,D2	; work = absday
	MOVE.L    D0,-(SP)	; save year
	MOVE.L    D0,D3
	MOVEQ     #100,D1
	LSL.L     #2,D1
	divul.l	d1,d1:d0	; d0=year/400
	SUB.L     D0,D2		; work-=year/400
	MOVE.L    D3,D0
	MOVEQ     #100,D1
	divul.l	d1,d1:d0	; d0=year/100
	ADD.L     D0,D2		; work+=year/100
	MOVE.L    D3,D0
	LSR.L     #2,D0
	SUB.L     D0,D2		; work-=year/4
	MOVE.L    D3,D0
	MOVE.L    #365,D1
	mulu.l	d1,d1:d0	; d0=year*365
	SUB.L     D0,D2		; d2=work-=year*365

	MOVE.L    D2,D0
	MOVEQ     #102,D1
	NOT.B     D1
	divul.l	d1,d1:d0	; d1=work%153
	MOVEQ     #10,D0
	mulu.l	d1,d1:d0	; d0=10*(work%153)
	MOVE.L    #305,D1
	divul.l	d1,d1:d0	; d0=10*(work%153)/305
	MOVE.L    D0,D3
	MOVE.L    D2,D0
	MOVEQ     #102,D1
	NOT.B     D1
	divul.l	d1,d1:d0	; d0=work/153
	MOVE.L    D0,D1
	LSL.L     #2,D1
	ADD.L     D0,D1		; d1=5*(work/153)
	ADD.L     D1,D3		; d2==work; d3==month

	MOVE.L    #306,D0
	MOVE.L    D3,D1
	mulu.l	d1,d1:d0	; d0=month*306
	ADDQ.L    #5,D0		; d0=month*306+5
	MOVEQ     #10,D1
	divul.l	d1,d1:d0	; d0=(month*306+5)/10
	ADDQ.L    #1,D2
	SUB.L     D0,D2		; d2=day=1+work-(month*306+5)/10
	MOVE.W    D2,mday(A0)
	MOVE.L    D3,D0
	ADDQ.L    #2,D0
	MOVEQ     #12,D1
	divul.l	d1,d1:d0	; d0=(month+2)/12
	ADDQ.L    #1,D1		; d1=(month+2)%12 +1
	MOVE.W    D1,month(A0)
	ADD.L     (SP)+,D0	; year+=(month+2)/12
	MOVE.W    D0,year(A0)
	MOVEM.L   (A7)+,D2-D3
	RTS


******* utility.library/Date2Amiga ***********************************
*
*   NAME
*	Date2Amiga -- Calculate seconds from 01-Jan-1978.  (V36)
*
*   SYNOPSIS
*	AmigaTime = Date2Amiga( Date )
*	D0                      A0
*
*	ULONG Date2Amiga( struct ClockData * );
*
*   FUNCTION
*	Calculates the number of seconds from 01-Jan-1978 to the date
*	    specified in the ClockData structure.
*
*   INPUTS
*	Date -- Pointer to a ClockData structure containing the date
*	    of interest.
*
*   RESULTS
*	AmigaTime -- The number of seconds from 01-Jan-1978 to the date
*	    specified in Date.
*
*   NOTES
*	This function does no sanity checking of the data in Date.
*
*   SEE ALSO
*	Amiga2Date(), CheckDate()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

;	ULONG __asm Date2Amiga(register __a0 struct ClockData *cd)
;	  {
;	   return (((CalcDate(cd->year,cd->month,cd->day) - CALC_DATE(1978,1,1))
;	         * 24 + cd->hour) * 60 + cd->min) * 60 + cd->sec;
;	  }

Date2AmigaS:
	MOVEM.L   D2-D3,-(A7)
	MOVEQ     #00,D0
	MOVE.W    year(A0),D0
	MOVEQ     #00,D1
	MOVE.W    month(A0),D1
	MOVEQ     #00,D2
	MOVE.W    mday(A0),D2
	BSR       CalcDateS
	MOVEQ     #00,D1
	MOVE.W    sec(A0),D1
	MOVEQ     #00,D2
	MOVE.W    min(A0),D2
	MOVEQ     #00,D3
	MOVE.W    hour(A0),D3
	SUBI.L    #722390,D0
	MOVE.L    D1,-(A7)
	MOVEQ     #24,D1
	JSR       UMult32S
	ADD.L     D3,D0
	MOVEQ     #60,D1
	JSR       UMult32S
	ADD.L     D2,D0
	MOVEQ     #60,D1
	JSR       UMult32S
	MOVE.L    (A7)+,D1
	ADD.L     D1,D0
	MOVEM.L   (A7)+,D2-D3
	RTS

_Date2AmigaH:
	MOVEM.L   D2-D3,-(A7)
	MOVEQ     #00,D0
	MOVE.W    year(A0),D0
	MOVEQ     #00,D1
	MOVE.W    month(A0),D1
	MOVEQ     #00,D2
	MOVE.W    mday(A0),D2
	BSR       CalcDateH
	MOVEQ     #00,D1
	MOVE.W    sec(A0),D1
	MOVEQ     #00,D2
	MOVE.W    min(A0),D2
	MOVEQ     #00,D3
	MOVE.W    hour(A0),D3
	SUBI.L    #722390,D0
	MOVE.L    D1,-(A7)
	MOVEQ     #24,D1
	mulu.l	d1,d0
	ADD.L     D3,D0
	MOVEQ     #60,D1
	mulu.l	d1,d0
	ADD.L     D2,D0
	MOVEQ     #60,D1
	mulu.l	d1,d0
	MOVE.L    (A7)+,D1
	ADD.L     D1,D0
	MOVEM.L   (A7)+,D2-D3
	RTS


******* utility.library/CheckDate ************************************
*
*   NAME
*	CheckDate -- Checks if a ClockData struct contains a leagal date.
*
*   SYNOPSIS
*	AmigaTime = CheckDate( Date )
*	D0                     a0
*
*	ULONG CheckDate( struct ClockData * );
*
*   FUNCTION
*	Determines if the Date is a legal date and returns the number
*	    of seconds to Date from 01-Jan-1978 if it is.
*
*   INPUTS
*	Date -- pointer to a ClockData structure.
*
*   RESULTS
*	AmigaTime -- 0 if Date invalid; otherwise, the number of seconds
*	    to Date from 01-Jan-1978.
*
*   NOTES
*
*   SEE ALSO
*	Amiga2Date(), Date2Amiga()
*
*   BUGS
*	The wday field of the ClockData structure is not checked.
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*   REGISTER USAGE
*
*

CheckDateS:
	lea	-CD_SIZE(sp),sp
	move.l	a0,-(sp)	; save it
	bsr	Date2AmigaS	; get AmigaTime
	move.l	(sp)+,a1	; ClockData
	move.l	sp,a0		; temp ClockData
	movem.l	d0/a1,-(sp)
	bsr	Amiga2DateS	; convert back
	bra.s	CDCommon

_CheckDateH:
	lea	-CD_SIZE(sp),sp
	move.l	a0,-(sp)	; save it
	bsr	_Date2AmigaH	; get AmigaTime
	move.l	(sp)+,a1	; ClockData
	move.l	sp,a0		; temp ClockData
	movem.l	d0/a1,-(sp)
	bsr	_Amiga2DateH	; convert back

CDCommon:
	movem.l	(sp)+,d0/a1
	move.l	sp,a0

; a1 -> ClockData struct passed in
; a0 -> ClockData struct created by converting the passed CD to AmigaTime
;	then converting back to CD
; d0 AmigaTime from passed ClockData struct

; if they are both the same, then the date/time is good

	move.l	#CD_SIZE-3,d1	; check the date (but not wday field)
CDls
	cmpm.b	(a0)+,(a1)+
	bne.s	BadDate		; BZZZZZZZZT, you loose!
	dbf	d1,CDls
CDe
	lea	CD_SIZE(sp),sp
	rts

BadDate
	moveq.l	#0,d0
	bra.s	CDe


	END
