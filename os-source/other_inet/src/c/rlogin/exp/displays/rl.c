
#define DEBUG 

#ifdef DEBUG
#define D(x) PutStr(x)
#else
#define D(x)  ;
#endif

#ifdef REPORT_PROGRESS
#define P(x) PutStr(x)
#else
#define P(x) ;
#endif

#define MIN_COLS      80L
#define MIN_ROWS      24L
#define MIN_ROWS_LACE 48L


#define TESTTEXT (STRPTR)"12345678901234567890123456789012345678901234567890123456789012345678901234567890" 
#define TT2      (STRPTR)"|        |         |         |         |         |         |         |         |"
#define TT3      (STRPTR)"0       10        20        30        40        50        60        70        80"

#include <exec/types.h>
#include <string.h>
#include "rl_include.h"

#define MEMFLAGS MEMF_PUBLIC|MEMF_CLEAR

	struct ScreenInfo {
		ULONG  si_vpmid ;
		WORD   si_Height ;
		WORD   si_Width ;
		UBYTE  *si_Title ;   /* for pub screen hooks */
		BYTE   si_BarHeight ;
		BYTE   si_WBorTop, si_WBorLeft, si_WBorRight, si_WBorBottom ;
		BYTE   si_Pad1 ;
		LONG   si_FontWide ;
		LONG   si_FontHigh ;
		LONG   si_MinWinWide ;
		LONG   si_MinWinHigh ;
		struct IBox si_ibox ;
		struct TextAttr *si_Font ;
		
		} ;

	struct glob {
		struct DOSLibrary    *g_DOSBase ;     
		struct Library       *g_UtilityBase ; 
		struct Library       *g_DiskFontBase ;
		struct Library       *g_SockBase ;
		struct IntuitionBase *g_IntuitionBase ;
		struct GfxBase       *g_GfxBase ;
		struct Library       *g_SysBase ;
		
		struct Screen        *g_Screen ;
		struct Window        *g_Window ;
		BOOL g_usescreen ;
		struct ScreenInfo g_ScreenInfo ;
		} ;    

                          
    struct TagItem screentags[] = {
		{ SA_Left,0L },   
		{ SA_Top, 0L },   
		{ SA_Title, (long)"Test screen title" },
		{ SA_Type, CUSTOMSCREEN },
		{ SA_Overscan, OSCAN_TEXT }
		} ;               
                          
enum windowtags {  W_WIDTH = 0 , 
                   W_HEIGHT, 
                   W_CUSTOMSCREEN, 
                   W_SIZEGADGET, 
                   W_DEPTHGADGET, 
                   W_DRAGBAR, 
                   W_CLOSEGADGET,
                   W_SIZEBBOTTOM,
                   W_TITLE,
                   } ;

int display( void ) ;    
ULONG getdisplaydims( struct Screen *scrn, struct IBox *box, struct glob * ) ;
char *mysprintf( char *, ...) ;
int foo( int, struct glob * ) ;
void getscreeninfo( struct Screen *s, struct ScreenInfo *si, struct glob *g ) ;
                          

int display(void)
{
	struct DOSLibrary    *DOSBase        = NULL ;
	struct Library       *UtilityBase    = NULL ;
	struct Library       *DiskFontBase   = NULL ;
	struct Library       *SockBase       = NULL ;
	struct IntuitionBase *IntuitionBase  = NULL ;
	struct GfxBase       *GfxBase        = NULL ;
	struct Library       *SysBase        = (*((struct Library **) 4)) ;
	struct Screen        *screen, *wbscr ;
	struct Window        *window ;
	struct RastPort      *rp ;
	
	struct NewScreen ns ;
	long opts[OPT_COUNT] ;
	ULONG vpmid ;
	struct IBox ibox ;
	struct ScreenInfo si, csi, wbsi ;
	char buffer[256] ;
	
	struct glob *glob = AllocVec((long)sizeof(struct glob), MEMFLAGS) ;
	struct TagItem scr_ti[5] ;
	struct TagItem win_ti[20] ;
	
	if( glob )
	{
		glob->g_usescreen = FALSE ;  /****************************/
		memset((char *)opts, '\0', sizeof(opts)) ;
		memset((char *)&ns, '\0', sizeof(struct NewScreen)) ;
		memset((char *)&si, '\0', sizeof(struct ScreenInfo)) ;
		memset((char *)&csi, '\0', sizeof(struct ScreenInfo)) ;
		memset((char *)&wbsi, '\0', sizeof(struct ScreenInfo)) ;
		ns.ViewModes = HIRES | LACE  ;
		
		if(DOSBase = glob->g_DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 36L))
		{
			P("2\n") ;
			if(GfxBase = glob->g_GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 36L))
			{
				P("3\n") ;
				if(IntuitionBase = glob->g_IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 36L))
				{
					P("4\n") ;
					if(wbscr = LockPubScreen( NULL ))
					{
						P("5\n") ;
						D("getscreeninfo: wb screen\n") ;
						getscreeninfo( wbscr, &wbsi, glob) ;
						glob->g_ScreenInfo = wbsi ;
						UnlockPubScreen(NULL,wbscr) ;
						P("6\n") ;
						if(wbsi.si_vpmid)
						{
							if( glob->g_usescreen )
							{
								P("7\n") ;
								scr_ti[0].ti_Tag  = SA_DisplayID ;
								scr_ti[0].ti_Data = (ULONG)wbsi.si_vpmid ;
								scr_ti[1].ti_Tag  = SA_Title ;
								scr_ti[1].ti_Data = (ULONG)"test screen title" ;
								scr_ti[2].ti_Tag =  TAG_DONE ;
								scr_ti[2].ti_Data = 0L ;
								P("8\n") ;
								if(glob->g_Screen = OpenScreenTagList(NULL, &scr_ti[0]))
								{
									D("getscreeninfo: custom screen\n") ;
									getscreeninfo( glob->g_Screen, &csi, glob) ;
									glob->g_ScreenInfo = csi ;
									si = csi ;
								}
								else
								{
									PutStr("couldn't open screen\n") ;
									goto bail ;
								}
							}

							win_ti[W_WIDTH].ti_Tag     = WA_Width ;
							win_ti[W_WIDTH].ti_Data    = si.si_MinWinWide ;
							win_ti[W_HEIGHT].ti_Tag    = WA_Height ;
							win_ti[W_HEIGHT].ti_Data   = si.si_MinWinHigh  ;
							win_ti[W_CUSTOMSCREEN].ti_Tag  = TAG_IGNORE ;
							win_ti[W_CUSTOMSCREEN].ti_Data = 0L ;
							win_ti[W_SIZEGADGET].ti_Tag  = WA_SizeGadget ;
							win_ti[W_SIZEGADGET].ti_Data = (LONG)TRUE ;  
							win_ti[W_DEPTHGADGET].ti_Tag  = WA_DepthGadget ;
							win_ti[W_DEPTHGADGET].ti_Data = (LONG)TRUE ;  
							win_ti[W_DRAGBAR].ti_Tag  = WA_DragBar ;
							win_ti[W_DRAGBAR].ti_Data = (LONG)TRUE ;  
							win_ti[W_CLOSEGADGET].ti_Tag  = WA_CloseGadget ;
							win_ti[W_CLOSEGADGET].ti_Data = (LONG)TRUE ;  
							win_ti[W_SIZEBBOTTOM].ti_Tag  = WA_SizeBBottom ;
							win_ti[W_SIZEBBOTTOM].ti_Data = (LONG)TRUE ;  
							win_ti[W_TITLE].ti_Tag  = WA_Title ;
							win_ti[W_TITLE].ti_Data = (LONG)" -- test window --" ;
							win_ti[9].ti_Tag   = TAG_IGNORE ;
							win_ti[9].ti_Data  = (LONG)0 ;
							win_ti[10].ti_Tag  = TAG_IGNORE ;
							win_ti[10].ti_Data = (LONG)0 ; 
							win_ti[11].ti_Tag  = TAG_IGNORE ;
							win_ti[11].ti_Data = (LONG)0 ;
							win_ti[12].ti_Tag  = TAG_IGNORE ;
							win_ti[12].ti_Data = (LONG)0 ;
							win_ti[13].ti_Tag  = TAG_IGNORE ;
							win_ti[13].ti_Data = (LONG)0 ;
							win_ti[14].ti_Tag  = TAG_IGNORE ;
							win_ti[14].ti_Data = (LONG)0 ;
							win_ti[15].ti_Tag  = TAG_IGNORE ;
							win_ti[15].ti_Data = (LONG)0 ;
							win_ti[16].ti_Tag  = TAG_IGNORE ;
							win_ti[16].ti_Data = (LONG)0 ;

							win_ti[17].ti_Tag  = TAG_END ;
							win_ti[17].ti_Data = 0L ;

							if( glob->g_Screen )
							{
								win_ti[W_CUSTOMSCREEN].ti_Tag = WA_CustomScreen ;
								win_ti[W_CUSTOMSCREEN].ti_Data = (LONG)glob->g_Screen ;
							}
							window = OpenWindowTagList( NULL, &win_ti[0]) ;
							if( window )
							{
								rp = window->RPort ;
								Move(rp, (LONG)si.si_WBorLeft,40L) ;
								Text(rp, TESTTEXT, 80L) ;
								Move(rp, (LONG)si.si_WBorLeft,40L + si.si_FontHigh) ;
								Text(rp, TT2, 80L) ;
								Move(rp, (LONG)si.si_WBorLeft,40L + (2*si.si_FontHigh)) ;
								Text(rp, TT3, 80L) ;
								Delay(300L) ;
								P("9\n") ;
								CloseWindow( window ) ;
							}
							else
							{
								PutStr("couldn't open window\n") ;
							}
							
							CloseScreen(glob->g_Screen) ;
						}
						else
						{
							PutStr("error: couldn't open screen\n") ;
						}
					}
bail:
				P("10\n") ;
					CloseLibrary((struct Library *)IntuitionBase) ;
				}
				P("11\n") ;
				CloseLibrary((struct Library *)GfxBase) ;
			}
			P("12\n") ;
			CloseLibrary((struct Library *)DOSBase) ;
		}
		FreeVec( glob ) ;
	}
	return(0) ;
}



unsigned long
getdisplaydims( struct Screen *scrn, struct IBox *box, struct glob *g )
{
	register struct IntuitionBase *IntuitionBase = g->g_IntuitionBase ;
	register struct GfxBase *GfxBase = g->g_GfxBase ;
    ULONG vpmid = 0L ;
    struct DimensionInfo di ;
    struct DisplayInfo displayinfo ;
    REGISTER struct Rectangle *rect ;
    

    box->Left = box->Top = 0 ;
            
    if ((vpmid = GetVPModeID( &scrn->ViewPort )) != INVALID_ID)
    {
        GetDisplayInfoData( 0, (UBYTE *)&di, (ULONG)sizeof(di), DTAG_DIMS, vpmid ) ;
        rect = &di.TxtOScan ;
        box->Width  = rect->MaxX - rect->MinX + 1 ;
        box->Height = rect->MaxY - rect->MinY + 1 ;
        box->Left   = scrn->LeftEdge < 0 ? -scrn->LeftEdge : 0 ;
        box->Top    = scrn->TopEdge < 0 ? -scrn->TopEdge : 0 ;
        GetDisplayInfoData(0, (UBYTE *)&displayinfo, (ULONG)sizeof(displayinfo), DTAG_DISP, vpmid ) ;
    }
    return( vpmid ) ;
}


/* -----------------------------------------------------------
 *
 *  struct ScreenInfo {
 *      ULONG si_vpmid ;
 *      WORD si_Height ;
 *      WORD si_Width ;
 *      UBYTE *si_Title ;       - for pub screen hooks
 *      BYTE si_BarHeight ;
 *      BYTE si_WBorTop, si_WBorLeft, si_WBorRight, si_WBorBottom ;
 *      struct IBox si_ibox ;
 *      struct TextAttr *si_Font ;
 *      } ;
 * -------------------------------------------------------------
 */


VOID
getscreeninfo( struct Screen *s, struct ScreenInfo *info, struct glob *g )
{
	
	struct DOSLibrary *DOSBase = g->g_DOSBase ;
	register struct ScreenInfo *i = info ;
	
	char buf[256] ;
	
	struct TextFont *textfont = g->g_GfxBase->DefaultFont ;
	P("getscreeninfo.\n") ;
	
	i->si_vpmid      = getdisplaydims( s, &info->si_ibox, g ) ;
	i->si_Height     = s->Height ;
	i->si_Width      = s->Width ;
	i->si_Title      = s->Title ;
	i->si_WBorTop    = s->WBorTop ;
	i->si_WBorLeft   = s->WBorLeft ;
	i->si_WBorRight  = s->WBorRight ;
	i->si_WBorBottom = s->WBorBottom ;
	i->si_BarHeight  = s->BarHeight ;
	i->si_Font       = s->Font ;
	i->si_FontHigh   = (long)textfont->tf_YSize ;
	i->si_FontWide   = (long)textfont->tf_XSize ;
	i->si_MinWinHigh = (long)((MIN_ROWS * i->si_FontHigh) + (i->si_WBorTop + i->si_WBorBottom + i->si_BarHeight)) ; 
	i->si_MinWinWide = (long)((MIN_COLS * i->si_FontWide) + (i->si_WBorLeft + i->si_WBorRight)) ;
	
	D( (mysprintf(buf,"gsi: vpmid = %08lx\n",info->si_vpmid)) ) ;
	D(mysprintf(buf, "h = %ld, w = %ld, top = %ld, lft = %ld rgt = %ld bot = %ld  bh = %ld\n",
		info->si_Height, info->si_Width, info->si_WBorTop, info->si_WBorLeft, info->si_WBorRight,
		info->si_WBorBottom, info->si_BarHeight )) ;
	D(mysprintf(buf, "font = %s\nTitle = %s\n", (char *)info->si_Font->ta_Name, (char *)info->si_Title)) ;
	D(mysprintf(buf,"fontwide = %ld\nfonthigh = %ld\n",info->si_FontWide,info->si_FontHigh)) ; 
	D(mysprintf(buf, "box: w= %ld  h= %ld  left= %ld  top= %ld\n",
		info->si_ibox.Width, info->si_ibox.Height, info->si_ibox.Left, info->si_ibox.Top )) ;
	D(mysprintf(buf,"minwide = %ld\nminhigh = %ld\n", info->si_MinWinWide, info->si_MinWinHigh)) ;
	D("returning from getscreeninfo()\n") ;

}

