/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: areaend.c,v 42.0 93/06/16 11:15:47 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include "areafill.h"
#include "/macros.h"
#include "c.protos"

void draw_fill_outline(w,minx,miny)
struct RastPort *w;
short minx,miny;
{
    short *save_patptr;
    register short *p;
    register char *flgsptr;
	struct AreaInfo *ai=w->AreaInfo;
	UBYTE	save_minterms[8];
	USHORT save_lptrn;
	UBYTE save_FgPen;
	UBYTE save_DrawMode;
	short	x0,y0;
	int i;

	save_lptrn = w->LinePtrn;
	save_FgPen = w->FgPen;
	save_DrawMode = w->DrawMode;
	for (i=0;i<8;i++) save_minterms[i] = w->minterms[i];

	w->DrawMode = 0;
	w->FgPen = w->AOlPen;
	genminterms(w);
	save_patptr = w->AreaPtrn;
	w->AreaPtrn = 0;
	w->LinePtrn = 0xffff;
	p = ai->VctrTbl;        flgsptr = ai->FlagTbl;
	for(i=ai->Count;i>0;NULL)
	{
	    /* bart - 04.29.86 */
	    /* notice the fancy footwork on the ++ and -- below */

	    if(*flgsptr & EXTENDED)
	    {
		BYTE saveflag;
		SHORT x1,y1;

		while( ((saveflag = *flgsptr++) & EXTENDED) &&
			((x0 = *p++) || TRUE) &&
			((y0 = *p++) || TRUE) &&
			(--i>0) )  
		{ 
		    BYTE ext_flag;

		    /* update pointers */

		    x1 = *p++;
		    y1 = *p++;
		    ext_flag = *flgsptr++;
		    --i; 

		    /* need to outline this ? */

		    if (saveflag & DRWBNDRY)
		    {
			SHORT cx,cy,a,b;
			    
			/* convert (x0,y0),(x1,y1) to proper arguments */

			cx = (x0+x1)>>1;
			cy = (y0+y1)>>1;
			a  = x1 - cx;
			b  = y1 - cy;

			switch(ext_flag)
			{
			    case(CIRCLE):
			    {
				draw_ellipse(w,minx+cx,miny+cy,a,b);
				break;
			    }
			    case(ELLIPSE):
			    {
				draw_ellipse(w,minx+cx,miny+cy,a,b);
				break;
			    }
			    default: ;
			}
		    }
		}
		if( !(saveflag & EXTENDED) ) --flgsptr;
	    }
	    else
	    {
		x0 = *p++; y0 = *p++;
		
		if (*flgsptr++ & DRWBNDRY)
		{
		    --i;
		    DRAW(w,minx+x0,miny+y0);
		}
		else
		{
		    --i;
		    MOVE(w,minx+x0,miny+y0);
		}
	    }
	}
	w->DrawMode = save_DrawMode;
	w->FgPen = save_FgPen;
	for (i=0;i<8;i++) w->minterms[i] = save_minterms[i];
	w->AreaPtrn = save_patptr;
	w->LinePtrn = save_lptrn;
}

