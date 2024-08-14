/******************************************************************************
*
*	$Id: newblitplate.c,v 39.0 91/08/21 17:20:50 chrisg Exp $
*
******************************************************************************/

/****i* blitplate.c ***************************************************/
/********************************************************************/
/* areafill code */
#include <exec/types.h>
#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <graphics/gfxbase.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <hardware/intbits.h>
#include <hardware/dmabits.h>
#include </sane_names.h>
#include "gfxblit.h"
#include "/macros.h"
#include "c.protos"

extern struct Custom custom;

/*#define DEBUG*/
/*#define DEBUGDEBUG*/

newbltpattern(w,ras,xl,yl,maxx,maxy,fillbytes)
register struct RastPort *w;
char *ras;
int maxx;
int maxy,fillbytes;
SHORT xl,yl;
{
    PLANEPTR newras;
    struct BitMap srcbm, destbm;
	int height;

	newras = ras;
	height = maxy - yl + 1;
    if (xl & 0xf)
    {
		if (newras)
		{
	    	/* xl is not already word aligned */
	    	/* adjust copy of ras to word boundary */
	   	 
	    	if ( (newras = (PLANEPTR)ALLOCRASTER((fillbytes+2)<<3,height)) == NULL )
	    	{
#ifdef DEBUG
			printf("newbltpattern: can't allocate newras...\n");
#endif
			return(NULL);
	    	}
	    	else
	    	{
			/* intialize dummy bitmap structures */
		
			BLTCLEAR(newras,(height<<16)|(fillbytes+2),2);
			srcbm.BytesPerRow = fillbytes;
			destbm.BytesPerRow = fillbytes+2;
			srcbm.Rows = destbm.Rows = height;
			srcbm.Flags = destbm.Flags = -1;
			srcbm.Depth = destbm.Depth = 1;
			srcbm.Planes[0] = ras;
			destbm.Planes[0] = newras;

			/* copy mask data from ras to newras with shift */
	
			BLTBITMAP(&srcbm,0,0,&destbm,(xl & 0xf),0,fillbytes<<3,height,ABC|ABNC,1,NULL);

#ifdef DEBUG
			printf("after bltbitmap in BltPattern ras= %lx newras=%lx\n",
					ras,newras);
			Debug();
#endif
			fillbytes += 2;
	    	}
		}
    }

	xl &= ~15;
    if (w->Layer)
    {
	    clipbltpattern(w,newras,xl,yl,maxx,maxy,fillbytes);
    }
    else
    {
	    oldbltpattern(w,newras,0,xl,yl,maxx,maxy,fillbytes,0,0,0,0);
    }
	if (newras != ras)
	{
		waitblitdone();
		FREERASTER(newras,(fillbytes)<<3,height);
	}
}
