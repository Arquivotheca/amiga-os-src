**
**	$Id: carddata.i,v 1.2 92/12/14 13:47:03 darren Exp $
**
**	Credit card resource privates
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/semaphores.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"

	INCLUDE	"tuples.i"

	INCLUDE	"card.i"
	INCLUDE	"gayle.i"

CR_STKSIZE	EQU	512
	BITDEF	CRB,SIGTASK,31

 STRUCTURE	CardResourceBase,LIB_SIZE
	UBYTE	crb_Flags
	UBYTE	crb_Flags2
	APTR	crb_ExecLib
	APTR	crb_UtilLib

	;
	; crb_Owner points to owner (CardHandle struct),
	; NULL means there is a card, and no owner,
	; -1 means there is no card, and cannot be owned
	;

	APTR	crb_Owner

	; and pointer to CardHandle which has not responded to
	; removed interrupt

	APTR	crb_WasOwner

	; pointer to interrupt structure - call on status
	; change if not NULL

	APTR	crb_StatusCallBack

	; card change counter

	ULONG	crb_ChangeCount

	APTR	crb_TimerIO
	STRUCT	crb_NotifyList,MLH_SIZE	;list of handlers to be notified when
					;a card is inserted

	STRUCT	crb_Int6,IS_SIZE
	STRUCT	crb_Int2,IS_SIZE

	; a message port structure - dont want to allocate any signal bit

	STRUCT	crb_TimerPort,MP_SIZE

	; A CardHandle structure for owning myself

	STRUCT	crb_MyHandle,CardHandle_SIZEOF

	; Storage to interpret Device Tuple Data

	STRUCT	crb_DeviceTData,DeviceTData_SIZEOF

	ALIGNWORD			;makes it possible to clear all
					;masks with one instruction
	; MUST be in order, and aligned

	UBYTE	crb_SpeedMask		;mask - 2 bits for access speed

	UBYTE	crb_VoltageMask		;mask - 2 bits for progamming voltage
	
	UBYTE	crb_SpeedCache		;cache last access speed set

	UBYTE	crb_PadMask

	ALIGNLONG

	STRUCT	crb_Stk,CR_STKSIZE
	STRUCT	crb_TC,TC_SIZE

	LABEL	crb_SIZEOF

*------- crb_Flags -----------------------------------------------

	BITDEF	CR,ACTIVITY,0		;Card detect change activity
	BITDEF	CR,TIMERUSED,1		;TRUE if timer request used
	BITDEF	CR,CCDETECT,2		;TRUE if CC detect change int

*------- CardHandle private flags -------------------------------

	BITDEF	CH,NOTIFIED,7		;Handle has been notified of
					;the current card
*------- CardAccessSpeed() defines ------------------------------

CARD_SPEED_250NS	EQU	0	;default
CARD_SPEED_150NS	EQU     GAYLEF_MEMORY_SPEED0
CARD_SPEED_100NS	EQU	GAYLEF_MEMORY_SPEED1
CARD_SPEED_720NS	EQU	GAYLEF_MEMORY_SPEED0!GAYLEF_MEMORY_SPEED1



*------- hard-coded memory mapped locations ---------------------

COMMON_MEMORY		EQU	$600000
ATTRIBUTE_MEMORY	EQU	$A00000
IO_MEMORY		EQU	$A20000
FLASH_ROM		EQU	$A40000

COMMON_MEM_SIZE		EQU	$400000
ATTR_MEM_SIZE		EQU	$020000
IO_MEM_SIZE		EQU	$020000

*------ Other flags ---------------------------------------------

ROMBUILD		EQU	1	;set TRUE for ROM build
DISABLE_DCACHE		EQU	0	;set TRUE if D-Cache at bootup


*------ Other EQUates -------------------------------------------

FALSE			EQU	0
TRUE			EQU	1

* Useful change register masks

IDE_MASK		EQU	(GAYLEF_CHANGE_IDEINT)
BVD_MASK		EQU	(GAYLEF_CHANGE_BVD2!GAYLEF_CHANGE_BVD1)
WR_MASK			EQU	(GAYLEF_CHANGE_WR)
CC_MASK			EQU	(GAYLEF_CHANGE_CCDET)
IRQ_MASK		EQU	(GAYLEF_CHANGE_BSYIRQ)

MISC_MASK		EQU	(GAYLEF_CHANGE_WR!GAYLEF_CHANGE_BSYIRQ)

GAYLE_CCDET_MASK	EQU	(BVD_MASK!MISC_MASK!IDE_MASK)
GAYLE_CHANGED_MASK	EQU	(GAYLE_CCDET_MASK!CC_MASK)

SETRESET		EQU	IDE_MASK!CC_MASK!BVD_MASK!WR_MASK!IRQ_MASK!GAYLEF_DETRESET


* Useful status register masks

IDDET			EQU	GAYLEF_STATUS_IDEINT
CCDET			EQU	GAYLEF_STATUS_CCDET
BVDET			EQU	GAYLEF_STATUS_BVD1!GAYLEF_STATUS_BVD2
WRDET			EQU	GAYLEF_STATUS_WR
BIDET			EQU	GAYLEF_STATUS_BSYIRQ


*------ Macros --------------------------------------------------

** Simulate access light

SIMLIGHT_ON	MACRO	;simulate ON light
		IFNE	DEBUG_DETAIL
		bclr	#01,$bfe001
		ENDC
		ENDM

SIMLIGHT_OFF	MACRO	;simulate ON light
		IFNE	DEBUG_DETAIL
		bset	#01,$bfe001
		ENDC
		ENDM

LIGHT_OFF	MACRO	;turn access light OFF
		SIMLIGHT_OFF
		ENDM

LIGHT_ON	MACRO	;turn access light ON
		SIMLIGHT_ON
		ENDM

