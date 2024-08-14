/* -----------------------------------------------------------------------
 * console.c    rlogin 38   rewrite
 *
 * $Locker:  $
 *
 * $Id: console.c,v 38.0 93/04/08 15:24:46 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	console.c,v $
 * Revision 38.0  93/04/08  15:24:46  bj
 * This is the complete rewrite of rlogin (version number 38.xx.)
 * These are the console routines.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/console.c,v 38.0 93/04/08 15:24:46 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#define CON 1

#include "rl.h"

/* ==============================================================
 * This function gets a character from the console and restarts 
 * the io request.
 * ==============================================================
 */

BYTE
console_getchar( struct glob *g )
{
	REGISTER SYSBASE ;
	REGISTER struct IOStdReq *readreq = g->g_ConReadReq ;
	BYTE rtn;


	if(!CheckIO((struct IORequest *)readreq))
	{
		WaitIO((struct IORequest *)readreq);
	}

	// get existing character then send request for the next //

	rtn = g->g_inchar ; 
	readreq->io_Command = CMD_READ;
	readreq->io_Data    = (APTR)&g->g_inchar ;
	readreq->io_Length  = sizeof(g->g_inchar);

	SendIO((struct IORequest *)readreq);
	return( rtn ) ;
}


/* ==============================================================
 * SetUpConsole()
 *
 * This function sets up the console device for both reading
 * and writing and *STARTS THE READ PROCESS !!!* via a call
 * to console_getchar() 
 * ==============================================================
 */


BOOL SetUpConsole(struct glob *g)
{
	DOSBASE(g) ;
	SYSBASE ;
	struct IOStdReq *cr, *cw ;
	struct MsgPort *crp, *cwp ;
	REGISTER struct glob *gl = g ;
	LONG error ;
 
	g->g_error = ERR_GETCONSOLE ;
	if(crp = CreateMsgPort())
	{
		if(cwp = CreateMsgPort())
		{
			if(cr  = CreateIORequest(crp, (long)sizeof(struct IOStdReq)))
			{
				if(cw  = CreateIORequest(cwp, (long)sizeof(struct IOStdReq)))
				{
					cw->io_Data         = (APTR)gl->g_Window ;
					cw->io_Length       = sizeof(struct Window) ;
					if((error = OpenDevice("console.device", gl->g_user_conunit, (struct IORequest *)cw, CONFLAG_DEFAULT)) != 0)
					{
						PutStr("error: console open.\n") ;
					}
					else
					{
						gl->g_ConReadPort     = crp ;
						gl->g_ConWritePort    = cwp ;
						gl->g_ConReadReq      = cr ;
						gl->g_ConWriteReq     = cw ;
						gl->g_ConUnit         = (struct ConUnit *)cw->io_Unit ;
						gl->g_console_is_open = 1 ;

						*cr = *cw;
						cr->io_Message.mn_ReplyPort = crp;
						cr->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
							// start the read //
						(void)console_getchar(g) ;
						gl->g_conread_was_used = 1 ;
						return(TRUE) ;
					}
					DeleteIORequest((struct IORequest *)cw) ;
				}
				DeleteIORequest((struct IORequest *)cr) ;
			}
			DeleteMsgPort(cwp) ;
		}
		DeleteMsgPort(crp) ;
	}
	return(FALSE) ;
}

/**************************************************************
 * KillConsole
 *************************************************************
 */

VOID
KillConsole(struct glob *g) 
{
	REGISTER SYSBASE ;
	REGISTER struct glob *gl = g ;
		
	if(gl->g_conwrite_was_used)
	{
		AbortIO((struct IORequest *)gl->g_ConWriteReq) ;
		WaitIO((struct IORequest *)gl->g_ConWriteReq) ;
	}

	if(gl->g_conread_was_used)
	{
		AbortIO((struct IORequest *)gl->g_ConReadReq) ;
		WaitIO((struct IORequest *)gl->g_ConReadReq) ;
	}

	if( gl->g_console_is_open)
	{
		CloseDevice((struct IORequest *)gl->g_ConWriteReq) ;
//		gl->g_console_is_open = 0 ;
	}
	
	if( gl->g_ConWriteReq )
	{
		DeleteIORequest((struct IORequest *)gl->g_ConWriteReq) ;
//		gl->g_ConWriteReq = NULL ;
	}

	if( gl->g_ConReadReq )
	{
		DeleteIORequest((struct IORequest *)gl->g_ConReadReq) ;
//		gl->g_ConReadReq = NULL ;
	}

	if( gl->g_ConWritePort ) ;
	{
		DeleteMsgPort(gl->g_ConWritePort) ;
//		gl->g_ConWritePort = NULL ;
	}

	if( gl->g_ConReadPort ) ;
	{
		DeleteMsgPort(gl->g_ConReadPort) ;
//		gl->g_ConReadPort = NULL ;
	}
}


/***************************************************************
 * CON_WRITE
 ***************************************************************
 */

VOID 
con_write(unsigned char *buf, ULONG len, struct glob *g)
{
	REGISTER struct IOStdReq *c = g->g_ConWriteReq ;
	REGISTER SYSBASE ;

		/* break big buffers into small pieces */

	
	while ( len > 80 ) 
	{
		c->io_Command = CMD_WRITE;
		c->io_Data    = (APTR)buf;
		c->io_Length  = 80L ;
		DoIO((struct IORequest *)c);
		buf += 80;
		len -= 80;
	}
	
	if( len ) 
	{
		c->io_Command = CMD_WRITE;
		c->io_Data    = (APTR)buf;
		c->io_Length  = (LONG)len; 
		DoIO((struct IORequest *)c);
	}
}

/* =============================================================
 * console_char_ready(void)
 *
 * returns TRUE if a keypress or has occurred,
 * and returns FALSE if not. Called in mainloop() function
 * =============================================================
 */

BOOL
console_char_ready( struct glob *g )
{
	SYSBASE ;

	return(CheckIO((struct IORequest *)g->g_ConReadReq)) ;
}

/* end */
