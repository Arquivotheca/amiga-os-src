	IFND	L64111_I
L64111_I		SET	1
**
**	$Id: l64111.i,v 40.3 94/01/26 11:58:37 kcd Exp $
**
**	Assembly include file for using the L64111
**
**      (C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	IFND	EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC


**
** L64111 register locations
**

DATA                  	EQU	$00
CONTROL_1             	EQU	$02
CONTROL_2             	EQU	$04
CONTROL_3             	EQU	$06
STATUS_INT_1    	EQU	$08
STATUS_INT_2    	EQU	$0A
TIMER_COUNT_DOWN      	EQU	$0C
TIMER_OFFSET_REGH     	EQU	$0E
TIMER_OFFSET_REGL     	EQU	$10
PARAMETRIC_DATA_WD_I  	EQU	$12
PARAMETRIC_DATA_WD_II 	EQU	$14
PARAMETRIC_DATA_WE_III	EQU	$16
PRES_TS_I     		EQU	$18
PRES_TS_II	    	EQU	$1A
PRES_TS_III   		EQU	$1C
PRES_TS_IV    		EQU	$1E
PRES_TS_V	     	EQU	$20
ANCILLARY_UD_FIFO     	EQU	$22
CHAN_BUF_STAT_CTR  	EQU	$24
CHANL_BUF_RD_CTR    	EQU	$28
CHANL_BUF_WR_CTR    	EQU	$26

; Bit defines for CONTROL_1.
ANC_DATA_FIFO_RST 	EQU	$80
SER_PAR           	EQU	$40
BYPASS            	EQU	$20
BUF_RST           	EQU	$04
SOFT_RST          	EQU	$02
DEC_STRT          	EQU	$01

; Bit defines for CONTROL_2.
ERROR_REPEAT 		EQU	$80
ERROR_MUTE   		EQU	$40
ERROR_IGNORE 		EQU	$00
SOFT_MUTE    		EQU	$20
SEL_16       		EQU	$10
AUDIO_ONLY   		EQU	$08
I2S	     		EQU	$04
PCM64FS	     		EQU	$02
PCM48FS	     		EQU	$01
PCB32FS	     		EQU	$00

; Bit defines for CONTROL_3.
AUDIO_STREAM_ID_USE 	EQU	$80
AUDIO_STREAM_ID_BIT 	EQU	$1F

; Bit defines for STATUS_INTERRUPT_1.
ANC_DATA_VALID      	EQU	$80
ANC_DATA_FIFO_OVRFL 	EQU	$40
ANC_DATA_FIFO_HFF   	EQU	$20
ERR_BUFF_UNDFLW     	EQU	$10
ERR_BUFF_OVL        	EQU	$08
BUFF_ALMST_EMTY     	EQU	$04

; Bit defines for STATUS_INTERRUPT_2.
SYNTAX_ERR_DET_W  	EQU	$80
PTS_AVAILABLE_W		EQU	$40
SYNC_AUD_W        	EQU	$20
SYNC_SYS_W        	EQU	$10
FRAME_DETECT_IN_W 	EQU	$08
ERR_CRC_W         	EQU	$04
NEW_FRAME_OUT_W   	EQU	$02

SYNTAX_ERR_DET_R  	EQU	$80
PTS_AVAILABLE_R		EQU	$40
SYNC_AUD_R        	EQU	$20
SYNC_SYS_R        	EQU	$10
FRAME_DETECT_IN_R 	EQU	$08
ERR_CRC_R         	EQU	$04
NEW_FRAME_OUT_R   	EQU	$02

; Bit mask for TIMER_COUNT_DOWN.
TCR_MASK 		EQU	$0F

; Common Bit mask for all CHANNEL_BUF registers.
CHANNEL_BUF_MASK 	EQU	$7F

L64111_UPPER_THRESHOLD	EQU	$30

	ENDC	; L64111_I