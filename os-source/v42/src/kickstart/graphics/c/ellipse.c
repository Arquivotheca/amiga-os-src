/******************************************************************************
*
*	$Id: ellipse.c,v 37.2 91/05/20 11:10:09 chrisg Exp $
*
******************************************************************************/

/* fast ellipse drawing routine */
/* after the algortihm found in IEEE CG&A September 1984 */
/* modified by bart 04.24.86 to keep ellipse symmetrical and within rectangle */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/copper.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>
#include <graphics/text.h>
#include "/macros.h"
#include "c.protos"

#define WritePixel	WRITEPIXEL

#define SQUEEZE

/* #define DEBUG */

/******************************************************************************/

#define	DOWN_ELLIPSE
#ifndef	DOWN_ELLIPSE

draw_ellipse(rp,cx,cy,a,b)
struct RastPort *rp;
SHORT cx,cy,a,b;
{
	LONG x,y;
	LONG t1,t2,t3;
	LONG t4,t5,t6;
	LONG t7,t8,t9;
	LONG d1,d2;
	LONG pixel_x_right,pixel_x_left;
	LONG pixel_y_bottom,pixel_y_top;
	LONG round_x,round_y;
	LONG round_cx,round_cy;
	BYTE line_done = FALSE; /* for one-dot mode */

#ifndef SQUEEZE
	BYTE bottom_done = FALSE; /* for new finish routine */
	BYTE top_done = FALSE; /* for new finish routine */
#endif

#ifdef CLIPPIX
	LONG min_x,min_y;
	LONG max_x,max_y;
#endif


	/* a,b are the one-half the elliptical width and height respectively */

	round_cx = cx; 	/* bart - 05.05.86 */
	round_cy = cy; 	/* bart - 05.05.86 */

	x = a; y = 0; 

	t1 = (a*a); t2 = 2*t1 ; t3 = 2*t2;
	t4 = (b*b); t5 = 2*t4 ; t6 = 2*t5;
	t7 = (a*t5); t8 = 2*t7 ; t9 = 0;
	d1 = t2 - t7 + t4 ; d2 = t1 - t8 + t5;

/* round off */

#define DOPIX(rp,x,y) WritePixel(rp,x,y);

/* 
#define CLIPPIX(rp,x,y) if(rp->Layer) { WritePixel(rp,x,y); } else { if((x >= min_x) && (x <= max_x) && (y >= min_y) && (y <= max_y)) { WritePixel(rp,x,y); } } */

#ifdef CLIPPIX

/* clip to non-layered rastport boundaries */

	min_x = 0;
	min_y = 0;

	max_x = (rp->BitMap->BytesPerRow << 3) - 1;
	max_y = (rp->BitMap->Rows) - 1;

#endif

	while (d2 < 0)
	{

		round_x = (x); 
		round_y = (y);

		pixel_x_right = round_cx+round_x;
		pixel_y_bottom = round_cy+round_y;

		pixel_x_left= round_cx-round_x;
		pixel_y_top= round_cy-round_y;

#ifndef SQUEEZE
		if (pixel_y_bottom >= pixel_y_top)
#endif
		{
		    if(rp->Flags & ONE_DOT)
		    {
			if((pixel_x_left) != pixel_x_right)
			{
			    DOPIX(rp,pixel_x_right,pixel_y_bottom);
			    DOPIX(rp,pixel_x_left,pixel_y_bottom);

			    if (pixel_y_bottom > pixel_y_top)
			    {
				DOPIX(rp,pixel_x_right,pixel_y_top);
				DOPIX(rp,pixel_x_left,pixel_y_top);
			    }
			}
		    }
		    else
		    {
#ifndef SQUEEZE
			if(pixel_x_left <= pixel_x_right)
#endif
			{
			    DOPIX(rp,pixel_x_right,pixel_y_bottom);

			    if(pixel_x_left < pixel_x_right)
			    {
				DOPIX(rp,pixel_x_left,pixel_y_bottom);
			    }

			    if (pixel_y_bottom > pixel_y_top)
			    {
				DOPIX(rp,pixel_x_right,pixel_y_top);

				if(pixel_x_left < pixel_x_right)
				{
				    DOPIX(rp,pixel_x_left,pixel_y_top);
				}
			    }
			}
		    }
		}

#ifdef DEBUG
		Debug();
#endif

		y += 1;
		t9 += t3;
		if (d1 < 0)
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else
		{
			x -= 1;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	if(x >= 0)
	do
	{
		BYTE line_flag = FALSE;

		round_x = (x);
		round_y = (y); 

		pixel_x_right = round_cx+round_x;
		pixel_y_bottom= round_cy+round_y;

		pixel_x_left = round_cx-round_x;
		pixel_y_top= round_cy-round_y;

#ifndef SQUEEZE
		if (pixel_y_bottom >= pixel_y_top)
#endif
		{
		    if(rp->Flags & ONE_DOT)
		    {
			if((!(line_done)) && ((pixel_x_left) != pixel_x_right))
			{
			    DOPIX(rp,pixel_x_right,pixel_y_bottom);
			    DOPIX(rp,pixel_x_left,pixel_y_bottom);
			    line_flag = TRUE; /* for one-dot mode */

			    if (pixel_y_bottom > pixel_y_top)
			    {
				DOPIX(rp,pixel_x_right,pixel_y_top);
				DOPIX(rp,pixel_x_left,pixel_y_top);
			    }
			}
		    }
		    else
		    {
#ifndef SQUEEZE
			if(pixel_x_left <= pixel_x_right)
#endif
			{
			    DOPIX(rp,pixel_x_right,pixel_y_bottom);
			    line_flag = TRUE;

			    if(pixel_x_left < pixel_x_right)
			    {
				DOPIX(rp,pixel_x_left,pixel_y_bottom);
			    }

			    if (pixel_y_bottom > pixel_y_top)
			    {
				DOPIX(rp,pixel_x_right,pixel_y_top);

				if(pixel_x_left < pixel_x_right)
				{
				    DOPIX(rp,pixel_x_left,pixel_y_top);
				}
			    }
			}

		    }
		}

#ifdef DEBUG
		Debug();
#endif
		/* did we write at least one pixel on this line ? */
		if (line_flag) line_done = TRUE;

		x -= 1;
		t8 -= t6;
		if (d2 < 0)
		{
			y += 1;
			line_done = FALSE; /* for one-dot mode */
			t9 += t3;
			d2 += t5 + t9 - t8;
		}
		else	d2 += t5 - t8;

	} while (x >= 0);
	
	/* new finish routine */

	if(!(rp->Flags & ONE_DOT))
	{
#ifndef SQUEEZE
	    SHORT last_y_bottom;
	    SHORT last_y_top;

#ifdef BARTDEBUG
	printf("entering finish routine...\n");
#endif
	    /* save last drawn top, bottom line coordinate */

	    if(line_done)
	    {
		round_y = y; 
	    }
	    else
	    {
		round_y = y-1; 
	    }

	    pixel_y_bottom = round_cy+round_y;
	    pixel_y_top = round_cy-round_y;

	    bottom_done = top_done = TRUE;
#else

#ifdef BARTDEBUG
	printf("entering finish routine...\n");
#endif

#endif

	    round_y = y;

#ifdef SQUEEZE
	    if(line_done) round_y += 1;
#endif

	    pixel_x_right = round_cx;

	    while (round_y <= b)
	    {
#ifndef SQUEEZE
		BYTE  bottom_flag = FALSE;
		BYTE  top_flag = FALSE;

		/* save last coordinate done, top and bottom */

		if(bottom_done) last_y_bottom        =  pixel_y_bottom;
		if(top_done)    last_y_top 		 =  pixel_y_top;
#endif
		/* new top and bottom */

		pixel_y_bottom= round_cy+round_y;
		pixel_y_top = round_cy-round_y;

#ifndef SQUEEZE
		/* last and new differ ? */

		if(last_y_bottom < pixel_y_bottom) 
		{
		    bottom_done = FALSE;
		}

		if(last_y_top > pixel_y_top)
		{
		    top_done = FALSE;
		}

		/* draw new pixel */

		if (!(bottom_done))
		{
		    if (pixel_y_bottom >= pixel_y_top)
		    {
			DOPIX(rp,pixel_x_right,pixel_y_bottom);
			bottom_flag = TRUE;
		    }
		}

		if(!(top_done))
		{
		    if (pixel_y_bottom > pixel_y_top)
		    {
			DOPIX(rp,pixel_x_right,pixel_y_top);
			top_flag = TRUE;
		    }
		}

		if(bottom_flag) bottom_done = TRUE;
		if(top_flag)    top_done    = TRUE;

#else
		DOPIX(rp,pixel_x_right,pixel_y_bottom);
		if (pixel_y_bottom > pixel_y_top)
		{
		    DOPIX(rp,pixel_x_right,pixel_y_top);
		}

#endif
		round_y += 1;
	    }
	}

}

#endif !DOWN_ELLIPSE

/******************************************************************************/
