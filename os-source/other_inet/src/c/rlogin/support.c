/* -----------------------------------------------------------------------
 * support.c  -   rlogin 38 rewrite
 *
 *                support functions
 *
 * $Locker:  $
 *
 * $Id: support.c,v 38.0 93/04/08 15:22:20 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	support.c,v $
 * Revision 38.0  93/04/08  15:22:20  bj
 * Initial checkin of the support functions for the rewrite of rlogin.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/support.c,v 38.0 93/04/08 15:22:20 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#include"rl.h"
#include "rlogin_rev.h" // for about()

/* ========================================================================
 * BOOL SetPen()  Boolean - returns FALSE if anything went wrong, else TRUE
 *
 * Sets a user defined set of RGB color values into a particular
 * pen to be used when opening a custom screen. 
 * ========================================================================
 */

BOOL 
SetPen( UWORD index, UBYTE *rgb, struct glob *g )
{
	REGISTER DOSBASE(g) ;
	REGISTER struct ColorSpec *cs = &g->g_colors[0] ;
	REGISTER UBYTE red,green,blue, *p = rgb ;
	REGISTER int x ;
	
	if( (index < 0 || index > MAX_COLORS) || ((strlen(rgb) != 3) ))
	{
		PutStr("bad rgb spec\n") ;
		return(FALSE) ;
	}
	
	red   = (UBYTE )toupper( *p ) ;
	p++ ;
	green = (UBYTE )toupper( *p ) ;
	p++ ;
	blue  = (UBYTE )toupper( *p ) ;

	if( my_ishex(red) && my_ishex(green) && my_ishex(blue))
	{
		red   -= ( red   >= 'A' && red   <= 'F' ) ? 55 : 48 ;
		green -= ( green >= 'A' && green <= 'F' ) ? 55 : 48 ;
		blue  -= ( blue  >= 'A' && blue  <= 'F' ) ? 55 : 48 ;
		
		for( x = 0 ; x < MAX_COLORS ; x++, cs++ )
		{
			if( cs->ColorIndex == -1 )
			{
				cs->ColorIndex  = (WORD)index ;
				cs->Red         = (UWORD)red ;
				cs->Green       = (UWORD)green ;
				cs->Blue        = (UWORD)blue ;
				g->g_use_colors = TRUE ;
				return(TRUE) ;
			}
		}
	}
	return(FALSE) ;
}
	
	
/* ===================================================================
 * GetWBDisplay()
 *
 * fills in the global structure with needed info about the current 
 * WB screen.
 * ===================================================================
 */

BOOL 
GetWBDisplay(struct glob *g )
{
	REGISTER GFXBASE(g) ;
	REGISTER INTUITIONBASE(g) ;
	REGISTER UTILITYBASE(g) ;
	REGISTER struct Screen *wbscr ;
	REGISTER struct WBDisplay *wbd = &g->g_WBDisplay ;

	g->g_error      = ERR_GETWBDISPLAY ;
	g->g_fontheight = GfxBase->DefaultFont->tf_YSize ;
	g->g_fontwidth  = GfxBase->DefaultFont->tf_XSize ;
	
	if(wbscr = (struct Screen *)LockPubScreen("Workbench"))
	{
		g->g_modeid                  = (ULONG)GetVPModeID(&wbscr->ViewPort) ;
		g->g_WBDisplay.cd_Depth      = (long)wbscr->RastPort.BitMap->Depth ;

/////////////////////////////////////////////////////////////////////////////
// these commented out lines are not used as of this time but could be
// useful in the future.
//
//		wbd->cd_Height               = wbscr->Height ;
//		wbd->cd_Width                = wbscr->Width ;
//		wbd->cd_left_border_width    = (ULONG)wbscr->WBorLeft ;
//		wbd->cd_right_border_width   = (ULONG)wbscr->WBorRight ;
//		wbd->cd_top_border_height    = (ULONG)wbscr->WBorTop ;
//		wbd->cd_bottom_border_height = (ULONG)wbscr->WBorBottom ;
/////////////////////////////////////////////////////////////////////////////
		wbd->cd_titlebar_height      = (LONG)wbscr->WBorTop + (LONG)g->g_fontheight + 1L ;
		wbd->cd_vert_borders_total   = (LONG)(wbd->cd_titlebar_height + wbscr->WBorBottom) ;
		wbd->cd_horiz_borders_total  = (LONG)(wbscr->WBorLeft + wbscr->WBorRight) ;

		g->g_80wide = UMult32(80L, g->g_fontwidth ) ; // used by WA_InnerWidth
		g->g_24high = UMult32(24L, g->g_fontheight) ; // used by WA_InnerHeight
		
		UnlockPubScreen( NULL, wbscr ) ;
		return(TRUE) ;
	}
	return(FALSE) ;
}		

/* ==================================================================
 * GetUserModeID()
 *
 * translates the user's request for a particular type of custom 
 * screen into a mode ID for the openscreen call.
 * ==================================================================
 */

ULONG
GetUserModeID(struct glob *g)
{
    REGISTER UTILITYBASE(g) ;
	REGISTER ULONG oldmodeid = g->g_modeid ;
    REGISTER struct display_mode *dm ;
	
	struct display_mode dmode[] = {
		{ "HIRES", HIRES_KEY },
		{ "LORES", LORES_KEY },
		{ "HIRESLACE", HIRESLACE_KEY },
		{ "LORESLACE", LORESLACE_KEY },
		{ "SUPER", SUPER_KEY },
		{ "SUPERLACE", SUPERLACE_KEY },
		{  NULL, 0L },
		} ;
	
    dm = &dmode[0] ;

	if( *(g->g_user_modeid) )
	{
		while( dm->dm_option != NULL )
		{
			if( Stricmp( dm->dm_option, g->g_user_modeid ) == 0L)
			{
				return(dm->dm_mode) ;
			}
			dm++ ;
		}
	}

	return(oldmodeid) ;
}

/* =================================================================
 * GetWindow()
 *
 * Creates a window on either the wb screen or a custom screen
 *
 * Be very thoughtful regarding any changes to the TagItem list here
 * as everything is very interdependant.
 * =================================================================
 */


BOOL 
GetWindow(struct glob *g )
{
	SYSBASE ;
	REGISTER INTUITIONBASE(g) ;
	REGISTER struct glob *gl = g ;
	
	/* these tagitem assume that we are opening on */
	/* the workbench screen                        */

	struct TagItem wt[] = {
		{ WA_NewLookMenus, TRUE }, // 0     constant
		{ WA_Title,0L},            // 1     constant
		{ WA_MinWidth, 50L },      // 2     constant
		{ WA_MinHeight, 40L },     // 3     constant
		{ WA_MaxWidth, -1L },      // 4     constant
		{ WA_MaxHeight, -1L},      // 5     constant
		{ WA_Activate,TRUE} ,      // 6     constant
		{ WA_CloseGadget, TRUE },  // 7     constant
		{ WA_DepthGadget, TRUE },  // 8     constant
		{ WA_DragBar, TRUE },      // 9     constant
		{ WA_Left, 0L },           // 10    adjustable
		{ WA_Top,  0L },           // 11    adjustable
		{ WA_InnerHeight, 200L },  // 12    adjustable
		{ WA_InnerWidth, 600L },   // 13    adjustable
		{ WA_SizeGadget, TRUE },   // 14    boolean re resize
		{ WA_SizeBBottom,TRUE},    // 15    boolean re resize
			// here begin changeables - default = wb screen //
		{ WA_AutoAdjust,TRUE},     // 16
		{ WA_SimpleRefresh,TRUE},  // 17
		{ TAG_IGNORE, 0L},         // 18  custom screen
		{ TAG_END, 1L },           // 19
		} ;
		
	g->g_error = ERR_GETWINDOW ;

	mysprintf(g->g_buffer, "Rlogin:%s", g->g_host) ; 
	CopyMem(g->g_buffer, g->g_window_title, MAX_TITLE) ;
	wt[ 1].ti_Data = (LONG)g->g_window_title ; // title
	wt[10].ti_Data = (LONG)g->g_user_left ;    // default = 0
	wt[11].ti_Data = (LONG)g->g_user_top  ;    // default = 0
	wt[12].ti_Data = (LONG)g->g_24high    ;    // 24 cols
	wt[13].ti_Data = (LONG)g->g_80wide    ;    // 80 cols

	if(gl->g_Screen)
	{
		wt[18].ti_Tag  = WA_CustomScreen ;
		wt[18].ti_Data = (LONG)g->g_Screen ;

		if(!gl->g_minsize)
		{
			wt[12].ti_Tag  = WA_Height ;
			wt[13].ti_Tag  = WA_Width ;
			if( gl->g_user_height )
			{
				wt[12].ti_Data = gl->g_user_height ;
			}
			else
			{
				wt[12].ti_Data = (LONG)gl->g_Screen->Height ;
			}

			if( gl->g_user_width )
			{
				wt[13].ti_Data = gl->g_user_width ;
			}
			else
			{
				wt[13].ti_Data = (LONG)gl->g_Screen->Width ;
			}
		}
	}
	else  // we are on the workbench screen
	{
		if(gl->g_user_height)
		{
			wt[12].ti_Tag  = WA_Height ;
			wt[12].ti_Data = (LONG)gl->g_user_height ;
		}
		else
		{
			wt[12].ti_Data = (LONG)gl->g_24high ;
		}

		if(gl->g_user_width)
		{
			wt[13].ti_Tag  = WA_Width ;
			wt[13].ti_Data = (LONG)gl->g_user_width ;
		}
		else
		{
			wt[13].ti_Data = (LONG)gl->g_80wide ;
		}
	}

		// turn off size gadget
		
	if(g->g_user_resize == 0L )
	{
		wt[14].ti_Data = wt[15].ti_Data = FALSE ;
	}

	if(g->g_user_conunit == 0L)
	{
		wt[16].ti_Tag = TAG_IGNORE ;
	}
	gl->g_Window = (struct Window *)OpenWindowTagList(NULL, /*wintags_wb*/ wt ) ;
	if(g->g_Window)
	{
		g->g_minhigh = g->g_24high + g->g_Window->BorderTop + g->g_Window->BorderBottom ;
		g->g_minwide = g->g_80wide + g->g_Window->BorderLeft + g->g_Window->BorderRight ;
	}
	return(gl->g_Window ? TRUE : FALSE ) ;
}

/* =================================================================
 * GetScreen()
 *
 * Creates a custom screen based upon the info supplied in the
 * global structure 'g'.
 *
 * this function returns FALSE _ONLY_ if (1)a custom screen was 
 * actually asked for and (2) this function failed to provide one. 
 * Else, we return TRUE to facilitate flow in the file _rl.c_.
 *
 * Be very thoughtful regarding any changes to the TagItem list here
 * as everything is very interdependant.
 * =================================================================
 */

BOOL
GetScreen( struct glob *g )
{
	INTUITIONBASE(g) ;
	UWORD pen_array = ~0 ;

	struct TagItem scrtags[] = {
		{ SA_LikeWorkbench, TRUE },      // 0
		{ SA_DisplayID, 0L },            // 1
		{ SA_Overscan, OSCAN_TEXT} ,     // 2   overscan
		{ SA_Title, 0L },                // 3
		{ SA_Depth, 3L },                // 4   orig value gets changed!
		{ TAG_IGNORE, 0L},               // 5   width
		{ TAG_IGNORE, 0L},               // 6   height
		{ SA_Interleaved,TRUE },         // 7
		{ SA_MinimizeISG, TRUE},         // 8
		{ SA_Pens, 0L },                 // 9
		{ TAG_IGNORE, 0L },              // 10   colors
		{ TAG_DONE, 1L },                // 11
		} ;
	
		
	g->g_error = ERR_GETSCREEN ;

	if(g->g_use_screen == 0L )
	{
		return(TRUE) ;
	}
	
#ifdef SCREENSIZING
	if( g->g_user_width )
	{
		scrtags[5].ti_Tag  = SA_Width ;
		scrtags[5].ti_Data = g->g_user_width ;
	}
	if( g->g_user_height )
	{
		scrtags[6].ti_Tag  = SA_Height ;
		scrtags[6].ti_Data = g->g_user_height ;
	}
#endif

	scrtags[1].ti_Data = g->g_modeid = GetUserModeID(g) ;

	scrtags[3].ti_Data = (LONG)(g->g_window_title) ;

	if(g->g_modeid == A2024TENHERTZ_KEY || g->g_modeid ==  A2024FIFTEENHERTZ_KEY)
	{
		scrtags[0].ti_Data = FALSE ;
	}
	if( g->g_user_depth )
	{
		g->g_depth = g->g_user_depth ;
	}
	else
	{
		if( g->g_WBDisplay.cd_Depth > 3 )
		{
			g->g_depth = 3L ;
		}
		else
		{
			g->g_depth = (long)g->g_WBDisplay.cd_Depth ;
		}
	}
	scrtags[4].ti_Data = g->g_depth ; ;

	if( g->g_use_colors )
	{
		scrtags[10].ti_Tag = SA_Colors ;
		scrtags[10].ti_Data = (LONG)&g->g_colors[0] ;
	}
	scrtags[9].ti_Data = (LONG)&pen_array ;

	return((g->g_Screen = (struct Screen *)OpenScreenTagList(NULL,scrtags)) ? TRUE : FALSE ) ;
}


/* ==========================================================================
 * handle_oob()  handle OutOfBand messages and data
 *
 * special stuff from the host.
 * ==========================================================================
 */

VOID
handle_oob( struct glob *g )
{
	SOCKBASE(g) ;
	BYTE c ;
	BYTE waste[1024] ;

	while(recv(g->g_socket, &c, sizeof(c), MSG_OOB) < 0) // find the mark
	{ 
		if (g->g_errno != EWOULDBLOCK) 
		{
			return ;
		}
		recv(g->g_socket,waste,sizeof(waste),0) ; // suck down a chunk
	}

	if(c & TIOCPKT_WINDOW)     // the host has asked us for our window size
	{
		send_window_size( g ) ;
	}

	if(c & TIOCPKT_FLUSHWRITE)   // suck down everything up to the mark
	{
		for (;;) 
		{
			LONG atmark ;
			int n ;

			atmark = 0; 
			if (s_ioctl(g->g_socket, SIOCATMARK, (BYTE *)&atmark) < 0) 
			{
				break ;
			}
			if (atmark)
			{
				break ;
			}

			if((n = recv(g->g_socket, waste, sizeof(waste),0)) <= 0)
			{
				break ;
			}
		}
	}
	
////////////////////////////////////////////////////////////////////////
#ifdef IMPLEMENTED /// these options are not implemented ///////////////

	if(c & TIOCPKT_NOSTOP)
	{
		DB("oob: nostop\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_DOSTOP)
	{
		DB("oob: dostop\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_FLUSHREAD)
	{
		DB("oob: flushread\n") ;
		/* Not implemented */
	}
	if(c & TIOCPKT_STOP)
	{
		DB("oob: stop\n") ;
		/* Not implemented */
	}

	if(c & TIOCPKT_START)
	{
		DB("oob: start\n") ;
		/* Not implemented */
	}

#endif // implemented /////////////////////////////////////////
///////////////////////////////////////////////////////////////
}

/* ==========================================================================
 * send_window_size()
 *
 * When we first rlogin to many U*ix systems the host asks about the size of
 * your window. This is the routine that calculates that and sends it to the
 * host via the socket.  We also do this whenever user resizes the window.
 * ==========================================================================
 */

VOID 
send_window_size( struct glob *g )
{
	SOCKBASE(g) ;

	struct {
		BYTE  ws_magic[4] ;         /* magic escape sequence */
		UWORD ws_row, ws_col ;      /* screen size in chars  */
		UWORD ws_width, ws_height ; /* screen size in pixels */
	} ws;

	int rows, cols, width, height ;

	get_window_sizes(&width, &height, &cols, &rows, g) ;

	ws.ws_magic[0] = 0377 ;
	ws.ws_magic[1] = 0377 ;
	ws.ws_magic[2] = 's' ;
	ws.ws_magic[3] = 's' ;
	ws.ws_row      = htons(rows) ;
	ws.ws_col      = htons(cols) ;
	ws.ws_width    = htons(width) ;
	ws.ws_height   = htons(height) ;
	send(g->g_socket, (BYTE *)&ws, sizeof(ws), 0) ;
}



/* ===================================================================
 * This function calculates the window size in pixels as well as the  
 * values for rows and columns as contained in the ConUnit structure.
 * Returns the values in place !  You can pass a NULL value in any/all
 * of the 'width, height, rows, cols' arguments and that arg will not
 * be processed.
 * ===================================================================
 */
 
VOID
get_window_sizes(int *width, int *height, int *cols, int *rows, struct glob *g)
{
	REGISTER struct Window *win = g->g_Window ;

	if(width)
	{
		*width = (int)(win->Width - (g->g_WBDisplay.cd_horiz_borders_total)) ;
	}

	if(height)
	{
		*height = (int)(win->Height - (g->g_WBDisplay.cd_vert_borders_total)) ;
	}

	if( cols )
	{
		*cols = (int)g->g_ConUnit->cu_XMax + 1 ;
	}
	if( rows )
	{
		*rows = (int)g->g_ConUnit->cu_YMax + 1 ;
	}
}

/* =========================================================================
 * About
 * =========================================================================
 */

VOID
About(struct glob *g)
{
	INTUITIONBASE(g) ;

	struct ReqArgs args ;
	UBYTE *v = g->g_verstag ;

	struct EasyStruct es = {
		(LONG)sizeof(struct EasyStruct),
		0L,
		(UBYTE *)"  About",
		(UBYTE *)"%s",
		(UBYTE *)"  Continue  "
		} ;

	args.arg1 = (LONG)( v + 8) ; // offset into version string to bypass
	args.arg2 = 0L ;             //   initial null and 'ver' chars

    EasyRequestArgs( g->g_Window, &es, NULL, (APTR)&args) ;
}


/* =========================================================================
 * AskExit
 * =========================================================================
 */


LONG
AskExit(struct glob *g)
{
	INTUITIONBASE(g) ;
	struct ReqArgs args = { 0L,0L,0L,0L,0L } ;

	struct EasyStruct askes = {
		(LONG)sizeof(struct EasyStruct),
		0L,
		(UBYTE *)"  Rlogin",
		(UBYTE *)"%s\nAre You Sure You Want To Quit?",
		(UBYTE *)"  Yes  |  No  "
		} ;

	args.arg1 = (LONG)g->g_window_title ;
   
	return(EasyRequestArgs( g->g_Window, &askes, NULL,(APTR)&args)) ;
}
