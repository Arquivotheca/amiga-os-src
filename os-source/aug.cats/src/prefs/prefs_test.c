/* prefs_test.c
 * Test the prefs.library
 * Written by David N. Junod, 14-Sept-90
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/notify.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/displayinfo.h>
#include <libraries/prefs_lib.h>
#include <utility/tagitem.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec.h>
#include <pragmas/dos.h>
#include <pragmas/intuition.h>
#include <pragmas/graphics.h>
#include <pragmas/utility.h>
#include <pragmas/prefs.h>

/* prefs_test.c: function prototypes */
BOOL OpenLibraries (VOID);
VOID CloseLibraries (VOID);
struct Screen *OpenPreferScreen (struct NewScreen *, struct List *, struct TagItem * attrs);
VOID CloseEnv (VOID);
BOOL OpenEnv (VOID);
VOID FreshenColors (VOID * pr);
VOID FreshenScreen (VOID * s);
VOID FreshenPointer (VOID * p);
VOID SetDefPointer (struct Window *win, struct List *list);
VOID SetWaitPointer (struct Window *win, struct List *list);
int main (int argc, char **argv);

extern struct Library *SysBase, *DOSBase;
struct Library *IntuitionBase, *GfxBase, *PrefsBase, *UtilityBase;

struct Screen *scr;
struct Window *win;
struct MsgPort *mp;
struct List pref_list;
BPTR pref_drawer;

main (int argc, char **argv)
{
    struct TagItem attrs[4];

    NewList (&(pref_list));

    if (OpenLibraries ())
    {
	if (mp = CreatePort (NULL, NULL))
	{
	    /* See if an argument was passed */
	    if (argc >= 2)
	    {
		if (!(pref_drawer = GetPrefsDrawer (argv[1], PREFS_FALLBACK_GPDF)))
		{
		    printf ("unable to get the preferences drawer\n");
		}
	    }

	    /* Set up the attributes */
	    attrs[0].ti_Tag = PREFS_NOTIFY_A;
	    attrs[0].ti_Data = (LONG) mp;
	    attrs[1].ti_Tag = PREFS_CALLBACK_A;
	    attrs[2].ti_Tag = PREFS_LIST_A;
	    attrs[2].ti_Data = (LONG) & (pref_list);
	    attrs[3].ti_Tag = TAG_DONE;
	    attrs[3].ti_Data = NULL;

	    /* Get the color palette */
	    attrs[1].ti_Data = (LONG) FreshenColors;
	    GetPref (PREFS_PALETTE, pref_drawer, attrs);

	    /* Get the screen mode */
	    attrs[1].ti_Data = (LONG) FreshenScreen;
	    GetPref (PREFS_SCREENMODE, pref_drawer, attrs);

	    /* Get the default pointer */
	    attrs[1].ti_Data = (LONG) FreshenPointer;
	    GetPref (PREFS_POINTER,    pref_drawer, attrs);

	    /* Get the busy pointer */
	    GetPref (PREFS_BUSYPOINTER,pref_drawer, attrs);

	    /* Open the test environment */
	    if (OpenEnv ())
	    {
		struct NotifyMessage *nmsg;
		struct IntuiMessage *msg;
		BOOL going = TRUE;
		LONG sigs;

		while (going && scr)
		{
		    /* Update the signal bits that we're waiting on */
		    sigs = (1L << win->UserPort->mp_SigBit);
		    sigs |= (1L << mp->mp_SigBit);

		    /* Wait for something to happen */
		    Wait (sigs);

		    /* Handle the window messages */
		    while (msg = (struct IntuiMessage *) GetMsg (win->UserPort))
		    {
			switch (msg->Class)
			{
			case CLOSEWINDOW:
			    going = FALSE;
			    break;
			case VANILLAKEY:
			    if (msg->Code == 'w')
				SetWaitPointer (win, &(pref_list));
			    if (msg->Code == 'd')
				SetDefPointer (win, &(pref_list));
			}

			ReplyMsg ((struct Message *) msg);
		    }

		    /* Handle the notification messages */
		    while (nmsg = (struct NotifyMessage *) GetMsg (mp))
		    {
			struct NotifyRequest *nr;
			struct Prefs *p;

			nr = nmsg->nm_NReq;
			p = &(((struct ScreenModePref *) nr->nr_UserData)->smp_Header);

			(*(p->p_Func)) ((VOID *) nr->nr_UserData);

			ReplyMsg ((struct Message *) nmsg);
		    }
		}

		/* Close our test environment */
		CloseEnv ();
	    }

	    /* Free whatever preference records we have */
	    FreePrefList (&(pref_list));

	    /* Unlock our preference drawer */
	    if (pref_drawer)
	    {
		UnLock (pref_drawer);
		pref_drawer = NULL;
	    }

	    /* Get rid of the notification port */
	    DeletePort (mp);
	}

	CloseLibraries ();
    }
}

BOOL OpenLibraries (VOID)
{

    if (IntuitionBase = OpenLibrary ("intuition.library", 36))
    {
	if (GfxBase = OpenLibrary ("graphics.library", 36))
	{
	    if (UtilityBase = OpenLibrary ("utility.library", 36))
	    {
		if (PrefsBase = OpenLibrary ("prefs.library", 36))
		{
		    return (TRUE);
		}
		CloseLibrary (UtilityBase);
	    }
	    CloseLibrary (GfxBase);
	}
	CloseLibrary (IntuitionBase);
    }
    return (FALSE);
}

VOID CloseLibraries (VOID)
{

    if (PrefsBase)
	CloseLibrary (PrefsBase);

    if (UtilityBase)
	CloseLibrary (UtilityBase);

    if (GfxBase)
	CloseLibrary (GfxBase);

    if (IntuitionBase)
	CloseLibrary (IntuitionBase);
}

struct Screen *OpenPreferScreen (
				     struct NewScreen * ns,
				     struct List * pref_list,
				     struct TagItem * attrs)
{
    struct Screen *scr = NULL;
    struct ScreenModePref *smp;
    struct PalettePref *pp;
    struct TagItem tag[6], *clone;

    /* Clone the list so that we can free it later */
    if (attrs)
    {
	clone = CloneTagItems (attrs);
    }

    /* See if we have palette preference to honor */
    if (pp = GetPrefRecord (pref_list, PREFS_PALETTE))
    {
	tag[0].ti_Tag = SA_Pens;
	tag[0].ti_Data = (LONG) pp->pp_Pens;
	tag[1].ti_Tag = SA_FullPalette;
	tag[1].ti_Data = TRUE;
	tag[2].ti_Tag = TAG_MORE;
	tag[2].ti_Data = (LONG) clone;

	/* To get one contiguous tag list */
	clone = CloneTagItems (tag);
    }

    /* See if we have screen mode preference to honor */
    if (smp = GetPrefRecord (pref_list, PREFS_SCREENMODE))
    {
	tag[0].ti_Tag = SA_DisplayID;
	tag[0].ti_Data = (LONG) smp->smp_ModeID;
	tag[1].ti_Tag = SA_Width;
	tag[1].ti_Data = (LONG) smp->smp_Width;
	tag[2].ti_Tag = SA_Height;
	tag[2].ti_Data = (LONG) smp->smp_Height;
	tag[3].ti_Tag = SA_Depth;
	tag[3].ti_Data = (LONG) smp->smp_Depth;
	tag[4].ti_Tag = SA_AutoScroll;
	tag[4].ti_Data = (LONG) smp->smp_AutoScroll;
	tag[5].ti_Tag = TAG_MORE;
	tag[5].ti_Data = (LONG) clone;

	/* To get one contiguous tag list */
	clone = CloneTagItems (tag);
    }

    /* Open up that screen */
    if (scr = OpenScreenTagList (ns, clone))
    {
	if (pp)
	{
	    /* Set the screen colors */
	    LoadRGB4 (&(scr->ViewPort), pp->pp_CRegs, pp->pp_NumColors);
	}
    }

    /* Get rid of our clone */
    if (clone)
	FreeTagItems (clone);

    return (scr);
}

BOOL OpenEnv (VOID)
{
    struct TagItem tag[3];

    /* Make sure the environment isn't already open */
    CloseEnv ();

    /* Provide some additional tags */
    tag[0].ti_Tag = SA_Title;
    tag[0].ti_Data = (LONG) "Test Screen";
    tag[1].ti_Tag = SA_PubName;
    tag[1].ti_Data = (LONG) "PREFS";
    tag[2].ti_Tag = TAG_DONE;

    /* Get the screen */
    if (scr = OpenPreferScreen (NULL, &(pref_list), tag))
    {

	/* Open our window */
	if (win = OpenWindowTags (NULL,
				  WA_CustomScreen, scr,
				  WA_Top, (scr->BarHeight + 1),
				  WA_Width, 320,
				  WA_Height, 100,
				  WA_IDCMP, (CLOSEWINDOW | VANILLAKEY),
				  WA_Flags, WINDOWCLOSE | WINDOWDRAG | ACTIVATE,
				  WA_Title, (LONG) "Test Window",
				  TAG_DONE))
	{
	    struct PointerPref *pp;

	    /* Get the pointer preference */
	    if (pp = GetPrefRecord (&(pref_list), PREFS_POINTER))
	    {
		/* Set the pointer */
		SetPointer (win, pp->pp_PData,
			    pp->pp_Height, pp->pp_Width,
			    pp->pp_XOffset, pp->pp_YOffset);
	    }

	    return (TRUE);
	}

	CloseScreen (scr);
	scr = NULL;
    }

    return (FALSE);
}

VOID CloseEnv (VOID)
{

    if (win)
	CloseWindow (win);
    win = NULL;
    if (scr)
	CloseScreen (scr);
    scr = NULL;
}

VOID FreshenColors (VOID * pr)
{
    struct PalettePref *pp = (struct PalettePref *) pr;
    struct TagItem tag[2];
    struct Prefs *p;

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) pp;
    tag[1].ti_Tag = TAG_DONE;

    GetPref (PREFS_PALETTE, pref_drawer, tag);

    p = &(pp->pp_Header);

    if (p->p_Flags & PREFS_CLOSEALL_F)
    {
	/* Gotta change everything */
	OpenEnv ();
    }
    else
    {
	/* Set the screen colors */
	LoadRGB4 (&(scr->ViewPort), pp->pp_CRegs, pp->pp_NumColors);
    }
}

VOID FreshenScreen (VOID * s)
{
    struct ScreenModePref *smp = (struct ScreenModePref *) s;
    struct TagItem tag[2];
    struct Prefs *p;

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) smp;
    tag[1].ti_Tag = TAG_DONE;

    GetPref (PREFS_SCREENMODE, pref_drawer, tag);

    p = &(smp->smp_Header);

    if (p->p_Flags & PREFS_CLOSEALL_F)
    {
	/* Gotta change everything */
	OpenEnv ();
    }
}

VOID FreshenPointer (VOID * s)
{
    struct PointerPref *pp = (struct PointerPref *) s;
    struct TagItem tag[2];

    /* Say that we want an update */
    tag[0].ti_Tag = PREFS_FRESHEN_A;
    tag[0].ti_Data = (LONG) pp;
    tag[1].ti_Tag = TAG_DONE;

    GetPref (NULL, pref_drawer, tag);
}

VOID SetDefPointer (struct Window *win, struct List *list)
{
    struct PointerPref *pp;

    if (win && (pp = GetPrefRecord (list, PREFS_POINTER)))
    {
	/* Set the pointer */
	SetPointer (win, pp->pp_PData,
		    pp->pp_Height, pp->pp_Width,
		    pp->pp_XOffset, pp->pp_YOffset);
    }
}

VOID SetWaitPointer (struct Window *win, struct List *list)
{
    struct PointerPref *pp;

    if (win && (pp = GetPrefRecord (list, PREFS_BUSYPOINTER)))
    {
	/* Set the pointer */
	SetPointer (win, pp->pp_PData,
		    pp->pp_Height, pp->pp_Width,
		    pp->pp_XOffset, pp->pp_YOffset);
    }
}
