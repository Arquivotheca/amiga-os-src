/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 * $Id: renderinfo.c,v 1.1 91/04/09 16:47:58 dlarson Exp $
 *
 * $Source: HOGNET:src/net/server/config/renderinfo.c,v $
 *
 * $Date: 91/04/09 16:47:58 $
 *
 * $Revision: 1.1 $
 *
 * $Log:	renderinfo.c,v $
 * Revision 1.1  91/04/09  16:47:58  dlarson
 * Initial revision
 * 
 * Revision 1.3  90/06/03  10:25:59  mks
 * Fixed the fact the bottom value was set wrong!
 * 
 * Revision 1.2  90/05/20  12:17:03  mks
 * New functions to allocate and free RenderInfo structure.
 * These should be used insted of the old FillIn_RenderInfo...
 * Now has a TextAttr in the RenderInfo
 * Now has the screen Width/Height in RenderInfo
 * Now can be passed a screen pointer
 *
 * Revision 1.1  90/05/09  21:45:02  mks
 * Source now fully under RCS...
 *
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
#include	"DefaultFonts.h"

/*
 * These define the amount of Red, Green, and Blue scaling used
 * to help take into account the different visual impact of those
 * colours on the screen.
 */
#define	BLUE_SCALE	2
#define	GREEN_SCALE	6
#define	RED_SCALE	3

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
SHORT ColourLevel(UWORD rgb)
{
	return(ColourDifference(rgb,0));
}

#define	MAX_COLOURS	16

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
		SHORT		colours[16];
		SHORT		colourlevels[16];
		SHORT		pens[16];
	struct	Screen		screen;

	if (!TheScreen) GetScreenData((CPTR)(TheScreen=&screen),sizeof(struct Screen),WBENCHSCREEN,NULL);

	ri->WindowTop=TheScreen->WBorTop;
	ri->WindowLeft=TheScreen->WBorLeft;
	ri->WindowRight=TheScreen->WBorRight;
	ri->WindowBottom=TheScreen->WBorBottom;
	ri->WindowTitle=TheScreen->BarHeight-TheScreen->BarVBorder+TheScreen->WBorTop;

	ri->ScreenWidth=TheScreen->Width;
	ri->ScreenHeight=TheScreen->Height;

	ri->FontSize=TheScreen->Font->ta_YSize;

	ri->TheFont=OpenFont(&TOPAZ80);

	if (ri->TheFont)
	{
		ri->TextAttr.ta_Name=ri->TheFont->tf_Message.mn_Node.ln_Name;
		ri->TextAttr.ta_YSize=ri->TheFont->tf_YSize;
		ri->TextAttr.ta_Style=ri->TheFont->tf_Style;
		ri->TextAttr.ta_Flags=ri->TheFont->tf_Flags;
	}
	else ri->TextAttr=TOPAZ80;

	numcolours=1 << (TheScreen->RastPort.BitMap->Depth);
	if (numcolours>16) numcolours=16;

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

/*
 * Close the font and free the memory...
 */
VOID CleanUp_RenderInfo(struct RenderInfo *ri)
{
	if (ri)
	{
		if (ri->TheFont) CloseFont(ri->TheFont);
		FreeMem(ri,sizeof(struct RenderInfo));
	}
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
