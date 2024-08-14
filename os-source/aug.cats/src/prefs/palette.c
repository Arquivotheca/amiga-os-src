#include "prefsbase.h"

void kprintf (void *, ...);
#define	DB(x)	;
#define	DD(x)	;

static VOID FillinPenSpec (struct PalettePref * pp);

struct PalettePref *
getpalettepref (BPTR drawer,STRPTR name,struct TagItem * attrs)
{
    struct Process *proc = (struct Process *) FindTask (NULL);
    struct ColorRegister pp_Colors[MAXCOLORS];
    struct PalettePref *pp;
    SHORT numc, depth;
    BPTR lock;
    LONG msize;

    if (lock = CurrentDir (drawer))
    {
	/* Clear the error field */
	proc->pr_Result2 = 0L;

	/* Compute the size of the preference structure */
	msize = sizeof (struct PalettePref);

	/* Allocate the preference structure */
	if (pp = (struct PalettePref *) AllocVec (msize, MEMF_CLEAR))
	{
	    struct IFFHandle *iff;

	    /* Allocate an IFF handle */
	    if (iff = AllocIFF ())
	    {
		/* Open the preference file */
		if (iff->iff_Stream = Open (name, MODE_OLDFILE))
		{
		    /* Indicate that the IFF handle is for a file */
		    InitIFFasDOS (iff);

		    /* Open the IFF handle for reading */
		    if (!(OpenIFF (iff, IFFF_READ)))
		    {
			/* Register where we want to stop */
			StopChunk (iff, ID_ILBM, ID_CMAP);

			/* Parse the file, stopping at the screen mode field */
			if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
			{
			    UWORD i, r, g, b;

			    /* Read in the preference structure */
			    pp->pp_NumColors =
			      ReadChunkRecords (iff, &pp_Colors, 3, MAXCOLORS);
			    DB (kprintf ("Read %ld colors\n", (LONG)pp->pp_NumColors));

			    /* Get the depth */
			    depth = (SHORT) GetTagData (PREFS_DEPTH, MAXCOLORS, attrs);
			    numc = (1 << depth);
			    DD (kprintf("ILBM colors %ld, ask %ld (depth %ld)\n",
				(LONG)pp->pp_NumColors, (LONG)numc, (LONG)depth));
			    pp->pp_NumColors = MIN (pp->pp_NumColors, numc);

			    /* Convert the colors */
			    for (i = 0; i < pp->pp_NumColors; i++)
			    {
				r = (pp_Colors[i].red >> 4);
				g = (pp_Colors[i].green >> 4);
				b = (pp_Colors[i].blue >> 4);
				pp->pp_CRegs[i] = (r << 8) | (g << 4) | (b);
			    }

			    /* Prefill the penspec */
			    pp->pp_Pens[0] = 1;	/* Detail */
			    pp->pp_Pens[1] = 0;	/* Block */
			    pp->pp_Pens[2] = 1;	/* Text */
			    pp->pp_Pens[3] = 2;	/* Shine */
			    pp->pp_Pens[4] = 1;	/* Shadow */
			    pp->pp_Pens[5] = 3;	/* Hifill */
			    pp->pp_Pens[6] = 1;	/* Hifill text */
			    pp->pp_Pens[7] = 0;	/* Background */
			    pp->pp_Pens[8] = 2;	/* Hilight text */
			    pp->pp_Pens[9] = ~0;

#if 0
			    pp->pp_NumColors = 4;
#endif

			    FillinPenSpec (pp);
			}
			else
			{
			    /* Wrong kind of preference file */
			    proc->pr_Result2 = ERROR_OBJECT_WRONG_TYPE;
			}

			/* Close the IFF handle */
			CloseIFF (iff);
		    }

		    /* Close the file */
		    Close (iff->iff_Stream);
		}

		/* Free the IFF handle */
		FreeIFF (iff);
	    }
	    else
	    {
		proc->pr_Result2 = ERROR_NO_FREE_STORE;
	    }

	    /* See if we were able to read the pref file */
	    if (proc->pr_Result2)
	    {
		/* An error occurred, free the structure */
		FreeVec ((APTR) pp);
		pp = NULL;
	    }

	}
	else
	{
	    /* Not enough memory */
	    proc->pr_Result2 = ERROR_NO_FREE_STORE;
	}

	CurrentDir (lock);
    }

    return (pp);
}

/*
 * These define the amount of Red, Green, and Blue scaling used
 * to help take into account the different visual impact of those
 * colors on the screen.
 */
#define	BLUE_SCALE	2
#define	GREEN_SCALE	6
#define	RED_SCALE	3

/*
 * This returns the color difference hamming value...
 */
static SHORT ColorDifference (UWORD rgb0, UWORD rgb1)
{
    register SHORT level;
    register SHORT tmp;

    tmp = (rgb0 & 15) - (rgb1 & 15);
    level = tmp * tmp * BLUE_SCALE;
    tmp = ((rgb0 >> 4) & 15) - ((rgb1 >> 4) & 15);
    level += tmp * tmp * GREEN_SCALE;
    tmp = ((rgb0 >> 8) & 15) - ((rgb1 >> 8) & 15);
    level += tmp * tmp * RED_SCALE;
    return (level);
}

/*
 * Calculate a rough brightness hamming value...
 */
static SHORT ColorLevel (UWORD rgb)
{

    return (ColorDifference (rgb, 0));
}

static VOID FillinPenSpec (struct PalettePref * pp)
{
    register SHORT loop;
    register SHORT loop1;
    register SHORT backpen;
    register SHORT tmp;
    SHORT colors[MAXCOLORS];
    SHORT colorlevels[MAXCOLORS];
    SHORT pens[MAXCOLORS];

    if (pp->pp_NumColors < 3)
    {
	pp->pp_Pens[detailPen] = 0;
	pp->pp_Pens[blockPen] = 1;
	pp->pp_Pens[textPen] = 1;
	pp->pp_Pens[shinePen] = 1;
	pp->pp_Pens[shadowPen] = 1;
	pp->pp_Pens[hilighttextPen] = 1;
	pp->pp_Pens[backgroundPen] = 0;
    }
    else
    {
	for (loop = 0; loop < pp->pp_NumColors; loop++)
	{
	    colors[loop] = pp->pp_CRegs[loop];
	    colorlevels[loop] = ColorLevel (colors[loop]);
	    pens[loop] = loop;
	}

	/* Sort darkest to brightest... */
	for (loop = 0; loop < (pp->pp_NumColors - 1); loop++)
	    for (loop1 = loop + 1; loop1 < pp->pp_NumColors; loop1++)
	    {
		if (colorlevels[loop] > colorlevels[loop1])
		{
		    tmp = colorlevels[loop];
		    colorlevels[loop] = colorlevels[loop1];
		    colorlevels[loop1] = tmp;

		    tmp = colors[loop];
		    colors[loop] = colors[loop1];
		    colors[loop1] = tmp;

		    tmp = pens[loop];
		    pens[loop] = pens[loop1];
		    pens[loop1] = tmp;
		}
	    }

	/* Now, pick the pens... HightLight... */
	loop = pp->pp_NumColors - 1;
	while (!(pp->pp_Pens[shinePen] = pens[loop--])) ;

	/* and Shadow... */
	loop = 0;
	while (!(pp->pp_Pens[shadowPen] = pens[loop++])) ;

	/* The BackGround pen... */
	if (!pens[loop])
	    loop++;
	pp->pp_Pens[hifillPen] = pens[backpen = loop];

	loop1 = 0;
	for (loop = 0; loop < pp->pp_NumColors; loop++)
	{
	    tmp = ColorDifference (colors[loop], colors[backpen]);
	    if (tmp > loop1)
	    {
		loop1 = tmp;
		pp->pp_Pens[hifilltextPen] = pens[loop];
	    }
	}

	backpen=0;
	while (pens[backpen]) backpen++;

	loop1 = 0;
	for (loop = 0; loop < pp->pp_NumColors; loop++)
	{
	    tmp = ColorDifference (colors[loop], colors[backpen]);
	    if ((tmp > loop1) && (pens[loop]!=1))
	    {
		loop1 = tmp;
		pp->pp_Pens[hilighttextPen] = pens[loop];
	    }
	}

	pp->pp_Pens[blockPen] = pp->pp_Pens[textPen];
	pp->pp_Pens[detailPen] = pp->pp_Pens[backgroundPen];
    }
}

struct PalettePref * GetPalettePref (
	BPTR drawer,
	STRPTR name,
	struct PalettePref *opp,
	struct TagItem * attrs)
{
    struct LibBase *lb = ((struct ExtLibrary *) PrefsBase)->el_Base;
    struct Process *proc = (struct Process *) FindTask (NULL);
    LONG msize = sizeof (struct PalettePref);
    struct PalettePref *pp = NULL;

    /* First try getting it from the passed directory */
    if (!(pp = getpalettepref (drawer, name, attrs)))
    {
	/* If the file wasn't found, then try in the system drawer */
	if (!(CompairLocks(drawer, lb->lb_SysPrefs)))
	{
	    /* Try getting the file from the system drawer */
	    if (pp = getpalettepref (lb->lb_SysPrefs, name, attrs))
	    {
		struct Prefs *p = &(pp->pp_Header);

		/* Tell where we came from */
		p->p_Flags |= PREFS_SYSTEM_F;
	    }
	}
    }

    /* See if we don't have a preference record due to no file */
    if (proc->pr_Result2 == ERROR_OBJECT_NOT_FOUND)
    {
	/* Allocate a default preference file */
	if (pp = (struct PalettePref *) AllocVec (msize, MEMF_CLEAR))
	{
	    struct Prefs *p = &(pp->pp_Header);

	    /* Tell where we came from */
	    p->p_Flags |= PREFS_DEFAULT_F;

	    /* Default color palette */
	    pp->pp_CRegs[0] = 0xAAA;
	    pp->pp_CRegs[1] = 0x000;
	    pp->pp_CRegs[2] = 0xFFF;
	    pp->pp_CRegs[3] = 0x68B;

	    /* Number of colors */
	    pp->pp_NumColors = 4;

	    /* Prefill the penspec */
	    pp->pp_Pens[0] = 1;	/* Detail */
	    pp->pp_Pens[1] = 0;	/* Block */
	    pp->pp_Pens[2] = 1;	/* Text */
	    pp->pp_Pens[3] = 2;	/* Shine */
	    pp->pp_Pens[4] = 1;	/* Shadow */
	    pp->pp_Pens[5] = 3;	/* Hifill */
	    pp->pp_Pens[6] = 1;	/* Hifill text */
	    pp->pp_Pens[7] = 0;	/* Background */
	    pp->pp_Pens[8] = 2;	/* Hilight text */
	    pp->pp_Pens[9] = ~0;
	}
    }

    /* See if we're freshening */
    if (opp && pp)
    {
	VOID FreePref (VOID *);
	struct Prefs *op = &(opp->pp_Header);
	WORD i;

	/* Clear the where from flags */
	op->p_Flags &= ~PREFS_WHERE_F;

	/* Copy the number of colors */
	opp->pp_NumColors = pp->pp_NumColors;

	/* Copy the colors */
	for (i = 0; i < opp->pp_NumColors; i++)
	    opp->pp_CRegs[i] = pp->pp_CRegs[i];

	/* Copy & inspect the penspec */
	for (i = 0; i < 9; i++)
	{
	    /* Compare the pens */
	    if (pp->pp_Pens[i] != opp->pp_Pens[i])
	    {
		/* Found a reason to shutdown */
		op->p_Flags |= PREFS_CLOSEALL_F;
	    }

	    /* Copy the penspec */
	    opp->pp_Pens[i] = pp->pp_Pens[i];
	}

	/* Free the new preference file */
	FreePref (pp);

	/* Return the same old preference file with new information */
	pp = opp;
    }

    return (pp);
}
