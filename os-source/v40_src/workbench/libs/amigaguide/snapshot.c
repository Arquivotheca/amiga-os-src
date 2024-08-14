/* snapshot.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

/* The extension for window snapshots */
#define	USE_DIR		"ENV:AmigaGuide"
#define	SAVE_DIR	"ENVARC:AmigaGuide"
#define	NAME		"AmigaGuide"

/* Simple scaling of coordinates */
#define SCALE(a,b,c) (((b) * (c)) / (a))

/*****************************************************************************/

struct TPrefs
{
    struct Prefs	tp_Prefs;
    UBYTE		tp_Pad;
};

/*****************************************************************************/

BOOL LoadSnapShot (struct AmigaGuideLib * ag, struct Client * cl)
{
    struct Rectangle rect;
    BOOL retval = FALSE;
    struct TPrefs tp;
    UBYTE name[128];
    ULONG mode;
    LONG len;

    /* Provide defaults */
    mode = GetVPModeID (&cl->cl_Screen->ViewPort);
    QueryOverscan (mode, &rect, OSCAN_TEXT);
    cl->cl_Prefs.p_Window.Top     = cl->cl_Screen->BarHeight + 1;
    cl->cl_Prefs.p_Window.Width   = MIN (cl->cl_Screen->Width, rect.MaxX - rect.MinX + 1) - 26;
    cl->cl_Prefs.p_Window.Height  = MIN (cl->cl_Screen->Height, rect.MaxY - rect.MinY + 1) - ((cl->cl_Prefs.p_Window.Top * 2) + 12);
    cl->cl_Prefs.p_FileReq.Top    = cl->cl_Prefs.p_Window.Top;
    cl->cl_Prefs.p_FileReq.Width  = 320;
    cl->cl_Prefs.p_FileReq.Height = cl->cl_Prefs.p_Window.Height;

    /* load user preferences */
#if 1
    sprintf (name, "%s/%s", NAME, cl->cl_ScreenNameBuffer);
#else
    sprintf (name, "%s/%s", NAME, cl->cl_Screen->DefaultTitle);
#endif

    len = GetVar (name, (char *) &tp, sizeof (struct TPrefs), GVF_GLOBAL_ONLY | GVF_BINARY_VAR | GVF_DONT_NULL_TERM);

    if (len == sizeof (struct Prefs))
    {
	/* see if the screen size has changed */
	if ((tp.tp_Prefs.p_SWidth != cl->cl_Screen->Width) || (tp.tp_Prefs.p_SHeight != cl->cl_Screen->Height))
	{
	    /* calculate new upper-left coordinates */
	    tp.tp_Prefs.p_Window.Left = (WORD) SCALE (tp.tp_Prefs.p_SWidth, cl->cl_Screen->Width, tp.tp_Prefs.p_Window.Left);
	    tp.tp_Prefs.p_Window.Top = (WORD) SCALE (tp.tp_Prefs.p_SHeight, cl->cl_Screen->Height, tp.tp_Prefs.p_Window.Top);

	    tp.tp_Prefs.p_Window.Width = (WORD) SCALE (tp.tp_Prefs.p_SWidth, cl->cl_Screen->Width, tp.tp_Prefs.p_Window.Width);
	    tp.tp_Prefs.p_Window.Height = (WORD) SCALE (tp.tp_Prefs.p_SHeight, cl->cl_Screen->Height, tp.tp_Prefs.p_Window.Height);
	}

	/* make sure we fit on the screen */
	if ((tp.tp_Prefs.p_Window.Left + tp.tp_Prefs.p_Window.Width) > cl->cl_Screen->Width)
	    tp.tp_Prefs.p_Window.Left -= (tp.tp_Prefs.p_Window.Left + tp.tp_Prefs.p_Window.Width - cl->cl_Screen->Width);
	if ((tp.tp_Prefs.p_Window.Top + tp.tp_Prefs.p_Window.Height) > cl->cl_Screen->Height)
	    tp.tp_Prefs.p_Window.Top -= (tp.tp_Prefs.p_Window.Top + tp.tp_Prefs.p_Window.Height - cl->cl_Screen->Height);

	/* Copy the structure */
	memcpy (&cl->cl_Prefs, &tp.tp_Prefs, sizeof (struct Prefs));

	retval = TRUE;
    }

    return (retval);
}

/*****************************************************************************/

BOOL SaveSnapShot (struct AmigaGuideLib * ag, struct Client * cl)
{
    struct Window *win = cl->cl_Window;
    struct Prefs *p = &cl->cl_Prefs;
    UBYTE name[128];
    BOOL retval;
    BPTR lock;

    p->p_SWidth = cl->cl_Screen->Width;
    p->p_SHeight = cl->cl_Screen->Height;

    if (!(cl->cl_Window->Flags & WFLG_ZOOMED))
    {
	p->p_Window = *((struct IBox *) & win->LeftEdge);
	p->p_Window.Width -= (win->BorderLeft + win->BorderRight + 4);
	p->p_Window.Height -= (win->BorderTop + win->BorderBottom + 2);
    }

    /* ENV: */
    if (lock = CreateDir (USE_DIR))
	UnLock (lock);
    if (lock = CreateDir (SAVE_DIR))
	UnLock (lock);
#if 1
    sprintf (name, "%s/%s", NAME, cl->cl_ScreenNameBuffer);
#else
    sprintf (name, "%s/%s", NAME, win->WScreen->DefaultTitle);
#endif
    retval = SetVar (name, (char *)p, sizeof (struct Prefs), GVF_GLOBAL_ONLY | GVF_SAVE_VAR | GVF_BINARY_VAR | GVF_DONT_NULL_TERM);

    return (retval);
}
