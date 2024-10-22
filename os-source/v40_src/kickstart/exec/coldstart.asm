*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		  Cold Initialization		 **
*	   **						 **
*	   ************************************************


*************************************************************************
*									*
*   Copyright 1984,85,88,89,90,91 Commodore-Amiga Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or translated	*
*   into any language or computer language, in any form or by any	*
*   means, electronic, mechanical, magnetic, optical, chemical,		*
*   manual or otherwise, without the prior written permission of	*
*   Commodore-Amiga Incorporated.					*
*									*
*************************************************************************

********************************************************************************
*
*	$Id: coldstart.asm,v 39.23 93/05/14 15:08:47 mks Exp $
*
*	$Log:	coldstart.asm,v $
* Revision 39.23  93/05/14  15:08:47  mks
* Changed a comment
* 
* Revision 39.22  93/04/02  09:41:52  mks
* Just some comment changes
*
* Revision 39.21  93/04/02  09:40:17  mks
* Changed the final marketing name to 3.1 rather than 3.01
*
* Revision 39.20  93/04/01  11:43:54  mks
* Coldstart is now the first module of EXEC...
*
* Revision 39.19  93/03/15  14:38:09  mks
* Gayle fix...
*
* Revision 39.18  93/02/15  17:29:06  mks
* Added the ReadGayle() code
*
* Revision 39.17  93/02/03  16:07:47  mks
* Even more ROMTAG search space if defined in ROMCONSTANTS
*
* Revision 39.16  93/01/18  16:01:28  mks
* Added the conditional inclusion of the EXTROM_LOWER and EXTROM_UPPER
* search such that CDGS and other 1meg ROM systems will work.
*
* Revision 39.15  93/01/06  17:07:32  mks
* Now only does the CIA mirror if on a non-A3000/A4000 class machine.
*
* Revision 39.14  93/01/04  15:38:27  mks
* Added CD-Game Machine fake-CIA turn-off to the early startup.
*
* Revision 39.13  92/11/05  08:30:05  mks
* Added the turning on and off the the credit card interface
* such that the under-body memory can work.
*
* Revision 39.12  92/07/21  18:48:02  mks
* Now, for 68040 systems, will make lowest address 4K rathern than 1K...
*
* Revision 39.11  92/06/07  12:57:27  mks
* Added all of the conditional ReKick code...
*
* Revision 39.10  92/05/28  19:04:13  mks
* Changed NEWLIST a0 to a bsr NewList to save ROM space
*
* Revision 39.9  92/04/10  18:45:10  mks
* Now does this for all builds...
*
* Revision 39.8  92/04/10  18:30:42  mks
* Added code such that A3000 builds will not copy the exec if the
* version is bad...  (That is, not copy the alert timeout!)
*
* Revision 39.7  92/04/09  04:31:12  mks
* Externalizes the scanBounds for sharing between files.
*
* Revision 39.6  92/04/09  03:26:47  mks
* Saved a bit of code by removing some debugging code such that
* it only is assembled in during debugging
*
* Revision 39.5  92/04/07  14:08:45  mks
* Added the storage and initialization of the Alert Timeout
*
* Revision 39.4  92/03/11  14:03:13  mks
* Added A1000jr support
*
* Revision 39.3  92/02/28  14:39:01  mks
* Removed the colour flash in exec startup...
*
* Revision 39.2  92/01/27  14:26:13  mks
* Removed the CDTV custom exec as it is no longer needed...
* (New magic in CDTV ROM)
*
* Revision 39.1  92/01/21  17:02:29  mks
* Folded in the CDTV_CR changes...
*
* Revision 39.0  91/10/15  08:26:27  mks
* V39 Exec initial checkin
*
********************************************************************************


;****** Exported Functions *******************************************

	XDEF	ColdReset
	XDEF	ColdStart
	XDEF	CrashReset

;****** Include Files ************************************************

	NOLIST
	INCLUDE "assembly.i"
	INCLUDE "types.i"
	INCLUDE "constants.i"
	INCLUDE "romconstants.i"
	INCLUDE "calls.i"
	INCLUDE "exec_rev.i"

	INCLUDE "nodes.i"
	INCLUDE "lists.i"
	INCLUDE "libraries.i"
	INCLUDE "execbase.i"
	INCLUDE "ables.i"
	INCLUDE "memory.i"
	INCLUDE "alerts.i"
	INCLUDE "tasks.i"
	INCLUDE "resident.i"

	INCLUDE "hardware/cia.i"
	INCLUDE "hardware/custom.i"
	INCLUDE "hardware/intbits.i"
	INCLUDE "hardware/dmabits.i"

	INCLUDE "internal/a3000_hardware.i"
	LIST


;****** Imported *****************************************************

	TASK_ABLES

	EXTERN_DATA	_ciaaddra
	EXTERN_DATA	_ciaapra
	EXTERN_DATA	_custom
	EXTERN_DATA	_color
	EXTERN_DATA	_intena

	EXTERN_DATA	SysLibTab	;function pointer table
	EXTERN_DATA	MAXSYSFUNC
	EXTERN_DATA	EXECBASETOTAL


;****** Imported Functions *******************************************

	EXTERN_CODE	AddMemList
	EXTERN_CODE	AddMemListInternal
	EXTERN_CODE	AllocAbsInternal
	EXTERN_CODE	FindCodeBefore
	EXTERN_CODE	TypeOfCPU

	EXTERN_CODE	Enqueue
	EXTERN_CODE	Allocate
	EXTERN_CODE	MakeFunctions
	EXTERN_CODE	GoAway

	EXTERN_CODE	NewList

	EXTERN_SYS	InitCode
	EXTERN_SYS	Debug
	EXTERN_SYS	Supervisor


	XDEF	ExecOrigin
	XDEF	ExecIdStr
	XDEF	VerRev		;Exec Version/Revision
	XDEF	KickstartVer	;Kickstart Version
	XDEF	KickstartRev	;Kickstart Revision

	XDEF	VERNUM
	XDEF	REVNUM

	XDEF	ExecName
	XDEF	TitleStr
	XDEF	LibraryStr

	XREF	StartExec
	XREF	EndMarker

*------ 68000 Reset Vector ----------------------------------------------------

	IFD	REKICK
ExecOrigin:	DC.W	REKICK_ROMS
	ELSE
ExecOrigin:	DC.W	BIG_ROMS	; For diagnostic cart...
	ENDC
		DC.W	JMPINSTR
		DC.L	ColdStart	; WARNING: Must be $FC00D2 (see
					; comments in coldstart.asm)

*------ Manufacturing Diagnostic Pattern  -------------------------------------

		DC.L	$0000FFFF

*------ Kickstart release number, inserted by the build process. --------------

KickstartVer:	DC.W	-1
KickstartRev:	DC.W	-1

*------ Exec Release Numbers (Binary) -----------------------------------------

VerRev:		DC.W	VERSION
		DC.W	REVISION

VERNUM		EQU	VERSION
REVNUM		EQU	REVISION

*------ System Serial Number --------------------------------------------------

		DC.L    -1

*------ Copyright -------------------------------------------------------------
*
* The design of these strings are important as other modules will be asking for
* them.  The strings should be as follows:
* -1 = The system name...
* -2 = Copyright and the dates, Trailing space *required*
* -3 = Copyright holder name, Trailing space *required*
* -4 = The "All Rights Reserved." string.
* -5 = The "marketing" name of the ROM.  (Such as 3.0 ROM)  (With space)
*
TitleStr:	dc.b	0		; Starting tag...		;  0
		dc.b	'AMIGA ROM Operating System and Libraries',0	; -1
		dc.b	'Copyright � 1985-1993 ',0			; -2
		dc.b	'Commodore-Amiga, Inc. ',0			; -3
		dc.b	'All Rights Reserved.',0			; -4

	IFNE	BETA_BUILD
		dc.b	'Beta'	; For use when not a RELEASE version
	ELSE
		dc.b	'3.1'	; Change to 3.02, etc. as needed...
	ENDIF
		dc.b	' ROM ',0					; -5

*------ Name String  ----------------------------------------------------------

ExecName:	dc.b	'exec'
LibraryStr:	dc.b	'.library',0	; Special, for internal tricks	; -6
ExecIdStr:	VSTRING							; -7

*------ Resident Tag ----------------------------------------------------------

		CNOP    2,4
ResidentTag:	DC.W    RTC_MATCHWORD
		DC.L    ResidentTag
		DC.L    EndMarker
		DC.B    RTF_SINGLETASK
		DC.B    VERNUM		* RT_VERSION
		DC.B    NT_LIBRARY	* RT_TYPE
		DC.B    105		* RT_PRI
		DC.L    ExecName
		DC.L    ExecIdStr
		DC.L    StartExec	* Exec RTF_SINGLETASK entrypoint

*------ Check for right size (must be exact) ----------------------------------
*
* Ugly kludge:  Exec must start with a jump to $xxxx00D2 and the
* instruction at $xxxx00D0 must be a reset!  (arg!)
*
ExecSize1:	ds.b	$D0-ExecSize1-ExecOrigin
ExecTest1:	IFNE	ExecTest1-ExecOrigin-$D0
		FAIL	'ERROR: Exec ColdStart not at correct address!'
		ENDC

*****i* exec.library/Uncallable/Coldstart **************************************
*
*   NAME
*	ColdStart -- system coldstart entry point
*
*   FUNCTION
*	This routine is responsible for power-on and hard-reset
*	software system initialization.  This routine makes no
*	assumptions about previous software and hardware states.
*
*   INPUT
*	A chaotic pile of disoriented bits.
*
*   RESULTS
*	An altogether totally integrated living system.
*
*   WARNING
*	This code is highly order dependent!  Warning:
*
*	The RESET instruction is kept for compatibility, some utilities
*	trace back the RESET vector then subtract 2.
*
*	ColdStart must end up to be $F800D2 or $FC00D2 for some strange
*	reasons.  In an environment with a ROM comming from virtual memory,
*	a software RESET will still put the real physical ROM down into
*	low memory.  Unless the numbers sync, a crash will occur.  Kludge.
*
********************************************************************************

ColdReset:		;Must be $xx00D0
	RESET

ColdStart:		;Must be $xx00D2
	;------ temporary supervisor stack (not used for a while):
	LEA.L	TEMP_SUP_STACK,SP	;:TODO: Do games use this area?
					;(I attempt to stay clear...)

*
*******************************************************************************
*
*  *************************************************************
*  ****
*  **** Major kludge!!!  ReKick code conditional assemble....
*  ****
*  **** This is *not* the mainline code.  Look for the ELSE
*  ****
*  *************************************************************
*
	IFD	REKICK
*
	;Old: Fill the traditional exception vectors with a temporary pointer.
	move.w	#BusErrorVector,a0	;Sign extends to 32 bits
	move.w	#($B4>>2),d1		;Fill to top of traps
	lea.l	coldExcept(pc),a1
cs_fillExcept:
	move.l	a1,(a0)+
	dbra	d1,cs_fillExcept


	;New: verify that the values actually took.  This is a cheap
	;power-on-only test for bad chip memory.   On soft reboot the
	;caches might invalidate this test.  Oh, well.
	move.w	#CC_BADRAM,d0
	move.w	#($B4>>2),d1
cs_checkExcept:
	cmp.l	-(A0),A1		;Tops down!
	bne	coldCrash
	dbf	D1,cs_checkExcept
*
* Get CPU type...
*
	BSR	TypeOfCPU
	MOVE.L	D0,A2
*
* Now set up as if we did not find an execbase...
*
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	moveq	#0,d6
	moveq	#0,d7
*
	move.l	#DEFAULT_TIMEOUT,a4	; Default alert timeout
*
	move.l	d2,a5			; No found execbase
	move.l	d2,a6			; No other execbase...
	move.l	d2,0			; Clear location 0...
*
* Rename the memorylist...
*
	LEA.L	LOWEST_USABLE,A0	; lower bound
	LEA.L	cs_ChipName(pc),A1      ; ASCII identifier
	move.l	a1,LN_NAME(a0)		; <* grin *>
*
* Get upper bound...
*
	move.l	MH_UPPER(a0),a3		; Upper CHIP memory...
*
* A0 is the memory header...
*
*******************************************************************************
*
* Standard assembly...  ***  Start of the mainline code...
*
	ELSE
*
************************************************************************
*
*	Hardware Settling
*	Note: old code did a 2,359,286 cycle delay at this point
*
*   REGISTERS
*	D5 = ROM Checksum (should be -1 if all is ok)
*
************************************************************************

;Code from <= V1.3 ROM
;	MOVE.L	#$20000,D0
;cs_settle:
;	SUBQ.L	#1,D0		;8
;	BGT.S	cs_settle	;10
;
;Code from 256K V1.4 ROMs
;;;;	IFNE	ROM_SIZE-$40000
;;;;	FAIL	"Recode the checksum function for >256 K byte ROMs!!!!"
;;;;	ENDC
;;;;	moveq	#-1,d1			;64K longwords


	IFNE	ROM_SIZE&$0003ffff
	FAIL	"Recode the checksum function for non-256K multiple ROMs!!!"
	ENDC

;Code for 512K V2.0 ROMs
	lea.l	ExecOrigin,a0
	moveq.l	#-1,d1				;first  dbra (64K multiples)
	moveq.l	#((ROM_SIZE/4)-1)>>16,d2	;second dbra
	moveq	#0,d5				;Zero out stored checksum

cs_chksum:
	add.l	(a0)+,d5        ;12 (nom)
	bcc.s	cs_nooverflow	;12
	addq.l	#1,d5
cs_nooverflow:
	dbra	d1,cs_chksum	;10
	dbra	d2,cs_chksum	;10

************************************************************************
*
*	Diagnostic Cartridge Intercept
*
************************************************************************

	IFEQ	DEVELOPMENT
		LEA.L cs_skipCart(PC),A5      * return address
		LEA.L ExecOrigin(PC),A0       * start of ROMs
		LEA.L CART_LOWER,A1
		CMP.L A0,A1
		BEQ.S cs_skipCart1
		CMP.W #DIAG_CART,(A1)         * look for diag cart takeover
		BNE.S cs_skipCart1
		JMP   INITOFFSET(A1)          * call cartridge
cs_skipCart1:
	IFEQ	A3000_ROMS
*
* Now check for the new credit card diagnostic cartridge (not on A3000)
*
gaylestatus	EQU	$DA8000
		BITDEF	GAYLE,STATUS_CCDISABLE,0

		move.b	#0,gaylestatus	; Turn on credit card interface
		nop			; Let the pipeline finish...

CREDIT_ATRIB	equ	$A00000	*Credit card attrib memory
CREDIT_COM	equ	$600000	*Credit card com memory
*
		lea.l	CREDIT_ATRIB,a1		* Point at memory...
		cmp.b	#$91,(a1)		* Check for special code
		bne.s	cs_skipCart		* Not special code
		addq.l	#2,a1
		cmp.b	#$05,(a1)		* Check length
		bne.s	cs_skipCart		* If no go, skip...
		addq.l	#2,a1
		cmp.b	#$23,(a1)		* Diagnostic code...
		bne.s	cs_skipCart		* Skip if not...
		addq.l	#2,a1
		move.b	(a1),d0
		ror.l	#8,d0			* Bring it around...
		addq.l	#2,a1
		move.b	(a1),d0
		ror.l	#8,d0			* Bring it around...
		addq.l	#2,a1
		move.b	(a1),d0
		ror.l	#8,d0			* Bring it around...
		addq.l	#2,a1
		move.b	(a1),d0
		ror.l	#8,d0			* Bring it around...
		lea.l	CREDIT_COM,a0		* Point at COM memory
		add.l	d0,a0			* Add in offset
		jmp		(a0)			* Call it...
*
	ENDC
cs_skipCart:

	IFEQ	A3000_ROMS
*
* Now turn off the credit card interface (card.resource will turn it on)
*
		move.b	#GAYLEF_STATUS_CCDISABLE,gaylestatus
*
	ENDC

	ENDC

	IFNE	DEVELOPMENT
	    btst.b #CIAB_GAMEPORT0,_ciaapra
	    bne.s  start_up	;button released, do normal


	    LEA.L  _custom,A4		* chipset base address
	    MOVE.W serdatr(A4),d0	* take one chip memory read delay
	    MOVE.W #$7FFF,D0		* clear all pattern
	    MOVE.W D0,intena(A4)           * disable all interrupts
	    MOVE.W D0,intreq(A4)           * clear all interrupts
	    MOVE.W D0,dmacon(A4)           * disable all dma
	    MOVE.W #BAUD_9600,serper(A4)   * set baud rate for debug stuff

	    MOVE.W #COLORON,bplcon0(A4)
	    MOVE.W #0,bpldat(A4)   * can't use CLR.W (it reads first)
	    MOVE.W #OK_DEBUG,color(A4)


	;Now kill all ROMTags in $F00000 memory
	    lea.l   CART_LOWER,a0
	    move.w  #RTC_MATCHWORD,d0
	    move.l  #((CART_UPPER-CART_LOWER)/2)-4,d1	;No tags at end

	    clr.l   (a0)+       ;Kill Exec ID
	    clr.l   (a0)+       ;Kill Exec ID
kf_loop     cmp.w   (a0)+,d0
	    bne.s   kf_nomatch	;No magic cookie...
	    move.l  a0,a1
	    subq.l  #2,a1
	    cmpa.l  (a0),a1
	    bne.s   kf_nomatch	;No back pointer...

	    ;---- found a valid ROMTag!, zap the back pointer! ----
	    clr.l   (a0)

kf_nomatch: subq.l  #1,d1
	    bne.s   kf_loop

	    clr.l   LOCATION_ZERO	;Kill any pending alert
	    clr.l   ABSEXECBASE

	    jmp    (a5)           ;jump back to where we came from
start_up:
cs_skipCart:
	ENDC




************************************************************************
*
*	Reset Major Hardware
*
************************************************************************

	IFEQ	A3000_ROMS
		; Only for non-A3000/A4000 systems...
		; Turn off the "mirror" CIA chips that float around
		; in the CD-Game Machine when it is connected to the
		; computer console.  These instructions will end up
		; turning off the normal CIA chips when in a standard
		; system setup due to the CIA address space warp.
		; (Should be of no consiquence in a standard system)
		clr.b	_ciaapra-$4000
		clr.b	_ciaaddra-$4000
	ENDC

	;------ turn off ROM phantom.  ciaapra=0 (LED ON,OVERLAY OFF)
	CLR.B	_ciaapra	;Unix ROM may have trashed this register...
	MOVE.B	#(CIAF_OVERLAY!CIAF_LED),_ciaaddra
	;------ we now have chip memory available! (poof...)

	;------ disable and clear all interrupts and DMA:
;	NPRINTF 2,<'clearing custom chips'>
	LEA.L	_custom,A4		* chipset base address
	MOVE.W	#$7FFF,D0		* clear all pattern
	MOVE.W	D0,intena(A4)           * disable all interrupts
	MOVE.W	D0,intreq(A4)           * clear all interrupts
	MOVE.W	D0,dmacon(A4)           * disable all dma
	MOVE.W	#BAUD_9600,serper(A4)   * set baud rate for debug stuff

	MOVE.W	#COLORON,bplcon0(A4)
	MOVE.W	#0,bpldat(A4)           * can't use CLR.W (it reads first)
	MOVE.W	#OK_HARDWARE,color(A4)



************************************************************************
*	Power up testing
*
*	The A3000 has a hardware bit that is set when power cycles.
*	If set, we clear all reboot functions, nuke the MMU, etc.
*
************************************************************************
*
*
*	Note-rom overlay is still enabled
*

	IFNE	A3000_ROMS
		;
		; The Gary chip has a bug; it does not clear itself on
		; software RESET.  This has been converted into a feature,
		; so we RESET the bits here.
		;
		; Sadly, there is nothing we can do about Ramsey (She
		; has some registers that, if trashed, stay trashed).
		;
		moveq	#0,d0
		move.b	d0,A3000_BusTimeoutMode		;DSACK timout
		move.b	d0,A3000_BusTimeoutEnable	;Timeout enabled

		moveq.l	#CACRF_ClearI!CACRF_EnableI,d0	;Disable D-Cache
		dc.w	$4E7B,$0002	;MOVEC D0,CACR
		dc.w	$4E7A,$0002	;MOVEC CACR,D0
		tst.l	d0
		beq.s	a3k_68040	;Skip MMU code for 68040 CPU. Sorry!

		;
		;  Test cold powerup detection bit
		;  WARNING: HOT reboot may have left funny stuff enabled...
		;
		btst.b	#7,A3000_PUD
		beq.s	a3k_warmboot

		NPRINTF 2,<'cold reboot detected'>

		;
		; A1000 systems do not have the magic MMU code...
		;
		IFNE	A1000_ROMS
			clr.l	ABSEXECBASE
			clr.l	LOCATION_ZERO
		ELSE
			;
			;  Leave zeros in all important CPU registers.  Try not
			;  to write to memory -- enforcer might be lurking.
			;
			lea.l	a3k_zeroes(pc),a0
			dc.w	$f010,$4000		; PMOVE (a0),TC
			dc.w	$f010,$0800		; PMOVE (a0),TT0
			dc.w	$f010,$0c00		; PMOVE (a0),TT1
			clr.l	ABSEXECBASE		; No Execbase
	    		clr.l   LOCATION_ZERO		; Kill any pending alert
			bra.s	a3k_exit_cold
a3k_zeroes:		dc.l	0
		ENDC

a3k_warmboot:

		IFNE	0
;#############################################################################
;	The following section of code was used in the Alpha-5
;	SuperKickstart boot ROMs.  It re-enables the MMU on
;	hard or soft reboot.
;
		;
		;  If MMU enabled, continue.
		;  If MMU _should_ be enabled, do so then reboot
		;	TODO: can we touch low memory??  Enforcer
		;
		clr.l	-(sp)
		dc.w	$f017,$4200		;PMOVE TC,(sp)
		IFGE	INFODEPTH-2
			move.l	(sp),d0
			NPRINTF 2,<'TC=%lx'>,D0
		ENDC
		tst.l	(sp)
		bmi.s	a3k_exit_on		;MMU already enabled!...
;		cmp.l	#SUPERKICK_TC_CONSTANT,(sp)
;		bne.s	a3k_exit_off		;Not a superkick reboot...
		beq.s	a3k_exit_off		;Not a superkick reboot...


		NPRINTF 2,<'Re-enabling MMU'>
		;
		;  Enable MMU and try again.
		;
		lea.l	a3k_startcode(pc),a0
		lea.l	$140,a1
		moveq	#(a3k_endcode-a3k_startcode)/4,d0
a3k_copyloop:	move.l	(a0)+,(a1)+
		dbra	d0,a3k_copyloop
		nop			;I don't trust these newfagled CPU's
		jmp	$140		;we never return from this jump

a3k_startcode:	bset.b	#7,(sp)         ;Set E bit to enable MMU
		dc.w	$f017,$4000	;PMOVE  (a7),TC
		nop			;sync pipeline
		jmp	$00f80002	;heavy cheating...
a3k_endcode:
;#############################################################################
a3k_exit_on:	NPRINTF 2,<'MMU already on'>
a3k_exit_off:	addq.l	#4,sp
		ENDC


a3k_exit_cold:
a3k_68040:	NPRINTF 2,<'End A3000 testing'>
		bclr.b	#7,A3000_PUD	;Clear cold-powerup indicator
	ENDC




************************************************************************
*
*	Check the ROM checksum we calculated earlier
*
*   REGISTERS
*	D5 = ROM Checksum (should be -1 if all is ok)
*
************************************************************************

	MOVE.W	#CC_BADROMSUM,D0
	IFNE	CHECKSUM_ROM
	not.l	d5		;-1 in D5 indicates a good checksum
	ENDC
	IFEQ	CHECKSUM_ROM
	moveq	#0,d5		;fake out checksum test
	ENDC
	bne	coldCrash



************************************************************************
*
*	Set up Temporary Processor Exception Vectors
*
*	I had some real neat code here to fill vectors and verify
*	the low 1K of memory.  It had to be removed for coldcapture/
*	games compatibility.  Some games would store information in
*	low memory, reboot, and expect the data.  Wayne Gretzky hockey
*	used location zero, for example.
*
************************************************************************

	;Old: Fill the traditional exception vectors with a temporary pointer.
	move.w	#BusErrorVector,a0	;Sign extends to 32 bits
	move.w	#($B4>>2),d1		;Fill to top of traps
	lea.l	coldExcept(pc),a1
cs_fillExcept:
	move.l	a1,(a0)+
	dbra	d1,cs_fillExcept


	;New: verify that the values actually took.  This is a cheap
	;power-on-only test for bad chip memory.   On soft reboot the
	;caches might invalidate this test.  Oh, well.
	move.w	#CC_BADRAM,d0
	move.w	#($B4>>2),d1
cs_checkExcept:
	cmp.l	-(A0),A1		;Tops down!
	bne	coldCrash
	dbf	D1,cs_checkExcept


	IFGE	INFODEPTH-20
	  move.l  LOCATION_ZERO,d0		;Check for "HELP" later
	  move.l  ABSEXECBASE,d1		;try to find old ExecBase
	  NPRINTF 20,<13,10,'LOCATION ZERO = %lx  ABSEXECBASE = %lx'>,d0,d1
	ENDC
	IFNE	TORTURE_TEST_0
	  move.l  #ZERO_COOKIE,LOCATION_ZERO
	ENDC


************************************************************************
*
*   REGISTERS
*	A7 -- temporary stack
*	A5 -- old ExecBase
*	A4 -- Alert Timeout
*	A3 -- top of chip ram
*	A2 -- temp (processor type)
*	DX -- information to copy to new ExecBase
*
*	Search for old ExecBase.  One of the following will be true:
*		1. There is nothing to find.
*		2. ExecBase is in "local" memory, available after RESET.
*		   This will be chip, $C00000 or motherboard memory.
*		   Here we have a chance of making games happy.
*		3. ExecBase is in autoconfigure memory, and won't be
*		   visible yet.
*
*******************************************************************************

		moveq	#0,d2
		moveq	#0,d3
		moveq	#0,d4
		moveq	#0,d5
		moveq	#0,d6
		moveq	#0,d7

		move.l	#DEFAULT_TIMEOUT,a4	; Default alert timeout

		move.l	ABSEXECBASE,d1
		move.l	d1,a6
		btst.b	#0,d1
		bne.s	cs_CorruptBase	* can't be odd...
		add.l	ChkBase(a6),d1
		not.l	d1
		bne.s	cs_NoBaseYet	* magic cookie tastes bad...

		;------ verify critical checksum:
		;[D1 - NULL]
		lea	SoftVer(a6),a0

		moveq	#(ChkSum-SoftVer)/2,d0
cs_ChkChk:	add.w	(a0)+,d1
		dbra	d0,cs_ChkChk
		not.w	d1
		bne.s	cs_NoBaseYet
		NPRINTF	4,<'CS-old ExecBase at %lx'>,a6

		;------ process to cold capture vector
		move.l	ColdCapture(a6),d0
		beq.s	cs_NoCapture


		move.l	d0,a0
		lea	cs_ReturnCap(pc),a5
		clr.l	ColdCapture(a6)	* prevent infinite reboot loop
		NPRINTF	4,<'ColdCapture-%lx'>,a0
		jmp	(a0)
cs_ReturnCap:	;NPRINTF 4,<'ColdCapture Return'>


cs_NoCapture:	movem.l	KickMemPtr(a6),d2/d3/d4	 ;D2-D4	- KickMem
		movem.l	ColdCapture(a6),d5/d6/d7 ;D5-D7	- Captures

; Very critical:  We MUST match versions otherwise we may get old NULL...
		cmp.w	#VERNUM,LIB_VERSION(a6)
		bne.s	cs_CorruptBase	; Skip AlertTimeout if not same...

		move.l	LastAlert+3*4(a6),a4     ;A4 = Alert Timeout...
cs_CorruptBase:	suba.l	a6,a6			 ;Search no more
cs_NoBaseYet:	move.l	a6,a5			 ;A5 - ExecBase for later...



*******************************************************************************
*	Before the chip memory test, turn off the data cache.  Soft reboot
*	may have screwed us.  We don't do this above to prevent fiddling with
*	the stack (those games can be picky).
*******************************************************************************

;------ determine processor type & turn on & flush instruction cache.
;------ this takes care of cache problems with the MakeFunctions below.
	NPRINTF 10,<'about to test CPU type...'>
	BSR	TypeOfCPU
	MOVE.L	D0,A2


;------ determine size of chip memory -----------------------------------------
;	a0 = zero
;	a1 = saved contents of location zero
;	a2 = <no touchie>
;	a3 = [End of chip]
;	a4 = <no touchie>
;	d0 = saved ram
;	d1 = magic value
;
	suba.l	a0,a0
	move.l	(a0),a1		;save contents of location zero
	clr.l	(a0)		;NULL location zero
	suba.l	a3,a3
	move.l	#$0F2D4B689,d1
	bra.s	enter_here

top_loop:
	move.l	d0,(a3)         ;restore location
enter_here:
	lea.l	$4000(a3),a3    ;bump to next 16K
	cmp.l	#LOC_MEM_UPPER,a3
	beq.s	endofmem
	move.l	(a3),d0
	move.l	d1,(a3)
	nop			;Force 68040 to write value to memory
	cmp.l	(a0),d1         ;check location zero for shadow
	beq.s	endofmem	;shadow detected...
	cmp.l	(a3),d1
	beq.s	top_loop	;more memory...

endofmem:
	move.l	d0,(a3)         ;restore last tested location
	move.l	a1,(a0)		;restore location zero
	NPRINTF 10,<'chip memory ends at $%lx'>,a3
;------------------------------------------------------------------------------
	;[A3=End of chip]



;------ build temporary memlist in chip memory
	move.l	d2,-(sp)
	LEA.L	LOWEST_USABLE,A0	; lower bound
;
	move.l	a2,d2			; Get CPU type...
	btst.l	#AFB_68040,d2		; Check if 68040
	beq.s	no_040
	lea.l	LOWEST_USABLE_040,A0	; Get lower memory bound of 040...
;
no_040:	LEA.L	cs_ChipName(pc),A1      ; ASCII identifier
	MOVE.L	A3,D0			; end to D0
	MOVE.L	A0,D1			; start to d1
	SUB.L	D1,D0			; End-start = size
	MOVE.W	#MEMF_24BITDMA!MEMF_LOCAL!MEMF_CHIP!MEMF_PUBLIC,D1
	MOVEQ	#CHIP_MEM_PRI,d2
	BSRSELF AddMemListInternal
	move.l	(sp)+,d2
*
* A0 is the memory header...
*
*******************************************************************************
*
* End of conditional ReKick/Normal code.  (Above is normal code)
*
	ENDC
*
*******************************************************************************

*******************************************************************************
* At this point we have a usable memory list in chip RAM, and can start
* to allocate out of it.
*******************************************************************************

;------ allocate a temporary sysbase and supervisor stack
	move.l	a0,-(sp)
	move.l	#EXECBASETOTAL,D0
	BSRSELF Allocate
	move.l	d0,a6
	SUB.W	#MAXSYSFUNC,a6		;MAXSYSFUNC is negative
	NPRINTF 10,<'temporary execbase at $%lx - size %lx'>,a6,#EXECBASETOTAL
	move.l	a6,ABSEXECBASE
	move.l	(sp)+,a1		; Get Memory header back...

	;------ clear out data area
	IFNE	(SYSBASESIZE&3)
	  FAIL	!!! SYSBASESIZE must be a longword multiple !!!
	ENDC
	MOVE.L	A6,A0
	MOVE.W	#((SYSBASESIZE>>2)-1),D1
cs_clrBase:
	CLR.L	(A0)+
	DBF	D1,cs_clrBase


******************************************************************************
*	Now set up just enough of Execbase to run RTF_SINGLETASK
******************************************************************************

;------ stash ROM revision number where people can find it
	move.w	KickstartRev(pc),SoftVer(a6)

;------ "cached" :-) above
	MOVE.W	A2,AttnFlags(A6)

;------ record local memory
	MOVE.L	A3,MaxLocMem(A6)

;------ restore stuff from old ExecBase (if found)
	move.l	a5,ChkBase(a6)			;Copy of location 4, or NULL
	movem.l	d2/d3/d4,KickMemPtr(a6)		;D3-D4	- KickMem
	movem.l	d5/d6/d7,ColdCapture(a6)	;D5-D7	- Captures
	move.l	a4,LastAlert+3*4(a6)		;A4 = Alert timeout...
	NPRINTF 10,<'ChkBase %lx KickMem %lx,%lx,%lx Captures %lx,%lx,%lx'>,a5,d2,d3,d4,d5,d6,d7

;------ save the old alert number
	moveq.l #-1,d6
	cmp.l	#'HELP',0
	bne.s	cs_NOalert
	movem.l ALERT_STORE,d6-d7
	bset.l	#31,d6	;Reboot alerts are never "recoverable" :-)
cs_NOalert:
	MOVEM.L D6-D7,LastAlert(A6)

;------ build the function table
	move.l	a1,-(sp)	; Save mem header...
	MOVE.L	A6,A0
	LEA.L	SysLibTab(PC),A1
	MOVE.L	A1,A2
	BSRSELF MakeFunctions		;cache clear handled above
	MOVE.W	D0,LIB_NEGSIZE(A6)      ;patch up func table size
	MOVE.W	#SYSBASESIZE,LIB_POSSIZE(A6)

;------ build a fake library list
	lea.l	LibList(a6),a0
	bsr	NewList		; a0 preserved

;------ Add our faked chip memory header to the system memory list
	lea.l	MemList(a6),a0
	bsr	NewList		; a0 preserved
	move.l	(sp)+,a1	; Get back mem header...
	BSRSELF Enqueue

;------ process $C00000 memory
	LEA.L	EXT_MEM_LOWER,a0
	LEA.L	EXT_MEM_UPPER,a1
	NPRINTF 400,<'About to sizeExtMem...'>
	bsr.s	sizeExtMem

	MOVE.L	A4,MaxExtMem(A6)

	lea.l	EXT_MEM_LOWER,a0
	lea.l	cs_FastName(pc),a1      ; ASCII identifier
	MOVE.L	A4,D0			; end to D0
	beq.s	cs_NoExtMem
	MOVE.L	A0,D1			; start to d1
	SUB.L	D1,D0			; End-start = size
	MOVE.W	#MEMF_24BITDMA!MEMF_LOCAL!MEMF_FAST!MEMF_PUBLIC,D1
	MOVEQ	#EXT_MEM_PRI,d2
	NPRINTF 10,<'Adding C00000 mem: base %lx,end-%lx,size-%ld'>,a0,a4,d0
	BSRSELF AddMemList
cs_NoExtMem:

;------ Find all resident tags
	NPRINTF 10,<'about to find SINGLETASK resident modules'>
	LEA	scanBounds(PC),A0
	BSR	FindCodeBefore
	MOVE.L	D0,ResModules(A6)

;----- resident code initialization:
	;
	; Environment at RTF_SINGLETASK time:
	;	A6=execbase.
	;	AllocMem works.
	;	ExecBase->ThisTask == 0
	;	68020 I-cache enabled
	;
	NPRINTF 18,<'About to init SINGLETASK resident modules'>
	MOVEQ.L #RTF_SINGLETASK,D0	* All code for single-tasking Exec
	MOVEQ.L #0,D1
	JSRSELF InitCode	;(D0,D1)

	NPRINTF 18,<'-After RTF_SINGLETASK- How did we get here??!!'>
	MOVE.W	#CC_NOMODULES,_color
cs_debug2:
	BRA.S	cs_debug2
*
* END OF COLDSTART
*
*******************************************************************************

;-----------------------------------------------------------------------------

;------ ROMTag search table area
;:TODO: - restrict $F00000 test to first n bytes.
;if a tag is found, then search the rest.
scanBounds:	XDEF	scanBounds
	DC.L	ExecOrigin
	DC.L	ExecOrigin+ROM_SIZE

	IFD	EXTROM_LOWER
		dc.l	EXTROM_LOWER
		dc.l	EXTROM_UPPER
	ENDC

	IFD	EXTROM1_LOWER
		dc.l	EXTROM1_LOWER
		dc.l	EXTROM1_UPPER
	ENDC

	DC.L	CART_LOWER
	DC.L	CART_UPPER
	DC.L	-1

cs_ChipName:	DC.B	'chip '
cs_FastName:	DC.B	'memory',0
		DS.W	0
;-----------------------------------------------------------------------------

*****i* Exec/Internal/sizeExtMem ***********************************************
*
*
********************************************************************************


sizeExtMem:	; ( a0: lower, a1: upper ), result in A4

	move.l	a0,a4
	add.l	#EXT_MEM_BLOCK,a0	; for mirror checking

sem_loop:
	;------ check if this is in the hardware chip space ------
	move.l	a4,a2
	add.l	#EXT_MEM_BLOCK,a2

	;------ clear intena
	move.w	#$3fff,intena-$1000(a2)

	;------ see if read side has cleared
	tst.w	intenar-$1000(a2)
	bne.s	sem_notchip

	;------ it did clear.  see if it changes
	move.w	#$bfff,intena-$1000(a2)
	cmp.w	#$3fff,intenar-$1000(a2)
	beq.s	sem_ischip

sem_notchip:
	;------ check if there is memory here
	move.w	#0,intena-$1000(a0)     ; clear location in block 0
	move.l	#$0F2D4,d1		; test pattern #1
	move.w	d1,intena-$1000(a2)     ;   write here
	cmp.w	intena-$1000(a2),d1     ;   verify write
	bne.s	sem_chkReturn		;   failed verify
	cmp.w	intena-$1000(a0),d1     ; check for echo
	beq.s	sem_echoPossible	;   only a possiblity

sem_checkPattern2:
	move.l	#$0B698,d1		; test pattern #2
	move.w	d1,intena-$1000(a2)     ;   write here
	cmp.w	intena-$1000(a2),d1     ;   verify write
	bne.s	sem_chkReturn		;   failed verify
	bra.s	sem_ismem		;   it's memory

sem_echoPossible:
	cmp.l	a0,a2			; check if echo is identity
	beq.s	sem_checkPattern2	;   yes, so it's not an echo

sem_ismem:
	;------ bump to next block
	move.l	a2,a4
	cmp.l	a4,a1
	bhi.s	sem_loop

sem_ischip:
	;------ clear out the int ena register
	move.w	#$7fff,intena-$1000(a2)

sem_chkReturn:
	;------ see if we allocated anything
	sub.l	#EXT_MEM_BLOCK,a0	; from mirror checking
	cmp.l	a0,a4
	bne.s	sem_return

	;------ there was no extended mem
	suba.l	a4,a4
sem_return:
	rts



******************************************************************************
*
*	Temporary exception handler
*
*	Handles ColdStart crashes that occur before the software is
*	set up.  These are always fatal.
*
******************************************************************************

coldExcept:
	; Only in debugging EXEC now...
	IFNE	INFODEPTH
		MOVEM.L	D0-D7/A0-A7,REG_STORE
		LEA.L	USP_STORE,a0
		MOVE.L	USP,A1
		MOVE.L	A1,(A0)+
		MOVE.L	(SP),(A0)+
		MOVE.L	4(SP),(A0)+
	ENDC


	AZ_TRIG 1
	IFNE	INFODEPTH
	   MOVEQ   #0,D0
	   MOVE.W  6(SP),D0
	   NPRINTF 2,<'Coldstart exception. [68010 Vector #=%lx]'>,D0

	   MOVE.L  SP,D0
	   MOVE.L  2(SP),D1	;Program counter
	   NPRINTF 2,<'USP=%lx SSP=%lx Program Counter=%lx'>,A1,D0,D1

	   MOVE.L  10(SP),D0	;PC
	   MOVE.L  02(SP),D1	;Fault Address
	   NPRINTF 2,<'If 68000 address error: PC=%lx, Fault at=%lx'>,D0,D1
	ENDC


	MOVE.W	#CC_EXCEPTION,D0


*
*
* The Offical system color crash (supervisor mode only):
*
*
coldCrash:
	NPRINTF 2,<'coldCrash'>
	LEA	_custom,A4		* chipset base address
	MOVE.W	#COLORON,bplcon0(A4)
	MOVE.W	#0,bpldat(A4)   ;can't use clr.w!
	MOVE.W	D0,color(A4)    ;everything else setup already

	;------ blink the LED 10 times:
	MOVEQ	#10,D1
	MOVEQ	#-1,D0
cc_on:
	BSET.B	#CIAB_LED,_ciaapra	; inside loop for delay
	DBF	D0,cc_on
	LSR	#2,D0			; change duty cycle
cc_off:
	BCLR.B	#CIAB_LED,_ciaapra	; inside loop for delay
	DBF	D0,cc_off
	DBF	D1,cc_on
	;[DROP]


*
*
* The Offical system reset (supervisor mode only):
*
*
		;[DROP]
CrashReset:


		MOVE.L	#$15000,D0
sr_delay:	;Use this instruction because it uses a non-cachable chip
		;memory cycle
		MOVE.W	#0,_color	;Fade to black...
		SUBQ.L	#1,D0
		BGT.S	sr_delay

		MOVE.W	#$4000,_intena	;DISABLE
		BRA	GoAway		;Execute RESET, jump, etc.


*****i* exec.library/ReadGayle ************************************************
*
*   NAME
*	ReadGayle - Read the Gayle ID register                           (V40)
*
*   SYNOPSIS
*	id=ReadGayle()
*	d0
*
*	ULONG ReadGayle(void);
*
*   FUNCTION
*	Check for the Gayle chip and if found, return the ID register...
*	Will return 0 if no Gayle...
*	--- PRIVATE ---  OS use only!  This function causes a bit of
*	a Disable()...
*
*   RESULT
*	id - The 8-bit ID from Gayle...
*	     0 if no Gayle is found...
*
*******************************************************************************

ReadGayle:	xdef	ReadGayle
		moveq.l	#0,d0			; Clear result...
		move.l	a5,a0			; Store here...
		lea	Do_Gayle(pc),a5		; Get address...
		JMPSELF	Supervisor		; Do the call...
		; Tail recursion helps out here...

Do_Gayle:	move.l	a0,a5			; Restore a5...
		; make sure this isn't a mirrored chip register (i.e. that we
		; have an IDE interface here).
		; hardware registers mirror every 512, starting at $dff000.
		; warning - different on different machines!
		; a500/a2000/a3000 - undecoded (random values or 0)
		; a1000 - mirrored chip regs
		; GAYLE id is at $de1000 (write anything, read back 8 times,
		; high bit has the data - pattern is $xx for current rev)

		; check by disable, play with possible intena, see if it
		; appears in the real intenar, then enable.
*
		lea	_custom,a0
		lea	$DE1000,a1		;GAYLE id address
*
		or.w	#$0700,SR		; Disable...  (RTE enables)
*
		move.w	intenar(a0),-(sp)	;save old value
		move.w	#$bfff,intena(a1)	;set all ables
		move.w	#$3fff,d1		;also flag for no mirror
		cmp.w	intenar(a0),d1
		bne.s	no_mirror
		move.w	d1,intena(a1)		;clear all ables
		tst.w	intenar(a0)
		bne.s	no_mirror
		moveq	#0,d1			;mirrored
no_mirror:					; leave d1 non-0
		; reset the saved values
		move.w	#$3fff,intena(a0)	;clear bits
		ori.w	#$8000,(sp)		;add set bit
		move.w	(sp)+,intena(a0)	;reset values

		tst.w	d1			;did we find mirroring?
		beq.s	no_hw			;yes, exit

		; no mirroring, can now check GAYLE id register safely
		move.b	d0,(a1)			;Write a Zero to flush bus...
		bsr.s	get_gid_bit		;bit 7
		bsr.s	get_gid_bit		; bit 6
		bsr.s	get_gid_bit		;  bit 5
		bsr.s	get_gid_bit		;   bit 4
		bsr.s	get_gid_bit		;    bit 3
		bsr.s	get_gid_bit		;     bit 2
		bsr.s	get_gid_bit		;      bit 1
		bsr.s	get_gid_bit		;       bit 0
*
no_hw:		cmp.b	#255,d0			; If 255, we return 0
		bne.s	ValidGayle		; If not 255, skip...
		moveq.l	#0,d0			; Set to 0...
ValidGayle:	rte

get_gid_bit:	; get a bit of the gayle ID register
		move.b	(a1),d1
		lsl.b	#1,d1			high bit into carry and x bits
		roxl.b	#1,d0			rotates x bit into low bit
		rts

		END
