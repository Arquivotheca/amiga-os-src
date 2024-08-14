/* ==========================================================================
 * console.c  for rlogin   Lattice 5.10
 *
 * $Locker:  $
 *
 * $Id: console.c,v 1.17 91/08/07 15:20:09 bj Exp $
 *
 * $Revision: 1.17 $
 *
 * $Log:	console.c,v $
 * Revision 1.17  91/08/07  15:20:09  bj
 * details.
 * 
 * Revision 1.16  91/08/06  13:26:20  bj
 * Changed to the global variabl structure (glob).
 * Added the About() function (finally!)
 * 
 * Revision 1.14  91/06/19  12:36:47  bj
 * Conclip support. Font bug fixed. Assorted details.
 * This is now 2.0x ONLY!
 * 
 * Revision 1.13  91/04/24  15:36:34  bj
 * Fixes and additions to give rlogin complete PAL mode compatibility
 * under 2.0. Does DisplayInfoData now to see what the text_oscan
 * values are.
 * 
 * Revision 1.12  91/04/12  14:13:56  bj
 * Added code to handle overscanned screens under 2.0. Bug #B13712. 
 * The window on a custom screen opens to the max of the oscan_txt
 * as set by the overscan preferences. User can override with the
 * -h and -w flags out to a max of the screen size. Default is
 * oscan_text.
 *
 * 
 * Revision 1.11  91/02/14  16:16:37  bj
 * Added ability to specify max window width even if on a
 * custom screen. Gives user ability to deal with "display
 * too wide" errors in vi, emacs, etc.
 *  
 * Tightened some code here and there. Details. etc.
 * 
 * Revision 1.10  90/12/06  15:36:45  bj
 * Fixed (for good, hopefully) the problems with calculating
 * the proper window sizes upon opening. This. Thanks to
 * Peter, the SIZEBBOTTOM thing is solved.
 * 
 * 
 * Revision 1.9  90/12/05  16:12:35  bj
 * fixed the value that rlogin sent to the host regarding
 * how many columns it could see. It was reporting one less
 * than was really there.
 * 
 * Revision 1.8  90/12/02  06:00:48  martin
 * fixed my previous fix for computing min win size under 2.0
 * 
 * Revision 1.7  90/12/01  22:00:18  martin
 * fixed bug that caused unpredictable window sizes under 2.0
 * 
 * Revision 1.6  90/11/29  15:11:23  bj
 * backed up to 5.05 to deter some odd crashes upon exit.
 * 
 * Revision 1.5  90/11/29  10:27:13  bj
 * rev 36.8 rlogin.  Fixes bad call to UnlockPubScreen that kept
 * the lock so any suqsequent attepts to resize screens with Prefs
 * editors, etc., would fail.
 * 
 * Revision 1.4  90/11/13  15:27:31  bj
 * no changes. test.
 * 
 * Revision 1.3  90/11/13  15:26:29  bj
 * Headers fixes.
 * 
 *
 *  $Header: HOG:Other/inet/src/c/rlogin/RCS/console.c,v 1.17 91/08/07 15:20:09 bj Exp $
 *
 * ==========================================================================
 */

#define CONSOLE_C 1


/* #define DEBUG 1 */

#ifdef DEBUG
#define DB(x)  KPrintF((x)) ;
#else
#define DB(x) ;
#endif

#include <stdio.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <intuition/screens.h>
#include <graphics/displayinfo.h>
#include <libraries/gadtools.h>
#include <devices/conunit.h>
#include <proto/all.h>
#include <ss/socket.h>

#include "rlglobal.h"

 /* =====================================================================
  * BOTTOM BORDER HEIGHT IS A CONSTANT !!
  *
  * Up thru OS 2.0x the bottom border height of a window with the 
  * SIZEBBOTTOM flag set is calc'd based upon the height of the tallest
  * gadget in the window's bottom border. Same under 1.3.  The 
  * constants for these values are located in the file
  * "intuition/images.h".  The LOWRES height value is '10' while
  * the value for the HIRES is '9' so I am setting the standard
  * define for this as '10 + 1' to minimize the chance that font
  * extenders will hit the border.
  * ====================================================================
  */
  
BYTE wtitle[128] = "Rlogin:" ;

#ifdef FOO
extern struct IOStdReq *CreateStdIO();
extern struct MsgPort  *CreatePort();
extern struct Library  *OpenLibrary();
extern struct Screen   *OpenScreen() ;
extern struct Window   *OpenWindow() ;
#endif

int cons_kybd_sigF;

struct IntuitionBase *IntuitionBase;
struct GfxBase       *GfxBase;
struct Library       *GadToolsBase ;
struct Library       *DiskfontBase ;
struct Library       *SockBase ;

STATIC LONG console_is_open = 0L ;

STATIC struct MsgPort  *crp, *cwp;
STATIC struct IOStdReq *cw,  *cr;
STATIC struct Screen   *scr = NULL , *myscreen = NULL ;

int min_wide = 640 ;     /* to start, we assume font width = 8. Can change!   */
int min_high = 200 ;   

struct ConUnit *conunit ;
struct Window *win;

int showtitle = 0 ;



STATIC struct TextFont *textfont ;    /* we use this to extract the current */
                                      /* workbench screen's font            */

	/* the menus */

extern struct Menu *menu ;
struct TextAttr *menufont_attr ;
struct NewMenu rlnewmenu[] = {
	{ NM_TITLE, (STRPTR)" Project ",0,0,0,0 },
	{   NM_ITEM, (STRPTR)" About ",(STRPTR)"A",0,0,(APTR)"ABOUT" },
	{   NM_ITEM, (STRPTR)" Quit ",(STRPTR)"Q",0,0,(APTR)"QUIT" },
	{ NM_TITLE, (STRPTR)" Paste Timing ",0,0,0,0 },
	{   NM_ITEM, (STRPTR)" 00/00 Seconds ",(STRPTR)"0",CHECKIT|CHECKED,(~1),(APTR)"PT00" },
	{   NM_ITEM, (STRPTR)" 10/50 Seconds ",(STRPTR)"1",CHECKIT,(~2),(APTR)"PT10" },
	{   NM_ITEM, (STRPTR)" 20/50 Seconds ",(STRPTR)"2",CHECKIT,(~4),(APTR)"PT20" },
	{   NM_ITEM, (STRPTR)" 30/50 Seconds ",(STRPTR)"3",CHECKIT,(~8),(APTR)"PT30" },
	{   NM_ITEM, (STRPTR)" 40/50 Seconds ",(STRPTR)"4",CHECKIT,(~16),(APTR)"PT40" },
	{   NM_ITEM, (STRPTR)" 50/50 Seconds ",(STRPTR)"5",CHECKIT,(~32),(APTR)"PT50" },
	{ NM_TITLE, (STRPTR)" Tools ",0,0,0,0 },
	{   NM_ITEM, (STRPTR)" Reset Console ",(STRPTR)"R",CHECKIT,0,(APTR)"RESET" },
	{   NM_ITEM, (STRPTR)" Line Wrap ",(STRPTR)"W",CHECKIT|CHECKED|MENUTOGGLE,0,(APTR)"LWRAP" },
	{   NM_ITEM, (STRPTR)" Jump Scroll ",(STRPTR)"J",CHECKIT|CHECKED|MENUTOGGLE,0,(APTR)"JSC" },
	{ NM_END,0,0,0,0,0 }
	} ;

APTR vi ;   /* visualinfo ptr */
int menu_is_set = 0 ;	
	

/* =============================================================
 * console_char_ready()
 *
 * returns true if a keypress or intuition action has occurred,
 * and returns FALSE if not. Used in main even loop.
 * =============================================================
 */

LONG
console_char_ready( VOID )
{
	return( (CheckIO((struct IORequest *)cr)) ? 1L : 0L ) ;
}

/* ============================================================= 
 * console_getchar()
 *
 * gets keypresses, intuition actions, etc. froom the console
 * device a character at a time. Multi-byte reads are not really
 * required here. While it might speed things up on one end, the
 * fact is that we have to parse the stream one-byte-at-a-time
 * anyway (in the main event loop)
 * =============================================================
 */

BYTE
console_getchar( VOID )
{
	STATIC BYTE c;
	BYTE rtn;

	if(!CheckIO((struct IORequest *)cr))
	{
		WaitIO((struct IORequest *)cr);
	}

	rtn = c;
	cr->io_Command = CMD_READ;
	cr->io_Data    = (APTR)&c;
	cr->io_Length  = sizeof(c);

	SendIO((struct IORequest *)cr);
	return( rtn ) ;
}


/* =============================================================
 * console_init()
 *
 * initializes everything
 *
 * NEEDS TO BE COMPLETELY REDONE !!!
 * =============================================================
 */

LONG
console_init( struct RLGlobal *glob )
{
	unsigned max_w, max_h ;
    struct IBox box;
    int max_text_w ;
    int max_text_h ;
    struct DimensionInfo di ;
    struct Rectangle *rect ;
    struct IBox ibox ;
    ULONG vpmodeID ;

#define DK(x) kprintf(x)

/* DK("doing lib bases\n") ;  */

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 36L) ;
	GfxBase       = (struct GfxBase *)OpenLibrary("graphics.library", 36L) ;
	DiskfontBase  = OpenLibrary("diskfont.library",  36L ) ;
	GadToolsBase  = OpenLibrary("gadtools.library",  36L ) ;
	
	if(!IntuitionBase || !GfxBase || !DiskfontBase || !GadToolsBase ) 
	{
		Write(Output(),"OpenLibrary Failure\n", NULL) ;
		Write(Output(),"This rlogin requires the 2.0 Operating System\n",46L) ;
		return( -1L ) ;
	}

	if( glob->rl_Unit == 100L )
	{
		glob->rl_Unit = 3L ;
	}

	crp = CreatePort(0L, 0) ;
	cwp = CreatePort(0L, 0) ;
	cr  = CreateStdIO(crp) ;
	cw  = CreateStdIO(cwp) ;
	strcat(wtitle, glob->rl_hostptr) ;
	nw.Title = wtitle ;

/*-----------------------------------------------------------------------
 * The way this works needs to be rethought !!!
 *
 * The user specified he wants a screen. Under 2.0 we can use the "Display
 * Database" to tell us about the current state of the user's setup. Most
 * importantly, we can see  what size he has set for the "viewable" area of
 * his monitor (overscan Prefs editor.)  As of 4-12-91 this worked like
 * this:
 *       ( o the custom screen is *always* opened full size. It's the )
 *       (   window that is really 'sized'                            )
 *
 *      1. If the user specifies a screen but does NOT specify a width
 *         and/or a height, the program will open the screen to either
 *         the size of the WB screen or the size of the 'overscan text'
 *         area, whichever is *smaller*.
 *      
 *      2. If the user specifies a screen and DOES specify a width and/or
 *         a height, the screen will be opened to either the size of the
 *         WorkBench screen or the dimensions that the user requested,
 *         whichever is smaller.
 *         
 *      3. The program will not allow you to request a window size smaller
 *         than what is needed for a 80 column by 25 line display (if
 *         indeed the display can even handle that large a display)
 *         ( this is not always true)
 *-----------------------------------------------------------------------
 */

	if( scr = (struct Screen *)LockPubScreen( "Workbench" ))
	{
		if( glob->rl_UseScreen != 1L )
		{
			if(((vi = (APTR)GetVisualInfo( scr, TAG_DONE )) == NULL) ||
			  ((menu = (struct Menu *)CreateMenus(rlnewmenu, TAG_DONE ))==NULL))
			{
				if( vi )
				{
					FreeVisualInfo( vi ) ;
					vi = NULL ;
				}
				UnlockPubScreen( NULL, scr ) ;
				return(-1L) ;
			}
			menufont_attr = scr->Font ;
			
			if( !LayoutMenus( menu, vi, GTMN_TextAttr, menufont_attr, TAG_DONE))
			{
				FreeVisualInfo( vi ) ;
				vi = NULL ;
				UnlockPubScreen( NULL, scr ) ;
				return(-1L) ;
			}
			FreeVisualInfo( vi ) ;
			vi = NULL ;
		}

		max_h       = scr->Height ;
		max_w       = scr->Width ;
		textfont    = GfxBase->DefaultFont ;
		glob->rl_fonthigh    = (LONG)textfont->tf_YSize ;
		glob->rl_fontwide    = (LONG)textfont->tf_XSize ;
		min_wide    = ((80 * glob->rl_fontwide) + scr->WBorLeft + scr->WBorRight) ;
		min_high    = ((24 * glob->rl_fonthigh) + scr->WBorTop + scr->WBorBottom + scr->Font->ta_YSize + 1) ;
		box.Left    = box.Top = 0 ;   
		if((vpmodeID = GetDisplayDims( scr, &box ))) /* workbench display info */
		{
			rect = &di.TxtOScan ;
			box.Width  = max_text_w = (rect->MaxX - rect->MinX + 1) ;
			box.Height = max_text_h = (rect->MaxY - rect->MinY + 1) ;
			box.Left   = (scr->LeftEdge < 0) ? -scr->LeftEdge : 0 ;
			box.Top    = (scr->TopEdge < 0)  ? -scr->TopEdge : 0 ;
		}
		else
		{
			VPrintf("Can't get Display Info Data.\n", NULL) ;
			return( -1L ) ;
		} 
		UnlockPubScreen((BYTE *)NULL,  scr ) ;
		scr = NULL ;
	}
	else
	{
		VPrintf("Can't Lock Workbench screen\n", NULL) ;
		return( -1L ) ;
	}

	if( glob->rl_resize )
	{
		min_high += SIZEGADHEIGHT ;
	}

	if( min_wide > max_w ) 
	{
		min_wide = max_w ;           /* careful! */
	}

	if( min_high > max_h )
	{
		min_high = max_h ;
	}


/*-----------------------------------------------------------------------
 * *** IF IF IF IF IF IF IF *** 
 *
 * ... user specified a screen, Under 2.0 we can use the "Display
 * Database" to tell us about the current state of the user's setup. Most
 * importantly, we can see  what size he has set for the "viewable" area of
 * his monitor (overscan Prefs editor.)  As of 4-12-91 this worked like
 * this:
 *       ( o the custom screen is *always* opened full size. It's the )
 *       (   window that is really 'sized'                            )
 *       (  this needs to be fixed! )
 *
 *      1. If the user specifies a screen but does NOT specify a width
 *         and/or a height, the program will open the screen to either
 *         the size of the WB screen or the size of the 'overscan text'
 *         area, whichever is *smaller*.
 *      
 *      2. If the user specifies a screen and DOES specify a width and/or
 *         a height, the screen will be opened to either the size of the
 *         WorkBench screen or the dimensions that the user requested,
 *         whichever is smaller.
 *         
 *  *** 3. The program will not allow you to request a window size smaller
 *         than what is needed for a 80 column by 25 line display (if
 *         indeed the display can even handle that large a display)
 *
 *  *** The __PV version does allow this !!
 *-----------------------------------------------------------------------
 */         

	if (glob->rl_UseScreen == 1) 
	{
		showtitle = 1 ;
		scrdef.Width  = bbwin.Width  = max_w ;   /* max_w */
		scrdef.Height = bbwin.Height = max_h ;   /* max_h */
		if( vpmodeID & A2024_MONITOR_ID)
		{
			scrdef.ViewModes = LACE|HIRES ;
		}
		else
		{
			scrdef.ViewModes = (max_w>=640 ? HIRES : 0) | (max_h>=300 ? LACE : 0) ;
		}
		scrdef.DefaultTitle = showtitle ? (UBYTE *)wtitle : NULL ;
		if(!(myscreen = OpenScreen((struct NewScreen *)&scrdef)))
		{
			return( -1L ) ;
		}
		else
		{
				/* custom screen display info */
			(VOID)GetDisplayDims( myscreen, &ibox ) ;
			max_text_w  = ibox.Width ;  /* these are the visible display   */
			max_text_h  = ibox.Height ; /* width & height variables        */
			if((vi = (APTR)GetVisualInfo( myscreen, TAG_DONE )) == NULL)
			{
				return(-1L) ;
			}
			menufont_attr = myscreen->Font ;
			
			menu = (struct Menu *)CreateMenus(rlnewmenu, GTMN_FrontPen,1,TAG_DONE ) ;
			if( ! menu )
			{
				return(-1L) ;
			}
			if( !LayoutMenus( menu, vi, GTMN_TextAttr, menufont_attr, TAG_DONE))
			{
				return(-1L) ;
			}
		}


		bbwin.Screen = myscreen ;  

		bbwin.Width  = (max_w > max_text_w) ? max_text_w : max_w ;
		bbwin.Height = (max_h > max_text_h) ? max_text_h : max_h ;
		

		/* this allows the user to specify a max window width even when on
		 * a custom screen.  This is because things like vi and such will
		 * freak if they see a column width that is too large. (2-14-90)
		 */
		
		if( glob->rl_Width )  /* user asked for a width */ 
		{
			if( glob->rl_Width >= max_w )        
			{                               
				bbwin.Width = max_w ;           
			}                               
			else if( glob->rl_Width <= min_wide )
			{                               
				bbwin.Width = min_wide ;        
			}                               
			else
			{                               
				bbwin.Width = glob->rl_Width ;      
			}                               
		}

	
		if( glob->rl_Height ) /* user asked for a height */               
		{     
			/* if __PV and glob->rl_Height ... */	   
			min_high    = ((5 * glob->rl_fonthigh) + myscreen->WBorTop + myscreen->WBorBottom + glob->rl_fonthigh + 1) ;
			if( glob->rl_Height >= max_h )        
			{                               
				bbwin.Height = max_h ;           
			}                               
			else if( glob->rl_Height <= min_high )
			{                               
				bbwin.Height = min_high ;        
			}                               
			else
			{                               
				bbwin.Height = glob->rl_Height ;      
			}       
			if( showtitle && ( bbwin.Height > (myscreen->Height- (myscreen->BarHeight - 6))))                        
			{
				bbwin.Height = myscreen->Height - (myscreen->BarHeight - 6 ) ;
			}
		}


		if( showtitle )
		{
			ShowTitle(myscreen,TRUE) ;
			bbwin.TopEdge = myscreen->BarHeight + 2 ;
			bbwin.Height -= myscreen->BarHeight + 2 ;
		}
		else
		{
			ShowTitle(myscreen, FALSE) ;
		}
		

		win = OpenWindow(&bbwin) ;

	} 
	else      /* else we're in a window on WBscreen */ 
	{         
		nw.Width = min_wide ;   /* default width */

		if( glob->rl_Width ) 
		{                    /* user asked for a width */
			if( glob->rl_Width >= max_w ) 
			{
				glob->rl_Width = max_w ;
			}
			else if( glob->rl_Width <= min_wide ) 
			{
				glob->rl_Width = min_wide ;
			}
			else
			{
				nw.Width = glob->rl_Width ;
			}
		}
		if( glob->rl_Left )    /* user wants a different X position */ 
		{
			if( glob->rl_Left + nw.Width <= max_w ) 
			{
				nw.LeftEdge = glob->rl_Left ;
			}
		}
		nw.Height = min_high ;       /* default height */
		if( glob->rl_Height )      /* user asked for a height */
		{
			if( glob->rl_Height >= max_h ) 
			{
				nw.Height = max_h ;
			}
			else if( glob->rl_Height > 200 ) 
			{
				nw.Height = glob->rl_Height ;
			}
			else if( glob->rl_Height < (3 * glob->rl_fonthigh)) 
			{
				nw.Height = (3 * glob->rl_fonthigh) ;
			}
		}

		if( glob->rl_Top )   /* user wants a different Y position */ 
		{
			if( glob->rl_Top + nw.Height <= max_h ) 
			{
				nw.TopEdge = glob->rl_Top ;
			}
		}

		if( nw.Height > max_h) 
		{
			nw.Height = max_h ;     /* necessary !! */
		}
		if( nw.Width  > max_w ) 
		{
			nw.Width = max_w ;    /* necessary !! */
		}
		
		if(glob->rl_resize) 
		{
			nw.Flags |= ( WINDOWSIZING|SIZEBBOTTOM ) ;
		}

		if( (glob->rl_Unit == 1L) || (glob->rl_Unit == 3L )) 
		{
			nw.Flags |= SIMPLE_REFRESH ;
		}

		win = OpenWindow(&nw);
		
	}

	if(!cr || !cw || !cwp || !crp || !win) 
	{
		return( -1L ) ;
	}
	
	if( myscreen ) 
	{
		(VOID)GetDisplayDims( myscreen, &ibox ) ;
	}
	
 	SetMenuStrip( win, menu ) ;
	menu_is_set = 1 ;
 
	cons_kybd_sigF = 1L << crp->mp_SigBit;
	cw->io_Data    = (APTR)win;
	cw->io_Length  = sizeof(*win);

	if(OpenDevice("console.device", glob->rl_Unit, (struct IORequest *)cw, 0L)) 
	{
		return( -1L ) ;
	}

	conunit = (struct ConUnit *)cw->io_Unit ;

	if( glob->rl_UseScreen != 1L ) 
	{
		WindowLimits( win, (20L * glob->rl_fontwide), (5L * glob->rl_fonthigh), -1L, -1L) ;
	}

	console_is_open = 1L ;
	*cr = *cw;
	cr->io_Message.mn_ReplyPort = crp;
	cr->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	console_getchar();

	return( 0L ) ;
}

/* =============================================================
 * console_write()
 *
 * sends text to the window.
 * =============================================================
 */

LONG
console_write(UBYTE *buf, LONG len)
{
	REGISTER UBYTE *src, *dest ;
	REGISTER LONG x, realcount ;
	REGISTER LONG doflash = 0L  ;
	UBYTE cwarray[88] ;

		/* break buf into small pieces */ 
	while ( len > 80L )       /* strip extra bells */
	{

		for( src = buf, dest = cwarray, doflash = realcount = x = 0L ; x < 80L ; src++, x++ ) 
		{
			*dest = *src ;
			if( *dest != 0x07 ) 
			{
				dest++ ;
				realcount++ ;
			}
			else 
			{
				doflash++ ;
			}
		}
		cw->io_Data    = (APTR)cwarray ;
		cw->io_Length  = realcount ;
		cw->io_Command = CMD_WRITE;
		DoIO((struct IORequest *)cw);
		buf += 80L ;
		len -= 80L ;
	}
	
	if(doflash) 
	{
		FlashBar() ;
	}

	if (len)         /* strip extra bells */
	{

		for( src = buf, dest = cwarray, doflash = realcount = x = 0 ; x < len ; src++, x++ )
		{
			*dest = *src ;
			if( *dest != 0x07 ) 
			{
				dest++ ;
				realcount++ ;
			}
			else 
			{
				doflash++ ;
			}
		}
		cw->io_Data    = (APTR)cwarray ;
		cw->io_Length  = realcount ;
		cw->io_Command = CMD_WRITE ;
		DoIO((struct IORequest *)cw) ;
	}
	
	if(doflash) 
	{
		FlashBar() ;
	}
	return( len ) ;
}

/* =============================================================
 * console_close()
 *
 * this routine shuts down everything - releases resources, etc
 * =============================================================
 */

LONG
console_close( VOID )
{
	if(cw) 
	{
		AbortIO((struct IORequest *)cw);
		WaitIO((struct IORequest *)cw);
	}

	if(cr) 
	{ 
		AbortIO((struct IORequest *)cr);
		WaitIO((struct IORequest *)cr);
	}

	if( menu_is_set )
	{
		ClearMenuStrip( win ) ;
		menu_is_set = 0L ;
	}

	if(console_is_open)
	{
		CloseDevice((struct IORequest *)cw);
		console_is_open = 0L ;
	}

	if(win)
	{
		CloseWindow(win);
		win = NULL;
	}

	if( vi != NULL )
	{
		FreeVisualInfo( vi ) ;
		vi = NULL ;
	}

	if(myscreen)
	{
		CloseScreen(myscreen);
		myscreen = NULL;
	}

	if(cw)
	{
		DeleteStdIO(cw);
		cw = NULL;
	}

	if(cr)
	{
		DeleteStdIO(cr);
		cr = NULL;
	}

	if(cwp)
	{
		DeletePort(cwp);
		cwp = NULL;
	}
    
	if(crp)
	{
		DeletePort(crp);
		crp = NULL;
	}

	if(GadToolsBase)
	{        
		FreeMenus( menu ) ;
		CloseLibrary(GadToolsBase) ;
		GadToolsBase = NULL ;  
	}

	if(DiskfontBase)
	{
		CloseLibrary(DiskfontBase) ;
		DiskfontBase = NULL ; 
	}

	if(IntuitionBase)
	{
		CloseLibrary((struct Library *)IntuitionBase);
		IntuitionBase = NULL;
	}

	if(GfxBase) 
	{
		CloseLibrary((struct Library *)GfxBase);
		GfxBase = NULL ;
	}

	return( 0L ) ;
}

/* ===================================================================
 * This function calculates the window size in pixels as well as the  
 * values for rows and columns as calculated from the FONT being used 
 * Returns the values in place !  You can pass a NULL value in any/all
 * of the 'width, height, rows, cols' arguments and that arg will not
 * be processed.
 * ===================================================================
 */
 
VOID
console_get_window_sizes(int *width, int *height, int *cols, int *rows,  \
                           	LONG fontwidth, LONG fontheight )
{
	if(width)
	{
		*width = win->Width;
	}

	if(height)
	{
		*height = win->Height;
	}

/* #define OLDSTYLE 1 */
#ifdef OLDSTYLE /* <= 36.14 */
	if(cols)
	{
		*cols = (int)((((win->Width) - (win->BorderLeft *2)) / (int)fontwidth) ) ;     
	}

	if(rows)
	{
		*rows = (int)((win->Height) - ((int)(win->BorderTop + win->BorderBottom))) ;
		*rows = *rows / (int)fontheight ;
	}
#else

	if( cols )
	{
		*cols = (int)conunit->cu_XMax + 1 ;
	}
	if( rows )
	{
		*rows = (int)conunit->cu_YMax + 1 ;
	}

#endif
	
}

/* ===================================================================
 * This function calculates the position and size of the screen's 
 * visible area (the area between the bezels. This is what the user sets
 * up with the OverScan Preferences Editor.
 *
 * We use this only under 2.0 !!
 * ===================================================================
 */

ULONG 
GetDisplayDims( struct Screen *scrn, struct IBox *box )
{
	ULONG vpmid ;
	struct DimensionInfo di ;
	struct DisplayInfo displayinfo ;
	REGISTER struct Rectangle *rect ;

    box->Left = box->Top = 0 ;
 	    
    if ((vpmid = GetVPModeID( &scrn->ViewPort )) != INVALID_ID)
    {
        GetDisplayInfoData( 0, (UBYTE *)&di, sizeof(di), DTAG_DIMS, vpmid ) ;
        rect = &di.TxtOScan ;
        box->Width  = rect->MaxX - rect->MinX + 1 ;
        box->Height = rect->MaxY - rect->MinY + 1 ;
        box->Left   = scrn->LeftEdge < 0 ? -scrn->LeftEdge : 0 ;
        box->Top    = scrn->TopEdge < 0 ? -scrn->TopEdge : 0 ;
		GetDisplayInfoData(0, (UBYTE *)&displayinfo, sizeof(displayinfo), DTAG_DISP, vpmid ) ;
    }
    return( vpmid ) ;
}



/****** end *****/
