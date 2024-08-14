$CASE
$INCLUDE (defs.asm)
;***************************************************************************
;
; Project:	Commodore
; File:		ariutil.asm
; Date:		19 July 1993
; Version:	0.1
; Status:	Draft
;
; Description:	contains the following arithmetic routines:
;		byte bcd_to_hex(byte bcd_in)
;		byte hex_to_bcd(byte hex_in)
;		void bcd_to_hex_time(struct time *t1, struct time *t2)
;		void hex_to_bcd_time(struct time *t1, struct time *t2)
;		byte compare_time(struct time *t1, struct time *t2)
;		void add_time(struct time *t1, struct time *t2, struct time *t3)
;		void subtract_time(struct time *t1, struct time *t2, struct time *t3)
;		void calc_disc_parm(void)
;		byte calc_jump_speed(void)
;		int calc_tracks(struct time *t1, struct time *t2)
;
; Decisions:	-
;
; HISTORY		AUTHOR COMMENTS
; 24 November 1992	creation H.v.K.
;
;***************************************************************************

PUBLIC	_bcd_to_hex, _bcd_to_hex_BYTE, _hex_to_bcd, _hex_to_bcd_BYTE
PUBLIC	_bcd_to_hex_time, _bcd_to_hex_time_BYTE, _store
PUBLIC	_hex_to_bcd_time, _hex_to_bcd_time_BYTE
PUBLIC	_compare_time, _compare_time_BYTE
PUBLIC	_add_time, _add_time_BYTE, _subtract_time, _subtract_time_BYTE
PUBLIC	_calc_tracks, _calc_tracks_BYTE

util		SEGMENT		CODE
util_data	SEGMENT		DATA

RSEG		util_data
_bcd_to_hex_BYTE:
_hex_to_bcd_BYTE:
_bcd_to_hex_time_BYTE:
_hex_to_bcd_time_BYTE:
_compare_time_BYTE:
_add_time_BYTE:
_subtract_time_BYTE:
_calc_tracks_BYTE:	DS	3

_store:			DS	12

RSEG		util

;***************************************************************************
; byte bcd_to_hex(byte bcd_in)
_bcd_to_hex:
	MOV     A,#0FH
	ANL     A,_bcd_to_hex_BYTE
	MOV     R7,A				;don't use another register for storing
	MOV     A,_bcd_to_hex_BYTE
	SWAP    A
	ANL     A,#0FH
	MOV     B,#10
	MUL     AB
	ADD     A,R7
	RET

;***************************************************************************
; byte hex_to_bcd(byte hex_in)
_hex_to_bcd:
	MOV     B,#10
	MOV     A,_hex_to_bcd_BYTE
	DIV     AB
	SWAP    A
	ANL     A,#0F0H
	ORL     A,B
	RET

;***************************************************************************
; void bcd_to_hex_time(struct time *t1, struct time *t2)
; t2 = bcd_to_hex(t1)
_bcd_to_hex_time:
	MOV	R2,#3				;struct time is 3 bytes
	MOV	R0,_bcd_to_hex_time_BYTE        ;R0 points to t1
	MOV	R1,_bcd_to_hex_time_BYTE+1      ;R1 points to t2
next_conv1:
	MOV	_bcd_to_hex_BYTE,@R0
	LCALL	_bcd_to_hex
	MOV	@R1,A
	INC	R0
	INC	R1
	DJNZ	R2,next_conv1
	RET

;***************************************************************************
; void hex_to_bcd_time(struct time *t1, struct time *t2)
; t2 = hex_to_bcd(t1)
_hex_to_bcd_time:
	MOV	R2,#3				;struct time is 3 bytes
	MOV	R0,_hex_to_bcd_time_BYTE        ;R0 points to t1
	MOV	R1,_hex_to_bcd_time_BYTE+1      ;R1 points to t2
next_conv2:
	MOV	_hex_to_bcd_BYTE,@R0
	LCALL	_hex_to_bcd
	MOV	@R1,A
	INC	R0
	INC	R1
	DJNZ	R2,next_conv2
	RET

;***************************************************************************
; byte compare_time(struct time *t1, struct time *t2)
; if t1 > t2 then return = bigger.
; if t1 < t2 then return = smaller.
; if t1 = t2 then return = equal.
_compare_time:
	MOV	R2,#3				;struct time is 3 bytes
	MOV	R0,_compare_time_BYTE        	;R0 points to t1
	MOV	R1,_compare_time_BYTE+1      	;R1 points to t2
next_compare:
	MOV	A,@R0
	CLR	C
	SUBB	A,@R1
	JC	report_smaller
	JZ	next_ptr
	SJMP	report_bigger
next_ptr:
	INC	R0
	INC	R1
	DJNZ	R2,next_compare
	MOV	A,#EQUAL
	RET
report_smaller:
	MOV	A,#SMALLER
	RET
report_bigger:
	MOV	A,#BIGGER
	RET

;***************************************************************************
; void add_time(struct time *t1, struct time *t2, struct time *t3)
; t3 = t1 + t2
_add_time:
	MOV	R0,_add_time_BYTE        	;R0 points to t1
	MOV	R1,_add_time_BYTE+1      	;R1 points to t2
	MOV	A,@R0
	ADD	A,@R1
	MOV	R2,A				;R2 is sum of minutes
	INC	R0
	INC	R1
	MOV	A,@R0
	ADD	A,@R1
	MOV	R3,A				;R3 is sum of seconds
	INC	R0
	INC	R1
	MOV	A,@R0
	ADD	A,@R1
	MOV	R4,A				;R4 is sum of frames
	CJNE	A,#75,chok1
chok1:
	JC	chok2
	SUBB	A,#75				;correct frames
	MOV	R4,A
	INC	R3				;also correct seconds
chok2:
	MOV	A,R3
	CJNE	A,#60,chok3
chok3:
	JC	chok4
	SUBB	A,#60				;correct seconds
	MOV	R3,A
	INC	R2				;also correct minutes
chok4:
	MOV	R0,_add_time_BYTE+2        	;R0 points to t3
	MOV	A,R2
	MOV	@R0,A				;store result.min
	INC	R0
	MOV	A,R3
	MOV	@R0,A				;store result.sec
	INC	R0
	MOV	A,R4
	MOV	@R0,A				;store result.frm
	RET

;***************************************************************************
; void subtract_time(struct time *t1, struct time *t2, struct time *t3)
; t3 = t1 - t2
_subtract_time:
	MOV	R0,_subtract_time_BYTE        	;R0 points to t1
	MOV	A,@R0
	MOV	R2,A				;R2 = t1.min
	INC	R0
	MOV	A,@R0
	MOV	R3,A				;R3 = t1.sec
	INC	R0
	MOV	A,@R0
	MOV	R4,A				;R4 = t1.frm
	MOV	R0,_subtract_time_BYTE+1       	;R0 points to t2
	MOV	A,@R0
	MOV	R5,A				;R5 = t2.min
	INC	R0
	MOV	A,@R0
	MOV	R6,A				;R6 = t2.sec
	INC	R0
	MOV	A,@R0
	MOV	R7,A				;R7 = t2.frm
	MOV	A,R4
	CLR	C
	SUBB	A,R7
	MOV	R4,A				;R4 = result.frm
	JNC	frmsok
	ADDC	A,#74				;correct frames
	MOV	R4,A
	CJNE	R3,#00,adjsec			;check if seconds is 0
	MOV	R3,#59				;correct seconds
	DEC	R2				;also correct minutes
	SJMP	frmsok
adjsec:
	DEC	R3				;only correct seconds
frmsok:
	MOV	A,R3
	CLR	C
	SUBB	A,R6
	MOV	R3,A				;R3 = result.sec
	JNC	secsok
	ADDC	A,#59				;correct seconds
	MOV	R3,A
	DEC	R2				;also correct minutes
secsok:
	MOV	A,R2
	CLR	C
	SUBB	A,R5				;A = result.min
	MOV	R0,_subtract_time_BYTE+2       	;R0 points to t3
	MOV	@R0,A				;store result.min
	INC	R0
	MOV	A,R3
	MOV	@R0,A				;store result.sec
	INC	R0
	MOV	A,R4
	MOV	@R0,A				;store result.frm
	RET


;===========================================================================
; function: multiply
; input: X=R0(msb),R1,R2(lsb), Y=R3(msb),R4,R5(lsb)
; output: R2(msb),R3,R4,R5,R6,R7(lsb)
; R2-R7 = X * Y
; registers used: A,PSW,B,R2,R3,R4,R5,R6,R7,DPH,DPL
multiply:
	MOV	DPH,R3		;save R3 in data pointer high
	MOV	DPL,R4		;save R4 in data pointer low
	PUSH	AR5		;save R5 on stack
	MOV	A,R2
	MOV	B,R5
	MUL	AB

	MOV	R7,A
	MOV	R6,B
;------------------------------- End of multiply : one byte * one byte -----
	MOV	A,R1
	MOV	B,R5
	MUL	AB

	ADD	A,R6
	MOV	R6,A
	CLR	A
	ADDC	A,B
	MOV	R5,A

	MOV	A,DPL
	MOV	B,R2
	MUL	AB

	ADD	A,R6
	MOV	R6,A
	MOV	A,B
	ADDC	A,R5
	MOV	R5,A
	CLR	A
	ADDC	A,#0
	MOV	R4,A

	MOV	A,DPL
	MOV 	B,R1
	MUL	AB

	ADD	A,R5
	MOV	R5,A
	MOV	A,B
	ADDC	A,R4
	MOV	R4,A
;------------------------------- End of multiply : two byte * two byte -----
	MOV	A,R0
	POP	B
	MUL	AB

	ADD	A,R5
	MOV	R5,A
	MOV	A,B
	ADDC	A,R4
	MOV	R4,A
	CLR	A
	ADDC	A,#0
	MOV	R3,A

	MOV	A,DPH
	MOV	B,R2
	MUL	AB

	ADD	A,R5
	MOV	R5,A
	MOV	A,B
	ADDC	A,R4
	MOV	R4,A
	CLR     A
	ADDC	A,R3
	MOV	R3,A

	MOV	A,DPL
	MOV	B,R0
	MUL	AB

	ADD	A,R4
	MOV	R4,A
	MOV	A,B
	ADDC	A,R3
	MOV	R3,A
	CLR     A
	ADDC	A,#0
	MOV	R2,A

	MOV	A,DPH
	MOV	B,R1
	MUL	AB

	ADD	A,R4
	MOV	R4,A
	MOV	A,B
	ADDC	A,R3
	MOV	R3,A
	MOV     A,R2
	ADDC	A,#0
	MOV	R2,A

	MOV	A,DPH
	MOV	B,R0
	MUL	AB

	ADD	A,R3
	MOV	R3,A
	MOV	A,B
	ADDC	A,R2
	MOV	R2,A
;--------------------------- End of multiply : three byte * three byte -----
	RET

;===========================================================================
; function: convert_time
; input: R0 pointer to struct time: (R0)->min, (R0+1)->sec, (R0+2)->frm
; output: R0(msb),R1,R2(lsb)
; R0-R2 = 4500*min + 75*sec + frm
; registers used: A,PSW,B,R0,R1,R2,R3,R4
convert_time:
	MOV	A,@R0
	MOV	R3,A		;R3=t.min
	INC	R0
	MOV	A,@R0
	MOV	R4,A		;R4=t.sec
	INC	R0
	MOV	A,@R0
	MOV	R2,A		;R2=t.frm

	MOV	A,R3
	MOV	B,#60
	MUL	AB		;t.min*60
	ADD	A,R4		;add seconds
	MOV	R4,A		;save seconds low
	MOV	A,B
	ADDC	A,#0
;	MOV	R3,A		;save seconds high
	MOV	B,#75
	MUL	AB		;multiply seconds high
	MOV	R0,B		;set result in R0
	MOV	R1,A            ;set result in R1
	MOV	A,R4
	MOV	B,#75
	MUL	AB		;multiply seconds low
	ADD	A,R2
	MOV	R2,A		;set result in R2
	MOV	A,B
	ADDC	A,R1
	MOV	R1,A		;set result in R1
	MOV	A,R0
	ADDC	A,#0
	MOV	R0,A		;set result in R0
	RET


;***************************************************************************
; int calc_tracks(struct time *t1, struct time *t2)
; return(R6 msb, R7 lsb) = number of tracks from t1 to t2 (two's complement)
;
; flag F0 is used for the jump direction: F0 = 0 -> jump forward
;					  F0 = 1 -> jump reverse
; scaling factor S = 256
;                    sqrt(S*A*Tbiggest + S*B) - sqrt(S*A*Tsmallest + S*B)
; number of tracks = ----------------------------------------------------
;                                           sqrt(S)
_calc_tracks:
	LCALL	_compare_time
	CJNE	A,#EQUAL,no_equal
;
;       addresses match so get out !!!!
;       -  -  -  -  -  -  -  -  -  -  -
	MOV	R6,#0
	MOV	R7,#0			;nr of grooves is 0000
	RET
no_equal:
	CJNE	A,#BIGGER,is_smaller
;
;       actual address > search address
;       -  -  -  -  -  -  -  -  -  -  -
	MOV	R0,_calc_tracks_BYTE	;R0 is pointer to biggest time
	SETB	F0			;set bit jump reverse
	SJMP	calcbiggest
is_smaller:
;
;       actual address < search address
;       -  -  -  -  -  -  -  -  -  -  -
	MOV	R0,_calc_tracks_BYTE+1	;R0 is pointer to biggest time
	CLR	F0			;clear bit jump reverse
calcbiggest:
	LCALL	tracks_calc		;R5(msb),R6,R7(lsb) is #tracks for biggest time
	MOV	R0,_calc_tracks_BYTE+1	;R0 is pointer to smallest time
	JB	F0,calcsmallest
	MOV	R0,_calc_tracks_BYTE    ;R0 is pointer to smallest time
calcsmallest:
	MOV	_calc_tracks_BYTE,R5	;first save #tracks for biggest time
	MOV	_calc_tracks_BYTE+1,R6
	MOV	_calc_tracks_BYTE+2,R7
	LCALL	tracks_calc		;R5(msb),R6,R7(lsb) is #tracks for smallest time

	CLR	C   			;now calc biggest-smallest
	MOV	A,_calc_tracks_BYTE+2
	SUBB	A,R7
	MOV	R7,A			;R7(lsb) difference
	MOV	A,_calc_tracks_BYTE+1
	SUBB	A,R6
	MOV	R6,A
	MOV	A,_calc_tracks_BYTE
	SUBB	A,R5
;	MOV	R5,A			;R5(msb) difference
					;now divide difference by 16 (scaling factor)
;	MOV	A,R5			;ACC=R5m-R5l
	MOV	R0,#AR6			;R0 points to R6
	XCHD	A,@R0			;ACC=R5m-R6l, R6=R6m-R5l
	XCH	A,R6			;ACC=R6m-R5l, R6=R5m-R6l
	SWAP	A			;ACC=R5l-R6m
	XCH	A,R6			;ACC=R5m-R6l, R6=R5l-R6m (R6=result msb)
	INC	R0			;R0 points to R7
	XCHD	A,@R0			;ACC=R5m-R7l, R7=R7m-R6l
	MOV	A,R7			;ACC=R7m-R6l
	SWAP	A			;ACC=R6l-R7m
	MOV	R7,A			;R7=R6l-R7m (R7=result lsb)

	JNB	F0,tracks_exit		;jump to exit if forward jump
;	MOV	A,R7			;add 1 track for reverse jumps
	ADD	A,#1			;lsb + 1
	JNC	twoscompl		;jump if no carry
	INC	R6			;msb + carry
twoscompl:
	CPL	A			;convert reverse jumps #tracks into two's complement value
	ADD	A,#1
	MOV	R7,A
	MOV	A,R6
	CPL	A
	ADDC	A,#0
	MOV	R6,A
tracks_exit:
	RET

;===========================================================================
; function: tracks_calc
; input: R0 pointer to struct time: (R0)->min, (R0+1)->sec, (R0+2)->frm
; output: R5(msb),R6,R7(lsb)
; R5-R7 = number of tracks at location pointed to by R0
; registers used: A,PSW,B,R0,R1,R2,R3,R4,R5,R6,R7,DPH,DPL
tracks_calc:
	LCALL	convert_time		;R0(msb),R1,R2(lsb) is total frames
; set multiply factor is disc parameter A: R3 = 0.parmA(msb)
;					   R4 = parmA(lsb).parmA+1(msb)
;					   R5 = parmA+1(lsb).0
	MOV	R7,AR0			;save R0 in R7
	MOV	A,#0BBH                 ;disc_parm_A ;ACC=parmA(msb).parmA(lsb)
	SWAP	A                       ;ACC=parmA(lsb).parmA(msb)
	MOV	R3,#0                   ;R3=0.0
	MOV	R0,#AR3			;R0 points to R3
	XCHD	A,@R0			;R3=0.parmA(msb), ACC=parmA(lsb).0
	MOV	R4,A			;R4=parmA(lsb).0
	MOV	A,#3DH                  ;disc_parm_A+1	;ACC=parmA+1(msb).parmA+1(lsb)
	SWAP	A			;ACC=parmA+1(lsb).parmA+1(msb)
	INC	R0			;R0 points to R4
	XCHD	A,@R0			;R4=parmA(lsb).parmA+1(msb), ACC=parmA+1(lsb).0
	MOV	R5,A			;R5=parmA+1(lsb).0 -> R3,R4,R5 contain correct parmA values
	MOV	R0,AR7			;restore R0 from R7
	LCALL	multiply		;R2(msb=0),R3,R4,R5,R6,R7(lsb) is output
; now add disc parameter B: R0,R1,R2 = parmB 3 bits shifted to the right (div 8)
	MOV	R0,#67H                 ;disc_parm_B
	MOV	R1,#1FH                 ;disc_parm_B+1
	MOV	R2,#0
	MOV	B,#3			;first shift parmB 3 bits right
shift_3r:
	CLR	C
	MOV	A,R0
	RRC	A
	MOV	R0,A
	MOV	A,R1
	RRC	A
	MOV	R1,A
	MOV	A,R2
	RRC	A
	MOV	R2,A
	DJNZ	B,shift_3r		;now R0(msb),R1,R2(lsb) is real parmB

	MOV	A,R5                    ;now add disc parameter B
	ADD	A,R2
	MOV	R5,A
	MOV	A,R4
	ADDC	A,R1
	MOV	R4,A
	MOV	A,R3
	ADDC	A,R0
	MOV	R3,A                    ;store result for square root calculation
	LCALL	square_root		;R5(msb),R6,R7(lsb) is output
	RET

;===========================================================================
; function: square_root
; input: R3(msb),R4,R5,R6,R7(lsb)
; output: R5(msb),R6,R7(lsb)
; R5-R7 = sqrt(R3-R7)
; registers used: A,PSW,B,R0,R1,R2,R3,R4,R5,R6,R7
square_root:
	PUSH    AR7
	PUSH    AR6
	PUSH	AR5		;save value on stack
	PUSH	AR4
	PUSH	AR3

	CLR     A               ;clear registers R0 .. R5   ( clear RE and KW)
	MOV     R0,A
	MOV     R1,A
	MOV     R2,A
	MOV     R3,A
	MOV	R4,A
	MOV	R5,A
	MOV     B,#20           ;init loop counter at 20 = 5 byte * 8 bit / 2
WORTEL_LOOP:
	JB      B.0,WORTEL5     ;pop value byte if 2 low bits B are 00
	JB      B.1,WORTEL5
	POP     AR6		;get byte in R6 to shift it in R3 .. R5
WORTEL5:
	MOV	A,R6		;Shift R6 two steps to the left
	RLC	A		;and shift 2 msb's in KW (reg 3 .. 5)
	MOV	R6,A
	MOV	A,R5
	RLC	A
	MOV	R5,A
	MOV	A,R4
	RLC	A
	MOV	R4,A
	MOV	A,R3
	RLC	A
	MOV	R3,A

	MOV	A,R6
	RLC	A
	MOV	R6,A
	MOV	A,R5
	RLC	A
	MOV	R5,A
	MOV	A,R4
	RLC	A
	MOV	R4,A
	MOV	A,R3
	RLC	A
	MOV	R3,A

	MOV     A,R2            ;calc.     RE := (RE * 2) + 1
	SETB    C
	RLC     A
	MOV     R2,A
	MOV     A,R1
	RLC     A
	MOV     R1,A
	MOV     A,R0
	RLC     A
	MOV     R0,A

	MOV     A,R3		;compare KW en RE,
	CJNE    A,AR0,WORTEL2	;jump if MSByte KW en RE are not equal
	MOV     A,R4
	CJNE    A,AR1,WORTEL2
	MOV     A,R5
	CJNE    A,AR2,WORTEL2
WORTEL2:
	JC      WORTEL1         ;goto (1) if KW < RE,   go through if KW >= RE
	MOV     A,R5            ;calc.  KW := KW - RE
;	CLR	C		;at this point Carry always 0
	SUBB    A,R2
	MOV     R5,A
	MOV     A,R4
	SUBB    A,R1
	MOV     R4,A
	MOV     A,R3
	SUBB    A,R0
	MOV     R3,A

	INC	R2              ;calc.  RE := RE + 1
	SJMP	WORTEL3
WORTEL1:
	DEC	R2           	;clear the lsb bit of RE  ( RE := RE -1 )
WORTEL3:
	DJNZ	B,WORTEL_LOOP	;check loop counter, continue while B <> zero

	MOV     A,R0            ;calc.  RE := RE / 2
	CLR     C
	RRC     A
	MOV     R5,A            ;msb of result to output register
	MOV     A,R1
	RRC     A
	MOV     R6,A		;msb of result to output register
	MOV     A,R2
	RRC     A
	MOV     R7,A		;lsb of result to output register
	RET


	END
