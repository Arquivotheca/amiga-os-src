*************************************************************************
*									*
*	datetime.s							*
*									*
*	this module contains functions for date/time conversion.	*
*									*
*	Copyright (c) 1987, 1988 by Ken Salmon, Arp Authors		*
*	Copyright (c) 1989 by Commodore-Amiga, Inc.			*
*									*
*	Edit History:							*
*		krs 12/14/87	added keyword Tomorrow			*
*				allow single digit for day or		*
*				month value in 'getnumb' routine	*
*		krs 01/09/87	added flag in StrStamp;  indicates	*
*				if day such as Monday, Tuesday...	*
*				should reference last week or next.	*
*				added variable format date input,	*
*				to match output.			*
*		krs 01/11/87	major revision.  multiple arguments	*
*				removed and replaced with single	*
*				pointer to new DateTime structure.	*
*		krs 01/22/87	valid years are now 1978-1999 and	*
*				2000-2045.				*
*									*
*		cdh 9/29/89	Convert for use in dos.library		*
*				See below for summary of changes.	*
*									*
*		rej 11/1/89	modified for compilation with dos	*
*									*
*************************************************************************
*
* Summary of changes made for ARP->DOS:
*
* - Function names "StrtoStamp -> StrtoDate" and "StamptoStr->StamptoDate"
*
* - Calling arguments in register D1 instead of A0
*
* - ARP functions "Strcmp" and "Strncmp" replaced by linked calls.
*
* - DOSBASE is assumed, for dl_Root, rather than needing DosBase(A6)
*
* - Bugs fixed.  String arrays are now initialized to NULL, and invalid
*	input datestamps return error conditions properly.
*
*************************************************************************

	include	"exec/types.i"
	include	"exec/libraries.i"
	include	"dos/dos.i"
	include	"dos/dosextens.i"
	include	"dos/datetime.i"

	xdef	_DateToStr,_StrToDate

EXTR	MACRO
	XREF	\2
_\1	EQU	\2
	ENDM

	EXTR	DosBase,@dosbase
	EXTR	Strcmp,@mystricmp
	EXTR	Strncmp,@mystrnicmp

*************************************************************************

* Macro DOSYSCALL calls a library function if the function is defined,
* or if the lib function is undefined, does "BSR _funcname"

DOSYSCALL	macro
	IFD	_LVO\1
	jsr	_LVO\1(A6)
	ENDC
	IFND	_LVO\1
	bsr	_\1
	ENDC
	ENDM

*************************************************************************


*
*	macro clear -- set register contents to zero
*
*	\1	register name
*
clear	macro
	moveq	#0,\1
	endm
*
*************************************************************************
*									*
*	DateToStr -- DateStamp to ASCII string conversion		*
*									*
*	SYNOPSIS:	error = DateToStr (datetime)			*
*			 d0		       a0 (NOW D1)		*
*									*
*	FUNCTION:	converts a binary date and time into an		*
*			ascii string.					*
*									*
*	INPUTS:		datetime = a DateTime structure, see below	*
*									*
*	RESULTS:	error = zero for success, non-zero if the	*
*				DateStamp represents an illegal date.	*
*									*
*	NOTES:								*
*									*
*	the user supplies the following fields of the DateTime		*
*	structure:							*
*									*
*		dat_Stamp	an AmigaDos DateStamp			*
*									*
*		dat_Format	0:  dd-mmm-yy		(AmigaDOS)	*
*				1:  yy-mm-dd		(international)	*
*				2:  mm-dd-yy		(USA)		*
*				3:  dd-mm-yy		(European)	*
*				anything else treated as 0		*
*									*
*		dat_Flags	DTB_SUBST requests substitution of	*
*				string such as Today, Monday, etc.	*
*				when appropriate.			*
*				DTB_FUTURE ignored by this function	*
*									*
*		dat_StrDay	points to buffer to receive day of	*
*				week string.				*
*									*
*		dat_StrDate	points to buffer to receive the date	*
*				string, as in dat_Format.		*
*									*
*		dat_StrTime	points to buffer to receive the time	*
*				as hh:mm:ss.				*
*									*
*	if any of dat_StrDay, dat_StrDate, or dat_StrTime are null,	*
*	the corresponding string will not be returned.			*
*									*
*************************************************************************
*
*	date/time block, passed to StrToDate() and DateToStr()
*
**** STRUCTURE DATETIME is now in arpbase.i
*
*    STRUCTURE	DateTime,0
*	STRUCT	dat_Stamp,ds_SIZEOF	;DOS DateStamp
*	UBYTE	dat_Format		;controls appearance of dat_StrDate
*	UBYTE	dat_Flags		;see BITDEF's below
*	APTR	dat_StrDay		;day of the week string
*	APTR	dat_StrDate		;date string
*	APTR	dat_StrTime		;time string
*	LABEL	dat_SIZEOF
*
*	flags for dat_Flags
*
*	BITDEF	DT,SUBST,0		;substitute Today, Tomorrow, etc.
*	BITDEF	DT,FUTURE,1		;day of the week is in future
*
*	date format values
*
*FORMAT_DOS	equ	0
*FORMAT_INT	equ	1
*FORMAT_USA	equ	2
*FORMAT_CDN	equ	3
*FORMAT_MAX	equ	FORMAT_CDN
*
*	registers common to StrToDate and DateToStr
*
FMT	equr	d7			;format value * 4
FLG	equr	d6			;flags
TOD	equr	d5			;today's date

DTB	equr	a5			;DateTime block
OFS	equr	a4			;offset pointer
DAT	equr	a3			;DTB + dat_StrDate
*
*	registers used by DateToStr
*
stpregs	reg	d2-d7/a2-a5
*
*	registers used by StrtoDate
*
ENG	equr	a2			;english strings pointer
YER	equr	d4			;binary year
MON	equr	d3			;binary month
DAY	equr	d2			;binary day
HOR	equr	d4			;binary hour
MIN	equr	d3			;binary minute
SEC	equr	d2			;binary second
SEP	equr	d5			;separator		
strregs	reg	d2-d7/a2-a5
*
*	miscellaneous
*
DASH	equ	$2d
COLON	equ	$3a
*
*	entry point.
*
_DateToStr:
	move.l	D1,A0			; cdh 9/30/89 - input arg in D1
	movem.l	stpregs,-(sp)
	bsr	common			;do some initialization
*
*************************************************************************
*									*
*	special case for AmigaDOS date format				*
*									*
*************************************************************************
*
*	if the AmigaDOS format is to be used, and the DateStamp is for
*	a date within the past 7 days, or is in the future, a special
*	string (Today, Yesterday, Friday, etc.) may be substituted in
*	place of what is shown in the template.
*
dosfmt	move.l	(DAT),d2		;maybe caller doesn't want this field
	beq.s	regdate
	btst	#DTB_SUBST,FLG		;substitution option?
	beq.s	regdate			;nope
	move.l	ds_Days(DTB),d0
	lea	today,a0
	sub.l	TOD,d0			;caller's DateStamp - today's
	beq.s	dosmove			;= 0 means today
	lea	tomorr,a0
	cmp.w	#1,d0			;maybe it's tomorrow
	beq.s	dosmove
	lea	future,a0
	bgt.s	dosmove			;some other future date
	lea	yester,a0
	cmp.w	#-1,d0			;maybe it's yesterday
	beq.s	dosmove
	cmp.w	#-7,d0			;maybe it's within the last week
	blt.s	regdate
*
*	the date is before yesterday, but within the last week.  figure
*	out what actual day of the week it is.
*
	bsr	calcday
*
*	a0 now points to a string for the day of the week.
*
dosmove	move.l	D2,a1			;dat_StrDate
1$	move.b	(a0)+,(a1)+		;copy string including null
	bne.s	1$
	bra	hours
*
*************************************************************************
*									*
*	regular date formatting						*
*									*
*************************************************************************
*
*	the DateStamp will now be converted to a regular date string,
*	in one of the formats such as yy-mm-dd.  the OFS pointer locates
*	the start of a list of byte offsets into dat_StrDate for each
*	yy-mm-dd component.
*
*	we initialize the string with a template containing enough dashes
*	and nulls that when the individual yy-mm-dd parts are filled in,
*	the result is a valid string.
*
regdate	move.l	(DAT),d0		;maybe caller doesn't want this field
	beq	hours
	move.l	d0,a1
	lea	tplate,a0		;some dashes and nulls
	moveq	#10-1,d0
1$	move.b	(a0)+,(a1)+		;copy template into dat_StrDate
	dbf	d0,1$
	lea	offsets,OFS		;base of offset list
	add.l	FMT,OFS
*
*	calculate the current year
*
	move.l	ds_Days(DTB),d4		;days since 78-01-01
	cmp.l	#365*100,D4		; About 100 years (cdh 9/30/89)
	 bhs	strserr

	addq.l	#1,d4			;correct offset
	moveq	#2,d5			;counter for leap years
	moveq	#78,d2			;counter for year
year1	move.l	#365,d3			;assume this year has 365 days
	and.b	#$03,d5			;but if leap year...
	bne.s	year2			;(works because 2000 IS a leap year)
	addq.l	#1,d3			;make it 366
year2	cmp.l	d3,d4			;have we found correct year?
	ble.s	year3
	sub.l	d3,d4			;no, reduce by 365 or 366
	addq.w	#1,d2			;try next year
	addq.b	#1,d5			;bump leap year modulo 4
	bra.s	year1
year3	move.l	d2,d1			;year relative to 1900
	cmp.w	#100,d1			;wrap around if >= 2000
	blt.s	1$
	sub.w	#100,d1			; make sure year is 0-99 for output
1$	bsr.s	ofset			;fill in yy
*
*	calculate the current month.
*
	lea	days,a2			;regular table of days in each month
	and.b	#$03,d5
	bne.s	month1
	lea	dayslp,a2		;alternate version for leap year
month1	moveq	#1,d2			;counter for month
month2	cmp.w	(a2),d4			;have we found proper month?
	ble.s	month3
	sub.w	(a2)+,d4		;no, reduce # days
	addq.b	#1,d2			;count month
	bra.s	month2
month3	move.l	d2,d1			;this is the month
	move.l	d1,-(sp)
	bsr.s	ofset			;fill in mm
	move.l	(sp)+,d1
*
*	if the date format is AmigaDOS, overlay the mm with a 3-character
*	alpha mmm.
*
	tst.l	FMT			;do we want 3 alpha or 2 numeric?
	bne.s	curday
	subq.l	#1,d1			;make alpha index relative to zero
	mulu	#3,d1
	lea	emonth,a0		;list of english months
	add.l	d1,a0
	subq.l	#2,a1			;backup over digital mm
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
*
*	the current day is whatever is left over
*
curday	move.l	d4,d1			;this is the day
	bsr.s	ofset			;store 2 ascii digits
	bra.s	hours
*
*************************************************************************
*									*
*	ofset -- locate receiving area in buffer			*
*									*
*************************************************************************
*
*	OFS	pointer to next offset byte
*	a1	(returned) pointer inside dat_StrDate
*
ofset	clear	d0
	move.b	(OFS)+,d0		;byte offset in dat_StrDate
	move.l	(DAT),a1
	add.l	d0,a1			; Slide into binasc
*	bsr.s	binasc
*	rts
*
*************************************************************************
*									*
*	binasc -- output 2 ascii digits					*
*									*
*************************************************************************
*
*	a1	pointer to receiving area
*	d1	binary value to convert (low word)
*	d0	scratch
*
binasc	move.b	#'0',d0			;ascii zero
	ext.l	d1
	divu	#10,d1			;10's digit
	add.b	d0,d1	
	move.b	d1,(a1)+
	swap	d1			;1's digit
	add.b	d0,d1
	move.b	d1,(a1)+
	rts
*
*************************************************************************
*									*
*	convert time from DateStamp to string				*
*									*
*************************************************************************
*
*	calculate hours
*
hours	move.l	dat_StrTime(DTB),d0	;maybe caller doesn't want this field
	beq.s	dayweek
	move.l	d0,a1
	move.l	ds_Minute(DTB),d1	;number of minutes since midnight
	cmp.l	#24*60,D1		; cdh 9/30/89
	 bhs.s	strserr			; unsigned! (carry clear)

	divu	#60,d1			;calculate number of hours
	move.l	d1,d2			;save remainder
	bsr.s	binasc
	move.b	#COLON,(a1)+
*
*	calculate minutes
*
	addq.l	#1,a0			;skip over colon
	move.l	d2,d1			;get minutes
	swap	d1			; remainder from hour calc
	bsr.s	binasc
	move.b	#COLON,(a1)+
*
*	calculate seconds
*
	move.l	ds_Tick(DTB),d1		;number of ticks since minute
	cmp.l	#TICKS_PER_SECOND*60,D1	; cdh 9/30/89
	 bhs.s	strserr			; unsigned!

	divu	#TICKS_PER_SECOND,d1	;calculate number of seconds
	bsr.s	binasc

	clr.b	(A1)			; Null terminate CDH 9/30/89
*
*************************************************************************
*									*
*	set up string for day of the week				*
*									*
*************************************************************************
*
*	locate and return a string for the day of the week
*
dayweek	move.l	dat_StrDay(DTB),d0	;maybe caller doesn't want this field
	beq.s	strsend
	move.l	d0,a1
	bsr.s	calcday
1$	move.b	(a0)+,(a1)+
	bne.s	1$
	clear	d0			;normal return
*
*	end of DateToStr
*
strsend	movem.l	(sp)+,stpregs
	not.l	d0			; invert to normal dos conventions REJ
	rts
strserr	moveq	#-1,d0			;error return
	bra.s	strsend
*
*************************************************************************
*									*
*	calcday -- calculate day of the week				*
*									*
*************************************************************************
*
*	this routine calculates the day of the week corresponding to 
*	ds_Days, and returns the following:
*
*	d0	(lower half) binary index 0 = Sunday, 1 = Monday, etc.
*	a0	pointer to string Sunday, Monday, etc.
*
*	d1 is destroyed.
*
calcday	move.l	ds_Days(DTB),d0		;DateStamp again
	clear	d1			;*** CDH 1/31/88 
	divu	#7,d0
	swap	d0			;remainder is the day we want
	lea	edaysof,a0
	move.b	0(a0,d0.w),d1		;offset from edays for this day string
	lea	edays,a0
	add.w	d1,a0			;pointer to day string
	rts
*
*************************************************************************
*									*
*	common -- common initialization used by StrToDate and		*
*		  DateToStr						*
*									*
*************************************************************************
*
*	initialize DTB, FMT, DAT, and TOD for quick reference
*
common	move.l	a0,DTB			;cache DateTime block
	lea	dat_StrDate(DTB),DAT	;cache pointer to date buffer
	move.b	dat_Flags(DTB),FLG	;cache flags
	clear	FMT
	move.b	dat_Format(DTB),FMT	;cache format type
	blt.s	1$			;if  < 0
	moveq	#FORMAT_MAX,d0
	cmp.l	d0,FMT			;or > limit
	ble.s	2$
1$	clear	FMT			;...substitute zero
2$	lsl.w	#2,FMT			;index * 4
*
* CDH 9/30/89 - fix for DOSBASE in A6 rather than ARPBASE
* REJ 11/1/89 - use internal @dosbase function instead - efficient
*
*	move.l	DosBase(a6),a0		;DosLibrary
	DOSYSCALL DosBase		;get dosbase into d0
	move.l	d0,a6
	move.l	dl_Root(a6),a0		;RootNode
	move.l	rn_Time+ds_Days(a0),TOD	;today's DateStamp
	rts
*
*************************************************************************
*									*
*	StrToDate -- ASCII string to DateStamp conversion		*
*									*
*	SYNOPSIS:	error = StrToDate (datetime)			*
*			  d0		       D1			*
*									*
*	FUNCTION:	converts an ASCII date string into the DOS	*
*			DateStamp format.  see note below.		*
*									*
*	INPUTS:		datetime = a DateTime structure, see below	*
*									*
*	RESULTS:	error  = zero for success, non-zero for		*
*				 error condition.			*
*									*
*	NOTES:								*
*									*
*	the user supplies the following fields of the DateTime		*
*	structure:							*
*									*
*		dat_Stamp	an AmigaDos DateStamp, initial		*
*				contents are ignored.			*
*									*
*		dat_Format	0:  dd-mmm-yy		(AmigaDOS)	*
*				1:  yy-mm-dd		(international)	*
*				2:  mm-dd-yy		(USA)		*
*				3:  dd-mm-yy		(European)	*
*				anything else treated as 0		*
*									*
*		dat_Flags	DTB_SUBST ignored by this function	*
*				DTB_FUTURE if set, means that date	*
*				strings such as Monday, etc. refer	*
*				to "next week"; if not set, these	*
*				refer to past dates.			*
*									*
*		dat_StrDay	ignored by this function		*
*									*
*		dat_StrDate	points to buffer containing the date	*
*				string, as in dat_Format.		*
*									*
*		dat_StrTime	points to buffer containing the time	*
*				string, as hh:mm:ss.			*
*									*
*	dat_StrDate, if present, points to a valid string representing	*
*	the date, such as Today, Tommorrow, Friday, or a string in	*
*	dat_Format style.  this will be converted to the ds_Days	*
*	portion of the DateStamp.					*
*									*
*	dat_StrTime, if present, points to a valid string representing	*
*	the time in format hh:mm:ss.  this will be converted to the	*
*	ds_Minutes and ds_Ticks portions of the DateStamp.		*
*									*
*	if either of these pointers is null, the corresponding		*
*	DateStamp field(s) will not be altered.				*
*									*
*************************************************************************
*
*	entry point
*
_StrToDate:
	move.l	D1,A0			; cdh 9/30/89 - input arg in D1
	movem.l	strregs,-(sp)
	bsr.s	common
*
*	check for Sunday thru Saturday, Today, Yesterday, and Tomorrow
*
	tst.l	(DAT)			;maybe caller doesn't want this field
	beq	sstime
	lea	edays,ENG		;list of english days
	clear	DAY
1$	move.l	(DAT),a0		;caller's string		
	move.l	ENG,a1			;next english day
	DOSYSCALL	Strcmp		;compare, ignoring case
	tst.l	d0
	beq.s	3$			;found a match
2$	tst.b	(ENG)+			;advance to next day string
	bne.s	2$
	addq.b	#1,DAY
	cmp.b	#10,DAY			;give up after Tomorrow
	blt.s	1$
	bra.s	pattern			;try the other pattern
*
*	we matched a day string, and DAY now contains its index.
*	calculate where this day is, relative to today's DateStamp.
*
3$	moveq	#0,d1
	cmp.b	#7,DAY			;today?
	beq.s	adjust
	moveq	#-1,d1
	cmp.b	#8,DAY			;yesterday?
	beq.s	adjust
	bgt.s	cdtomor			;tomorrow?
	move.l	DAY,d1
	move.l	TOD,d0			;today's DateStamp
	divu	#7,d0			;find day of the week
	swap	d0			;remainder 0 = Sunday etc.
	ext.l	d0
	btst	#DTB_FUTURE,FLG
	bne.s	4$
	sub.l	d0,d1			;caller's date - today
	blt.s	adjust
	subq.l	#7,d1			;if +ve, back up to last week
	bra.s	adjust
4$	sub.l	d0,d1
	bgt.s	adjust
	addq.l	#7,d1
	bra.s	adjust
cdtomor	moveq	#1,d1			;Tomorrow
adjust	move.l	TOD,d0			;today's DateStamp again
	add.l	d1,d0			;back up to relative day
	move.l	d0,ds_Days(DTB)
	bra	sstime
*
*************************************************************************
*									*
*	decode yy-mm-dd components (varies with dat_Format)		*
*									*
*************************************************************************
*
pattern	move.l	(DAT),d0		;maybe caller doesn't want this field
	beq	sstime
	move.l	d0,a0
	lea	cmpymd,a2		;component ordering
	add.l	FMT,a2
	moveq	#'-',SEP		;seperator between components
	bsr	decomp
	blt	strerr
	subq.b	#1,MON			;make month relative to zero
	cmp.b	#12,MON
	bge	strerr
*
*	we now have separate year, month, and day components in binary.
*	if the combination makes sense, construct a DateStamp in d0.
*
*	the following year ranges are acceptable:
*
*		78 - 99  (1978 - 1999)
*		00 - 45  (2000 - 2045)
*
	cmp.l	#99,YER			;reject > 99
	bgt	strerr
	moveq	#78,d0
	cmp.l	d0,YER			;1978 or later is OK
	bge.s	1$
	cmp.w	#45,YER
	bgt	strerr			;reject 1946 to 1977 inclusive
	moveq	#-22,d0			;increment from 1978 to 2000
1$	sub.l	d0,YER			;make year relative to 1978
	moveq	#0,d0			;accumulated days
	moveq	#2,d1			;counter for leap year
2$	tst.l	YER			;cycle thru all years after 1978
	beq.s	ckddays
	add.l	#365,d0			;accumulate days for current year
	and.b	#$03,d1			;check for leap year...
	bne.s	3$			; works because 2000 IS a leap year...
	addq.l	#1,d0			;...need an extra day if it is
3$	addq.b	#1,d1			;advance leap year tracker
	subq.l	#1,YER
	bra.s	2$
*
*	add in the number of days in the current month.
*
ckddays	lea	days,a0			;list of days in each month
	and.b	#$03,d1			;if this is leap year...
	bne.s	1$
	lea	dayslp,a0		;...use alternate list
1$	move.l	MON,d1
	lsl.w	#1,d1			;index month * 2
	cmp.w	0(a0,d1.w),DAY		;make sure # days is legit
	bgt.s	strerr
	add.l	DAY,d0			;accumulate days
*
*	accumulate the number of days in prior months during this year.
*
	clear	d1
2$	subq.b	#1,MON			;backup thru months
	blt.s	3$			;that's enough
	move.w	(a0)+,d1		;number of days in this month
	add.l	d1,d0
	bra.s	2$
3$	subq.l	#1,d0			;final adjustment
	move.l	d0,ds_Days(DTB)
*
*************************************************************************
*									*
*	decode hh:mm[:ss] string					*
*									*
*************************************************************************
*
*	hh:mm:ss
*
sstime	move.l	dat_StrTime(DTB),d0	;maybe caller doesn't want this field
	beq.s	strdone
	move.l	d0,a0
	lea	cmphms,a2		;component ordering
	moveq	#COLON,SEP		;seperator between components
	bsr.s	decomp
	blt.s	strerr
	moveq	#23,d1
	cmp.l	d1,HOR			;highest time can be 23:59:59
	bgt.s	strerr
	moveq	#60,d1
	mulu	d1,HOR			;convert hours to minutes
	cmp.l	d1,MIN
	bge.s	strerr
	add.l	HOR,MIN			;add to hours value
	cmp.l	d1,SEC
	bge.s	strerr
	mulu	#50,SEC			;convert seconds to ticks
	move.l	MIN,ds_Minute(DTB)
	move.l	SEC,ds_Tick(DTB)
*
*	end of StrToDate.
*
strdone	moveq	#0,d0			;normal return
strret	movem.l	(sp)+,strregs
	not.l	d0			; convert to normal DOS conventions REJ
	rts
*
*	error return from StrToDate.
*
strerr	moveq	#-1,d0
	bra.s	strret
*
*************************************************************************
*									*
*	decomp -- extract components of a date or time string		*
*									*
*************************************************************************
*
*	a0	date or time string
*	a2	component list
*
decomp	clear	d7
	clear	YER			;initialize YER/MON/DAY HOR/MIN/SEC
	clear	MON
	clear	DAY
1$	move.b	(a2)+,d7		;component offset
	blt.s	dcdone			;last component
	cmp.b	#month,d7		;month may be a string
	bne.s	2$
	bsr.s	getmnth			;see if month string
	bra.s	3$			;valid month
2$	bsr.s	getnumb			;get integer
3$	blt.s	dcerr			;illegal character
	cmp.l	#$7fff,d1		;make sure within range for short
	bgt.s	dcerr
	jmp	4$(pc,d7.w)		;component index
4$	bra.s	5$
	bra.s	6$
	bra.s	7$
	bra.s	5$
	bra.s	6$
	bra.s	7$
5$	move.l	d1,YER			;year or hour
	bra.s	8$
6$	move.l	d1,MON			;month or minute
	bra.s	8$
7$	move.l	d1,DAY			;day or second
*	move.b	d1,0(DTB,d7.w)		;save binary value
8$	cmp.b	d0,SEP			;if separator, continue
	beq.s	1$
	cmp.b	#minute,d7
	bne.s	1$
dcdone	tst.b	d0			;make sure last compent was last
	bne.s	dcerr
	rts
dcerr	moveq	#-1,d0
	rts
*
*************************************************************************
*									*
*	getmnth -- extract month from string				*
*									*
*************************************************************************
*
*	this routine tries to decipher the month component of the date
*	string as 3 alphabetic characters, and return the binary 
*	equivalent for Jan, Feb, etc.  if that fails, it's still
*	possible for the component to be a decimal integer string.
*
*	a0	string pointer
*	d1	binary result.  will be negative if error encountered.
*	SEP	separator character
*	d0	scratch
*	a1	scratch
*
getmnth	move.l	a0,-(sp)		;preserve string
	lea	emonth,a1		;english months
	moveq	#1,d1
1$	move.l	(sp),a0			;caller's string again
	moveq	#3,d0			;only check 3 chars
	movem.l	d1/a1,-(sp)
	DOSYSCALL	Strncmp		;compare 3 bytes ignoring case
	movem.l	(sp)+,d1/a1
	tst.l	d0
	beq.s	gmatch			;found the month
	addq.l	#3,a1			;advance to next month
	addq.b	#1,d1
	cmp.b	#13,d1			;stop after Dec[ember]
	blt.s	1$
*
*	didn't match any 'mmm' string.  try for numeric value.
*
	move.l	(sp),a0			;caller's string again
	bsr.s	getnumb
	bra.s	gmdone
*
*	matched a 'mmm' string.  advance the string pointer to seperator
*	or end of string.
*
gmatch	move.l	(sp),a0			;start of string
1$	move.b	(a0)+,d0		;next character
	beq.s	gmdone
	cmp.b	SEP,d0			;stop at separator
	bne.s	1$
gmdone	addq.l	#4,sp			;restore stack
	tst.l	d1			;set CCR for return value
	rts
*
*************************************************************************
*									*
*	getnumb -- extract next integer from string			*
*									*
*************************************************************************
*
*	a0	string buffer
*	d2	seperator character
*	d1	binary result.  will be negative if error encountered
*	d0	scratch
*
*	on return the CCR will reflect the value in d1.
*	AND d0 will be the first non-converted character!!! (evil evil) REJ
*
getnumb	clear	d1
	move.b	(a0),d0
	beq.s	gnerr			;no characters
1$	move.b	(a0)+,d0		;next character from string
	beq.s	gnret
	cmp.b	SEP,d0			;stop at seperator
	beq.s	gnret
	sub.b	#'0',d0			;convert ascii digit to binary
	blt.s	gnerr
	cmp.b	#9,d0			;make sure it's a valid digit
	bgt.s	gnerr
	mulu	#10,d1			;shift previous value left one digit
	add.b	d0,d1			;accumulate result
	bra.s	1$
gnret	tst.l	d1
	rts
*
*	return any negative value if character illegal
*
gnerr	moveq	#-1,d1
	bra.s	gnret
*
*
*************************************************************************
*									*
*	local data storage						*
*									*
*************************************************************************
*
emonth	dc.b	'JanFebMarAprMayJunJulAugSepOctNovDec'
tplate	dc.b	'-------',0,0,0		;template for dd-mmm-yy
*
*	days in each month.  normal year, followed by leap year.
*
	cnop	0,2
days	dc.w	31,28,31,30,31,30,31,31,30,31,30,31
dayslp	dc.w	31,29,31,30,31,30,31,31,30,31,30,31
*
*	the following 4-byte groups, ordered by format number, give the
*	byte offset of each yy-mm-dd component with a formatted string.
*	used by DateToStr.
*
offsets	dc.b	7,3,0,0			;dd-mmm-yy
	dc.b	0,3,6,0			;yy-mm-dd
	dc.b	6,0,3,0			;mm-dd-yy
	dc.b	6,3,0,0			;dd-mm-yy
*
*	the following 4-byte groups, ordered by format number, give the
*	order of the yy-mm-dd and hh:mm:ss components.  used by StrToDate.
*
year	equ	0*2
month	equ	1*2
day	equ	2*2
hour	equ	3*2
minute	equ	4*2
second	equ	5*2
cmpymd	dc.b	day,month,year,-1
	dc.b	year,month,day,-1
	dc.b	month,day,year,-1
	dc.b	day,month,year,-1
cmphms	dc.b	hour,minute,second,-1
*
*	english days.  the list at 'edaysof' gives the byte offset
*	from 'edays' for each day.
*
*	notice:  do not change the order of the following strings,
*		 all the way down to 'Future'!!!!!!
*
edaysof	dc.b	0,7,14,22,32,41,48
edays	dc.b	'Sunday',0
	dc.b	'Monday',0
	dc.b	'Tuesday',0
	dc.b	'Wednesday',0
	dc.b	'Thursday',0
	dc.b	'Friday',0
	dc.b	'Saturday',0
today	dc.b	'Today',0
yester	dc.b	'Yesterday',0
tomorr	dc.b	'Tomorrow',0
future	dc.b	'Future',0

	end
