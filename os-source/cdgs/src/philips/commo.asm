$CASE
$debug

$title ('Commo Interface')


	NAME            COMMO_TXD_RXD






;*
;*      Copyright (c) 1992, Philips Consumer Electronics KMG Laser Optics ORD
;*
;* File name    : commo.asm
;*
;* Description  : This module is responsible for receiveing and transmitting
;*              : of single commo commands (max 16).
;*              :
;*              :
;*              :
;*              :
;*
;* Decisions    :
;*              :
;*              :
;*
;* History      : Created by: J.J.M.M. Geelen  date: 01 - 03 - 1993
;*              :
;*              :
;*
;* Version      : 0.2
;*
;* Status       : Creation phase
;*              :
;*              : 21 Sept '93
;*		: module spend time changed into 32 msec.
;*		:
;*
;* Action list  :
;*              :
;*              :
;*
;*
;*


				CODE_COMMO_TXD_RXD                  SEGMENT CODE

				VAR_COMMO_TXD_RXD                   SEGMENT DATA

				BIT_COMMO_TXD_RXD                   SEGMENT BIT



				EXTRN DATA (_commo_timer, _module_timer)

				PUBLIC  _COMMO_INIT, _COMMO_INTERFACE, _NEW_CMD_RECEIVED

				PUBLIC  _GET_BUFFER, _SEND_STRING, _SEND_STRING_BYTE

				PUBLIC  _SEND_STRING_READY, _FREE_CMD_BUFFER, _GET_BUFFER_BYTE

				PUBLIC  _command_length_table

; constant definitions

MODULE_SPEND_TIME               EQU             32/8        ; 32 msec then leave module
COMMO_TXD_TIME_OUT              EQU             500/8           ; transmit time-out = 500 ms
COMMO_RXD_TIME_OUT              EQU             500/8           ; receive time-out = 500 ms
COMMO_ERR_TIME_OUT		EQU		1000/8		; error recovery time-out = 1000 ms
COMMO_ZERO_LENGTH		EQU		128		; number of zero bytes to send

;******************************************************************************
;* Operation return values                                                    *
;******************************************************************************

COMMO_FALSE                     EQU             00h             ; false value
COMMO_TRUE                      EQU             01h             ; true value
COMMO_ERROR                     EQU             02h             ; error value

COMMO_READY_WITHOUT_ERROR       EQU             00h             ; commo ready without error value
COMMO_READY_WITH_ERROR          EQU             01h             ; commo ready with error value
COMMO_BUSY                      EQU             03h             ; commo busy value
COMMO_PENDING                   EQU             04h             ; commo pending value

COMMO_NO_COMMAND                EQU	        00h		; no command received
COMMO_NEW_COMMAND	        EQU		01h				; new command received
COMMO_SAME_COMMAND              EQU		02h				; same command received
COMMO_CMD_ERROR                 EQU		03h				; command received with error

; Commo table positions
;
COMMO_IDLE_POS                  EQU  	       0       ; idle sate

; receiver calls for command buffer

COMMO_RXD_OPCODE_POS                EQU         1       ; receive (opcode) mode waiting for DATA low
COMMO_RXD_PARM_POS                  EQU         2       ; receive (parameter) mode waiting for DATA low
COMMO_RXD_CHECKSUM_POS              EQU         3       ; receive (checksum) mode waiting for DATA low

; transmissions calls

COMMO_TXD_DATA_POS                  EQU         4       ; transmit mode (opcode & data) waiting for DATA high
COMMO_TXD_CHECKSUM_POS              EQU         5       ; transmit mode (checksum) waiting for DATA high

; error calls (uot of synchronisation)

COMMO_ERR_SEND_POS		    EQU         6				; transmit 128 zeroes
COMMO_ERR_RECV_POS		    EQU         7				; receive data till non zero data-byte (= opcode); Commo port definitions NOT DEFINED YET

COMMO_PORT_LINES                    EQU         0E0h    ; commo port mask
COMMO_BUS_POLLING                   EQU         0E0h    ; commo bus polling condition
COMMO_BUS_RECEIVE		    EQU	        0A0h    ; (clk = 1, data = 0, dir = 1) commo bus receive

commo_port                          EQU         p3      ; port 3

commo_dir                           BIT         0B5h    ; (P3.5) commo direction line
commo_data                          BIT         0B6h    ; (P3.6) commo data line
commo_clk                           BIT         0B7h    ; (P3.7 )commo clock line


; transmission status (commo_status bits 1,0)

TXD_STATUS_POS			    EQU		0	 ; times to shift
TXD_STATUS_MASK			    EQU		03h	 ; txd status mask
TXD_STATUS_MASK_CLEAR		    EQU		0FCh	 ; txd clear mask
TXD_STATUS_MASK_RDY_NERR	    EQU		00h	 ; txd status = ready without error
TXD_STATUS_MASK_RDY_ERR             EQU         01h	 ; txd status = ready with error
TXD_STATUS_MASK_PENDING             EQU         02h	 ; txd status = pending
TXD_STATUS_MASK_BUSY                EQU         03h	 ; txd status = busy
					      
; command buffer status also called rxd-status (commo_status bits 3,2)

RXD_STATUS_MASK_POS		    EQU		2	; times to shift
RXD_STATUS_MASK			    EQU		0Ch	; rxd status mask
RXD_STATUS_MASK_CLEAR		    EQU		0F3h	; rxd clear mask
RXD_STATUS_MASK_NO_CMD              EQU		00h	; rxd status = no command
RXD_STATUS_MASK_NW_CMD              EQU		04h	; rxd status = new command
RXD_STATUS_MASK_SM_CMD              EQU		08h	; rxd status = same command
RXD_STATUS_MASK_ER_CMD              EQU		0Ch	; rxd status = command received with error

; commo interface operation status (commo_status bits 5,4)

OP_STATUS_POS	    		   EQU  	4		; shift four times or swap
OP_STATUS_MASK			   EQU		30H		; operation mask
OP_STATUS_MASK_CLEAR		   EQU          0CFH    	; operation clear mask
OP_STATUS_MASK_NOP                 EQU		00H		; operation status = no operation
OP_STATUS_MASK_RXD                 EQU		10H		; operation status = receive command
OP_STATUS_MASK_TXD                 EQU          20H             ; operation status = transmit data
OP_STATUS_MASK_ERR                 EQU          30H             ; operation status = out of synchronisation


COMMO_OPCODE_MASK		   EQU		0FH		; command opcode mask
COMMO_INDEX_MASK		   EQU		0F0H    	; command index mask

; Commo interface command opcode values

ABORT_COMMAND			   EQU  	9		; abort command


; buffer sizes

COMMAND_BUFFER_SIZE		   EQU		12		; command buffer size


RSEG    VAR_COMMO_TXD_RXD

_GET_BUFFER_BYTE:
position:
_SEND_STRING_BYTE:
mode:                              DS 		1  
pointer:                           DS 		1	                      
count:                             DS 		1                            


commo_state:                       DS 		1               ; comma-interface state register for stae-machine
cmd_length:                        DS 		1               ; command length register, used receiveing commands
checksum:			   DS 		1				; checksum register
byte_counter:                      DS 		1               ; count register, number of bytes the transfer (checksum excluded)

; by RXD byte_counter will incremented till equal than 'command_length'
; byte_counter is also used as pointer in command buffer
; by TXD byte_counter is the number of bytes to transmit (will decremented)
;
saved_byte_count:    		   DS 		1		; transmit saved byte counter

byte_pointer:                      DS 		1               ; pointer register, pointer to byte to transmit
commo_status:                  	   DS 		1               
; bit 1,0 TXD-status
; bit 3,2 RXD-status
; bit 5,4 OPERATION-status
; bit 7,6 reserved
last_command:                      DS 		1               ; save register of previuos receiced command (index,opcode)

command_buffer:                    DS 		12              ; command buffer for 1 opcode and 11 parameters


RSEG    BIT_COMMO_TXD_RXD

commo_cmd_received:                DBIT    	1               ; commo command received
commo_txd_req:                     DBIT    	1               ; commo transmit request
commo_checksum_txd_req:            DBIT    	1               ; commo commo transmit request
commo_report_cmd_sts:		   DBIT	   	1	        ; 1 -> report new cmd received

RSEG    CODE_COMMO_TXD_RXD

_command_length_table:

; length = opcode + # parameters   (excluding checksum)

		DB 1    ; X0 RESEND
		DB 2    ; X1 STOP
		DB 1    ; X2 PAUSE
		DB 1    ; X3 PLAY
		DB 12   ; X4 SETPLAY
		DB 2    ; X5 ACTIVITY AND LED CONTROL
		DB 1    ; X6 SENDQ
		DB 1    ; X7 SENDID
		DB 4    ; X8 SEEK
		DB 1    ; X9 ABORT
		DB 1    ; XA RESERVED
		DB 1    ; XB RESERVED
		DB 1    ; XC ENTER DIAGNOSTICS
		DB 2    ; XD DIAGNOSTICS
		DB 1    ; XE RESERVED
		DB 1    ; XF RESERVED

; end _command_length_table


;*
;* Function name: COMMO_STATE_MACHINE
;*              :
;*              :
;*
;* Abstract     : This function calls a function in the commo-state-table
;*              :
;*              :
;*
;* Input        : ACCU = absolute table position
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       : ACCU = next absolute state position
;*              :
;*              :
;*
;* Pre          :
;*              :
;*              :
;*
;* Post         :
;*              :
;*              :
;*/


COMMO_STATE_MACHINE:

; { acc.7 = acc.6 = 0 & acc.5 - acc.0 = absolute position in table }

	rl	a                               ; multiply by two
	rl      a                               ; multiply by two (=total four)
	mov     dptr,#COMMO_STATE_TABLE         ; get table address
	jmp     @A + dptr                       ; jump into table


; END COMMO_STATE_MACHINE


COMMO_STATE_TABLE:

; receiver calls

	ljmp    COMMO_IDLE                  ; 0     idle sate
	nop

	ljmp    COMMO_RXD_OPCODE            ; 1     receive (opcode) mode waiting for DATA low
	nop

	ljmp    COMMO_RXD_PARM              ; 2     receive (parameter) mode waiting for DATA low
	nop

	ljmp    COMMO_RXD_CHECKSUM          ; 3     receive (checksum) mode waiting for DATA low
	nop


; transmissions calls

	ljmp    COMMO_TXD_DATA              ; 4     transmit mode (opcode & data) waiting for DATA high
	nop
	ljmp    COMMO_TXD_CHECKSUM          ; 5     transmit mode (checksum) waiting for DATA high
	nop

; error calls (out of synchronisation)

	ljmp    COMMO_ERR_SEND          	; 6		tranmit zeroes
	nop

	ljmp	COMMO_ERR_RECV			; 7		receive till non zero data-byte received
	nop

; end table


;*
;* Function name: COMMO_INTERFACE
;*              :
;*              :
;*
;* Abstract     : This function controls the commo interface bus and
;*              : should be called repeatly in the main loop
;*              :
;*
;* Input        :
;*              :
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       :
;*              :
;*              :
;*
;* Pre          :
;*              :
;*              :
;*
;* Post         :
;*              :
;*              :
;*/

_COMMO_INTERFACE:

	mov     _module_timer,#MODULE_SPEND_TIME	; set module timer

COMMO_INTERFACE_LOOP:
	mov	a,commo_state				; get commo state
	lcall   COMMO_STATE_MACHINE                     ; call state machine
	mov	commo_state,a                           ; save commo state
;
;  { acc = commo-state }
;
	jz      COMMO_INTERFACE_EXIT                    ; if commo-state = idle then exit
;
; { commo-state <> idle }
;
	clr     a                                       ; clear acc
	cjne    a,_module_timer,COMMO_INTERFACE_LOOP    ; if a <> timer_module then jump
;
; { commo-state <> idle & moduler_timer = 0 }
;
; Check for time-out
;
; debug 	don't check on time-outs 
	 sjmp	 COMMO_INTERFACE_EXIT			; 

	 
	 cjne    a,_commo_timer,COMMO_INTERFACE_EXIT    ; jump to exit if no time-out
;
;  { commo-state <> idle & moduler_time = 0 & _commo_timer = 0 => TIME-OUT }
;
; Handle here all COMMO time-out's
;

	mov	a,commo_status				; get commo status
	anl	a,#OP_STATUS_MASK			; strip operation status
	cjne	a,#OP_STATUS_MASK_RXD,COMMO_INTERFACE_NRXD
;
; { operation status = rxd }
;
	setb	commo_report_cmd_sts			; report new command

	anl	commo_status,#RXD_STATUS_MASK_CLEAR	; strip rxd status
	orl     commo_status,#RXD_STATUS_MASK_ER_CMD    ; rxd-status = error
	sjmp	COMMO_INTERFACE_TMOUT


COMMO_INTERFACE_NRXD:
;
; { operation status <> rxd }
;
	mov     a,commo_status				; get commo status
	anl	a,#OP_STATUS_MASK			; strip operation status
	cjne	a,#OP_STATUS_MASK_TXD,COMMO_INTERFACE_NTXD
;
; { operation status = txd }
;
	anl	commo_status,#TXD_STATUS_MASK_CLEAR	
	orl	commo_status,#TXD_STATUS_MASK_RDY_ERR	; txd-status = error
	clr	commo_txd_req				; transmit request = 0
	clr	commo_checksum_txd_req                  ; transmit checksum request = 0

COMMO_INTERFACE_NTXD:
COMMO_INTERFACE_TMOUT:
	setb    commo_data
	setb    commo_dir
	setb    commo_clk
	mov	checksum,#00				; clear checksum
	mov     commo_state,#COMMO_IDLE_POS		; enter idle state

COMMO_INTERFACE_EXIT:
;
; { commo-state = idle  or  module-timer = 0 or commo-timer = 0 }
;
; NOTE: the idle state has a higher priority then time-outs
;
	ret                                             ; return

;*****************************************************************************
; commo state machine function calls                                           *
;*****************************************************************************
;
;
;*****************************************************************************
; commo state: idle                                                            *
;*****************************************************************************

COMMO_IDLE:

; Pre: all commo interface lines set to high

	mov	a,commo_status					; get comm status
	anl	a,#OP_STATUS_MASK				; strip TXD status
	cjne	a,#OP_STATUS_MASK_TXD,COMMO_IDLE_RECEIVE	; jump if operation <> TXD
;
; { operation = TXD => check for transmission }
;
	sjmp	COMMO_IDLE_TXD_DATA				; in transmit mode

COMMO_IDLE_RECEIVE:
;
; { operation <> TXD => check for receiving data }
;
	mov	a,commo_port					; read all commo interface lines
	anl	a,#COMMO_PORT_LINES				; strip commo interface lines
	cjne	a,#COMMO_BUS_RECEIVE,COMMO_IDLE_TXD_DATA	; jump if (dir <> 1 or data <> 0 or clk <> 1)
;
; { operation <> TXD and (dir = 1 and data = 0 and clk = 1) => commo bus in receive mode }
;
	mov	a,commo_status				        ; get commo statatus
	anl	a,#RXD_STATUS_MASK			        ; strip cmd-buffer status
	cjne	a,#RXD_STATUS_MASK_NO_CMD,COMMO_IDLE_RXD_NFREE   

; { a = RXD_STATUS_MASK_NO_CMD => command buffer free }
;
	anl	commo_status,#OP_STATUS_MASK_CLEAR	        ; clear op. status
	orl	commo_status,#OP_STATUS_MASK_RXD	        ; op. status = RXD

	mov	_commo_timer,#COMMO_RXD_TIME_OUT	        ; set receive time-out
	mov	byte_counter,#00			        ; reset byte-counter
	mov	checksum,#00				        ; clear checksum
	mov	a,#COMMO_RXD_OPCODE_POS			        ; get next state
	ret


COMMO_IDLE_RXD_NFREE:
;
; { command buffer not free }
;
COMMO_IDLE_TXD_DATA:
;
; { op. status <> RXD or { command buffer not free }  =>  op. status = NOP or TXD
;
	jnb	commo_txd_req,COMMO_IDLE_TXD_CHKSUM
;
; { commo_txd_req = 1 => data transmit request }
;
	mov	byte_counter,saved_byte_count		        ; get transnit byte counter

	clr	commo_dir					; clear dir line (transmit data)
	anl	commo_status,#OP_STATUS_MASK_CLEAR		; clear op. status
	orl	commo_status,#OP_STATUS_MASK_TXD		; op. status = TXD

	anl     commo_status,#TXD_STATUS_MASK_CLEAR	       ; clear op. status
	orl	commo_status,#TXD_STATUS_MASK_BUSY	       ; op. status = TXD

	mov	_commo_timer,#COMMO_TXD_TIME_OUT	       ; set transmit time-out

	mov	a,#COMMO_TXD_DATA_POS			       ; get next state
	ret


COMMO_IDLE_TXD_CHKSUM:
;
; { commo_txd_req = 0 => no data transmit request }
;
	jnb	commo_checksum_txd_req,COMMO_IDLE_EXIT
;
; { commo_checksum_txd_req = 1 => ONLY checksum transmit request }
;
	clr	commo_dir				       ; clear dir line (transmit data)
	anl	commo_status,#OP_STATUS_MASK_CLEAR	       ; clear op. status
	orl	commo_status,#OP_STATUS_MASK_TXD	       ; op. status = TXD

	anl	commo_status,#TXD_STATUS_MASK_CLEAR	       ; clear op. status
	orl	commo_status,#TXD_STATUS_MASK_BUSY	       ; op. status = TXD

	mov	_commo_timer,#COMMO_TXD_TIME_OUT	       ; set transmit time-out


	mov	a,#COMMO_TXD_CHECKSUM_POS		       ; get next state

	ret

COMMO_IDLE_EXIT:
	mov     a,#COMMO_IDLE_POS                              ; next state is idle
	ret                                                    ; return





;*****************************************************************************
; commo state: receive (opcode) mode waiting for DATA low
;*****************************************************************************

COMMO_RXD_OPCODE:
;
; Pre condition: dir = 1, clk = 1, data = 0
;
	lcall	GET_RXD_DATA				       ; get byte from commo interface
;
; { acc = data-byte read, dir = 0, data = x, clk = 1 }
;
	jnz	COMMO_RXD_OPCODE_NON_ZERO		       ; jump if received data-byte <> 0
;
; { data-byte = 0 => error }
;
	mov	byte_counter,#COMMO_ZERO_LENGTH	               ; get number of zero bytes to send

	mov	_commo_timer,#COMMO_ERR_TIME_OUT	       ; set transmit time-out

	mov	a,#COMMO_ERR_SEND_POS			       ; get next state
	ret


COMMO_RXD_OPCODE_NON_ZERO:
;
; { data-byte (opcode) <> 0 => no error }
;       
	mov	b,a					       ; save data-byte in b-reg.
	mov	checksum,a				       ; checksum = index/opcode

; store data byte in command buffer

	mov	a,byte_counter				       ; a = byte_counter
	add	a,#command_buffer			       ; a = &(command_buffer[byte_counter])
	mov	r0,a					       ; r0 = a
	mov	@r0,b					       ; store byte in command_buffer

	inc 	byte_counter				       ; increment byte counter
							       ; calculate command length
	mov	a,b					       ; get saved index/opcode
	anl	a,#COMMO_OPCODE_MASK			       ; get opcode only
	mov	dptr,#_command_length_table		       ; dptr = _command_length_table
	movc	a,@a + dptr				       ; a = command length
	mov	cmd_length,a				       ; cmd_length = a = command length
	setb	commo_dir				       ; set dir-line high
	cjne	a,byte_counter,COMMO_RXD_OPCODE_PARM
;                                                              ; { a = byte_counter = (1) => no parameter command }
;
	mov	a,#COMMO_RXD_CHECKSUM_POS	       	       ; get new state
	ret


COMMO_RXD_OPCODE_PARM:
;
; { a <> byte_counter => one or more parameter command }
;
	mov	a,#COMMO_RXD_PARM_POS		       	       ; get next state
	ret


;*****************************************************************************
; commo state: receive (parameter) mode waiting for DATA low
;*****************************************************************************

COMMO_RXD_PARM:
;
; dir = 1, clk = 1, data = ?
;
	jb	commo_data,COMMO_RXD_PARM_EXIT
;
; { commo_data = 0 => receive parameter }
;
	lcall	GET_RXD_DATA			       ; get byte from commo interface
;
; { acc = data-byte read, dir = 0, data = x, clk = 1 }
;
	mov	b,a				       ; save data-byte in b-reg.

	mov	a,byte_counter			       ; a = byte_counter
	add	a,#command_buffer		       ; a = &(command_buffer[byte_counter])
	mov	r0,a				       ; r0 = a
	mov	@r0,b				       ; store byte in command_buffer

	mov	a,b                                    ; a = saved data-byte
	add	a,checksum			       ; a = a + checksum
	mov	checksum,a			       ; checksum = checksum + data-byte

	inc 	byte_counter			       ; increment byte counter

	setb	commo_dir			       ; set dir-line high

	mov	a,byte_counter			       ; a = byte_counter
	cjne	a,cmd_length,COMMO_RXD_PARM	       ; if more byte to receive jump
;															  to beginning of this state-handler
; { byte_counter = cmd_length => receive checksum }
;
	mov	a,#COMMO_RXD_CHECKSUM_POS	      ; get next state
	ret  

COMMO_RXD_PARM_EXIT:

	mov     a,#COMMO_RXD_PARM_POS                 ; stay in current state
	ret                                           ; return



;*****************************************************************************
; commo state: receive (checksum) mode waiting for DATA low
;*****************************************************************************

COMMO_RXD_CHECKSUM:
;
; dir = 1, clk = 1, data = ?
;
	jb	commo_data,COMMO_RXD_CHECKSUM_EXIT
;
; { commo_data = 0 => receive parameter }
;
	lcall	GET_RXD_DATA			      ; get byte from commo interface
;
; { acc = checksum-byte read, dir = 0, data = x, clk = 1 }
;
	cpl	a				      ; get inverse of checksum
	cjne	a,checksum,COMMO_RXD_CHECKSUM_ERROR   ; jump checksum error
;
; { calculated checksum = received checksum }
;
; get received index/opcode
;
	mov	a,command_buffer			; get opcode
	cjne	a,last_command,COMMO_RXD_CHECKSUM_NEW   ; jump if current cmd <> last cmd
;
; { current cmd = last cmd }
;
	anl	commo_status,#RXD_STATUS_MASK_CLEAR     ; strip rxd status
	orl     commo_status,#RXD_STATUS_MASK_SM_CMD    ; rxd-status = same cmd
	setb	commo_report_cmd_sts			; report new cmd
	sjmp	COMMO_RXD_CHECKSUM_READY		; read

COMMO_RXD_CHECKSUM_NEW:
;
; { current cmd <> last cmd => new cmd }
;
	mov	last_command,a				; last_command = opcode just received
	anl	commo_status,#RXD_STATUS_MASK_CLEAR     ; strip rxd status
	orl     commo_status,#RXD_STATUS_MASK_NW_CMD    ; rxd-status = new cmd
	setb	commo_report_cmd_sts			; report new cmd
	sjmp	COMMO_RXD_CHECKSUM_READY		; read


COMMO_RXD_CHECKSUM_ERROR:
;
; { calculated checksum <> received checksum }
;
	anl	commo_status,#RXD_STATUS_MASK_CLEAR     ; strip rxd status
	orl     commo_status,#RXD_STATUS_MASK_ER_CMD    ; rxd-status = error cmd
	setb	commo_report_cmd_sts			; report new cmd


COMMO_RXD_CHECKSUM_READY:
	setb	commo_dir			        ; set dir line high
	mov	checksum,#00				; clear checksum
	anl	commo_status,#OP_STATUS_MASK_CLEAR      ; operation status = nop
	mov	a,#COMMO_IDLE_POS			; enter idle state
	ret

COMMO_RXD_CHECKSUM_EXIT:

	mov     a,#COMMO_RXD_CHECKSUM_POS               ; stay in current commo state
	ret                                             ; return




;*****************************************************************************
; commo state: transmit mode (opcode & data) waiting for DATA high
;*****************************************************************************
COMMO_TXD_DATA:
;
; { dir = 0, data = ?, clk = 1, byte_counter <> 0, byte_counter = 'valid' }
;
	jnb	commo_data,COMMO_TXD_DATA_EXIT		; jump if data-line low
;
; { commo_data = 1 => host can accept data }
;
	mov	r0,byte_pointer			        ; r0 = source
	mov	a,@r0					; a = data-byte to transmit
	lcall	TRANSMIT_TXD_DATA			; transmit data byte
;
; { dir = 1, data = ?, clk = 1 }
;
; calculate checksum
;
	mov	a,@r0					; a = data-byte just transmitted
	add	a,checksum				; a = data-byte + checksum
	mov     checksum,a				; checksum = checksum + data-byte

	clr     commo_dir				; clear dir line
	inc	byte_pointer				; point to next byte

	djnz	byte_counter,COMMO_TXD_DATA		; jump if byte_counter non zero
;
; { byte_counter = 0 => transmit checksum }
;
	clr	commo_txd_req				; clear transmit request

	jb	commo_checksum_txd_req,COMMO_TXD_DATA_CHKSM
;
; { commo_checksum_txd_req = 0 => no checksum to transmit }
;
	anl	commo_status,#TXD_STATUS_MASK_CLEAR 	; clear txd status (= ready without error)
	mov	a,#COMMO_IDLE_POS			; enter idle state (op = txd)
	ret                                             
								
COMMO_TXD_DATA_CHKSM:
;
; { commo_checksum_txd_req = 1 => checksum to transmit }
;

	mov	a,#COMMO_TXD_CHECKSUM_POS		; get next state
	ret

COMMO_TXD_DATA_EXIT:
	mov     a,#COMMO_TXD_DATA_POS                   ; stay in current state
	ret                                             ; return



;*****************************************************************************
; commo state: transmit mode (checksum) waiting for DATA high
;*****************************************************************************

COMMO_TXD_CHECKSUM:

;
; { dir = 0, data = ?, clk = 1, byte_counter  = 0 }
;
	jnb	commo_data,COMMO_TXD_CHECKSUM_EXIT	; jump if data-line low
;
; { commo_data = 1 => host can accept data }
;                                                       
	mov	a,checksum				; get checksum byte
	cpl	a					; inverse a
	lcall	TRANSMIT_TXD_DATA			; transmit data byte
;
; { dir = 1, data = ?, clk = 1 }
;
	anl	commo_status,#TXD_STATUS_MASK_CLEAR	; clear op. status
	orl	commo_status,#TXD_STATUS_MASK_RDY_NERR	; txd status = ready without error

	anl	commo_status,#OP_STATUS_MASK_CLEAR	; clear op status

	clr	commo_txd_req				; clear transmit request
	clr	commo_checksum_txd_req			; clear checksum transmit request
	mov	checksum,#00				; reset checksum reg.
	mov	a,#COMMO_IDLE_POS			; enter idle state (op = txd)
	ret

COMMO_TXD_CHECKSUM_EXIT:

	mov     a,#COMMO_TXD_CHECKSUM_POS		; stay in current state
	ret                                             ; return





;*****************************************************************************
; commo state: send 128 zero bytes over commo interface
;*****************************************************************************

COMMO_ERR_SEND_DIR:

	clr	commo_dir				 ; clear dir line
	nop						 ; delay one micro (to be sure)
	nop						 ; delay one micro (to be sure)

;
; enrty for state 'commo error send'
;
COMMO_ERR_SEND:
;
; { acc = data-byte read, dir = 0, data = x, clk = 1 byte_counter <> 128 }
;
	jnb    commo_data,COMMO_ERR_SEND_EXIT		; jump if data-line low
;
; { commo_data = 1 => host can accept data }
;
	clr	a					; clear accu (= data-byte to transmit)
	lcall	TRANSMIT_TXD_DATA			; transmit data byte
;
; { dir = 1, data = ?, clk = 1 }
;
	djnz	byte_counter,COMMO_ERR_SEND_DIR		; jump if byte_counter non zero
;
; { byte_counter = 0 }
;
	mov	a,#COMMO_ERR_RECV_POS			; get next state
	ret

COMMO_ERR_SEND_EXIT:

	mov     a,#COMMO_ERR_SEND_POS                   ; stay in current state
	ret                                             ; return

;*****************************************************************************
; commo state: receive 'null data' till data-byte non zero
;*****************************************************************************

COMMO_ERR_RECV:

; Pre condition: dir = 1, clk = 1, data = ?

	jb	commo_data,COMMO_ERR_RECV_EXIT
;
; { comm_data = 0 => receive byte }
;
	lcall	GET_RXD_DATA				; get byte from commo interface
;
; { acc = data-byte read, dir = 0, data = x, clk = 1 }
;
	nop
	setb	commo_dir				; set dir-line high
	nop

	jz	COMMO_ERR_RECV				; jump if received data-byte = 0
;
; { data-byte (opcode) <> 0 => no error }
;
	mov	b,a					; save received byte in B-reg.
	mov	checksum,a				; checksum = index/opcode

	anl	a,#COMMO_OPCODE_MASK			; get opcode only
	mov	dptr,#_command_length_table		; dptr = _command_length_table
	movc	a,@a + dptr				; a = command length
	mov	cmd_length,a			        ; cmd_length = a = command length

	mov	byte_counter,#00			; clear byte counter

	anl     commo_status,#OP_STATUS_MASK_CLEAR	; clear op status
	orl     commo_status,#OP_STATUS_MASK_RXD	; op. status = rxd

	mov	_commo_timer,#COMMO_RXD_TIME_OUT	; set receive time-out

;
; { command buffer is free }
;
	mov     command_buffer,b		        ; store recv. byte in cmd-buffer
	inc	byte_counter				; increment byte counter

	mov	a,byte_counter		 		; get byte-counter for compare
	cjne	a,cmd_length,COMMO_ERR_RECV_RXD_PARM
;
; { byte-counter = cmd-length }
;
	mov	a,#COMMO_RXD_CHECKSUM_POS       	; get next state
	ret

COMMO_ERR_RECV_RXD_PARM:
;
; { byte-counter <> cmd-length }
;
	mov		a,#COMMO_RXD_PARM_POS           ; get next state
	ret


COMMO_ERR_RECV_EXIT:

	mov     a,#COMMO_ERR_RECV_POS                   ; stay in current state
	ret                                             ; return


;******************************************************************************
;*                                                                            *
;*  Interface Operations                                                      *
;*                                                                            *
;******************************************************************************


;*
;* Function name: COMMO_INIT
;*              :
;*              :
;*
;* Abstract     : This function performs all initialisations for the commo interface module
;*              :
;*              :
;*
;* Input        :
;*              :
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       :
;*              :
;*              :
;*
;* Pre          : TRUE
;*              :
;*              :
;*
;* Post         : all variable initialised
;*              :
;*              :
;*/

_COMMO_INIT:

	mov     commo_state,#COMMO_IDLE_POS                 ; commo-interface state = idle

	mov     cmd_length,00                               ; command length = 00
	mov	checksum,#00		       		    ; checksum = 00
	mov     byte_counter,#00                            ; count register = 00
	mov     byte_pointer,#00                            ; pointer register = 00
	mov     commo_status,#00                	    
; bit 1,0 RXD-status = 0
; bit 3,2 TXD-status = 0
; bit 5,4 OPERATION-status = 0

	mov	 saved_byte_count,#00			    ; saved transmit counter = 0

	mov     last_command,#00                            ; save register of previuos receiced command = 00 (dummy value)

	mov     command_buffer,#00                          ; command buffer first byte = 00

	clr     commo_cmd_received	                    ; commo command received = 0
	clr     commo_txd_req                               ; commo transmit request = 0
	clr     commo_checksum_txd_req                      ; commo commo transmit request = 0
	clr	commo_report_cmd_sts			    ; commo report command status = 0

	ret                             	      	    ; return

; End COMMO_INIT




;*
;* Function name: new_cmd_received
;*              :
;*              :
;*
;* Abstract     : This function tests if a new commo command is received
;*              :
;*              :
;*
;* Input        :
;*              :
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       : result = { COMMO_NO_COMMAND, COMMO_NEW_COMMAND, COMMO_SAME_COMMAND, COMMO_CMD_ERROR
;*              :
;*
;* Pre          : TRUE
;*              :
;*              :
;*
;* Post         : commo receive status returned
;*              :
;*              :

_NEW_CMD_RECEIVED:
	jnb	commo_report_cmd_sts,NEW_COMMO_RECEIVED_EXIT
;
; { commo_report_cmd_sts = 1 }
;
	clr	commo_report_cmd_sts		; clear commo report command status
;
; return cmd buffer sts
;
	mov	a,commo_status			; get commo status
	anl	a,#RXD_STATUS_MASK		; strip cmd buffer status
	rr	a				; shift right
	rr      a                               ; shift right
	ret


NEW_COMMO_RECEIVED_EXIT:
;
; { commo command_received = 0 => no commo command received }
;
	clr	a				; no_cmd
	ret

; END NEW_COMMO_RECEIVED

;*
;* Function name: SEND_STRING
;*              :
;*              :
;*
;* Abstract     : This function checks first if the transfer is not busy.
;*              : If not busy then it requests for transmission of data.
;*              :
;*              :
;*
;* Input        : mode          { <> 0 =>  COMPLETE,  transmit data (no checksum)
;*              :                  = 0 =>  APPEND, 	  send data (no checksum)
;*              :               }
;*              : pointer       byte pointer to source address
;*              : count         number of data bytes to transmit
;*
;* Output       :
;*              :
;*
;* Return       : FALSE         transmit request not accepted
;*              : TRUE          transmit request accepted
;*              :
;*              :
;*              :
;*
;* Pre          : Commo interface initialised
;*              :
;*              :
;*
;* Post         : if result = COMMO_TRUE => transmit request accepted
;*              : if result <> COMMO_FALSE =>transmit not request accepted (busy)
;*              :
;*

_SEND_STRING:
	jb	commo_txd_req,SEND_STRING_FALSE
	jb	commo_checksum_txd_req,SEND_STRING_FALSE 
;
; { commo_txd_req = 0 and commo_checksum_txd_req = 0 }
;
	mov	byte_pointer,pointer		; save byte pointer (parameter)

	mov	a,mode				; get mode parameter
	jz      SEND_STRING_APPEND                       
;
; { mode <> 0 => complete }
;
	setb	commo_checksum_txd_req		; request to send also checksum

SEND_STRING_APPEND:

	mov	a,count				; get count parameter
	jz	SEND_STRING_TRUE		; jump if count = 0
;
; { count <> 0 }
;                                                        
	mov	saved_byte_count,a		; save counter value
	setb	commo_txd_req			; transmit request = true

SEND_STRING_TRUE:

	anl	commo_status,#TXD_STATUS_MASK_CLEAR
	orl	commo_status,#TXD_STATUS_MASK_PENDING	; txd status = 'pending'

	mov	a,#COMMO_TRUE                   ; return true
	ret                                     ; return

SEND_STRING_FALSE:

	mov	a,#COMMO_FALSE                  ; return false
	ret                                     ; return


; END SEND_STRING


;*
;* Function name: SEND_STRING_READY
;*              :
;*              :
;*
;* Abstract     : This function returns the servo transmission status
;*              :
;*              :
;*
;* Input        :
;*              :
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       : transmission status = { READY_WITHOUT_ERROR, READY_WITH_ERROR,
;*              :                         PEDING, BUSY, DISABLED }
;*              :
;*
;* Pre          : DSA module initialised
;*              :
;*              :
;*
;* Post         : Servo transmission status returned
;*              :
;*              :
;*/

_SEND_STRING_READY:

	mov	a,commo_status         		; get commo status
	anl	a,#TXD_STATUS_MASK		; strip txd status
	ret                                     ; return

;END SEND_STRING_READY




;*
;* Function name: FREE_CMD_BUFFER
;*              :
;*              :
;*
;* Abstract     : This function releases the command recieve buffer for accepting new commands.
;*              :
;*              :
;*
;* Input        :
;*              :
;*              :
;*
;* Output       :
;*              :
;*              :
;*
;* Return       :
;*              :
;*
;* Pre          :
;*              :
;*              :
;*
;* Post         : buffer free status returned
;*              :
;*              :

_FREE_CMD_BUFFER:

	jb	commo_report_cmd_sts,FREE_CMD_BUFFER_FALSE
;
;  { commo_report_cmd_sts = 0 => command reported }
;
	mov	a,commo_status                   ; get commo status
	anl	a,#OP_STATUS_MASK
	cjne	a,#OP_STATUS_MASK_RXD,FREE_CMD_BUFFER_TRUE
;
; { op. status = rxd => receiving data }
;
FREE_CMD_BUFFER_FALSE:

	mov	a,#COMMO_FALSE			  ; return false
	ret
				     
FREE_CMD_BUFFER_TRUE:
;
; { op. status <> rxd => not receiving data and 'command is reported'
;
	anl	commo_status,#RXD_STATUS_MASK_CLEAR ; rxd status = no command
	mov	command_buffer,#00		    ; clear opcode position

	mov	a,#COMMO_TRUE			    ; return true
	ret                         		    ; return

; END FREE_CMD_BUFFER */


;*
;* Function name: GET_BUFFER
;*              :
;*              :
;*
;* Abstract     : This function return a byte from the command buffer at requested position.
;*	        :
;*	        :
;*
;* Input        : position
;*	        :
;*	        :
;*
;* Output       :
;*	        :
;*	        :
;*
;* Return       : requested byte
;*	        :
;*	        :
;*
;* Pre          : RXD-status <> no_command
;*	        :
;*	        :
;*
;* Post         : if PRE-condition is true then requested byte returned
;*		:
;*		:
;*/

_GET_BUFFER:

	mov	a,commo_status		    ; get commo status
	anl	a,#RXD_STATUS_MASK          ; strip rxd status
	cjne	a,#RXD_STATUS_MASK_NO_CMD,GET_BUFFER_RXD
;
; { rxd status = no command }
;
	clr		a		    ; return zero (dummy)
	ret


GET_BUFFER_RXD:
;
; { rxd status <> no command }
;
	mov	a,#command_buffer     	    ; get command_buffer[0] address
	add 	a,position                  ; a = pointed address
	mov	r0,a			    ; r0 = a = pointed address
	mov	a,@r0			    ; a = immediate data, pointed by r0
	ret

; End GET_BUFFER */


;* Function name: GET_CURRENT_BUFFER_SIZE
;*              :
;*              :
;*
;* Abstract     :
;*	        :
;*	        :
;*             
;* Input        :
;*              :
;*	        :
;*
;* Output       :
;*	        :
;*	        :
;*
;* Return       :
;*	        :
;*	        :
;*
;* Pre          :
;*	        :
;*	        :
;*           
;* Post         :
;*	        :
;*	        :
;*/

_GET_CURRENT_BUFFER_SIZE:

	mov	a,#COMMAND_BUFFER_SIZE	     ; return command_buffer size
	ret

; End GET_CURRENT_BUFFER_SIZE;*/









;******************************************************************************
;*                                                                            *
;*  Function CAlls shared by Interface Operations                             *
;*                                                                            *
;******************************************************************************
;
;* Function name: GET_RXD_DATA
;*              :
;*              :
;*
;* Abstract     : This function receves one byte from the commo interface.
;*              :
;*	        :
;*
;* Input        :
;*	        :
;*	        :
;*
;* Output       :
;*	        :
;*	        :
;*
;* Return       : ACC = 'received byte'
;*	        :
;*	        :
;*
;* Pre          : DIR =1, DATA = 0, CLK = 1, op-status = RXD
;*	        :
;*	        :
;*
;* Post         : data-byte read, dir = 0, data = x, clk = 1
;*	        :
;*	        :
;*/
;
; Implementors note: 
; This subroutine is setup such that when clk is low the
; data is read after 2 a 3 micro seconds without using
; extra instructions.
;
GET_RXD_DATA:
	clr	commo_clk	; clear clock (host sets data-bit
	mov	b,#7		; bit-count = 8 - 1

LOOP_RXD:
	mov	c,commo_data	; copy data-line into carry
	setb	commo_clk	; clear clk (host sets data)

	rrc	a               ; update previous received data-bit
	clr	commo_clk       ; clear clock
	djnz	b,LOOP_RXD	; jump if bit-count non zero
;
; { b (bit-count) = 0 => read last bit }
;
	mov	c,commo_data	; copy data-line into carry
	setb	commo_clk	; clear clk (host sets data)


	rrc     a               ; update previous received data-bit
	clr	commo_dir	; ackowledge byte read
	ret

; /* End GET_RXD_DATA ;*/



;* Function name: TRANSMIT_TXD_DATA
;*              :
;*              :
;*
;* Abstract     : This function transmits one data byte over the commo interface.
;*	        :
;*	        :
;*
;* Input        : acc = 'byte to transmit'
;*	        :
;*	        :
;*
;* Output       :
;*	        :
;*	        :
;*           
;* Return       :
;*               :
;*	        :
;*
;* Pre          : dir = 0, data = 1, clk = 1
;*	        :
;*	        :
;*
;* Post         :
;*	        :
;*	        :
;*/

TRANSMIT_TXD_DATA:
	rrc	a		; get first bit (lsb) in carry
	mov	commo_data,c	; place bit on data line
	clr	commo_clk	; transfer mode
	mov	b,#7		; bit-count = 8 - 1
LOOP_TXD:
	setb	commo_clk       ; clock data-bit in host
	rrc     a		; get next bit in carry


	mov	commo_data,c	; place bit on data line
	clr	commo_clk	; transfer mode
	djnz	b,LOOP_TXD 	; jump if non zero
;
; { b (bit-count) = 0 => seven bit send
;
	setb	commo_clk	; clock last bit in host (msb)
	setb	commo_dir	; acknowledge byte transfer
	setb	commo_data	; data line high
	ret

; /* End TRANSMIT_TXD_DATA; */


	END
