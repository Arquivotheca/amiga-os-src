
**** configregs.i *********************************************************
*
*  register and bit definitions for expansion boards
*
* Copyright (C) 1985,  Commodore-Amiga, Inc., All rights reserved.
*
* $Header$
*
* $Locker$
*
* $Log$
*
****************************************************************************

	IFND	LIBRARIES_CONFIGREGS_I
LIBRARIES_CONFIGREGS_I	SET	1

** Expansion boards are actually organized such that only one nibble per
** word (16 bits) are valid information.  This table is structured
** as LOGICAL information.  This means that it never corresponds
** exactly with a physical implementation.
**
** The expansion space is logically split into two regions:
** a rom portion and a control portion.  The rom portion is
** actually stored in one's complement form (except for the
** er_type field).


 STRUCTURE ExpansionRom,0
    UBYTE	er_Type
    UBYTE	er_Product
    UBYTE	er_Flags
    UBYTE	er_Reserved03
    UWORD	er_Manufacturer
    ULONG	er_SerialNumber
    UWORD	er_InitDiagVec
    UBYTE	er_Reserved0c
    UBYTE	er_Reserved0d
    UBYTE	er_Reserved0e
    UBYTE	er_Reserved0f
    LABEL	ExpansionRom_SIZEOF

 STRUCTURE ExpansionControl,0
    UBYTE	ec_Interrupt		; interrupt control register
    UBYTE	ec_Reserved11
    UBYTE	ec_BaseAddress		; set new config address
    UBYTE	ec_Shutup		; don't respond, pass config out
    UBYTE	ec_Reserved14
    UBYTE	ec_Reserved15
    UBYTE	ec_Reserved16
    UBYTE	ec_Reserved17
    UBYTE	ec_Reserved18
    UBYTE	ec_Reserved19
    UBYTE	ec_Reserved1a
    UBYTE	ec_Reserved1b
    UBYTE	ec_Reserved1c
    UBYTE	ec_Reserved1d
    UBYTE	ec_Reserved1e
    UBYTE	ec_Reserved1f
    LABEL	ExpansionControl_SIZEOF

**
** many of the constants below consist of a triplet of equivalent
** definitions: xxMASK is a bit mask of those bits that matter.
** xxBIT is the starting bit number of the field.  xxSIZE is the
** number of bits that make up the definition.	This method is
** used when the field is larger than one bit.
**
** If the field is only one bit wide then the xxB_xx and xxF_xx convention
** is used (xxB_xx is the bit number, and xxF_xx is mask of the bit).
**

** manifest constants */
E_SLOTSIZE		EQU	$10000
E_SLOTMASK		EQU	$ffff
E_SLOTSHIFT		EQU	16

** these define the two free regions of Zorro memory space.
** THESE MAY WELL CHANGE FOR FUTURE PRODUCTS!
E_EXPANSIONBASE 	EQU	$e80000
E_EXPANSIONSIZE 	EQU	$080000
E_EXPANSIONSLOTS	EQU	8

E_MEMORYBASE		EQU	$200000
E_MEMORYSIZE		EQU	$800000
E_MEMORYSLOTS		EQU	128



******* ec_Type definitions */

** board type -- ignore "old style" boards */
ERT_TYPEMASK		EQU	$c0
ERT_TYPEBIT		EQU	6
ERT_TYPESIZE		EQU	2
ERT_NEWBOARD		EQU	$c0


** type field memory size */
ERT_MEMMASK		EQU	$07
ERT_MEMBIT		EQU	0
ERT_MEMSIZE		EQU	3


** other bits defined in type field */
	BITDEF	ERT,CHAINEDCONFIG,3
	BITDEF	ERT,DIAGVALID,4
	BITDEF	ERT,MEMLIST,5


** er_Flags byte -- for those things that didn't fit into the type byte */
	BITDEF	ERF,MEMSPACE,7		; wants to be in 8 meg space.  Also
					;     implies that board is moveable
	BITDEF	ERF,NOSHUTUP,6		; board can't be shut up.  Must not
					;     be a board.  Must be a box that
					;     does not pass on the bus.


** interrupt control register */
	BITDEF	ECI,INTENA,1
	BITDEF	ECI,RESET,3
	BITDEF	ECI,INT2PEND,4
	BITDEF	ECI,INT6PEND,5
	BITDEF	ECI,INT7PEND,6
	BITDEF	ECI,INTERRUPTING,7


	ENDC	!LIBRARIES_CONFIGREGS_I
