*******************************************************************************
*
* Rdargs.s
*
*	This file contains assembly sources for the functions
* _ReadArgs(),_FreeArgs,_FindArg(), and _ReadItem().
*
*	Also included are BLIB callouts for the BCPL library interface, with
* callouts _blib_rdargs,_blib_rditem,_blib_findarg
*
* Copyright (c) 1989, Microsmiths Inc, Commodore Inc.
* All Rights Reserved.
*
*	History:
*		9/2/89	cdh Initial version based on C version by RJesup
*		9/4/89	cdh Send initial version + test files to rjesup
*		9/15/89 cdh reworked blib versions, etc.  Source to rjesup.
*		9/19/89	cdh Add "ReadArgs()" callout.  Uses blib_rdargs plus...
*		9/22	cdh Use new DOS LVO's.  Fix a couple bugs.
*		9/25	rej modify to link with dos (no LVO usage)
*		10/5	cdh Several intermediate rewrites based on
*			    shifting specs.  Now "complete".
*		10/9	cdh Added multiargs and reallocating buffers.
*		12/3	cdh Fix /N buffer problem.  Add flag bit options,
*			    also switch to full size buffers.
*
*******************************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
FUNCDEF     MACRO   * function
_LVO\1	    EQU	    FUNC_CNT
FUNC_CNT    SET     FUNC_CNT-6
            ENDM

FUNC_CNT    SET     LIB_NONSTD
	INCLUDE	"exec/exec_lib.i"
	INCLUDE	"exec/libraries.i"
*	INCLUDE	"dos/dos_lib.i"
	INCLUDE	"dos/dos.i"
	INCLUDE	"dos/rdargs.i"

AbsExecBase	EQU	4

EXECCALL macro
	move.l	AbsExecBase,a6
	jsr	_LVO\1(a6)
	ENDM

EXTR	MACRO
	XREF	\2
_\1	EQU	\2
	ENDM

	EXTR	SetIoErr,@SetIoErr
	EXTR	Output,@output
	EXTR	Input,@input
	XREF	@mystricmp
	XREF	@toUpper
	XREF	_Flush
	XREF	_FGetC
	XREF	_UnGetC
	XREF	_VFPrintf
	EXTR	StrToLong,@Atol

*****************************************************************************
*
* cdh 9/7/89
*	This macro allows use of the new dos.library function calls which
* are not yet available as LVO callouts.
*	The macro will call the ROM functions if they have been defined,
* in libraries/dos_lib.i, or if the functions have not yet been defined
* the macro will call them as xref'ed functions by the name of the function
* preceeded by an underscore.
*
*****************************************************************************
SYSCALL	macro
	IFD	_LVO\1
	ERROR!
*	jsr	_LVO\1(A6)
	ENDC
	IFND	_LVO\1
*	ERROR!!! You must supply _LVO\1
*	xref	_\1
	jsr	_\1
	ENDC
	ENDM

SYSJMP	macro
	IFD	_LVO\1
	ERROR!
*	jmp	_LVO\1(A6)
	ENDC
	IFND	_LVO\1
*	ERROR!!! You must supply _LVO\1
*	xref	_\1
	jmp	_\1
	ENDC
	ENDM

* Equated values used for ASCII ReadChar() comparisons
	IFND	TAB
TAB	set	9		; ASCII Tab character
	ENDIF
	IFND	LF
LF	set	10		; ASCII Linefeed
	ENDIF
	IFND	ENDSTREAMCH
ENDSTREAMCH	set	-1
	ENDIF

	xdef	_blib_rdargs,_ReadArgs,_FreeArgs
	xdef	_blib_rditem,_ReadItem
	xdef	_blib_findarg,_FindArg
	xdef	Toupper

**********************************************************************
* Internal representation of the template is an initialized
* BYTE array with the folowing bit definitions
*
* Aaray is a table of / keys used for parsing the template
*
* Note the usage with /F including ARG_TYPEF_KEYWORD as well
* as ARG_TYPEF_REST
**********************************************************************

	BITDEF	ARG_TYPE,REQUIRED,0	; /A
	BITDEF	ARG_TYPE,KEYWORD,1	; /K
	BITDEF	ARG_TYPE,SWITCH,2	; /S
	BITDEF	ARG_TYPE,NUMBER,3	; /N(umber)
	BITDEF	ARG_TYPE,REST,4		; /F(est of line)
	BITDEF	ARG_TYPE,TOGGLE,5	; /T(oggle On/Off)
	BITDEF	ARG_TYPE,MULTI,6	; /M (was /...)
	BITDEF	ARG_TYPE,DONE,7		; Already specified (internal)

Aaray:	dc.b	ARG_TYPEF_REQUIRED,'A'
	dc.b	ARG_TYPEF_KEYWORD,'K'
	dc.b	ARG_TYPEF_SWITCH,'S'
	dc.b	ARG_TYPEF_NUMBER,'N'
	dc.b	ARG_TYPEF_REST,'F'	; +ARG_TYPEF_KEYWORD
	dc.b	ARG_TYPEF_TOGGLE,'T'
	dc.b	ARG_TYPEF_MULTI,'.'
	dc.b	ARG_TYPEF_MULTI,'M'
	dc.b	0,0

* internal-only flags for RDA_Flags - note: avoid collisions with ARP
	BITDEF	RDA,PROMPT_SHOWN,30
* flag to know if we allocated the RDA_Buffer pointer
	BITDEF	RDA,OURBUFFER,29
* mask of internal flags
RDA_INTERNAL_FLAGS	EQU	RDAF_PROMPT_SHOWN!RDAF_OURBUFFER

* Ammount of stack space needed by ReadArgs for temp processing
RA_STACK	equ	MAX_TEMPLATE_ITEMS+4*MAX_MULTIARGS+4

* Buffer size used for allocating string buffer space
VECSIZE		equ	128


**********************************************************************
* Shared register equates for ReadArgs() and blib_rdargs
**********************************************************************

A_Keys		equr	A2	; Pointer to template
A_W		equr	A3	; Pointer to buffer for parsed results
A_Argv		equr	A4	; User's Argv array 

; D2 is a general-purpose "scratch" register, used for calling args etc.
D_IsBCPL	equr	D3

D_Multi		equr	D4	; Multiarg index
D_Size		equr	D5	; Highest_address+1 for A_W buffer.
D_Numbargs	equr	D6	; word sized variable
D_Argno		equr	D7	; word sized variable

; This is a special register equate for the template byte image array
; It could be switched to another address register, if stack usage
; in ReadArgs() changes.  BEWARE THAT SP IS CURRENTLY USED AS
; THIS BASE ADDRESS THROUGHOUT READARGS!!!!
AA_Argimage	equr	SP

**********************************************************************
* FreeArgs( RDA_struct )
*
*	Frees all allocations associated with this RDA struct.
* Currently this is just a linked list of AllocVec allocations
* in the RDA_DAList slot of the struct, but the function call is
* the documented interface, not the data structure.
**********************************************************************
_FreeArgs:
	movem.l	A2/A6,-(sp)
	tst.l	D1
	 beq.s	2$		; Nothing to free.
	move.l	D1,A0

	; used to try to clear only for our buffer.  Broke that and got it
	; backwards.  Now we say you must reset rda_buffer for each and
	; every call.  Argh.
	clr.l	RDA_Buffer(A0)	; we set RDA_Buffer internally

	;-- in case it gets reused, unset our flag bits
	and.l	#RDA_INTERNAL_FLAGS,RDA_Flags(a0)	; careful of address

	move.l	RDA_DAList(A0),A2
	clr.l	RDA_DAList(A0)	; so you can call FreeArgs again safely...

1$:	move.l	A2,A1		; arg for freevec
	move.l	A2,d0		; check for done
	 beq.s	2$
	move.l	(A2),A2		; Get next link
	EXECCALL FreeVec	; a1
	bra.s	1$

2$:	movem.l	(sp)+,A2/A6
	rts


**********************************************************************
* Internal callouts used for allocating buffer space for ReadArgs()
**********************************************************************
GetVec:
	move.l	A6,-(sp)
	move.l	d1,d0
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,D1
	EXECCALL AllocVec	; d0,d1
	move.l	(sp)+,A6
	rts

*
* Note: DAList requires the first longword to be the ptr to the next entry!
*
GimmeAVec:
	move.l	D_IsBCPL,A0		; struct RDArg
	moveq.l	#RDAF_NOALLOC,D0	; if NOALLOC, never allocate more space
	and.l	RDA_Flags(A0),D0
	 bne.s	2$
	addq.l	#4,D1			; add a pointer for linking
	bsr.s	GetVec
	tst.l	D0			; did the alloc succeed?
	 beq.s	2$
	move.l	D0,A0
	move.l	D_IsBCPL,A1		; RDArg
	move.l	RDA_DAList(A1),(A0)	; add to head of list
	move.l	A0,RDA_DAList(A1)
	addq.l	#4,D0			; return ptr to rest of memory
	rts

2$:	moveq.l	#0,D0
	rts

**********************************************************************
* ReadArgs()
*
* See functional spec for documentation.
*
* This entry point sets up the registers and allocations used by
* the shared ReadArgs/blib_rdargs code, and transfers control to
* the shared code.
*
**********************************************************************
_ReadArgs:
	movem.l	D2-D7/A2-A5,-(sp)
	sub.w	#RA_STACK+4,sp		; word gets bumped to long on addrs
					; +4 is for result of RdItem!
	move.l	D1,A_Keys		; template
	move.l	D2,A_Argv		; buffer for destination

	moveq.l	#0,D_Multi		; Initialize "multiarg" index.
					; FIX! + 4 for random safety!
	moveq	#(VECSIZE+4)/2,d1	; for use later
	asl.l	#1,d1			; smaller than move.l

* Check to see if application has provided an optional input
* data structure.  If not, allocate one and initialize it.
	tst.l	D_IsBCPL		; input parameter - struct RDArgs (D3)
	 beq.s	AllocRDArg		; Use specified optional struct.

	move.l	D_IsBCPL,A0		; struct RDArg
	clr.l	RDA_DAList(A0)		; For safety, initialize to NULL.
	tst.l	RDA_Buffer(A0)		; did user supply the buffer?
	 bne.s	have_buffer

	; we must allocate the buffer - d1 has VECSIZE in it
	; Note: GimmeAVec allocates and links into RDA_DAList
	bsr.s	GimmeAVec		; be careful NOT to deallocate the RDArg
	move.l	D_IsBCPL,a0		; get ptr back
	move.l	d0,RDA_Buffer(a0)	; did we succeed?
	 beq	ra_errmem
	bra.s	set_buffer		; common code - set buffer size
	
* Allocate the RDA struct, plus some initial buffer space.
* The allocation is made such that allocations will be
* multiples of VECSIZE bytes for minimal fragmentation.
AllocRDArg:
	moveq	#RDA_SIZEOF+4,d0	; + 4 for the DAList.Next ptr
	add.l	d0,d1			; d1 = RDA_SIZEOF + VECSIZE + 4
	bsr.s	GetVec			; can't use GimmeAVec
	move.l	D0,D_IsBCPL
	 beq	ra_errmem		; Bomb out error

* This moves the ptr past the DAList next ptr
	addq.l	#4,D_IsBCPL		; Advance past hook
	move.l	D_IsBCPL,A0
	lea	RDA_SIZEOF(A0),A1	; Initialize the buffer.
	move.l	A1,RDA_Buffer(A0)
	move.l	D0,RDA_DAList(A0)	; Must track this allocation.

set_buffer:				; common code, see above - careful
					; note: smaller than amount actually
					; allocated! FIX!
; no longer needed - see freeargs
;	bset.b	#RDAB_OURBUFFER-24,RDA_Flags(a0) ; remember we allocated it
	move.l	#VECSIZE,RDA_BufSiz(A0)	; (was wierd)

have_buffer:				; comes here if buffer exists or after
	move.l	RDA_Buffer(A0),A_W	; it's allocated
	move.l	A_W,D_Size
	add.l	RDA_BufSiz(A0),D_Size

	move.l	A_Keys,A0
	bsr	strlen
	bsr.s	ra_initkeys		; Initialize the ARGV image array
	 bne	ra_error		; Bad template! Oh shazbad!

	bra	RA_Merge

***********************************************************************
*
* error = ra_initkeys( A_Keys, D0=strlen(A_Keys) )
*
* This internal subroutine is called by both the blib and the DOS
* versions of ReadArgs() to initialize the byte array of key specifiers
* corresponding to the ARGV template items.
*
* The initialized array is on the stack, starting at 4(sp).
*
* Byte values are initialized to the ARG_TYPE specifiers defined above.
*
* Register "D_Numbargs" is initialized to the number of args present.
*
* The return value, if positive, is an IoErr() return code.
*
***********************************************************************
ra_initkeys:
	movem.l	D3/A_Keys/A3,-(sp)
	lea	16(sp),A3		; Byte array to initialize
	move.l	D0,D2			; Calling arg, template length
	moveq	#0,D_Numbargs
	move.l	#0,D3			; Multi flag.

* Loop here on key values
1$:	addq.w	#1,D_Numbargs

	clr.b	(A3)+
	cmp.w	#MAX_TEMPLATE_ITEMS,D_Numbargs	; FIX? should be long?
	 ble.s	2$
	moveq.l	#ERROR_LINE_TOO_LONG,D2		; not real descriptive
	bra.s	ra_rts				; Overflow!!!!!!!!!

2$:	subq.l	#1,D2
	 blt.s	ra_ki_end

	move.b	(A_Keys)+,D0
	cmp.b	#'/',D0
	 bne.s	10$		; Scan for any slash at end of keyword

	move.b	(A_Keys)+,D0
	subq.l	#1,D2
	bsr	Toupper

	lea	Aaray,A0	; a 2 byte/entry table - flag and character
3$:	move.w	(A0)+,D1	; get both
	 beq.s	10$
	cmp.b	D0,D1		; compare character
	 bne.s	3$
	lsr.w	#8,D1		; get flags from upper byte
	or.b	D1,-1(A3)
	btst.b	#ARG_TYPEB_MULTI,D1
	 beq.s	10$
	bset.b	#0,D3
	 beq.s	10$

	move.l	#ERROR_BAD_TEMPLATE,d2	; we don't allow more than 1 /m!
	bra.s	ra_rts

10$:	cmp.b	#',',D0		; Check for end of keyword ","
	 bne.s	2$		; if not, normal loop point
	bra.s	1$		; End of this keyword, initialize for next

ra_ki_end:
	moveq.l	#0,D2
ra_rts:	movem.l	(sp)+,D3/A_Keys/A3
	rts

**********************************************************************
* status = blib_rdargs( UBYTE *keys, LONG *argv, LONG size )
*   D0   = 			D1          D2        D3
*
* Parses the input command line, which is in the buffered BCPL input
* stream, with the ADOS keyword template "keys", putting the resulting
* parsed keyword values into buffer "argv".  argv buffer is also used
* as the destination for parsed strings, with total length of the
* buffer specified by "size", in BYTES.  This buffer should be long
* enough to allow a longword for each template item, plus string
* storage space for the entire input command string parsed into
* arguments.
*
* The return value of blib_rdargs is ZERO on failure, or a pointer
* to the first free space in "argv" which was not allocated.
*
* This code is shared with the dos.library "ReadArgs()" callout.
*
* Must clear argv array, since BCPL expects it! - REJ
**********************************************************************
_blib_rdargs:
	movem.l	D2-D7/A2-A5,-(sp)
	sub.w	#RA_STACK+4,sp		; .w gets extended to .l

	move.l	D1,A_Keys		; note: BSTR!
	move.l	D2,A_Argv
	move.l	D3,D_Size

	add.l	A_Argv,D_Size		; Needs highest addr, not 
					; length, of the buffer
	moveq.l	#0,D_IsBCPL
	moveq.l	#0,D_Multi		; Initialize "multiarg" index.

	moveq.l	#0,D0			; Initialize the shared Argv
	move.b	(A_Keys)+,D0
	bsr	ra_initkeys		; doesn't assume cstr
	subq.l	#1,A_Keys		; Need to restore for findarg

	move.l	A_Argv,A_W		; Clear ARGV for blib
	move.w	D_Numbargs,D0
3$:	clr.l	(A_W)+
	 dbra	D0,3$

**********************************************************************
* ReadArgs merge point
*
* Here, the argv array is initialized and D_Numbargs is set to the
* number of arguments present in the template.  Loop on input items,
* with a case switch based on RdItem case return values.
*
**********************************************************************
RA_Merge:
ra_Loop:
*
* Semantics of /F: it MUST be the last argument in the template.
* /F is filled when either all previous arguments are filled (..._DONE),
* OR when the first unrecognized item is read.  NOTE that if it has been
* read using rditem, the quotes must be restored, unless they used
* Foo="sdkjfkdjf", where Foo is the /F argument.  Argh.
*
* This also means ? will work right.
*

ra_NormalLoop:
	moveq.l	#-1,D_Argno

* NOTE - SPACE AVAILABLE IN A_W ARRAY IS NOW CALCULATED AS NEEDED.
* This is to allow working with re-allocatable buffers.
* A_W is the working address of the current buffer, D_Size is the
* upper bound.  The function "MyRdItem" sets up the value of "MaxBytes"
* for use in the shared code of the "ReadItem" call.

	bsr	MyRditem	; Sets up needed parameters
	move.l	d0,RA_STACK(sp)	; save result of rditem on stack
				; NOTE! MyRditem modifies d2!!!!!!
	 bne	ra_NotDone

* Case 0, end of input stream
* Scan through the Argv byte-image array to make sure there were no
* unspecified "/A" keywords.
*
* NOTE - clearing unused Arvv items is not needed here now since
* ReadArgs() is speced to need an initialized array.  For BCPL,
* the array has been cleared in the setup code.

* Note use of A_Argv and A_Keys registers here!  (D2 is scratch)

	move.l	AA_Argimage,A_Keys
1$:	subq.w	#1,D_Numbargs
	 blt.s	5$

	moveq	#0,D2			; use D2 as flag for /M/N
	btst.b	#ARG_TYPEB_MULTI,(A_Keys)
	 beq.s	2$

**********************************************************************
* Process the MULTIARG array for return.
* The array must be cloned into allocated buffer space from the
* stack; the buffer is either allocated in existing string
* buffer space after longword allignment, if there is enough room,
* or a new vector is allocated.
**********************************************************************
	move.l	D_Multi,(A_Argv)
	 beq.s	2$		; No args to copy, done!

* See if there is space in the buffer for the array
	addq.l	#3,A_W		; Long allign A_W, first
	move.l	A_W,D0
	lsr.l	#2,D0
	asl.l	#2,D0
	move.l	D0,A_W		; rounds up to next longword
				; leave in d0 for 4$ if we go there

	move.l	D_Multi,D1	; 4x the number of multi-items
	addq.l	#4,D1		; Here's how much space we need (1LW for NULL)

; Note that we're zapping D_Size permanently here, since this is the
; last time it will be used.
	sub.l	A_W,D_Size	; Here's how much space we've got.
	sub.l	D1,D_Size	; Is there enough space for 
	 bhs.s	4$		; Have enough space

	bsr	GimmeAVec	; Need to allocate. Size is in D1
				; gets memory, links into dalist.
				; doesn't touch d2

; d0 either has A_W or the result of GimmeAVec
4$:	move.l	D0,(A_Argv)	; Store the multiarg pointer in the current slot
	 beq.s	ra_errmem	; No memory, punt error!

; Copy over the temporary MultiArg array from the stack into
; the buffered allocation space
	move.l	D0,A1
	move.l	D_Multi,D0	; don't kill D_Multi, so I can test it (REJ)
	lea	MAX_TEMPLATE_ITEMS(AA_Argimage),A0 ; array on stack
3$:	move.l	A1,D_Size	; save the last argument of a multi (REJ)
				; should REALLY use another register! FIX! REJ
	move.l	(A0)+,(A1)+
	subq.l	#4,D0		; was D_Multi (REJ)
	 bgt.s	3$
	clr.l	(A1)		; Array needs null terminator

	;-- handle /M/N, make sure D2 is right
	btst.b	#ARG_TYPEB_NUMBER,(A_Keys)	; check for /M/N
	sne	D2		; set D2 != 0 if this is a /M/N

**********************************************************************
* Finish up processing for this returned slot, check for missing
* /A arguments, and loop on Argv slots.
**********************************************************************
2$:	addq.l	#4,A_Argv
	move.b	(A_Keys)+,D0	; here this is AA_Argimage, current entry
	 bmi.s	1$
	btst.b	#ARG_TYPEB_REQUIRED,D0
	 beq.s	1$

; Unspecified key value ... if bit zero
; is set, it was a "/A" key, thus error

; If there was a multi-arg, AND there are at least two items in the multiarg,
; THEN move the last item from the multiarg to this one.  Ugly, has some holes.
; (mainly when the person specifies the Multi arg explicitely)  -REJ
; (also only works if /M is before /A) - REJ
;
; Don't do this if arg is /K/A, since keyword is required
; Don't do it if the multiarg was  /M/N (?FIX?)

	btst.b	#ARG_TYPEB_KEYWORD,d0	; begin REJ code
	 bne.s	6$

	moveq	#8,D0
	cmp.l	D0,D_Multi		; enough items in /M?
	 blt.s	6$

	tst.l	D2			; was it a /M/N?
	 bne.s	6$			; yes, don't steal

; now steal the last one - D_Size holds a pointer to the last multiarg entry
	move.l	D_Size,A0		; pointer to last non-null multi
	subq.l	#4,D_Size		; move back one, in case of 2+ /A's
	subq.l	#4,D_Multi		; one less multiarg available
	move.l	(A0),-4(A_Argv)		; we already incremented dest-register
	clr.l	(A0)			; remove from multi-arg array
	bra.s	1$			; end REJ code

6$:
	; a real error
	moveq.l	#ERROR_REQUIRED_ARG_MISSING,D2
	bra.s	ra_error

**********************************************************************
* Done processing the output Argv array.  Get outa here, closing
* the door on the way out.
**********************************************************************
5$:	bsr	CS_ReadChar	; we don't care if we're at EOF here
	move.l	D_IsBCPL,D0
	 bne.s	rdargs_rts
	move.l	A_W,D0		; Finish up - clear the EOL character,
	bra.s	rdargs_rts	; and return the Argv buffer pointer

**********************************************************************
* Check for other ReadItem() case values here.
* Good values here are QUOTED or UNQUOTED returns from ReadItem().
**********************************************************************
ra_NotDone:
	subq.l	#1,D0
	 beq.s	ra_case1	; Case 1 is an UNQuoted argument
	subq.l	#1,D0
	 beq	ra_case2	; Case 2 is a Unquoted argument

**********************************************************************
* ReadArgs() error returns
*
* ra_error is ReadArgs error exit.  D2 is value for IoErr()
* Here handle all errors in rdargs.
*
**********************************************************************
ra_err1:
	moveq.l	#ERROR_LINE_TOO_LONG,D2	; For IoErr()
	bra.s	ra_error
ra_err_noarg:
	moveq.l	#ERROR_KEY_NEEDS_ARG,D2
	bra.s	ra_error
ra_errmem:
	moveq.l	#ERROR_NO_FREE_STORE,D2
ra_error:
	bsr	CS_ReadChar	; First, scan to end of line
	cmp.b	#LF,D0		; discarding unused input.
	 beq.s	1$
	cmp.b	#';',D0
	 beq.s	1$
	tst.l	d0		; eof is (LONG) -1
	 bpl.s	ra_error

* Next, if there is anything allocated, it must be freed.
1$:	move.l	D_IsBCPL,D1
	 beq.s	2$
	bsr	_FreeArgs

2$:	move.l	D2,D1		; Set error condition code
	SYSCALL	SetIoErr	; (Error_Line_Too_Long)
	moveq.l	#0,D0		; Return NULL for error
rdargs_rts:
	add	#RA_STACK+4,sp
	movem.l	(sp)+,D2-D7/A2-A5
	rts

**********************************************************************
* Case 1, normal item with no quotes.  These items are handled like
* quoted arguments, except first they are checked to see if they
* are KEYWORD values.
**********************************************************************
ra_case1:
	move.l	A_Keys,D1
	move.l	A_W,D2
	bsr	MyFindArg
	move.w	D0,D_Argno	; Save the Argv #
	 blt	check_qmark	; No match

; It _is_ a keyword (D_Argno has index).
; Get argument type to for any special handling
; Note - numbers are just strings, at this point, no special handling.
; LEAVE D0 ALONE NOW - it is used as Argno << 2 for SWITCH case.

	move.b	0(AA_Argimage,D_Argno.w),D1
	btst.b	#ARG_TYPEB_SWITCH,D1
	 bne.s	arg_is_switch	; Switch, Go For It.

	btst.b	#ARG_TYPEB_REST,D1
	 beq.s	string_follows	; Check for "Rest Of Line Whooser!!!"

*
* Someone specified a /F keyword (foo="bar xyzzy").  Note that any quotes
* remain! FIX!
*
*** HERE, READ IN A /F ARGUMENT, reading everything to end-of-line.

ra_DoRestOfLine:
	moveq	#0,D2

ra_SlashF_loop:
	;-- Loop reading all chars until EOF or newline, and put into
	;-- a single string for /F argument.
	;-- D2 has current size of string read in, A_W has ptr to string storage

	;-- check if the buffer is full
	move.l	D_Size,D0		; end of buffer + 1
	sub.l	A_W,D0			; minus current buffer ptr
	sub.l	D2,D0			; minus number of chars added so far
	 bgt.s	ra_SlashF_DoChar	; ok to add a character

	;-- out of mem in string buffer, expand it
	bsr	NewBuffer		; returns cc's!!
	 beq	ra_errmem

	moveq.l	#0,D0			; NOTE! Newbuffer returns old A_W in A0!
33$:	move.b	(A0)+,0(A_W,D0.l)	; copy over old string buffer into new
	addq.l	#1,D0
	cmp.l	D0,D2
	 bgt.s	33$

ra_SlashF_DoChar:
	;-- Read a char and process it

	bsr	CS_ReadChar
	tst.l	D0
	 bmi.s	ra_SlashF_done		; really an EOF test - REJ
	cmp.b	#LF,D0
	 beq.s	ra_SlashF_done
	move.b	D0,0(A_W,d2.l)		; store char in buffer
	addq.l	#1,D2			; bump # of characters added to buffer
	bra.s	ra_SlashF_loop		; and get another one.

ra_SlashF_done:
	;-- don't strip spaces if it was quoted
	;-- note that if the keyword was specified, it NEVER looks quoted
	;-- so it always strips
	moveq	#ITEM_QUOTED,d0
	cmp.l	RA_STACK(sp),d0
	beq.s	no_space

	;-- we may need to remove a final ' ' from the string, if we got
	;-- here via ra_DoSlashF and there are no other arguments.
strip_loop:
	move.b	-1(A_W,D2.l),d0
	cmp.b	#' ',d0
	beq.s	is_space
	cmp.b	#$09,d0			; tab
	bne.s	no_space
is_space:
	subq.l	#1,d2
	bgt.s	strip_loop		; if d2 <= 0, no space left(????)

no_space:
	clr.b	0(A_W,D2.l)		; null-terminate
	bra	not_qmark		; unreads the LF/EOF for us
					; then goes to ra_storearg
					; then back to ra_Loop, and next time
					; rditem should return 0.
*
* Comes here from ra_case2.  We found an item (not keyword), and a slash-f
* item is the next to be filled.  grab item, put in buffer, then grab the
* rest of the line.
*
* D_Argno is set correctly.
*
* requote  it if there were quotes, add space otherwise
* There's no simple solution to the *N, **, etc problems.
*
ra_DoSlashF:
	move.l	A_W,a0		; A_W points to first char of argument
	bsr	strlen		; get length of argument
	move.l	d0,d2		; used as offset to current end of string
				; can't be empty
	moveq	#ITEM_QUOTED,d0
	cmp.l	RA_STACK(sp),d0
	beq.s	ra_SlashF_loop	; don't add space, don't strip spaces

	moveq	#' ',d0		; safe because it overwrites the \0.
	move.b	d0,0(A_W,d2.l)	; follow first arg by a space (oh well) FIX!
	addq.l	#1,d2		; count the space
	bra.s	ra_SlashF_loop	; go grab rest of line, starting by checking
				; if the space requires us to expand the
				; buffer.

***** "/S" keyword comes through to arg_is_switch
arg_is_switch:
	;-- can't use ra_SetFlag, since it skips the next string
	asl.w	#2,D_Argno	; Switch value, no arg required.
	moveq.l	#-1,D1
	move.l	D1,0(A_Argv,D_Argno.w)
	bra	ra_Loop

**********************************************************************
* Keyword expecting a following string parameter.
**********************************************************************
string_follows:
	bsr	MyRditem	; NOTE!!! Modifies D2!!!!!!!
	cmp	#-2,D0		; Word check is sufficient here.
	 bne.s	2$		; Check for "=" parameter,
	bsr	MyRditem	; if so read another parameter
				; NOTE!!! Modifies D2!!!!!!!
2$:	tst.l	D0		; Make sure the parameter is valid
	 beq	ra_err_noarg	; keyword needs =
	 blt	ra_err1		; line too long
	bra.s	ra_case2	; Continue with CASE 2 code


**********************************************************************
* Argument was not a template keyword.  Check to see if it was
* "?", and the last thing on the command line... if so, reprompt and
* continue parsing with new line of input.
*
* Note that arguments already parsed remain!
**********************************************************************
check_qmark:
	bsr	CS_ReadChar
	tst.l	d0
	 bmi.s	ra_case2	; EOF (don't unread the char)
	cmp.b	#LF,D0
	 bne.s	not_qmark	; Wasn't end of line, no reprompt

	tst.l	D_IsBCPL
	 bne.s	10$
	cmp.w	#$013f,(A_W)	; 01, '?'
	 bne.s	not_qmark		; Parameter not "?"

	lea	bcolonstr,A0	; Need to re-prompt.  Here is the 
	move.l	A_Keys,D1	; BCPL prompt string, for displaying
	lsr.l	#2,D1		; template
	bra.s	30$

10$:	cmp.b	#'?',(A_W)	; Check C string for "?"
	 bne.s	not_qmark
	tst.b	1(A_W)
	 bne.s	not_qmark

	;-- check "should we prompt" flag in structure
	move.l	D_IsBCPL,A1	; rdargs struct
	btst.b	#RDAB_NOPROMPT,RDA_Flags+3(a1)	; careful! btst of mem!
	 bne.s	not_qmark

	lea	colonstr,A0	; C string for displaying template

	move.l	A_Keys,D1	; first time, use template

	;-- careful of bit numbers and offsets!!!
	bset.b	#RDAB_PROMPT_SHOWN-24,RDA_Flags(a1)
	beq.s	30$

	move.l	RDA_ExtHelp(A1),D1
	 bne.s	30$		; Use extended help string instead!
	move.l	A_Keys,D1	; no extended string, re-show template

30$:	movem.l	d1-d3,-(sp)	; a0 has format, d1 has template
	move.l	sp,d3		; pointer to args (D1 is the only arg)
	move.l	a0,d2		; format string
	SYSCALL	Output		; get output filehandle (careful of regs)
	move.l	d0,d1
	SYSCALL	VFPrintf	; Display the template
	movem.l	(sp)+,d1-d3	; restore regs/cleanup stack

	SYSCALL	Output
	move.l	D0,D1
	SYSCALL	Flush		; Flush output (buffered).
	bra	ra_Loop

not_qmark:	bsr	CS_UnReadChar	; Restore the last-read character!

**********************************************************************
* CASE 2 - this is the case for quoted strings, which don't get
* checked for keywords etc as with CASE 1.
* Note that values checked in CASE 1 will fall through to the
* code for CASE 2 after checking for keywords, etc.
*
* The CASE 2 code handles bookeeping for the argv[] array
* and string allocation space.
*
**********************************************************************
ra_case2:
	tst.w	D_Argno
	 bge.s	ra_storearg	; See if the keyword slot is known.

	;-- D_Argno must be -1.w here
	move.l	sp,A0		; Was not a keyword - put it in next free.
1$:	addq.w	#1,D_Argno	; Loop through all Argv key slots.
	moveq.l	#ERROR_TOO_MANY_ARGS,D2
	cmp.w	D_Numbargs,D_Argno
	 bge	ra_error	; All slots are filled.  Error!

	btst.b	#ARG_TYPEB_MULTI,(A0)
	 bne.s	ra_storearg	; Continue to fill the MultiArg?

	move.b	(A0)+,D1
	 bmi.s	1$		; Already filled slot

	btst.b	#ARG_TYPEB_REST,D1
	 bne	ra_DoSlashF	; is there a /F argument out there?
				; if so, read rest of line and store

	btst.b	#ARG_TYPEB_KEYWORD,D1
	 bne.s	1$		; no keyword speced, ignore it

	and.b	#ARG_TYPEF_TOGGLE+ARG_TYPEF_SWITCH,D1	;+ARG_TYPEF_REST REJ
	 bne.s	1$		; No autofill here.

* Auto-fill all other slot types.

**********************************************************************
* Here, D_Argno is set to the desired slot to fill with current arg.
**********************************************************************
ra_storearg:
	;-- d0 must get the value BEFORE the bset of typeb_done!
	move.b	0(AA_Argimage,D_Argno.w),D0
	bset.b	#ARG_TYPEB_DONE,0(AA_Argimage,D_Argno.w)
	asl.w	#2,D_Argno

	tst.l	D_IsBCPL
	 bne.s	ra_StoreNotBcpl

* Handle blib_rdargs storage here.
	move.l	A_W,D1
	lsr.l	#2,D1		; Set D1 to BPTR to string
	move.l	D1,0(A_Argv,D_Argno.w)

	move.b	(A_W),D0	; get value to add to A_W in D0
	and.w	#$00fc,D0
	addq.w	#4,D0
	add.w	D0,A_W		; Common code to store arg and bump A_W
	bra	ra_Loop		; Loop to next keyword.

**********************************************************************
* DOS ReadArgs() storage here.
*
* First check for "/N", "/T", or "/..." slot types for special
* handling.
**********************************************************************

* Subroutine for errors
ra_BadNum:
	moveq.l	#ERROR_BAD_NUMBER,D2	; common bad number code
	bra	ra_error

* Entrypoint....
ra_StoreNotBcpl:
	btst	#ARG_TYPEB_MULTI,D0	; /M and /N depend on d0 being set
	 bne.s	ra_StoreMulti

	;-- see if someone has already specified this argument
	;-- multiargs gets hit N times
	btst	#ARG_TYPEB_DONE,D0
	 beq.s	ra_NotSpeced

	;-- if it's a switch, allow it to be specified multiple times
	btst	#ARG_TYPEB_SWITCH,D0
	 bne.s	ra_NotSpeced

	;-- it was specified already - error out
	move.l	#ERROR_TOO_MANY_ARGS,d2
	bra	ra_error

ra_NotSpeced:
	btst	#ARG_TYPEB_TOGGLE,D0
	 bne	ra_Toggle
	btst	#ARG_TYPEB_NUMBER,D0
	 beq	ra_Vanilla		; No special type, just store.

***** Process a "/N" argument ... 
ra_Number:
	move.l	d0,-(sp)		; save flags for this template entry
	clr.l	-(sp)			; space for number on stack
	move.l	sp,D2
	move.l	A_W,D1
	SYSCALL	StrToLong
	move.l	(sp)+,D1		; number
	move.l	(sp)+,D2		; flags for template (only free reg)
	tst.l	D0
	 ble.s	ra_BadNum		; no chars or error

	;-- make sure all characters were used to by StrToLong...
	move.l	d0,a1			; strlen doesn't touch d1 OR a1!!!!!
	move.l	A_W,a0
	bsr	strlen			; returns len in d0
	cmp.l	a1,d0
	 bne.s	ra_BadNum		; not all chars were digits!

	move.l	A_W,D0			; Longword-align the A_W buffer ptr
	addq.l	#3,D0
	and.l	#$fffffffc,D0
	move.l	D0,A_W
	move.l	D_Size,D0
	sub.l	A_W,D0			; We now have the # of bytes available.
	subq.l	#4,D0			; Is there space for a longword in the
	 bge.s	5$			; buffer?

	bsr	NewBuffer		; returns cc's!!
	  beq	ra_errmem

5$:	;-- have number, now store it, update stuff, and loop
	move.l	D1,(A_W)		; store number in buffer space
	btst	#ARG_TYPEB_MULTI,d2	; is this a /M/N template entry?
	beq.s	10$			; no

	;-- /M/N add to array, then loop 
	lea	MAX_TEMPLATE_ITEMS(AA_Argimage),A0
	move.l	A_W,0(A0,D_Multi.w)	; set /m array entry to point to it
	addq.l	#4,D_Multi
	bra.s	20$			; goto common number_done code

10$:	;-- regular /n
	move.l	A_W,0(A_Argv,D_Argno.w)	; set template entry to point to it
	;-- fall thru
20$:
	addq.l	#4,A_W
	bra.s	ra_Next			; loop

***** Store current string in MULTIARG array.
***** it's on the stack, after the template array of chars
ra_StoreMulti:
	cmp.l	#4*(MAX_MULTIARGS-1),D_Multi
	 bge	ra_err1			; Too many multiargs. Generic.
	btst	#ARG_TYPEB_NUMBER,d0	; still set from above!  Handle /M/N
	 bne.s	ra_Number		; it has code for handling multiargs

	;-- string multiarg - add ptr to array, then skip string and loop
	lea	MAX_TEMPLATE_ITEMS(AA_Argimage),A0
	move.l	A_W,0(A0,D_Multi.w)
	addq.l	#4,D_Multi
	bra.s	ra_SkipString

***** Process a TOGGLE key
***** "/T" keyword comes through to arg_is_toggle
ra_Toggle:				; Handle "Toggle" key
	;-- I decided 0/1 was silly for it to accept
*	lea	one(pc),a0		; Accepts "1/0/Y/N/ON/OFf"
*	bsr.s	ra_compstr		; compare string!
*	 beq.s	ra_SetFlag		; must make sure that's all!
*	lea	zero(pc),a0
*	bsr.s	ra_compstr		; returns with cc's set!
*	 beq.s	ra_ClearFlag

	;-- d2 is offset in array to string...
	moveq	#(yes-Toggle_Table),d2	; Yes
	bsr.s	ra_compstr
	 beq.s	ra_SetFlag
	addq.l	#(no-yes),d2		; No
	bsr.s	ra_compstr
	 beq.s	ra_ClearFlag
	addq.l	#(on-no),d2		; on
	bsr.s	ra_compstr
	 beq.s	ra_SetFlag
	addq.l	#(off-on),d2		; off
	bsr.s	ra_compstr
	 bne	ra_err_noarg		; no argument or bad one: error
	;-- fall through

ra_ClearFlag:
	moveq	#0,d0
	;-- fall through
store_result:
	move.l	d0,0(A_Argv,D_Argno.w)

* Continue here when storing string-type arguments, advancing the
* buffer space pointer to the next free space.

ra_SkipString:
	move.l	A_W,A0
	bsr	strlen
	add.l	D0,A_W
	addq.l	#1,A_W		; Add this len to A_W for next free
ra_Next:
	bra	ra_Loop		; Loop to next keyword.

ra_compstr:
	move.l	A_W,a1			; compare word against A_W
	lea	Toggle_Table(pc,d2.w),a0	; 8 bit displacement
	bsr	@mystricmp
	tst.l	d0			; return cc's set!
	rts

***** Vanilla keyword.
ra_Vanilla:
	move.l	A_W,d0
	bra.s	store_result
ra_SetFlag:
	moveq.l	#-1,D0
	bra.s	store_result


Toggle_Table:
*one:	dc.b	'1',0
*zero:	dc.b	'0',0
yes:	dc.b	'yes',0
no:	dc.b	'no',0
on:	dc.b	'on',0
off:	dc.b	'off',0
	ds.w	0		; align

*****************************************************************************
*
* status = FindArg( char *template, char *kwrd )
*   D0			     D1		   D2
*
* Searches *template to see if keyword *kwrd is in the template.
* If it is, the argument # is returned, otherwise -1 is returned.
*
*****************************************************************************

A_kwrd		equr	A2
A_template	equr	A3

D_klen		equr	D2
D_wp		equr	D3
D_wlen		equr	D4
D_fargno	equr	D5


* MyFindArg is internal entry point for ReadArgs/rdargs
MyFindArg:
	tst.l	D_IsBCPL
	 bne.s	_FindArg

* _blib_FindArg is the BCPL entry point for "findarg".
* Enter here with APTR to BSTR "keys" in D1,
*		  APTR to BSTR "w" in D2

_blib_findarg:
	movem.l	D2-D5/A2/A3,-(sp)

	move.l	D1,A_template
	move.l	D2,A_kwrd

	moveq.l	#0,D_wlen
	move.b	(A_kwrd)+,D_wlen
	moveq.l	#0,D_klen
	move.b	(A_template)+,D_klen	; D_klen is only used for BCPL
	addq	#1,D_klen
	bra.s	findarg_common

* _FindArg is the dos.library entry point, as defined above
_FindArg:
	movem.l	D2-D5/A2/A3,-(sp)

	move.l	D1,A_template
	move.l	D2,A_kwrd

	move.l	A_kwrd,A0
	bsr	strlen
	move.l	D0,D_wlen		; Need length of keyword

	moveq	#-1,D_klen		; This length is ignored for Cstr

findarg_common:
	moveq.l	#0,D_fargno

* Reset to start of keyword string
findarg_reset:
	moveq.l	#0,D_wp

* Loop here through the template string, looking for a match against
* the keyword string.
* FIX! might be more efficient to keep lengths around, and use strnicmp!
findarg_loop:
	move.b	(A_template)+,D0	; Get next template character
	 beq.s	findarg_tempend		; end of template, go check for match
	subq	#1,D_klen		; Check for BCPL end of template string
	 beq.s	findarg_tempend		; May be a match, go see

	cmp.b	#',',D0
	 beq.s	findarg_check
	cmp.b	#'=',D0			; Check for separators in the
	 beq.s	findarg_check		; template string.
	cmp.b	#'/',D0
	 beq.s	findarg_check

	bsr	Toupper		; Check for match of template char
	move.b	D0,-(sp)
	move.b	0(A_kwrd,D_wp),D0
	bsr	Toupper
	addq.l	#1,D_wp
	cmp.b	(sp)+,D0
	 beq.s	findarg_loop		; Loop while matching

* No match on this template item.
* Loop here skipping in template string until the next template
* keyword begins.  This is different than the BCPL implementation
* which used a "MATCHING/SKIPPING" state variable.
findarg_skip:
	move.b	(A_template)+,D0	; Get next template char, loop
	 beq.s	findarg_notfound	; End of C-template
	subq	#1,D_klen
	 beq.s	findarg_notfound	; Got to end of template, no match.

* Entry point for findarg_skip with char already in D0
findarg_skip_char:
	cmp.b	#'=',D0
	 beq.s	1$
	cmp.b	#',',D0
	 bne.s	findarg_skip

	addq.l	#1,D_fargno		; Bump to next "ArgNo"
1$:	bra.s	findarg_reset		; and reset loop for next try

*
* Reached separator character in key.
* If at end of keyword, match, else enter skip loop.
*
* NOTE: usage of D_wp does NOT guarantee a BCPL key won't scan past
*	the end of keyword "w", but DOES guarantee correct return value.
*	With C strings, the character comparison will ensure termination
*	at end of "w".
*
findarg_check:
	cmp	D_wp,D_wlen
	 beq.s	findarg_argno
	bra.s	findarg_skip_char

* Reached end of template.
* If at end of keyword, match, else return "no match".
* See note above on D_wp
findarg_tempend:
	cmp	D_wp,D_wlen
	 beq.s	findarg_argno

findarg_notfound:
	moveq.l	#-1,D_fargno
findarg_argno:
	move.l	D_fargno,D0
	movem.l	(sp)+,D2-D5/A2/A3
	rts

*****************************************************************************
* status =  ReadItem(UBYTE *commandname, LONG maxchars, struct CSource *CSource)
*   D0		                D1		D2	      D3
*
*	Reads the next item from the input stream, into the buffer
* commandname which is limited to maxchars bytes in length.  The
* resulting string is saved as a BSTR if isbcpl, else is a CSTR.  If the
* string is in quotes, it is parsed for escape sequences.
*
*	Return values are as follows:
*		-2	"=" Symbol was the first character!
*		-1	error
*		0	*N, ;, endstreamch before anything else
*		1    unquoted item
*		2    quoted item
*
* NOTE - the BLIB calling spec is for "maxlongs" instead of "maxchars"
* bytes allowed.  The interface code which massages parameters is
* specified to shift "maxlongs" such that it represents "maxchars".
*
* MyRditem is used by rdargs(), to set up A_W in D1.
*	D3 is already in place (CSource == isbcpl).  MODIFIES D2!!!!
*
*****************************************************************************

A_CommandName	equr	A2

; A_W (A3) is used internally!

D_Maxchars	equr	D2	; Calling arg position
D_CSource	equr	D3	; Calling arg position
D_Inquote	equr	D4

; D_Size (D5) is used internally!

D_Flag		equr	D6	; Internal flag for call from ReadArgs.
D_Currpos	equr	D7

; D_Flag values -1 for BCPL, 0 for ReadItem(), 1 for internal ReadArgs()
; MyRditem modifies D2!!!

MyRditem:
	move.l	D_Size,D_Maxchars
	sub.l	A_W,D_Maxchars	; Calculate space available.
	subq.l	#1,D_Maxchars	; allocate space for null
	 bge.s	not_full	; make sure we have space for the null!

	;-- No space for null byte, allocate bigger buffer
	tst.l	D_IsBCPL	; if BCPL, don't allocate new buffer
	 beq.s	rditem_buf_fail
	bsr	NewBuffer
	 bne.s	MyRditem	; will recalc D_Maxchars, and continue

	;-- failure, return -1 (no regs to restore)
rditem_buf_fail:
	moveq.l	#-1,d0
	rts
	
not_full:
	move.l	A_W,D1

	tst.l	D_IsBCPL
	 beq.s	_blib_rditem

	movem.l	D2-D4/D6/D7/A2,-(sp)
	moveq.l	#1,D_Flag
	bra.s	rditem_merge

_blib_rditem:
	movem.l	D2-D4/D6/D7/A2,-(sp)

	subq.l	#1,D_Maxchars	; Lopp off a char for null termination
	moveq.l	#-1,D_CSource	; Special BCPL entry condition
	moveq.l	#-1,D_Flag
	bra.s	rditem_merge

_ReadItem:
	movem.l	D2-D4/D6/D7/A2,-(sp)
	moveq.l	#0,D_Flag

rditem_merge:
	;-- code above guarantees space for the null byte in blib_rditem and
	;-- in MyRditem.  _ReadItem MUST have a buffersize of 1 or more!

	move.l	D1,A_CommandName
	clr.b	(A_CommandName)	; Null string for starters.

	moveq.l	#0,D_Currpos

1$:	bsr	CS_ReadChar	; Skip leading spaces and tabs
	cmp.b	#' ',D0
	 beq.s	1$
	cmp.b	#TAB,D0
	 beq.s	1$		; EOF and LF fall through to rditem_enter

	moveq	#0,D_Inquote	; Initialize "inquote" status
	cmp.b	#'"',D0
	 bne.s	rditem_enter	; Skip ReadChar (already in d0)
	move.b	D0,D_Inquote	; Set value of Inquote to '"'

* Loop here on input characters
rditem_loop:
	bsr	CS_ReadChar

rditem_enter:
	cmp.b	#LF,D0
	 beq	rditem_endloop
	tst.l	D0		; EOF
	 bmi	rditem_endloop

	tst.w	D_Inquote	; Switch on quotes
	 beq.s	10$		; if in quotes, ignore *e, *n

	cmp.b	D_Inquote,D0	; A quote - is this the end?
	 beq	rditem_quoted

	cmp.b	#'*',D0		; Check for escape string processing
	 bne.s	20$

	bsr	CS_ReadChar	; Process escapes based on next char
	tst.l	d0		; EOF inside a quote
	 bmi	rditem_endloop
	cmp.b	#LF,d0		; end-of-line inside quote
	 beq.s	rditem_endloop	; was bmi for no apparent reason, fixed it.
	cmp.b	#'e',D0		
	 beq.s	2$
	cmp.b	#'E',D0		; (E)scape
	 beq.s	2$

	cmp.b	#'n',D0
	 beq.s	3$
	cmp.b	#'N',D0		; (N)ewline
	 bne.s	20$		

3$:	moveq	#LF,D0
	bra.s	20$
2$:	moveq	#$1B,D0
;	bra.s	20$		; Branch not needed

* Not quoted - check for terminator characters
10$:	cmp.b	#';',D0
	 beq.s	rditem_endloop
	cmp.b	#' ',D0
	 beq.s	rdil1
	cmp.b	#TAB,D0
	 beq.s	rdil1
	cmp.b	#'=',D0
	 bne.s	20$

*** Do all of '=' character handling here, special case moved
*** from terminal processing
	tst.l	D_Currpos
	 bne.s	rdil1			; = was not first char in loop

	moveq.l	#-2,D0			; Special exit value
	bra.s	rditem_rts		; WITH NO UnReadChar!

* Save this character in the commandname buffer, if there is room
20$:	addq.l	#1,D_Currpos

	cmp.l	D_Maxchars,D_Currpos
	 blt.s	21$
	tst.w	D_Flag			; fail if BCPL or ReadItem()
	 ble.s	rditem_fail

**** INTERNAL CODE FOR USE WITH READARGS BUFFER REALLOCATIONS
* Note that register D0 has the next character to insert!

	bsr	NewBuffer
	 beq.s	rditem_fail

	move.l	A_W,A_CommandName
	move.l	D_Currpos,D1
24$:	subq.l	#1,D1
	 ble.s	25$
	move.b	(A0)+,(A_CommandName)+	; NOTE! NewBuffer returns old A_W in A0!
	bra.s	24$
25$:	move.l	D_Size,D_Maxchars
	sub.l	A_W,D_Maxchars

***** END INTERNAL CODE

21$:	tst.w	D_Flag			; Check for "BCPL"
	 blt.s	22$
	move.b	D0,(A_CommandName)+	; Store C-string
	clr.b	(A_CommandName)
23$:	bra	rditem_loop

22$:	move.b	D0,0(A_CommandName,D_Currpos)	; Store BSTR
	clr.b	1(A_CommandName,D_Currpos)
	move.b	D_Currpos,(A_CommandName)
	bra.s	23$

rdil1:	tst.w	D_Flag			; Don't unread TAB or SPACE
	 bpl.s	rdil2			; UNLESS bcpl.

; End of while loop
; NOTE: d0 has character returned from ReadChar!!!
rditem_endloop:
	;-- These are commented out, so it will unread EOF, and thus force
	;-- the shell when it calls FGetC after ReadItem to get the actual
	;-- reason ReadItem returned (aka EOF).
*	tst.l	d0
*	 bmi.s	rdil2		; don't unread on EOF!
	bsr.s	CS_UnReadChar	; Put back char.  Might make conditional.

rdil2:	tst.w	D_Inquote
	 bne.s	rditem_fail		; Unterminated quote

	move.l	D_Currpos,D0
	 beq.s	rditem_rts		; Return zero if no chars read
	moveq.l	#1,D0
	bra.s	rditem_rts		; Read something

rditem_quoted:
	moveq.l	#2,D0
	bra.s	rditem_rts
rditem_fail:
	moveq.l	#-1,D0
rditem_rts:
	movem.l	(sp)+,D2-D4/D6/D7/A2
	rts

**************************************************************************
*
* CS_ReadChar, CS_UnReadChar functions are internal functions
* which read either from the CSource, or from ReadChar/UnReadChar.
*
CS_ReadChar:
	move.l	D_CSource,D0
	 beq.s	1$
	addq.l	#1,D0
	 beq.s	1$

	move.l	D_CSource,A0
	move.l	CS_Buffer(A0),D0
	 beq.s	1$

	move.l	D0,A1
	move.l	CS_CurChr(A0),D1
	cmp.l	CS_Length(A0),D1
	 bge.s	2$
	addq.l	#1,CS_CurChr(A0)
	moveq.l	#0,D0
	move.b	0(A1,D1.l),D0
	rts

2$:	moveq.l	#ENDSTREAMCH,D0
	rts

1$:	SYSCALL	Input			; get input filehandle REJ
	move.l	d0,d1
	SYSCALL	FGetC
	rts

CS_UnReadChar:
	move.l	D_CSource,D0
	 beq.s	1$
	addq.l	#1,D0
	 beq.s	1$

	move.l	D_CSource,A0
	move.l	CS_Buffer(A0),D0
	 beq.s	1$

	tst.l	CS_CurChr(A0)
	 ble.s	2$
	subq.l	#1,CS_CurChr(A0)
*	moveq.l	#-1,D0
2$:	rts

*2$:	moveq.l	#0,D0		; Return status not used internally
*	rts

1$:	SYSCALL	Input			; get input filehandle REJ
	move.l	d0,d1			; fh
	move.l	d2,-(a7)		; save reg, might be live REJ
	moveq	#-1,d2			; restore last character read
	SYSCALL	UnGetC
	move.l	(a7)+,d2		; restore - returns character unread
	rts

*****************************************************************************
* NewBuffer()
*
* Internal subroutine allocates a new A_W buffer which is a rounded
* multiple of 256 bytes larger than the current A_W/D_Size buffer.
*
* Register D0 is preserved (normally it is a character to store)
*
* Registers A_W and D_Size are set to newly allocated values. Note
* that this is done internal to ReadItem(), which is a different
* register context than ReadArgs(), but the internal callout does not
* use these two registers.
*
* Returns with Z flag set indicating status - Z means FAILED TO ALLOCATE.
*
* Returns with original A_W in A0, for copying over old contents.
*
*****************************************************************************

NewBuffer:
	movem.l	D0/D1,-(sp)

	move.l	D_Size,D1
	sub.l	A_W,D1

	add	#VECSIZE+16,D1
	and.l	#-VECSIZE,D1
	subq.l	#8,D1
	move.l	D1,D_Size
	addq.l	#4,d1		; KLUDGE!! Force buffer to have extra bytes!
				; FIX!!!!!
	bsr	GimmeAVec
	tst.l	D0
	 beq.s	1$

	move.l	A_W,A0			; Original start address
	move.l	D0,A_W
	add.l	A_W,D_Size

1$:	movem.l	(sp)+,D0/D1
	rts


*****************************************************************************
********	Following function(s) might be LVO calls	*************
*****************************************************************************

* Don't touch d1/a1!

strlen:	moveq.l	#-1,D0
1$:	addq.l	#1,D0
	tst.b	(A0)+
	 bne.s	1$
	rts


***************************************
* convert character in d0 to upper case.
* do not change any other registers
* Not base register dependant
***************************************
Toupper:
	movem.l	d1/a0-a1,-(sp)
	bsr	@toUpper
	movem.l (sp)+,d1/a0-a1
	rts

*****************************************************************************

bcolonstr:
	dc.b	'%b: ',0

colonstr:
	dc.b	'%s: ',0

