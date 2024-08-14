**
**	$Id: tuples.asm,v 1.6 92/11/30 10:32:06 darren Exp $
**
**	Credit card resource (tuple related code)
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	INCLUDE	"carddata.i"
	INCLUDE	"exec/types.i"

	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/resident.i"

	INCLUDE	"exec/ables.i"

	INCLUDe	"tuples.i"
	INCLUDE	"gayle.i"
	INCLUDE	"debug.i"


** Exports

	XDEF	CopyTuple
	XDEF	DeviceTuple
	XDEF	IfAmigaXIP

** Imports

	XREF	IfRemoved
	XREF	CardSetSpeed
	XREF	MemoryMap

** Assumptions

	IFNE	CISTPL_NULL
	FAIL	"CISTPL_NULL not 0 - recode!"
	ENDC

** Register equates

HANDLE		EQUR	A1		;ptr to CardHandle
BUFFER		EQUR	A0		;ptr to Buffer
CACHEBUFFER	EQUR	A5		;cached ptr to Buffer

CARDMEMMAP	EQUR	A2		;ptr to card memory map

CARDMEMPTR	EQUR	A3		;ptr to card mem being searched
CARDMEMEOM	EQUR	A4		;ptr to end of card mem area


DATABYTE	EQUR	D0		;used to store data bytes

TUPLECODE	EQUR	D1		;Tuple code to find

SKIP		EQUR	D2		;Next byte skip value (1 or 2)

COPYCOUNT	EQUR	D3		;used for loops

LINKCOUNT	EQUR	D4		;Link counter (decrements)

CACHESKIP	EQUR	D5		;cache skip value
CACHELINK	EQUR	D6		;Cached Link address
CACHELINKEOM	EQUR	D7		;Cached Link address (end of mem)


		CNOP	0,4

******* card.resource/CopyTuple ****************************************
*
*   NAME
*	CopyTuple -- Find/copy a credit card tuple.
*   
*   SYNOPSIS
*	success = CopyTuple( CardHandle, buffer, tuplecode, size );
*	d0			a1	 a0	 d1	    d0
*
*	BOOL CopyTuple( struct CardHandle *, UBYTE *, ULONG, ULONG );
*
*   FUNCTION
*	This function scans credit-card memory for a tuple, and
*	if found, copies it into a supplied buffer.  The entire
*	tuple (including the tuple code, and link) are copied to
*	the supplied buffer.  The number of bytes copied to the
*	buffer will be 2 bytes, plus the argument "size", or the
*	value in the tuple LINK field (whichever is SMALLER).
*
*	The software calling this function is responsible for
*	examining the copy of the tuple (e.g., recognition of
*	fields, recognition of stop bytes, etc. within the tuple).
*	
*	This function does the best job it can to find a tuple
*	in attribute, or common memory.  It follows tuple chains,
*	and skips odd bytes in attribute memory.
*
*	This function monitors for credit-card removal while reading data.
*	If the credit-card is removed while a byte is being read, it will
*	stop searching, and return FALSE.
*
*	This function does not protect against another task writing
*	to credit-card memory while data is being read.  The device
*	is responsible for single-threading reads/writes to the
*	credit card as needed.
*
*	This function can be used to find multiple tuple codes; this
*	is a very rare case, so the mechanism provided for doing so is
*	unusual.  See INPUTS below for more information.
*
*	This function does not read bytes within the body of any tuples
*	except for the tuple you want copied, and the basic compatibility
*	tuples this function understands (see list below).
*
*	On some machines this function may slow down memory access
*	speed while reading the tuple chain.  This is done to prevent
*	potential problems on slow cards.  By examining the CISTPL_DEVICE,
*	and CISTPL_DEVICE_A tuples, you can determine the best possible
*	memory access speed for the type of card inserted.  Because memory
*	access speed may be slowed down, calls to this function should
*	be single-threaded.
*
*	The Card Information Structure must start with a CISTPL_DEVICE
*	tuple stored as the first tuple in attribute memory.  If not,
*	this function will search for a CISTPL_LINKTARGET tuple
*	stored at byte 0 of common memory.  Therefore it is possible
*	to store a CIS on cards which do not have any writeable
*	attribute memory, though this may cause problems for other
*	software implemented on other machines.  For example, some
*	SRAM cards do not come with writeable attribute memory, and/or
*	some may have OPTIONAL EEPROM memory which may not have been
*	initialized by the card manufacturer.  While it could be
*	argued that such cards do not conform to the PCMCIA PC Card
*	Standard, such cards are cheaper, and therefore likely to be
*	used.
*
*   INPUTS
*	CardHandle - Same handle as that used when OwnCard() was
*	called.
*
*	buffer - Pointer to a buffer where the tuple will be copied.
*
*	The buffer should be at least as large as "size" + 8 (see
*	use of "size" argument below).  Therefore the minimum buffer
*	size is 8 bytes.  See NOTES below.
*
*	tuplecode - The tuple code you want to search for.  This is
*	a ULONG value.  The upper 16 bits should be 0, or a number between
*	1-32K where a value of 0 means you want to find the first tuple
*	match, a value of 1 the second, etc.  For example -
*
*	0x41 means find the FIRST tuple equal to $41.
*
*	((1<<16)|(0x41)) means find the SECOND tuple equal to $41.
*
*	((2<<16)|(0x41)) means find the THIRD tuple equal to $41.
*
*	size - The maximum number of bytes you want copied (not
*	       including the tuple code, and link).  The actual number
*	       of bytes copied may be less than "size" if the tuple
*	       link field is less than "size".
*
*	       A size of 0 will result in only the tuple code, and
*	       link being copied to your buffer if the tuple is found.
*
*	       If you do not care how many bytes are copied, any unsigned
*	       value of 255 or greater will do.  In this case a maximum
*	       of 257 bytes might be copied to your buffer (if the
*	       tuple link field is the maximum of 255).
*
*	       Other sizes are useful if you know the size of the tuple
*	       you want copied, or you know there are active registers
*	       stored within the tuple, and only want to copy a portion
*	       of a tuple.
*
*   RETURNS
*
*	TRUE if the tuple was found, and copied, else FALSE.
*	This function may also return false if the CIS is believed
*	to be corrupt, or if the card is removed while reading the
*	tuples.
*
*   NOTES
*
*	This function can be called multiple times (adjusting the "size"
*	argument) to read a tuple of variable length, or unknown size.
*
*	Your supplied buffered is used by this function for working
*	storage - the contents of it will be modified even if this
*	function fails to find the tuple you want a copy of.
*
*	This function should NOT be used to find/copy tuples of type :
*
*	- CISTPL_LONGLINK_A
*	- CISTPL_LONGLINK_C
*	- CISTPL_NO_LINK
*	- CISTPL_LINKTARGET
*	- CISTPL_NULL
*	- CISTPL_END
*	
*	These tuples are automatically handled for you by this function.
*
*   SEE ALSO
*	OwnCard()
*
***********************************************************************

CopyTuple:

	PRINTF	DBG_ENTRY,<'CopyTuple($%lx,$%lx,$%lx,$%lx)'>,A1,A0,D1,D0

		movem.l	d2-d7/a2-a5,-(sp)

		move.l	d0,-(sp)		;cache size value

		movea.l	BUFFER,CACHEBUFFER

		lea	MemoryMap,CARDMEMMAP

	; scan attribute memory first

		movea.l	cmm_AttributeMemory(CARDMEMMAP),CARDMEMPTR

		movea.l	CARDMEMPTR,CARDMEMEOM
		add.l	#ATTR_MEM_SIZE,CARDMEMEOM
		moveq	#02,SKIP		;skip odd bytes

	; set-up implied Attribute -> Common memory link

		move.l	cmm_CommonMemory(CARDMEMMAP),CACHELINK

		move.l	CACHELINK,CACHELINKEOM
		add.l	#COMMON_MEM_SIZE,CACHELINKEOM

		moveq	#01,CACHESKIP

	; slow down memory access speed - must use protect functions
	; to fiddle with gayle hardware

		move.b	crb_SpeedMask(a6),crb_SpeedCache(a6)


		moveq	#CARD_SPEED_720NS,d0
		bsr	CardSetSpeed

		tst.b	d0
		beq	tuplenomatch
	
	; first tuple is suppose to be CISTPL_DEVICE, if not
	; assume bad tuple chain, or CIS written by something
	; like the HP Palm-Top which does not check for
	; overlapped Attribute and common memory.  Therefore
	; we look for a CISTPL_LINKTARGET tuple in common memory

		cmp.b	#CISTPL_DEVICE,(CARDMEMPTR)
		bne	endofchain

	; 11/4/92 -- Because of a hardware bug in the A600/A1200/CDTV-CR,
	; it turns out that attribute memory accesses present a
	; 2+ Megabyte address to the card.  Therefore, for cards
	; which ignore the REG line, a 2+ Megabyte memory address
	; is presented when accessing COMMON memory.  If the card
	; is less than 2 Megabytes, it either does not decode all
	; of the address lines, or returns junk.  However if the
	; card is > 2 Megabytes, it will read/write from $800000
	; rather than $600000 when an access is made to $A00000.
	;
	; To work around this hardware bug, I am going to do some
	; validation below which is technically illegal since I
	; am going to read common memory space!  Hopefully this
	; affects no cards adversly.
	;
	; For RAM, ROM, EPROM, of FLASH, no harm.
	;
	; For IO cards, no harm if the card is really just one
	; space of attribute memory.  The only potential
	; problem comes from a card which has read sensitive registers
	; in common memory space.  Oh well, the bug needs a work
	; around or we are looking at junk at $800000 for cards
	; which have the problem.
	; 

		movem.l	d1-d4/a0-a3/a6,-(sp)

		move.l	crb_ExecLib(a6),a6

		move.l	CACHELINK,a2
		move.l	#$200000,d2

tst_dtuple:
		moveq	#04,d4			;try read 5x
		move.l	0(a2,d2.L),d3
		
	; validate stability of $800000 memory

tst_amirror:
		nop

		cmp.l	0(a2,d2.L),d3
		bne.s	dtuple_ok

		JSRLIB	CacheClearU

		dbf	d4,tst_amirror

		cmp.l	(a3),d3			;assume ATTR mem is stable
		bne.s	dtuple_ok

	PRINTF	DBG_FLOW,<'--$800000 == $%lx'>,D3

		cmp.l	(a2),d3			;compare with $600000
		beq.s	dtuple_ok		;if equal, must all be same memory

	PRINTF	DBG_FLOW,<'--WARNING GRT 2MEG card?'>

		moveq	#00,d2			;$A00000/$800000 confusion TRUE

dtuple_ok:
		tst.l	d2
		movem.l	(sp)+,d1-d4/a0-a3/a6	;CC unaffected
		BEQ_S	endofchain

		BRA_S	scantuples

	; skip per cached link value

skiplink:

	PRINTF	DBG_FLOW,<'--Skip (%ld*%ld) bytes'>,LINKCOUNT,SKIP

	; tuple LINK $FF?  If so, treat as end of chain

		cmp.b	#CISTPL_END,LINKCOUNT
		BEQ_S	endofchain

		mulu	SKIP,LINKCOUNT
		adda.l	LINKCOUNT,CARDMEMPTR

	PRINTF	DBG_FLOW,<'--Next tuple @ $%lx'>,CARDMEMPTR

scantuples:

	; copy code, and link to buffer for examination
	;--
	;   Copy Tuple Code, and Link to buffer
	;--

		movea.l	CACHEBUFFER,BUFFER
		bra.s	skipNULLS

	; skip past all NULL tuples (e.g., empty SRAM card)

skipCISTPL_NULL:
		adda.l	SKIP,CARDMEMPTR
skipNULLS:
		cmpa.l	CARDMEMEOM,CARDMEMPTR
		bcc	tuplenomatch			;>=

	; check to see if the card is still inserted before accessing
	; credit-card memory -- NOTE that we check BEFORE reading the
	; next byte; if the HANDLE does not match, we won't read

		bsr	IfRemoved
		BNE_S	tuplenomatch

		move.b	(CARDMEMPTR),DATABYTE		;TPL_CODE
		beq.s	skipCISTPL_NULL

	; one more check to make sure we still have a card

		bsr	IfRemoved
		BNE_S	tuplenomatch

	; we should never read past end of an area while scanning tuples.
	; if so we have found a corrupt CIS (e.g., stream of 0's or other
	; junk)

	; copy tuple code

		moveq	#TUPLE_SIZEOF,COPYCOUNT		;grab 2 bytes
		bsr	copyCISbytes
		BEQ_S	tuplenomatch

		moveq	#00,LINKCOUNT
		move.b	TPL_LINK(CACHEBUFFER),LINKCOUNT

	; We got something back (see if its what they asked for)

		move.b	(CACHEBUFFER),DATABYTE		;TPL_CODE

		cmp.b	DATABYTE,TUPLECODE
		BEQ_S	tuplematch

	; see if its a link related tuple

		cmp.b	#CISTPL_NO_LINK,DATABYTE
		beq	tuplenolink

		cmp.b	#CISTPL_LONGLINK_A,DATABYTE
		beq	tuplelonglinkA

		cmp.b	#CISTPL_LONGLINK_C,DATABYTE
		beq	tuplelonglinkC

	; see if its CISTPL_END tuple

		cmp.b	#CISTPL_END,DATABYTE
		bne	skiplink

	; end of chain - see if we have a LINK to follow

endofchain:

	PRINTF	DBG_FLOW,<'--End of tuple chain found'>

		tst.l	CACHELINK
		BEQ_S	tuplenomatch

	PRINTF	DBG_FLOW,<'--Tuple Link @ $%lx'>,CACHELINK

	; yes, follow link

		movea.l	CACHELINK,CARDMEMPTR
		movea.l	CACHELINKEOM,CARDMEMEOM
		move.l	CACHESKIP,SKIP

	;
	; clear implied link (so that implied link from attribute to
	; common memory is NULL) - a new LongLink must be found, else
	; we quit at next CISTPL_END.
	;

		moveq	#00,CACHELINK

	; make sure link points to LinkTarget tuple

		movea.l	CACHEBUFFER,BUFFER

		moveq	#TUPLE_SIZEOF,COPYCOUNT
		BSR_S	copyCISbytes
		BEQ_S	tuplenomatch
	
		moveq	#00,LINKCOUNT
		move.b	TPL_LINK(CACHEBUFFER),LINKCOUNT

		cmp.b	#CISTPL_LINKTARGET,(CACHEBUFFER)	;TPL_CODE
		bne.s	tuplenotlink

	PRINTF	DBG_FLOW,<'--Link-Target CODE found'>

	; validate 3 bytes of "CIS" are in body

		moveq	#TP_LinkTarget_SIZEOF-TPLTG_TAG,COPYCOUNT
		BSR_S	copyCISbytes
		beq.s	tuplenomatch

		cmp.b	#'C',TPLTG_TAG+0(CACHEBUFFER)
		bne.s	tuplenotlink

		cmp.b	#'I',TPLTG_TAG+1(CACHEBUFFER)
		bne.s	tuplenotlink	
		
		cmp.b	#'S',TPLTG_TAG+2(CACHEBUFFER)
		beq	skiplink

tuplenotlink:

	PRINTF	DBG_FLOW,<'--Link-Target BODY not CIS'>

tuplenomatch:

		moveq	#FALSE,d0

CopyTupleExit:

		lea	4(sp),sp			;pull size

		move.l	d0,d2				;cache return code
		move.b	crb_SpeedCache(a6),d0
		bsr	CardSetSpeed			;reset speed
		tst.l	d0
		beq.s	copytuplegone
		move.l	d2,d0				;return code

copytuplegone:
		
		movem.l	(sp)+,d2-d7/a2-a5

	PRINTF	DBG_EXIT,<'%ld=CopyTuple()'>,D0

		rts

	; copy rest of tuple (up to a max value of size) if the
	; count value has been met

tuplematch:
		swap	TUPLECODE			;count in high word
		subq.w	#1,TUPLECODE
		bmi.s	copythistuple

		swap	TUPLECODE			;not yet
		bra	skiplink

copythistuple:

	PRINTF	DBG_FLOW,<'--Tuple Match found'>
		
		cmp.l	(sp),LINKCOUNT
		bls.s	duptuple

	PRINTF	DBG_FLOW,<'--Size requested < link'>

		move.l	(sp),LINKCOUNT
		beq.s	tuplecopied			; special case

duptuple:

	PRINTF	DBG_FLOW,<'--Copying %ld bytes of tuple body'>,LINKCOUNT

		move.l	LINKCOUNT,COPYCOUNT
		beq.s	tuplecopied

		bsr.s	copyCISbytes
		beq	tuplenomatch

tuplecopied:
		moveq	#TRUE,D0
		bra	CopyTupleExit

	;--
	;  Copy X # of bytes of CIS to (BUFFER)+
	;
	;  All bytes MUST be read via this section of code (except for
	;  CISTPL_NULL skipping above).
	;--

copyCISbytes

	; decrement link count
		sub.l	COPYCOUNT,LINKCOUNT

		subq.l	#1,COPYCOUNT

	; never read past end of memory area
copyCISbytesl:
		cmpa.l	CARDMEMEOM,CARDMEMPTR
		bcc.s	corruptCIS		;>=


		move.b	(CARDMEMPTR),(BUFFER)+
		adda.l	SKIP,CARDMEMPTR

	; check to see if last byte read was read while card is inserted

		bsr	IfRemoved
		bne.s	corruptCIS

		dbf	COPYCOUNT,copyCISbytesl

		moveq	#TRUE,D0
		rts

corruptCIS:
		moveq	#FALSE,D0
		rts

	;--
	;  CISTPL_NO_LINK tuple - clear implied link
	;--
tuplenolink:

	PRINTF	DBG_FLOW,<'--CISTPL_NO_LINK found'>

		moveq	#00,CACHELINK
		bra	skiplink
	;--
	;  CISTPL_LONGLINK_A
	;--
tuplelonglinkA:

	PRINTF	DBG_FLOW,<'--CISTPL_LONKLINK_A found'>

		move.l	cmm_AttributeMemory(CARDMEMMAP),CACHELINK
		move.l	#ATTR_MEM_SIZE,CACHELINKEOM
		add.l	CACHELINK,CACHELINKEOM
		moveq	#01,CACHESKIP		;skip odd bytes
		bra.s	calculatelink

	;--
	;  CISTPL_LONKLINK_C
	;--
tuplelonglinkC:

	PRINTF	DBG_FLOW,<'--CISTPL_LONKLINK_A found'>

		move.l	cmm_CommonMemory(CARDMEMMAP),CACHELINK
		move.l	#COMMON_MEM_SIZE,CACHELINKEOM
		add.l	CACHELINK,CACHELINKEOM
		moveq	#00,CACHESKIP		;all bytes

	;
	; Grab the next 4 bytes, and use it to calc a 32 bit offset
calculatelink:
		moveq	#TP_LongLink_SIZEOF-TPLL_ADDR,COPYCOUNT
		BSR_S	copyCISbytes
		beq	tuplenomatch

		lea	TPLL_ADDR(CACHEBUFFER),BUFFER

	;
	; calc 32 bit address such that
	;
	; CIS has "AABBCCDD" translates into: $DDCCBBAA
	;

makelong:
		moveq	#TP_LongLink_SIZEOF-TPLL_ADDR-1,COPYCOUNT
makelongv:
		move.b	(BUFFER)+,DATABYTE
		ror.l	#8,DATABYTE
		dbf	COPYCOUNT,makelongv

	PRINTF	DBG_FLOW,<'--Link offset @ $%lx'>,DATABYTE

	; add this amount to our base, and skip ahead to the next tuple
	; adjust for Attribute, or Common Memory

		lsl.l	CACHESKIP,DATABYTE	; could be 0
		add.l	DATABYTE,CACHELINK	; adjust
		addq.l	#1,CACHESKIP		; 0->1 or 1->2

	PRINTF	DBG_FLOW,<'--Link address @ $%lx'>,DATABYTE

		bra	skiplink


******* card.resource/IfAmigaXIP **************************************
*
*   NAME
*	IfAmigaXIP -- Check if a card is an Amiga execute-in-place card.
*   
*   SYNOPSIS
*	result=IfAmigaXIP( handle )
*	d0		       a2
*
*	struct Resident *IfAmigaXIP( struct CardHandle * );
*
*   FUNCTION
*	Check to see if a card in the slot is an Amiga execute-in-place
*	card.  The Card Information Structure must have a valid
*	CISTPL_AMIGAXIP tuple.
*	
*	Tuples can be treated like structures.  The format of a
*	CISTPL_AMIGAXIP tuple is -
*	
*
* 	struct TP_AmigaXIP {
*		UBYTE	TPL_CODE;
*		UBYTE	TPL_LINK;
*		UBYTE	TP_XIPLOC[4];
*		UBYTE	TP_XIPFLAGS;
*		UBYTE	TP_XIPRESRV;
*	};
*
*
*	The TPL_CODE field must be equal to CISTPL_AMIGAXIP (0x91).
*
*	The TPL_LINK field must be equal to the length of the body
*	of a CISTPL_AMIGAXIP tuple (0x06).
*
*	The TP_XIPLOC array is the memory location of your ROM-TAG
*	stored in little-endian order.  This value is stored as an
*	"offset" into common memory as is the standard for storing 32 bit
*	bit pointers in tuples.
*
*	For example, a pointer to a ROM-TAG stored at an offset of
*	0x00000200 would be stored as four bytes like so:
*
*		0x00, 0x02, 0x00, 0x00
*
*	Currently credit-card common memory is mapped starting at
*	memory location 0x600000.  Because a ROM-TAG is used,
*	it is implied that execute-in-place code can be compiled/linked
*	to use absolute references.  It is believed that most developers
*	will not want to have to write pc-relative code only.
*
*	The TP_XIPFLAGS field is treated as a set of flag bits.
*	See card.i/h for defined bits.  All undefined bits MUST be
*	0.
*
*	The TP_XIPRESRV field must be 0 for now.
*	
*
*	The system polls for cards which have a CISTPL_AMIGAXIP tuple
*	at the same time that it searches for devices to boot off.
*	When a card with a valid CISTPL_AMIGAXIP tuple is found, the
*	system will call your ROM-TAG via Exec's InitResident() function.
*
*	The system examines the return code from InitResident().  A
*	NULL return in D0 means you are done with the card, and it can
*	be removed by the user.  A successful return indicates you are
*	still using the card.  Some programs (e.g., some games) may
*	never return.  The only requirement is that if you do return,
*	you must leave the system in a "good" state (including returning
*	most of, or all the memory you allocated).  The standard
*	convention for preserving registers apply.
*
*	Note that your execute-in-place code will not be called
*	a second time, unless you have returned a non-successful
*	result.  In this case your execute-in-place code MUST
*	assume the user can remove, and insert your card again.
*	There are a variety of ways to check for re-insertion
*	(e.g., search for a message port, device, library, task,
*	etc., that you created).
*
*	Note that your execute-in-place code runs in an environment
*	similar to boot block games; before DOS has been initialized!
*
*	Your execute-in-place code should NOT try to initialize DOS
*	because DOS requires a suitable disk-like device be at
*	the head of the expansion base mountlist to boot off.
*
*	If you need DOS, it is possible to boot off a credit-card using
*	carddisk.device.  Such cards will require a valid
*	CISTPL_DEVICE tuple, and CISTPL_FORMAT tuple.  A portion
*	of the card can be used for a minimal number of data blocks
*	like the method described above.
*
*	However this method is not recommended, though it is anticipated
*	that some developers will have thought of, and used this
*	method anyway.  If you must do this, at least use the
*	CardHandle returned by OwnCard() to set hardware reset
*	on removal else the machine will likely crash anyway if
*	the card is removed while your execute-in-place code is running.
*
*   RETURNS
*	Pointer to a ROM-TAG on the card, or NULL indicating that:
*
*	o The card does not meet the above requirements, or
*	o The card has been removed, or
*	o You are not the owner of the credit-card.
*
*   NOTES
*	This function is being made public so developers can test
*	test that their execute-in-place credit-cards meet the
*	requirements for an Amiga execute-in-place card.  In general
*	there is no reason for devices, or applications to use this
*	function.
*
*	Amiga execute-in-place software is identified via a tuple code
*	reserved for manufacturer specific use, therefore the manufacturer
*	of the card may freely use the CISTPL_VERS_1, or CISTPL_VERS_2
*	tuples to place identification information on the credit-card.
*
*	No attempt has been made to make use of the MS-DOS execute-in-place
*	method; it is believed that most manufacturers of Amiga
*	execute-in-place software will prefer a simple, and small
*	scheme for running their execute-in-place code.
*
*   SEE ALSO
*	OwnCard(), resources/card.h, resources/card.i
*
***********************************************************************

IfAmigaXIP:
	PRINTF	DBG_ENTRY,<'IfAmigaXIP()'>

		sub.l	#TP_AmigaXIP_SIZEOF+8,sp

		move.l	sp,a0
		move.l	a2,a1
		move.l	#CISTPL_AMIGAXIP,d1	;tuple
		moveq	#(TP_AmigaXIP_SIZEOF-TUPLE_SIZEOF),d0

		bsr	CopyTuple

		tst.l	d0
		beq.s	notbootable

	PRINTF	DBG_FLOW,<'Found a CISTPL_AMIGABOOT tuple code'>

		lea.l	TPL_LINK(sp),a0
		cmp.b	#(TP_AmigaXIP_SIZEOF-TUPLE_SIZEOF),(a0)+
		bne.s	notbootable

	; grab pointer to ROM-TAG

		moveq	#03,d1
makelongD0:
		move.b	(a0)+,d0
		ror.l	#8,d0
		dbf	d1,makelongD0

	; and grab flags

		move.b	(a0),d1

	; validate ROM-TAG exists on word boundary

		btst	#00,d0
		bne.s	notbootable	;odd?

		add.l	#COMMON_MEMORY,d0
		movea.l	d0,a0

		cmp.w	#RTC_MATCHWORD,(a0)+
		bne.s	notbootable

		cmp.l	(a0),d0
		beq.s	isbootable


notbootable:
		moveq	#00,d0	
isbootable:
		add.l	#TP_AmigaXIP_SIZEOF+8,sp

	PRINTF	DBG_EXIT,<'$%lx=IfAmigaXIP()'>,D0

		rts

******* card.resource/DeviceTuple *************************************
*
*   NAME
*	DeviceTuple -- Decode a device tuple
*   
*   SYNOPSIS
*	return=DeviceTuple( tuple_data, storage)
*		   	    a0		a1	
*
*	ULONG DeviceTuple( UBYTE *, struct DeviceTData *);
*
*   FUNCTION
*	Extracts SIZE, TYPE, and SPEED from a device tuple (generally
*	obtained with CopyTuple()).
*
*   INPUTS
*	tuple_data - Pointer to a CISTPL_DEVICE tuple (generally obtained
*	with CopyTuple()).
*
*	storage - Pointer to a DeviceTData structure in which results
*	are to be stored.
*
*	struct	DeviceTData {
*		ULONG	dtd_DTsize;		/* Size of card (bytes)	*/
*		ULONG	dtd_DTspeed;		/* Speed in nanoseconds	*/
*		UBYTE	dtd_DTtype;		/* Type of card		*/
*		UBYTE	dtd_DTflags;		/* Other flags		*/
*	};
*
*
*   RETURN
*	SIZE (same as dtd_DTsize) if the Device Tuple could be decoded.
*	FALSE (0) if the tuple is believed to be invalid.  The tuple is
*	considered to be invalid if:
*
*		The tuple link value is 0.
*
*		The device type/speed byte is $00, or $FF.
*
*		The device type is DTYPE_EXTEND, which is undefined
*		as of this writing.
*
*		The extended speed byte is a value which is
*		undefined as of this writing.
*
*		The extended speed byte is extended again which is
*		undefined as of this writing.
*
*		The device Size byte is $FF.
*
*   NOTES
*	Some cards may not have a size specified in the device
*	tuple.  An example would be an I/O card.  The size would be
*	returned as one (1) in this case.
*
*	You should not call this function with a partial CISTPL_DEVICE
*	tuple, or the return values may be junk.
*
*   SEE ALSO
*	CopyTuple(), resources/card.h, resources/card.i
*
***********************************************************************


DeviceTuple:
	PRINTF	DBG_ENTRY,<'DeviceTuple($%lx,$%lx)'>,A0,A1
		
		movem.l	d2/a2,-(sp)

	; fail if link is 0

		addq.l	#1,a0
                move.b	(a0)+,d2	;link

		beq	device_invalid

	; fail if speed/type byte is $00, or $ff

		move.b	(a0)+,d0
		beq	device_invalid
		cmp.b	#CISTPL_END,d0
		beq	device_invalid

	PUSHBYTE	D0
	PRINTF	DBG_FLOW,<'Type/Speed byte = $%lx'>
	POPLONG		1

		moveq	#00,d1
		move.b	d0,d1

	; extract type

		lsr.b	#DTYPE_SHIFT,d0
		cmp.b	#DTYPE_EXTEND,d0
		beq	device_invalid

	PUSHBYTE	D0
	PRINTF	DBG_FLOW,<'Device Type = $%lx'>
	POPLONG		1

		move.b	d0,dtd_DTtype(a1)

	; I've decided to let the application deal with the WPS
	; bit if it needs it; the spec is unclear if its going to be
	; used or not.

	; determine speed

		and.b	#DSPEED_MASK,d1

		cmp.b	#DSPEED_EXT,d1
		beq.s	calcspeed

		lea	speedtable_nor(pc),a2
		move.b	0(a2,d1.w),d1
		move.l	d1,dtd_DTspeed(a1)

	PRINTF	DBG_FLOW,<'Speed is %ld ns'>,D1

		BRA_S	device_speed

calcspeed:

	PRINTF	DBG_FLOW,<'Speed field is extended'>

	; if the extended byte is extended, its undefined as of
	; this writing, and I don't know what it means - dont use


		moveq	#00,d1
		moveq	#00,d0

	; high bit indicates its extended again

		subq.b	#1,d2
		BEQ_S	device_invalid

		move.b	(a0)+,d1
		BMI_S	device_invalid

	PUSHBYTE	D1
	PRINTF	DBG_FLOW,<'Speed extended field = $%lx'>
	POPLONG		1

	; calc speed

		move.b	d1,d0
		and.b	#DSPEED_EXT_EXP_MASK,d0
		lsr.b	#DSPEED_EXT_MAN_SHIFT,d1

		lea	speedtable_exp(pc),a2
		lsl.w	#2,d0
		move.l	0(a2,d0.w),d0

		lea	speedtable_man(pc),a2
		move.b	0(a2,d1.w),d1

	PRINTF	DBG_OSCALL,<'UMult32($%lx,$%lx)'>,D0,D1

		move.l	a6,-(sp)
		move.l	crb_UtilLib(a6),a6
		JSRLIB	UMult32

	; and divide result by 10 to obtain actual value

		moveq	#10,d1

	PRINTF	DBG_OSCALL,<'UDivMod32($%lx,$%lx)'>,D0,D1

		JSRLIB	UDivMod32

		move.l	(sp)+,a6

		move.l	d0,dtd_DTspeed(a1)

	PRINTF	DBG_FLOW,<'Speed is %ldns'>,D0

device_speed:		

	; calculate size now if tuple is long enough



		moveq	#01,d0			;clear bits, and set size 1
		moveq	#00,d1

		subq.b	#1,d2
		BEQ_S	device_sized

		move.b	(a0)+,d0		;next byte is size

	PRINTF	DBG_FLOW,<'Size byte = %ld'>,D0

		cmp.b	#CISTPL_END,d0		;is invalid?
		BEQ_S	device_invalid

		move.b	d0,d1
		and.b	#DSIZE_MASK,d0

		lea	unitsizes(pc),a2
		lsl.w	#2,d0			;for lookup table
		move.l	0(a2,d0.w),d0

	PRINTF	DBG_FLOW,<'Size from table = %ld'>,D0

		lsr.b	#DUNITS_SHIFT,d1	;# of adddress units -1
		addq	#1,d1

	PRINTF	DBG_OSCALL,<'UMult32($%lx,$%lx)'>,D0,D1

		move.l	a6,-(sp)
		move.l	crb_UtilLib(a6),a6
		JSRLIB	UMult32
		move.l	(sp)+,a6

device_sized:

	PRINTF	DBG_FLOW,<'Card is size %ld bytes'>,D0

		move.l	d0,dtd_DTsize(a1)
		bne.s	device_valid		;return TRUE?

device_invalid:

		moveq	#FALSE,d0	;Failed

device_valid:
		movem.l	(sp)+,d2/a2

	PRINTF	DBG_EXIT,<'$%lx=DeviceTuple()'>,D0

		rts

unitsizes:
		dc.l	DSIZE_512
		dc.l	DSIZE_2K
		dc.l	DSIZE_8K
		dc.l	DSIZE_32K
		dc.l	DSIZE_128K
		dc.l	DSIZE_512K
		dc.l	DSIZE_2M
		dc.l	DSIZE_RESERVED	;0 yields 0 size result

	; exponent portion of extended speeds (ns)
	; 0 means too slow

speedtable_exp:
	;		ns------ us--------------- ms--------------
		dc.l	1,10,100,1000,10000,100000,1000000,10000000


	; mantissa portion of extended speeds (x10)
	; 0 means undefined

speedtable_man:
		dc.b	0,10,12,13,15,20,25,30,35,40,45,50,55,60,70,80

	; standard speed specifier nanoseconds (0 means reserved)

speedtable_nor:
		dc.b	0,250,200,150,100,0,0


		END