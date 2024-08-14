/*
 *		colors.c
 *
 * ========================================================================= 
 * Colors.c - decides color pens for installer utility                       
 * By Talin. (c) 1990 Sylvan Technical Arts                                  
 * ========================================================================= 
 *	
 *	
 * Prototypes for functions defined in colors.c
 *
 *	
 *	SHORT 			ColorDifference			(UWORD , UWORD );
 *	SHORT 			ColorLevel				(UWORD );
 *	void 			assign_colors			(void);
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */



#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <exec/memory.h>

#include "functions.h"

#include "xfunctions.h"

extern struct GfxBase *GfxBase;

/*
 * These define the amount of Red, Green, and Blue scaling used
 * to help take into account the different visual impact of those
 * colors on the screen.
 */
#define	BLUE_SCALE	2
#define	GREEN_SCALE	6
#define	RED_SCALE	3

#ifndef ONLY2_0

/*
 * This returns the color difference hamming value...
 */

SHORT ColorDifference (UWORD rgb0, UWORD rgb1)
{
	register	SHORT	level;
	register	SHORT	tmp;

	tmp		= (rgb0 & 15) - (rgb1 & 15);
	level	= 	tmp*tmp*BLUE_SCALE;
	tmp		= ((rgb0>>4) & 15) - ((rgb1>>4) & 15);
	level  += 	tmp*tmp*GREEN_SCALE;
	tmp		= ((rgb0>>8) & 15) - ((rgb1>>8) & 15);
	level  += 	tmp*tmp*RED_SCALE;
	return (level);
}

/*
 * Calculate a rough brightness hamming value...
 */

SHORT ColorLevel(UWORD rgb)
{
	return(ColorDifference(rgb,0));
}

#endif

#define	MAX_COLORS	4

extern struct Window 		*window;
extern WORD 		 		lightest_color,
					 		darkest_color,
					 		text_color,
					 		highlight_color;
extern struct IntuitionBase *IntuitionBase;

void assign_colors(void)
{
	struct Screen *screen = window->WScreen;

	if (screen->ViewPort.ColorMap->Count < MAX_COLORS)
	{
		lightest_color = darkest_color = text_color = highlight_color = 1;
	}
	else
	{
#ifndef ONLY2_0
		if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
		{
			struct DrawInfo	*di;

			if (di = GetScreenDrawInfo(screen))
			{
				text_color 		= di->dri_Pens[textPen];
				highlight_color =  di->dri_Pens[hifillPen];
				darkest_color 	=  di->dri_Pens[shadowPen];
				lightest_color 	=  di->dri_Pens[shinePen];

				FreeScreenDrawInfo(screen,di);
				return;
			}

			lightest_color = 2;
			darkest_color = 1;
			text_color = 1;
			highlight_color = 3;
		}
#ifndef ONLY2_0
		else
		{
			UWORD			color0;
			UWORD			color1, color2, color3;
			SHORT			level1, level2, level3;
			SHORT			differ1,differ2,differ3;

			color0 = GetRGB4(screen->ViewPort.ColorMap,0L);
			color1 = GetRGB4(screen->ViewPort.ColorMap,1L);
			color2 = GetRGB4(screen->ViewPort.ColorMap,2L);
			color3 = GetRGB4(screen->ViewPort.ColorMap,3L);

			level1 = ColorLevel(color1);
			level2 = ColorLevel(color2);
			level3 = ColorLevel(color3);
			differ1 = ColorDifference(color0,color1);
			differ2 = ColorDifference(color0,color2);
			differ3 = ColorDifference(color0,color3);

			lightest_color = 3;

			if (level1 > level2)
			{
				if (level1 > level3)
				{
					lightest_color = 1;
					if (level2 < level3) 
					{ 
						darkest_color = 2; 
						highlight_color = 3; 
					}
					else 
					{ 
						darkest_color = 3; 
						highlight_color = 2; 
					}
				}
				else
				{
					darkest_color = 2;
					highlight_color = 1;
				}
			}
			else
			{
				if (level2 > level3)
				{
					lightest_color = 2;
					if (level1 < level3) 
					{ 
						darkest_color = 1; 
						highlight_color = 3; 
					}
					else 
					{ 
						darkest_color = 3; 
						highlight_color = 1; 
					}
				}
				else
				{
					darkest_color = 1;
					highlight_color = 2;
				}
			}

			text_color = 3;

			if (differ1 > differ2)
			{
				if (differ1 > differ3) 
					text_color = 1;
			}
			else 
			if (differ2 > differ3) 
				text_color = 2;
		}
#endif
	}
}
