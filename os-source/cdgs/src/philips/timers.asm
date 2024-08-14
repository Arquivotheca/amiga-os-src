$CASE
$INCLUDE (defs.asm)
;***************************************************************************
;
; Project:	Commodore
; File:		timers.asm
; Date:		08 July 1993
; Version:	0.1
; Status:	Draft
;
; Description:	this module controls all software timers.
;               every 8 msec a timer interrupt is generated and then all
;               software timers are decremented when they are not already 00.
;               also a routine is added to make some sofware delays.
;		it contains 2 routines to reserve/release some space on stack
;		to put a temporary struct time.
;
; Interface functions: void timer_interrupt(void) -> timer0 interrupt service routine, called every 8 msec
;                      void timer_init(void)
;                      void delay(byte nr_of_delays_500usec)
;
; Decisions:	if the timer tick of 8 msec has to be changed, then the
;		function "calc_jump_speed" in file ariutil.asm has to be
;		dramatically changed also (calculation of jump speed is based
;		on the timer tick of 8 msec)
;
; HISTORY		AUTHOR COMMENTS
; 06 November 1992	creation H.v.K.
; 17 June 1993		remark about 8 msec timer tick added.
; 29 June 1993 		added progression timer.
; 05 July  1993		made tray_timer seperate timer.
; 08 July 1993		timer interrupt service routine changed to decrement
;			all timers of the timers segment.
;
;***************************************************************************
NAME	timer_module

timers		SEGMENT	DATA

timerprocs	SEGMENT	CODE


PUBLIC	_timer_init, _delay, _delay_BYTE
PUBLIC	_servo_timer, _play_timer,_progress_timer
PUBLIC	_module_timer, _commo_timer
PUBLIC	_simulation_timer, _search_timer, _kick_brake_timer

RSEG			timers
_delay_BYTE:		DS	1
_servo_timer:		DS	1	;first timer of segment
_module_timer:		DS	1
_commo_timer:		DS	1
_kick_brake_timer:	DS	1
_progress_timer:	DS	1
_simulation_timer:	DS	1
_search_timer:		DS	1
_play_timer:     	DS	1       ;last timer of segment

;--------------------------------------------------------------------------
; Function:	timer0 interrupt service routine
; Input: 	-
; Output        -
; Abstract: 	every 8 msec all timers which are not 00 are decremented
; Decisions:	-
;--------------------------------------------------------------------------
	CSEG	at (000BH)		;vector address timer0 interrupt
	ljmp	timer_interrupt


;--------------------------------------------------------------------------
; Function:	interrupt 0 service routine
; Input: 	-
; Output        -
; Abstract: 	 interrupt every falling edge scor
; Decisions:	-
;--------------------------------------------------------------------------
	CSEG	at (0003H)		;vector address interrupt 0
	ljmp	int0_interrupt

	RSEG	timerprocs
timer_interrupt:
	push	AR0
	push    PSW
	mov	TH0,#-250               ;re-init timer tick of 8msec
	mov	R0,#_servo_timer	;point to first timer of segment
checktimer:
	cjne	@R0,#0,subtimer		;check if timer=00
nexttimer:
	inc	R0			;point to next timer
	cjne	R0,#_play_timer+1,checktimer

	pop	PSW
	pop	AR0
	reti

subtimer:
	dec	@R0			;timer=timer-1
	sjmp	nexttimer


;--------------------------------------------------------------------------
; Function:	timer_init
; Input: 	-
; Output        -
; Abstract: 	init timer0 for an interrupt rate of 8 msec
; Decisions:	-
;--------------------------------------------------------------------------
_timer_init:
	anl	TMOD,#0F0H		;setup timer0 in mode0
	mov	TH0,#-250               ;init timer tick of 8msec
	clr	TF0
	clr	PT0
	setb	TR0
	setb	ET0
	ret


;--------------------------------------------------------------------------
; Function:    delay
; Input:       _DELAY_BYTE: (1 ..255)
; Output       -
; Abstract:    make delay of _DELAY_BYTE * 500 usec
; Decisions:	-
;--------------------------------------------------------------------------
_delay:
	mov	R0,#250
	djnz	R0,$
; 500 usec delay: test if again 500 usec delay is nessecary
	djnz    _delay_BYTE,_delay
	ret


PUBLIC 	_alloc_struct_time, _release_struct_time

alloc_code	SEGMENT		CODE

RSEG		alloc_code

;--------------------------------------------------------------------------
; Function:    _alloc_struct_time
; Input:       -
; Return:      acc: pointer to first allocated byte
; Output       -
; Abstract:    this function allocates three bytes to contain an absolute time frame
; Decisions:   -
;--------------------------------------------------------------------------
_alloc_struct_time:

	pop	acc			; acc return address high
	pop	b			; b   return address low

	push	b
	push	b
	push	b			; create 3 bytes on stack

	push	b
	push	acc			; put return address back on stack
	mov	a,sp                    ; sp - 4 points to first available byte
	add	a,#0FCH

	ret

;--------------------------------------------------------------------------
; Function:    _release_struct_time
; Input:       -
; Return:      -
; Output       -
; Abstract:    this function releases the formaly allocated bytes
; Decisions:   -
;--------------------------------------------------------------------------
_release_struct_time:
	pop	acc                     ; return address high
	pop	b			; return address low

	pop	ar0
	pop	ar0
	pop	ar0			; release 3 bytes

	push	b
	push	acc			; restore return address

	ret


intcode		SEGMENT 	CODE	
intbit		SEGMENT         BIT

RSEG		intbit
_scor_edge:	DBIT		1

RSEG	intcode

	EXTRN DATA (_scor_counter)
	PUBLIC	_scor_edge

int0_interrupt:
	push	ACC
	push	PSW
	setb	_scor_edge			;indicate edge detedted to subcode module
	mov	A,_scor_counter
	jz	leave_interrupt
	dec	_scor_counter

leave_interrupt:
	pop	PSW
	pop	ACC
	reti

	END
