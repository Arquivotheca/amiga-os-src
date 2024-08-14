/* -----------------------------------------------------------------------
 * clip.c   rlogin 38    
 *
 *   these functions support the console cut & paste operations. These
 *   make use of knowledge of the internal operation of this and should
 *   not be the basis for demonstarting the proper way to do this. This
 *   bypasses the clipboard and takes the info direcxtly from the internal
 *   buffer.
 *
 * $Locker:  $
 *
 * $Id: clip.c,v 38.0 93/04/13 11:28:40 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	clip.c,v $
 * Revision 38.0  93/04/13  11:28:40  bj
 * This is the complete rewrite of rlogin (v38)
 * These are the conclip support routines.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/clip.c,v 38.0 93/04/13 11:28:40 bj Exp $
 *
 *------------------------------------------------------------------------
 */


#include"rl.h"


/* ====================================================================
 * SetUpClipStuff()  -  this function initilizes the signal bit and
 *                      NewLists the list header for the cut and paste
 * ====================================================================
 */

BOOL
SetUpClipStuff(struct glob *g)
{
	SYSBASE ;
	
    g->g_error = ERR_SETUPCLIP ;
	if( (g->g_clipsig = (UBYTE)AllocSignal(-1L)) != -1)
	{
		g->g_clipevent = 1L << g->g_clipsig ;
		NewList(&g->g_paste) ;
		return(TRUE) ;
	}
	return(FALSE) ;
}

/* ====================================================================
 * KillClipStuff()  -  Frees the signal bit. Used when program exits
 * ====================================================================
 */
 
VOID 
KillClipStuff(struct glob *g)
{
	SYSBASE ;

	if(g->g_clipsig)
	{
		FreeSignal((LONG)g->g_clipsig) ;
		g->g_clipsig = -1 ;
	}
}
	
/* ======================================================================
 * MakeClipMine() - this function is called when the program receives the
 *                  left-amiga/v control sequence. This finds the conclip
 *                  port and pulls the clipped text into a linked list.
 *                  It then sets the 'clip_in_progress' flag so the main
 *                  loop of the program can operate with knowledge that
 *                  this is in operation.
 * ======================================================================
 */

#define TYPE_READ   25
#define TYPE_REPLY  27

LONG
MakeClipMine(struct glob *g)
{
	SYSBASE ;
	REGISTER struct MsgPort *pasteport ;
	REGISTER struct MsgPort *conclipport ;
	REGISTER struct MinNode *node ;
	struct CmdMsg cm ;
	LONG retval = 0L ;

	if(pasteport = CreateMsgPort()) ;
	{
		cm.cm_Msg.mn_Node.ln_Type = 0 ;
		cm.cm_Msg.mn_Length       = (LONG)sizeof( struct CmdMsg ) ;
		cm.cm_Msg.mn_ReplyPort    = pasteport ;
		cm.cm_type                = TYPE_READ ;

		Forbid() ;
		if( conclipport = FindPort("ConClip.rendezvous"))
		{
			PutMsg(conclipport,(struct Message *)&cm) ;
			Permit() ;
			WaitPort(pasteport) ;
			if(cm.cm_error == 0 && cm.cm_type == TYPE_REPLY)
			{
				/* pull pastenodes off conclip list & add them to ours */
				node = (struct MinNode *)RemHead((struct List *)&cm.cm_Chunks) ;
				while( node )
				{
					AddTail((struct List *)&g->g_paste,(struct Node *)node) ;
					node = (struct MinNode *)RemHead((struct List *)&cm.cm_Chunks) ;
				}
				g->g_pastenode=(struct MinNode *)RemHead((struct List *)&g->g_paste) ;
				g->g_clip_in_progress = TRUE ;
				retval = 1L ;
			}
		}
		else
		{
			Permit() ;
			g->g_clip_in_progress = 0 ;
		}
		DeleteMsgPort( pasteport ) ;
	}                         
	return( retval ) ;
}

/* =========================================================================
 * GetClipChar()
 *
 * This function is called only by main() when the clipevent signal
 * gets set. 
 *
 * This function gets one character from the linked list of clipped text and
 * returns it. If the character in question is the last one in that node, it 
 * will free the memory and update the list to point to the next one (if any)
 * =========================================================================
 */

LONG
GetClipChar( UBYTE *thechar, struct glob *g )
{
	SYSBASE ;
	UBYTE ch ;
	REGISTER struct MinNode *pn = g->g_pastenode ;

	if( pn )
	{
		ch = ((struct CHRSChunk *)pn)->text[g->g_pcount++] ;
		if( g->g_pcount > ((struct CHRSChunk *)pn)->size )
		{
			if(((struct CHRSChunk *)pn)->freeme)
			{
				FreeVec(((struct CHRSChunk *)pn)->freeme) ;
			}
			FreeVec(pn) ;
			g->g_pastenode = (struct MinNode*)RemHead((struct List *)&g->g_paste) ;
			g->g_pcount = 0L ;
		}
		*thechar = ch ;
		return(0L) ;
	}
	*thechar = 0 ;
	return(1L) ;
}

/* =====================================================================
 * KillPaste( VOID )
 *
 * This function gets called if the user hits control-c during a paste
 * operation (in mainloop()). This frees all remaining memory and sets
 * the flags to 'not doing paste any more'
 * =====================================================================
 */

VOID
KillPaste(struct glob *g)
{
	SYSBASE ;

	while(g->g_pastenode)
	{
		if(((struct CHRSChunk *)g->g_pastenode)->freeme)
		{
			FreeVec(((struct CHRSChunk *)g->g_pastenode)->freeme) ;
		}
		FreeVec(g->g_pastenode) ;
		g->g_pastenode = (struct MinNode*)RemHead((struct List *)&g->g_paste) ;
	}
	g->g_pcount = 0L ;
	g->g_clip_in_progress = 0 ;
}

/* end */
