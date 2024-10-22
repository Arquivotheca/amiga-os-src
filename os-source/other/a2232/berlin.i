**
**	$Id: berlin.i,v 1.2 91/08/10 22:30:40 bryce Exp Locker: bryce $
**
**	Generic device driver: Berlin board-specific defines
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	IFND	LIBRARIES_CONFIGVARS_I
	INCLUDE "libraries/configvars.i"
	ENDC
	IFND	EXEC_SEMAPHORES_I
	INCLUDE "exec/semaphores.i"
	ENDC
	IFND	HARDWARE_INTBITS_I
	INCLUDE "hardware/intbits.i"
	ENDC

;------- Flags --------------------------------------------------------------
HANDSHAKE_KLUDGE	EQU 0	;Kludge Quick I/O handling for Handshake 1.xx


;------- Default serial paramter settings -----------------------------------
SER_DEFAULT_BRKTIME	EQU 250000	;In microseconds


;------- Constants for the 7 port 6502/6551 board ---------------------------
EXPANSION_MANUFACTURER_ID   EQU 514 ;Commodore
OLD_EXPANSION_PRODUCT_ID    EQU 69  ;Greg Berlin's 7 port 6502/6551 board
EXPANSION_PRODUCT_ID	    EQU 70  ;Greg Berlin's new 7 port 6502/6551 board
MAGIC_INTERRUPT_FREQUENCY   EQU 5
MAGIC_INTERRUPT_PRIORITY    EQU -10 ;Priority on level 2 int server chain

CACHE_ROM_VERSION	    EQU 37  ;Version to start worrying about the
				    ; 68030 data cache "bug".


NumACIAs	    EQU 7	    ;Number of ACIAs on each board


;---------- Local macros ----------------------------------------------------

;
;Swizzle a 6502 pointer at \1 into a 68000 pointer in D0.  Use d1 as scratch
;
SWIZZLE 	MACRO	;D0=swapped 16 bit 6502 pointer D1-destroyed
PointerLocked\@:
		move.w	\1,d0
		move.b	d0,d1			;xxyy
		bmi.s	PointerLocked\@
		asl.w	#8,d1			;xxyy -> yy00
		lsr.w	#8,d0			;xxyy -> 00xx
		or.w	d1,d0
		ENDM

;
;Swazzle: the opposite of Sizzle!
;Converts a 68000 pointer in d0:16 to a 6502 pointer in d0:16.
;D1 is scratch.
;
SWAZZLE 	MACRO	;D0=pointer to be swapped   d1-destroyed
		move.l	d0,d1			;xxyy
		asl.w	#8,d1			;xxyy -> yy00
		lsr.w	#8,d0			;xxyy -> 00xx
		or.w	d1,d0
		ENDM


;
;Busy-wait for byte at \1 to be non-negative
;
BUSYWAIT	MACRO	;Busy-wait for type byte at \1 to be non-negative
BusyWait\@:
		tst.b	\1
		bmi.s	BusyWait\@
		ENDM


;
;---- calculate total pending read bytes in 6502's buffer
;   \1-ControlArea
;   \2-Unit
;   D0-true count
;   D1-scratch
;
NUMBYTES	MACRO
		moveq	#0,d0			;high half of true count
		SWIZZLE acia_rcvr_head(\1)
		sub.w	mdu_rcvr_tail(\2),d0    ;head-tail=size
		bpl.s	cr_GotTrueCount\@
		add.w	mdu_rcvr_size(\2),d0
cr_GotTrueCount\@:
		;[D0-true 32 bit count of bytes in buffer (0...BUFSIZE)]
		ENDM


;
;---- calculate total pending write bytes in 6502's buffer
;   \1-ControlArea
;   \2-Unit
;   D0-true count
;   D1-scratch
;
NUMBYTESW	MACRO
		moveq	#0,d0			;high half of true count
		SWIZZLE acia_xmit_tail(\1)
		move.w	d0,d1
		move.w	mdu_xmit_head(\2),d0
		sub.w	d1,d0			;head-tail=size
		bpl.s	cw_GotTrueCount\@
		add.w	mdu_xmit_size(\2),d0
cw_GotTrueCount\@:
		neg.w	d0
		add.w	mdu_xmit_size(\2),d0
		;[D0-true 32 bit count of bytes in buffer (0...BUFSIZE)]
		ENDM

;
;Enable/disable PORTS IRQ (nicer than DISABLE/ENABLE)
;
STOPIRQ 	MACRO
		XREF	_intena
		move.w	#INTF_PORTS,_intena
		ENDM

STARTIRQ	MACRO
		move.w	#INTF_SETCLR!INTF_PORTS,_intena
		ENDM

;
;Special REMOVE that preserves A1
;
REMOVE_A1   MACRO   ;D0-destroyed A0-destroyed A1-node (saved)
	    MOVE.L  LN_SUCC(A1),D0  ;SUCC in D0
	    MOVE.L  LN_PRED(A1),A0  ;PRED in A0
	    MOVE.L  D0,LN_SUCC(A0)
	    EXG.L   D0,A0	    ;SUCC in A0, PRED in D0
	    MOVE.L  D0,LN_PRED(A0)

	    IFNE    DEBUG_DETAIL
	    MOVE.L  #$DEAD0001,0(a1)
	    MOVE.L  #$DEAD0002,4(a1)
	    ENDC

	    ENDM


;------------------------------------------------------------------

global_interrupt_freq	EQU $0e
global_interrupt_stash	EQU $0f     ;copied by driver to _freq each int.
global_service_channel	EQU $10
global_service_misc	EQU $11     ;:NOTE: 6502 bug, non-atomic updates
global_channels 	EQU $12
global_need_tbe_int	EQU $13
global_startupflag	EQU $14
global_acia_start	EQU $20
global_memory_size	EQU $4000   ;16K

global_ack_irq		EQU $4000   ;Ack IRQ from 6502
global_long_stop_6502	EQU $8000   ;Assert hard RESET
global_long_start_6502	EQU $c000   ;Release hard RESET
SUPERMAN		EQU 0


; There is one Control area in the 6502's address space
; for each ACIA.  See the 6502 documentation for the bit
; descriptions.
;
  STRUCTURE   acia_struct,0
    UBYTE   acia_status_channel
    UBYTE   acia_status_acia
    UWORD   acia_rcvr_head
    UWORD   acia_rcvr_tail
    UWORD   acia_xmit_tail  ;6502 owned, non-atomic, top bit locks
    UWORD   acia_xmit_head  ;68000 owned.  atomic
     UBYTE  acia_command_flag
     UBYTE  acia_command
    UBYTE   acia_control
    UBYTE   acia_break_length
     UBYTE  acia_jam_flag
     UBYTE  acia_jam_character
    UBYTE   acia_tbe_flag
    UBYTE   acia_flow_control
     UBYTE  acia_xon_character	    ;must be word aligned
     UBYTE  acia_xoff_character
    UBYTE   acia_high_water
    UBYTE   acia_low_water
    UBYTE   acia_rcvr_buffer_min_page
    UBYTE   acia_rcvr_buffer_max_page
    UBYTE   acia_xmit_buffer_min_page
    UBYTE   acia_xmit_buffer_max_page
    STRUCT  acia_private,6
    LABEL   acia_struct_SIZEOF


;acia_flow_control bits
    BITDEF  FLOW,XIN,0	;Reciever will generate Xoff
    BITDEF  FLOW,7IN,2	;Reciever will generate RTS
    BITDEF  FLOW,XOUT,3 ;Transmitter will respect Xoff
    BITDEF  FLOW,7OUT,5 ;Transmitter will respect CTS


;---------- Aliases ---------------------------------------------------------

CLARK_KENT	EQU SUPERMAN
;
;   cd_IntServer hold a permanently allocated interrupt structure.
;   If LN_TYPE is == 0, then the interrupt structure is not currently
;   added to the system.
;
cd_IntServer	EQU cd_Unused	;Interrupt server (one per board)




;----------------------------------------------------------------------------
; Each unit that this multiple contoller knows about has it's own unit
; structure.  All relavant information is stored in the unit structure.
; The device base does not contain any hardware-specific information.
;
; The "mdu_BoardBase" pointer must remain valid for all active and inactive
; unit structures (the interrupt function depends on this).
;
  STRUCTURE MultiDev_Unit,0
    APTR    mdu_ControlArea	;Address of the 6510<->68000 control area
    APTR    mdu_BoardBase	;Base of the board this unit is on.
				;this is assumed to be on a 64K boundary!
    APTR    mdu_SysBase 	;Copy of location 4 for quick access
    APTR    mdu_ConfigDev	;ConfigDev for this board

    ;------ xmit ------
    STRUCT  mdu_xmit_LIST,MLH_SIZE ;List of pending transmit requests
    UWORD   mdu_xmit_head	;Private image of acia_xmit_head
    UWORD   mdu_xmit_max	;Top of xmit buffer+1	    (static)
    UWORD   mdu_xmit_min	;Bottom of xmit buffer	    (static)
    UWORD   mdu_xmit_size	;size of xmit buffer

    ;------ rcvr ------
    STRUCT  mdu_rcvr_LIST,MLH_SIZE ;List of pending receive requests
    UWORD   mdu_rcvr_tail	;Private image of acia_rcvr_tail
    UWORD   mdu_rcvr_max	;Top of rcvr buffer+1
    UWORD   mdu_rcvr_min	;Bottom of rcvr buffer
    UWORD   mdu_rcvr_size	;size of rcvr buffer

    ULONG   mdu_rcvr_thresh	;point at which we force-drain input chars

    ;------ misc ------
    UWORD   mdu_UseCount	;Count of current opens (-1 for exclusive)
    UBYTE   mdu_ACIANumber	;Which ACIA (logical channel) we talk to
    UBYTE   mdu_Pad0

    ;------ locking ---
    STRUCT  mdu_SS,SS_SIZE
    UWORD   muu_Pad1

    LABEL   mdu_SIZE


*****************************************************************
* serial.i -- external declarations for Serial Port Driver
*****************************************************************
;
;	 BITDEF  IOSER,ABORT,5	   ;	 "    rqst-aborted bit
;	 BITDEF  IOSER,ACTIVE,4    ;	 "    rqst-qued-or-current bit
;
;SerErr_BaudMismatch	 EQU	 2	 ;baud rate not supported by hardware

SerErr_UnitBusy 	EQU	16	;selected unit is already in use

	 BITDEF  IOSER,QUEUED,6    ; IO_FLAGS rqst-queued bit

		;:TODO:  Look at this