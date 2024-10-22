/* fake_bru.c */




#include "standard.h"

#include "bru_ipc.h"



void bru_test( void );
void get_reply( struct MsgPort *replyport );
void backup( void );


struct BruMessage *brumsg;
struct MsgPort *replyport;



main( int argc, char *argv[] )
{
	int i;


	fprintf( stderr, "BRU: starting fake_bru\n" );

	for( i=0; i<argc; i++ )
	{
		fprintf( stderr, "BRU: Arg %2d is \"%s\"\n", i, argv[i] );
	}

	bru_test();

	fprintf( stderr, "BRU: done with fake_bru\n" );
}



void bru_test( void )
{
	replyport = CreatePort( "rats", 0 );
	if( replyport == NULL )
	{
		fprintf( stderr, "BRU: Can't create reply port\n" );
		return;
	}

	brumsg = create_bru_message( replyport );
	if( brumsg == NULL )
	{
		DeletePort( replyport );
		fprintf( stderr, "BRU: Can't create bru msg\n" );
		return;
	}


	if(  send_bru_message( brumsg, "HDBACKUP_CBM", "volume" ) != 0  )
	{
		fprintf( stderr, "BRU: Unable to send bru msg [1]\n" );
		return;
	}

	get_reply( replyport );

	backup();

	fprintf( stderr, "\n" );

	delete_bru_message( brumsg );
	DeletePort( replyport );
}



void get_reply( struct MsgPort *replyport )
{
	struct Message *msg;
	struct BruMessage *brumsg2;


	fprintf( stderr, "BRU: waiting for reply\n" );

	WaitPort( replyport );

	fprintf( stderr, "BRU: got reply\n" );

	msg = GetMsg( replyport );
	if(  is_bru_message( (struct BruMessage *)msg )  )
	{
		fprintf( stderr, "BRU: reply is a BRU msg\n" );

		brumsg2 = (struct BruMessage *)msg;

		fprintf( stderr, "BRU: Code: %d    String: \"%s\"\n",
			brumsg2->result_code, &brumsg2->result_string );
	}
	else
	{
		fprintf( stderr, "BRU: Not a BRU message!\n" );
	}
}



void backup( void )
{
	do
	{
		if(  send_bru_message( brumsg, IPCPORT_NAME, "filename" ) != 0  )
		{
			fprintf( stderr, "BRU: Unable to send bru msg [2]\n" );
			return;
		}

		get_reply( replyport );
	}
	while(  strncmp( &brumsg->result_string[0], "::", 2 ) != 0  );


	if(  send_bru_message( brumsg, IPCPORT_NAME, "finished" ) != 0  )
	{
		fprintf( stderr, "BRU: Unable to send bru msg [3]\n" );
		return;
	}

	get_reply( replyport );
}

