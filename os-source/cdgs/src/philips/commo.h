/* 
 * 	Copyright (c) 1992, Philips Consumer Electronics KMG Laser Optics ORD 
 *
 * File name    : commo.h
 *
 * Description  : This header file contains the interface operations prototypes
 *              : and constants.
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



extern void COMMO_INIT(void);
extern void COMMO_INTERFACE(void);

extern Byte NEW_CMD_RECEIVED(void);
extern Byte GET_BUFFER(Byte);
extern Byte SEND_STRING(Byte, Byte *, Byte);
extern Byte SEND_STRING_READY(void);
extern Byte FREE_CMD_BUFFER(void);

extern rom Byte command_length_table[];


/******************************************************************************
 * Commo Interface Operation return values                                    *
 ******************************************************************************/

#define	COMMO_FALSE                         0x00            /* false value						*/
#define	COMMO_TRUE                          0x01            /* true value                       */
#define	COMMO_ERROR                         0x02            /* error value                      */

#define	COMMO_READY_WITHOUT_ERROR         	0x00            /* commo ready without error value  */
#define	COMMO_READY_WITH_ERROR            	0x01            /* commo ready with error value     */
#define	COMMO_BUSY                        	0x03            /* commo busy value                 */
#define	COMMO_PENDING                      	0x04            /* commo pending value              */

#define	COMMO_NO_COMMAND					0x00			/* no command received              */
#define	COMMO_NEW_COMMAND					0x01			/* new command received             */
#define	COMMO_SAME_COMMAND                  0x02			/* same command received            */
#define	COMMO_CMD_ERROR                     0x03			/* command received with error      */


/******************************************************************************
 * Commo Interface Operation mask values                                      *
 ******************************************************************************/

#define	COMMO_OPCODE_MASK					0x0F		    /* command opcode mask				*/
#define	COMMO_INDEX_MASK					0xF0			/* command index mask               */

#define	SEND_STRING_COMPLETE                1				/* mode for 'Send string'including checksum */
#define	SEND_STRING_APPEND                  0				/* mode for 'Append' (no checksum)          */