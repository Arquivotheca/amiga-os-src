;/*
LC -L -b0 -ms -v -rr -cfistqmc -d1 textkind.c
Quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/hooks.h>
#include <string.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>


extern struct Library *SysBase;
extern struct Library *DOSBase;
       struct Library *GfxBase;
       struct Library *IntuitionBase;
       struct Library *GadToolsBase;
       struct Library *UtilityBase;
       struct Library *AslBase;


#define	TEMPLATE	""
#define	OPT_COUNT	1


struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};



VOID main(VOID)
{
struct RdArgs *rdargs;
LONG           opts[OPT_COUNT];
struct Window *wp;
APTR vi;
struct Gadget *lastAdded;
struct Gadget *gadgets;
struct NewGadget ng;
struct IntuiMessage *intuiMsg;

    IntuitionBase = OpenLibrary("intuition.library", 0);
    GfxBase = OpenLibrary("graphics.library", 0);
    GadToolsBase = OpenLibrary("gadtools.library", 0);
    UtilityBase = OpenLibrary("utility.library", 0);

    memset(opts,0,sizeof(opts));
    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
    {
        if (wp = OpenWindowTags(NULL,WA_AutoAdjust,  TRUE,
                                     WA_SizeGadget,  TRUE,
                                     WA_DepthGadget, TRUE,
                                     WA_DragBar,     TRUE,
                                     WA_CloseGadget, TRUE,
                                     WA_Activate,    TRUE,
                                     WA_IDCMP,       IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_REFRESHWINDOW,
                                     WA_Width,       275,
                                     WA_Height,      120,
                                     WA_SimpleRefresh, TRUE,
                                     TAG_DONE))
        {
            if (vi = GetVisualInfoA(wp->WScreen,NULL))
            {
                lastAdded = CreateContext(&gadgets);

                ng.ng_LeftEdge    = 50;
                ng.ng_TopEdge     = 20;
                ng.ng_Width       = 160;
                ng.ng_Height      = 10;
                ng.ng_GadgetText  = "Test";
                ng.ng_GadgetID    = 0;
                ng.ng_TextAttr    = &topazAttr;
                ng.ng_UserData    = 0;
                ng.ng_Flags       = NULL;
                ng.ng_VisualInfo  = vi;

                if (lastAdded = CreateGadget(TEXT_KIND,lastAdded,&ng,GTTX_Text,  "This is a test test test test test",
                                                                     GTTX_Clipped, TRUE,
                                                                     TAG_DONE))
                {
                    AddGList(wp,gadgets,-1,-1,NULL);
                    RefreshGList(gadgets,wp,NULL,-1);
                    GT_RefreshWindow(wp,NULL);

                    DrawBevelBox(wp->RPort,20,40,100,14,GT_VisualInfo,vi,
                                                        GTBB_FrameType, BBFT_ICONDROPBOX,
                                                        TAG_DONE);

                    while (TRUE)
                    {
                        WaitPort(wp->UserPort);
                        intuiMsg = (struct IntuiMessage *)GetMsg(wp->UserPort);

                        if (intuiMsg->Class == IDCMP_REFRESHWINDOW)
                        {
                            GT_BeginRefresh(wp);
                            GT_EndRefresh(wp,TRUE);
                        }
                        else
                        {
                            break;
                        }
                        ReplyMsg(intuiMsg);
                    }
                }
            }
        }
        CloseWindow(wp);

        FreeArgs(rdargs);
    }

    CloseLibrary(IntuitionBase);
    CloseLibrary(GfxBase);
    CloseLibrary(GadToolsBase);
    CloseLibrary(UtilityBase);
}
