/* environment.c
 * Screen environment
 *
 */

#include "wdisplay.h"

BOOL OpenEnvironment (struct AppInfo * ai)
{
    BOOL retval = FALSE;
    ULONG modeid;
    ULONG extra;
    LONG key;

    /* Create the Intuition message port */
    if (ai->ai_IDCMP = CreatePort (0, NULL))
    {
	if (IntuitionBase->lib_Version >= 36)
	{
	    /* Lock the screen that they want */
	    if (ai->ai_Screen = LockPubScreen ((STRPTR) ai->ai_Options[OPT_PUBSCREEN]))
	    {
		/* Get the mode of the screen */
		modeid = GetVPModeID (&(ai->ai_Screen->ViewPort));

		/* Get the screen display information */
		if (GetDisplayInfoData (NULL, (UBYTE *) & ai->ai_Disp, sizeof (struct DisplayInfo), DTAG_DISP, modeid))
		{
		    /* Get the screen draw information */
		    if (ai->ai_DI = GetScreenDrawInfo (ai->ai_Screen))
		    {
			/* Bring the screen to front */
			ScreenToFront (ai->ai_Screen);

			/* Do we have Workbench? */
			if (WorkbenchBase)
			{
			    /* Create the AppWindow/AppMenu message port */
			    ai->ai_AWMPort = CreatePort (NULL, 0);
			}
		    }
		}
	    }
	}
	else
	{
	    /* Get a lock on the Workbench screen */
	    if (ai->ai_Screen = (struct Screen *) OpenWorkBench ())
	    {
		/* Lock Intuition */
		key = LockIBase (0);

		/* Get a pointer to the first screen */
		ai->ai_Screen = ((struct IntuitionBase *) (IntuitionBase))->FirstScreen;

		/* Step through screens */
		while (ai->ai_Screen)
		{
		    /* Is this the Workbench screen */
		    if (ai->ai_Screen->Flags & WBENCHSCREEN)
		    {
			ai->ai_Disp.Resolution.x = 22;
			ai->ai_Disp.Resolution.y = 52;
			if (ai->ai_Screen->Height >= 400)
			{
			    ai->ai_Disp.Resolution.y = 26;
			}
			break;
		    }

		    /* Get a pointer to the next screen */
		    ai->ai_Screen = ai->ai_Screen->NextScreen;
		}

		/* Unlock Intuition */
		UnlockIBase (key);
	    }
	}

	/* Do we have a screen yet? */
	if (ai->ai_Screen)
	{
	    if (ai->ai_TextFont = OpenFont (ai->ai_Screen->Font))
	    {
		/* Allocate the cursor */
		ai->ai_Cursor = AllocAnts ();

		/* Get a lock on the preference directory */
		ai->ai_Prefs = Lock ("ENV:", ACCESS_READ);

		/* Indicate success */
		retval = TRUE;
	    }

	    /* Create the secondary bitmap */
	    extra = (ULONG) (ai->ai_Screen->BitMap.Depth > 8 ? ai->ai_Screen->BitMap.Depth - 8 : 0);
	    ai->ai_BMapS = AllocVec (sizeof (struct BitMap) + (extra << 2), MEMF_CLEAR);
	}
    }

    return (retval);
}

VOID CloseEnvironment (struct AppInfo * ai)
{
    ILBM *ir = ai->ai_Picture;
    register WORD i;

    if (ir)
    {
	for(i = 0; i < ir->ir_NumAlloc; i++)
	{
	    FreePalette (ai->ai_Screen->ViewPort.ColorMap, ir->ir_Allocated[i]);
	}
	ir->ir_NumAlloc = 0;

	FreeILBM (ir);
    }

    /* NULL pointers are fine. */
    FreeAnts (ai->ai_Cursor);

    if (ai->ai_AWMPort)
    {
	DeletePort (ai->ai_AWMPort);
    }

    if (ai->ai_IDCMP)
    {
	DeletePort (ai->ai_IDCMP);
    }

    if (ai->ai_Prefs)
    {
	UnLock (ai->ai_Prefs);
    }

    if (ai->ai_DI)
    {
	FreeScreenDrawInfo (ai->ai_Screen, ai->ai_DI);
    }

    if (ai->ai_TextFont)
    {
	CloseFont (ai->ai_TextFont);
    }

    /* Free the secondary bitmap */
    FreeSecondaryBM (ai);

    /* Unlock the public screen */
    if (IntuitionBase->lib_Version >= 36)
    {
	/* Unlock the screen the public screen */
	if (ai->ai_Screen)
	{
	    UnlockPubScreen (NULL, ai->ai_Screen);
	    ai->ai_Screen = NULL;
	}
    }
}
