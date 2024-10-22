**
**	$Id: autoconfig.asm,v 1.4 93/10/14 15:27:28 darren Exp $
**
**      autoconfig CDGS MPEG board
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**

*
* This module does something which an autoconfig ROM is really not meant
* to do.  The CDGS MPEG module has no hardware to map in an extension
* or replacement O.S. ROM.  It does however have autoconfig hardware
* which we will use below to swizzle new/replacement O.S. modules in
* to exec's ResModules list.  Fortunately diag_init is run early on
* right after expansion, so we can get in early and replace modules
* before ROMBOOT time (see STRAP).  If this were not the case, we
* would have to perform a reboot at ROMBOOT time and add new modules
* via KICKTAGS.
*
* CAVEATS:
*
* Much like exec, this module has to scan the ROM its in for ROM-TAGS.
* Like exec, the size, and addresses of the resolved ROM code must
* be known in advance.
*
* Unfortunately, this autoconfig board COULD be mapped in anywhere in
* ZORRO-II space.  Thats bad, as ALL modules must therefore be
* relocatable (of course ROMTAGS cannot be relocatable by definition).
* Relocating ROMTAGS requires:
*
*	o The ROMTAG itself
*	o The init table if AUTOINIT
*	o The func init table if LONG WORDS & AUTOINIT
*	o The struct init table if there are ANY long words which are pointers
*
*	The last is particularly problematic because it would require
*	walking the struct init table.  Sniffing out longs, and
*	GUESSING if the long is infact a pointer (presumably by
*	doing some bounds checking??)
*
* Because ROMTAGS and AUTOCONFIG was not really meant to be used to add
* multiple modules, we have a choice:
*
*	o Not support relocation of the board - fail diag_init
*	  gracefully.
*
*	o Support relocation of modules ONLY if NOT AUTOINIT.
*	  The init code must be smart enough to use WORD offsets
*	  for function init, no long word pointers in struct init,
*	  and must of course be entirely relocatable.
*
* For this first release, we will opt for option #1.
*
* Fortunately, this autoconfig board will be mapped in at $200000 in
* the CDGS which is sufficient until such time as we need to build
* a general purpose ZORRO-II card.  Thats probably not going to be
* a big problem either since most of the modules in this ROM are
* CDGS specific, and are not needed in a general ZORRO-II card.  Infact
* many of these modules ASSUME CDGS hardware and will blow-up if run
* on another machine.
*

*
* Copyright (c) 1992 Commodore-Amiga, Inc.
*
* This example is provided in electronic form by Commodore-Amiga, Inc. for
* use with the "Amiga ROM Kernel Reference Manual: Devices", 3rd Edition,
* published by Addison-Wesley (ISBN 0-201-56775-X).
*
* The "Amiga ROM Kernel Reference Manual: Devices" contains additional
* information on the correct usage of the techniques and operating system
* functions presented in these examples.  The source and executable code
* of these examples may only be distributed in free electronic form, via
* bulletin board or as part of a fully non-commercial and freely
* redistributable diskette.  Both the source and executable code (including
* comments) must be included, without modification, in any copy.  This
* example may not be published in printed form or distributed with any
* commercial product.  However, the programming techniques and support
* routines set forth in these examples may be used in the development
* of original executable software products for Commodore Amiga computers.
*
* All other rights reserved.
*
* This example is provided "as-is" and is subject to change; no
* warranties are made.  All use is at your own risk. No liability or
* responsibility is assumed.
*
**********************************************************************
*
* Sample autoboot code fragment
*
**********************************************************************

**
**
** These are the calling conventions for the Diag routine
**
** A7 -- points to at least 2K of stack
** A6 -- ExecBase
** A5 -- ExpansionBase
** A3 -- your board's ConfigDev structure
** A2 -- Base of diag/init area that was copied
** A0 -- Base of your board
**
** Your Diag routine should return a non-zero value in D0 for success.
** If this value is NULL, then the diag/init area that was copied
** will be returned to the free memory pool.
**

	    INCLUDE "autoconfig.i"

	XREF	_LVOAlert

**	BNRYONLY

**
** Assumptions
**

	IFGT	ROMCODESIZE-BOARDSIZE
	FAIL	"BOARDSIZE < ROMCODESIZE; modify .i file"
	ENDC

            CODE


*******  RomStart  ***************************************************
**********************************************************************

RomStart:

	IFGT    ROMINFO
;
;   ExpansionRom structure
;
;   Note - If you implement your ExpansionRom and ExpansionControl
;   with PALS, then you can comment out everything until DiagStart:
;   (ie. Make ROMID EQU 0)

*    ; High nibbles of first two words ($00,$02) are er_Type (not inverted)
						; er_Type
		dc.w	$D000			;   11xx normal board type
						;   xx0x not in memory free list
						;   xxx1 Diag valid (has driver)
		dc.w	(SIZE_FLAG<<12)&$7000	;   0xxx not chained
						;   xnnn flags board size

*    ; High nibbles of next two words are er_Product
*    ; These are inverted (~), as are all other words except $40 and $42

						; er_Product
		dc.w	(~(PRODUCT_ID<<8))&$f000,(~(PRODUCT_ID<<12))&$f000

						; er_Flags
		dc.w	(~$C000)&$f000		;   ~1xxx board is moveable
						;   ~x1xx board can't be shut up
		dc.w	(~0)&$f000		;

		dc.w	(~0)&$f000,(~0)&$f000	; er_Reserved03

						; er_Manufacturer
		dc.w	(~(MANUF_ID))&$f000,(~(MANUF_ID<<4))&$f000
		dc.w	(~(MANUF_ID<<8))&$f000,(~(MANUF_ID<<12))&$f000

						; er_SerialNumber
		dc.w	(~(VERSION))&$f000,(~(VERSION<<4))&$f000
		dc.w	(~(VERSION<<8))&$f000,(~(VERSION<<12))&$f000
		dc.w	(~(REVISION))&$f000,(~(REVISION<<4))&$f000
		dc.w	(~(REVISION<<8))&$f000,(~(REVISION<<12))&$f000

						; er_InitDiagVec
		dc.w	(~((DiagStart-RomStart)))&$f000
		dc.w	(~((DiagStart-RomStart)<<4))&$f000
		dc.w	(~((DiagStart-RomStart)<<8))&$f000
		dc.w	(~((DiagStart-RomStart)<<12))&$f000

		dc.w	(~0)&$f000,(~0)&$f000	; er_Reserved0c
		dc.w	(~0)&$f000,(~0)&$f000	; er_Reserved0d
		dc.w	(~0)&$f000,(~0)&$f000	; er_Reserved0e
		dc.w	(~0)&$f000,(~0)&$f000	; er_Reserved0f

	IFNE	*-RomStart-$40
	FAIL	"ExpansionRom structure not the right size"
	ENDC

		;Note: nibbles $40 and $42 are not to be inverted
		dc.w	(0)&$f000,(0)&$f000	; ec_Interrupt (no interrupts)
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved11
		dc.w	(~0)&$f000,(~0)&$f000	; ec_BaseAddress (write only)
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Shutup (write only)
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved14
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved15
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved16
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved17
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved18
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved19
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1a
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1b
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1c
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1d
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1e
		dc.w	(~0)&$f000,(~0)&$f000	; ec_Reserved1f

	IFNE	*-RomStart-$80
	FAIL	"Expansion Control structure not the right size"
	ENDC

	ENDC	;ROMINFO


	IFGT	HAS_DRIVER

*******  DiagStart  **************************************************
DiagStart:  ; This is the DiagArea structure whose relative offset from
            ; your board base appears as the Init Diag vector in your
            ; autoconfig ID information.  This structure is designed
            ; to use all relative pointers (no patching needed).
            dc.b    DAC_WORDWIDE+DAC_CONFIGTIME    ; da_Config
            dc.b    0                              ; da_Flags
            dc.w    EndCopy-DiagStart              ; da_Size
            dc.w    DiagEntry-DiagStart            ; da_DiagPoint
            dc.w    BootEntry-DiagStart            ; da_BootPoint
            dc.w    DevName-DiagStart              ; da_Name
            dc.w    0                              ; da_Reserved01
            dc.w    0                              ; da_Reserved02

******* Strings referenced in Diag Copy area  ************************
DevName:    dc.b    'config_mpeg',0                     ; Name string
            ds.w    0              ; word align

*******  BootEntry  **************************************************
**********************************************************************

BootEntry:
	COLOR_DEBUG	$000F

		moveq	#00,d0			;return diag_init FAILed
		rts

*******  DiagEntry  **************************************************
**********************************************************************
*
*   success = DiagEntry(BoardBase,DiagCopy, configDev)
*   d0                  a0        a2        a3
*
*   Called by expansion.libraries diag_init module IF:
*
*   o the DAC_CONFIGTIME bit is set in da_Config
*
*   o da_BootPoint is <> 0
*
*   o da_DiagPoint is <> 0 and has an offset to the patching code below
*
**********************************************************************

DiagEntry:
*
* Scan our configured ROM for ROMTAGS.
*
*
* This code jmps back into the ROM code so that we do not copy all of
* the relocation code into RAM
*
	; check if the autoboot ROM is configured at the correct
	; memory location

	COLOR_DEBUG	$0F00

		cmpa.l	#ROMCODEADDR,a0
		bne.s	BootEntry

	; enable all 256K of ROM space

		move.l	a0,a1
		adda.l	#MPEG_CONTROL_PORT,a1
		move.w	#CD32_MAP_UMPEG,(a1)

	; and jump to ROM

		lea	RelocateRom-RomStart(a0),a1
		jmp	(a1)

*
* End of the Diag copy area which is copied to RAM
*
		ds.w	0
EndCopy:
*************************************************************************

RelocateRom:
	COLOR_DEBUG	$00F0

		movem.l	d2-d7/a2-a6,-(sp)

		move.l	$4,a6			;prepare for use of execbase
		lea	EndRelocate(pc),a5

		move.l	a0,d3
		add.l	#ROMCODESIZE,d3		;max address for scan

		move.w	#RTC_MATCHWORD,d2

	; create ROMTAG list (list struct on stack, allocate nodes)

		sub.l	#LH_SIZE,sp
		move.l	sp,a1
		NEWLIST	A1

dscantags:
		cmp.l	d3,a5
		bhi.s	dreplacertags

	; compare for matchword

		cmp.w	(a5)+,d2
		bne.s	dscantags

		lea	-2(a5),a1

		cmp.l	(a5),a1
		bne.s	dscantags

		cmp.l	4(a5),d3		;check endskip for safety
		bcs.s	dscantags

	; found a ROMTAG - allocate memory for a node to track this ROMTAG

		moveq	#ROMTAG_ENTRY_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR!MEMF_REVERSE,d1
		JSRLIB	AllocMem

	; the memory allocation should never ever fail!

		tst.l	d0
		beq	dalert_nomem

	; add this node to the list of ROMTAGS

		move.l	d0,a1
		lea	-2(a5),a0
		move.l	a0,rte_romtagptr(a1)
		move.l	sp,a0

		JSRLIB	AddTail

		move.l	4(a5),a5		;grab endskip pointer
		bra.s	dscantags

	; all rom tags found, now splice these into exec's list of
	; resident modules

dreplacertags:

		move.l	sp,a0
		JSRLIB	RemHead

		tst.l	d0
		beq	dendscan

		move.l	d0,a4
		move.l	rte_romtagptr(a4),a3
		move.b	RT_PRI(a3),d3

	;
	; 1.) scan exec's ResModule list for a matching module
	;
	; NO MATCH:
	;
	;	splice this module into exec's list based on priority.
	;
	;	use node memory for kicktag pointers
	;
	; MATCH:
	;
	;	check that this module is >= version/priority
	;	 than exec's list.
	;
	;	replace in exec's list if >= version/priority
	;
	;	free node in either case.
	;	


		move.l	ResModules(a6),a5
		moveq	#00,d6			;flag if this module exists
						;on resident list somewhere
ddscantags:
		move.l	(a5)+,d2
		beq.s	dsplicemod		;end of list, not found
		bgt.s	dcmptags

	; negative means linked

		bclr	#31,d2
		move.l	d2,a5
		bra.s	ddscantags

	; check if the module on the resident list matches the
	; current resident module

dcmptags:
		move.l	d2,a2
		move.l	RT_NAME(a3),a1
		move.l	RT_NAME(a2),a0
dcmpname:
		cmp.b	(a0)+,(a1)+
		bne.s	ddscantags
		
		tst.b	-1(a0)
		bne.s	dcmpname
		
	; the name of this module matches, so check if this module
	; is > whats already on the resident list.

		moveq	#01,d6			;flag it exists so we don't splice it

		move.b	RT_VERSION(a3),d0
		cmp.b	RT_VERSION(a2),d0	;if replacement mod is < current res mod
		blt.s	ddscantags		;do not replace
		bgt.s	dreplacemod		;if VER >, replace mod

	; VERSION is equal, test PRI

		cmp.b	RT_PRI(a2),d3
		blt.s	ddscantags		;signed compare!

	; this module has a > VERSION, or >=PRI, replace it

dreplacemod:

		move.l	a3,-4(a5)		;swizzle in pointer

	; fall through, D6 is TRUE, so free node

dsplicemod:
		tst.l	d6			;if match found, free node
		bne.s	dfreenode

	; this module does not already exist, so needs to be spliced
	; into the ResModule list - quickly rewalk list looking for
	; PRI comparison

		move.l	ResModules(a6),a0
dsscantags:

		move.l	(a0)+,d0
		beq.s	dsplicetag
		bgt.s	dscmptag

		bclr	#31,d0
		move.l	d0,a0
		bra.s	dsscantags
dscmptag:
		move.l	d0,a1
		cmp.b	RT_PRI(a1),d3
		ble.s	dsscantags		;signed compare

	; now A2 has the address of the last pointer on this
	; list of a module which is >= this new ROMTAGs priority.
	;
	; What we will do is change that pointer to be a link
	; to our node, copy the romtag pointer to our node,
	; point the next item in the node to the spliced in
	; ROMTAG, and then link that back to the next item
	; in the original list.
	;
	; the trick here is that the ROMTAG_ENTRY structure is
	; long enough for 3 pointers; just what we need for a splice
	; so the memory for the node is left un-freed


	IFNE	ROMTAG_ENTRY_SIZEOF-12
	FAIL	"ROMTAG_ENTRY_SIZEOF <> 12; recode!"	
	ENDC
dsplicetag:
	; The module to be splice is > priority than the next
	; module on the chain, so place it ahead of that module

		move.l	a3,(a4)			;splice in new item
		move.l	d0,4(a4)		;copy ROMTAG pointer to node
		move.l	a4,d0
		bset	#31,d0
		move.l	d0,-4(a0)		;place a link in original list to the node
		move.l	a0,d0
		bset	#31,d0
		move.l	d0,8(a4)		;link back to original list (may be the NULL item)
		bra	dreplacertags

	; free temporary node for holding ROM tag pointers

dfreenode:
		move.l	a4,a1
		moveq	#ROMTAG_ENTRY_SIZEOF,d0
		JSRLIB	FreeMem
		bra	dreplacertags

	;
	; the new modules have all been patched in - fix up the
	; checksum, and get out

dendscan:
		JSRLIB	SumKickData		;FORBID implied as we
						;are running on exec's single task

		move.l	d0,KickCheckSum(a6)

		add.l	#LH_SIZE,sp
		movem.l	(sp)+,d2-d7/a2-a6

	; return FALSE so our diag area is freed, and the pointer to
	; this area is cleared in the configdev structure.  The configdev
	; structure is still available, but there is no single driver
	; which needs to be configured for this board at ROMBOOT time

		moveq	#00,d0
		rts

dalert_nomem:
		DEADALERT	(AN_Unknown!AG_NoMemory!AO_ExecLib)

*
* Standard copyright notice for legal protection.  Indicates this
* ROM is the property of Commodore-Amiga, Inc. and is encoded in
* standard ASCII format in the ROM code.  Also identification info
* for tracking purposes.  Dates are inclusive from 1985 through
* 1993 as much new code is based on work and existing Commodore
* source code.

*
		COPYRIGHT_NOTICE

* End of the relocation code
*
		ds.w	0
EndRelocate:
*************************************************************************


	ENDC	; HAS_DRIVER

            END
