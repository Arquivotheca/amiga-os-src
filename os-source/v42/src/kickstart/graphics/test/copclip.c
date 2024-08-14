/*
    ucop3.c - copper list VPort clip test

     author: Bill Barton
             Digital Creations
             916/344-4825
             bix: peabody

    compile: SAS/C 5.10b
             lc -cfqsu -v ucop3
             blink FROM lib:c.o ucop3.o TO ucop3 LIB lib:lc.lib lib:amiga.lib sc

        run: ucop3
             Drag custom screen to observe effects.
             Close window to exit.

    Demonstrates a user copper list clipping bug related to
    VTAG_USERCLIP_SET.

    Run this program and drag its screen down until the grey bars
    vanish off the bottom of the display.  Notice the bottom line
    has a line of grey bars.  It shouldn't.  In fact if this
    screen were behind another screen, this bug wouldn't happen at
    all.
*/

#include <exec/memory.h>
#include <graphics/copper.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>
#include <hardware/custom.h>
#include <intuition/intuition.h>


    /* location of color register copper instructions */
#define ZAP_Top         100
#define ZAP_Height      10
#define ZAP_Row(n)      (ZAP_Top + (n) * ZAP_Height)
#define ZAP_NRows       4

    /* location of boxes */
#define BOX_Left        30
#define BOX_Top         15
#define BOX_Height      150
#define BOX_Width       50
#define BOX_Col(n)      (BOX_Left + (n) * BOX_Width)
#define BOX_NCols       ZAPPEN_Count

    /* color registers to affect */
#define ZAPPEN_Base     3
#define ZAPPEN_Count    4

struct Library *GfxBase, *IntuitionBase;

struct UCopList *ucoplist;

UWORD pens[] = { ~0 };

struct TagItem vtags[] = {
    { VTAG_USERCLIP_SET, 1L },
    TAG_END
};

void drawboxes (struct Window *);
void build_ucopper (struct Screen *);
void kill_ucopper (struct Screen *);

int main (void)
{
    struct Screen *screen = NULL;
    struct Window *window = NULL;

    if (!(IntuitionBase = OpenLibrary ("intuition.library", 37))) goto clean;

    if (!(GfxBase = OpenLibrary ("graphics.library", LIBRARY_MINIMUM))) goto clean;

    if (!(screen = OpenScreenTags ( NULL,
                                    SA_DisplayID,   LORES_KEY,          /* the display mode and depth don't affect the problem. */
                                    SA_Depth,       3L,
                                    SA_Title,       "UCoplist problem",
                                    SA_Pens,        pens,
                                    TAG_END ))) goto clean;

    VideoControl (screen->ViewPort.ColorMap, vtags);
    MakeScreen (screen);
    RethinkDisplay();

    if (!(window = OpenWindowTags ( NULL,
                                    WA_CustomScreen,    screen,
                                    WA_Top,             12L,
                                    WA_Height,          (long)(screen->Height - 12),
                                    WA_Flags,           WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH,
                                    WA_IDCMP,           IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY,
                                    TAG_END ))) goto clean;

    drawboxes (window);
    build_ucopper (screen);

    WaitPort (window->UserPort);

    kill_ucopper (screen);

clean:
    if (window) CloseWindow (window);
    if (screen) CloseScreen (screen);
    if (IntuitionBase) CloseLibrary (IntuitionBase);
    if (GfxBase) CloseLibrary (GfxBase);

    return 0;
}

void drawboxes (struct Window *window)
{
    struct RastPort *rp = window->RPort;
    int i;

    SetDrMd (rp, JAM1);
    BNDRYOFF (rp);

    for (i=0; i<BOX_NCols; i++) {
        SetAPen (rp, ZAPPEN_Base + i);
        RectFill (rp, BOX_Col(i), BOX_Top, BOX_Col(i) + BOX_Width - 1, BOX_Top + BOX_Height - 1);
    }
}


/* create a copper list to modify 4 color registers at various locations on the screen */

void build_ucopper (struct Screen *screen)
{
    extern struct Custom far custom;
    int i,j;
    UWORD color = 0;

    if (ucoplist = AllocMem (sizeof *ucoplist, MEMF_PUBLIC|MEMF_CLEAR)) {

        CINIT (ucoplist, ZAP_NRows * (ZAPPEN_Count + 1) + 1);

        for (i=0; i<ZAP_NRows; i++) {
            CWAIT (ucoplist, ZAP_Row(i), 0);
            for (j=0; j<ZAPPEN_Count; j++) {
                CMOVE (ucoplist, custom.color[ZAPPEN_Base+j], color += 0x111);
            }
        }
        CEND (ucoplist);

        screen->ViewPort.UCopIns = ucoplist;
        MakeScreen (screen);
        RethinkDisplay();
    }
}

void kill_ucopper (struct Screen *screen)
{
    if (ucoplist) {
        screen->ViewPort.UCopIns = NULL;
        MakeScreen (screen);
        RethinkDisplay();

        FreeCopList(ucoplist->FirstCopList);         /* free up last copper made */
        FreeMem(ucoplist,sizeof *ucoplist);         /* free up the copper header */
        ucoplist = NULL;                           /* don't use any freed memory */
    }
}

