**
**	$Id: write.asm,v 36.73 92/12/15 10:44:26 darren Exp $
**
**      workhorse console write command
**
**      (C) Copyright 1985,1989,1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	console

**	Includes

	INCLUDE	"cddata.i"

	INCLUDE	"graphics/text.i"
	INCLUDE	"intuition/intuition.i"
	INCLUDE	"intuition/intuitionbase.i"

**	Exports

	XDEF	CDCBWrite
	XDEF	CDWrite

	XDEF	G0Handler
	XDEF	sgrPrimary
	XDEF	sgrNewMask

**	Imports


	XLVO	Forbid			; Exec
	XLVO	Permit			;
	XLVO	RawDoFmt		;
	XLVO	ObtainSemaphore		;
	XLVO	ReleaseSemaphore	;

	XLVO	Text			; Graphics
	XLVO	SetAPen			;
	XLVO	SetBPen			;
	XLVO	SetDrMd			;
	XLVO	SetSoftStyle		;

	XLVO	InstallClipRegion	; Layers

	XLVO	DisplayBeep		; Intuition

	XREF	PutReadByte

	XREF	EndCommand
	XREF	WriteReset

	XREF	LockDRPort
	XREF	UnLockRPort
	XREF	CursDisable
	XREF	CursEnable
	XREF	CursOn
	XREF	CursOff
	XREF	CursUpdate
	XREF	CursDown
	XREF	CursLeft
	XREF	CursRight
	XREF	CursUp
	XREF	CursRender
	XREF	ScrollYDisplay
	XREF	InsDelChar
	XREF	InsDelLine
	XREF	ClearEOL
	XREF	ClearEOD
	XREF	ClearDisplay
	XREF	RestoreRP
	XREF	Tab
	XREF	BackTab
	XREF	SetTab
	XREF	ClearTab
	XREF	ClearTabs
	XREF	ReSizeUnit
	XREF	RefreshDamage
	XREF	RefreshUnit
	XREF	SelectAbort
	XREF	IfNewSize

	XREF	CDRead

	XREF	GetEvents		; task


**	Assumptions

	IFNE	cu_PNPData-cu_PNPCurr-2
	FAIL	"cu_PNPData does not follow cu_PNPCurr"
	ENDC
	IFNE	RP_INVERSVID-4
	FAIL	"RP_INVERSVID not bit 2, recode"
	ENDC
	IFNE	CUB_IMPLICITNL-7
	FAIL	"CUB_IMPLICITNL not sigh bit, recode"
	ENDC

**	Assumptions

	IFNE	CONU_LIBRARY+1
	FAIL	"constant CONU_LIBRARY not -1"
	ENDC

	IFNE	CONU_STANDARD
	FAIL	"constant CONU_STANDARD not 0"
	ENDC

	IFNE	CONU_CHARMAP&1-1
	FAIL	"low bit not set in constant CONU_CHARMAP"
	ENDC

	IFNE	CONU_SNIPMAP&1-1
	FAIL	"low bit not set in constant CONU_SNIPMAP"
	ENDC

** Special Comments

**	Reserve <CSI><parameters>]
**
**	For third party developers - used by Bill Hawes DisplayHandler
**					--Darren--
**

******* console.device/CMD_WRITE *************************************
*
*   NAME
*	CMD_WRITE -- Write ANSI text to the console display.
*
*   FUNCTION
*	Write a text record to the display.  Interpret the ANSI
*	control characters in the data as described below.  Note
*	that the RPort of the console window is in use while this
*	write command is pending.
*
*   IO REQUEST INPUT
*	io_Message	mn_ReplyPort set if quick I/O is not possible
*	io_Device	preset by the call to OpenDevice
*	io_Unit		preset by the call to OpenDevice
*	io_Command	CMD_WRITE
*	io_Flags	IOF_QUICK if quick I/O possible, else zero
*	io_Length	sizeof(*buffer), or -1 if io_Data is null
*			terminated
*	io_Data		a pointer to a buffer containing the ANSI text
*			to write to the console device.
*
*   IO REQUEST RESULTS
*	io_Error	the error result (no errors are reported as of V36)
*	io_Actual	the number of bytes written from io_Data
*	io_Length	zero
*	io_Data		original io_Data plus io_Actual
*
*   ANSI CODES SUPPORTED
*
*	Codes are specified in the standard fashion for ANSI documents,
*	as the two 4 bit nibbles that comprise the character code,
*	high nibble first, separated by a slash.  Thus 01/11 (ESC) is
*	a character with the hex value 1B (or the decimal value 27).
*
*	A character on the Amiga falls into one of the following four
*	ranges:
*	00/ 0-01/15	C0: ASCII control characters.  See below.
*	02/ 0-07/15	G0: ASCII graphic characters.  These characters
*			have an image that is displayed.  Note that the
*			DEL character is displayed by the Console Device:
*			it is not treated as control character here.
*	08/ 0-09/15	C1: ANSI 3.41 control characters.  See below.
*	10/ 0-15/15	G1: ECMA 94 Latin 1 graphic characters.
*
*	Independent Control Functions (no introducer) --
*	Code	Name	Definition
*	-----	---	----------------------------------------------
*	00/ 7	BEL	BELL: actually an Intuition DisplayBeep()
*	00/ 8	BS	BACKSPACE
*	00/ 9	HT	HORIZONTAL TAB
*	00/10	LF	LINE FEED
*	00/11	VT	VERTICAL TAB
*	00/12	FF	FORM FEED
*	00/13	CR	CARRIAGE RETURN
*	00/14	SO	SHIFT OUT: causes all subsequent G0 (ASCII)
*			characters to be shifted to G1 (ECMA 94/1)
*			characters.
*	00/15	SI	SHIFT IN: cancels the effect of SHIFT OUT.
*	01/11	ESC	ESCAPE
*
*	Code or Esc Name Definition
*	-----	--- ---- ---------------------------------------------
*	08/ 4	D   IND	 INDEX: move the active position down one line.
*	08/ 5	E   NEL	 NEXT LINE
*	08/ 8	H   HTS  HORIZONTAL TABULATION SET
*	08/13	M   RI	 REVERSE INDEX
*	09/11	[   CSI	 CONTROL SEQUENCE INTRODUCER: see next list
*
*	ISO Compatible Escape Sequences (introduced by Esc) --
*	Esc   Name Definition
*	----- ---- ---------------------------------------------------
*	c     RIS  RESET TO INITIAL STATE: reset the console display.
*
*	Control Sequences, with the number of indicated parameters.
*	i.e. <CSI><parameters><control sequence letter(s)>.  Note the
*	last entries consist of a space and a letter.  CSI is either
*	9B or Esc[.  A minus after the number of parameters (#p)
*	indicates less is valid.  Parameters are separated by
*	semicolons, e.g. Esc[14;80H sets the cursor position to row
*	14, column 80.
*	CSI #p	Name Definition
*	--- --- ---- -------------------------------------------------
*	@   1-	ICH  INSERT CHARACTER
*	A   1-	CUU  CURSOR UP
*	B   1-	CUD  CURSOR DOWN
*	C   1-	CUF  CURSOR FORWARD
*	D   1-	CUB  CURSOR BACKWARD
*	E   1-	CNL  CURSOR NEXT LINE
*	F   1-	CPL  CURSOR PRECEDING LINE
*	H   2-	CUP  CURSOR POSITION
*	I   1-	CHT  CURSOR HORIZONTAL TABULATION
*	J   1-	ED   ERASE IN DISPLAY (only to end of display)
*	K   1-	EL   ERASE IN LINE (only to end of line)
*	L   1-	IL   INSERT LINE
*	M   1-	DL   DELETE LINE
*	P   1-	DCH  DELETE CHARACTER
*	R   2	CPR  CURSOR POSITION REPORT (in Read stream only)
*	S   1-	SU   SCROLL UP
*	T   1-	SD   SCROLL DOWN
*	W   n	CTC  CURSOR TABULATION CONTROL
*	Z   1-	CBT  CURSOR BACKWARD TABULATION
*	f   2-	HVP  HORIZONTAL AND VERTICAL POSITION
*	g   1-	TBC  TABULATION CLEAR
*	h   n	SM   SET MODE: see modes below.
*	l   n	RM   RESET MODE: see modes below.
*	m   n	SGR  SELECT GRAPHIC RENDITION
*	n   1-	DSR  DEVICE STATUS REPORT
*	t   1-	aSLPP SET PAGE LENGTH (private Amiga sequence)
*	u   1-	aSLL  SET LINE LENGTH (private Amiga sequence)
*	x   1-	aSLO  SET LEFT OFFSET (private Amiga sequence)
*	y   1-	aSTO  SET TOP OFFSET (private Amiga sequence)
*	{   n	aSRE  SET RAW EVENTS (private Amiga sequence)
*	|   8	aIER  INPUT EVENT REPORT (private Amiga Read sequence)
*	}   n	aRRE  RESET RAW EVENTS (private Amiga sequence)
*	~   1	aSKR  SPECIAL KEY REPORT (private Amiga Read sequence)
*	 p  1-	aSCR  SET CURSOR RENDITION (private Amiga sequence)
*	 q  0	aWSR  WINDOW STATUS REQUEST (private Amiga sequence)
*	 r  4	aWBR  WINDOW BOUNDS REPORT (private Amiga Read sequence)
*	 s  0	aSDSS SET DEFAULT SGR SETTINGS (private Amiga sequence-V39)
*	 v  1	aRAV  RIGHT AMIGA V PRESS (private Amiga Read sequence-V37)
*
*	Modes, set with <CSI><mode-list>h, and cleared with
*	<CSI><mode-list>l, where the mode-list is one or more of the
*	following parameters, separated by semicolons --
*	Mode	Name Definition
*	------- ---- -------------------------------------------------
*	20	LNM  LINEFEED NEWLINE MODE: if a linefeed is a newline
*	>1	ASM  AUTO SCROLL MODE: if scroll at bottom of window
*	?7	AWM  AUTO WRAP MODE: if wrap at right edge of window
*
*    NOTES
*	The console.device recognizes these SGR sequences.
*	Note that some of these are new to V36.
*
*	SGR (SELECT GRAPHICS RENDITION)
*		Selects colors, and other display characteristics
*		for text.
*
*	Syntax:
*		<ESC>[graphic-rendition...m
*
*	Example:
*		<ESC>[1;7m   (sets bold, and reversed text)
*
*	Parameters:
*
*		0	- Normal colors, and attributes
*		1	- Set bold
*		2	- Set faint (secondary color)
*		3	- Set italic
*		4	- Set underscore
*		7	- Set reversed character/cell colors
*		8	- Set concealed mode.
*		22	- Set normal color, not bold	(V36)
*		23	- Italic off			(V36)
*		24	- Underscore off		(V36)
*		27	- Reversed off			(V36)
*		28	- Concealed off			(V36)
*
*		30-37	- Set character color
*		39	- Reset to default character color
*
*		40-47	- Set character cell color
*		49	- Reset to default character cell color
*
*		>0-7	- Set background color		(V36)
*			  Used to set the background color before
*			  any text is written.  The numeric parameter
*			  is prefixed by ">".  This also means that if
*			  you issue an SGR command with more than one
*			  parameter, you must issue the digit only
*			  parameters first, followed by any prefixed
*			  parameters.	  			  
*
*	V39 console.device takes advantage of the ability to mask
*	bitplanes for faster scrolling, clearing, and rendering.
*	The actual number of bitplanes scrolled depends on which
*	colors you set via the SGR sequences.  For those using
*	the defaults of PEN color 1, and cell color 0, console.device
*	only needs to scroll 1 bitplane.  The actual number
*	of bitplanes scrolled is reset when ESCc is sent, and when
*	the console window is entirely cleared (e.g., FF).  In
*	general this should cause no compatability problems, unless
*	you are mixing console rendering with graphic.library calls
*	in the same portions of your window.  Console.device considers
*	the number of bitplanes it must scroll, and the screen display
*	depth so that interleaved bitplane scrolling can be taken
*	advantage of in cases where performance is not significantly
*	affected (interleaved scrolling, and masking are mutually
*	exclusive).  The determination of how many planes to scroll
*	is undefined, and may change in the future.
*
*	V39 console.device supports a new private sequence (aSDSS)
*	intended for use by users who prefer to change their default
*	SGR settings.  When this private Amiga sequence is sent to the
*	console, the current Pen color, Cell color, Text style, and
*	Reverse mode (on or off), are set as defaults.  When ESC[0m
*	is issued, the settings are restored to the preferred settings.
*	ESC[39m, and ESC[49m are likewise affected.  In general
*	applications should not make use of this private sequence as it
*	is intended for users who would normally include it as part of
*	their shell startup script.  The normal defaults are reset
*	when ESCc is issued.
*
*    BUGS
*	Does not correctly display cursor in SuperBitMap layers for
*	versions prior to V36.
*
*	Concealed mode should not be used prior to V39 console.device.
*	Prior to V39 concealed mode masked all rastport output, the
*	effect of which varied.  As of V39, text output is simply
*	hidden by setting the pen colors.  Scrolling, clearing,
*	cursor rendering, etc., are unaffected.  For maximum
*	compatability it is recommended you simply set the colors
*	yourself, and not used concealed mode.
*
*	V36-V37 character mapped mode console.device windows could
*	crash, or behave erratically if you scroll text DOWN more
*	than a full window's worth of text.  This bug has been fixed
*	in V39 console.  The only work-around is to avoid sending
*	scroll down, or cursor up commands which exceed the window
*	rows (this is not a problem for unit 0 console windows).
*
*    SEE ALSO
*	ROM Kernel Manual (Volume 1), exec/io.h
*	
********************************************************************************

CDCBWrite	EQU	-1

CDWrite:
		movem.l d2-d7/a2-a4,-(a7)
		move.l	a1,a3		    ; save I/O Request
		move.l	IO_UNIT(a3),a2
		clr.l	IO_ACTUAL(a3)

CDMulti_Write:
		moveq	#00,d7
		not.b	d7			;255L

		lea	cd_EVSemaphore(a6),a0
		LINKEXE	ObtainSemaphore

		bsr	LockDRPort

		bsr	GetEvents		;handle pending events

		lea	cd_EVSemaphore(a6),a0
		LINKEXE	ReleaseSemaphore

		bsr	IfNewSize		;last moment check for a new size
		beq.s	wCheckSelecting		;which didnt get sent to the inputhandler
		bsr	ReSizeUnit		;task yet - RARE/May never happend

wCheckSelecting:

		;-- check for active selection
		btst	#CUB_SELECTED,cu_Flags(a2)
		beq.s	wSetAttrs
		bsr	SelectAbort

wSetAttrs:
		;-- set RastPort attributes
		move.l	cu_Mask(a2),cd_RastPort+rp_Mask(a6)	;FgPen/BgPen/AOLPen

		move.b	cu_DrawMode(a2),cd_RastPort+rp_DrawMode(a6)
		move.l	cu_Minterms(a2),cd_RastPort+rp_minterms(a6)
		move.l	cu_Minterms+4(a2),cd_RastPort+rp_minterms+4(a6)
		move.l	cu_Font(a2),cd_RastPort+rp_Font(a6)
		move.w	cu_AlgoStyle(a2),cd_RastPort+rp_AlgoStyle(a6)
		move.l	cu_TxHeight(a2),cd_RastPort+rp_TxHeight(a6)
		move.l	cu_TxBaseline(a2),cd_RastPort+rp_TxBaseline(a6)

		move.l	IO_LENGTH(a3),d0
		beq.s	done
		cmpi.l	#-1,d0
		bne.s	loop1

		moveq	#-1,d0
		move.l	IO_DATA(a3),a0
getNum:
		tst.b	(a0)+
		dbeq	d0,getNum
		not.l	d0		; null character is not in string
		move.l	d0,IO_LENGTH(a3)
		beq.s	done

loop1:
		;simple means of handling this problem of windows being
		;too small to render in.  We don't lose old info in the
		;buffer map, but we do ignore any attempts to write data.
		;
		;given ROM space, and time limits, this seems to be a
		;reasonable solution.  1.3 would blow up when you wrote
		;text via console.device too a tiny window.
		;

		btst	#CUB_TOOSMALL,cu_Flags(a2)
		bne.s	done


loop:
		tst.l	d7
		bmi.s	done

		;HACK here to support those who mixed text, and graphics
		;and relied on dumb console.device scrolling, and clearing
		;under <= V34 console.device.  Example: Disney Animator!
		;
		;This hack lies about DisplayXL/YL - sets them to max
		;for each write, hence maximum scrolls are always
		;done for non-character mapped windows (UNIT 0).
		

		btst	#0,cu_DevUnit+3(a2)
		bne.s	mappedunit
		move.l	cu_XMax(a2),cu_DisplayXL(a2)	;and Y
		addq.w	#1,cu_DisplayXL(a2)		;+1 past last possible position

mappedunit:
		move.l	IO_DATA(a3),a0
		clr.w	d0
		move.b	(a0),d0
		move.w	cu_PState(a2),d1
		lsl	#2,d1
		lea	parseTable(pc),a1
		move.l	0(a1,d1.w),a1
		jsr	(a1)
		tst.l	IO_LENGTH(a3)
		bne.s	loop

		moveq	#00,d7				;all done

done:
 		tst.l	cu_CM+cm_AllocSize(a2)
 		beq.s	unlockDR
 		;-- check for LAYERREFRESH generated here
 		move.l	cd_RastPort+rp_Layer(a6),a0
 		move.w	lr_Flags(a0),d0
 		and.w	#LAYERREFRESH,d0
 		beq.s	unlockDR
 		
 		bsr	RefreshDamage

unlockDR:
		bsr	UnLockRPort

	;------ Deal with pending events (layers unlocked)

		bclr	#CUB_CTRLG,cu_States(a2)
		beq.s	nobell

		move.l	cu_Window(a2),a0
		move.l	wd_WScreen(a0),a0
		LINKINT	DisplayBeep

nobell:
		tst.l	d7
		bmi	CDMulti_Write

	;------ check if this satisfied a read
		LINKEXE	Forbid
		move.w	cu_ReadLastIn(a2),d0
		cmp.w	cu_ReadLastOut(a2),d0
		beq.s	readPermit
		move.l	MP_MSGLIST(a2),a1
		tst.l	(a1)
		beq.s	readPermit

		bsr	CDRead

readPermit:
		LINKEXE Permit

		move.l	a3,a1
		movem.l (a7)+,d2-d7/a2-a4
		moveq	#0,d0
		bra	EndCommand

*------ Parse Table --------------------------------------------------

parseTable:
		dc.l	vanillaP    ;normal character
		dc.l	escP	    ;escape pending
		dc.l	escIP	    ;intermediate escape pending
		dc.l	csiPP	    ;CSI parameters pending
		dc.l	csiIP	    ;CSI intermediate pending


E3FpHandler:
CSIIHandler:
E3FtHandler:
E2FpHandler:
nil:
nextChar:
		addq.l	#1,IO_DATA(a3)
		addq.l	#1,IO_ACTUAL(a3)
		subq.l	#1,d7
		subq.l	#1,IO_LENGTH(a3)
		rts


*------ vanillaP -----------------------------------------------------
*
*   vanillaP -- characters not preceeded by an introducer
*
vanillaP:
		move.w	d0,d1
		btst	#7,d0	    ;check if in C0-GL or C1-GR
		bne.s	c1GR
		andi.w	#$0060,d1   ;check if control
		beq.s	c0Ctrl	    ;  go perform C0 control
		move.l	cu_GLHandler(a2),a1
		jmp	(a1)	    ;go perform GL code

c0Ctrl:
		bra	C0Handler   ;go perform C0 code

c1GR:
		andi.w	#$0060,d1   ;check if control
		beq	C1Handler	; go perform C1 code
		bra	G1Handler   ;go perform GR code

*------ escP ---------------------------------------------------------
*
*   escP -- unparameterized escape sequences
*
escP:
		btst	#7,d0	;check for 8-bit characters (an error)
		bne.s	escErr
		btst	#6,d0	;check for Fe, Fs groups
		bne.s	escFeOrFs
		btst	#5,d0	;check for control codes (an error)
		bne.s	escIOrFp

*   00-$1F: a control code
escErr:
		clr.w	cu_PState(a2)
		bra.s	vanillaP    ;invoke VanillaP "recursively"

escIOrFp:
		btst    #4,d0		; check for I group
		bne.s	escFp

*   20-$2F: an intermediate character
		bsr.s	putPIC
		move.w	#cu_PSESCI,cu_PState(a2)    ;update the parse state
		bra	nextChar

*   30-$3F: a private escape sequence
escFp:
		bra	    E2FpHandler	; (there are initially none)

escFeOrFs:
		bclr	#6,d0
		bclr	#5,d0	;check for C1 control sequence
		bne.s	escFs

*   40-$5F: a means of specifying controls from the C1 set
		clr.w	cu_PState(a2)
		bra	C1Handler

*   60-$7F: ISO standard escape sequences
escFs:
		bra	E2FsHandler


;------ putPIC -------------------------------------------------------
putPIC:
		lea	cu_PICNext(a2),a0
		move.w	(a0),d1
		move.b	d0,cu_PICData-cu_PICNext(a0,d1.w)
		addq.w	#1,d1
		cmp.w	#PICBSIZE,d1
		bge.s	ppicDone
		move.w	d1,(a0)
ppicDone:
		rts


*------ escIP --------------------------------------------------------
*
*   escIP -- parameterized escape sequences
*
escIPContd:
		move.l	IO_DATA(a3),a0
		clr.w	d0
		move.b	(a0),d0
escIP:
		btst	#7,d0	;check for 8-bit characters (an error)
		bne	escErr
		btst	#6,d0	;check for Ft group
		bne.s	escIFt
		btst	#5,d0	;check for control codes (an error)
		beq	escErr
		btst	#4,d0	;check for I group
		bne.s	escIFp

*   20-$2F: an intermediate character
		bsr.s	putPIC

		addq.l	#1,IO_DATA(a3)
		addq.l	#1,IO_ACTUAL(a3)
		subq.l	#1,d7
		subq.l	#1,IO_LENGTH(a3)
		bne.s	escIPContd
		rts

*   30-$3F: a private escape sequence
escIFp:
		bsr	E3FpHandler ;(there are initially none)
		bra	clearPS

*   40-$7F: standard parameterized escape sequences
escIFt:
		bsr	E3FtHandler ;(there are initially none)
		bra	clearPS

*------ csiPP --------------------------------------------------------
*
*   csiPP -- gather CSI parameters
*
csiPPContd:
		move.l	IO_DATA(a3),a0
		clr.w	d0
		move.b	(a0),d0
csiPP:
		btst	#7,d0	;check for 8-bit characters (an error)
		bne	escErr
		bclr	#6,d0	;check F group
		bne	csiF
		btst	#5,d0	;check for control codes (an error)
		beq	escErr
		btst	#4,d0	;check for P parameters
		bne.s	csiP

*   20-$2F: an intermediate character
		clr.w	cu_PICNext(a2)
		bsr	putPIC
		move.w	#cu_PSCSII,cu_PState(a2)    ;update the parse state
		bra	nextChar

*   30-$3F: numeric parameters
csiP:
		lea	cu_PNPCurr(a2),a0
		move.w	(a0),d1
		cmp.b	#'9',d0
		bgt.s	csiPDelimit
		move.w	d0,d2
		move.w	cu_PNPData-cu_PNPCurr(a0,d1.w),d0
		cmpi.w	#-5,d0
		bne.s	csiPMore
		moveq	#0,d0
csiPMore:
		and.w	#$0F,d2
		muls	#10,d0
		bmi.s	csiPPrivMul
		add.w	d2,d0
updatePWord:
		move.w	d0,cu_PNPData-cu_PNPCurr(a0,d1.w)

csiPNext:
		addq.l	#1,IO_DATA(a3)
		addq.l	#1,IO_ACTUAL(a3)
		subq.l	#1,d7
		subq.l	#1,IO_LENGTH(a3)
		bne	csiPPContd
		rts

csiPPrivMul:
		sub.w	d2,d0
		bra.s	updatePWord

csiPDelimit:
		sub.w	#';',d0
		bne.s	csiPPrivate
		addq.w	#2,d1
		cmp.w	#PNPBSIZE,d1
		blt.s	csiBoundedPNP
		subq.w	#2,d1
csiBoundedPNP:
		move.w	#-5,cu_PNPData-cu_PNPCurr(a0,d1.w)
		move.w	d1,(a0)
		bra.s	csiPNext

csiPPrivate:
		bmi.s	csiPNext
		neg.w	d0		; initialize parameter for private
		bra.s	updatePWord	;   use -- the collection ranges are:
					;   <    -1[n]*
					;   =    -2[n]*
					;   >    -3[n]*
					;   ?    -4[n]*
					;
					;   nil  -5
					;   eop  -6


*   40-$7F: a completed control sequence, sans any intermediate
csiF:
		bsr.s	eopPNP

		cmpi.b	#$2F,d0
		bhi.s	csiFp
		bsr	CSIHandler

clearPS:
		clr.w	cu_PState(a2)
		rts

csiFp:
		bsr	CSIpHandler
		bra.s	clearPS


eopPNP:
		;--	mark parameter eop and reset cu_PNPCurr for reading
		lea	cu_PNPCurr(a2),a0
		move.w	(a0),d1
		cmp.w	#PNPBSIZE-2,d1
		bge.s	epnpMark
		addq.w	#2,d1
epnpMark:
		move.w	#-6,cu_PNPData-cu_PNPCurr(a0,d1.w)
		clr.w	(a0)
		rts


*------ csiIP --------------------------------------------------------
*
*   csiIP -- gather CSI intermediates
*
csiIPContd:
		move.l	IO_DATA(a3),a0
		clr.w	d0
		move.b	(a0),d0
csiIP:
		btst	#7,d0	;check for 8-bit characters (an error)
		bne	escErr
		bclr	#6,d0	;check F group
		bne.s	csiIF
		btst	#5,d0	;check for control codes (an error)
		beq	escErr
		btst	#4,d0	;check for P parameters
		bne	escErr

*   20-$2F: an intermediate character
		bsr	putPIC

		addq.l	#1,IO_DATA(a3)
		addq.l	#1,IO_ACTUAL(a3)
		subq.l	#1,d7
		subq.l	#1,IO_LENGTH(a3)
		bne.s	csiIPContd
		rts

*   40-$7F: a completed control sequence, with intermediate(s)
csiIF:
		bsr.s	eopPNP

		cmpi.b	#$2F,d0
		bhi.s	csiIFp
		bsr	CSIIHandler
		bra.s	clearPS

csiIFp:
		bsr	CSIIpHandler
		bra.s	clearPS


*------ G0Handler -----------------------------------------------------
*
*   G0Handler -- process vanilla characters from left code table
*
*   NOTE: as an extended capability, this also allows 8th bit set chars
*
G0Handler:
		move.l	a0,a4		;save string start
	    ;-- find the number of contiguous vanilla characters
		move.l	IO_LENGTH(a3),d1
g0Loop:
		move.b	(a0)+,d0
		and.b	#$60,d0		; check if control
		beq.s	gHandler	;
		subq.l	#1,d1
		bne.s	g0Loop		; there are possibly more controls

gHandler:
	    ;-- get number of vanilla characters
		move.l	IO_LENGTH(a3),d2
		sub.l	d1,d2		; get number of chars to output

g1Entry:
		bsr	CursDisable

	    ;-- get the amount of text outputable on the current line
nextLine:
		moveq	#00,d3		; clear upper word
		move.w	cu_XCP(a2),d1
		move.w	cu_XMax(a2),d3
		sub.w	d1,d3
		addq.w	#1,d3
		cmp.l	d3,d2
		bge.s	willWrap
		move.w	d2,d3
willWrap:

	    ;-- set up address registers and output Text
		;-- ensure the text CP is correct
		move.w	cu_XCP(a2),d1
		beq.s	installClipRegPosition
		move.w	d1,d0
		add.w	d3,d0
		cmp.w	cu_XMax(a2),d0
		ble.s	genXOrigin
		;--	the text position indicates to use the clip region
installClipRegPosition:
		;--	    check if already installed
		btst.b	#CDB_CLIPINSTALLED,cd_Flags(a6)
		bne.s	genXOrigin

		;--	    check if algorithmic style forces it
		move.b	cu_AlgoStyle(a2),d0
		and.w	#FSF_BOLD!FSF_ITALIC,d0
		bne.s	installClipReg

		;--	    check if kernless font makes it not necessary
		btst	#CUB_KERNLESS,cu_Flags(a2)
		bne.s	genXOrigin

		;--	install the clip region
installClipReg: 
		bset.b	#CDB_CLIPINSTALLED,cd_Flags(a6)
		move.l	cd_RastPort+rp_Layer(a6),a0
		move.l	cu_ClipRegion(a2),a1
		LINKLAY	InstallClipRegion
		move.l	d0,cd_PrevClipRegion(a6)
		move.w	cu_XCP(a2),d1

genXOrigin:
		lea	cd_RastPort(a6),a1
		mulu	cu_XRSize(a2),d1
		add.w	cu_XROrigin(a2),d1
		move.w	d1,rp_cp_x(a1)
		move.w	cu_YCP(a2),d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		add.w	cu_TxBaseline(a2),d1
		move.w	d1,rp_cp_y(a1)
		moveq	#0,d0
		move.w	d3,d0
		move.l	a4,a0
		LINKGFX Text

	    ;-- fill the buffers if appropriate
		tst.l	cu_CM+cm_AllocSize(a2)	; check if buffered
		beq	updateWriteCount

		movem.l	d2-d5/a4,-(a7)

		;-- build text attribute
		move.b	cu_AlgoStyle(a2),d4
		lsl.w	#CMAS_SOFTSTYLE-CMAS_BGPEN,d4
		or.b	cu_BgPen(a2),d4
		lsl.w	#CMAS_BGPEN,d4
		;	check CONCEALED: set FgPen == BgPen if CONCEALED
		tst.b	cu_ConcealMask(a2)
		beq.s	notConcealed
		or.b	cu_BgPen(a2),d4
		bra.s	chkDrawMode

notConcealed:
		or.b	cu_FgPen(a2),d4

chkDrawMode:
		btst.b	#2,cu_DrawMode(a2)	; RP_INVERSVID
		beq.s	chkImplicit
		bset	#CMAB_INVERSVID,d4

chkImplicit:
		tst.b	cu_Flags(a2)		; btst #CUB_IMPLICITNL,
		bpl.s	setRendered
		bset	#CMAB_IMPLICITNL,d4

setRendered:
		bset	#CMAB_RENDERED,d4

		;used for fill attribute - fill with valid
		;spaces of background color for pen/cell

		move.w	#CMAF_RENDERED,d5		;set rendered bit
		move.b	cu_BgColor(a2),d0
		move.b	d0,d1
		lsl.b	#CMAS_BGPEN,d1
		or.b	d0,d5
		or.b	d1,d5

		;--	ensure buffer is initialized to CP
		movem.w	cu_XCP(a2),d0/d1
		cmp.w	cu_DisplayYL(a2),d1
		blt.s	checkTabbed
		bgt.s	initMapGap
		cmp.w	cu_DisplayXL(a2),d0
		beq	fillBuffer
		blt.s	checkTabbed
		bra.s	initLineGap

		;--	initialize empty lines between Display.L and CP
initMapGap:
		move.w	cu_DisplayYL(a2),d0
		sub.w	d0,d1			; lines count
		lsl.w	#2,d0			; y line start pointer offset
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		add.w	d0,a0
		move.w	cu_DisplayXL(a2),d0	; x offset for first line
		move.w	cu_XMax(a2),d2		; cm_DisplayWidth - 1
		bra.s	initMapYDBF
initMapYLoop:
		move.l	(a0)+,a1
		add.w	d0,a1
		sub.w	d2,d0
		bgt.s	initMapXDone
		neg.w	d0
		add.l	a1,a1
initMapXLoop:
		clr.w	(a1)+
		dbf	d0,initMapXLoop
initMapXDone:
		moveq	#0,d0
initMapYDBF:
		dbf	d1,initMapYLoop

		movem.w	cu_XCP(a2),d0/d1

checkTabbed:
		tst.w	d0
		bne.s	checkLeft
		btst.b	#CUB_TABBED,cu_Flags(a2)
		beq.s	checkLeft
		tst.w	d1
		beq.s	checkLeft
		;--	initialize end of previous line to blanks
		subq.w	#1,d1
		lsl.w	#2,d1
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		add.w	d1,a0
		move.l	(a0),a1
		move.w	cu_CM+cm_DisplayWidth(a2),d0
		add.w	d0,a1
		move.l	a1,a0
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1
		bra.s	tabbedLineDBF
tabbedLineLoop:
		tst.w	-(a1)
		bmi.s	tabbedDone
		move.b	#' ',-(a0)
		move.w	d5,(a1)
tabbedLineDBF:
		dbf	d0,tabbedLineLoop

tabbedDone:
		movem.w	cu_XCP(a2),d0/d1
		bra.s	checkLeft

initLineGap:
		move.w	cu_DisplayXL(a2),d1	; x start offset
		sub.w	d1,d0			; fill count
		exg	d0,d1			; offset in d0, count in d1
		bra.s	fillLineGap

checkLeft:
		tst.w	d0
		beq.s	fillBuffer
		cmp.w	cu_DisplayYL(a2),d1
		blt.s	checkLeftByLooking
		beq.s	checkLeftOnDYL
		move.w	d0,d1
		moveq	#0,d0			; offset in d0, count in d1
		bra.s	fillLineGap

checkLeftOnDYL:
		cmp.w	cu_DisplayXL(a2),d0
		beq.s	fillBuffer
		bgt.s	initLineGap

checkLeftByLooking:
		lsl.w	#2,d1			; y line start pointer offset
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d1.w),a1
		add.w	d0,a1
		add.l	a1,a1
		move.w	d0,d1
checkLeftLoop:
		tst.w	-(a1)
		dbmi	d0,checkLeftLoop
		bmi.s	checkLeftCount
		moveq	#0,d0

checkLeftCount:
		sub.w	d0,d1
		bge.s	fillLineGap

fillBuffer:
		moveq	#0,d1			; zero fill count

		;--	fill to left with spaces
fillLineGap:
		move.w	cu_YCP(a2),d2
		lsl.w	#2,d2			; y line start pointer offset
		move.l	cu_CM+cm_AttrDispLines(a2),a0
		move.l	0(a0,d2.w),a0
		add.w	d0,a0
		move.l	a0,a1
		add.l	cu_CM+cm_AttrToChar(a2),a0
		add.l	a1,a1
		bra.s	fillLineDBF
fillLineLoop:
		move.b	#' ',(a0)+
		move.w	d5,(a1)+
fillLineDBF:
		dbf	d1,fillLineLoop


		;--	set TABBED attribute for first character
		bclr.b	#CUB_TABBED,cu_Flags(a2)
		beq.s	fillBufferDBF
		bset	#CMAB_TABBED,d4
		bra.s	fillBufferDBF


		;--	fill buffer
fillBufferLoop:
		move.b	(a4)+,(a0)+
		move.w	d4,(a1)+
		bclr	#CMAB_TABBED,d4
fillBufferDBF:
		dbf	d3,fillBufferLoop

		movem.l	(a7)+,d2-d5/a4

	    ;-- update the count
updateWriteCount:
		add.l	d3,IO_DATA(a3)
		add.l	d3,IO_ACTUAL(a3)
		sub.l	d3,IO_LENGTH(a3)
		sub.l	d3,d7

	    ;-- update cm_Display.L
		movem.w	cu_XCP(a2),d0/d1
		add.w	d3,d0
		cmp.w	cu_DisplayYL(a2),d1
		blt.s	g0CursUpdate
		bgt.s	g0UpdateDisplayL
		cmp.w	cu_DisplayXL(a2),d0
		ble.s	g0CursUpdate
g0UpdateDisplayL:
		movem.w	d0/d1,cu_DisplayXL(a2)

	    ;-- move cursor right
g0CursUpdate:
		move.w	d3,d0		;one blip per character
		bsr	CursRight

	    ;-- check if more characters
		add.w	d3,a4		;update string pointer
		sub.l	d3,d2		;update vanilla count
		bhi	nextLine

g0CursEnable:
		bra	CursEnable	; bsr ... rts


*------ G1Handler -----------------------------------------------------
*
*   G1Handler -- process vanilla characters from right code table
*
*   NOTE: this sets the most significant bit and handles one at a time
*
G1Handler:
		moveq	#1,d2		; output one character
		move.b	(a0),d0
		or.b	#$80,d0		; set the msb in the character
		move.b	d0,-(a7)
		move.l	a7,a4		; point to 8-bit-set character
		bsr	g1Entry
		addq.l	#2,a7		; move.b ,-(a7) pushes 2 bytes
		rts


*------ C0Handler ----------------------------------------------------
*
*	C0Handler -- execute a control character from the C0 set.
C0Handler:
		add.b	d0,d0
		move.w	c0Table(pc,d0.w),d0
		jmp	c0Table(pc,d0.w)

c0Table:
		dc.w	nil-c0Table ;NUL
		dc.w	nil-c0Table ;SOH
		dc.w	nil-c0Table ;STX
		dc.w	nil-c0Table ;ETX
		dc.w	nil-c0Table ;EOT
		dc.w	nil-c0Table ;ENQ
		dc.w	nil-c0Table ;ACK
		dc.w	c0BEL-c0Table ;Bell
		dc.w	c0BS-c0Table  ;BackSpace
		dc.w	c0HT-c0Table  ;Horizontal Tab
		dc.w	c0LF-c0Table  ;Line Feed
		dc.w	c0VT-c0Table  ;Vertical Tab
		dc.w	c0FF-c0Table  ;Form Feed
		dc.w	c0CR-c0Table  ;Carriage Return
		dc.w	c0SO-c0Table  ;Shift Out
		dc.w	c0SI-c0Table  ;Shift In
		dc.w	nil-c0Table ;DLE
		dc.w	nil-c0Table ;DC1
		dc.w	nil-c0Table ;DC2
		dc.w	nil-c0Table ;DC3
		dc.w	nil-c0Table ;DC4
		dc.w	nil-c0Table ;NAK
		dc.w	nil-c0Table ;SYN
		dc.w	nil-c0Table ;ETB
		dc.w	nil-c0Table ;CAN
		dc.w	nil-c0Table ;EOM
		dc.w	nil-c0Table ;SUB
		dc.w	c0ESC-c0Table ;Escape introducer
		dc.w	nil-c0Table ;File Seperator
		dc.w	nil-c0Table ;Group Seperator
		dc.w	nil-c0Table ;Record Seperator
		dc.w	nil-c0Table ;Unit Seperator

*- - -	BEL  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Bell
*
c0BEL:
		bset	#CUB_CTRLG,cu_States(a2)
		moveq	#00,d7			;break write

		bra	nextChar


*- - -	BS   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   BackSpace
*
c0BS:
		moveq	#1,d0
		bsr	CursLeft
		bra	nextChar

*- - -	HT   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Horizontal Tab
*
c0HT:
		bsr	Tab
		bra	nextChar

*- - -	LF   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*- - -	IND  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Line Feed
*   Index
*
c0LF:
		btst	#(M_LNM&07),cu_Modes+(M_LNM/8)(a2)
		bne.s	NEL
IND:
		bclr	#CUB_IMPLICITNL,cu_Flags(a2)
		bne	nextChar
		moveq	#1,d0
		bsr	CursDown
		bsr	updateRPCP
		bra	nextChar

*- - -	VT   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*- - -	RI   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Vertical Tab
*   Reverse Index
*
c0VT:
RI:
		moveq	#1,d0
		bsr	CursUp
		bra	nextChar

*- - -	FF   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Form Feed
*
c0FF:
		
		clr.l	cu_XCP(a2)   ;set to upper left corner

		lea	cd_RastPort(a6),a1
		;-- ensure the text CP is correct
		move.w	cu_XROrigin(a2),d1
		move.w	d1,rp_cp_x(a1)
		move.w	cu_YROrigin(a2),d1
		add.w	cu_TxBaseline(a2),d1
		move.w	d1,rp_cp_y(a1)
		bsr	ClearDisplay

		;-- Cursor is already cleared, so just redraw
		;-- cursor pattern cleared by ClearDisplay()

		clr.l	cu_XCCP(a2)
		and.b	#(~(CUF_IMPLICITNL!CUF_TABBED))&$ff,cu_Flags(a2)
		bsr	CursRender

		bra	nextChar

*- - -	CPL  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Cursor Previous Line
*
CPL:
		bsr	cu
		bclr	#CUB_IMPLICITNL,cu_Flags(a2)
		bne	nextChar
		bsr	CursUp
		bra.s	c0CR

*- - -	CNL  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Cursor Next Line
*
CNL:
		bsr	cu
		bra.s	nextLines

*- - -	NEL  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   NEw Line
*
NEL:
		moveq	#1,d0
nextLines:
		bclr	#CUB_IMPLICITNL,cu_Flags(a2)
		bne	nextChar
		bsr	CursDown
	;-- that was linefeed, now fall thru to CR

*- - -	CR   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Carriage Return
*
c0CR:
		clr.w	cu_XCP(a2)   ;set to rht margin
		bsr	CursUpdate
		bra	nextChar


*- - -	SO   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Shift Out
*
c0SO:
		move.l	#G1Handler,cu_GLHandler(a2)
		bra	nextChar

*- - -	SI   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Shift In
*
c0SI:
		move.l	#G0Handler,cu_GLHandler(a2)
		bra	nextChar

*- - -	ESC  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Escape
*
c0ESC:
		move.w	#cu_PSESC,cu_PState(a2)
		clr.w	cu_PICNext(a2)
		bra	nextChar

*- - -	HTS  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Horizontal Tabulation Set
*
HTS:
		bsr	SetTab
		bra	nextChar

*- - -	CSI  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Control Sequence Introducer
*
CSI:
		move.w	#cu_PSCSI,cu_PState(a2)
		;	cu_PNPCurr = 0, cu_PNPData(a2) = -5
		move.l	#$0000fffb,cu_PNPCurr(a2)
		bra	nextChar


*------ C1Handler ----------------------------------------------------
*
*	C1Handler -- execute a control character from the C1 set.
C1Handler:
		add.b	d0,d0	    ;also gets rid of msb
		move.w	c1Table(pc,d0.w),d0
		jmp	c1Table(pc,d0.w)

c1Table:
		dc.w	nil-c1Table ; future standard
		dc.w	nil-c1Table ; future standard
		dc.w	nil-c1Table ; future standard
		dc.w	nil-c1Table ; future standard
		dc.w	IND-c1Table ; INDEX: move the active position down one line
		dc.w	NEL-c1Table ; NEXT LINE:
		dc.w	nil-c1Table ; START OF SELECTED AREA:
		dc.w	nil-c1Table ; END OF SELECTED AREA:
		dc.w	HTS-c1Table ; HORIZONTAL TABULATION SET:
		dc.w	nil-c1Table ; HORIZONTAL TABULATION WITH JUSTIFICATION:
		dc.w	nil-c1Table ; VERTICAL TABULATION SET:
		dc.w	nil-c1Table ; PARTIAL LINE DOWN:
		dc.w	nil-c1Table ; PARTIAL LINE UP:
		dc.w	RI-c1Table  ; REVERSE INDEX:
		dc.w	nil-c1Table ; SINGLE SHIFT TWO:
		dc.w	nil-c1Table ; SINGLE SHIFT THREE:
		dc.w	nil-c1Table ; DEVICE CONTROL STRING:
		dc.w	nil-c1Table ; PRIVATE USE ONE:
		dc.w	nil-c1Table ; PRIVATE USE TWO:
		dc.w	nil-c1Table ; SET TRANSMIT STATE:
		dc.w	nil-c1Table ; CANCEL CHARACTER
		dc.w	nil-c1Table ; MESSAGE WAITING
		dc.w	nil-c1Table ; START OF PROTECTED AREA
		dc.w	nil-c1Table ; END OF PROTECTED AREA
		dc.w	nil-c1Table ; future standard
		dc.w	nil-c1Table ; future standard
		dc.w	nil-c1Table ; future standard
		dc.w	CSI-c1Table ; CONTROL SEQUENCE INTRODUCER: see next list
		dc.w	nil-c1Table ; STRING TERMINATOR
		dc.w	nil-c1Table ; OPERATING SYSTEM COMMAND
		dc.w	nil-c1Table ; PRIVACY MESSAGE
		dc.w	nil-c1Table ; APPLICATION PROGRAM COMMAND

*------ E2FsHandler ---------------------------------------------------
*
*   E2FsHandler -- process standard ISO escape sequence
*
E2FsHandler:
		cmpi.b	#3,d0
		beq.s	resetFs	    ; RIS  RESET TO INITIAL STATE

escEnd:
		clr.w	cu_PState(a2)
		bra	nextChar

resetFs:
		bsr	WriteReset
		bra.s	escEnd


*------ CSIHandler ---------------------------------------------------
*
*	CSIHandler -- execute a single character CSI function
*
CSIHandler:
		add.b	d0,d0	    ;also gets rid of msb
		move.w	csiTable(pc,d0.w),d0
		jmp	csiTable(pc,d0.w)

csiTable:
		dc.w	ICH-csiTable	; INSERT CHARACTER
		dc.w	CUU-csiTable	; CURSOR UP
		dc.w	CUD-csiTable	; CURSOR DOWN
		dc.w	CUF-csiTable	; CURSOR FORWARD
		dc.w	CUB-csiTable	; CURSOR BACKWARD
		dc.w	CNL-csiTable	; CURSOR NEXT LINE
		dc.w	CPL-csiTable	; CURSOR PRECEEDING LINE
		dc.w	nil-csiTable	; CURSOR HORIZONTAL ABSOLUTE
		dc.w	CUP-csiTable	; CURSOR POSITION
		dc.w	CHT-csiTable	; CURSOR HORIZONTAL TABULATION
		dc.w	ED-csiTable	; ERASE IN DISPLAY
		dc.w	EL-csiTable	; ERASE IN LINE
		dc.w	IL-csiTable	; INSERT LINE
		dc.w	DL-csiTable	; DELETE LINE
		dc.w	nil-csiTable	; ERASE IN FIELD
		dc.w	nil-csiTable	; ERASE IN AREA
		dc.w	DCH-csiTable	; DELETE CHARACTER
		dc.w	nil-csiTable	; SELECT EDITING EXTENT MODE
		dc.w	nil-csiTable	; CURSOR POSITION REPORT
		dc.w	SU-csiTable	; SCROLL UP
		dc.w	SD-csiTable	; SCROLL DOWN
		dc.w	nil-csiTable	; NEXT PAGE
		dc.w	nil-csiTable	; PREVIOUS PAGE
		dc.w	CTC-csiTable	; CURSOR TABULATION CONTROL
		dc.w	nil-csiTable	; ERASE CHARACTER
		dc.w	nil-csiTable	; CURSOR VERTICAL TABULATION
		dc.w	CBT-csiTable	; CURSOR BACKWARD TABULATION
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	; HORIZONTAL POSITION ABSOLUTE
		dc.w	nil-csiTable	; HORIZONTAL POSITION RELATIVE
		dc.w	nil-csiTable	; REPEAT
		dc.w	nil-csiTable	; DEVICE ATTRIBUTES
		dc.w	nil-csiTable	; VERTICAL POSITION ABSOLUTE
		dc.w	nil-csiTable	; VERTICAL POSITION RELATIVE
		dc.w	HVP-csiTable	; HORIZONTAL AND VERTICAL POSITION
		dc.w	TBC-csiTable	; TABULATION CLEAR
		dc.w	SMo-csiTable	; SET MODE
		dc.w	nil-csiTable	; MEDIA COPY
		dc.w	nil-csiTable	;
		dc.w	nil-csiTable	;
		dc.w	RM-csiTable	; RESET MODE
		dc.w	SGR-csiTable	; SELECT GRAPHIC RENDITION
		dc.w	DSR-csiTable	; DEVICE STATUS REPORT
		dc.w	nil-csiTable	; DEFINE AREA QUALIFICATION

getPNPWord:
		lea	cu_PNPCurr(a2),a0
		move.w	(a0),d1
		move.w	cu_PNPData-cu_PNPCurr(a0,d1.w),d0
		addq.w	#2,d1
		cmp.w	#-6,d0
		beq.s	lastGPNPWord
		move.w	d1,(a0)		; guaranteed non-zero
lastGPNPWord:
		rts

*- - -	CUU  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   CUrsor Up
*
CUU:
		bsr.s	cu
		bsr	CursUp
		bra	nextChar

*- - -	CUD  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   CUrsor Down
*
CUD:
		bsr.s	cu
		bsr	CursDown
		bra	nextChar

cu:
		bsr.s	getPNPWord
		tst.w	d0
		bgt.s	cuSpecified
		moveq	#1,d0		    ;default is 1
cuSpecified:
		rts

*- - -	CUF  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   CUrsor Forward
*
CUF:
		bsr.s	cu
		bsr	CursRight
		bra	nextChar

*- - -	CUB  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   CUrsor Backward
*
CUB:
		bsr.s	cu
		bsr	CursLeft
		bra	nextChar

*- - -	CUP  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*- - -	HVP  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	CUrsor Position
*	Horizontal and Vertical Position
*
CUP:
HVP:
		bsr.s	cu
		cmp.w	cu_YMax(a2),d0
		bgt.s	cupYMax
		subq.w	#1,d0
cupYSet:	
		move.w	d0,cu_YCP(a2)

		bsr.s	cu
		cmp.w	cu_XMax(a2),d0
		bgt.s	cupXMax
		subq.w	#1,d0
cupXSet:	
		move.w	d0,cu_XCP(a2)
		bsr	CursUpdate
		bra	nextChar

cupYMax:
		move.w	cu_YMax(a2),d0
		bra.s	cupYSet

cupXMax:
		move.w	cu_XMax(a2),d0
		bra.s	cupXSet

*- - -	CHT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	Cursor Horizontal Tabulation
*
CHT:
		bsr	cu
		move.w	d0,-(a7)
chtLoop:
		bsr	Tab
		subq.w	#1,(a7)
		bne.s	chtLoop
		addq.w	#2,a7
		bra	nextChar


*- - -	ICH  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Insert CHaracter
*
ICH:
		bsr	cu
		neg.w	d0
		bsr	InsDelChar
		bra	nextChar


*- - -	DCH  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Delete CHaracter
*
DCH:
		bsr	cu
		bsr	InsDelChar
		bra	nextChar


*- - -	IL - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Insert Line
*
IL:
		bsr	cu
		neg.w	d0
		bsr	InsDelLine
		bra	nextChar


*- - -	DL - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Delete Line
*
DL:
		bsr	cu
		bsr	InsDelLine
		bra	nextChar


*- - -	EL - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Erase in Line
*
EL:
		bsr.s	updateRPCP
		bsr	CursDisable
		bsr	ClearEOL
		bsr	CursEnable
		bra	nextChar


*- - -	ED - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Erase in Display
*
ED:
		bsr.s	updateRPCP
		bsr	CursDisable

		bsr	cu
		subq.w	#2,d0
		bne.s	edEOD
		;-- erase entire display
		bsr	ClearDisplay
		bra.s	edCE

edEOD:
		;-- erase to end of display
		bsr	ClearEOD

edCE:
		bsr	CursEnable
		bra	nextChar

*------ updateRPCP ---------------------------------------------------
*
*	ensure the text CP is correct
*
updateRPCP:
		lea	cd_RastPort(a6),a1
		move.w	cu_XCP(a2),d1
		mulu	cu_XRSize(a2),d1
		add.w	cu_XROrigin(a2),d1
		move.w	d1,rp_cp_x(a1)
		move.w	cu_YCP(a2),d1
		mulu	cu_YRSize(a2),d1
		add.w	cu_YROrigin(a2),d1
		add.w	cu_TxBaseline(a2),d1
		move.w	d1,rp_cp_y(a1)
		rts


*- - -	SU   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Scroll Up
*
SU:
		bsr	cu
		lea	cd_RastPort(a6),a1
		bsr	CursDisable
		bsr	ScrollYDisplay
		bsr	CursEnable
		bra	nextChar

*- - -	SD   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Scroll Down
*
SD:
		bsr	cu
		neg.w	d0
		lea	cd_RastPort(a6),a1
		bsr	CursDisable
		bsr	ScrollYDisplay
		bsr	CursEnable
		bra	nextChar


*- - -	CTC  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	Cursor Tabulation Control
*
CTC:
		bsr	getPNPWord
		beq	nextChar
		tst.w	d0
		bmi.s	ctcSetTab
		cmpi.w	#CTC_HSETTAB,d0
		beq.s	ctcSetTab
		cmpi.w	#CTC_HCLRTAB,d0
		beq.s	ctcClrTab
		cmpi.w	#4,d0
		beq.s	ctcClrTabs
		cmpi.w	#CTC_HCLRTABSALL,d0
		bne.s	CTC
ctcClrTabs:
		bsr	ClearTabs
		bra.s	CTC
ctcClrTab:
		bsr	ClearTab
		bra.s	CTC
ctcSetTab:
		bsr	SetTab
		bra.s	CTC


*- - -	CBT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	Cursor Backward Tabulation
*
CBT:
		bsr	cu
		move.w	d0,-(a7)
cbtLoop:
		bsr	BackTab
		subq.w	#1,(a7)
		bne.s	cbtLoop
		addq.w	#2,a7
		bra	nextChar


*- - -	TBC  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	Tabulation Clear
*
TBC:
		bsr	getPNPWord
		beq	nextChar
		tst.w	d0
		bmi.s	tbcClrTab
		cmpi.w	#TBC_HCLRTAB,d0
		beq.s	tbcClrTab
		cmpi.w	#TBC_HCLRTABSALL,d0
		bne.s	TBC

		bsr	ClearTabs
		bra.s	TBC
tbcClrTab:
		bsr	ClearTab
		bra.s	TBC

		
*- - -	SMo  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Mode
*
SMo:
		bsr	getPNPWord
		beq	nextChar
		bsr.s	checkM
		bmi.s	SMo
		bset	d1,0(a0,d0.w)
		bra.s	SMo


checkM:
		tst.w	d0
		bpl.s	stdM
		cmp.w	#PM_ASM,d0
		bne.s	checkAWM
		moveq	#PMB_ASM,d0
		bra.s	validM
checkAWM:
		cmp.w	#PM_AWM,d0
		bne.s	unknownM
		moveq	#PMB_AWM,d0
		bra.s	validM
stdM:
		cmp.w	#M_LNM,d0
		bgt.s	unknownM
validM:
		move.w	d0,d1
		lsr.w	#3,d0
		andi.w	#7,d1
		lea	cu_Modes(a2),a0
		rts
unknownM:
		moveq	#-1,d0
		rts

*- - -	RM   - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Reset Mode
*
RM:
		bsr	getPNPWord
		beq	nextChar
		bsr.s	checkM
		bmi.s	RM
		bclr	d1,0(a0,d0.w)
		bra.s	RM


*- - -	SGR  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Graphic Rendition
*
SGR:
		bsr	getPNPWord
		beq.s	sgrNextChar
		tst.w	d0
		bpl.s	sgrSpecified
		;--	check for ">n": background color
		neg.w	d0
		cmp.w	#5,d0
		beq.s	sgrDefault
		;--	look for 300..363 [0..63], 30..39 [0-9], 3 []
		cmp.w	#364,d0
		bge.s	sgrNextChar	; ignore outside 0..63
		sub.w	#300,d0
		bge.s	sgrSetExplicitBG ; 300..363 [0..63]
		add.w	#300-30,d0
		bmi.s	sgrChkDefaultBG	; could be 3 []
		cmp.w	#10,d0
		bge.s	sgrNextChar	; ignore if not 30..39 [0-9]

		;--	set global background color
sgrSetExplicitBG:
		bset	#CUFB_FIXEDBG,cu_FixedFlags(a2)
sgrSetGlobBG:
		cmp.b	cu_BgColor(a2),d0
		beq.s	sgrNextChar
		move.b	d0,cu_BgColor(a2)

		bsr	sgrNewMask

		tst.l	cu_CM+cm_AllocSize(a2)
		beq.s	sgrNextChar

		bsr	RefreshUnit
sgrNextChar:

		bra	nextChar

sgrChkDefaultBG:
		cmp.w	#3-30,d0	; just ">" ?
		bne.s	sgrNextChar
		;--	    default
		bclr	#CUFB_FIXEDBG,cu_FixedFlags(a2)
		move.b	cu_BgPen(a2),d0
		bra.s	sgrSetGlobBG


		;--	set standard SGR code
sgrDefault:
		moveq	#0,d0		; default is zero

sgrSpecified:
		cmpi.w	#SGR_DEFAULTBG,d0
		bgt.s	SGR

		move.w	d0,d1
		add.b	d1,d1
		move.w	sgrTable(pc,d1.w),d1
		jsr	sgrTable(pc,d1.w)

		bra.s	SGR


sgrTable:
		dc.w	sgrPrimary-sgrTable	; PRIMARY RENDITION
		dc.w	sgrBold-sgrTable	; BOLD
		dc.w	sgrFgFaint-sgrTable	; SECONDARY COLOR
		dc.w	sgrItalic-sgrTable	; ITALIC
		dc.w	sgrUnderscore-sgrTable	; UNDERSCORE
		dc.w	sgrRRts-sgrTable	; SLOW BLINK
		dc.w	sgrRRts-sgrTable	; FAST BLINK
		dc.w	sgrReverse-sgrTable	; NEGATIVE (REVERSE) IMAGE
		dc.w	sgrConcealed-sgrTable	; CONCEALED
		dc.w	sgrRRts-sgrTable	; CROSSED OUT

		dc.w	sgrRRts-sgrTable	; PRIMARY FONT
		dc.w	sgrRRts-sgrTable	; FIRST ALTERNATE FONT
		dc.w	sgrRRts-sgrTable	; SECOND ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; THIRD ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; FOURTH ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; FIFTH ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; SIXTH ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; SEVENTH ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; EIGHTH ALTERNATIVE FONT
		dc.w	sgrRRts-sgrTable	; NINTH ALTERNATE FONT

		dc.w	sgrRRts-sgrTable	; FRAKTUR
		dc.w	sgrRRts-sgrTable	; DOUBLY UNDERLINED
		dc.w	sgrNormal-sgrTable	; NORMAL COLOR, NOT BOLD
		dc.w	sgrNotItalic-sgrTable	; NOT ITALIC, NOT FRAKTUR
		dc.w	sgrNotUnderscore-sgrTable ; NOT UNDERLINED
		dc.w	sgrRRts-sgrTable	; STEADY, NOT BLINKING
		dc.w	sgrRRts-sgrTable	; reserved
		dc.w	sgrPositive-sgrTable	; POSITIVE IMAGE
		dc.w	sgrRevealed-sgrTable	; REVEALED
		dc.w	sgrRRts-sgrTable	; NOT CROSSED OUT

		dc.w	sgrFgColor-sgrTable	; BLACK DISPLAY
		dc.w	sgrFgColor-sgrTable	; RED DISPLAY
		dc.w	sgrFgColor-sgrTable	; GREEN DISPLAY
		dc.w	sgrFgColor-sgrTable	; YELLOW DISPLAY
		dc.w	sgrFgColor-sgrTable	; BLUE DISPLAY
		dc.w	sgrFgColor-sgrTable	; MAGENTA DISPLAY
		dc.w	sgrFgColor-sgrTable	; CYAN DISPLAY
		dc.w	sgrFgColor-sgrTable	; WHITE DISPLAY
		dc.w	sgrRRts-sgrTable	; reserved
		dc.w	sgrFgDefault-sgrTable	; DEFAULT DISPLAY

		dc.w	sgrBgColor-sgrTable	; BLACK BACKGROUND
		dc.w	sgrBgColor-sgrTable	; RED BACKGROUND
		dc.w	sgrBgColor-sgrTable	; GREEN BACKGROUND
		dc.w	sgrBgColor-sgrTable	; YELLOW BACKGROUND
		dc.w	sgrBgColor-sgrTable	; BLUE BACKGROUND
		dc.w	sgrBgColor-sgrTable	; MAGENTA BACKGROUND
		dc.w	sgrBgColor-sgrTable	; CYAN BACKGROUND
		dc.w	sgrBgColor-sgrTable	; WHITE BACKGROUND
		dc.w	sgrRRts-sgrTable	; reserved
		dc.w	sgrBgDefault-sgrTable	; DEFAULT BACKGROUND


* -   -	SGR PRIMARY	-   -   -   -   -   -   -   -   -   -   -   -
*
*	PRIMARY RENDITION
*
sgrPrimary:
	; unset concealed, and reset BgPen for first call through
	; sgrNewMask

		clr.b	cu_ConcealMask(a2)
		clr.b	cu_BgPen(a2)

		move.b	cu_NormDMode(a2),d0
***		moveq	#RP_JAM2,d0
		bsr.s	setDrawMode

		bsr	sgrFgDefault
		bsr	sgrBgDefault

		moveq	#0,d0
		moveq	#FSF_BOLD+FSF_ITALIC+FSF_UNDERLINED,d1

	; if style has not been set, use defaults above

		tst.b	cu_NormStyle(a2)
		beq.s	setSoftStyle		; bsr ... rts

	; else restore using last saved defaults

		move.w	cu_NormAlgoStyle(a2),cu_AlgoStyle(a2)	;&TxFlags
		rts

* -   -	SGR BOLD	-   -   -   -   -   -   -   -   -   -   -   -
*
*	BOLD
*
sgrBold:
		moveq	#FSF_BOLD,d0
setBold:
		moveq	#FSF_BOLD,d1
		bra.s	setSoftStyle

* -   -	SGR ITALIC	-   -   -   -   -   -   -   -   -   -   -   -
*
*	ITALIC
*
sgrItalic:
		moveq	#FSF_ITALIC,d0
setItalic:
		moveq	#FSF_ITALIC,d1
setSoftStyle:

		lea	cd_RastPort(a6),a1
		LINKGFX	SetSoftStyle
		move.w	cd_RastPort+rp_AlgoStyle(a6),cu_AlgoStyle(a2)
sgrRRts:
		rts

* -   -	SGR UNDERSCORE	-   -   -   -   -   -   -   -   -   -   -   -
*
*	UNDERSCORE
*
sgrUnderscore:
		moveq	#FSF_UNDERLINED,d0
setUnderscore:
		moveq	#FSF_UNDERLINED,d1
		bra.s	setSoftStyle

* -   -	SGR NEGATIVE	-   -   -   -   -   -   -   -   -   -   -   -
*
*	NEGATIVE (REVERSE) IMAGE
*
sgrReverse:
		moveq	#RP_JAM2+RP_INVERSVID,d0
setDrawMode:
		move.b	d0,cu_ConcealDRMD(a2)
		tst.b	cu_ConcealMask(a2)
		bne.s	setConcealMD

setDRMD:
		lea	cd_RastPort(a6),a1
		LINKGFX	SetDrMd
		move.b	cd_RastPort+rp_DrawMode(a6),cu_DrawMode(a2)
		move.l	cd_RastPort+rp_minterms(a6),cu_Minterms(a2)
		move.l	cd_RastPort+rp_minterms+4(a6),cu_Minterms+4(a2)
setConcealMD:
		rts

* -   -	SGR CONCEALED	-   -   -   -   -   -   -   -   -   -   -   -
*
*	CONCEALED
*
sgrConcealed:
		move.b	#-1,cu_ConcealMask(a2)

	; cache pens

		move.b	cu_FgPen(a2),cu_ConcealFG(a2)
		move.b	cu_BgPen(a2),cu_ConcealBG(a2)

		moveq	#RP_JAM2,d0
		bsr.s	setDRMD

		move.b	cu_BgPen(a2),d0
		bsr.s	sgrSetAPen

		move.b	cu_BgPen(a2),d0
		bra.s	sgrSetBPen


* -   -	SGR NORMAL  - 	-   -   -   -   -   -   -   -   -   -   -   -
*
*	NORMAL COLOR, NOT BOLD
*
sgrNormal:
		moveq	#0,d0
		bsr.s	setBold
		bra.s	sgrFgDefault

* -   -	SGR NOT ITALIC	-   -   -   -   -   -   -   -   -   -   -   -
*
*	NOT ITALIC, NOT FRAKTUR
*
sgrNotItalic:
		moveq	#0,d0
		bra.s	setItalic

* -   -	SGR NOT UNDERLINED  -   -   -   -   -   -   -   -   -   -   -
*
*	NOT UNDERLINED

*
sgrNotUnderscore:
		moveq	#0,d0
		bra.s	setUnderscore

* -   -	SGR POSITIVE    -   -   -   -   -   -   -   -   -   -   -   -
*
*	NOT INVERSE VIDEO
*
sgrPositive:
		moveq	#RP_JAM2,d0
		bra.s	setDrawMode

* -   -	SGR REVEALED    -   -   -   -   -   -   -   -   -   -   -   -
*
*	NOT CONCEALED
*
sgrRevealed:

		clr.b	cu_ConcealMask(a2)

		move.b	cu_ConcealDRMD(a2),d0
		bsr.s	setDRMD

		move.b	cu_ConcealFG(a2),d0
		bsr.s	sgrSetAPen

		move.b	cu_ConcealBG(a2),d0
		bra.s	sgrSetBPen


* -   -	SGR DISPLAY	-   -   -   -   -   -   -   -   -   -   -   -
*
*	all DISPLAY colors
*
sgrFgFaint:
		moveq	#32,d0
		bra.s	sgrFgColor
sgrFgDefault:
		moveq	#30,d0
		add.b	cu_NormFG(a2),d0
sgrFgColor:
		sub.b	#30,d0
		move.b	d0,cu_ConcealFG(a2)
		tst.b	cu_ConcealMask(a2)
		bne.s	sgrConcealPen
sgrSetAPen:		
		move.b	d0,cu_FgPen(a2)
		lea	cd_RastPort(a6),a1
		LINKGFX	SetAPen
		move.l	cd_RastPort+rp_minterms(a6),cu_Minterms(a2)
		move.l	cd_RastPort+rp_minterms+4(a6),cu_Minterms+4(a2)
		bra.s	sgrNewMask
sgrConcealPen:
		rts

* -   -	SGR BACKGROUND	-   -   -   -   -   -   -   -   -   -   -   -
*
*	all BACKGROUND colors
*
sgrBgDefault:
		;--FIX, confusing BgColor with cell color

		move.b	cu_NormBG(a2),d0

*		moveq	#00,d0
*
*		move.b	cu_BgColor(a2),d0
		bra.s	sgrbcEntry

sgrBgColor:
		sub.b	#40,d0
sgrbcEntry:
		move.b	d0,cu_ConcealBG(a2)
		tst.b	cu_ConcealMask(a2)
		bne.s	sgrConcealPen
sgrSetBPen:
		move.b	d0,cu_BgPen(a2)
		lea	cd_RastPort(a6),a1
		LINKGFX	SetBPen
		move.l	cd_RastPort+rp_minterms(a6),cu_Minterms(a2)
		move.l	cd_RastPort+rp_minterms+4(a6),cu_Minterms+4(a2)

	; fall through

* -   -	SGR NEW MASK	-   -   -   -   -   -   -   -   -   -   -   -
*
*	recalc new drawing masks for faster scroll/clear/draw
*
sgrNewMask:

		move.b	cu_ScrollMask(a2),d0
		or.b	cu_BgPen(a2),d0
		or.b	cu_FgPen(a2),d0
		or.b	cu_BgColor(a2),d0

	; for a mask of 1, always use as is

		cmp.b	#1,d0
		beq.s	sgrsetNewMask

	; if 3 planes, use interleave scrolling (rare case)

		cmp.b	#3,d0
		bhi.s	sgrsetAllMask

	; 2 planes - common case for colors; see if reasonable
	; to mask, or scroll all based on depth of bitmap target

		cmp.b	#3,cu_Depth(a2)
		bhi.s	sgrsetNewMask		;4-8 planes; optimize

	; use mask of $FF (so we can get interleave scrolls)

sgrsetAllMask:
		moveq	#-1,d0

	
sgrsetNewMask:
		
	; use MinMask to disable masking for non V39 screens

		or.b	cu_MinMask(a2),d0

		move.b	d0,cu_ScrollMask(a2)
		move.b	d0,cu_Mask(a2)

		move.b	d0,cd_RastPort+rp_Mask(a6)
		rts


*- - -	DSR  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Device Status Report
*

DSR:
		bsr	getPNPWord
		cmp.w	#DSR_CPR,d0
		bne	nextChar

		LINKEXE	Forbid
		moveq	#0,d0
		move.w  cu_XCP(a2),d0
		addq.w	#1,d0
		move.l	d0,-(a7)
		move.w  cu_YCP(a2),d0
		addq.w	#1,d0
		move.l	d0,-(a7)
		move.l  a7,a1
		lea	dsrFormat(pc),a0
		lea	rawPutBBuff(pc),a2
		LINKEXE	RawDoFmt
		move.l	IO_UNIT(a3),a2
		addq.w	#8,a7
		LINKEXE	Permit
		bra	nextChar


dsrFormat:
		dc.b	$9B
		dc.b	'%ld;%ldR'
		dc.b	0
		ds.w	0

rawPutBBuff:
		tst.b	d0
		beq.s	rpbRts
		movem.l	a2/a6,-(a7)
		move.l	IO_UNIT(a3),a2
		move.l	IO_DEVICE(a3),a6
		bsr	PutReadByte
		movem.l	(a7)+,a2/a6
rpbRts:
		rts


*------ CSIpHandler --------------------------------------------------
*
*	CSIpHandler -- execute a single character private CSI function
*
CSIpHandler:
		sub.b	#$34,d0
		bmi	nextChar
		add.b	d0,d0	    ;also gets rid of msb
		move.w	csipTable(pc,d0.w),d0
		jmp	csipTable(pc,d0.w)

csipTable:

		dc.w	aSLPP-csipTable	; Set Page Length
		dc.w	aSLL-csipTable	; Set Line Length
		dc.w	nil-csipTable	;
		dc.w	nil-csipTable	;
		dc.w	aSLO-csipTable	; Set Left Offset
		dc.w	aSTO-csipTable	; Set Top Offset
		dc.w	nil-csipTable	;
		dc.w	aSRE-csipTable	; Set Raw Events
		dc.w	nil-csipTable	; (report raw events: read only)
		dc.w	aRRE-csipTable	; Reset Raw Events
		dc.w	nil-csipTable	; (report function keys: read only)
		dc.w	nil-csipTable	; ERROR


*- - -	aSLPP  - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Page Length
*
aSLPP:
		bsr	CursDisable
		bsr	getPNPWord
		tst.w	d0
		bmi.s	offSPL
		bset	#CUFB_FIXEDPL,cu_FixedFlags(a2)
		subq.w	#1,d0
		move.w	d0,cu_FixedYMax(a2)
		bra.s	setSize
offSPL:
		bclr	#CUFB_FIXEDPL,cu_FixedFlags(a2)
		bra.s	setSize


*- - -	aSLL - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Line Length
*
aSLL:
		bsr	CursDisable
		bsr	getPNPWord
		tst.w	d0
		bmi.s	offSLL
		bset	#CUFB_FIXEDLL,cu_FixedFlags(a2)
		subq.w	#1,d0
		move.w	d0,cu_FixedXMax(a2)
		bra.s	setSize
offSLL:
		bclr	#CUFB_FIXEDLL,cu_FixedFlags(a2)

setSize:
		bsr	ReSizeUnit
		bsr	CursEnable
		bra	nextChar


*- - -	aSLO - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Left Offset
*
aSLO:
		bsr	CursDisable
		bsr	getPNPWord
		tst.w	d0
		bmi.s	offSLO
		bset	#CUFB_FIXEDLO,cu_FixedFlags(a2)
		move.w	d0,cu_XROrigin(a2)
		bra.s	setSize
offSLO:
		bclr	#CUFB_FIXEDLO,cu_FixedFlags(a2)
		bra.s	setSize


*- - -	aSTO - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Top Offset
*
aSTO:
		bsr	CursDisable
		bsr	getPNPWord
		tst.w	d0
		bmi.s	offSTO
		bset	#CUFB_FIXEDTO,cu_FixedFlags(a2)
		move.w	d0,cu_YROrigin(a2)
		bra.s	setSize
offSTO:
		bclr	#CUFB_FIXEDTO,cu_FixedFlags(a2)
		bra.s	setSize


*- - -	aSRE - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Raw Events
*
aSRE:
		bsr	getPNPWord
		beq	nextChar
		cmp.w	#IECLASS_MAX,d0
		bhi.s	aSRE		; bgt unsigned
		move.w	d0,d1
		lsr.w	#3,d0
		andi.w	#7,d1
		lea	cu_RawEvents(a2),a0
		bset	d1,0(a0,d0.w)
		bra.s	aSRE


*- - -	aRRE - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Reset Raw Events
*
aRRE:
		bsr	getPNPWord
		beq	nextChar
		cmp.w	#IECLASS_MAX,d0
		bhi.s	aRRE		; bgt unsigned
		move.w	d0,d1
		lsr.w	#3,d0
		andi.w	#7,d1
		lea	cu_RawEvents(a2),a0
		bclr	d1,0(a0,d0.w)
		bra.s	aRRE


*------ CSIIpHandler -------------------------------------------------
*
*	CSIIpHandler -- a CSI function with private intermediate(s)
*
CSIIpHandler:
		;--	only handle ' ' intermediate
		cmp.w	#1,cu_PICNext(a2)
		bne	nextChar
		cmpi.b	#' ',cu_PICData(a2)
		bne	nextChar

		sub.b	#$30,d0
		beq.s	aSCR			; SET CURSOR RENDITION
		subq.b	#1,d0
		beq.s	aWSR			; WINDOW STATUS REPORT
		subq.b	#2,d0			; SET DEFAULT SGR SETTINGS
		bne	nextChar


* - - aSDSS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*	Save current FG/BG COLOR as defaults (for defaults, and ESC0m)
*	Save current PEN STYLE(s) & DRAW MODE for ESC[0m
*
aSDSS:
		move.b	cu_FgPen(a2),cu_NormFG(a2)
		move.b	cu_BgPen(a2),cu_NormBG(a2)
		move.b	cu_ConcealDRMD(a2),cu_NormDMode(a2)
		move.w	cu_AlgoStyle(a2),cu_NormAlgoStyle(a2)
		move.b	#1,cu_NormStyle(a2)
		bra	nextChar

*- - -	aSCR - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Set Cursor Rendition
*
aSCR:

		bsr	CursDisable
		bsr	getPNPWord
		tst.w	d0
		bne.s	onSCR
		bsr	CursOff
		bra.s	endSCR
onSCR:
		bsr	CursOn
endSCR:
		bsr	CursEnable
		bra	nextChar


*- - -	aWSR - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*
*   Window Status Report
*
aWSR:
		LINKEXE	Forbid
		moveq	#0,d0
		move.w  cu_XMax(a2),d0
		addq.w	#1,d0
		move.l	d0,-(a7)
		move.w  cu_YMax(a2),d0
		addq.w	#1,d0
		move.l	d0,-(a7)
		move.l  a7,a1
		lea	wsrFormat(pc),a0
		lea	rawPutBBuff(pc),a2
		LINKEXE	RawDoFmt
		move.l	IO_UNIT(a3),a2
		addq.w	#8,a7
		LINKEXE	Permit
		bra	nextChar


wsrFormat:
		dc.b	$9B
		dc.b	'1;1;%ld;%ld r'
		dc.b	0
		ds.w	0


	END
