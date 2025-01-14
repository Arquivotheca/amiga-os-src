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

#include "standard.h"

#include "dbug.h"
#include "bru_ipc.h"




/* DTM_NEW */

/* Allocate a BruMessage structure and perform it's low-level
 * initialization.
 */

struct BruMessage *create_bru_message( struct MsgPort *replyport )
{
	struct BruMessage *brumsg;


	brumsg = (struct BruMessage *)AllocMem( sizeof(struct BruMessage),
		MEMF_PUBLIC | MEMF_CLEAR );
	if( brumsg == NULL )
	{
		/* Out of memory? */
		return( NULL );
	}

	brumsg->message.mn_Node.ln_Name = BRM_TAG;
	brumsg->message.mn_Node.ln_Type = NT_MESSAGE;
	brumsg->message.mn_ReplyPort = replyport;
	brumsg->message.mn_Length = sizeof(struct BruMessage);

	return( brumsg );
}



/* DTM_NEW */

void delete_bru_message( struct BruMessage *brumsg )
{
	if( brumsg != NULL )
	{
		FreeMem( brumsg, brumsg->message.mn_Length );

	}
}



/* DTM_NEW */

/* Initialize the user data fields and send the message to the named port.
 * This expects a message structure created by a call to
 * create_bru_message().
 *
 * Return is 0 for success.
 */

int send_bru_message( struct BruMessage *brumsg, char *portname,
	char *string )
{
	struct MsgPort *port;


	if( strlen(string) >= BRM_STRINGSIZE )
	{
		/* String too long! */
		return( -1 );
	}

	strcpy( &brumsg->cmd[0], string );
	strcpy( &brumsg->result_string[0], "" );
	brumsg->result_code = 0;

	Forbid();
	if(  ( port = FindPort( portname ) ) != NULL )
	{
		/* Found the port, let's send the message */
		PutMsg( port, (struct Message *)brumsg );
	}
	Permit();

	if( port == NULL )
	{
		/* Port was not found */
		return( -1 );
	}

	return( 0 );
}



/* DTM_NEW */
/* Return is zero for success */

int reply_bru_message( struct BruMessage *brumsg, int code, char *string )
{
	if( string == NULL )
	{
		string = "";
	}

	if( strlen(string) >= BRM_STRINGSIZE )
	{
		/* String too long! */
		return( -1 );
	}

	/* Setup the return data fields */
	brumsg->result_code = code;
	strcpy( &brumsg->result_string[0], string );
	strcpy( &brumsg->cmd[0], "" );

	ReplyMsg( (struct Message *)brumsg );

	return( 0 );
}



/* DTM_NEW */

/* Verifies that the message actually originated from BRU.
 * This is just an additional bit of safety...
 */

BOOL is_bru_message( struct BruMessage *brumsg )
{
	if(  strcmp( brumsg->message.mn_Node.ln_Name, BRM_TAG ) == 0  )
	{
		return( TRUE );
	}

	return( FALSE );
}

