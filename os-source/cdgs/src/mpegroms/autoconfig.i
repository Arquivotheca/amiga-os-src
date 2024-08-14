**
**	$Id: autoconfig.i,v 1.3 93/10/14 15:16:28 darren Exp $
**
**      autoconfig.i CDGS MPEG board
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**

            INCLUDE "exec/types.i"
            INCLUDE "exec/nodes.i"
            INCLUDE "exec/lists.i"
            INCLUDE "exec/resident.i"
	    INCLUDE "exec/memory.i"
	    INCLUDE "exec/alerts.i"
	    INCLUDE "exec/macros.i"
	    INCLUDE "exec/execbase.i"
            INCLUDE "libraries/configvars.i"

	    INCLUDE "mpegrom_rev.i"

COPYRIGHT_NOTICE	MACRO

			dc.b	'Copyright © 1985-1993',0
			dc.b	'Commodore-Amiga, Inc.',0
			dc.b	'All Rights Reserved.'

			VERSTAG

			ENDM

**
** Equates required for CDGS MPEG specific build of this module
**

ROMCODESIZE		EQU	$040000	;256K
ROMCODEADDR		EQU	$200000	;ROM code resolved at $200000
					;actual configed location may differ

RELOCATE_ROMTAGS	EQU	1	;recreate location adjusted ROMTAGS
					;in RAM?


ROMINFO     EQU      1
ROMOFFS     EQU     $0
HAS_DRIVER  EQU	     1

* ROMINFO defines whether you want the AUTOCONFIG information in
* the beginning of your ROM (set to 0 if you instead have PALS
* providing the AUTOCONFIG information instead)
*
* ROMOFFS is the offset from your board base where your ROMs appear.
* Your ROMs might appear at offset 0 and contain your AUTOCONFIG
* information in the high nibbles of the first $40 words ($80 bytes).
* Or, your autoconfig ID information may be in a PAL, with your
* ROMs possibly being addressed at some offset (for example $2000)
* from your board base.  This ROMOFFS constant will be used as an
* additional offset from your configured board address when patching
* structures which require absolute pointers to ROM code or data.

* See the Addison-Wesley Amiga Hardware Manual for more info.

MANUF_ID	EQU	$202		; CBM
PRODUCT_ID	EQU	$6A		; CDGS MPEG Board ID

BOARDSIZE       EQU     $100000         ; How much address space board decodes
SIZE_FLAG	EQU     5		; Autoconfig 3-bit flag for BOARDSIZE
					;   0=$800000(8meg)  4=$80000(512K)
					;   1=$10000(64K)    5=$100000(1meg)
					;   2=$20000(128K)   6=$200000(2meg)
					;   3=$40000(256K)   7=$400000(4meg)

*
* the PAL version of the MPEG board only allocated 128K of ROM space
* but the UMPEG version supports a full 256K, however it must be
* enabled by setting bit 12 of address 0x40000 - this is a WRITE ONLY
* register so the mpeg device driver MUST be sure to set this bit
* anytime it writes


MPEG_CONTROL_PORT	EQU	$40000
CD32_MAP_UMPEG		EQU	$01000

*
* structures
*
 STRUCTURE	ROMTAG_ENTRY,0
	STRUCT	rte_node,MLN_SIZE
	APTR	rte_romtagptr
	LABEL	ROMTAG_ENTRY_SIZEOF


*
* macros
*
COLOR_DEBUG	MACRO
		IFNE	0
			move.w	#\1,$dff180
		ENDC
		ENDM
