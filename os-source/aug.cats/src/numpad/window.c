
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

/* window.c -- Intuition Interface */

#include "local.h"

static BYTE dummy;


#if WINDOW  /*##### All the following is disabled if the commodity #####*/
            /*##### does not support a window.                     #####*/

struct Window   *window = NULL; /* our window */

void            *vi          = NULL;
struct Screen   *myscreen    = NULL;
SHORT           topborder;
struct TextFont *font        = NULL;
struct Gadget   *glist       = NULL;
struct Menu     *menu        = NULL;
BOOL            menuattached = NULL;
struct DrawInfo *mydi        = NULL;
BOOL            IDCMPRefresh = NULL;

                 /* save window positions and dims left,top,width,height  */
static WORD savewindow[ 4 ]={WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT};

static char WindowTitle[255];  /* buffer to hold cooked window title */

/****i* Blank.ld/handleIMsg() ******************************************
*
*   NAME
*        handleIMsg -- Handle window IDCMP messages.
*
*   SYNOPSIS
*        handleIMsg(msg);
*
*        VOID handleIMsg(struct IntuiMessage *msg);
*
*   FUNCTION
*        This function handles all the IntuiMessages for standard
*        commodities functions. If the message is for an application
*        Gadget or Menu the appropriate application function,
*        handleCustomMenu() or HandleGadget(), is called.
*
*   INPUTS
*        msg = The current IntuiMessage.
*
*   RESULT
*        The appropriate action for the message is performed.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID handleIMsg(struct IntuiMessage *msg)
{
   ULONG   class;
   UWORD   code;
   struct  Gadget *gaddress;

   class    = msg->Class;
   code     = msg->Code;
   gaddress = (struct Gadget *) msg->IAddress;

   D1( kprintf("window.c: handleIMsg() enter\n") );

   GT_ReplyIMsg( (struct Msg *) msg );

   switch ( class )
   {
      case CLOSEWINDOW:
         D1( printf("window.c: handleIMsg(CLOSEWINDOW)\n") );
         shutdownWindow();
         break;         /* not reached   */
      case REFRESHWINDOW:
         D1( printf("window.c: handleIMsg(REFRESHWINODW)\n") );
         IDCMPRefresh=TRUE;
         refreshWindow();
         IDCMPRefresh=FALSE;
         break;
      case MENUPICK:
         D1( printf("window.c: handleIMsg(MENUPICK)\n") );
         handleCustomMenu(code);
         break;
      case GADGETUP:
         D1( printf("window.c: handleIMsg(GADGETUP) GadgetID=%lx\n",gaddress->GadgetID) );
         HandleGadget(gaddress->GadgetID & GADTOOLMASK,code);
         break;
   }
}
/****i* Blank.ld/setupWindow() ******************************************
*
*   NAME
*        setupWindow -- Perform whatever steps necessary to make the
*                       window visible.
*
*   SYNOPSIS
*        setupWindow()
*
*        VOID setupWindow(VOID);
*
*   FUNCTION
*        This function is used to make the window visible. If the window
*        is not opened this function will open it. If the window is already
*        open it will be brought to the front so it is visible. This
*        routine handles all the ugliness of new look and changing window
*        title bar font heights.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID setupWindow()
{
   D1( printf("window.c: setupWindow() enter\n") );

   if(window)
   {   
      D1( printf("window.c: setupWindow() WindowToFront()\n") );
      WindowToFront( window );
      return;      /* already setup, nothing to re-init   */
   }
   if( ! myscreen)
   {
      if( ! (myscreen=LockPubScreen(NULL)))
      {
         D1( printf("window.c: setupWindow() could not LockPubScreen()\n") );
         return;
      }
   }
   D1( printf("window.c: setupWindow() LockPubScreen(NULL) = %lx\n",myscreen) );

   if( ! mydi)
   {
      if( ! (mydi=GetScreenDrawInfo(myscreen)))
      {
         D1( printf("window.c: setupWindow() could not GetScreenDrawInfo()\n") );
         return;
      }
   }
   D1( printf("window.c: setupWindow() GetScreenDrawInfo(0x%lx) = %lx\n",myscreen,mydi) );

   topborder=myscreen->WBorTop + (myscreen->Font->ta_YSize +1);
   D1( printf("window.c: setupWindow() topborder = %ld\n",topborder) );

   if( ! vi)
   {
      if( ! (vi=GetVisualInfo(myscreen,TAG_DONE)))
      {
         D1( printf("window.c: setupWindow() could not GetVisualInfo()\n") );
         goto EXIT;
      }
   }
   D1( printf("window.c: setupWindow() GetVisualInfo() = %lx\n",vi) );

   if ( ! (window=getNewWindow()))
   {
      D1( printf("window.c: setupWindow() could not getNewWindow()\n") );
      goto EXIT;
   }
   D1( printf("window.c: setupWindow() getNewWindow() = %lx\n",window) );

   iport    = window->UserPort;
   isigflag = 1L << iport->mp_SigBit;

   addGadgets(window);

   setupCustomMenu();
   if(menu)
   {
      if( ! LayoutMenus(menu,vi,TAG_DONE))
      {
         D1( printf("window.c: setupWindow() could not LayoutMenus()\n") );
      }
   }
   menuattached=SetMenuStrip(window,menu);
   D1( printf("window.c: setupWindow() SetMenuStrip() = %lx\n",menuattached) );

   refreshWindow();

EXIT:
}
/****i* Blank.ld/shutdownWindow() ******************************************
*
*   NAME
*        shutdownWindow -- Perform the steps necessary to close the window.
*
*   SYNOPSIS
*        shutdownWindow()
*
*        VOID shutdownWindow(VOID);
*
*   FUNCTION
*        Closes the window and remembers its position for the next open.
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID shutdownWindow()
{
   WORD   *wp;

   D1( printf("window.c: shutdownWindow() enter\n") );

   if ( ! window )
   {
      D1( printf("window.c: shutdownWindow() window not open!\n") );
      return;
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

   D1( printf("window.c: shutdownWindow() ClosingWindow(%lx)\n",window) );
   CloseWindow(window);
   D1( printf("window.c: shutdownWindow() after CloseWindow()\n") );
   window   = NULL;
   iport    = NULL;
   isigflag = NULL;

   removeGadgets();
   D1( printf("window.c: shutdownWindow() after removeGadgets()\n") );

   if(vi)
   {
      D1( printf("window.c: shutdownWindow() FreeVisualInfo(%lx)\n",vi) );
      FreeVisualInfo(vi);
      vi=NULL;
   }
   if(mydi)
   {
      D1( printf("window.c: shutdownWindow() FreeScreenDrawInfo(%lx)\n",mydi) );
      FreeScreenDrawInfo(myscreen,mydi);
      mydi=NULL;
   }
   if(myscreen)
   {
      D1( printf("window.c: shutdownWindow() UnlockPubScreen(%lx)\n",myscreen) );
      UnlockPubScreen(NULL,myscreen);
      myscreen=NULL;
   }
}

/****i* Blank.ld/getNewWindow() ******************************************
*
*   NAME
*        getNewWindow -- Open the window remembering the old postition
*                        if reopening.
*
*   SYNOPSIS
*        window = getNewWindow();
*
*        struct Window *getNewWindow(VOID);
*
*   FUNCTION
*        This function opens the commodities window. It automatically
*        sets the window title to reflect the current HotKey. If the
*        window has already been opened once then the previous position
*        and size (if sizing is enabled) are used for this open.
*
*   INPUTS
*        None.
*
*   RESULT
*        Returns a pointer to the opened window or NULL on error.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*        setupWindow();
*        shutdownWindow();
*
*****************************************************************************
*
*/
struct  Window *getNewWindow()
{
   struct  NewWindow   nw;
   WORD   *wp = savewindow;

   D1( printf("window.c: getNewWindow() enter\n") );

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

   D1( printf("window.c: getNewWindow() before OpenWindowTags()\n") );

   return ((struct Window *) OpenWindowTags(&nw,
                             WA_InnerHeight,WINDOW_INNERHEIGHT,
                             WA_AutoAdjust,TRUE,WA_PubScreen,myscreen,TAG_DONE));
}
/****i* Blank.ld/addGadgets() ******************************************
*
*   NAME
*        addGadgets -- Add all the standard and application specific
*                      gadgets to the window.
*
*   SYNOPSIS
*        result = addGadgets(window);
*
*        int addGadgets(struct Window *window);
*
*   FUNCTION
*        Sets up the environment for gadget toolkit gadgets and calls
*        addCustomGadgets() to add the application specific gadgets
*        to the window.
*
*   INPUTS
*        window = Pointer to the window.
*
*   RESULT
*        Returns TRUE if all went well, FALSE otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*        setupCustomGadgets();
*        removeGadgets();
*
*****************************************************************************
*
*/
int addGadgets(struct Window *window)
{
   struct Gadget *gad;

   D1( printf("window.c: addGadgets() enter\n") );

   /*  Open desired font: */
   if( ! font )
   {
      D1( printf("window.c: addGadgets() Opening font\n") );
      if (!(font = OpenFont(&mydesiredfont)))
      {
         D1( printf("window.c: addGadgets() Could not open font\n") );
         return(FALSE);
      }
   }

   gad = CreateContext(&glist);

   setupCustomGadgets(&gad);

   if(!gad)
   {
      D1( printf("window.c: addGadgets() error adding gadgets\n") );
      if(glist)
      {
         FreeGadgets(glist);
         glist=NULL;
      }
      if(font)
      {
         CloseFont(font);
         font=NULL;
      }
      return(FALSE);
   }

   AddGList(window, glist, ((UWORD) -1), ((UWORD) -1), NULL);
   RefreshGList(window->FirstGadget, window, NULL, ((UWORD) -1));
   GT_RefreshWindow(window,NULL);
   return(TRUE);
}
/****i* Blank.ld/removeGadgets() ******************************************
*
*   NAME
*        removeGadgets -- Remove and deallocate all standard and
*                         application gadgets from the window.
*
*   SYNOPSIS
*        removeGadgets()
*
*        VOID removeGadgets(VOID);
*
*   FUNCTION
*        This function performs gadget cleanup. It is responsible for
*        deallocating and removing all gadgets from the window before
*        it is closed.
*
*   INPUTS
*        None.
*
*   RESULT
*        All gadgets are freed and removed from the window.
*
*   EXAMPLE
*
*   NOTES
*        Uses the global variable glist which is a linked list off all
*        the gadget toolkit gadgets.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
void removeGadgets()
{
   if(glist)
   {
      D1( kprintf("window.c: removeGadgets() FreeingGadgets(%lx)\n",glist) );
      FreeGadgets(glist);
      glist=NULL;
   }

   if (font)
   {
      D1( kprintf("window.c: removeGadgets() Closing font %lx\n", font) );
      CloseFont(font);
      font=NULL;
   }
}
#endif /* WINDOW */
