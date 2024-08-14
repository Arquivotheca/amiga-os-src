

SKIP_ODD	MACRO
SOFFSET     SET     SOFFSET+1
			ENDM

		STRUCTURE CheckPointSerial,0
			BYTE	cps_InputPort
			STRUCT	cps_pad1,31
			BYTE	cps_ModeRegA
			SKIP_ODD
			LABEL	cps_StatusRegA
			BYTE	cps_ClockSelRegA
			SKIP_ODD
			BYTE	cps_CommandRegA
			SKIP_ODD
			LABEL	cps_RecBufferA
			BYTE	cps_XmitBufferA
			SKIP_ODD
			LABEL	cps_InputPortChange
			BYTE	cps_AuxControlReg
			SKIP_ODD
			LABEL	cps_IntStatusReg
			BYTE	cps_IntMaskReg
			SKIP_ODD
			LABEL	cps_CurCounterMSB
			BYTE	cps_CountTimerUR
			SKIP_ODD
			LABEL	cps_CurCounterLSB
			BYTE	cps_CountTimerLR
			SKIP_ODD
			BYTE	cps_ModeRegB
			SKIP_ODD
			LABEL	cps_StatusRegB
			BYTE	cps_ClockSelRegB
			SKIP_ODD
			BYTE	cps_CommandRegB
			SKIP_ODD
			LABEL	cps_RecBufferB
			BYTE	cps_XmitBufferB
			SKIP_ODD
			BYTE	cps_IntVectorReg
			SKIP_ODD
			LABEL	cps_InputPortUnlatched
			BYTE	cps_OutputPortConfig
			SKIP_ODD
			LABEL	cps_StartCounter
			BYTE	cps_OutputPortSet
			SKIP_ODD
			LABEL	cps_StopCounter
			BYTE	cps_OutputPortReset


; Register bit definitions:

;	Mode Register "1":

MR1_5DB		EQU		$0		; 5 data bits
MR1_6DB		EQU		$1		; 6 data bits
MR1_7DB		EQU		$2		; 7 data bits
MR1_8DB		EQU		$3		; 8 data bits

MR1_EP		EQU		$0		; Even parity
MR1_OP		EQU		$4		; Odd parity
MR1_SP		EQU		$8		; Space parity
MR1_MP		EQU		$C		; Mark parity
MR1_NP		EQU		$10		; No parity
MR1_MD		EQU		$18		; Multi-drop, Data
MR1_MA		EQU		$1C		; Multi-drop, Address

MR1_CH		EQU		$0		; Character error mode
MR1_BLK		EQU		$20		; Block error mode

MR1_IRX		EQU		$0		; Interrupt on Rx Ready
MR1_IFF		EQU		$40		; Interrupt on FIFO Full

MR1_RTS		EQU		$80		; Reciever controls RTS

; Mode Register "2":

MR2_1SB		EQU		$7		; One stop bit
MR2_2SB		EQU		$F		; Two stop bits

MR2_CTS		EQU		$10		; CTS enable transmitter

MR2_RTS		EQU		$20		; Transmitter controls RTS

MR2_ECH		EQU		$40		; Auto echo
MR2_LLP		EQU		$80		; Local loopback
MR2_RLP		EQU		$C0		; Remote loopback

; Clock Source Register.
; These values for transmitter rate, << 4 for receiver rate.

CSR_50		EQU		$0		; 50 baud (ACR_BRS = 0)
CSR_75		EQU		$0		; 75 baud (ACR_BRS = 1)
CSR_110		EQU		$1		; 110
CSR_134		EQU		$2		; 134.5
CSR_200		EQU		$3		; 200 (ACR_BRS=0)
CSR_150		EQU		$3		; 150 (ACR_BRS=1)
CSR_300		EQU		$4		; 300
CSR_600		EQU		$5		; 600
CSR_12H		EQU		$6		; 1200
CSR_10H		EQU		$7		; 1050 (ACR_BRS=0)
CSR_20H		EQU		$7		; 2000 (ACR_BRS=1)
CSR_24H		EQU		$8		; 2400
CSR_48H		EQU		$9		; 4800
CSR_72H		EQU		$A		; 7200 (ACR_BRS=0)
CSR_18H		EQU		$A		; 1800 (ACR_BRS=1)
CSR_96H		EQU		$B		; 9600
CSR_38K		EQU		$C		; 38,400 (ACR_BRS=0)
CSR_19K		EQU		$C		; 19,200 (ACR_BRS=1)
CSR_TIM		EQU		$D		; baud rate = Timer / 16
CSR_IPX		EQU		$E		; baud rate = IP / 16
CSR_IP1		EQU		$F		; baud rate = IP / 1

; Command register

CR_RCE		EQU		$1		; Enable receiver
CR_RCD		EQU		$2		; Disable receiver

CR_TE		EQU		$4		; Enable transmitter
CR_TD		EQU		$8		; Disable transmitter

CR_MR1		EQU		$10		; Reset MR pointer to MR1
CR_RR		EQU		$20		; Reset receiver
CR_RT		EQU		$30		; Reset transmitter
CR_RE		EQU		$40		; Reset error status
CR_RCB		EQU		$50		; Reset channel's break-change interrupt
CR_SB		EQU		$60		; Start break
CR_EB		EQU		$70		; End break

; Status Register:

SR_RXR		EQU		$1		; Reciever data ready
SR_FFU		EQU		$2		; Reciever FIFO full
SR_TXR		EQU		$4		; Transmitter Ready
SR_TXE		EQU		$8		; Transmitter Empty
SR_OVR		EQU		$10		; Receiver overrun
SR_PAR		EQU		$20		; Parity error
SR_FRA		EQU		$40		; Framing error
SR_BRE		EQU		$80		; Received break

; Bit # versions...
SRB_RXR		EQU		0		; Reciever data ready
SRB_FFU		EQU		1		; Reciever FIFO full
SRB_TXR		EQU		2		; Transmitter Ready
SRB_TXE		EQU		3		; Transmitter Empty
SRB_OVR		EQU		4		; Receiver overrun
SRB_PAR		EQU		5		; Parity error
SRB_FRA		EQU		6		; Framing error
SRB_BRE		EQU		7		; Received break

; Aux control register:

ACR_II0		EQU		$1		; Cause interrupt on change in IP0
ACR_II1		EQU		$2		; Cause interrupt on change in IP1
ACR_II2		EQU		$4		; Cause interrupt on change in IP2
ACR_II3		EQU		$8		; Cause interrupt on change in IP3

; Counter/timer mode:

ACR_EXT		EQU		$0		; External clock source (IP2)
ACR_TXA		EQU		$10		; TxCA X 1 clock source
ACR_TXB		EQU		$20		; TxCB X 1 clock source
ACR_X16		EQU		$30		; X1/CLK / 16 clock source
ACR_IP2		EQU		$40		; External (IP2)
ACR_X1		EQU		$60		; X1/CLK / 1 clock source

ACR_BRS		EQU		$80		; Baud rate line select

; Input port change register:

IPC_LP0		EQU		$0		; Input level P0
IPC_LP1		EQU		$1		; Input level P1
IPC_LP2		EQU		$2		; Input level P2
IPC_LP3		EQU		$3		; Input level P3
IPC_DP0		EQU		$4		; Delta detected P0
IPC_DP1		EQU		$5		; Delta detected P1
IPC_DP2		EQU		$6		; Delta detected P2
IPC_DP3		EQU		$7		; Delta detected P3

; Interrupt status register and Interrupt Mask Register:

ISR_TRA		EQU		$0		; Transmit Ready port A
ISR_RRA		EQU		$1		; Receiver Ready port A
ISR_DBA		EQU		$2		; Break delta A
ISR_CTR		EQU		$3		; Counter/timer ready
ISR_TRB		EQU		$4		; Transmit Ready B
ISR_RRB		EQU		$5		; Receiver Ready B
ISR_DBB		EQU		$6		; Delta Break B
ISR_IPC		EQU		$7		; Input port change

; Latch register bit definitions, bit number versions:

LATB_DSRB 	EQU		0		; DSR for channel B
LATB_DCDB 	EQU		1		; DCD for channel B
LATB_RIB 	EQU		2		; RI  for channel B
LATB_DSRA 	EQU		3		; DSR for channel A
LATB_DCDA 	EQU		4		; DCD for channel A
LATB_RIA 	EQU		5		; RI  for channel A

; Mask versions:
LAT_DSRA 	EQU		(1<<LATB_DSRA)	; DSR for channel A
LAT_DCDA 	EQU		(1<<LATB_DCDA)	; DCD for channel A
LAT_RIA 	EQU		(1<<LATB_RIA)	; RI  for channel A
LAT_DSRB 	EQU		(1<<LATB_DSRB)	; DSR for channel B
LAT_DCDB 	EQU		(1<<LATB_DCDB)	; DCD for channel B
LAT_RIB 	EQU		(1<<LATB_RIB)	; RI  for channel B
