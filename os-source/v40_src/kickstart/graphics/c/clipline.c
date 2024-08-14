/******************************************************************************
*
*	$Id: clipline.c,v 39.0 91/08/21 17:15:22 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <graphics/clip.h>

/*#define DEBUG*/
#include "cdrawinfo.h"
#include "c.protos"

int __regargs lineclip(struct ClipRect *cr,struct cdraw_info *cdi,
				int *px1,int *py1,int *px2,int *py2)
{
    char code1,code2,code;
    int x1,y1,x2,y2;
    int x,y;
    int x0,y0;

    x1 = *px1;
    x2 = *px2;
    y1 = *py1;
    y2 = *py2;
    code1 = cdi->code1;
    code2 = cdi->code2;
    x0 = x1;
    y0 = y1;

    while ( code1 | code2)
    {
	if (code1 & code2)  return(TRUE);   /* clipped out */
	code = code1;
	if (code == 0)  code = code2;

	if (code & ISLESSX)
	{
#ifdef DEBUG
		if (cdi->dx == 0)
		{
			printf("abort divide 1\n");
			Debug();
		}
#endif
	    y = y0 + IDivS_round(IMulS(cdi->dy,(cr->bounds.MinX-x0)),cdi->dx);

	    x = cr->bounds.MinX;
	}
	else
	if (code & ISGRTRX)
	{
#ifdef DEBUG
		if (cdi->dx == 0)
		{
			printf("abort divide 1\n");
			Debug();
		}
#endif
	    y = y0 + IDivS_round(IMulS(cdi->dy,(cr->bounds.MaxX-x0)),cdi->dx);
	    x = cr->bounds.MaxX;
	}
	else
	if (code & ISLESSY)
	{
#ifdef DEBUG
		if (cdi->dy == 0)
		{
			printf("abort divide 1\n");
			Debug();
		}
#endif
	    x = x0 + IDivS_round(IMulS(cdi->dx,(cr->bounds.MinY-y0)),cdi->dy);
	    y = cr->bounds.MinY;
	}
	else
	if (code & ISGRTRY)
	{
#ifdef DEBUG
		if (cdi->dy == 0)
		{
			printf("abort divide 1\n");
			Debug();
		}
#endif
	    x = x0 + IDivS_round(IMulS(cdi->dx,(cr->bounds.MaxY-y0)),cdi->dy);
	    y = cr->bounds.MaxY;
	}
	if ( code == code1)
	{
	    *px1 = x1 = x;
	    *py1 = y1 = y;
	    code1 = getcode(cr,x,y);
	}
	else
	{
	    *px2 = x2 = x;
	    *py2 = y2 = y;
	    code2 = getcode(cr,x,y);
	}
    }
	return(FALSE); /* remember to indicate not clipped out */
}
	
