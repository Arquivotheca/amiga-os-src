#ifndef MYCXWIN_H
#define MYCXWIN_H

/**********************************************************************/
/* definitions for global variables declared in custom.c and          */
/* referenced by the standard modules.                                */
/**********************************************************************/
extern struct TextAttr mydesiredfont;

/**********************************************************************/
/* Window control                                                     */
/*                                                                    */
/* If WINDOW = 0 then this commodity will NOT have a popup window     */
/* If WINDOW = 1 this commodity will support a popup window with the  */
/*               attributes defined below.                            */
/**********************************************************************/
#define WINDOW 1

#if WINDOW
#define W(x) x
#else
#define W(x) ;
#endif

#if WINDOW

extern struct Window   *window; /* our window */
extern struct TextFont *font;

#define WINDOW_LEFT   10
#define WINDOW_TOP    10
#define WINDOW_WIDTH  300
#define WINDOW_HEIGHT 190
#define WINDOW_INNERHEIGHT 180

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


/**********************************************************************/
/* Gadget control                                                     */
/*                                                                    */
/* Here are the gadget specific definitions. Note that these are      */
/* included only if WINDOW=1 since gadgets make no sense without a    */
/* window.                                                            */
/**********************************************************************/
#define GAD_F1       1
#define GAD_F2       2
#define GAD_F3       3
#define GAD_F4       4
#define GAD_F5       5
#define GAD_F6       6
#define GAD_F7       7
#define GAD_F8       8
#define GAD_F9       9
#define GAD_F10      10
#define GAD_HIDE     11
#define GAD_DIE      12
#define MY_NWAY      13

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

#endif /* MYCXWIN_H */
