/* GameControl.c
 * 
 * CLI usage:
 *     [run] gamecontrol [screen <PubScreenName>]
 * Workbench usage:
 *     double-click on the icon
 *
 * The purpose of this beastie is to test the mechanical contacts on the game
 * controller to see if there are any spurious directions.  It's quite
 * visual.  To test, simply run this from CLI or Workbench, and have a game
 * controller plugged into Port 0.  Press the directional keypad in such
 * a way as to go along a straight line in one direction.  If you don't,
 * there's a problem with the controller.
 *
 * It also happens to be yet another test of lowlevel.library's
 * SystemControl(SCON_AddCreateKeys) function, which is why it lives
 * where it lives on the server.
 *
 * $VER: GameControl_c 1.1 (18.05.93) by J.W. Lockhart
 *
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <graphics/gfx.h>
#include <graphics/display.h>
#include <graphics/rastport.h>
#include <graphics/regions.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/rpattr.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/lowlevel.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/intuition_protos.h>
#include <stdlib.h>
#include <string.h>

#include <pragmas/lowlevel_pragmas.h>

#include "gamecontrol_protos.h"

/* Defines ------------------------------------------ */
#ifndef PROGNAME
    /* conditional so that we can redefine it for the Wrap version */
#define PROGNAME "GameControl"
#endif
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "SCREEN"
#define MAX_XGRID 31       /* 31 horizontal boxes           */
#define MAX_YGRID  9       /* 9 vertical boxes              */
#define GRIDSIZE  20       /* 20 pixels by 20 pixels        */
#define PEN_FG    1        /* foreground/text pen           */
#define PEN_BG    0        /* background/transparent pen    */
#define PEN_HI    3        /* highlight pen                 */
#define PEN_FLASH 4        /* flash pen (shine)             */


/* Structs ------------------------------ */
struct Opts {
    STRPTR screen;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern VOID KPrintF(STRPTR,...);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *LowLevelBase;
struct Library *KeymapBase;
struct Window  *win;
struct RastPort *rp;
struct EasyStruct easyStruct, *ez;
UBYTE  ezBuf[100];
BOOL   cliStart, keysOn;
struct Rectangle grid[MAX_XGRID][MAX_YGRID];
int    curX, curY;              /* actually, current grid[][] numbers */

VOID main(int argc, UBYTE *argv[]) {
    struct Screen *scr;
    UWORD code, qual;
    ULONG class;
    struct IntuiMessage *msg;

    cliStart = (argc != 0);
    ez = &easyStruct;
    ez->es_StructSize = sizeof(struct EasyStruct);
    ez->es_Flags = NULL;
    ez->es_GadgetFormat = "OK";
    ez->es_Title = PROGNAME;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    if (!turnOnLLKeys(TRUE)) {
        ezReq("SystemControl() failed!\nCannot read controller!", NULL);
        GoodBye(RETURN_FAIL);
    }
    keysOn = TRUE;

    if (scr = LockPubScreen(opts.screen)) {  /* null opts.screen == Workbench */
        win = OpenWindowTags(NULL, 
                             WA_Title, PROGNAME,
                             WA_PubScreen, scr,
                             WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE,
                             WA_RMBTrap, TRUE,
                             WA_InnerWidth, 620,
                             WA_InnerHeight, 180,
                             WA_DragBar, TRUE,
                             WA_CloseGadget, TRUE,
                             WA_DepthGadget, TRUE,
                             WA_SizeGadget, FALSE,
                             WA_Activate, TRUE,
                             WA_SimpleRefresh, TRUE,
                             TAG_DONE);
        UnlockPubScreen(NULL,scr);
    }
    else {
        ezReq("Can't lock screen to open window!\n", NULL);
        GoodBye(RETURN_FAIL);
    }

    if (!win) {
        ezReq("Can't open window!\n", NULL);
        GoodBye(RETURN_FAIL);
    }

    rp = win->RPort;
    initGrid();
    drawGrid();

    curX = MAX_XGRID / 2 ;
    curY = MAX_YGRID / 2 ;
    liteBox(curX, curY);

    while (1) {
        WaitPort(win->UserPort);
        while (msg = (struct IntuiMessage *)GetMsg(win->UserPort)) {
            class = msg->Class;
            code  = msg->Code;
            qual  = msg->Qualifier;
            switch (class) {
                case IDCMP_CLOSEWINDOW:
                    ReplyMsg((struct Message *)msg);
                    GoodBye(RETURN_OK);
                    break;
                case IDCMP_MOUSEMOVE:
                case IDCMP_MOUSEBUTTONS:
                case IDCMP_RAWKEY:
                    /* we look for PORT(anything) in case we ever need to... */
                    switch (code) {
                        case RAWKEY_PORT0_BUTTON_PLAY:
                            break;
                        case RAWKEY_PORT0_JOY_UP:
                        case RAWKEY_PORT1_JOY_UP:
                        case RAWKEY_PORT2_JOY_UP:
                        case RAWKEY_PORT3_JOY_UP:
                            changeBox(curX, curY, curX, curY-1);
                            break;
                        case RAWKEY_PORT0_JOY_DOWN:
                        case RAWKEY_PORT1_JOY_DOWN:
                        case RAWKEY_PORT2_JOY_DOWN:
                        case RAWKEY_PORT3_JOY_DOWN:
                            changeBox(curX, curY, curX, curY+1);
                            break;
                        case RAWKEY_PORT0_JOY_LEFT:
                        case RAWKEY_PORT1_JOY_LEFT:
                        case RAWKEY_PORT2_JOY_LEFT:
                        case RAWKEY_PORT3_JOY_LEFT:
                            changeBox(curX, curY, curX-1, curY);
                            break;
                        case RAWKEY_PORT0_JOY_RIGHT:
                        case RAWKEY_PORT1_JOY_RIGHT:
                        case RAWKEY_PORT2_JOY_RIGHT:
                        case RAWKEY_PORT3_JOY_RIGHT:
                            changeBox(curX, curY, curX+1, curY);
                            break;
                        default:
                            break;
                    }
                    break;
                case IDCMP_VANILLAKEY:
                    /* shouldn't get it */
                    break;
                case IDCMP_NEWSIZE:
                case IDCMP_REFRESHWINDOW:
                    drawGrid();
                    liteBox(curX, curY);
                    break;
                
                default:
                    break;
            }
            ReplyMsg((struct Message *)msg);
        }
    }

    GoodBye(RETURN_OK);
}

/* turnOnLLKeys ===============================================================
   use lowlevel.library/SystemControl() to turn on the flow of RawKey
   game controller events to our task (TRUE), or to turn them off (FALSE).
   Returns TRUE for success, FALSE otherwise.
 */
BOOL turnOnLLKeys(BOOL on) {
    ULONG retVal;

    if (on) {
        retVal = SystemControl(SCON_AddCreateKeys, 1UL, TAG_DONE);
    }
    else {
        retVal = SystemControl(SCON_RemCreateKeys, 1UL, TAG_DONE);
    }

    return((BOOL)(retVal == 0UL));
}

#ifdef DEBUG
/* dumpBox =====================================================
   debugging tool, tells grid number and Rectangle coordinates
 */
VOID dumpBox(STRPTR desc, int boxX, int boxY) {
    KPrintF("%s: [%ld][%ld] = %ld, %ld, %ld, %ld\n",
             desc,
             (LONG)boxX,
             (LONG)boxY,
             (LONG)grid[boxX][boxY].MinX,
             (LONG)grid[boxX][boxY].MinY,
             (LONG)grid[boxX][boxY].MaxX,
             (LONG)grid[boxX][boxY].MaxY);
}
#endif


/* changeBox ============================================
   If valid new box, re-grid the old one and highlight
   the new one.  change global curX and curY.

   Else if invalid new box, just flash the old one
   and don't change anything.

   Inputs are xy coords for grid[][]
      (grid[x][y])
*/
VOID changeBox(int oldX, int oldY, int newX, int newY) {

    /* only 1 coordinate will be changing,
       but it can't change beyond our array limits
     */

#ifdef WRAP
    if (newX >= MAX_XGRID) {
        newX = 0;
    }
    if (newX < 0) {
        newX = MAX_XGRID-1;
    }
    if (newY >= MAX_YGRID) {
        newY = 0;
    }
    if (newY < 0) {
        newY = MAX_YGRID-1;
    }
#else
    if ((newX >= MAX_XGRID) || (newX < 0)) {
        flashBox(oldX, oldY);
        return;
    }
    if ((newY >= MAX_YGRID) || (newY < 0)) {
        flashBox(oldX, oldY);
        return;
    }
#endif

/*
    dumpBox("Old", oldX, oldY);
    dumpBox("New", newX, newY);
*/

    gridBox(oldX, oldY);
    liteBox(newX, newY);
    curX = newX;
    curY = newY;

    return;
}

/* flashBox ==================================================
 */
VOID flashBox(int x, int y) {
    WORD miX, miY, maX, maY;

    miX = grid[x][y].MinX + 2;
    miY = grid[x][y].MinY + 2;
    maX = grid[x][y].MaxX - 2;
    maY = grid[x][y].MaxY - 2;

    SetAPen(rp, PEN_FLASH);
    RectFill(rp, miX, miY, maX, maY);

    Delay(2L);

    SetAPen(rp, PEN_HI);
    RectFill(rp, miX, miY, maX, maY);

    return;
}

/* gridBox ===================================================
   Draw a grid around a box.  That is, fill the box with black,
   allow for borders, then fill the smaller box with grey.
   Inputs are valid x and y such that "grid[x][y]" is a valid
   reference.
 */
VOID gridBox(int x, int y) {
    WORD miX, miY, maX, maY;

    miX = grid[x][y].MinX;
    miY = grid[x][y].MinY;
    maX = grid[x][y].MaxX;
    maY = grid[x][y].MaxY;

    SetAPen(rp, PEN_FG);
    RectFill(rp, miX, miY, maX, maY);

    miX += 2;
    miY += 2;
    maX -= 2;
    maY -= 2;

    SetAPen(rp, PEN_BG);
    RectFill(rp, miX, miY, maX, maY);

    return;
}

/* liteBox ===================================================
    Highlight a box, leaving the borders ("grid") intact.
 */
VOID liteBox(int x, int y) {
    WORD miX, miY, maX, maY;

    miX = grid[x][y].MinX + 2;
    miY = grid[x][y].MinY + 2;
    maX = grid[x][y].MaxX - 2;
    maY = grid[x][y].MaxY - 2;

    SetAPen(rp, PEN_HI);
    RectFill(rp, miX, miY, maX, maY);

    return;
}


/* drawGrid =========================================
    actually, does a bunch of rectfills :-)
    Easier to adjust the grid thickness that way.
 */
VOID drawGrid(VOID) {
    int i, j;
    SHORT mix, miy, maX, maY, minVal, maxVal;


    SetAPen(rp,PEN_FG);
    SetDrMd(rp,JAM1);

    minVal = grid[0][0].MinY;
    maxVal = grid[0][MAX_YGRID-1].MaxY;
    for (i = 1; i < MAX_XGRID; i++) {
        mix = grid[i][MAX_YGRID-1].MinX - 1;
        maX = grid[i][MAX_YGRID-1].MinX + 1;
        RectFill(rp, mix, minVal, maX, maxVal);
    }

    minVal = grid[0][0].MinX;
    maxVal = grid[MAX_XGRID-1][0].MaxX;
    for (j = 1; j < MAX_YGRID; j++) {
        miy = grid[MAX_XGRID-1][j].MinY - 1;
        maY = grid[MAX_XGRID-1][j].MinY + 1;
        RectFill(rp, minVal, miy, maxVal, maY);
    }

    /* now the special case, the outer borders...should also look snazzy. */

    /* top */
    RectFill(rp,grid[0][0].MinX, grid[0][0].MinY, grid[MAX_XGRID-1][0].MaxX, grid[0][0].MinY+1);

    /* right */
    RectFill(rp,grid[MAX_XGRID-1][0].MaxX-1,grid[MAX_XGRID-1][0].MinY,grid[MAX_XGRID-1][0].MaxX,grid[MAX_XGRID-1][MAX_YGRID-1].MaxY);

    /* bottom */
    RectFill(rp,grid[0][MAX_YGRID-1].MinX,grid[0][MAX_YGRID-1].MaxY-1,grid[MAX_XGRID-1][0].MaxX,grid[0][MAX_YGRID-1].MaxY);

    /* left */
    RectFill(rp,grid[0][0].MinX, grid[0][0].MinY,grid[0][0].MinX+1,grid[0][MAX_YGRID-1].MaxY);

    return;
}

/* initGrid =======================================
   fill in values for grid
 */
VOID initGrid(VOID) {
    int i, j, row, col, winBottom, winRight, rowAdj, colAdj;

    row = win->BorderLeft;
    winBottom = win->Height - win->BorderBottom - 1;
    winRight =  win->Width - (win->BorderLeft + win->BorderRight - 1);
    rowAdj = GRIDSIZE;

    for (i = 0; i < MAX_XGRID; i++) {
        /* move across */
        col = win->BorderTop;
        colAdj = GRIDSIZE;
        for (j = 0; j < MAX_YGRID; j++) {
            /* then move down each column */
            grid[i][j].MinX = row;
            grid[i][j].MinY = col;
            grid[i][j].MaxX = row + rowAdj;
            grid[i][j].MaxY = col + colAdj;
            col += GRIDSIZE;
            if ((col+colAdj) > winBottom) {
                colAdj = winBottom - col;
            }
        }
        row += GRIDSIZE;
        if ((row+rowAdj) >= winRight) {
            rowAdj = winRight - row;
        }
    }
    return;
}

/* ezReq ================================================================
   Pop up an EasyRequester to tell the user something.  Since we
   only offer "OK" to click upon, we don't return anything.
 */
VOID ezReq(STRPTR str, APTR arg1, ...) {

    ez->es_TextFormat = str;
    EasyRequestArgs(win, ez, NULL, &arg1);

    return;
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {

    if (win) {
        CloseWindow(win);
    }

    if (keysOn) {
        turnOnLLKeys(FALSE);
    }

    if (KeymapBase) {
        CloseLibrary(KeymapBase);
    }

    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    if (GfxBase) {
        CloseLibrary(GfxBase);
    }

    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    if (rdargs) {
        FreeArgs(rdargs);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    if (cliStart) {
        rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
        if (!rdargs) {
            PrintFault(IoErr(), PROGNAME);
            return(FALSE);
        }
    }

    IntuitionBase = OpenLibrary("intuition.library", 37L);
    if (!IntuitionBase) {
        Printf(ERRMSG_LIBNOOPEN, "intuition.library", 37L);
        return(FALSE);
    }

    GfxBase = OpenLibrary("graphics.library", 37L);
    if (!GfxBase) {
        ezReq(ERRMSG_LIBNOOPEN, "graphics.library", 37L);
        return(FALSE);
    }

    LowLevelBase = OpenLibrary("lowlevel.library", 40L);
    if (!LowLevelBase) {
        ezReq(ERRMSG_LIBNOOPEN, "lowlevel.library", 40L);
        return(FALSE);
    }

    /* in case we ever use keyboard events such as cursor_up, etc. */
    KeymapBase = OpenLibrary("keymap.library", 0L);
    if (!KeymapBase) {
        ezReq(ERRMSG_LIBNOOPEN, "keymap.library", 0L);
        return(FALSE);
    }
    return(TRUE);
}

