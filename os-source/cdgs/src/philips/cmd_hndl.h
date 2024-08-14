/*
 * 	Copyright (c) 1992, Philips Consumer Electronics KMG Laser Optics ORD 
 *
 * File name    : cmd_hndlr.h
 *
 * Description  :
 *              :
 *              :
 *
 * Decisions    :
 *              :
 *              :
 *
 * History      : Created by: J.J.M.M. Geelen  date:
 *              :
 *              :
 *
 * Version      : 0.1
 *
 * Status       : Creation phase
 *              :
 *              :
 *
 * Action list  :
 *              :
 *              :
 *
 *
 */

/* Macro definitions */

extern Byte New_command(void);
extern Byte Cmd_acception_status(void);
extern Byte Command_handler_ready(void);

#define 	RESEND			0x80
#define 	S_STAT			0x81
#define 	S_CMD_ERR   		0x82
#define		S_ID			0x83
#define 	LED_CNTRL      	 	0x84
#define 	SET_ERROR_STATUS        0x85
#define 	SEND_AUTO_Q		0x86
#define		SEND_Q			0x87
#define         S_DISK_ERR		0x88
#define         S_CLOSED		0x89

#define         SEND_SUBCODE_REQ	0x07

#define 	START_PAUSE     	0x00
#define 	START_PLAY      	0x01
#define 	MODIFY_PLAY     	0x02
#define 	STOP_C	        	0x03
#define 	PLAY_PAUSE		0x04
#define 	NEW_PLAY		0x05
#define 	SEEK_PLAY		0x06
#define 	SEEK_STOP		0x07
#define 	PAUSE_PLAY		0x08
#define		ENT_DIA			0x09
#define		DIA			0x0A
#define		SEEK_PAUSE		0x0B
#define         OPEN_C			0x0C
#define		AREA_ERROR		0x0D

#define		NAME			0x02
#define 	NUM			0x0A
#define		REV			0x12
#define 	IDLE			0xFF
#define		NO_CMD			0x00

#define 	OPEN_S			0x00
#define 	STOP_S			0x01
#define 	PLAY_S			0x02
#define 	PAUSE_S			0x03

#define		AUTO_Q			0x06

#define     	ERR			0x02
#define     	ADRCTL          	0x03
#define     	TNO             	0x04
#define     	INDEX           	0x05
#define     	MIN             	0x06
#define     	SEC            	 	0x07
#define     	FRM             	0x08
#define     	ZERO            	0x09
#define     	AMIN            	0x0A
#define     	ASEC            	0x0B
#define     	AFRM            	0x0C
#define     	LEVEL_D         	0x0D
#define     	LEVEL_E         	0x0E

#define     	SMIN			0x01
#define     	SSEC           	 	0x02
#define     	SBLK           	 	0x03
#define     	EMIN           	 	0x04
#define     	ESEC            	0x05
#define     	EBLK            	0x06
#define     	MODE           	 	0x07
#define     	SPEED           	0x08
#define     	SESSION         	0x09
#define     	QMODE           	0x0A
#define     	RSVD            	0x0B

#define		STOPPED_CLOSED		0x01
#define		STOPPED_OPEN		0x00

#define		MS_80			10

