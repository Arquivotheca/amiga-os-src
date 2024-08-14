/* window.c -- Intuition Interface
 * Copyright 1988, I and I Computing, all right reserved
 * May be freely used and distributed.
 */

#include "mycxapp.h"

static BYTE dummy;

#if WINDOW
struct Window *OpenWindowTags(struct NewWindow *,ULONG,...);

struct Screen *LockPubScreen(ULONG);
void UnlockPubScreen(ULONG,struct Screen *);
BOOL SetMenuStrip(struct Window *,struct Menu *);

struct Window   *window = NULL; /* our window */

void            *vi          = NULL;
SHORT           topborder;
struct TextFont *font        = NULL;
struct Gadget   *glist       = NULL;
struct Menu     *menu        = NULL;
BOOL            menuattached = NULL;

                 /* save window positions and dims left,top,width,height  */
static WORD savewindow[ 4 ]={WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT};

static char WindowTitle[255];  /* buffer to hold cooked window title */

BOOL IDCMPRefresh = FALSE;

VOID handleIMsg( msg )
struct IntuiMessage   *msg;
{
   ULONG   class;
   UWORD   code;
   struct  Gadget *gaddress;

   class    = msg->Class;
   code     = msg->Code;
   gaddress = (struct Gadget *) msg->IAddress;

   D1( kprintf("window: handleIMsg() enter\n") );

   GT_ReplyIMsg( (struct Msg *) msg );

   switch ( class )
   {
      case CLOSEWINDOW:
         D1( printf("window: handleIMsg(CLOSEWINDOW)\n") );
         shutdownWindow();
         break;         /* not reached   */
      case REFRESHWINDOW:
         D1( printf("window: handleIMsg(REFRESHWINODW)\n") );
/*
         GT_BeginRefresh( window );
*/
	 IDCMPRefresh = TRUE;
         refreshWindow();
	 IDCMPRefresh = FALSE;
/*
         GT_EndRefresh( window, 1L );
*/
         break;
      case MENUPICK:
         D1( printf("window: handleIMsg(MENUPICK)\n") );
         handleCustomMenu(code);
         break;
      case GADGETUP:
         D1( printf("window: handleIMsg(GADGETUP) GadgetID=%lx\n",gaddress->GadgetID) );
         HandleGadget(gaddress->GadgetID & GADTOOLMASK,code);
         break;
   }
}
VOID setupWindow()
{
   struct Screen *myscreen;

   D1( printf("window: setupWindow() enter\n") );

   if(window)
   {   
      D1( printf("window: setupWindow() WindowToFront()\n") );
      WindowToFront( window );
      return;      /* already setup, nothing to re-init   */
   }

   if( ! (myscreen=LockPubScreen(NULL)))
   {
      D1( printf("window: setupWindow() could not LockPubScreen()\n") );
      return;
   }
   D1( printf("window: setupWindow() LockPubScreen(NULL) = %lx\n",myscreen) );

   topborder=myscreen->WBorTop + (myscreen->Font->ta_YSize +1);
   D1( printf("window: setupWindow() topborder = %ld\n",topborder) );

   if( ! (vi=GetVisualInfo(myscreen,TAG_DONE)))
   {
      D1( printf("window: setupWindow() could not GetVisualInfo()\n") );
      goto EXIT;
   }
   D1( printf("window: setupWindow() GetVisualInfo() = %lx\n",vi) );

   if ( ! (window=getNewWindow()))
   {
      D1( printf("window: setupWindow() could not getNewWindow()\n") );
      goto EXIT;
   }
   D1( printf("window: setupWindow() getNewWindow() = %lx\n",window) );

   iport    = window->UserPort;
   isigflag = 1L << iport->mp_SigBit;

   addGadgets(window,vi);

   setupCustomMenu();
   if(menu)
   {
      if( ! LayoutMenus(menu,vi,TAG_DONE))
      {
         D1( printf("window: setupWindow() could not LayoutMenus()\n") );
      }
   }
   menuattached=SetMenuStrip(window,menu);
   D1( printf("window: setupWindow() SetMenuStrip() = %lx\n",menuattached) );

   refreshWindow();

EXIT:

   if(myscreen)
   {
      D1( printf("window: setupWindow() EXIT: UnlockPubScreen(%lx)\n",myscreen) );
      UnlockPubScreen(NULL,myscreen);
      myscreen=NULL;
   }
}

VOID shutdownWindow()
{
   WORD   *wp;

   D1( printf("window: shutdownWindow() enter\n") );

   if ( ! window )
   {
      D1( printf("window: shutdownWindow() window not open!\n") );
      return;
   }

   if(vi)
   {
      D1( printf("window: setupWindow() EXIT: FreeVisualInfo(%lx)\n",vi) );
      FreeVisualInfo(vi);
      vi=NULL;
   }

   /* save window position   */
   wp    = savewindow;
   *wp++ = window->LeftEdge;
   *wp++ = window->TopEdge;
   *wp++ = window->Width;
   *wp   = window->Height;

   if(menuattached)
   {
      ClearMenuStrip(window);
      menuattached=NULL;
   }
   if(menu)
   {
      FreeMenus(menu);
      menu=NULL;
   }

   D1( printf("window: shutdownWindow() ClosingWindow(%lx)\n",window) );
   CloseWindow(window);
   D1( printf("window: shutdownWindow() after CloseWindow()\n") );
   window   = NULL;
   iport    = NULL;
   isigflag = NULL;

   removeGadgets();
   D1( printf("window: shutdownWindow() after removeGadgets()\n") );
}

struct  Window *getNewWindow()
{
   struct  NewWindow   nw;
   WORD   *wp = savewindow;

   D1( printf("window: getNewWindow() enter\n") );

   sprintf(WindowTitle,"%s: HotKey=%s",COM_TITLE,hotkeybuff);

   nw.LeftEdge    = *wp++;
   nw.TopEdge     = *wp++;
   nw.Width       = *wp++;
   nw.Height      = *wp++;
   nw.DetailPen   = (UBYTE) -1;
   nw.BlockPen    = (UBYTE) -1;
   nw.IDCMPFlags  = IFLAGS;
   nw.Flags       = WFLAGS;
   nw.FirstGadget = NULL;
   nw.CheckMark   = NULL;
   nw.Title       = WindowTitle;
   nw.Screen      = NULL;
   nw.BitMap      = NULL;
   nw.MinWidth    = WINDOW_MIN_WIDTH;
   nw.MinHeight   = WINDOW_MIN_HEIGHT;
   /* work around bug  */
   nw.MaxWidth    = WINDOW_MAX_WIDTH;
   nw.MaxHeight   = WINDOW_MAX_HEIGHT;
   nw.Type        = WBENCHSCREEN;

   D1( printf("window: getNewWindow() before OpenWindowTags()\n") );
   /*return ((struct Window *) OpenWindow(&nw));*/

   return ((struct Window *) OpenWindowTags(&nw,
                             W_INNERHEIGHT,WINDOW_INNERHEIGHT,
                             W_AUTOADJUST,TRUE));
}

int addGadgets(window,vi)
struct Window *window;
void *vi;
{
   struct Gadget *gad=NULL;

   D1( printf("window ($%lx): addGadgets() enter\n",window) );

   /*  Open desired font: */
   if( ! font )
   {
      D1( printf("window: addGadgets() Opening font\n") );
      if (!(font = OpenFont(&mydesiredfont)))
      {
         D1( printf("window: addGadgets() Could not open font\n") );
         return(0);
      }
   }

   gad = CreateContext(&glist);

/*   D1( printf("CreateContext gad=$%lx\n",gad) ); */

   setupCustomGadgets(&gad,vi);

   if(!gad)
   {
      D1( printf("window: addGadgets() error adding gadgets\n") );
      if(glist)
      {
         FreeGadgets(glist);
         glist=0;
      }
      if(font)
      {
         CloseFont(font);
         font=0;
      }
      return(0);
   }

   AddGList(window, glist, ((UWORD) -1), ((UWORD) -1), NULL);
   RefreshGList(window->FirstGadget, window, NULL, ((UWORD) -1));
   GT_RefreshWindow(window,NULL);
   return(1);
}

void removeGadgets()
{
   if(glist)
   {
      D1( kprintf("window: removeGadgets() FreeingGadgets(%lx)\n",glist) );
      FreeGadgets(glist);
      glist=0;
   }

   if (font)
   {
      D1( kprintf("window: removeGadgets() Closing font %lx\n", font) );
      CloseFont(font);
      font=0;
   }
}
/******************************/
/* temporary GadToolKit stubs */
/******************************/
void __stdargs GT_SetGadgetAttrs(struct Gadget *gad, struct Window *win,
    struct Requester *req, ULONG firsttag, ...)

   {
   GT_SetGadgetAttrsA(gad, win, req, (struct TagItem *)&firsttag);
   }

void * __stdargs GetVisualInfo(struct Screen *scr, ULONG firsttag, ...)

   {
   return(GetVisualInfoA(scr, (struct TagItem *)&firsttag));
   }

struct Gadget * __stdargs CreateGadget(ULONG kind, struct Gadget *gad,
    struct NewGadget *ng, ULONG firsttag, ...)

   {
   return(CreateGadgetA(kind, gad, ng, (struct TagItem *)&firsttag));
   }

struct Menu * __stdargs CreateMenus(struct NewMenu *newmenu,
    ULONG firsttag, ...)

   {
   return(CreateMenusA(newmenu, (struct TagItem *)&firsttag));
   }

BOOL __stdargs LayoutMenus(struct Menu *firstmenu, void *vi,
    ULONG firsttag, ...)

   {
   return(LayoutMenusA(firstmenu, vi, (struct TagItem *)&firsttag));
   }

BOOL __stdargs LayoutMenuItems(struct MenuItem *firstitem, void *vi,
    ULONG firsttag, ...)

   {
   return(LayoutMenuItemsA(firstitem, vi, (struct TagItem *)&firsttag));
   }

void __stdargs DrawBevelBox(struct RastPort *rport, WORD left, WORD top,
    WORD width, WORD height, ULONG firsttag, ...)

   {
   DrawBevelBoxA(rport, left, top, width, height,
       (struct TagItem *)&firsttag);
   }
/*********************/
struct Window *OpenWindowTags(nw, tags)

    struct NewWindow *nw;
    ULONG tags;

    {
    struct Window __stdargs *OpenWindowTagList(struct NewWindow *, ...);

    return (OpenWindowTagList(nw, &tags));
    }
#endif /* WINDOW */
