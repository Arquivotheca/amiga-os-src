/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 *                          DiskSpeed v4.2
 *                          ScsiSpeed v4.2
 *                                by
 *                           Michael Sinz
 *
 *            Copyright (c) 1989-1992 by MKSoft Development
 *
 *			MKSoft Development
 *			163 Appledore Drive
 *			Downingtown, PA 19335
 *
 * Yes, this is yet another disk speed testing program, but with a few
 * differences.  It was designed to give the most accurate results of the
 * true disk performance in the system.  For this reason many of
 * DiskSpeed's results may look either lower or higher than current disk
 * performance tests.
 *
 ******************************************************************************
 *									      *
 *	Reading legal mush can turn your brain into guacamole!		      *
 *									      *
 *		So here is some of that legal mush:			      *
 *									      *
 * Permission is hereby granted to distribute this program's source	      *
 * executable, and documentation for non-commercial purposes, so long as the  *
 * copyright notices are not removed from the sources, executable or	      *
 * documentation.  This program may not be distributed for a profit without   *
 * the express written consent of the author Michael Sinz.		      *
 *									      *
 * This program is not in the public domain.				      *
 *									      *
 * Fred Fish is expressly granted permission to distribute this program's     *
 * source and executable as part of the "Fred Fish freely redistributable     *
 * Amiga software library."						      *
 *									      *
 * Permission is expressly granted for this program and it's source to be     *
 * distributed as part of the Amicus Amiga software disks, and the	      *
 * First Amiga User Group's Hot Mix disks.				      *
 *									      *
 ******************************************************************************
 */

/*
 * This file contains the definition of the rendering information
 * for elements on the screen.  This information is used to generate
 * the correct pen colours for items on the screen...
 */

#include	<exec/types.h>
#include	<exec/memory.h>
#include	<graphics/view.h>
#include	<graphics/text.h>
#include	<intuition/intuition.h>
#include	<intuition/screens.h>

#include	<proto/exec.h>
#include	<proto/intuition.h>
#include	<proto/graphics.h>

#include	"RenderInfo.h"

/*
 * These define the amount of Red, Green, and Blue scaling used
 * to help take into account the different visual impact of those
 * colours on the screen.
 */
#define	BLUE_SCALE	2
#define	GREEN_SCALE	6
#define	RED_SCALE	3

#define	MAX_COLOURS	16

/*
 * This returns the colour difference hamming value...
 */
SHORT ColourDifference(UWORD rgb0, UWORD rgb1)
{
register	SHORT	level;
register	SHORT	tmp;

	tmp=(rgb0 & 15) - (rgb1 & 15);
	level=tmp*tmp*BLUE_SCALE;
	tmp=((rgb0>>4) & 15) - ((rgb1>>4) & 15);
	level+=tmp*tmp*GREEN_SCALE;
	tmp=((rgb0>>8) & 15) - ((rgb1>>8) & 15);
	level+=tmp*tmp*RED_SCALE;
	return(level);
}

/*
 * Calculate a rough brightness hamming value...
 */
#define	ColourLevel(x)	ColourDifference(x,0)

/*
 * For new programs, this also opens fonts...
 */
static VOID NewFillIn_RenderInfo(struct RenderInfo *ri,struct Screen *TheScreen)
{
register	SHORT		numcolours;
register	SHORT		loop;
register	SHORT		loop1;
register	SHORT		backpen;
register	SHORT		tmp;
		SHORT		colours[MAX_COLOURS];
		SHORT		colourlevels[MAX_COLOURS];
		SHORT		pens[MAX_COLOURS];
	struct	Screen		screen;

	if (!TheScreen) GetScreenData((APTR)(TheScreen=&screen),sizeof(struct Screen),WBENCHSCREEN,NULL);

	ri->WindowTitle=TheScreen->BarHeight-TheScreen->BarVBorder+TheScreen->WBorTop;

	numcolours=1 << (TheScreen->RastPort.BitMap->Depth);
	if (numcolours>MAX_COLOURS) numcolours=MAX_COLOURS;

	if (numcolours<3)
	{	/* Some silly person is running with 2 colours... */
		ri->BackPen=0;
		ri->Highlight=1;
		ri->Shadow=1;
		ri->TextPen=1;
	}
	else
	{
		for (loop=0;loop<numcolours;loop++)
		{
			colours[loop]=GetRGB4(TheScreen->ViewPort.ColorMap,(LONG)loop);
			colourlevels[loop]=ColourLevel(colours[loop]);
			pens[loop]=loop;
		}

		/* Sort darkest to brightest... */
		for (loop=0;loop<(numcolours-1);loop++)
		{
			for (loop1=loop+1;loop1<numcolours;loop1++)
			{
				if (colourlevels[loop]>colourlevels[loop1])
				{
					tmp=colourlevels[loop];
					colourlevels[loop]=colourlevels[loop1];
					colourlevels[loop1]=tmp;

					tmp=colours[loop];
					colours[loop]=colours[loop1];
					colours[loop1]=tmp;

					tmp=pens[loop];
					pens[loop]=pens[loop1];
					pens[loop1]=tmp;
				}
			}
		}

		/* Now, pick the pens... HightLight... */
		loop=numcolours-1;
		while (!(ri->Highlight=pens[loop--]));

		/* and Shadow... */
		loop=0;
		while (!(ri->Shadow=pens[loop++]));

		/* The BackGround pen... */
		if (!pens[loop]) loop++;
		ri->BackPen=pens[backpen=loop];

		loop1=0;
		for (loop=0;loop<numcolours;loop++)
		{
			tmp=ColourDifference(colours[loop],colours[backpen]);
			if (tmp>loop1)
			{
				loop1=tmp;
				ri->TextPen=pens[loop];
			}
		}
	}
}

VOID CleanUp_RenderInfo(struct RenderInfo *ri)
{
	if (ri) FreeMem(ri,sizeof(struct RenderInfo));
}

/*
 * Use this screen for the render information.  If the screen is NULL
 * it will use the WorkBench screen...
 */
struct RenderInfo *Get_RenderInfo(struct Screen *TheScreen)
{
register	struct	RenderInfo	*ri;

	if (ri=AllocMem(sizeof(struct RenderInfo),MEMF_PUBLIC|MEMF_CLEAR))
	{
		NewFillIn_RenderInfo(ri,TheScreen);
	}
	return (ri);
}
