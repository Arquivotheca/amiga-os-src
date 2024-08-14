/***************************************************************************
*
* Project:	BASIC PLAYER MODULE
* File:		defs.h
* Date:		06 July 1993
* Version:	0.1
* Status:	Draft
*
* Description:	contains all generally used definitions
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 16 November 1992	creation H.v.K.
* 09 April 1993		play command "PLAY_ERROR" removed.
*			player module command "PLAYER_HANDLE_ERROR" added.
* 13 April 1993		commands "VOLUME_UP" and "VOLUME_DOWN"
*			merged to one command: "SET_VOLUME".
*			new command: "JUMP_TRACKS".
* 20 April 1993		dac-output modes defined.
* 23 April 1993		inputs for function "is_subcode" changed.
* 04 May 1993		definitions for function "get_jump_status" added.
* 13 May 1993		command for play module "PLAY_STARTUP" added.
* 14 May 1993		new command for start_stop module: SS_IDLE.
* 			new command for tray module: TRAY_IDLE.
* 26 May 1993		new command for player module: SET_SERVICE_MODE.
*			service command opcodes added.
* 28 May 1993		error handling opcode moved to 0x00.
*			defined: MAX_LEGAL_NORMAL_ID and MAX_LEGAL_SERVICE_ID.
* 18 June 1993		all servo related defines are moved to "serv_def.h".
* 24 June 1993		new player error code: HF_DETECTOR_ERROR.
* 29 June 1993		new command for start_stop module: SS_MOTOR_OFF.
* 06 July 1993		lid switch states defined.
*
***************************************************************************/
/* when single session toc reading: */
#define	DO_MULTI_SESSION	/* comment this line */
				/* in file defs.asm change line:
				   %SET (HF_DETECTOR,%DO_MULTI_SESSION)
				   into:
				   %SET (HF_DETECTOR,%DO_SINGLE_SESSION) */

#define	const				rom

#define	MAX_TRACK_STORED_IN_TOC		20

typedef	unsigned char			byte;

struct interface_field {
	byte	p_status;
	byte	a_command;
	byte	param1;
	byte	param2;
	byte	param3;
		       };

/* definitions of a_command (application commands = process_id-1) */
/* ==== normal mode opcodes ==== */
#define	TRAY_OUT_OPC			0x00
#define	TRAY_IN_OPC			0x01
#define	START_UP_OPC			0x02
#define	STOP_OPC			0x03
#define	PLAY_TRACK_OPC			0x04
#define	PAUSE_ON_OPC			0x05
#define	PAUSE_OFF_OPC			0x06
#define	SEEK_OPC			0x07
#define	READ_TOC_OPC			0x08
#define	READ_SUBCODE_OPC		0x09
#define	SINGLE_SPEED_OPC		0x0A
#define	DOUBLE_SPEED_OPC		0x0B
#define	SET_VOLUME_OPC			0x0C
#define	JUMP_TRACKS_OPC			0x0D
#define ENTER_SERVICE_MODE_OPC		0x0E

/* ==== service mode opcodes ==== */
#define ENTER_NORMAL_MODE_OPC		0x0F
#define LASER_ON_OPC			0x10
#define LASER_OFF_OPC			0x11
#define FOCUS_ON_OPC			0x12
#define FOCUS_OFF_OPC			0x13
#define SPINDLE_MOTOR_ON_OPC		0x14
#define SPINDLE_MOTOR_OFF_OPC		0x15
#define RADIAL_ON_OPC			0x16
#define RADIAL_OFF_OPC			0x17
#define MOVE_SLEDGE_OPC			0x18
#define JUMP_GROOVES_OPC		0x19
#define WRITE_CD6_OPC			0x1A
#define WRITE_DSIC2_OPC			0x1B
#define READ_DSIC2_OPC			0x1C

/* ==== internal opcodes ==== */
#define	IDLE_OPC			0xFF
#define	ERROR_HANDLING_ID		0x00
#define MAX_LEGAL_NORMAL_ID		0x0F
#define MAX_LEGAL_SERVICE_ID		0x1D

/* definitions of tray module commands */
#define	TRAY_IDLE			0x00
#define	TRAY_OUT			0x01
#define	TRAY_IN				0x02

/* definitions of start/stop module commands */
#define	SS_IDLE				0x00
#define	SS_STOP				0x01
#define	SS_START_UP			0x02
#define	SS_SPEED_N1			0x03
#define	SS_SPEED_N2			0x04
#define	SS_MOTOR_OFF			0x05

/* definitions of play module commands */
#define	PLAY_IDLE			0x00
#define PLAY_STARTUP                    0x01
#define	PAUSE_ON			0x02
#define	PAUSE_OFF			0x03
#define	JUMP_TO_ADDRESS			0x04
#define	PLAY_TRACK			0x05
#define	PLAY_READ_SUBCODE		0x06
#define	PLAY_READ_TOC			0x07
#define	PLAY_PREPARE_SPEED_CHANGE	0x08
#define	PLAY_RESTORE_SPEED_CHANGE	0x09
#define	PLAY_SET_VOLUME			0x0A
#define	PLAY_JUMP_TRACKS		0x0B

/* definitions of player module commands */
#define	PLAYER_IDLE			0x00
#define PLAYER_HANDLE_ERROR		0x01
#define SET_SERVICE_MODE		0x02

struct time {
	byte	min;
	byte	sec;
	byte	frm;
	    };

struct subcode_frame {
	byte		conad;
	byte		tno;
	byte		index;
	struct time	r_time;
	byte		zero;
	struct time	a_time;
		     };

struct  bytes {
	byte	high;
	byte	low;
	      };
union int_high_low {
	int		int_i;
	struct bytes	byte_hl;
		   };
typedef union int_high_low 	int_hl;

/* definitions for the process execution states (should be the same as in defs.asm) */
#define	BUSY				0
#define READY				1
#define ERROR				2
#define PROCESS_READY			3

/* definitions for the compare_time function (should be the same as in defs.asm) */
#define SMALLER				0
#define EQUAL				1
#define BIGGER				2
	
/* definitions for function "is_subcode" inputs */
#define ALL_SUBCODES			0
#define ABS_TIME			1
#define CATALOG_NR			2
#define ISRC_NR				3
#define FIRST_LEADIN_AREA		4
#define LEADIN_AREA			5
#define PROGRAM_AREA			6
#define LEADOUT_AREA			7

#define CLOSED				0
#define OPEN				1
#define LID_OPEN			0
#define LID_CLOSED              	1

#define TRUE				1
#define FALSE				0

/* definitions of player_error */
#define NO_ERROR			0x00
#define	ILLEGAL_COMMAND			0x01
#define	ILLEGAL_PARAMETER		0x02
#define SLEDGE_ERROR			0x03
#define FOCUS_ERROR			0x04
#define MOTOR_ERROR			0x05
#define RADIAL_ERROR			0x06
#define PLL_LOCK_ERROR			0x07
#define SUBCODE_TIMEOUT_ERROR		0x08
#define SUBCODE_NOT_FOUND		0x09
#define	TRAY_ERROR			0x0A
#define	TOC_READ_ERROR			0x0B
#define	JUMP_ERROR			0x0C
#define	HF_DETECTOR_ERROR		0x0D
