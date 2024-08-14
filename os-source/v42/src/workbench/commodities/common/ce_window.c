
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <graphics/text.h>
#include <libraries/gadtools.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "ce_custom.h"
#include "ce_window.h"
#include "ce_strings.h"


/*****************************************************************************/


extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern STRPTR          cxPopKey;

struct Window   *cxWindow;
struct Screen   *cxScreen;
struct DrawInfo *cxDrawInfo;
APTR            *cxVisualInfo;
struct Menu     *cxMenus;
struct Gadget   *cxGadgets;
struct MsgPort  *intuiPort;
ULONG            intuiMask;

WORD windowLeft   = WINDOW_LEFT;
WORD windowTop    = WINDOW_TOP;
WORD windowWidth  = WINDOW_WIDTH;
WORD windowHeight = WINDOW_HEIGHT;
char windowTitle[80];

static ULONG tag1 = TAG_IGNORE;
static ULONG tag2 = TAG_IGNORE;


/*****************************************************************************/


VOID CreateWindow(VOID)
{
UWORD zoomSize[4];

    if (cxWindow)
    {
        WindowToFront(cxWindow);
        ActivateWindow(cxWindow);
        if (cxWindow->Flags & WFLG_ZOOMED)
            ZipWindow(cxWindow);
        return;
    }

    if (cxScreen = LockPubScreen(NULL))
    {
        if (cxDrawInfo = GetScreenDrawInfo(cxScreen))
        {
            if (cxVisualInfo = GetVisualInfo(cxScreen,TAG_DONE))
            {
                sprintf(windowTitle,GetStr(MSG_WINDOW_TITLE),GetStr(CE_NAME),cxPopKey);

                zoomSize[0] = 0xffff;
                zoomSize[1] = 0xffff;
                zoomSize[2] = 200;
                zoomSize[3] = cxScreen->WBorTop + cxScreen->Font->ta_YSize + 1;

                if (cxWindow = OpenWindowTags(NULL,tag1,           windowLeft,
                                                   tag2,           windowTop,
                                                   WA_InnerWidth,  windowWidth,
                                                   WA_InnerHeight, windowHeight,
                                                   WA_IDCMP,       IDCMPFLAGS,
                                                   WA_Flags,       WINDOWFLAGS,
                                                   WA_Title,       windowTitle,
                                                   WA_AutoAdjust,  TRUE,
                                                   WA_PubScreen,   cxScreen,
                                                   WA_NewLookMenus,TRUE,
                                                   WA_Zoom,        zoomSize,
                                                   TAG_DONE))
                {
                    intuiPort = cxWindow->UserPort;
                    intuiMask = 1L << intuiPort->mp_SigBit;

                    tag1 = WA_Left;
                    tag2 = WA_Top;

                    if (CreateCustomGadgets())
                    {
                        if (CreateCustomMenus())
                        {
                            RefreshWindow(FALSE);
                            return;
                        }
                    }
                }
            }
        }
    }

    DisposeWindow();
}


/*****************************************************************************/


VOID DisposeWindow(VOID)
{
    if (cxWindow)
    {
        ClearMenuStrip(cxWindow);

        windowLeft = cxWindow->LeftEdge;
        windowTop  = cxWindow->TopEdge;

        if (!(cxWindow->Flags & WFLG_ZOOMED))
        {
            windowWidth  = cxWindow->Width - cxWindow->BorderLeft - cxWindow->BorderRight;
            windowHeight = cxWindow->Height - cxWindow->BorderTop - cxWindow->BorderBottom;
        }

        CloseWindow(cxWindow);
    }

    if (cxScreen)
    {
        UnlockPubScreen(NULL,cxScreen);
        FreeScreenDrawInfo(cxScreen,cxDrawInfo);
    }

    FreeMenus(cxMenus);
    FreeGadgets(cxGadgets);
    FreeVisualInfo(cxVisualInfo);

    cxMenus      = NULL;
    cxGadgets    = NULL;
    cxScreen     = NULL;
    cxWindow     = NULL;
    cxDrawInfo   = NULL;
    cxVisualInfo = NULL;
    intuiPort    = NULL;
    intuiMask    = NULL;
}
