/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: vlimit.c,v 42.0 93/06/16 11:16:26 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	vlimit.c,v $
 * Revision 42.0  93/06/16  11:16:26  chrisg
 * initial
 * 
 * Revision 42.0  1993/06/01  07:17:51  chrisg
 * *** empty log message ***
 *
*   Revision 39.2  93/05/06  12:12:17  spence
*   Replaced all references to mspc->ms_Node.xln_Library with GBASE.
*   
*   Revision 39.1  91/10/04  16:09:04  chrisg
*     used __regargs in 2 functions
*   
*   Revision 39.0  91/08/21  17:23:02  chrisg
*   Bumped
*   
*   Revision 37.3  91/05/20  11:22:13  chrisg
*   Added prototypes, improved warnings, and glue code. still needs work.
*   
 * Revision 1.1  91/05/20  10:48:40  chrisg
 * Initial revision
 * 
*   Revision 37.2  91/05/02  15:26:49  chrisg
*   had to change </macros.h> to "/macros.h"
*   
*   Revision 37.1  91/05/02  13:00:42  chrisg
*    changed "../" to "/" for native build
*   
*   Revision 37.0  91/01/07  15:21:08  spence
*   initial switchover from V36
*   
*   Revision 36.13  90/07/27  17:00:30  bart
*   id
*   
*   Revision 36.12  90/03/28  09:13:28  bart
*   *** empty log message ***
*   
*   Revision 36.11  89/03/01  13:28:26  bart
*   *** empty log message ***
*   
*   Revision 36.10  88/09/13  17:01:12  bart
*   videoscan() processing and denise backwards compatibility
*   
*   Revision 36.9  88/09/13  11:22:02  bart
*   back to maxoscan for jimm
*   
*   Revision 36.8  88/09/13  10:26:03  bart
*   normal_oscan
*   
*   Revision 36.7  88/09/12  12:19:18  bart
*   *** empty log message ***
*   
*   Revision 36.6  88/09/11  18:13:56  bart
*   undefine debug
*   
*   Revision 36.5  88/09/11  17:34:48  bart
*   better hmin, standard dimensions
*   
*   Revision 36.4  88/08/12  13:17:58  bart
*   mspc->DeniseMinDisplayColumn
*   
*   Revision 36.3  88/08/07  16:51:02  bart
*   checkpoint - assembly interface to monitorspec routines
*   
*   Revision 36.2  88/09/01  09:36:12  bart
*   now calls maxoscan subroutine
*   
*   Revision 36.1  88/08/29  10:00:30  bart
*   documentation change
*   
*   Revision 36.0  88/08/26  17:08:04  bart
*   added to rcs for updating
*   
*
******************************************************************************/

/* vlimit.c -- interrogate monitorspec and determine legalview limits */

#include <exec/types.h>
#include <exec/lists.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxnodes.h>
#include <graphics/monitor.h>
#include <graphics/view.h>
#include "/macros.h"
#include "c.protos"

/* #define DEBUG */

#define COLORCLOCKS (mspc->total_colorclocks)

#define GRANULARITY 8
#define FETCH_MASK  (~(GRANULARITY-1))
#define DATAFETCH_MIN 0x18

/* voodoo -- for now */
#define DENISE_OFFSET 9

#define MODE_OFFSET \
( (mode & SUPERHIRES)? -2: ((mode & HIRES)? 0: 4) )

#define DENISE_TO_AGNUS(x) \
{ (x) -= DENISE_OFFSET; (x) >>= 1; (x) -= MODE_OFFSET; }

#define AGNUS_TO_DENISE(x) \
{ (x) += MODE_OFFSET; (x) <<= 1; (x) += DENISE_OFFSET; }

#define NORMALIZE_PIXELS(x) \
{ if ((mode & HIRES)||(mode & SUPERHIRES)) (x) >>= 1; \
  if (mode & SUPERHIRES) (x) >>= 1; }

#define SCALE_PIXELS(x) \
{ if ((mode & HIRES)||(mode & SUPERHIRES)) (x) <<= 1; \
  if (mode & SUPERHIRES) (x) <<= 1; }
    
/* utilities */

UWORD __regargs agnus_max( UWORD amin, UWORD clocks)
{
	UWORD amax;

	amax =  clocks - GRANULARITY;
	amax -= (((amax-(amin+2))+1)%GRANULARITY);
	amax &= ~1;
	amax += GRANULARITY;

	return(amax);
}

/* if no error, returns densise lores pixels in minX, maxX */
/* returns, rows in minY, maxY */ 

__regargs oscan( struct MonitorSpec *mspc, struct Rectangle *rect,
			UWORD mode, UWORD extra)
{ 
    LONG error = FALSE; 

    if(mspc && rect)
    {
	UWORD xmin; /* graphics minimum pixel */
	UWORD xmax; /* graphics maximum pixel */
	UWORD hmin; /* hardware minimum fetch */
	UWORD hmax; /* hardware maximum fetch */
	UWORD mask = FETCH_MASK; /* alignment */

	if (!(GBASE->ChipRevBits0 & GFXF_HR_DENISE))
	{
	    NORMALIZE_PIXELS(mask);
	}

	hmin = xmin = mspc->DeniseMinDisplayColumn;
	xmax = mspc->DeniseMaxDisplayColumn + extra;

	DENISE_TO_AGNUS(hmin);

	hmin &= mask;
	hmin =  MAX(hmin,DATAFETCH_MIN); 

	hmax =  agnus_max( hmin, COLORCLOCKS );

	AGNUS_TO_DENISE(hmin);
	AGNUS_TO_DENISE(hmax);

	rect->MinX = MAX(hmin,xmin);
	rect->MinY = mspc->min_row;
	rect->MaxX = MIN(hmax,xmax);
	rect->MaxY = mspc->total_rows;

#ifdef DEBUG
	kprintf("oscan: mspc == %lx,\n", mspc);
	kprintf("extra: %5ld,\n", extra);
	kprintf("rect minX: %5ld\n",rect->MinX);
	kprintf("rect minY: %5ld\n",rect->MinY);
	kprintf("rect maxX: %5ld\n",rect->MaxX);
	kprintf("rect maxY: %5ld\n",rect->MaxY);
#endif

    }
    else
    {
	error = TRUE;
    }

    return(error);
}

/* only the specific rectangle within the video rectangle is "legal" */
/* converted to a view relative displayclip it will display as many pixels */
/* as the hardware will allow, including any software tricks to do the job */

videoscan(struct MonitorSpec *mspc, struct Rectangle *rect, UWORD mode)
{ 
    LONG error = oscan( mspc, rect, mode, DENISE_OFFSET );

    return(error);
}

/* any noninverted display rectangle contained within maxoscan is "legal" */

maxoscan(struct MonitorSpec *mspc,struct Rectangle *rect,UWORD mode)
{ 
    LONG error = oscan( mspc, rect, mode, 0 );

    return(error);
}

/* accounts for maxoscan minus standard screen dimensions */

__regargs vlimit(struct MonitorSpec *mspc,struct Rectangle *rect,UWORD mode)
{ 
    LONG error = FALSE; 

    if(mspc && mspc->ms_maxoscan && rect )
    {
	Point size;

        size.x = GBASE->NormalDisplayColumns >> 1;
	size.y = GBASE->NormalDisplayRows;

	/* call indirect */
	error = mspc_oscan(GBASE,mspc,rect,NULL); 

	rect->MaxX = (rect->MaxX-rect->MinX)+1;
	rect->MaxY = (rect->MaxY-rect->MinY)+1;

	/* call indirect */
	mspc_scale
	(GBASE,mspc,&size.x,TO_MONITOR,&size.x);

	rect->MaxX -= size.x; /* subtract scaled standard screen dimension */
	rect->MaxY -= size.y; /* subtract scaled standard screen dimension */

	rect->MaxX = (rect->MaxX+rect->MinX)-1;
	rect->MaxY = (rect->MaxY+rect->MinY)-1;

#ifdef DEBUG
	kprintf("vlimit: mspc == %lx,\n", mspc);
	kprintf("rect minX: %5ld\n",rect->MinX);
	kprintf("rect minY: %5ld\n",rect->MinY);
	kprintf("rect maxX: %5ld\n",rect->MaxX);
	kprintf("rect maxY: %5ld\n",rect->MaxY);
#endif

    }
    else
    {
	error = TRUE;
    }

    return(error);
}

