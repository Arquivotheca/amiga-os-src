/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

#define BRM_STRINGSIZE		256

#define BRM_TAG				"BRUMSG"



struct BruMessage {
	struct Message	message;

	char			cmd[BRM_STRINGSIZE];
		/* Command line from BRU */

	UWORD			result_code;
		/* Error code return. */

	char			result_string[BRM_STRINGSIZE];
		/* Result string return. */
};



extern struct BruMessage *create_bru_message( struct MsgPort *replyport );
extern void delete_bru_message( struct BruMessage *brumsg );
extern int send_bru_message( struct BruMessage *brumsg, char *portname,
	char *string );
extern int reply_bru_message( struct BruMessage *brumsg, int code,
	char *string );
extern BOOL is_bru_message( struct BruMessage *brumsg );

