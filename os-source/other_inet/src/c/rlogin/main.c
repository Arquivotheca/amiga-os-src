/* -----------------------------------------------------------------------
 * main.c   -  rlogin 38 rewrite  This is the main entry point but NOT
 *                                the main loop of the program.
 *
 * $Locker:  $
 *
 * $Id: main.c,v 38.0 93/04/08 15:49:01 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	main.c,v $
 * Revision 38.0  93/04/08  15:49:01  bj
 * This is the complete rewrite of rlogin (v38.xx)
 * This is the main entry point for the program.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/main.c,v 38.0 93/04/08 15:49:01 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/* =====================================================================
 * rl.c     proto rlogin display stuff
 * =====================================================================
 */

#define MAIN 1 

#include "rl.h"

#include "rlogin_rev.h"

UBYTE  *v = VERSTAG ;

int
rl(VOID)  // entry point
{
	SYSBASE ;
	struct IntuitionBase *IntuitionBase ;
	struct Library       *GadToolsBase ;
	struct glob *glob ;
	
	LONG error = ERR_INITGLOB ;

	if(glob = InitGlob())
	{	
		CopyMem(VERSTAG,glob->g_verstag, 60L) ;

		if(GetLibs(glob))
		{
			IntuitionBase = glob->g_IntuitionBase ;
			GadToolsBase  = glob->g_GadToolsBase ;

			if(GetUserPrefs(glob ))
			{
				if(GetWBDisplay(glob))
				{
					if(GetScreen(glob))
					{
						if(GetWindow(glob)) 
						{
							if(glob->g_visualinfo = GetVisualInfoA(glob->g_Window->WScreen,NULL))
							{
								if( SetUpConsole(glob))
								{
									if(InitSocketStuff(glob))
									{
										if(SetUpClipStuff(glob))
										{
											if(SetUpMenus(glob))
											{
												mainloop(glob) ;
												KillMenus(glob) ;
											}
											KillClipStuff(glob) ;
										}
										KillSocketStuff(glob) ;
									}
									KillConsole(glob) ;
								}
								FreeVisualInfo(glob->g_visualinfo) ;
							}
							CloseWindow(glob->g_Window) ;
						}
						CloseScreen(glob->g_Screen ) ; // ok to pass null
					}
				}
			}
			KillLibs(glob) ; // only exec functions callable past here
		}
		error = glob->g_error ; /* save it now! */
		FreeMem(glob,(LONG)sizeof(struct glob)) ;
	}
	return((int)error) ;
}

/* =======================================================================
 * InitGlob() - This returns a "struct glob *" if all is OK, NULL if not.
 *
 * This function initializes all the non-zero values in the global struct
 * =======================================================================
 */

struct glob *
InitGlob(VOID)
{
	SYSBASE ; // cannot use any other libraries here!! Not open yet!

	REGISTER struct ColorSpec *cs ;
	REGISTER int x ;
	REGISTER struct glob *g ;

	if( g = AllocMem((LONG)sizeof(struct glob),MEMF_CLEAR))
	{
		for(cs=(struct ColorSpec *)g->g_colors,x=0 ; x<MAX_COLORS ; x++,cs++)
		{
			cs->ColorIndex = -1 ;
		}

		g->g_myself        = (struct Task *)FindTask(NULL) ;
		g->g_user_resize   = 1L ;  // resize is on
		g->g_user_conunit  = 3L ;  // default to mapped console with clipping
		g->g_80wide        = 640L ;
		g->g_clipsig       = -1 ;
		g->g_user_linewrap = TRUE ;
		g->g_user_jump     = TRUE ;
		CopyMem( "rlamiga/9600", g->g_termtype, MAX_TERMTYPE) ;
		CopyMem( TITLE, g->g_window_title, MAX_TITLE) ;
	}
	return(g) ;
	
}	

/* ====================================================================
 * GetLibs
 *
 * This function MUST return with either everything ok or everything
 * that was passed in left in the same state it was in when the 
 * function was called :
 *
 *     o All library base pointers (in the global structure) MUST be 
 *       either valid or NULL.
 *
 * ====================================================================
 */

#define INAME "intuition.library"
#define UNAME "utility.library"

BOOL
GetLibs(struct glob *g)
{
	SYSBASE ;
	struct IntuitionBase *IntuitionBase ;
	struct GfxBase       *GfxBase ;
	struct DosLibrary    *DOSBase ;
	struct Library       *UtilityBase ;
	struct Library       *GadToolsBase ;
	LONG error = ERR_GETLIBS ;

	if(IntuitionBase = (struct Intuitionbase *)OpenLibrary(INAME, 37L))
	{
		if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37L))
		{
			if(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37L ))
			{
				if(UtilityBase = OpenLibrary(UNAME,36L))
				{
					if(GadToolsBase = OpenLibrary("gadtools.library",37L))
					{    // a-ok. now we can copy the values to global struct.
						g->g_GadToolsBase  = GadToolsBase ;
						g->g_UtilityBase   = UtilityBase ;
						g->g_DOSBase       = DOSBase ;
						g->g_GfxBase       = GfxBase ;
						g->g_IntuitionBase = IntuitionBase ;
						return(TRUE) ;
					}
					CloseLibrary((struct Library *)UtilityBase) ;
				}
				CloseLibrary((struct Library *)DOSBase) ;
			}
			CloseLibrary((struct Library *)GfxBase ) ;
		}
		CloseLibrary((struct Library *)IntuitionBase ) ;
	}
	return(FALSE) ;
}

/* ====================================================================
 * KillLibs()
 *
 * This function closes down all the appliprog libraries
 * ====================================================================
 */

VOID
KillLibs(struct glob *g)
{
	SYSBASE ;

	if(g->g_UtilityBase)
	{
		CloseLibrary(g->g_UtilityBase) ;
	}
	if(g->g_GadToolsBase)
	{
		CloseLibrary(g->g_GadToolsBase) ;
	}
	if(g->g_GfxBase)
	{
		CloseLibrary((struct Library *)g->g_GfxBase) ;
	}
	if(g->g_IntuitionBase)
	{
		CloseLibrary(g->g_IntuitionBase) ;
	}
	if(g->g_DOSBase) 
	{
		CloseLibrary((struct Library *)g->g_DOSBase) ;
	}
}

/* end */
