#include "configopts.h"
#include "defaultfonts.h"
#include "menus.h"
#include <workbench/startup.h>

#define BASEHEIGHT 180
#define BASEWIDTH  500

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *AslBase;
extern struct WBStartup *WBenchMsg;

void _main(p)
char *p;
{
   struct RenderInfo *rinfo;
   struct NewWindow nw;
   struct Window *window;
   struct GLOBAL global;
   struct IntuiMessage *imsg;

   rinfo = NULL;
   window = NULL;
   
   GfxBase       = (struct GfxBase *)      OpenLibrary("graphics.library", 0);
   IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0);
   AslBase       =                         OpenLibrary("asl.library", 0);
   rinfo = Get_RenderInfo(NULL);

   memset((char *)&global, 0, sizeof(global));
   memset((char *)&nw, 0, sizeof(nw));
   nw.Width      = BASEWIDTH  + rinfo->WindowLeft  + rinfo->WindowRight;
   nw.Height     = BASEHEIGHT + rinfo->WindowTitle + rinfo->WindowBottom;
   nw.LeftEdge   = (rinfo->ScreenWidth  - nw.Width)  / 2;
   nw.TopEdge    = (rinfo->ScreenHeight - nw.Height) / 2;
   nw.DetailPen  = -1;
   nw.BlockPen   = -1;
   nw.IDCMPFlags = REFRESHWINDOW | MENUPICK | GADGETDOWN | GADGETUP | VANILLAKEY | MOUSEBUTTONS;
   nw.Flags      = WINDOWDEPTH | WINDOWDRAG | SIMPLE_REFRESH | ACTIVATE;
   nw.Type       = WBENCHSCREEN;

   if (WBenchMsg != NULL)
   {
      /* Set the current directory to the one that they specified */
      /* via the workbench message startup                        */
      if (WBenchMsg->sm_NumArgs)
      {
         global.olddir = CurrentDir(
                WBenchMsg->sm_ArgList[WBenchMsg->sm_NumArgs-1].wa_Lock);
         global.setdir = 1;
      }
   }
   window = OpenWindow(&nw);
   if (window == NULL) fatal(&global, "Can't open window\n");

   if (AslBase != NULL)      global.filereq = AllocFileRequest();
   global.window    = window;
   global.rp        = window->RPort;
   global.highlight = rinfo->Highlight;
   global.shadow    = rinfo->Shadow;
   global.textpen   = rinfo->TextPen;
   global.backpen   = rinfo->BackPen;
   global.rinfo     = rinfo;
   global.scrnum    = 1;
   global.left      = rinfo->WindowLeft;
   global.right     = window->Width - rinfo->WindowRight;
   global.top       = rinfo->WindowTitle;
   global.bottom    = window->Height - rinfo->WindowBottom;
   
   resetall(&global);
   SetFont(global.rp, rinfo->TheFont);
   setup(&global);

   while(global.scrnum)
   {
      while ((imsg = (struct IntuiMessage *)GetMsg(window->UserPort)) == NULL)
      {
         Wait(1 << window -> UserPort->mp_SigBit);
      }
      switch(imsg->Class)
      {
         case GADGETDOWN:
            break;
         case GADGETUP:
            handlehit(&global, (struct Gadget *)imsg->IAddress, 
                      imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT) ?1:0);
            break;
         case MOUSEBUTTONS:
            break;
         case REFRESHWINDOW:
            global.reset = 1;
            refresh(&global);
            global.reset = 0;
            break;
         case MENUPICK:
            handlemenu(&global, MENUNUM(imsg->Code), ITEMNUM(imsg->Code));
            break;
         case VANILLAKEY:
            handlekey(&global, imsg->Code,
                      imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT) ?1:0);
         case MOUSEMOVE:
            break;
      }
      ReplyMsg((struct Message *)imsg);
   }

   ClearMenuStrip( window );
   freegads(&global);
   FreeRemember(&(global.strmem), TRUE);
   if (window != NULL)         CloseWindow(window);
   if (rinfo != NULL)          CleanUp_RenderInfo(rinfo);
   if (global.filereq != NULL) FreeFileRequest(global.filereq);
   if (AslBase != NULL)        CloseLibrary(AslBase);
   if (GfxBase != NULL)        CloseLibrary((struct Library *)GfxBase);
   if (IntuitionBase != NULL)  CloseLibrary((struct Library *)IntuitionBase);
   if (global.setdir)          CurrentDir(global.olddir);
}

void fatal(global, str)
struct GLOBAL *global;
char *str;
{
   ClearMenuStrip( global->window );
   freegads(global);
   FreeRemember(&(global->strmem), TRUE);
   if (global->window != NULL)   CloseWindow(global->window);
   if (global->rinfo != NULL)    CleanUp_RenderInfo(global->rinfo);
   if (global->filereq != NULL)  FreeFileRequest(global->filereq);
   if (AslBase != NULL)          CloseLibrary(AslBase);
   if (GfxBase != NULL)          CloseLibrary((struct Library *)GfxBase);
   if (IntuitionBase != NULL)    CloseLibrary((struct Library *)IntuitionBase);
   if (global->setdir)           CurrentDir(global->olddir);
   _exit(1);
}

void MemCleanup(){}
