
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

#ifndef APP_H
#define APP_H

#ifdef LATTICE
#include <pragmas/gadtools_pragmas.h>
#endif

/*
#define DEBUG
*/

#include <clib/all_protos.h>

/**********************************************************************/
/* Prototypes for functions declared in app.c and called from the     */
/* standard modules.                                                  */
/**********************************************************************/
VOID setupCustomGadgets(struct Gadget **);
VOID HandleGadget(ULONG,ULONG);
VOID setupCustomMenu(VOID);
VOID handleCustomMenu(UWORD);
VOID refreshWindow(VOID);
BOOL setupCustomCX(VOID);
VOID shutdownCustomCX(VOID);
VOID handleCustomCXMsg(ULONG);
VOID handleCustomCXCommand(ULONG);
VOID handleCustomSignal(VOID);

/**********************************************************************/
/* Prototypes for functions declared in the standard modules and      */
/* called by app.c                                                    */
/**********************************************************************/
VOID setupWindow(VOID);
VOID shutdownWindow(VOID);
VOID terminate(VOID);

/**********************************************************************/
/* Prototypes for functions declared in application modules and       */
/* called by app.c                                                    */
/**********************************************************************/
BOOL setupAutopoint(VOID);
VOID MyHandleCustomSignal(VOID);

/**********************************************************************/
/* definitions for global variables declared in the standard modules  */
/* referenced by app.c                                                */
/**********************************************************************/
extern CxObj                  *broker;
extern SHORT                  topborder;
extern VOID                   *vi;
extern struct Menu            *menu;
extern struct Library         *GadToolsBase;
extern struct Gadget          *glist;
extern char                   **ttypes;
extern struct MsgPort         *cxport;
extern struct IntuitionBase   *IntuitionBase;
extern struct DrawInfo        *mydi;
extern BOOL                   IDCMPRefresh;
extern ULONG                  csigflag;
extern struct Task            *maintask;
extern LONG                   csignal;

/**********************************************************************/
/* definitions for global variables declared in app.c and             */
/* referenced by the standard modules.                                */
/**********************************************************************/
extern struct TextAttr mydesiredfont;

/**********************************************************************/
/* Commodities specific definitions.                                  */
/*                                                                    */
/* COM_NAME  - used for the scrolling display in the Exchange program */
/* COM_TITLE - used for the window title bar and the long description */
/*             in the Exchange program                                */
/* COM_DESC  - Commodity description used by the Exchange program     */
/* CX_DEFAULT_PRIORITY - default priority for this commodities broker */
/*                       can be overidden by using icon TOOL TYPES    */
/**********************************************************************/
#define COM_NAME  "NumPad"
#define COM_TITLE "NumPad"
#define COM_DESCR "Numeric Pad on CTRL-ALT IOP block"
#define CX_DEFAULT_PRIORITY 0
#define CX_DEFAULT_POP_KEY ("shift f1")
#define CX_DEFAULT_POP_ON_START ("NO")
#define APPARGS " "

/**********************************************************************/
/* Custom Signal control                                              */
/*                                                                    */
/* If CSIGNAL = 0 then this commodity will NOT have a custom signal   */
/* If CSIGNAL = 1 this commodity will support a custom signal         */
/**********************************************************************/
#define CSIGNAL 1

/**********************************************************************/
/* Window control                                                     */
/*                                                                    */
/* If WINDOW = 0 then this commodity will NOT have a popup window     */
/* If WINDOW = 1 this commodity will support a popup window with the  */
/*               attributes defined below.                            */
/**********************************************************************/
#define WINDOW 0

#if WINDOW
#define W(x) x
#else
#define W(x) ;
#endif

#if WINDOW

extern struct Window   *window; /* our window */
extern struct TextFont *font;

#define WINDOW_LEFT   134
#define WINDOW_TOP    64
#define WINDOW_WIDTH  362
#define WINDOW_HEIGHT 68
#define WINDOW_INNERHEIGHT 70

#define WINDOW_SIZING 0
#if WINDOW_SIZING
#define WINDOW_MAX_WIDTH  -1
#define WINDOW_MIN_WIDTH  50
#define WINDOW_MAX_HEIGHT -1
#define WINDOW_MIN_HEIGHT 30
#define WFLAGS (ACTIVATE | WINDOWCLOSE | WINDOWDRAG | WINDOWSIZING | WINDOWDEPTH | SIMPLE_REFRESH )
#else
#define WINDOW_MAX_WIDTH  WINDOW_WIDTH
#define WINDOW_MIN_WIDTH  WINDOW_WIDTH
#define WINDOW_MAX_HEIGHT WINDOW_HEIGHT
#define WINDOW_MIN_HEIGHT WINDOW_HEIGHT
#define WFLAGS (ACTIVATE | WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH | SIMPLE_REFRESH )
#endif  /* WINDOW_SIZING */

#define IFLAGS (MENUPICK | MOUSEBUTTONS | GADGETUP | GADGETDOWN | MOUSEMOVE | CLOSEWINDOW | REFRESHWINDOW )

                                     /* hotkey definitions                */
#define POP_KEY_ID     (86L)         /* pop up identifier                 */

/**********************************************************************/
/* Gadget control                                                     */
/*                                                                    */
/* Here are the gadget specific definitions. Note that these are      */
/* included only if WINDOW=1 since gadgets make no sense without a    */
/* window.                                                            */
/**********************************************************************/
#define GAD_HIDE     1
#define GAD_DIE      2

/**********************************************************************/
/* Menu control                                                       */
/*                                                                    */
/* Here are the menu specific definitions. Note that these are        */
/* included only if WINDOW=1 since menus make no sense without a      */
/* window.                                                            */
/**********************************************************************/
#define MENU_HIDE     1
#define MENU_DIE      2

#endif  /* WINDOW */

/**********************************************************************/
/* Debug control                                                      */
/*                                                                    */
/* The first define converts any printfs that got in by mistake into  */
/* kprintfs. If you are debuging to the console you can change        */
/* kprintfs into printfs.                                             */
/* The D1(x) define controls debugging in the standard modules. Use   */
/* The D(x) macro for debugging in the app.c module.                  */
/**********************************************************************/
void kprintf(char *,...);
#define printf kprintf

#ifdef DEBUG
#define D1(x) x
#define D(x)  x
#else
#define D1(x) ;
#define D(x)  ;
#endif /* NO DEBUG */

#endif /* APP_H */
