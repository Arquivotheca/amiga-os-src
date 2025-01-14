**
**	$Id: resource.asm,v 39.10 93/05/04 14:58:49 darren Exp $
**
**	Credit card resource
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

** Includes

	INCLUDE	"exec/types.i"

	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/io.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/initializers.i"

	INCLUDE	"exec/ables.i"
	INCLUDE	"internal/librarytags.i"
	INCLUDE	"utility/utility.i"

	INCLUDE	"devices/timer.i"
	INCLUDE	"hardware/intbits.i"
	INCLUDE	"hardware/custom.i"

	INCLUDE	"carddata.i"
	INCLUDE	"gayle.i"
	INCLUDE	"cardres_rev.i"
	INCLUDE	"debug.i"

** Exports

	XDEF	OwnCard
	XDEF	ReleaseCard
	XDEF	BeginCardAccess
	XDEF	CardResetRemove
	XDEF	GetCardMap
	XDEF	ReadCardStatus
	XDEF	CardSetSpeed
	XDEF	CardAccessSpeed
	XDEF	IfRemoved

	XDEF	resname

	XDEF	ident

	XDEF    MemoryMap

** Imports

	XREF	_intena			;from amiga.lib
	XREF	_intenar		;
	XREF	_intreq			;
	XREF	_ciaa			;
	XREF	_custom			;

	XREF	_LVOPermit		;exec

	XREF	CopyTuple		;tuples.asm
	XREF	DeviceTuple
	XREF	IfAmigaXIP

	XREF	AddSysMem		;sysmem.asm

	XREF	EndModule		;EndModule.asm


** Assumptions

	IFNE	crb_VoltageMask-crb_SpeedMask-1
	FAIL	"crb_VoltageMask does not follow crb_SpeedMask, recode"
	ENDC

	IFNE	crb_SpeedCache-crb_VoltageMask-1
	FAIL	"crb_SpeedCache does not follow crb_VoltageMask, recode"
	ENDC
	IFNE	crb_PadMask-crb_SpeedCache-1
	FAIL	"crb_PadMask does not follow crb_SpeedCache, recode"
	ENDC



** Equates

SAVEROM		EQU	DEBUG_DETAIL
TESTGAYLE	EQU	0

** ROMTAG

	IFEQ	ROMBUILD

		moveq	#-1,d0
		rts

	ENDC

** ROM TAG for card.resource

crresident:
		dc.w	RTC_MATCHWORD
		dc.l	crresident
		dc.l	EndModule
		dc.b	RTF_COLDSTART
		dc.b	VERSION
		dc.b	NT_RESOURCE
		dc.b	48		;after timer.device
		dc.l	resname
		dc.l	ident
		dc.l	MakeResource


resname:
		CARDRESNAME

ident:
		VSTRING


		ds.w	0

CRStructInit:

	; library struct initialization

		INITLONG	LN_NAME,resname
		INITLONG	LIB_IDSTRING,ident
		INITWORD	LIB_REVISION,REVISION
		INITWORD	LIB_VERSION,VERSION
		INITBYTE	LN_TYPE,NT_RESOURCE

	; Init my card handle structure (used for system memory)
	
		INITLONG	crb_MyHandle+LN_NAME,resname
		INITBYTE	crb_MyHandle+cah_CardFlags,(CARDF_RESETREMOVE!CARDF_IFAVAILABLE)

	; Init message port for timer requests

		INITLONG	crb_TimerPort+LN_NAME,resname
		INITBYTE	crb_TimerPort+LN_TYPE,NT_MSGPORT
		INITBYTE	crb_TimerPort+MP_SIGBIT,CRBB_SIGTASK

	; Init card-change detect interrupt

		INITLONG	crb_Int6+LN_NAME,resname
		INITLONG	crb_Int6+IS_CODE,DetectChangeInt
		INITBYTE	crb_Int6+LN_TYPE,NT_INTERRUPT

	; after cia server

		INITBYTE	crb_Int6+LN_PRI,-127

	; Init status change detect interrupt

		INITLONG	crb_Int2+LN_NAME,resname
		INITLONG	crb_Int2+IS_CODE,StatusChangeInt
		INITBYTE	crb_Int2+LN_TYPE,NT_INTERRUPT

	; priority MUST be 127!  near top of list

		INITBYTE	crb_Int2+LN_PRI,127

	; Init task struct

		INITLONG	crb_TC+LN_NAME,resname
		INITBYTE	crb_TC+LN_PRI,15
		INITBYTE	crb_TC+LN_TYPE,NT_TASK

		dc.w	0

CRFuncInit:
		dc.w	-1

		dc.w	OwnCard-CRFuncInit
		dc.w	ReleaseCard-CRFuncInit

		dc.w	GetCardMap-CRFuncInit

		dc.w	BeginCardAccess-CRFuncInit
		dc.w	EndCardAccess-CRFuncInit

		dc.w	ReadCardStatus-CRFuncInit

		dc.w	CardResetRemove-CRFuncInit
		dc.w	CardMiscControl-CRFuncInit
		dc.w	CardAccessSpeed-CRFuncInit
		dc.w	CardProgramVoltage-CRFuncInit
		dc.w	CardResetCard-CRFuncInit

		dc.w	CopyTuple+(*-CRFuncInit)
		dc.w	DeviceTuple+(*-CRFuncInit)
		dc.w	IfAmigaXIP+(*-CRFuncInit)

		dc.w	CardForceChange-CRFuncInit
		dc.w	CardChangeCount-CRFuncInit

		dc.w	CardInterface-CRFuncInit
		dc.w	CardResVec1-CRFuncInit
		dc.w	CardResVec2-CRFuncInit
		dc.w	CardResVec3-CRFuncInit

		dc.w	-1

	; Card memory map structure - may be extended in the future
	; or some pointers may be NULLed should future machines
	; support a limited credit-card gate array (e.g., no support
	; for IO_MEMORY.
	;


		cnop	0,4
MemoryMap:
		dc.l	COMMON_MEMORY
		dc.l	ATTRIBUTE_MEMORY
		dc.l	IO_MEMORY
	; as of V39, CardMem structure has been extended

		dc.l	COMMON_MEM_SIZE
		dc.l	ATTR_MEM_SIZE
		dc.l	IO_MEM_SIZE

***********************************************************************

MakeResource:
	PRINTF	DBG_ENTRY,<'MakeResource()'>

		movem.l	d2/a2-a6,-(sp)

	PRINTF	DBG_FLOW,<'Checking for existance of GAYLE'>

	IFNE	GAYLEB_ID-7
	FAIL	"GAYLE ID bit not high bit, recode"
	ENDC

	IFNE	TESTGAYLE

	; make sure the gayleid register does not sit in some other
	; machines mirrored chip register space.  Based upon the same
	; code in the ide.device which uses the GAYLE gate array

		lea	_custom,a0
		lea	gayleregid,a1

		moveq	#00,d1

		DISABLE				;turn off master able

		move.w	intenar(a0),d0		;cache

	; note that we only WRITE to the gayleid register space,
	; never read, so if we are writing a gayleid register mirror,
	; we have no problem.

		move.w	#$bfff,intena(a1)	;set all ables
		move.w	#$3fff,d2		;and set flag for no mirror

		cmp.w	intenar(a0),d2
		bne.s	no_mirror

		move.w	d2,intena(a1)		;clear all ables
		tst.w	intenar(a0)
		bne.s	no_mirror

		moveq	#1,d1			;mirrored == TRUE

no_mirror:
		move.w	d2,intena(a0)		;clear ables
		ori.w	#$8000,d0		;add set bit
		move.w	d0,intena(a0)		;restore ables

		ENABLE

		tst.w	d1
		bne	resource_failed

		moveq	#03,d2			;read 4 bits
	;;;	moveq	#00,d1

		move.b	d1,(a1)			;write to register
readidreg:
		move.b	(a1),d0
		lsl.b	#1,d0			;high bit into carry and x
		roxl.b	#1,d1			;CAPE optimizes this
		dbf	d2,readidreg

		cmp.b	#(GAYLE_ID>>4),d1
		bne	resource_failed

	PRINTF	DBG_FLOW,<'GAYLE ID Register matched'>

	ENDC

	;; save ROM, use exec code

		move.l	$4,a6
		JSRLIB	ReadGayle
		lsr.b	#4,d0			;check upper 4 bits (VER) only
		cmp.b	#(GAYLE_ID>>4),d0
		bne	resource_failed
		
	;;; Quick way to check for expansion RAM > 4Megs on A1200
	;;; if any, skip card.resource stuff

		move.l	#(COMMON_MEMORY+$100),a1	;skip memheader
		JSRLIB	TypeOfMem

		tst.l	d0
		bne	resource_failed

	;;; Enable interface (even if we fail to make the resource!

		move.b	#00,gaylestatus

	PRINTF	DBG_FLOW,<'0==TypeOfMem()'>

		lea	CRFuncInit(pc),a0
		lea	CRStructInit(pc),a1
		sub.l	a2,a2			;no initializer
		move.l	#crb_SIZEOF,d0
		JSRLIB	MakeLibrary

	PRINTF	DBG_OSCALL,<'Card Resource Library Base @ $%lx'>,D0

		tst.l	d0
		beq	resource_failed

		move.l	d0,a5
		move.l	a6,crb_ExecLib(a5)

	;-- As this will be an early init ROM module, no error
	;-- check is done for the following memory/resource allocations
	;-- since ROM space is very tight, and there is no reason why
	;-- any of this should fail short of catastrophe


		moveq	#OLTAG_UTILITY,d0

	PRINTF	DBG_OSCALL,<'TaggedOpenLibrary($%lx,$%lx)'>,D0

		JSRLIB	TaggedOpenLibrary

	PRINTF	DBG_OSCALL,<'Utility Library @ $%lx'>,D0

		move.l	d0,crb_UtilLib(a5)
		beq	resource_failed		;what??

	;-- init notification list

		lea	crb_NotifyList(a5),a0
		NEWLIST	a0

	;-- init timer request

		lea	crb_TimerPort(a5),a0

	PRINTF	DBG_OSCALL,<'MessagePort @ $%lx'>,A0

		move.l	a0,a3			;cache
		moveq	#IOTV_SIZE,d0

		JSRLIB	CreateIORequest

	PRINTF	DBG_OSCALL,<'IORequest @ $%lx'>,d0

		move.l	d0,crb_TimerIO(a5)
		move.l	d0,a1			;cache IO Request

	;-- Open timer.device
		suba.l	a0,a0			;use ODTAG_TIMER
	;	lea	timername(pc),a0
		moveq	#UNIT_VBLANK,d0
		moveq	#00,d1

		JSRLIB	OpenDevice

	PRINTF	DBG_OSCALL,<'OpenDevice() returned %ld'>,d0

	;-- init notification task, and message port

		lea	crb_TC(a5),a1

		move.l	a1,MP_SIGTASK(a3)	;task to signal

		lea	MP_MSGLIST(a3),a0
		NEWLIST	a0

		lea	crb_Stk(a5),a0

		move.l	a0,TC_SPLOWER(a1)
		move.w	#((CR_STKSIZE/2)-1),d0
		move.w	#$09999,d1
pattern0Loop:
		move.w	d1,(a0)+
		dbf	d0,pattern0Loop
	;	lea	crb_Stk+CR_STKSIZE(a5),a0
		move.l	a0,TC_SPUPPER(a1)
		move.l	a5,-(a0)		;argument (resource base)
		move.l	a0,TC_SPREG(a1)

		lea	NewCard(pc),a2
		suba.l	a3,a3

	PRINTF	DBG_OSCALL,<'AddTask($%lx,$%lx,$%lx)'>,A1,A2,A3

		JSRLIB	AddTask

	PRINTF	DBG_OSCALL,<'AddTask() returned'>

	;-- initialize card-change detect interrupt

		lea	crb_Int6(a5),a1
		move.l	a5,IS_DATA(a1)

	;-- add card-change detect interrupt (level 6)

		moveq	#INTB_EXTER,d0
		JSRLIB	AddIntServer

	PRINTF	DBG_OSCALL,<'INTB_EXTER Interrupt server added'>

	;-- determine initial state of card (in or out)

		DISABLE

	;-- Clear all status change ints except IDE
	;-- Clear any pending CC detect change due to exec
	;-- turning off CC interface, me turning it back on

		move.b	#IDE_MASK,gaylechange

	;-- tell card to generate level 6 interrupts on change-detect

		bset	#GAYLEB_INT_CCDET,gayleint

	PRINTF	DBG_HARDWARE,<'Level 6 card-change int turned on'>

		moveq	#-1,d2		;mark as empty slot

	;-- check status bit for insertion; no problem if we
	;-- in a state of card insert/remove since we'll
	;-- catch it real soon in the interrupt code.

                btst    #GAYLEB_STATUS_CCDET,gaylestatus
		beq.s	nocardpresent

		bsr	ApplyReset
		
	PRINTF	DBG_FLOW,<'We have a card in the slot'>

		moveq	#00,d2		;mark as available

nocardpresent:
	;-- mark as empty slot, or card present

		move.l	d2,crb_Owner(a5)
		
		ENABLE

	;-- initialize status change detect interrupt

		lea	crb_Int2(a5),a1
		move.l	a5,IS_DATA(a1)

	;-- add it

		moveq	#INTB_PORTS,d0
		JSRLIB	AddIntServer

	;-- and turn on interrupts for change, but not BVD2/DA since
	;-- we do not want a lot of interrupts during digital audio.
	;-- If BVD2 signals a warning condition, though it can only
	;-- be monitored by the owning .device.  We'll use infrequent
	;-- polling to check battery in a battery checker.  BVD1 is
	;-- for SC, which will be necessary for some programmable devices
	;-- as a means of indicating a change in RDY/BSY status.


SETREG	SET	GAYLEF_INT_BVD1!GAYLEF_INT_WR!GAYLEF_INT_BSYIRQ

		bclr	#GAYLEB_INT_BVD2,gayleint	;turn off BVD2
		ori.b	#SETREG,gayleint

	;-- These registers states are assumed (per hardware spec)
	;
	; Gayle Status Regiser -
	;
	;	CC det	- check as needed
	;	BVD2	- don't care (let application deal with this)
	;	BVD1	- don't care (let application deal with this)
	;	WR	- don't care (let application deal with this)
	;	BSYIRQ	- don't care (let application deal with this)
	;	DIGAUD	- don't care (let application deal with this; 0 default)
	;	CC dis	- Enable; disabled by early expansion code
	;
	; Gayle Change Register -
	;
	;	CC det  - Don't care.  If card is inserted AFTER
	;		  powerup, but before we add the interrupt,
	;		  then it will be debounced, and likely we
	;		  WONT be able to use it as system memory.
	;		  This isn't a problem for execute-in-place cards.
	;
	;	BVD2	- don't care (interrupt as needed)
	;	BVD1	- don't care (interrupt as needed)
	;	WR	- don't care (interrupt as needed)
	;	BSYIRQ	- don't care (interrupt as needed)
	;	RESET	- Programmed by this resource (initially OFF
	;		  per hardware spec)
	;	BERR	- Assumed to be OFF per hardware spec (not used
	;		  by resource because interrupts, and reset
	;		  are adequate).
	;
	; Gayle Int Register -
	;
	;	CC det	- Turned ON by this resource (enable level 6 int
	;		  on CC change detect)
	;	BVD2	- Turned ON by resource.
	;	BVD1	- Turned ON by resource.
	;	WR	- Turned ON by resource.
	;	BSYIRQ	- Turned ON by resource.
	;	BVD lev	- Programmed by resource (0 = level 2 int - default)
	;	BSY lev	- Programmed by resource (0 = level 2 int - default)
	;
	; Gayle Control Register -
	;
	;	CC Memory 1	- to be programmed as needed
	;	CC Memory 2	- to be programmed as needed
	;	Program 12V	- assumed to be 0 (per spec)
	;	Program 5V	- assumed to be 0 (per spec)
	;
	;			- Default access speed is 250ns
	;			- Default program voltage is 0V

	;-- simulate access light by bringing LED to default (dimmed)

	SIMLIGHT_OFF

	;-- try to configure card as system memory

		bsr	AddSysMem

	;-- add resource if not being used as system memory

		lea	crb_MyHandle(a5),a0
		cmpa.l	crb_Owner(a5),a0
		beq.s	resource_failed
				
	;-- clear reset on remove flag so this handle can be
	;-- used by strap; saves a bit of ROM space sharing the
	;-- handle in this way

		bclr	#CARDB_RESETREMOVE,cah_CardFlags(a0)

	;-- strap, and carddisk.device will not bother if they
	;-- cannot open the resource - as should be the case for
	;-- other devices.  This saves some run-time memory, since
	;-- card ownership is impossible

		movea.l	a5,a1
		JSRLIB	AddResource

	PRINTF	DBG_OSCALL,<'Resource added'>

	;-- simulate access light by bringing LED to default (off)
resource_failed:


		movem.l	(sp)+,d2/a2-a6

	PRINTF	DBG_EXIT,<'Exit MakeResource()'>

	; fall through to RTS

*****i* card.resource/ReservedVectors *********************************
*
*   NAME
*	Reserved Vectors for future use as needed.
*   
***********************************************************************
CardResVec1:
CardResVec2:
CardResVec3:

	; fall through

******* card.resource/CardInterface ***********************************
*
*   NAME
*	CardInterface -- Determine the type of card interface.
*   
*   SYNOPSIS
*	return = CardInterface()
*	d0
*
*	ULONG CardInterface( void );
*
*   FUNCTION
*	This function is used to determine the type of credit-card
*	(hardware) interface available.  For the most part the
*	card.resource hides the hardware details from devices within its
*	function calls.  However should we need to provide a work-around
*	because of differences, or limitations imposed by future interface
*	hardware, this function must be used to identify which interface
*	is available.
*
*   RETURN
*	A ULONG value as defined in card.h/i.
*
*   NOTES
*	In general only I/O devices (e.g., a device which interfaces
*	with a modem card) would need to provide work-arounds,
*	or alternative code.  An example would be a change in the way
*	interrupt requests from the card are handled.  Specific details
*	will be provided as need in the future.  I/O devices) should abort
*	properly if this function returns a value which is unknown.
*
*	Current implementations (see card.h/i) -
*
*	CARD_INTERFACE_AMIGA_0
*	-------------------------------------------------------------
*
*	The card slot can be configured for use as an I/O interface
*	by using the CardMiscControl() function.
*
*	The card slot inhibits writes to cards which do not negate
*	the WP status bit.  This can be overridden by using the
*	CardMiscControl() function.
*
*	Changes in the interrupt request line are latched by a gate-array,
*	and have to be obtained via the status change mechanism provided
*	when you call the OwnCard() function.  The interrupt is cleared
*	when you return from the status change interrupt.  A level 2
*	interrupt is generated.  Usually you will want to clear the
*	interrupt on the card at this time, and Signal() a task.  The IRQ
*	line is the same as the RDY/BSY line.
*
*	Changes in BVD1, WP, and RDY/BSY are also latched by the gate-array,
*	and are obtainable via the status change mechanism provided by
*	the OwnCard() function.  A level 2 interrupt is generated.
*
*	Changes in BVD2 (also used for digital audio) have to be
*	monitored via polling.  Generally this will cause no problem.
*	Monitoring changes in BVD1 & BVD2 to monitor for low battery
*	condition can be handled by a low priority tool which periodically
*	checks the condition of both lines using the ReadCardStatus()
*	function.
*
*	As of card.resource V39 (check VERSION in resource base), the
*	CardMiscControl() function can be used to enable/disable
*	status change interrupts for changes in BVD1, BVD2, and the
*	RDY/BSY status line.  Status change interrupts for WR
*	(Write-protect enable/disable) are always enabled.  The default
*	state of enabled/disabled status change interrupts noted above
*	are unchanged, and automatically reset to the defaults when
*	a card is removed, or when even a task releases ownership
*	of the card.
*
*	Some PC oriented eight (8) bit cards may require you read
*	odd-byte I/O address registers at the corresponding even-byte
*	address plus 64K.  There is sufficient I/O address space
*	provided that exceeding I/O address space should not be a problem.
*
*	Your code should wait at least 1 millisecond for Vpp to stabilize
*	after voltage change (see CardProgramVoltage()).
*
*    SEE ALSO
*	CardMiscControl(), resources/card.i, resources/card.h
*
***********************************************************************

*	CARD_INTERFACE_CDTV_0
*	 -------------------------------------------------------------
*
*	The card slot cannot be fully configured for use as an I/O interface,
*	though you must still try to do so using the CardMiscControl()
*	function.  This tells the CDTV operating system to ignore changes
*	in BVD1 & BVD2; the front panel battery indicator is turned off.
*	Interrupts from the card are also enabled, and disabled via
*	the CardMiscControl() function.
*
*	The card slot does not inhibit writes to cards, regardless of
*	the state of the WP bit.
*
*	Changes in the interrupt request line are tied directly to the
*	interrupt controller which interrupts the CPU.  A level 2
*	interrupt is generated until the interrupt is cleared on the
*	card (e.g. responding to the interrupt by writing to a control
*	register on the card).  The IRQ line is the same as the RDY/BSY
*	line.  Therefore you must use Exec's AddIntServer() function
*	to install an INTB_PORTS interrupt server.  The I/O card must be
*	capable of generating level sensitive interrupts like those used
*	by the Amiga.  You must configure the card slot for the I/O
*	interface to enable interrupts from the card.
*
*	Changes in BVD1, WP, and RDY/BSY are obtained by polling during
*	VBLANK.  These changes can be obtained via the status change
*	interrupt mechanism provided by OwnCard().
*
*	Changes in BVD2 have to be monitored via polling.  See note
*	above.
*
*	Digital Audio (audio waveforms generated by the card, and mixed
*	with Amiga audio) is not supported by this interface.
*
*	Some PC oriented eight (8) bit cards may require you read
*	odd-byte I/O address registers at the corresponding even-byte
*	address plus 64K.  There is sufficient I/O address space
*	provided that exceeding I/O address space should not be a problem.
*
*	Your code should wait at least 1 millisecond for Vpp to stabilize
*	after voltage change (see CardProgramVoltage()).
*
CardInterface:
		moveq	#00,d0
		rts

******* card.resource/OwnCard *****************************************
*
*   NAME
*	OwnCard -- Own credit card registers, and memory
*   
*   SYNOPSIS
*	return = OwnCard( handle )
*	d0		a1
*
*	struct CardHandle *OwnCard( struct CardHandle * );
*
*   FUNCTION
*	This function is used to obtain immediate, or deferred
*	ownership of a credit-card in the credit-card slot.
*
*	Typically an EXEC STYLE DEVICE will be written to interface
*	between an application, and a credit card in the slot.  While
*	applications, and libraries can attempt to own a credit-card
*	in the card slot, the rest of this documentation assumes a
*	device interface will be used.
*
*	Because credit-cards can be inserted, or removed by the user at
*	any time (otherwise known as HOT-INSERTION, and HOT-REMOVAL),
*	the card.resource provides devices with a protocol which
*	lets many devices bid for ownership of a newly inserted card.
*
*	In general, devices should support HOT-REMOVAL, however there
*	are legitimate cases where HOT-REMOVAL is not practical.  For
*	these cases this function allows you to own the resource using
*	the CARDB_RESETREMOVE flag.  If the card is removed before your
*	device calls ReleaseCard(), the machine will RESET.
*
*   INPUTS
*	handle - pointer to a CardHandle structure.
*
*		struct CardHandle {
*		struct Node 		 cah_CardNode;
*		struct Interrupt	*cah_CardRemoved;
*		struct Interrupt	*cah_CardInserted;
*		struct Interrupt	*cah_CardStatus;
*		UBYTE			 cah_CardFlags;
*		};
*
*	The following fields in the structure must be filled
*	in by the application before calling OwnCard() -
*
*	cah_CardNode.ln_Pri -
*
*		See table below.  The Node field is used by the resource to
*		add your handle to a sorted list of CardHandle structures. 
*		This list is used by the resource to notify devices when the
*		device owns the credit-card.
*
*		Your device will only be notified (at most) one time
*		per card insertion, and perhaps less often if some
*		higher priority device on the notification list retains
*		ownership of a card in the slot.
*
*	Priority	Comments
*	--------------------------------------------------
*		>= 21	Reserved for future use
*
*		10-20	To be used by third party devices (e.g.,
*			I/O CARD manufacturers) which look for
*			specific card tuples to identify credit-cards.
*
*		01-19	Reserved for future use
*
*		00	To be used by general purpose devices which
*			have loose card specification requirements.
*
*		<= -1	Reserved for future use
*
*		  
*	cah_CardNode.ln_Type -
*
*		Must be set to 0 for now.  This field may be used in the
*		future to identify an extended CardHandle structure.
*
*	cah_CardNode.ln_Name -
*
*		Must be initialized to NULL, or name of device which owns
*		this structure.
*
*	cah_CardRemoved -
*
*		Pointer to an initialized interrupt structure.  Only the
*		is_Data, and is_Code fields need to be initialized.  This
*		is the interrupt code which will be called when a credit-card
*		which your device owns is removed.  Once you receive this
*		interrupt, all credit-card interface control registers are
*		reset (e.g., programming voltage, access speed, etc.), and 
*		you should stop accessing the card as soon as possible.
*
*		Because your code is called on interrupt time, you
*		should do the least amount possible, and use little
*		stack space.
*
*		This pointer can be NULL if you have asked for reset
*		on card-removal, and you never turn reset off.
*
*	cah_ CardInserted -
*
*		Pointer to an initialized interrupt structure.  Only the
*		is_Data, and is_Code fields need to be initialized.  This
*		is the code which will be called when your CardHandle owns
*		the credit-card in the slot.
*
*		Note that your code may be called on the context of
*		an interrupt, or a task in FORBID, therefore you should
*		do the least amount possible, and use little stack space.
*
*		Note that it is possible to receive a card removed
*		interrupt immediately before you receive this interrupt if
*		the card is removed while your CardInserted interrupt
*		is being called.
*		
*		Your device owns the credit-card until the card is manually
*		removed by the user, or you release the card by calling
*		ReleaseCard().
*
*		Your device should examine the card in the slot (e.g., look
*		for specific tuples), and decide if the card is of a type your
*		device understands.
*
*		If not, release ownership of the card by calling
*		ReleaseCard() so that other devices will be given a chance to
*		examine the current card in the credit-card slot.
*
*	cah_CardStatus -
*
*		Pointer to an initialized interrupt structure.  Only the
*		is_Data, and is_Code fields need to be initialized.
*
*		Note that your code will be called on the context of
*		an interrupt, therefore you should do the least
*		amount possible, and use little stack space.
*
*		Note that it is possible to receive a card removed
*		interrupt immediately before you receive this interrupt
*		if the card is removed during this interrupt.
*		
*		If this pointer is NULL, you will not receive card status
*		change interrupts.
*
*		Your interrupt code will be called with a mask value in
*		register D0, and a pointer to your data in A1.
*
*		The mask value in D0 can be interpreted using the same bit
*		definitions returned by ReadCardStatus().  Note that more
*		than one bit may be set, and the mask only tells you what has
*		changed, not the current state.
*
*		Use ReadCardStatus() if you need to determine the current
*		state of the status bits.
*
*		Not all status change interrupts will necessarily be
*		enabled on all systems.  For example, on some systems
*		BVD2/DA status change interrupts will not be enabled so
*		that digital audio can occur without generating many
*		interrupts.  Status change interrupts are typically meant
*		to be used for monitoring BSY/IRQ, WR, and BVD1/SC.  Battery
*		voltage low detection would best be done by a separate
*		utility which periodically polls BVD1 & BVD2 by using the
*		ReadCardStatus() function.
*
*		Typically the mask value in D0 MUST be returned unchanged
*		on exit from your code.  The return value in D0 is then used
*		to clear the source(s) of the interrupt.
*
*		In the rare case that you need to keep a status change
*		interrupt active, clear the appropriate bit(s) in D0 before
*		returning via RTS.  Clear no bits other than those defined
*		as valid bits for ReadCardStatus()!
*
*		!!!NEW FOR V39!!!
*
*		See definition of CARDB_POSTSTATUS below.
*
*	cah_CardFlags -
*
*		Optional flags (all other bits must be 0).
*
*		- CARDB_RESETREMOVE means you want the machine to
*		  reset if the credit-card in the slot is removed
*		  while you own the credit-card.
*
*		- CARDB_IFAVAILABLE means you only want ownership of
*		  the credit-card in the slot if it is immediately
*		  available.
*
*		If it is available, your CardHandle structure will be added
*		to a list so that you can be notified via an interrupt when
*		the credit-card is removed by the user.
*
*		If the credit-card is not immediately available (either
*		because there is no credit-card in the slot, or because some
*		other device owns the credit-card), your CardHandle structure
*		will NOT be added to the notification list.
*
*		- CARDB_DELAYOWNERSHIP means you never want a successful
*		  return from OwnCard() even if the credit-card is
*		  available.  Rather you will be notified of ownership
*		  via your cah_CardInserted interrupt.  If you use this flag,
*		  OwnCard() will always return -1.  This flag cannot be used
*		  with the CARDB_IFAVAILABLE flag.
*
*		- CARDB_POSTSTATUS is new for V39 card.resource (check
*		  resource base VERSION before using).  It is meant to be
*		  used by drivers which want to service the card hardware
*		  AFTER the status change interrupt has been cleared on the
*		  gate array.  Previously a PORTS interrupt server had 
*		  to be added to do this; this is somewhat more efficient, and
*		  easier to use.  Your status change interrupt is first called
*		  with status change bits in register D0.  You would examine
*		  these bits, and set a flag(s) for the POST callback.  When
*		  you return from the status change interrupt, the interrupt
*		  on the gate array is cleared (based on what you return in
*		  register D0), and your status change interrupt is immediately 
*		  called again, but this time with 0 in D0.  The value you
*		  return in D0 for the POST callback case is ignored.
*		  
*
*	ALL other fields are used by the resource, and no fields in the
*	structure may be modified while the structure is in use by the
*	resource.  If you need to make changes, you must remove your
*	CardHandle (see ReleaseCard()), make the changes, and then call
*	OwnCard() again.
*
*   RESULTS
*	 0  - indicates success, your device owns the credit card.
*
*	-1  - indicates that the card cannot be owned (most likely 
*	      because there is no card in the credit card slot).
*
*	ptr - indicates failure.  Returns pointer to the CardHandle
*	      structure which owns the credit card.
*
*   NOTES
*	This function should only be called from a task.
*
*  	CardHandle interrupts are called with a pointer to your data
*	in A1, and a pointer to your code in A5.  With the exception
*	of status change interrupts, D0-D1, A0-A1, and A5-A6 may be
*	treated as scratch registers.  Status change interrupts are
*	also called with meaningful data in D0, and expect D0 be
*	preserved upon RTS from your code.  No other registers are
*	guaranteed to contain initialized data.  All other registers
*	must be preserved.
*
*    SEE ALSO
*	ReleaseCard(), ReadCardStatus(), resources/card.i, resources/card.h
*
**********************************************************************
OwnCard:

	PRINTF	DBG_ENTRY,<'OwnCard($%lx)'>,a1

		movem.l	a5-a6,-(sp)

		move.l	crb_ExecLib(a6),a5
		exg	a5,a6

	; check for delayed option

		btst	#CARDB_DELAYOWNERSHIP,cah_CardFlags(a1)
		BEQ_S	notdelayed

	PRINTF	DBG_FLOW,<'DELAYOWNERSHIP flag set'>

	; add to list

		lea	crb_NotifyList(a5),a0

		FORBID

	PRINTF	DBG_OSCALL,<'Enqueue($%lx,$%lx)'>,A0,A1

		JSRLIB	Enqueue

		PERMIT

		bsr	NotifyInsert

		movem.l	(sp)+,a5-a6
		moveq	#-1,d0			;always return failure
		rts

notdelayed:

		DISABLE
	; This work is all done inside of DISABLE so that during this
	; time the resource cannot take an interrupt if the card is
	; removed while ownership is being granted.  IF the card is
	; removed, ownership will be lost again immediately.

	; check if card is available

		tst.l	crb_Owner(a5)
		BNE_S	cardowned

	; reset all gayle registers to known state

		bsr	ResetGayleRegs

	; see if they want reset on card removal

		btst	#CARDB_RESETREMOVE,cah_CardFlags(a1)
		beq.s	addnotify

	PRINTF	DBG_FLOW,<'RESET on REMOVE requested'>

	; tell gayle to hardware RESET machine on card-change
	; set DETRESET - may reset immediately if the CC_DET
	; bit is already set - ok, fine, so we reboot - no different
	; then if the user pulls the card right after this instruction

		move.b	#SETRESET,gaylechange

addnotify:


	; grant ownership of card

	PRINTF	DBG_FLOW,<'Card ownership granted'>

		move.l	a1,crb_Owner(a5)
		move.l	cah_CardStatus(a1),crb_StatusCallBack(a5)

	; add to list

		lea	crb_NotifyList(a5),a0

	PRINTF	DBG_OSCALL,<'Enqueue($%lx,$%lx)'>,A0,A1

		JSRLIB	Enqueue

                moveq   #00,d0                  ;TRUE

attemptdone:
		ENABLE

	PRINTF	DBG_EXIT,<'$%lx=OwnCard()'>,D0

		movem.l	(sp)+,a5-a6
		rts


	; *************************************************************
	; Credit card is owned, or missing
	; *************************************************************

cardowned:
	; see if they wanted immediate mode only

		btst	#CARDB_IFAVAILABLE,cah_CardFlags(a1)
		bne.s	refusecard


	; no, so add to list of handlers
		
		lea	crb_NotifyList(a5),a0

	PRINTF	DBG_OSCALL,<'Enqueue($%lx,$%lx)'>,A0,A1

		JSRLIB	Enqueue

refusecard:
		move.l	crb_Owner(a5),d0		;-1, or ptr
		BRA_S	attemptdone

******* card.resource/ReleaseCard *******************************
*
*   NAME
*	ReleaseCard -- Release ownership of credit card
*   
*   SYNOPSIS
*	ReleaseCard( handle, flags )
*		     a1	     d0
*
*	void ReleaseCard( struct CardHandle *, ULONG );
*
*   FUNCTION
*	This function releases ownership of the credit card in the
*	slot.
*
*	The access light (if any) is automatically turned off
*	(if it was turned on) when you release ownership of
*	a card you owned, and all credit-card control registers
*	are reset to their default state.
*
*	You must call this function if -
*
*	You own the credit-card, and want to release it so that
*	other devices on the notification list will have a chance
*	to examine the credit-card in the card slot.
*
*	You took a Card Removed interrupt while you owned the
*	credit-card.  If so, you MUST call this function, else
*	no other task will be notified of newly inserted cards.  On
*	some machines the credit-card interface hardware may also
*	be left disabled until you respond to the card removed interrupt
*	by calling this function.
*
*	You want to remove yourself from the notification list (see
*	optional flags below).
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	flags - Optional flags.
*
*		- CARDB_REMOVEHANDLE means you want remove your
*		CardHandle structure from the notification list
*		whether or not you currently own the credit-card
*		in the card slot.  The node structure in your
*		CardHandle will be removed from the notification
*		list, and ownership will be released if you were
*		the owner of the card.
*
*   NOTES
*	This function should only be called from a task.
*
*    SEE ALSO
*	OwnCard(), resources/card.i, resources/card.h
*
**********************************************************************
ReleaseCard:

	PRINTF	DBG_ENTRY,<'ReleaseCard($%lx,$%lx)'>,a1,d0


		movem.l	a5-a6,-(sp)
		move.l	crb_ExecLib(a6),a5
		exg	a5,a6

	; force data cache flush before agreeing that no more data
	; access will be done by owner

		movem.l	d0/a1,-(sp)
		JSRLIB	CacheClearU	
		movem.l	(sp)+,d0/a1

		moveq	#00,d1

	; disabled so that we don't take a remove card interrupt
	; in the midst of this processing.

		DISABLE

	; check to see if this is the card owner

	PRINTF	DBG_FLOW,<'Current owner node @$%lx'>,crb_Owner(a5)

		cmpa.l	crb_Owner(a5),a1
		bne.s	notowner

	PRINTF	DBG_FLOW,<'Card Owner = $%lx'>,A1

	; mark that this handle is done with this card

		bset	#CHB_NOTIFIED,cah_CardFlags(a1)

	; indicate card is free for use

		move.l	d1,crb_Owner(a5)
		move.l	d1,crb_StatusCallBack(a5)

	; and turn off access light in case it hasn't already
	; been turned off

		LIGHT_OFF
notowner:

	; check to see if this was the owner after card removal

		cmpa.l	crb_WasOwner(a5),a1
		bne.s	wasnotowner

	; handle use to be owner, but a card was removed while the
	; card was owned.  It finally responded that it noticed the
	; interrupt by calling this function.

	PRINTF	DBG_FLOW,<'***ACKNOWLEDGE CARD REMOVED***'>

		move.l	d1,crb_WasOwner(a5)

	;
	; and renable GAYLE interface - having done this, we will
	; take our software interrupt, and decide then if there
	; is, or is not another card in the slot (after debouncing).
	;
	; may result in immediate interrupt following ENABLE

		move.b	#00,gaylestatus

	; and turn off access light in case it hasn't already
	; been turned off

		LIGHT_OFF
wasnotowner:

	; See if they want to remove the handle from the notification list

		btst	#CARDB_REMOVEHANDLE,d0
		beq.s	notifynext

	; yes, remove node from notify list

	PRINTF	DBG_OSCALL,<'Remove($%lx)'>,A1

		JSRLIB	Remove

notifynext:

	; its safe to break DISABLE here

		ENABLE

		bsr.s	NotifyInsert

		movem.l	(sp)+,a5-a6

	PRINTF	DBG_EXIT,<'Exit ReleaseCard()'>

                rts

*****i* card.resource/NotifyInsert ************************************
*
*   NAME
*	NotifyInsert -- notify CardHandle that it owns the resource
*   
*   SYNOPSIS
*	NotifyInsert( void )
*
*	a5 = resource base
*	a6 = exec base
*
*   FUNCTION
*	Private function - set reset on remove if requested, and
*	notify CardHandle that it owns the credit card.
*
*	This function can be called on the context of a task, or
*	interrupt - it DISABLES, and verifies that their is someone
*	to notify (else exits).  The CardHandle notified is notified
*	within DISABLE, so its not possible to take a CardRemoved
*	interrupt until AFTER the CardInsert call back is done.
*
*	Caller of this function preserves/restores A5.
*
***********************************************************************
NotifyInsert:

		movem.l	a5-a6,-(sp)

		DISABLE

	; check if card is available, and inserted

		tst.l	crb_Owner(a5)
		BNE_S	notifydone

	PRINTF	DBG_FLOW,<'Card is available'>

	; reset all gayle registers to known state

		BSR_S	ResetGayleRegs

	; find next on list of those which have not been
	; notified about this card

		lea	crb_NotifyList(a5),a0

	; scan through list finding first which has not been notified
	; about this card

scannodes:
		TSTNODE	A0,A0
		BEQ_S	notifydone
		btst	#CHB_NOTIFIED,cah_CardFlags(a0)
		bne.s	scannodes

	; Give ownership to next on list, and notify

	PRINTF	DBG_FLOW,<'Next CardHandle @ $%lx'>,A0

	; see if they want reset on card removal

		btst	#CARDB_RESETREMOVE,cah_CardFlags(a0)
		beq.s	wakenode

	PRINTF	DBG_FLOW,<'RESET on REMOVE requested'>


	; tell gayle to hardware RESET machine on card-change
	; set DETRESET - may reset immediately if the CC_DET
	; bit is already set - ok, fine, so we reboot - no different
	; then if the user pulls the card right after this instruction

		move.b	#SETRESET,gaylechange

wakenode:


		move.l	a0,crb_Owner(a5)
		move.l	cah_CardStatus(a0),crb_StatusCallBack(a5)

	; FORBID so that we don't lose control here for a long period
	; of time.  Ideally the code called will set a bit, and signal
	; a task to wake up, but because of the order of things, this
	; can be done outside of DISABLE

		FORBID

		ENABLE


	; mark that this handle owns the card in the slot
	; call the handles notification code

		move.l	cah_CardInserted(a0),a0

		movem.l	IS_DATA(a0),a1/a5

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'JSR $%lx($%lx)'>,a5,a1

		jsr	(a5)

		movem.l	(sp)+,a5-a6	;restore execbase too

		PERMIT
		rts
notifydone:
		ENABLE
		movem.l	(sp)+,a5/a6
		rts


*****i* card.resource/ResetGayleRegs **********************************
*
*   NAME
*	ResetGayleRegs -- reset all gayle registers
*   
*   SYNOPSIS
*	ResetGayleRegs( )
*
*   FUNCTION
*	Private function - Used to reset all gayle control
*	registers so that the next device notified owns the card.  
*	in a known state.
*
*   NOTES
*	PRESERVE ALL CPU REGISTERS!
***********************************************************************
ResetGayleRegs:

CLEARREG	SET	IDE_MASK!CC_MASK
ORENA_INTS	SET	GAYLEF_INT_BVD1!GAYLEF_INT_WR!GAYLEF_INT_BSYIRQ

	; Clear all status change ints except IDE, and CC
	; Turn off RESET, and BERR bits too

		move.b	#CLEARREG,gaylechange

	; clean-up any writes to status register (clear all bits
	; including CC_DET, WR, BVD1, BVD2, IRQ, DIG_AUD, CC_DISABLE

		move.b	#00,gaylestatus

	; clean-up any writes to control register (set default memory
	; speed, and programming voltage)

		move.b	#00,gaylecontrol

	; reset interrupt enable/disable to defaults

		bclr	#GAYLEB_INT_BVD2,gayleint	;turn off BVD2
		or.b	#(ORENA_INTS),gayleint		;turn on BVD1, WR, and BSY

	; clear all masks

		clr.l	crb_SpeedMask(a5)

		rts

*****i* card.resource/IfRemoved *************************************
*
*   NAME
*	IfRemoved -- check to see if the card has been removed
*   
*   SYNOPSIS
*	status=IfRemoved(resource,handle)
*	CC		a6	  a1
*
*   FUNCTION
*	Private function - Used to test for card removal.
*	
*   RESULTS
*	TRUE if card has been removed, else FALSE (in CC).
*
*   NOTES
*	PRESERVE ALL CPU REGISTERS!
***********************************************************************
IfRemoved:
		cmpa.l	crb_Owner(a6),a1

	; add additional code here on CD-TV to test for latched
	; changed bit too

		rts

******* card.resource/CardResetRemove *********************************
*
*   NAME
*	CardResetRemove -- Set/Clear reset on card removal.
*   
*   SYNOPSIS
*	success=CardResetRemove( handle, flag );
*				 a1      d0
*
*	BOOL CardResetRemove( struct CardHandle *, ULONG );
*
*   FUNCTION
*	Used to set/clear HARDWARE RESET on card change detect.
*
*	This function should generally not be used by devices
*	which support HOT-REMOVAL.  HARDWARE RESET on removal
*	is generally intended for execute-in-place software, or
*	ram cards whose memory has been added as system ram.
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	flag - TRUE if you want to SET HARDWARE RESET on credit
*	       card removal.  FALSE if you want to CLEAR HARDWARE
*	       RESET.
*
*   RETURNS
*	1  - Success.
*
*	0  - Function failed (most likely because the card was removed
*	     by the user, and you are no longer the owner of the card).
*
*	-1 - This function is not being made available.
*
*   NOTES
*	This function should only be called from a task.
*
*   SEE ALSO
*	OwnCard()
*
***********************************************************************


CLEARREG	SET	IDE_MASK!CC_MASK!BVD_MASK!WR_MASK!IRQ_MASK

CardResetRemove:
	PRINTF	DBG_ENTRY,<'CardResetRemove($%lx,%ld)'>,A1,D0

		move.l	crb_ExecLib(a6),a0

	; clear -- write all 1's except for RESET and BERR bits

		move.b	#CLEARREG,d1

		tst.b	d0
		beq.s	clearremove

	; set -- write all 1's except for BERR bit

		ori.b	#GAYLEF_DETRESET,d1

clearremove:
		moveq	#FALSE,d0

		DISABLE	A0,NOFETCH

	; verify that this handle owns the card

		cmpa.l	crb_Owner(a6),a1
		bne.s	setresetexit

	; Set/clear RESET bit -- can cause an immediate reset if
	; the card has already been removed before the DISABLE.
	;
	; No problem - its no worse than the card being removed
	; a moment after the next instruction.
	;

		move.b	d1,gaylechange

		moveq	#TRUE,d0

setresetexit:
		ENABLE	A0,NOFETCH

	PRINTF	DBG_EXIT,<'%ld=CardResetRemovet()'>,D0

		rts

******* card.resource/CardMiscControl *********************************
*
*   NAME
*	CardMiscControl -- Set/Clear miscellaneous control bits
*   
*   SYNOPSIS
*	control_bits=CardMiscControl( handle, control_bits );
*	d0			 	a1      	d1
*
*	UBYTE CardMiscControl( struct CardHandle *, UBYTE );
*
*   FUNCTION
*	Used to set/clear miscellaneous control bits (generally for
*	use with I/O cards).
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	control_bits - A mask value of control bits to be turned on/off.
*
*	The bit values which might be usable are defined in card.h/i.
*
*	For example, to enable digital audio, and disable hardware
*	write-protect (if supported), you would call this function with these
*	values --
*
*	CARDF_DISABLE_WP|CARDF_ENABLE_DIGAUDIO
*	
*	Then to turn off digital audio, but leave write-protect
*	disable, you would use a value of --
*
*	CARDF_DISABLE_WP
*
*	Finally too reenable write protect, call this function with
*	a mask value of 0.
*
*   RETURNS
*	control_bits - The same mask value you called this function
*	with if successful.  If one, or more bits has been cleared
*	in the return mask, this would indicate that the control bit
*	is not being supported, or that the card has been removed
*	by the user.
*
*	For example, if you called this function with a mask value
*	of --
*
*	CARDF_DISABLE_WP|CARDF_ENABLE_DIGAUDIO
*
*	And this function returned a value of --
*
*	CARDF_DISABLE_WP
*
*	This would indicate that it is not possible to enable digital
*	audio (most likely because this feature has not been implemented).
*	
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*	!!!IMPORTANT!!!
*
*	You should ALWAYS try to enable digital audio for I/O cards
*	as this will also configure the card socket for the I/O
*	interface (if supported).
*
*	Not all cards will connect the write-enable line (e.g.,
*	some I/O cards).  On some machines (e.g., the A600) it will
*	not be possible to write to such cards unless you disable
*	write-protection by using this function.
*
*	!!!NEW!!!
*
*	For card.resource V39 (check resource base for VERSION before
*	using), new bits have been defined which let you enable/disable
*	particular status change interrupts.  See CardInterface() for
*	defaults.  These new bits are backwards compatable with V37
*	for which only the CARDB_DISABLE_WP, and CARDB_ENABLE_DIGAUDIO
*	bits were defined.  These new bits allow you to enable, or
*	disable specific status change interrupts including BVD1/SC,
*	BVD2/DA, and BSY/IRQ.  The defaults for these status change
*	interrupts are unchanged from V37, and WR (Write-protect) status
*	change interrupts are always enabled as they use to be.
*
*	An example of use:
*
*	CARD_INTF_SETCLR!CARD_INTF_BVD1
*
*	Would enable BVD1/SC status change interrupts, and not change
*	the enable/disable state for BVD2/DA or BSY/IRQ status change
*	interrupts.  If the change was made successfully, the
*	CARD_INTB_BVD1 bit would be set in register D0 when this function
*	returns.
*
*   SEE ALSO
*	CardInterface(), resources/card.h, resources/card.i
*
***********************************************************************

MASKCONTROL	SET	WRDET!GAYLEF_STATUS_DIGAUDIO
INTCONTROL	SET	CARD_INTF_SC!CARD_INTF_DA!CARD_INTF_IRQ
ANDCONTROL	SET	GAYLEF_INT_IDEINT!GAYLEF_INT_CCDET!GAYLEF_INT_WR!GAYLEF_LEVEL_BVD!GAYLEF_LEVEL_BSYIRQ

CardMiscControl:
	PRINTF	DBG_ENTRY,<'CardMiscControl($%lx,$%lx)'>,A1,D1

		movem.l	d2-d3,-(sp)

		move.b	d1,d2			;grab enable/disable bits
		and.b	#INTCONTROL,d2
		move.b	d1,d3
		and.b	#CARD_INTF_SETCLR,d3	;and enable/disable setclr

		moveq	#00,d0

	; prevent setting anything other than WR, DIGAUDIO

		and.b	#MASKCONTROL,d1

		move.l	crb_ExecLib(a6),a0

		DISABLE	A0,NOFETCH

		cmpa.l	crb_Owner(a6),a1
		bne.s	setmiscexit

		move.b	d1,gaylestatus		;set bits

		move.b	d1,d0

	; enable/disable interrupts
		tst.b	d2			;see if anything set to change
		beq.s	setmiscexit

		or.b	d2,d0			;set changed

		lea.l	gayleint,a1

		tst.b	d3
		beq.s	clearmiscints

	; set some bits in interrupt control register

		or.b	d2,(a1)			;enable these
		bra.s	setmiscexit

clearmiscints:
	; clear some bits in interrupt control register

		eor.b	#INTCONTROL,d2		;invert optional bits
		or.b	#ANDCONTROL,d2		;and add no change bits
		and.b	d2,(a1)			;clear relevant bits

setmiscexit:
		ENABLE	A0,NOFETCH

		movem.l	(sp)+,d2-d3

	PRINTF	DBG_EXIT,<'$%lx=CardMiscControl()'>,D0

		RTS

******* card.resource/CardAccessSpeed *********************************
*
*   NAME
*	CardAccessSpeed -- Select best possible memory access speed.
*   
*   SYNOPSIS
*	result=CardAccessSpeed( handle, nanoseconds );
*	d0			 a1      d0
*
*	ULONG CardAccessSpeed( struct CardHandle *, ULONG );
*
*   FUNCTION
*	This function is used to set memory access speed for all CPU
*	accesses to card memory.
*
*	Typically this information would be determined by first examining
*	the Card Information Structure.
*
*	Then you would use this function to let the card.resource
*	select the best possible access speed for you, however note
*	that the range of possible access speeds may vary on some
*	machines (depending on the type of credit-card interface
*	hardware being provided).
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	nanoseconds - Preferred access speed in nanoseconds.
*
*   RETURNS
*	Speed - Access speed selected by resource (in nanoseconds). 
*
*	0  - Not successful.  Either because the credit-card was
*	removed, or the access speed you requested is slower than
*	that supported by the credit-card interface hardware.
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*   SEE ALSO
*	OwnCard()
*
***********************************************************************

CardAccessSpeed:

	PRINTF	DBG_ENTRY,<'CardAccessSpeed($%lx,%ld)'>,A1,D0

		move.l	a2,-(sp)

		lea	speeds(pc),a2
		lea	speedmasks(pc),a0
findspeed:
		cmp.l	(a2)+,d0
		bls.s	foundspeed
		addq.l	#1,a0
		tst.l	(a2)
		bne.s	findspeed

		moveq	#FALSE,d0		;too slow

		bra.s	selectspeedexit

foundspeed:
		move.b	(a0),d0

		bsr.s	CardSetSpeed

		tst.l	d0
		beq.s	selectspeedexit

		move.l	-(a2),d0		;return access speed

selectspeedexit:
		move.l	(sp)+,a2

	PRINTF	DBG_EXIT,<'%ld=CardAccessSpeed()'>,D0

		RTS

speeds:
		dc.l	100
		dc.l	150
		dc.l	250
		dc.l	720
		dc.l	0			;impossible

speedmasks:
		dc.b	CARD_SPEED_100NS
		dc.b	CARD_SPEED_150NS
		dc.b	CARD_SPEED_250NS
		dc.b	CARD_SPEED_720NS

		cnop	0,2

*****i* card.resource/CardSetSpeed ************************************
*
*   NAME
*	CardSetSpeed -- Set credit-card memory access speed.
*   
*   SYNOPSIS
*	success=CardSetpeed( handle, speedbits )
*	d0			 a1      d0
*
*	ULONG CardSetSpeed( struct CardHandle *, UBYTE );
*
*   FUNCTION
*	Used to set access speed for all CPU accesses to credit-card
*	memory.
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	speedbits - Privately defined bits.
*
*   RETURNS
*	1  - Successful.
*
*	0  - Not successful.  Most likely because the credit-card
*	card has been removed, and you are no longer the owner.
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*	Preserves d1,a0,a1
***********************************************************************


CardSetSpeed:
		movem.l	d1/a0,-(sp)

	; Writes to gayle control register; value is to be
	; OR'ed with the cached value for programming voltage.

		move.b	d0,d1
		and.b	#(GAYLEF_MEMORY_SPEED1!GAYLEF_MEMORY_SPEED0),d1

		move.l	crb_ExecLib(a6),a0

		moveq	#FALSE,d0

		DISABLE	A0,NOFETCH

		cmpa.l	crb_Owner(a6),a1
		bne.s	setspeedexit

		move.b	d1,crb_SpeedMask(a6)	;cache
		or.b	crb_VoltageMask(a6),d1
		move.b	d1,gaylecontrol		;set

		moveq	#TRUE,d0
setspeedexit:
		ENABLE	A0,NOFETCH

failsetspeed:
		movem.l	(sp)+,d1/a0
		RTS


******* card.resource/CardProgramVoltage ******************************
*
*   NAME
*	CardProgramVoltage -- Set programming voltage.
*   
*   SYNOPSIS
*	success=CardProgramVoltage( handle, voltage );
*					a1      d0
*
*	LONG CardProgramVoltage( struct CardHandle *, ULONG );
*
*   FUNCTION
*	Used to set programming voltages (e.g., for FLASH-ROM/EPROM
*	cards).
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*	voltage - See card.i/h for valid values.
*
*   RETURNS
*	1  - Successful.
*
*	0  - Not successful.  Most likely because the credit-card
*	card has been removed, and you are no longer the owner.
*
*	-1 - This function is not being supported.  On some machines
*	with a minimal (hardware) credit-card interface, this feature
*	may not be possible.
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*       !!!WARNING!!!
*
*	Flash-ROM programming requires careful coding to prevent
*	leaving the Erase command on too long.  Failure to observe
*	the maximum time between the Erase command, and the Erase-Verify
*	command can make a Flash-ROM card unusable.  Some Flash-ROM cards
*	may provide an internal watch-dog timer which protects the card.
*
*	Because of the relatively long time (e.g., 10ms) between Erase, and
*	Erase-Verify which must be observed, the need for such critical
*	timing can be problematic on a multi-tasking machine.
*
*	Vendors of Flash-ROM's recommend a high priority interrupt
*	generated by a 10ms timer be used to turn off Erase.  On the
*	Amiga this can be accomplished by using a CIA-B interval timer.
*	The timer.device also provides a mechanism for generating a low
*	priority interrupt.  The timer.device is easier to use than CIA
*	interval timers, though not as accurate or as safe.
*
*	Even if the Flash-ROM card provides an internal watch-dog timer,
*	implementation of the code during Erase should assume that
*	the Flash-ROM does not.
*
*   SEE ALSO
*	OwnCard(), resources/card.h, resources/card.i
*
***********************************************************************


CardProgramVoltage:
	PRINTF	DBG_ENTRY,<'CardProgramVoltage($%lx,$%lx)'>,A1,D0

	; Writes to gayle control register; value is to be
	; OR'ed with the cached value for mem speed access

		move.b	d0,d1
		and.b	#(GAYLEF_PROGRAM_12V!GAYLEF_PROGRAM_5V),d1

		moveq	#FALSE,d0

		move.l	crb_ExecLib(a6),a0

		DISABLE	A0,NOFETCH

		cmpa.l	crb_Owner(a6),a1
		bne.s	setvoltsexit

		move.b	d1,crb_VoltageMask(a6)	;cache
		or.b	crb_SpeedMask(a6),d1	;or together
		move.b	d1,gaylecontrol		;set

		moveq	#TRUE,d0
setvoltsexit:
		ENABLE	A0,NOFETCH

	PRINTF	DBG_EXIT,<'$%lx=CardProgramVoltage()'>,D0

		RTS

******* card.resource/CardResetCard ***********************************
*
*   NAME
*	CardResetCard -- Reset credit-card.
*   
*   SYNOPSIS
*	success=CardResetCard( handle );
*				a1
*
*	BOOL CardResetCard( struct CardHandle * );
*
*   FUNCTION
*	Used to reset a credit-card.  Some cards, such as some
*	configurable cards can be reset.
*
*	Asserts credit-card reset for at least 10us.
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*   RETURNS
*	TRUE  - Successful.
*
*	FALSE  - Not successful.  Most likely because the credit-card
*	card has been removed, and you are no longer the owner.
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*	***IMPORTANT***
*
*	It is the responsibility of the card owner to reset
*	configurable cards, or any other type of card such as
*	some I/O cards before calling ReleaseCard() if the owner
*	has made use of that card such that it is no longer in its
*	reset state (unless you are releasing the card because it
*	has been removed).
*
*	If the card manufacturer indicates that a certain amount
*	of time must elapse between end of reset, and completion
*	of card initialization, you should wait at least that long
*	before releasing the card (unless you are releasing the card
*	because it has been removed).
*
*   SEE ALSO
*	OwnCard(), ReleaseCard()
*
***********************************************************************

CardResetCard:

	PRINTF	DBG_ENTRY,<'CardResetCard($%lx)'>,A1

		move.l	crb_ExecLib(a6),a0

		moveq	#FALSE,d0

		DISABLE	A0,NOFETCH

		cmpa.l	crb_Owner(a6),a1
		bne.s	cardresetexit

	; set both reset & bberr bits to assert reset

		bsr.s	ApplyReset

		moveq	#TRUE,d0
cardresetexit:

		ENABLE	A0,NOFETCH

	PRINTF	DBG_EXIT,<'$%lx=CardResetCard()'>,D0

		RTS

*
* ApplyReset() for at least 10us
*
ApplyReset:
	PRINTF	DBG_ENTRY,<'ApplyReset'>

		move.l	#gaylehedley,a1

		moveq	#9,d0
		move.b	#00,(a1)	;turn reset on
reset10us:
		tst.b	_ciaa		;approx 1.4us min per read
		dbf	d0,reset10us

		move.b	(a1),d0		;read register - turn off reset
		rts
		
******* card.resource/GetCardMap **************************************
*
*   NAME
*	GetCardMap -- Obtain pointer to CardMemoryMap structure
*   
*   SYNOPSIS
*	pointer=GetCardMap()
*	d0
*
*	struct CardMemoryMap *GetCardMap( void );
*
*   FUNCTION
*	Obtain pointer to a CardMemoryMap structure.  The structure
*	is READ only.
*
*	Devices should never assume credit-card memory appears
*	at any particular place in memory.  By using this function
*	to obtain pointers to the base memory locations of the various
*	credit-card memory types, your device will continue to work
*	properly should credit cards appear in different memory
*	locations in future hardware.
*
*   RETURNS
*	Pointer to CardMemoryMap structure -
*
*		struct CardMemoryMap {
*		UBYTE	*cmm_CommonMemory;
*		UBYTE	*cmm_AttributeMemory;
*		UBYTE	*cmm_IOMemory;
*		};
*
*	As of card.resource V39, this structure has been extended to
*	include the size of these memory regions.  See card.h/card.i
*	for the new fields.  If card.resource V39, use the constants
*	in the CardMemoryMap structure rather than hard coded constants
*	for memory region size.
*
*   NOTES
*	If any pointer in the structure is NULL, it means this type
*	of credit-card memory is not being made available.
*
*   SEE ALSO
*	resources/card.h, resources/card.i
*
***********************************************************************
GetCardMap:

	PRINTF	DBG_ENTRY,<'GetCardMap()'>

		move.l	#MemoryMap,D0

	PRINTF	DBG_EXIT,<'$%lx=GetCardMap'>,D0

		rts

******* card.resource/ReadCardStatus **********************************
*
*   NAME
*	ReadCardStatus -- Read credit card status register
*   
*   SYNOPSIS
*	status=ReadCardStatus()
*	d0
*
*	UBYTE ReadCardStatus( void );
*
*   FUNCTION
*	Returns current state of the credit card status register.
*
*	See card.h/i for bit definitions.
*
*	Note that the meaning of the returned status bits may vary
*	depending on the type of card inserted in the slot, and
*	mode of operation.  Interpretation of the bits is left
*	up to the application.
*
*   RETURNS
*	A UBYTE value to be interpreted as status bits.
*
*   NOTES
*	This function may be called from within a task, or from any level
*	interrupt.
*
*   SEE ALSO
*	resources/card.h, resources/card.i
*
***********************************************************************
ReadCardStatus:

READS_MASK	EQU	(CCDET!BVDET!WRDET!BIDET)

	PRINTF	DBG_ENTRY,<'ReadCardStatus()'>

		moveq	#00,d0
		move.b	gaylestatus,d0
		and.b	#READS_MASK,d0

	PRINTF	DBG_EXIT,<'$%lx=ReadCardStatus()'>,D0

		rts

******* card.resource/BeginCardAccess *********************************
*
*   NAME
*	BeginCardAccess -- Called before you begin credit-card memory access
*   
*   SYNOPSIS
*	result=BeginCardAccess( handle )
*	d0			a1
*
*	BOOL BeginCardAccess( struct CardHandle * );
*
*   FUNCTION
*	This function should be called before you begin access
*	to credit-card memory.
*
*	Its effect will depend on the type of Amiga machine your
*	code happens to be running on.  On some machines it
*	will cause an access light to be turned ON.
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*   RETURNS
*	TRUE if you are still the owner of the credit-card, and
*	memory access is permitted.  FALSE if you are no longer
*	the owner of the credit-card (usually indicating that
*	the card was removed).
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*	It is highly recommended that you call this function
*	before accessing credit-card memory, as well as checking
*	the return value.  If it is a return value of FALSE, you
*	should stop accessing credit-card memory.
*
*   SEE ALSO
*	OwnCard(), EndCardAccess()
*
***********************************************************************
BeginCardAccess:

	PRINTF	DBG_ENTRY,<'BeginCardAccess($%lx)'>,A1

	IFNE SAVEROM

		moveq	#FALSE,d0

		cmpa.l	crb_Owner(a6),a1		;atomic
		bne.s	accessdenied

	; test here incase BeginCardAccess() is called from within DISABLE

		btst	#GAYLEB_CHANGE_CCDET,gaylechange
		bne.s	accessdenied

	; on the A300 with GAYLE, the above test will FAIL if
	; the card was removed while owned.

	; TURN ON LIGHT here - if any!

		LIGHT_ON

		moveq	#TRUE,d0
accessdenied:

	PRINTF	DBG_EXIT,<'$%lx=BeginCardAccess()'>,D0

		rts

	ENDC	;fall through, and save some ROM space

******* card.resource/EndCardAccess ***********************************
*
*   NAME
*	EndCardAccess -- Called at the end of credit-card memory access
*   
*   SYNOPSIS
*	result=EndCardAccess( handle )
*	d0			a1
*
*	ULONG EndCardAccess( struct CardHandle * );
*
*   FUNCTION
*	This function should be called when you are done accessing
*	credit-card memory.
*
*	Its effect will depend on the type of Amiga machine your
*	code happens to be running on.  On some machines it
*	will cause an access light to be turned OFF in approximately
*	1/2 second.
*
*	On machines which support an access light, the light will
*	automatically be turned off when you call ReleaseCard().
*
*   INPUTS
*	handle - Same handle as that used when OwnCard() was called.
*
*   RETURNS
*	TRUE if you are still the owner of the credit-card.  FALSE
*	if you are no longer the owner of the credit-card (usually
*	indicating the card was removed).
*
*   NOTES
*	This function may be called from within a task, or from a level 1
*	or level 2 interrupt.
*
*	It is highly recommended that you call this function
*	after accessing credit-card memory, as well as checking
*	the return value.  If it is a return value of FALSE, you
*	should stop accessing credit-card memory, and conclude
*	that the card was removed before this function was called.
*
*	On some machines it is possible that the credit-card will be
*	removed before you receive the removed interrupt.
*
*   SEE ALSO
*	OwnCard(), ReleaseCard(), BeginCardAccess()
*
***********************************************************************

EndCardAccess:
	PRINTF	DBG_ENTRY,<'EndCardAccess($%lx)'>,A1

		moveq	#FALSE,d0

		cmpa.l	crb_Owner(a6),a1		;atomic
		bne.s	cardwasremoved

	; on the A300 with GAYLE, the above test will FAIL if
	; the card was removed while owned.

	; test here incase EndCardAccess() is called from within DISABLE

		btst	#GAYLEB_CHANGE_CCDET,gaylechange
		bne.s	cardwasremoved

		LIGHT_OFF

		moveq	#TRUE,d0
cardwasremoved:

	PRINTF	DBG_EXIT,<'$%lx=EndCardAccess()'>,D0

		rts

*****i* card.resource/DetectChangeInt *********************************
*
*   NAME
*	DetectChangeInt -- card change interrupt
*   
*   SYNOPSIS
*	void DetectChangeInt(void)
*
*   FUNCTION
*	This interrupt is called when a card is inserted, or
*       removed.  Its LEVEL 6, so no DISABLE is needed.
*
**********************************************************************

CLEARREG	SET	IDE_MASK!BVD_MASK!WR_MASK!IRQ_MASK

DetectChangeInt:

	; was this interrupt for us?

		btst	#GAYLEB_CHANGE_CCDET,gaylechange
		beq.s	notmine			;CC set - do not terminate

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'DETECT CHANGE'>

	; clear CC DET change - clears interrupt

	; write 1's to all bits except for CC_DET, RESET, and BERR
	; assume RESET bit is FALSE since we would have reset if it
	; had been set

		move.b	#CLEARREG,gaylechange

	; fall through 

	; defer interrupt - cause a level 2 (minimize time at level
	; 6 which interferes with serial transfers)

		bset	#CRB_CCDETECT,crb_Flags(a1)

	; cause level 2 interrupt

		move.w	#INTF_SETCLR!INTF_PORTS,_intreq

	; leave with non-zero in CC - terminate chain

notmine:
		rts

******* card.resource/CardForceChange ***********************************
*
*   NAME
*	CardForceChange -- Force a card change.
*   
*   SYNOPSIS
*	success = CardForceChange( VOID )
*	d0
*
*	BOOL CardForceChange( VOID );
*
*   FUNCTION
*	This function is not intended for general use.  Its
*	purpose is to force a credit-card change as if
*	the user had removed, or inserted a card.
*	
*	This function is intended to be used by a utility program
*	which needs to force the current card owner to release
*	ownership of the card, thereby allowing the utility an
*	opportunity to own the credit-card.
*
*   RESULT
*	TRUE if the function succeeded, FALSE if card change is
*	not allowed.  This function will generally succeed, unless
*	someone is using the card in reset remove mode at the time
*	this function is called.
*
*   NOTES
*	This function should only be called from a task.
*
*   SEE ALSO
*
***********************************************************************

CardForceChange:

	PRINTF	DBG_ENTRY,<'CardForceChange()'>

		move.l	crb_ExecLib(a6),a0

		moveq	#FALSE,d0

		DISABLE	A0,NOFETCH

	; do not change if reset on removal is set

		btst	#GAYLEB_DETRESET,gaylechange
		bne.s	cannotchange

	; indicate card change, and cause a level 2 interrupt

		bset	#CRB_CCDETECT,crb_Flags(a6)

		move.w	#INTF_SETCLR!INTF_PORTS,_intreq

		ENABLE	A0,NOFETCH

		moveq	#TRUE,d0
cannotchange:

	PRINTF	DBG_ENTRY,<'%ld=CardForceChange()'>,D0


		rts

******* card.resource/CardChangeCount ***********************************
*
*   NAME
*	CardChangeCount -- Obtain card change count.
*   
*   SYNOPSIS
*	count = CardChangeCount( VOID )
*	d0
*
*	ULONG CardChangeChange( VOID );
*
*   FUNCTION
*	This function returns the card change count.  The
*	counter is incremented by one for every removal, and
*	for every successful insertion (a card which is inserted
*	long enough to be debounced before it is removed again).
*
*   RESULT
*	The change count number.
*
*   NOTES
*	This function may be called from a task, or any level interrupt.
*
*   SEE ALSO
*
***********************************************************************

CardChangeCount:

	PRINTF	DBG_ENTRY,<'CardChangeCount()'>

		move.l	crb_ChangeCount(a6),d0

	; private return - to be used by strap, or me

		lea	crb_MyHandle(a6),a0

	PRINTF	DBG_EXIT,<'%ld=CardChangeCount()'>,D0

		rts

*****i* card.resource/NewCard *****************************************
*
*   NAME
*	NewCard -- Task - deal with notification of newly inserted
*   	cards.
*
*   FUNCTION
*	Check for a new, and debounced card when we are woken
*	up via a timer request.
**********************************************************************

		CNOP	0,4
NewCard:

	; grab resource base

		move.l	4(sp),a5

	PRINTF	DBG_ENTRY,<'Card resource task started $%lx'>,A5

		move.l	crb_ExecLib(a5),a6

	; make sure signal bit is allocated in task structure

		moveq	#CRBB_SIGTASK,d0
		JSRLIB	AllocSignal

		BRA_S	waitagain

	; we woke up because our timer request returned
	; so force a reset of card for at least 10us inside of
	; DISABLE
	
sleepagain:
		bsr	ApplyReset

		ENABLE
isactivity:


	PRINTF	DBG_FLOW,<'Restart timer?'>

	; do not reuse IO request (we will wake-up again)

		bset	#CRB_TIMERUSED,crb_Flags(a5)
		bne.s	waitagain
	;
	; Note that we have to send of at least 2 such timer
	; requests to debounce a newly inserted card.
	;
	; By splitting the debounce time, half the debounce time
	; is used for debouncing, and the other half is for card
	; initialization after reset has been applied.
	;

		move.l	crb_TimerIO(a5),a1
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		clr.l	TV_SECS+IOTV_TIME(a1)
		move.l	#100000,TV_MICRO+IOTV_TIME(a1)

	PRINTF	DBG_OSCALL,<'SendIO($%lx)'>,A1

		JSRLIB	SendIO

	PRINTF	DBG_OSCALL,<'Timer request sent'>


waitagain:

		move.l	#CRBF_SIGTASK,d0

	PRINTF	DBG_OSCALL,<'Wait($%lx)'>,D0

		JSRLIB	Wait

	PRINTF	DBG_OSCALL,<'$%lx=Wait()'>,D0

checkport:
		lea	crb_TimerPort(a5),a0

	PRINTF	DBG_OSCALL,<'Calling GetMsg($%lx)'>,A0

	; Check to see if we woke up because of a timer.device reply

		JSRLIB	GetMsg			; remove reply message

	PRINTF	DBG_OSCALL,<'$%lx=GetMsg()'>,D0

	; if not, assume we were signaled by the card change detect
	; interrupt, and restart the timer request (if it hasn't
	; been started already)


		tst.l	d0
		BEQ_S	isactivity

	; timer request returned

		bclr	#CRB_TIMERUSED,crb_Flags(a5)

	; get ready with list

		lea	crb_NotifyList(a5),a0

		DISABLE

	; check for any new activity within DISABLE - if new
	; activity, restart timer IO

		bclr	#CRB_ACTIVITY,crb_Flags(a5)
		BNE_S	sleepagain

	PRINTF	DBG_FLOW,<'No new activity'>

	; check to see if somehow we got an interrupt, but
	; the previous owner still has not acknowledged the
	; interrupt (should not happen with GAYLE)

		tst.l	crb_WasOwner(a5)
		BNE_S	sleepagain

	; settled down - is there a card?  If we get this test wrong,
	; its ok, since we will take an immediate interrupt after
	; ENABLE again, and crb_Owner() will be set to -1 again

                btst    #GAYLEB_STATUS_CCDET,gaylestatus
		beq.s	endoflist

	PRINTF	DBG_FLOW,<'Newly inserted card'>

	; indicate card is available, and inserted

		clr.l	crb_Owner(a5)

	; increment card change count

		addq.l	#1,crb_ChangeCount(a5)

notnotified:

	; Clears notification flag for all CardHandles on list

		TSTNODE	A0,A0
		beq.s	endoflist
		bclr	#CHB_NOTIFIED,cah_CardFlags(a0)
		bra.s	notnotified


endoflist:

		ENABLE

		bsr	NotifyInsert
		bra	waitagain


*****i* card.resource/StatusChangeInt *********************************
*
*   NAME
*	StatusChangeInt -- status change interrupt
*   
*   SYNOPSIS
*	void StatusChangeInt(void)
*
*   FUNCTION
*	This interrupt is called when there is a status change in
*	BVD1, BVD2, WR enable, or BSY/IRQ.  And we also check
*	for 
*
*   NOTE
*	This server must be at priority 127, else we might miss
*	the interrupt generated by poking the intreq register
*	when we got the level 6 interrupt should some server
*	with a higher priority terminate the server chain at
*	just the right time.
*
**********************************************************************
StatusChangeInt:

	; check for interrupt generated by poking paula

		bclr	#CRB_CCDETECT,crb_Flags(a1)
		BEQ_S	notccdetect

		move.l	a1,a5

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'Detect change'>

	; reset all gayle registers; in particular I don't want
	; programming voltages set for the next card.  AND clears
	; status change interrupts!

		bsr	ResetGayleRegs

	; card detect change - did someone own the card?  If so,
	; tell them to let it go, and we knew that a card was
	; inserted.

		moveq	#-1,d1

		move.l	crb_Owner(a5),d0

		cmp.l	d1,d0		;is already removed?
		BEQ_S	noowner

	; increment card change count

		addq.l	#1,crb_ChangeCount(a5)

		move.l	d1,crb_Owner(a5)	;indicate no card in slot

		tst.l	d0		;is unowned?
		BEQ_S	noowner

		move.l	d0,a1		;pointer to owner

	; Disable the credit-card interface!  Do not allow anymore
	; access to this card, or any other newly inserted card until
	; this owner acknowledges the interrupt by releasing the card.
	;
	; This is done because we will assume their interrupt code
	; is signaling a task, or setting a bit somewhere; since we
	; have no idea how long it will take for their task to wake
	; up and notice the change, we want to prevent anyone from
	; accessing an inserted card until this owner responds by
	; releasing the card in the slot!
	;

		move.b	#GAYLEF_STATUS_CCDISABLE,gaylestatus

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'Removed while owned'>

	; mark that this card WAS the owner when removed
	; so we will know that when they ReleaseCard() it was
	; also an interrupt acknowledge

		move.l	a1,crb_WasOwner(a5)

	; stop sending status changes too - once this happens,
	; we don't want to confuse anyone with a status change
	; int after (e.g., WP enable change)

		clr.l	crb_StatusCallBack(a5)

	; Call interrupt code to notify owner of card removal
	; This pointer can actually be NULL!  Infact I make use
	; of this in the BootCard code - which only accesses
	; the credit-card via resource calls (which return FALSE)
	; if the card is gone

		move.l	cah_CardRemoved(a1),d0
		beq.s	nohandle

		move.l	d0,a0
		movem.l	IS_DATA(a0),a1/a5

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'JSR $%lx($%lx)'>,a5,a1

		jsr	(a5)

nohandle:

		moveq	#00,d0		; do not terminate server chain
		rts

noowner:

	; All we know is that there is card change detect activity
	; The card may have already been in, and is now coming out,
	; or it was out, and is coming in.  Because the card may
	; jiggle, we may get many interrupts through here.
	;

	; set the activity bit - the bit may be set many times
	; until card insertion/removal finally settles down

	PRINTF	(DBG_INTERRUPT+DBG_FLOW),<'Set activity flag'>

		bset	#CRB_ACTIVITY,crb_Flags(a5)

		move.l	crb_ExecLib(a5),a6
		move.l	#CRBF_SIGTASK,d0
		lea	crb_TC(a5),a1

	PRINTF	(DBG_INTERRUPT+DBG_OSCALL),<'Signal($%lx,$%lx)'>,A1,D0

		JSRLIB	Signal

	PRINTF	(DBG_INTERRUPT+DBG_EXIT),<'Exit detect interrupt'>

		moveq	#00,d0		; do not terminate server chain
		rts	


;---------------------------------------------------------------------
;- Handle status change interrupts - if not cc detect change
;-
;- Should we get a cc detect change, we will catch it next time through
;---------------------------------------------------------------------

	; clear CC DET change - clears interrupt


NOTREG		SET	GAYLEF_CHANGE_BVD1!GAYLEF_CHANGE_BVD2!WR_MASK!IRQ_MASK

	; was this interrupt for us?  bail ASAP if not

notccdetect:
		lea	gaylechange,a0

		move.b	(a0),d0
		move.b	d0,d1
		and.b	#NOTREG,d1
		beq.s	nostatchange		;ignore if no changes

		and.b	gayleint,d1		;ignore spurios
		beq.s	clearstatus

	; call only status hook with relevant only.  If relevant + spurious
	; bit, clear spurious on next interrupt (if any)

		and.b	#(~(NOTREG)),d0
		or.b	d1,d0

	PRINTF	(DBG_INTERRUPT+DBG_ENTRY),<'Status Change Interrupt'>

		move.l	crb_StatusCallBack(a1),d1
		beq.s	clearstatus

		movem.l	d1/a0-a1,-(sp)

		move.l	d1,a0
		movem.l	IS_DATA(a0),a1/a5
		jsr	(a5)

		movem.l	(sp)+,d1/a0-a1

		eor.b	#NOTREG,d0
		or.b	#IDE_MASK!CC_MASK,d0
		move.b	d0,(a0)		;clear gayle interrupt

	; post call back with 0 in D0?

		move.l	crb_Owner(a1),a1

		btst	#CARDB_POSTSTATUS,cah_CardFlags(a1)
		beq.s	nostatchange

		moveq	#00,d0			;call with 0 in D0

		move.l	d1,a0
		movem.l	IS_DATA(a0),a1/a5
		jsr	(a5)

		bra.s	endstatchange

clearstatus:

	; clear active - only those that were grabbed above in
	; disable, so if for example, WP was changed above, but
	; not BVD1, and then BVD1 is TRUE after reading the hardware,
	; I still write a 1 to BVD1 so its not cleared this time
	; around.  No touchy IDE INT, CC_DET, RESET, or BERR


		eor.b	#NOTREG,d0
		or.b	#IDE_MASK!CC_MASK,d0	;no touch
		move.b	d0,(a0)			;clear interrupt

endstatchange:
		moveq	#00,d0			;CC is EQU
nostatchange:
		rts

		END
