******************************************************************************
*
*	$Id: nvram.asm,v 40.4 93/04/19 16:25:02 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	nvram.asm,v $
* Revision 40.4  93/04/19  16:25:02  brummer
* Fix to move chip write cycle delay to before checking for
* last block in multiple block write.  This fixes the bug where
* unlocking/locking multiple items on Martin's screen only got
* every other one.
* 
* Revision 40.3  93/03/26  10:15:20  brummer
* Comment out unused functions nvram_read_one and nvram_write_one.
*
* Revision 40.2  93/03/23  13:09:47  brummer
* Add conditional assembly instructions for DISK based version.
*
* Revision 40.1  93/03/01  15:43:00  brummer
* Use short branch instead of long where appropriate
*
* Revision 40.0  93/02/25  12:03:30  brummer
* fix revision to 40.0
*
* Revision 39.5  93/02/25  11:29:43  brummer
* fixed external defs
*
* Revision 39.4  93/02/04  11:12:41  brummer
* Add XREFs for assembly and C type labels
*
* Revision 39.3  93/02/03  14:22:00  brummer
* fix revision number
*
* Revision 1.3  93/02/02  18:10:50  brummer
* Various comment fixes.
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	NOLIST
        INCLUDE "exec/ables.i"
        INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/io.i"
        INCLUDE	"exec/lists.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/memory.i"
        INCLUDE	"exec/nodes.i"
	INCLUDE "exec/strings.i"
        INCLUDE "exec/types.i"
	INCLUDE "hardware/custom.i"
        INCLUDE	"hardware/cia.i"
	INCLUDE "hardware/intbits.i"
	LIST
	INCLUDE "nvram.i"

        XREF	_intena
	XREF	_custom
	XREF	_ciaa

	SECTION	Code


 IFGT	DISKVERSION


 ELSE


*****i* nonvolatile.library/NVRAM/#? ****************************************
*
*  This file contains routines responsible for low level access to the
*  serial access Nonvolatile RAM chip.  The mid level access to Nonvolatile
*  RAM is through the routine in the file nvramtree.asm.
*  The following low level routines are available :
*
;	XDEF	_nvram_write_one
;	XDEF	nvram_write_one
;	XDEF	_nvram_read_one
;	XDEF	nvram_read_one
	XDEF	_nvram_write_mult
	XDEF	nvram_write_mult
	XDEF	_nvram_read_mult
	XDEF	nvram_read_mult
*
*  Note :	This version of code is really only for the CDGS ROM
*		version.  In the GS, there is a pullup on the NVRAM
*		that allows the low level code to detect no ACKs.  This
*		is NOT the case for regular computer system, so that
*		the low level routine wait_for_ack may be fooled into
*		thinking the NVRAM chip is acking us.
*		It may be necessay to modify the code to account for this
*
******************************************************************************




; *****************************************************************************
;
; The following routine will do the low level bit fiddling required to write
; a single byte to the nonvolatile RAM device.
;
;
;	On Entry:	a1 = Offset into nvram
;			d0 = Data to write in LS byte
;
;	On Exit:	Carry Flag = 0, transfer OK
;			Carry Flag = 1, transfer error
;			d0.w = 0000, transfer OK
;			d0.w = FFFF, transfer error
;
;	Regs used:	all
;
;_nvram_write_one:
;nvram_write_one:
;
; Initialize registers :
;
;	lea	nvram_port_map,a5		; a5 gets address of nvram device
;	move.l	d0,d6				; d6 gets data to send
;	ror.l	#8,d6				; d6 gets data to send in MS byte
;
; Send START to device :
;
;	bsr	nvram_start_device		;
;
; Send device address, write command :
;
;	bsr	nvram_xlate_address		; d0 gets formatted dev adrs
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Send nvram address :
;
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Send data byte to device :
;
;	move.l	d6,d0				; d0 gets data to send
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Send STOP to device :
;
;	bsr	nvram_stop_device		;
;
; Non-error return :
;
;18$:	move.w	#0,d0				; d0 gets good indicator
;	CLEAR_CARRY				; set OK flag
;	rts					; return
;
; Error return :
;
;20$:	move.w	#$0FFFF,d0			; d0 get error indicator
;	SET_CARRY				; set ERROR flag
;	rts					; return



; *****************************************************************************
;
; The following routine will do the low level bit fiddling required to read
; a single byte from the nonvolatile RAM.
;
;
;	On Entry:	a1 = Offset into nvram
;
;	On Exit:	d0 = Data read from nvram in bits 16..23
;			Carry Flag = 0, transfer OK
;			Carry Flag = 1, transfer error
;			d0.w = 0000, transfer OK
;			d0.w = FFFF, transfer error
;
;	Regs used:	all
;
;_nvram_read_one:
;nvram_read_one:
;
; Initialize registers :
;
;	lea	nvram_port_map,a5		; a5 gets address of nvram device
;
; Send START to device :
;
;	bsr	nvram_start_device		;
;
; Send device address, dummy write command :
;
;	bsr	nvram_xlate_address		; d0 gets formatted dev adrs
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Send nvram address :
;
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Send START to device :
;
;	bsr	nvram_start_device		;
;
; Send device address, read command :
;
;	bsr	nvram_xlate_address		; d0 gets formatted dev adrs
;	ori.l	#READCMDF_,d0			; d0 gets read command bit set
;	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
;	bsr	nvram_wait_ack			;
;	bcs.s	20$				;
;
; Read data byte from device :
;
;	bsr	nvram_clock_byte_in		; clock data into LS byte of d0
;	bcs.s	20$				;
;
; Send STOP to device :
;
;	bsr	nvram_stop_device		;
;
; Non-error return :
;
;18$:	swap	d0				;
;	move.w	#0,d0				;
;	CLEAR_CARRY				; set OK flag
;	rts					; return
;
; Error return :
;
;20$:	swap	d0				;
;	move.w	#$0FFFF,d0			;
;	SET_CARRY				; set ERROR flag
;	rts					; return



; *****************************************************************************
;
; The following routine will do the low level bit fiddling required to write
; multiple bytes to the nonvolatile RAM.
;
;
;	On Entry:	a0 = Pointer to buffer of write data
;			a1 = Offset into buffer of write data (also nvram offset)
;			d0 = Count of data to write
;
;	On Exit:	Carry Flag = 0, transfer OK
;			Carry Flag = 1, transfer error
;			d0.w = 0000, transfer OK
;			d0.w = FFFF, transfer error
;
;	Regs used:	all
;
_nvram_write_mult:
nvram_write_mult:
;
; Initialize registers :
;
	lea	nvram_port_map,a5		; a5 gets address of nvram device
	move.l	d0,d6				; d6 gets data count
;
; Send START to device :
;
2$:	bsr	nvram_start_device		;
;
; Send device address, write command :
;
	bsr	nvram_xlate_address		; d0 gets formatted dev adrs
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
	bsr	nvram_wait_ack			;
	bcs.s	20$				;
;
; Send nvram address :
;
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Read byte from buffer into MS byte, increment nvram offset :
;
4$:	move.l  0(a0,a1.w),d0			; d0 gets data from buffer
	addq.w	#1,a1				; increment offset
;
; Wait for ACK :
;
	bsr	nvram_wait_ack			;
	bcs.s	20$				;
;
; Send data byte to device :
;
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Determine if nvram address is on page boundry :
;
	move.w	a1,d1				; d1 gets nvram offset
	andi.w	#$0F,d1				; is it on 16 byte boundry ?
	bne.s	8$				; if no, jump to check for more data
;
; Offset is on boundry :
;	- wait for ACK
;	- send STOP
;	- check for more data to write :
;		- If no, jump to exit
;		- If yes, delay for write cycle and jump to continue from top
;
	bsr	nvram_wait_ack			; wait for ACK
	bcs.s	20$				; j if error

	bsr	nvram_stop_device		; send STOP

	DELAY_WR				; delay for device write cycle
	subq	#1,d6				; more data to send ?
	beq.s	18$				; if no, j to end
	bra.s	2$				; j to continue loop
;
; Offset is NOT on boundry :
;	- check for more data to write
;		- If no, wait for ACK, send STOP, and exit
;		- If yes, j to continue with next byte
;
8$:	subq	#1,d6				; more data to send ?
	bne.s	4$				; if yes, j to continue

	bsr	nvram_wait_ack			; wait for ACK
	bcs.s	20$				; j if error

	bsr	nvram_stop_device		; send STOP
;
; Non-error return :
;
18$:	move.l	d6,d0				; return indication of word count
	swap	d0				;
	move.w	#0,d0				;
	CLEAR_CARRY				; set OK flag
	rts					; return
;
; Error return :
;
20$:	move.l	d6,d0				; return indication of word count
	swap	d0				;
	move.w	#$0FFFF,d0			;
	SET_CARRY				; set ERROR flag
	rts					; return



; *****************************************************************************
;
; The following routine will do the low level bit fiddling required to read
; multiple bytes from the nonvolatile RAM.
;
;
;	On Entry:	a0 = Pointer to buffer for read data
;			a1 = Offset into buffer for read data (also nvram offset)
;			d0 = Count of data to read
;
;	On Exit:	Carry Flag = 0, transfer OK
;			Carry Flag = 1, transfer error
;			d0.w = 0000, transfer OK
;			d0.w = FFFF, transfer error
;
;	Regs used:	all
;
_nvram_read_mult:
nvram_read_mult:
;
; Initialize registers :
;
	lea	nvram_port_map,a5		; a5 gets address of nvram device
	move.l	d0,d6				; d6 gets data count
;
; Send START to device :
;
2$:	bsr	nvram_start_device		;
;
; Send device address, dummy write command :
;
	bsr.s	nvram_xlate_address		; d0 gets formatted dev adrs
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
	bsr	nvram_wait_ack			;
	bcs.s	20$				;
;
; Send nvram address :
;
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
	bsr	nvram_wait_ack			;
	bcs.s	20$				;
;
; Send START to device :
;
	bsr.s	nvram_start_device		;
;
; Send device address, read command :
;
	bsr.s	nvram_xlate_address		; d0 gets formatted dev adrs
	ori.l	#READCMDF_,d0			; d0 gets read command bit set
	bsr	nvram_clock_byte_out		; clock MS byte out of d0
;
; Wait for ACK :
;
	bsr	nvram_wait_ack			;
	bcs.s	20$				;
;
; Read data byte from device and write it to buffer :
;
6$:	bsr	nvram_clock_byte_in		; clock data into LS byte of d0
	bcs.s	20$				;
	move.b  d0,0(a0,a1.w)			; d0 gets data from buffer
	addq.w	#1,a1				; increment offset
;
; Determine if nvram address is over page boundry :
;
	move.w	a1,d1				; d1 gets nvram offset
	andi.w	#$0F,d1				; is it on 16 byte boundry ?
	bne.s	8$				; if no, j to continue
;
; Offset is on boundry :
;	- send STOP
;	- check for more data to read
;		- If no, jump to exit
;		- If yes, jump to  continue from top
;
	bsr.s	nvram_stop_device		; send STOP

	subq.l	#1,d6				; more data to read ?
	beq.s	18$				; if no, j to end
	bra.s	2$				; j to continue
;
; Offset is NOT on boundry :
;	- check for more data to read
;		- If no, send STOP and jump to exit
;		- If yes, send ACK and jump to continue with next byte
;
8$:	subq.l	#1,d6				; more data to read ?
	beq.s	16$				; if no, j to end

	bsr	nvram_send_ack			; send ACK
	bra.s	6$				; j to continue

16$:	bsr.s	nvram_stop_device		; send STOP
;
; Non-error return :
;
18$:	move.l	d6,d0				; return indication of word count
	swap	d0				;
	move.w	#0,d0				;
	CLEAR_CARRY				; set OK flag
	rts					; return
;
; Error return :
;
20$:	move.l	d6,d0				; return indication of word count
	swap	d0				;
	move.w	#$0FFFF,d0			;
	SET_CARRY				; set ERROR flag
	rts					;


; *****************************************************************************
;
; The following subroutine will take the nvram offset value (10 bits) and
; translate it into the formatted bit stream necessary for device and
; word addressing in the device.  The bit stream will include signal, device
; address, r/w bit, and word address.
;
; Note: The read/write bit in the bit stream will be cleared (write).
;
;
;	On Entry:	a1 = Offset into nvram
;
;	On Exit:	d0 = Most significant word is formatted bit stream
;
;	Regs used:	d0
;
nvram_xlate_address:

	moveq	#0,d0			; clear return value
	move.w	a1,d0			; d0 LS word gets offset
	swap	d0			; d0 MS word gets offset
	rol.l	#8,d0			; d0 LS 2 bits gets A9 and A8
	lsl.w	#1,d0			; make room for r/w bit
	ori.b	#nvram_signal,d0	; mix in device signal
	ror.l	#8,d0			; return value in MS word

	rts



; *****************************************************************************
;
; The following subroutine will send the nonvolatile device a START signal.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;
;	On Exit:	d5 = State of device signals
;
;	Regs used:	d5
;
nvram_start_device:
;
; Drive clock high and data high :
;
	move.l	#CLOCKF_HIGH!DATAF_HIGH!CLOCKF_OE!DATAF_OE,d5	;
	move.l	d5,(a5)						; clock high and data high
	DELAY_SUSTA						; delay start setup
;
; Transition data high to low while clock is high :
;
	andi.l	#~DATAF_HIGH,d5					;
	move.l	d5,(a5)						; clock high and data low
	DELAY_HDSTA						; delay start hold time

	rts



; *****************************************************************************
;
; The following subroutine will send the nonvolatile device a STOP signal.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;
;	On Exit:	d5 = State of device signals
;
;	Regs used:	d5
;
nvram_stop_device:
;
; Drive data low then clock high :
;
	andi.l	#~DATAF_HIGH,d5				;
	ori.l	#DATAF_OE,d5				;
	move.l	d5,(a5)					;
	DELAY_SUSTO					; delay stop setup
	ori.l	#CLOCKF_HIGH,d5
	move.l	d5,(a5)					;
	DELAY_SUSTO					; delay stop setup
;
; Transition data low to high while clock is high :
;
	ori.l	#DATAF_HIGH,d5				;
	move.l	d5,(a5)					; clock high and data high
	DELAY_BUF					; delay stop hold time
;
; Finish by not driving clock or data :
;
	moveq	#0,d5					;
	move.l	d5,(a5)					; clock OE low and data OE low

	rts



; *****************************************************************************
;
; The following clock a single byte out to the nonvolatile device.  The data to
; be clocked out must be in the high byte of d0.  Data will be clocked out MSB
; first with a full 32 bit logical shift left using d0.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;			d0 = Data to be clocked out in MS byte
;
;	On Exit:	d0 = long word logically shifted left by 8
;
;	Regs used:	d0,d1,d5
;
nvram_clock_byte_out:
;
; Setup registers :
;
	moveq	#7,d1					; d1 gets bit counter
;
; Drive clock low :
;
2$:	andi.l	#~CLOCKF_HIGH,d5			;
	move.l	d5,(a5)					;
	DELAY_CLKLOW					; delay clock low time
;
; Get next data bit and send it to nvram device :
;
	andi.l	#~DATAF_HIGH,d5				; clear data bit
	ori.l	#DATAF_OE,d5				;
	lsl.l	#1,d0					; is next data bit set ?
	bcc.s	4$					; if no, j to skip data high setting
	ori.l	#DATAF_HIGH,d5				; set data bit high in output value
4$:	move.l	d5,(a5)					; clock low and data ??
	DELAY_CLKLOW					; delay clock low time
;
; Drive clock high to signal data is ready :
;
	ori.l	#CLOCKF_HIGH,d5				;
	move.l	d5,(a5)					; clock high and data ??
	DELAY_CLKHIGH					; delay clock high time
	dbra	d1,2$					; loop for all bits
;
; Finish by driving clock low and release data :
;
	andi.l	#~(CLOCKF_HIGH!DATAF_OE),d5		;
	move.l	d5,(a5)					; clock low and data low
	DELAY_CLKLOW					; delay clock low time

	rts



; *****************************************************************************
;
; The following will clock a single byte in from the nonvolatile device.  The data
; will be clocked into the low byte of d0.  Data will be clocked in MSB
; first with a full 32 bit logical shift left using d0.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;
;	On Exit:	d0 = long word logically shifted left by 8
;
;	Regs used:	d0,d1,d2,d5
;
nvram_clock_byte_in:
;
; Setup registers :
;
	moveq	#7,d1				; d1 gets bit counter
	andi.l	#~DATAF_OE,d5			; release data
	move.l	d5,(a5)
;
; Drive clock low to allow data to change :
;
2$:	andi.l	#~CLOCKF_HIGH,d5		; clock low
	move.l	d5,(a5)				;
	DELAY_AA				; delay for data valid
;
; Drive clock high while reading :
;
	ori.l	#CLOCKF_HIGH,d5			; clock high
	move.l	d5,(a5)
	DELAY_CLKHIGH
;
; Read data bit and merge into return byte :
;
	move.l	(a5),d2				; d2 gets nvram port data
10$:	lsl.l	#1,d0				; make room for new bit
	btst	#DATAB_HIGH,d2			; is data bit high ?
	beq.s	12$				; if no, j to skip bit set
	ori.b	#1,d0				; set data bit high
12$:	dbra	d1,2$				; loop for all bits
;
; Finish by driving clock low :
;
	andi.l	#~CLOCKF_HIGH,d5		;
	move.l	d5,(a5)				; clock low and data low
	DELAY_CLKLOW				; delay clock low time

18$:	CLEAR_CARRY				; set OK flag
	rts					; return



; *****************************************************************************
;
; The following subroutine will send the nonvolatile device an ACK signal.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;
;	On Exit:	CF = 0
;
;	Regs used:	d5
;
nvram_send_ack:
;
; Drive data low :
;
	andi.l	#~DATAF_HIGH,d5			; data low
	ori.l	#DATAF_OE,d5			;
	move.l	d5,(a5)				; clock OE low and data low
	DELAY_AA
;
; Drive clock high :
;
	ori.l	#CLOCKF_HIGH,d5			; clock high
	move.l	d5,(a5)				;
	DELAY_CLKHIGH
;
; Finish by driving clock low and release data :
;
	andi.l	#~CLOCKF_HIGH!DATAF_OE,d5	; clock low
	move.l	d5,(a5)				;
	CLEAR_CARRY				; set OK flag
	rts					; return



; *****************************************************************************
;
; The following subroutine will wait for an ACK from the nonvolatile device.
;
;
;	On Entry:	a5 = Memory mapped address of nonvolatile device
;
;	On Exit:	CF = 0, ACK was received successfully
;			CF = 1, time out on ACK
;
;	Regs used:	d2,d5
;
nvram_wait_ack:
;
; Drive Clock high for 9th clock and release data :
;
	ori.l	#CLOCKF_HIGH,d5			; clock high
	andi.l	#~DATAF_OE,d5			; release data
	move.l	d5,(a5)				;
;
; Wait for nvram device to drive data low :
;
2$:	moveq	#nvram_delay_count,d2		; d2 gets wait count
4$:	DELAY_AA				;
	move.l	(a5),d5				; d5 gets nvram port data
	btst	#DATAB_HIGH,d5			; is clock low ?
	beq.s	6$				; if yes, j to continue
	dbra	d2,4$				; loop for wait count
	bra.s	16$				; j to return error
;
; Finish by driving clock low :
;
6$:	andi.l	#~CLOCKF_HIGH,d5		; clock low
	move.l	d5,(a5)				;
	CLEAR_CARRY				; set OK flag
	rts					; return

16$:	andi.l	#~CLOCKF_HIGH,d5		; clock low
	move.l	d5,(a5)				;
	SET_CARRY				; set ERROR flag
	rts					; return


 ENDIF


	END
