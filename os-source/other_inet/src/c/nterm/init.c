/***************************************************\
* 																	 *
*  master initialization for the terminal program 	 *
* 																	 *
\***************************************************/

#include "st.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench/startup.h"
#include "term.h"
#include "colors.h"
#include "serial.h"
#include "meta.h"

/* #define DEBUG /* */
/* #define printf kprintf /* */

extern init_tabs(), Screen_Normal(), Set_Terminal();
extern InitKeyMapping();
extern struct Screen *s;
extern struct Window *w;
extern unsigned char readchar, string[], *strptr;
extern UBYTE terminal_mode, done, Back[], Fore[], str_start, numrows, first_time, screen_reverse;
extern struct Menu menus[];
extern struct WBStartup *WBenchMsg;
extern ULONG sbit, cbit;
extern UWORD scr_width;

struct Screen *OpenScreen();
struct Window *OpenWindow();
struct TextAttr MyFont = {"topaz.font",8,FS_NORMAL,FPF_ROMFONT};
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Preferences MyPrefs; /* my copy of preferences */
struct Process *process;
char title[] =
"RloginVT 1.1                                                          ";

/*  12345678911234567892123456789312345678941234567895123456789612345 65 */
/*	"AmigaTerm V1.0.16   looking for fonts, wait...                    "; */
UBYTE title_length = 21;
ULONG FontBase, DosBase, ibit, abit;
UBYTE init_state; /* how far we got into the initialize */
/*                    012345678901234 */
char vname[32];

init(argc,argv) /* initialize all */
int argc;
char *argv[];
{
int i, file = 0;
ULONG scr_height, win_height;

	if (first_time) { /* if this is the very first init */
	init_state = 0; /* clear the init state inidicator */
/* open graphics */
	if ((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",0))==0) return(FALSE);
	init_state++; /* (1) graphics now open */

/* open intuition */
	if ((IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",0))==0) return(FALSE);
	init_state++; /* (2) intuition now open */

/* open DOS */
	if ((DosBase=OpenLibrary(DOSNAME,0))==0) return(FALSE);
	init_state++; /* (3) DOS now open */
	} /* if (first_time) */

/* open screen */
	scr_height = numrows==MINROWS ? 200 : 400; /* set screen height */
	win_height = scr_height - 8; /* set window height */
#ifdef DEBUG
printf("trying to open screen, scr_height=%ld, win_height=%ld\n",scr_height,win_height);
#endif
	{
	struct NewScreen ns;
	ns.LeftEdge = 0;
	ns.TopEdge = 0;
	ns.Width = scr_width;
	ns.Height = scr_height;
	ns.Depth = 1;
	ns.DetailPen = COLOR0;
	ns.BlockPen = COLOR1;
	ns.ViewModes = HIRES | (scr_height==200 ? NULL : LACE);
	ns.Type = CUSTOMSCREEN;
	ns.Font = &MyFont;
	ns.DefaultTitle = title;
	ns.Gadgets = NULL;
	ns.CustomBitMap = NULL;
	s = OpenScreen(&ns);
	}
	if (!s) return(FALSE); /* if couldnt open screen */
	init_state++; /* (4) screen now open */

#ifdef DEBUG
printf("Getting prefs\n");
#endif
	if (first_time) { /* if run for the first time, get prefs */
		GetPrefs(&MyPrefs,sizeof(MyPrefs)); /* get preferences */
		for (i=0; i<3; i++) {
			Back[i] = (MyPrefs.color0 >> ((2-i)<<2)) & 0xf; /* calc rgb */
			Fore[i] = (MyPrefs.color1 >> ((2-i)<<2)) & 0xf; /* calc rgb */
		}
	}
	Set_Screen_Colors();

/* open window */
#ifdef DEBUG
printf("trying to open window\n");
#endif
	{
	struct NewWindow nw;
	nw.LeftEdge = 0;
	nw.TopEdge = 8;
	nw.Width = scr_width;
	nw.Height = win_height;
	nw.DetailPen = COLOR0;
	nw.BlockPen = COLOR1;
	nw.IDCMPFlags = IDCMP_FLAGS;
	nw.Flags = SMART_REFRESH|BORDERLESS|ACTIVATE|NOCAREREFRESH|REPORTMOUSE|BACKDROP;
	nw.FirstGadget = NULL;
	nw.CheckMark = NULL;
	nw.Title = NULL;
	nw.Screen = s;	
	nw.BitMap = NULL;
	nw.MinWidth = nw.Width;
	nw.MinHeight = nw.Height;
	nw.MaxWidth = nw.Width;
	nw.MaxHeight = nw.Height;
	nw.Type = CUSTOMSCREEN;
	w = (struct Window *)OpenWindow(&nw);
	}
	if (w==0) return(FALSE); /* if cant open window */
	ShowTitle(s,FALSE); /* force my screen behind my BACKDROP window */
	init_state++; /* (5) window now open */

/* open serial */
	init_state++; /* (6) serial now open (MUST BE OUTSIDE 'if') */

/* open rom font */
	if (first_time) {
#ifdef DEBUG
printf("trying to open rom font\n");
#endif
		if ((FontBase = OpenFont(&MyFont))==0) return(FALSE); /* if cant open rom font */
	}
	init_state++; /* (7) rom font now open (MUST BE OUTSIDE 'if') */

	if (first_time) {
	/* open the disk fonts */
#ifdef DEBUG
	printf("trying to init disk fonts\n");
#endif
	InitDiskFonts(); /* init (open) the disk fonts */
	} /* if (first_time) */
	init_state++; /* (8) disk fonts now open (MUST be outside the 'if') */

/* init misc. */
#ifdef DEBUG
printf("trying to init buffer\n");
#endif
	init_state++; /* (9) buffer, etc now done (MUST BE OUTSIDE 'if') */

/* init menus */
#ifdef DEBUG
printf("trying to init menu strip\n");
#endif
	SetMenuStrip(w, menus); /* */
	init_state++; /* (10) menu strip now attached */

/* switch my window pointer */
	init_state++; /* (11) mem now allocated for ptr & ptr now set to me */

/* init system requestor re-direction */
	process = (struct Process *)FindTask(NULL); /* get the process ptr */
	Requests_to_Me();
	init_state++; /* (12) system requestors now re-directed to me */

#ifdef DEBUG
printf("trying to set rom font\n");
#endif
	if (!SetFont(w->RPort,FontBase)) return(FALSE); /* if cant set rom font */

#ifdef DEBUG
printf("trying to open console\n");
#endif
	if (CDOpen(w)!=0) return(FALSE); /* if cant open the console device */
	init_state++; /* (13) console now open */

#ifdef DEBUG
printf("trying to init key mapping\n");
#endif
	if (first_time) {
		if (!InitKeyMapping()) return(FALSE); /* if cant init key mapping */
	}
	else WriteKeyMap(); /* re-write the keymap to the new console */

/* init to VT100 terminal */
#ifdef DEBUG
printf("trying to init terminal type\n");
#endif
	strptr = &string[str_start = 0]; /* init to null */
	if (!Set_Terminal(terminal_mode)) return(FALSE);

	Msg("Terminal OK"); /* clear msg window */

	if (first_time) {
#ifdef DEBUG
printf("trying to init f1-f10\n");
#endif
		for (i=0; i<NUMFUNC; i++) {
			Set_Function_Key("", i+1);
		}
#ifdef DEBUG
printf("trying to load settings\n");
#endif
		Load_Settings(0); /* load in default settings file */
	}

	ibit = 1 << w->UserPort->mp_SigBit; /* intuitions signal bit */
	abit = sbit | cbit | ibit; /* all signal bits Im interested in */

#ifdef DEBUG
printf("end of init, a=%08lx, s=%08lx, c=%08lx, i=%08lx\n",abit,sbit,cbit,ibit);
#endif
	first_time = FALSE;
	return(TRUE); /* flag that all inits were done ok */
}

bye(full_exit)
BYTE full_exit;
{
#ifdef DEBUG
printf("inside bye, full_exit=%ld, init_state = %ld\n",full_exit, init_state);
#endif
	switch (init_state) {

	case 13: init_state--;
		CDClose();
#ifdef DEBUG
printf("console closed\n");
#endif

	case 12: init_state--;
		Requests_to_System();
#ifdef DEBUG
printf("sys req now back to system\n");
#endif

	case 11: init_state--;


	case 10: init_state--;
		ClearMenuStrip(w);
#ifdef DEBUG
printf("menu strip cleared\n");
#endif

	case 9: init_state--;

	case 8: init_state--;
		if (full_exit) {
			CloseDiskFonts(); /* close the disk fonts (maybe) */
#ifdef DEBUG
printf("disk fonts closed\n");
#endif
		}

	case 7: init_state--;
		if (full_exit) {
			CloseFont(FontBase);
#ifdef DEBUG
printf("rom font closed\n");
#endif
		}

	case 6: init_state--;

	case 5: init_state--;
		CloseWindow(w);
#ifdef DEBUG
printf("window closed\n");
#endif

	case 4: init_state--;
		CloseScreen(s);
#ifdef DEBUG
printf("screen closed\n");
#endif

	if (!full_exit) return(TRUE); /* this is only a partial shut_down */

	case 3: init_state--;
		CloseLibrary(DosBase);
#ifdef DEBUG
printf("DOS closed\n");
#endif

	case 2: init_state--;
		CloseLibrary(IntuitionBase);
#ifdef DEBUG
printf("intuition closed\n");
#endif

	case 1: init_state--;
		CloseLibrary(GfxBase);
#ifdef DEBUG
printf("graphics closed\n");
#endif

	case 0: /* do nothing */
	default: break;
	}
}

Requests_to_Me()
{
	process->pr_WindowPtr = w; /* re-direct system requestors to me */
}

Requests_to_System()
{
	process->pr_WindowPtr = NULL; /* re-direct sys req to system */
}

