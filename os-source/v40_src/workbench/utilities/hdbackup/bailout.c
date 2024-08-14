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

#include <workbench/startup.h>

#include "bailout.h"
#include "brushell.h"
#include "mainwin.h"
#include "dbug.h"



extern struct WBStartup *WBenchMsg;



static int myreq( char *string, char *buttons )
{
	static struct EasyStruct es = {
		sizeof( struct EasyStruct ),
		0,
		"HDBackup",
		NULL,
		NULL
	};

	es.es_TextFormat = string;
	es.es_GadgetFormat = buttons;
	
	return(  (int)EasyRequest( mainwin, &es, NULL, 0 )  );
}



/*
 * display the string and either quit no matter what (mode==1) or
 * let the user decide if a retry is in order (mode==0)
 */

static int inner_bailout( char *string, ERRORCODE code, int mode )
{
    char *buttontext;
    int rc;
    char buf[100];

    
    DBUG_ENTER ("inner_bailout");

    /* decide wether to display try again in one of the gadgets */
    if( mode )
	{
		buttontext = "Leave Program";
    }
	else
	{
		buttontext = "Try Again|Leave Program";
    }
    
    if( IntuitionBase )
	{
		/* 
		 * if intuition has been opened, put up an autorequester
		 * with our error message
		 */
		sprintf (buf, "%s   [ %04x ]", string, code);

		/* and post it */
		rc = myreq( buf, buttontext );
    }
	else
	{
		/* 
		 * Unable to use intuition, see if we were started from
		 * CLI and can print the error that way.  If not, then we
		 * have no way to talk to the user.  Just cleanup and exit.
		 */
		if( WBenchMsg == NULL )
		{
		    puts (string);
		}
		
		rc = 0;			/* don't ask, just exit the program */
    }

    /* 
     * if this is a query bailout and the user said try again, do not
     * exit, just return to caller
     */
    if(  ( mode == 0 ) && rc )
	{
		DBUG_RETURN( 0 );
    }
    
    /* Execute the programs standard cleanup routine and exit */
    cleanup();
    
    exit( RETURN_FAIL );

    DBUG_RETURN( 0 );
}



/* This will display the string and exit */

void bailout( char *string, ERRORCODE code )
{
    DBUG_ENTER ("bailout");
    inner_bailout (string, code, 1);
    DBUG_VOID_RETURN;
}



/*
 * This one will display the string and return to the calling function
 * if the user says too (assuming intuition is open)
 *
 * Usage would be as thus:
 *
 *		while(  ! (someBase = OpenLibrary("some.library",0L))  )
 *		{
 *			querybailout("Unable to open some Library");
 *		}
 *
 */

void querybailout( char *string, ERRORCODE code )
{
    DBUG_ENTER ("querybailout");
    inner_bailout (string, code, 0);
    DBUG_VOID_RETURN;
}



#if 0
void _abort ()
{
    DBUG_ENTER ("_abort");
    bailout ("Control 'C' termination by user.", ERC_NONE | ERR10);
    DBUG_VOID_RETURN;
}
#endif



int mention_error( char *string, ERRORCODE code )
{
    char buf[256];


    DBUG_ENTER ("mention_error");

    sprintf( buf, "%s   [ %04x ]", string, code);

    DBUG_RETURN(  myreq( buf, "OK" )  );
}



int retry_error( char *string, ERRORCODE code )
{
    char buf[256];


    DBUG_ENTER ("retry_error");
    sprintf( buf, "%s   [ %04x ]", string, code );
    DBUG_RETURN (  myreq( buf, "Try Again|Proceed Anyway" )  );
}



int ask_question( char *string )
{
    DBUG_ENTER ("ask_question");
    DBUG_RETURN(  myreq( string, "Yes|No" )  );
}



void tell_user( char *string )
{
    DBUG_ENTER ("tell_user");
    (void)myreq( string, "OK" );
    DBUG_VOID_RETURN;
}
