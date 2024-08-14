/* 
 * 	Copyright (c) 1992, Philips Consumer Electronics KMG Laser Optics ORD 
 *
 * File name    : sts_q_id.h
 *
 * Description  : This header file contains all the prototypes and constants
 *              : for the packet buffer module (STATUS_Q_ID).
 *              :
 *
 * Decisions    :
 *              :
 *              :
 *
 * History      : Created by: J.J.M.M. Geelen  date: 09 - 03 - '93
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
/* Error detect mask */

#define	ERROR_DETECT_MASK					0x10				/* test is done on bit 4 */


/*
 * Function name: GET_DRV_STATUS
 *              :
 *              :
 *
 * Abstract     : This macro return the drive status which is encoded in x
 *			    :
 *		        :
 *
 * Input        : x => byte containing the drive status
 *			    :
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       : drive status = { 	STOPPED_AND_DOOR_OPEN = 0,
 *				:                   STOPPEN_AND_DOOR_CLOSED = 1,
 *				:					PLAYING = 2,
 *				:					PAUSED = 3 }
 *			    :
 *		        :
 *
 * Pre          : True
 *			    :
 *		        :
 *
 * Post         : drive status returned
 *			    :
 *		        :
 */

#define	GET_DRV_STATUS(x)		( (Byte) ((x) & 3) )



/*
 * Function name: STORE_DRV_STATUS
 *              :
 *              :
 *
 * Abstract     : This macro encodes the drive status in 'y'
 *			    :
 *		        :
 *
 * Input        : y -> byte to store 'drive status'
 *			    : x -> drive status
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       :
 *			    :
 *		        :
 *
 * Pre          : True
 *			    :
 *		        :
 *
 * Post         : drive status encoded in 'y'
 *			    :
 *		        :
 */

#define	STORE_DRV_STATUS(y, x)  ( (y) = (((y) & 0xFC) | ((x) & 0x03)) )


/*
 * Function name: CHECK_ERROR_CONDITION
 *              :
 *              :
 *
 * Abstract     :
 *			    :
 *		        :
 *
 * Input        :
 *			    :
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       : error condition status
 *			    : 00 -> when no error (command in progress information)
 *		        : 10 -> when there is an error
 *
 * Pre          :
 *			    :
 *		        :
 *
 * Post         :
 *			    :
 *		        :
 */

#define	CHECK_ERROR_CONDITION(y)  ( (Byte) ((y) & ERROR_DETECT_MASK) )



/*
 * Function name: GET_CMD_OR_ERROR
 *              :
 *              :
 *
 * Abstract     : This macro returns the encode command in progress status
 *			    : or the error condition
 *		        :
 *
 * Input        :
 *			    :
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       : command progress or error condition
 *			    :
 *		        : bit 4 = 0 => bit 3,2,1,0 -> command in progress information
 *				: bit 4 = 1 => bit 3,2,1,0 -> error condition
 *
 * Pre          : True
 *			    :
 *		        :
 *
 * Post         : command progress or error condition returned
 *			    :
 *		        :
 */

#define	GET_CMD_OR_ERROR(x)  ( (Byte) (((x) >> 3) & 0x01F) )


/*
 * Function name: STORE_CMD_PROGRESS
 *              :
 *              :
 *
 * Abstract     : This macro encodes the Command in progress information in 'y'
 *			    :
 *		        :
 *
 * Input        : y -> variable to encode Command in progress information
 *			    : x -> Command in progress information
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       :
 *			    :
 *		        :
 *
 * Pre          : True
 *			    :
 *		        :
 *
 * Post         : Command in progress information encoded in 'y'
 *			    :
 *		        :
 */

#define	STORE_CMD_PROGRESS(y, x) 	( (y) = ( ((y) & 0x07) | (((x) << 3) & 0x78)) )



/*
 * Function name: STORE_ERROR_COND
 *              :
 *              :
 *
 * Abstract     : This macro encodes the Error condition in 'y'
 *			    :
 *		        :
 *
 * Input        : y -> variable to encoded the Error condition
 *			    : x -> Error condition
 *		        :
 *
 * Output       :
 *			    :
 *		        :
 *
 * Return       :
 *			    :
 *		        :
 *
 * Pre          : True
 *			    :
 *		        :
 *
 * Post         : Error condition encoded in 'y'
 *			    :
 *		        :
 */

#define	STORE_ERROR_COND(y, x)  ( (y) = ((((y) & 0x07) | 0x80) | (((x) << 3) & 0xF8)) )



/* Q_ready and ID_ready Packet lengths */

#define	STATUS_PACKET_LENGTH				 2		/* excluding checksum */
#define	Q_PACKET_LENGTH						15 		/* excluding checksum */
#define	ID_PACKET_LENGTH					20		/* excluding checksum */


/* drive status */

#define	STOPPED_AND_DOOR_OPEN		    0
#define	STOPPEN_AND_DOOR_CLOSEDV            1
#define	PLAYING                             2
#define	PAUSED                              3


/* Command in progress information */

#define	COMPLETE	     	      	    0
#define	PLAYING_REQ_SECTORS                 1

#define	SPINNING_DOWN                       4
#define	SPINNING_UP                         5

#define	SEEKING                             8


/* Error condition */

#define	BAD_COMMAND			    0
#define	CHECKSUM_ERROR                      1

#define	DISK_UNREADABLE                     3
#define	INVALID_START_ADDRESS               4
#define	WRONG_TYPE_OF_DATA                  5

#define FOCUS_SERVO_ERROR                   9
#define	SPINDLE_SERVO_ERROR                 10
#define	TRACKING_SERVO_ERROR                11
#define	SLED_ERROR                          12
#define	ABNORMAL_TRACK_JUMP                 13
#define ABNORMAL_SEEK                       14
#define	DRIVE_ERROR	                    15

extern Byte Store_update_status(Byte);
extern Byte Store_command(Byte);
extern Byte Store_status(Byte);
extern Byte Store_cmd_progress(Byte);
extern Byte Store_error_condition(Byte);
extern Byte Store_drive_status(Byte);
extern Byte Store_q_id(Byte, Byte);
extern Byte Get_command(void);
extern Byte Get_status(void);
extern Byte Get_q_id(Byte);
extern Byte Get_update_status(void);
extern Byte *Get_sts_q_id_ptr(void);
extern void Clear_update(void);
extern void Init_sts_q_id(void);
extern byte Get_last_status_update(void);

/* return values for interface operation 'Get_update_status()' */

#define	NO_UPDATE			0
#define	STATUS_UPDATE                   1
#define	Q_READY                         2
#define	ID_READY                        3

/* additional 'update status' */

#define DISPATCH_STATUS                	4
#define STATUS_Q_ID_ARRAY_SIZE		20
