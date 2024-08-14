/* snapshot.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DT(x)	;

/*****************************************************************************/

/* The extension for window snapshots */
#define	USE_DIR		"ENV:MultiView"
#define	SAVE_DIR	"ENVARC:MultiView"

#define	NAME		"MultiView"

/* Simple scaling of coordinates */
#define SCALE(a,b,c) (((b) * (c)) / (a))

/*****************************************************************************/

struct TPrefs
{
    struct Prefs	tp_Prefs;
    UBYTE		tp_Pad;
};

/*****************************************************************************/

BOOL LoadSnapShot (struct GlobalData * gd)
{
    struct Rectangle rect;
    BOOL retval = FALSE;
    struct TPrefs tp;
    UBYTE name[128];
    ULONG mode;
    LONG len;

    /* Provide defaults */
    mode = GetVPModeID (&gd->gd_Screen->ViewPort);
    QueryOverscan (mode, &rect, OSCAN_TEXT);
    gd->gd_Prefs.p_Window.Top     = gd->gd_Screen->BarHeight + 1;
    gd->gd_Prefs.p_Window.Width   = MIN (gd->gd_Screen->Width, rect.MaxX - rect.MinX + 1) - 26;
    gd->gd_Prefs.p_Window.Height  = MIN (gd->gd_Screen->Height, rect.MaxY - rect.MinY + 1) - ((gd->gd_Prefs.p_Window.Top * 2) + 12);
    gd->gd_Prefs.p_FileReq.Top    = gd->gd_Prefs.p_Window.Top;
    gd->gd_Prefs.p_FileReq.Width  = 320;
    gd->gd_Prefs.p_FileReq.Height = gd->gd_Prefs.p_Window.Height;

    /* load user preferences */
#if 1
    DT (kprintf ("load \"%s\"\n", gd->gd_ScreenNameBuffer));
    sprintf (name, "%s/%s", NAME, gd->gd_ScreenNameBuffer);
#else
    sprintf (name, "%s/%s", NAME, gd->gd_Screen->DefaultTitle);
#endif

    len = GetVar (name, (char *) &tp, sizeof (struct TPrefs), GVF_GLOBAL_ONLY | GVF_BINARY_VAR | GVF_DONT_NULL_TERM);

    DB (kprintf ("%ld=GetVar (%ld)\n", len, sizeof (struct Prefs)));

    if (len == sizeof (struct Prefs))
    {
	/* see if the screen size has changed */
	if ((tp.tp_Prefs.p_SWidth != gd->gd_Screen->Width) || (tp.tp_Prefs.p_SHeight != gd->gd_Screen->Height))
	{
	    /* calculate new upper-left coordinates */
	    tp.tp_Prefs.p_Window.Left = (WORD) SCALE (tp.tp_Prefs.p_SWidth, gd->gd_Screen->Width, tp.tp_Prefs.p_Window.Left);
	    tp.tp_Prefs.p_Window.Top = (WORD) SCALE (tp.tp_Prefs.p_SHeight, gd->gd_Screen->Height, tp.tp_Prefs.p_Window.Top);

	    tp.tp_Prefs.p_Window.Width = (WORD) SCALE (tp.tp_Prefs.p_SWidth, gd->gd_Screen->Width, tp.tp_Prefs.p_Window.Width);
	    tp.tp_Prefs.p_Window.Height = (WORD) SCALE (tp.tp_Prefs.p_SHeight, gd->gd_Screen->Height, tp.tp_Prefs.p_Window.Height);
	}

	/* make sure we fit on the screen */
	if ((tp.tp_Prefs.p_Window.Left + tp.tp_Prefs.p_Window.Width) > gd->gd_Screen->Width)
	    tp.tp_Prefs.p_Window.Left -= (tp.tp_Prefs.p_Window.Left + tp.tp_Prefs.p_Window.Width - gd->gd_Screen->Width);
	if ((tp.tp_Prefs.p_Window.Top + tp.tp_Prefs.p_Window.Height) > gd->gd_Screen->Height)
	    tp.tp_Prefs.p_Window.Top -= (tp.tp_Prefs.p_Window.Top + tp.tp_Prefs.p_Window.Height - gd->gd_Screen->Height);

	/* Copy the structure */
	memcpy (&gd->gd_Prefs, &tp.tp_Prefs, sizeof (struct Prefs));

	retval = TRUE;
    }

    return (retval);
}

/*****************************************************************************/

BOOL SaveSnapShot (struct GlobalData * gd)
{
    struct Window *win = gd->gd_Window;
    struct Prefs *p = &gd->gd_Prefs;
    UBYTE name[128];
    BOOL retval;
    BPTR lock;

    p->p_SWidth = gd->gd_Screen->Width;
    p->p_SHeight = gd->gd_Screen->Height;

    if (!(gd->gd_Window->Flags & WFLG_ZOOMED))
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
    DT (kprintf ("save \"%s\"\n", gd->gd_ScreenNameBuffer));
    sprintf (name, "%s/%s", NAME, gd->gd_ScreenNameBuffer);
#else
    sprintf (name, "%s/%s", NAME, win->WScreen->DefaultTitle);
#endif
    retval = SetVar (name, (char *)p, sizeof (struct Prefs), GVF_GLOBAL_ONLY | GVF_SAVE_VAR | GVF_BINARY_VAR | GVF_DONT_NULL_TERM);

    DB (kprintf ("%ld=SetVar (%ld)\n", len, sizeof (struct Prefs)));

    return (retval);
}
