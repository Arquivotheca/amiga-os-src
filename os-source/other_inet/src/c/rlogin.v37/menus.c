/* -----------------------------------------------------------------------
 * menus.c    rlogin 38  rewrite
 *
 * These functions handle the menus. Initialization, creation, handling
 * of menu events and the deletion of the menus are all done here.
 *
 * $Locker:  $
 *
 * $Id: menus.c,v 38.0 93/04/08 15:42:37 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	menus.c,v $
 * Revision 38.0  93/04/08  15:42:37  bj
 * This is the complete rewrite of rlogin (v38.xx)
 * These are the menu handling functions.
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/menus.c,v 38.0 93/04/08 15:42:37 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#include "rl.h"

/* =======================================================================
 * SetUpMenus()  Boolean - returns FALSE if anything went wrong, else TRUE
 *
 * This creates the menus. Adjustments are made according to user prefs.
 * At this time the only menu items that can be changed are "jump scroll",
 * "line wrap" and "paste delay". Changing these things is very dependant
 * upon the position of the item in the NewMenu list. Ve VERY CAREFUL here
 * if additions of position cnahges are to be made.
 * =======================================================================
 */

BOOL
SetUpMenus(struct glob *g)
{
	REGISTER INTUITIONBASE(g) ;
	REGISTER GADTOOLSBASE(g) ;
	ULONG delay, tmp ;
	struct NewMenu *nm ;

	struct NewMenu rlnewmenu[] = {
	 { NM_TITLE, (STRPTR)"Project",0,0,0,0 },
	 {   NM_ITEM, (STRPTR)"About",(STRPTR)"A",0,0,(APTR)ABOUT },
	 {   NM_ITEM, (STRPTR)"Quit",(STRPTR)"Q",0,0,(APTR)QUIT },
	 { NM_TITLE, (STRPTR)"Paste Timing",0,0,0,0 },
	 {   NM_ITEM, (STRPTR)"00/00 Seconds",(STRPTR)"0",CHECKIT|CHECKED,(~1),(APTR)PT00 },
	 {   NM_ITEM, (STRPTR)"10/50 Seconds",(STRPTR)"1",CHECKIT,(~2),(APTR)PT10 },
	 {   NM_ITEM, (STRPTR)"20/50 Seconds",(STRPTR)"2",CHECKIT,(~4),(APTR)PT20 },
	 {   NM_ITEM, (STRPTR)"30/50 Seconds",(STRPTR)"3",CHECKIT,(~8),(APTR)PT30 },
	 {   NM_ITEM, (STRPTR)"40/50 Seconds",(STRPTR)"4",CHECKIT,(~16),(APTR)PT40 },
	 {   NM_ITEM, (STRPTR)"50/50 Seconds",(STRPTR)"5",CHECKIT,(~32),(APTR)PT50 },
	 { NM_TITLE, (STRPTR)"Settings",0,0,0,0 },
/* 11 */ {   NM_ITEM, (STRPTR)"Line Wrap",(STRPTR)"W",CHECKIT|MENUTOGGLE,0,(APTR)LWRAP },
/* 12 */ {   NM_ITEM, (STRPTR)"Jump Scroll",(STRPTR)"J",CHECKIT|MENUTOGGLE,0,(APTR)JSC },
	 { NM_TITLE, (STRPTR)"Display",0,0,0,0 },
	 {   NM_ITEM, (STRPTR)"Reset Console",(STRPTR)"R",CHECKIT,0,(APTR)RESET },
	           /*** ALL ITEMS BELOW THIS POINT WILL BE NOT BE SEEN ON CUSTOM SCREENS */
	 {   NM_ITEM, (STRPTR)"24 Rows x 80 Cols",(STRPTR)"M",0,0,(APTR)MINSIZE },
	 {   NM_ITEM, (STRPTR)"80 Columns",(STRPTR)"8",0,0,(APTR)WIDE80 },
	 { NM_END,0,0,0,0,0 }

	} ;

	struct TagItem create_tags[] = {
		{ GTMN_FullMenu, TRUE} ,
		{ GTMN_FrontPen,0L},
		{ TAG_END, 0L  },
		} ;
	
	struct TagItem layout_items[] = {
		{ GTMN_TextAttr,0L},
		{ GTMN_NewLookMenus,TRUE},
		{ GTMN_FrontPen,1L},
		{ TAG_END,0L},
		} ;

	g->g_error = ERR_SETUPMENUS ;

	if( g->g_paste_delay )
	{
		delay = g->g_paste_delay + BASEID ;
		for(nm = rlnewmenu ; nm->nm_Type != NM_END ; nm++)
		{
			
			if(((tmp = (ULONG)nm->nm_UserData) < BASEID) || (tmp > (BASEID + 99)))
			{
				continue ; // not a pastedelay menuitem
			}
			if( tmp == delay)
			{
				nm->nm_Flags |= CHECKED ;
			}
			else
			{
				nm->nm_Flags &= ~CHECKED ;
			}
		}
	}

	if( g->g_user_linewrap )
	{
		rlnewmenu[11].nm_Flags |= CHECKED ;
	}
	if( g->g_user_jump )
	{
		rlnewmenu[12].nm_Flags |= CHECKED ;
	}
	
	if(g->g_Menu = (struct Menu *)CreateMenusA( rlnewmenu, create_tags ))
	{
		layout_items[0].ti_Data = (LONG)g->g_Window->WScreen->Font ;

		if(LayoutMenusA(g->g_Menu,g->g_visualinfo, layout_items))
		{
			SetMenuStrip(g->g_Window, g->g_Menu) ;
			g->g_menu_is_attached = TRUE ;
		}
		else
		{
			FreeMenus(g->g_Menu) ;
			g->g_Menu = NULL ;
		}
	}

	return((BOOL)g->g_Menu) ;
}

/* ======================================================================
 * KillMenus   
 *
 * this function removes the menu from the window.
 * ======================================================================
 */

VOID
KillMenus(struct glob *g)
{
	REGISTER GADTOOLSBASE(g) ;
	REGISTER INTUITIONBASE(g) ;

	if(g->g_Menu)
	{
		if( g->g_menu_is_attached )
		{
			ClearMenuStrip(g->g_Window) ;
			g->g_menu_is_attached = FALSE ;
		}
		FreeMenus(g->g_Menu) ;
	}
}

/* =====================================================================
 * HandleMenus - this function handles menu operations. 
 *
 * All I/O is being done through the console device, so we get streams
 * of characters that hold the 'class', 'code', etc info. This must be
 * be parsed out and converted to real values. We use ReadArgs for this.
 *
 * =====================================================================
 */

#define M_TEMPLATE "AA/A/N,BB/A/N,CC/A/N,DD/M"	
#define OPT_A 0
#define OPT_B 1
#define OPT_C 2
#define OPT_D 3
#define OPT_T 4

#define MAX_CONVERT 20L // the data we need will be in the first 20 
                        // bytes of the buffer!

VOID
HandleMenu( struct glob *g, UBYTE *data )
{
	INTUITIONBASE(g) ;
	DOSBASE(g) ;

	REGISTER LONG   command ;
	REGISTER struct MenuItem *item ;
	REGISTER UWORD  code ;
	UBYTE buffer[MAX_CONVERT + 16L], *src, *dest ;
	REGISTER struct RDArgs *ra, *ra_send ;
	ULONG opts[OPT_T] ;
	int x ;
	UBYTE closegad_on[]  = { '\x9b', '1','1','{','\0' } ;
	UBYTE closegad_off[] = { '\x9b', '1','1','}','\0' } ;
	UBYTE linewrap_on[]  = { '\x9b', '\x3f', '\x37', '\x68' } ;
	UBYTE linewrap_off[] = { '\x9b', '\x3f', '\x37', '\x6c' } ;
  	
	memset((BYTE *)opts,  0,(LONG)sizeof(opts)) ;
	memset((BYTE *)buffer,0,MAX_CONVERT + 4L) ;
	
	src  = data + 2 ; // offset to bypass 'esc 9b' seq in buffer
	dest = buffer ;
	x = 0 ;

		// -----------------------------------------------------------
		// we copy the first 20 bytes from the passed buffer into our
		// own. We convert any ';' characters (the console stream uses
		// the semi-colon as the field delimiter) to a space. This
		// process results in a buffer that readargs() can understand.
		// -----------------------------------------------------------

	while( x++ < MAX_CONVERT )
	{
		*dest = (*src == ';') ? ' ' : *src ;
		src++ ;
		dest++ ;
	}
	*dest++ = '\n' ; // make sure we terminate in a newline+null
	*dest   = '\0' ; // as readargs needs this.

	if(ra_send = AllocDosObject(DOS_RDARGS, NULL))
	{
		ra_send->RDA_Source.CS_Buffer = (UBYTE *)buffer ;
		ra_send->RDA_Source.CS_Length = (LONG)strlen(buffer) ; 
		ra_send->RDA_Source.CS_CurChr = 0L ;
		ra_send->RDA_DAList = NULL ;
		ra_send->RDA_Buffer = NULL ;

		if (ra = ReadArgs(M_TEMPLATE,opts,ra_send))
		{
			code = (UWORD) *(LONG *)opts[OPT_C] ;

			while( code != MENUNULL )
			{
				item = (struct MenuItem *)ItemAddress( g->g_Menu, (LONG)code ) ;
				command = (LONG)(GTMENUITEM_USERDATA( item )) ;

				switch(command)
				{
					case ABOUT   :
									About(g) ;
									break ;
					case QUIT    :
									CLOSEGAD_OFF ;
									if((g->g_getout = AskExit(g)) == 0L )
									{
										CLOSEGAD_ON ;
									}
									break ;
					case PT00    :
					case PT10    :
					case PT20    :
					case PT30    :
					case PT40    :
					case PT50    :
									g->g_paste_delay = (command - BASEID) ;
									break ;
					case RESET   :
									con_write("\xF", 1L, g) ;
									break ;
					case LWRAP   :
									if(g->g_user_linewrap)
									{
										LINEWRAP_OFF ;
										g->g_user_linewrap = FALSE ;
									}
									else
									{
										LINEWRAP_ON ;
										g->g_user_linewrap = TRUE ;
									}
									break ;
					case JSC     :
									g->g_user_jump = g->g_user_jump ? FALSE : TRUE ;
									break ; 
					case MINSIZE :
									SizeWindow(g->g_Window,
									           g->g_minwide - (long)g->g_Window->Width,
									           g->g_minhigh - (long)g->g_Window->Height) ;
									break ;
					case WIDE80  :
									SizeWindow(g->g_Window,
									           g->g_minwide - (long)g->g_Window->Width,0L) ;
									break ;
					default      :
									break ;
				}		
				code = item->NextSelect ;
			}
			FreeArgs(ra) ;
		}
		else
		{	
			PrintFault(IoErr(),"menu") ;
		}
		FreeDosObject(DOS_RDARGS, ra_send) ;
	}
	else
	{
		PrintFault(IoErr(),"menu") ;
	}
}


/* end */