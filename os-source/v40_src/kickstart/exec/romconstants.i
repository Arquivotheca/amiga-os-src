**
**	$Id: romconstants.i,v 39.15 93/05/18 10:27:18 mks Exp $
**
**	(C) Copyright 1985-91 Commodore-Amiga, Inc.
**	    All Rights Reserved
**


*
* Include the file needed for the machine defined
*
* The A3000 is the A3000 (or A3000+)
* The A2000 is the A500/A600/A2000 physical machine
* The A1200 is the AGA A1200 machine (used to be same as A2000)
* The A4000 is the A4000.
* The CDGS  is the AGA CD-Game Machine
*
* Labels are:  MACHINE_A3000	MACHINE_A2000	MACHINE_A4000	MACHINE_A1200
*              LOC_20*		LOC_20		LOC_20*		LOC_20*
*              LOC_F0*		LOC_F0		LOC_F0*		LOC_F0*
*              LOC_F8		LOC_F8		LOC_F8		LOC_F8
*
*              MACHINE_CDGS
*              LOC_20*
*              LOC_F0*
*              LOC_F8
*
* Symbols marked with "*" are not currently supported...
*
	IFD MACHINE_A3000
		IFD LOC_20
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F0
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F8
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 1		; Flag that this is for A3000
A3000_SUPERKICKSTART	EQU 1
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC
	ENDC

	IFD MACHINE_A2000
		IFD LOC_20
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
REKICK			EQU 1		; If defined, build of ReKick
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 0
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC

		IFD LOC_F0
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
CHECKSUM_ROM		EQU 0
A3000_ROMS		EQU 0
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC

		IFD LOC_F8
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 0
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC
	ENDC

	IFD MACHINE_A4000
		IFD LOC_20
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F0
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F8
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 1		; A1000 systems use this plus...
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 1		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC
	ENDC

	IFD MACHINE_A1200
		IFD LOC_20
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F0
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F8
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 9*60	; 9 seconds (NTSC) 10.8 seconds (PAL)
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 0
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC
	ENDC

	IFD MACHINE_CDGS
		IFD LOC_20
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F0
		FAIL	'Invalid configuration'
		ENDC

		IFD LOC_F8
*************** Tunable Parameters ********************************************
DEFAULT_TIMEOUT		EQU 0		; None...
EXTROM_LOWER		EQU $00E00000	; $00E00000 to $00E80000
EXTROM_UPPER		EQU $00E80000
EXTROM1_LOWER		EQU $00A80000	; $00A80000 to $00B80000
EXTROM1_UPPER		EQU $00B80000
CHECKSUM_ROM		EQU 1
A3000_ROMS		EQU 0
A3000_SUPERKICKSTART	EQU 0
A1000_ROMS		EQU 0		; A1000jr systems...

CART_LOWER		EQU $00F00000	; ROMTag search bounds
CART_UPPER		EQU $00F80000

SYSSTK_SIZE		EQU $001800 	; system stack size (6K)
USRSTK_SIZE		EQU $001000 	; initial exec.task stack size

LOC_MEM_LOWER		EQU $000000 	; local (chip) memory lower bound
LOWEST_USABLE		EQU $000400 	; start of allocatable chip memory
LOWEST_USABLE_040	EQU $001000	; Start of allocatable chip memory 040
LOC_MEM_UPPER		EQU $200000 	; local memory upper bound
CHIP_MEM_PRI		EQU -10

EXT_MEM_LOWER		EQU $c00000	; extended memory lower bound
EXT_MEM_UPPER		EQU $dc0000
EXT_MEM_BLOCK		EQU $040000 	; search granularity
EXT_MEM_PRI		EQU	-5
*******************************************************************************
		ENDC
	ENDC

*************** ROM ID codes for the first word of all ROMs *******************
*************** (The second word is a JMP instruction) ************************
OLD_ROMS	EQU $1111  ; 256K Exec ROMS
DIAG_CART	SET OLD_ROMS
EXEC_CART	EQU $1112  ; exec code for testing
UNIX_CART	EQU $1113  ; Amiga Unix ("Amix") boot ROMs: "thanks" Johann
BIG_ROMS	EQU $1114  ; 512K Exec ROMS
CDTV_CART	EQU $1115  ; CDTV add-on ROM
REKICK_ROMS	EQU $1116  ; ReKick roms...
PROG_CART	EQU $2222  ; program cartridge code

ROM_SIZE	EQU	$00080000	; Must be 64K multiple (512K currently)

*******************************************************************************
*
* BETA_BUILD is set to 1 for beta builds, 0 for final releases...
*
BETA_BUILD	set	1

* If we are final release, we turn off the alert
	IFD	FINAL_RELEASE
BETA_BUILD	set	0
	ENDC

* If we are no an F8 build, we turn on the alert (no non-F8 build is final)
	IFND	LOC_F8
BETA_BUILD	set	1
	ENDC
*
*******************************************************************************
