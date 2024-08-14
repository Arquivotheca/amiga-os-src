/* about.c
 *
 */

#include "wdisplay.h"
#include "wdisplay_rev.h"

#define	ABOUT_IDCMP	IDCMP_CLOSEWINDOW

extern struct TagItem WA_Tags[];

struct ExtNewWindow AboutNW =
{
    0, 0, 320, 0, 0, 1,

    NULL,

    WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE |
    WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH | WFLG_RMBTRAP | WFLG_NW_EXTENDED,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    5, 5,
    -1, -1,
    CUSTOMSCREEN,
    WA_Tags
};

#define	ABOUT_NAME	0
#define	ABOUT_COPYRIGHT	1
#define	ABOUT_AUTHOR	2
#define	ABOUT_PICTURE	3
#define	ABOUT_COUNT	4

struct AboutWork
{
    struct IntuiText aw_IText[ABOUT_COUNT];
    UBYTE aw_Text[ABOUT_COUNT][50];
};

#define	AWSIZE	sizeof(struct AboutWork)

LONG OpenAboutWindow (struct AppInfo * ai)
{
    register WORD i, j, k;
    struct AboutWork *aw;
    struct NewWindow *nw;

    if (!(ai->ai_InfoWin))
    {
	/* Get a pointer to the NewWindow structure */
	nw = &AboutNW;

	/* Get the window title */
	nw->Title = GetWDisplayString (ai, MISC_ABOUT_TITLE);

	/* Set the screen */
	nw->Screen = ai->ai_Screen;

	/* Restore the position */
	nw->LeftEdge = ai->ai_Window->LeftEdge + ai->ai_InfoLeft;
	nw->TopEdge = ai->ai_Window->TopEdge + ai->ai_InfoTop;
	nw->Height = ai->ai_Screen->BarHeight + (ABOUT_COUNT * 9) + 16;

	/* open the window */
	if (ai->ai_InfoWin = OpenWindow (nw))
	{
	    /* set up the message port information */
	    ai->ai_InfoWin->UserPort = ai->ai_IDCMP;
	    ModifyIDCMP (ai->ai_InfoWin, ABOUT_IDCMP);
	}
    }

    if (ai->ai_InfoWin)
    {
	/* Bring the window to the front */
	WindowToFront (ai->ai_InfoWin);

	/* Allocate the work buffer */
	if (aw = AllocMem (AWSIZE, MEMF_CLEAR))
	{
	    /* Compute the left edge of each line */
	    j = ai->ai_InfoWin->Width - ai->ai_InfoWin->BorderLeft - ai->ai_InfoWin->BorderRight;
	    for (i = 0, k = 2; i < ABOUT_COUNT; i++, k += 9)
	    {
		switch (i)
		{
		    case ABOUT_NAME:
			/* Set the name and version */
			sprintf (aw->aw_Text[i], "Window Display %d.%d (%s)", VERSION, REVISION, DATE);
			break;

		    case ABOUT_COPYRIGHT:
			/* Build the copyright notice */
			sprintf (aw->aw_Text[i], "%s © 1991 Commodore-Amiga, Inc.", GetWDisplayString (ai, MISC_COPYRIGHT));
			break;

		    case ABOUT_AUTHOR:
			sprintf (aw->aw_Text[i], "%s David N. Junod", GetWDisplayString (ai, MISC_WRITTEN_BY));
			break;

		    case ABOUT_PICTURE:
			/* Set the picture information */
			if (ai->ai_Picture)
			{
			    sprintf (aw->aw_Text[ABOUT_PICTURE], "    %d x %d x %d    ",
				     ai->ai_Picture->ir_BMHD.w,
				     ai->ai_Picture->ir_BMHD.h,
				     ai->ai_Picture->ir_NumColors);
			}
			else
			{
			    strcpy (aw->aw_Text[ABOUT_PICTURE], "    <no picture>    ");
			}
			k += 8;
			break;
		}

		/* Fill out the IntuiText */
		aw->aw_IText[i].FrontPen = 1;
		aw->aw_IText[i].BackPen = 0;
		aw->aw_IText[i].DrawMode = JAM2;
		aw->aw_IText[i].TopEdge = k;
		aw->aw_IText[i].ITextFont = &Topaz80;
		aw->aw_IText[i].IText = aw->aw_Text[i];

		aw->aw_IText[i].LeftEdge = (j - IntuiTextLength (&aw->aw_IText[i])) / 2;

		if (i < (ABOUT_COUNT - 1))
		{
		    aw->aw_IText[i].NextText = &aw->aw_IText[(i + 1)];
		}
	    }

	    /* Display the text */
	    PrintIText (ai->ai_InfoWin->RPort, aw->aw_IText, ai->ai_InfoWin->BorderLeft, ai->ai_InfoWin->BorderTop);

	    FreeMem (aw, AWSIZE);
	}
    }

    return (0L);
}

VOID UpdateAboutWindow (struct AppInfo * ai)
{

    if (ai->ai_InfoWin)
    {
	OpenAboutWindow (ai);
    }
}

VOID CloseAboutWindow (struct AppInfo * ai)
{

    /* Is the window open? */
    if (ai->ai_InfoWin)
    {
	/* Remember the position */
	ai->ai_InfoLeft = ai->ai_InfoWin->LeftEdge - ai->ai_Window->LeftEdge;
	ai->ai_InfoTop = ai->ai_InfoWin->TopEdge - ai->ai_Window->TopEdge;

	/* Close the window */
	CloseWindowSafely (ai->ai_InfoWin);
	ai->ai_InfoWin = NULL;
    }
}
