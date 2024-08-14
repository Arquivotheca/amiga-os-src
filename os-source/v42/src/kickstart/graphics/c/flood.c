/******************************************************************************
*
*	$Id: flood.c,v 42.1 93/07/20 13:38:31 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include "/macros.h"
#include "/gfxbase.h"
#include "/clip.h"
#include "/rastport.h"
#include <hardware/blit.h>
#include "/sane_names.h"
#include "/gfxpragmas.h"
#include "c.protos"

/*#define DEBUG*/
/* #define MASKDEBUG */

#ifdef DEBUG
#define KPRINTF
#endif

#ifdef KPRINTF
#define printf kprintf
#endif

newfill(w,cw,mode,x,y)
register struct RastPort *w;
int x,y;
int mode;
struct Layer *cw;
{
    struct RastPort floodrast;
    struct BitMap bitmap;
    struct BitMap savebitmap;
    int savedepth;
    /*char **p;*/
    int fillbytes;
    int pen;
    SHORT planecount;
    int error = FALSE;

#ifdef DEBUG
        printf("newfill: starting newfill...\n");
#endif

    if (mode == 0)  pen = w->AOlPen;
    else
    {
	if ( (pen = READPIXEL(w,x,y) ) == -1) return(-1);
    }

    /* newfill relies on w->TmpRas being at least as large a this layer's maximum rassize */

    fillbytes = (w->TmpRas->Size);	/* default if no layer */

#ifdef DEBUG
    printf("tmpras size == %lx\n",fillbytes);
#endif

    /* starting at (x,y) change all adjoining pixels from color p to penclra */
    shortinitrast(&floodrast);
    floodrast.BitMap = &bitmap;
    floodrast.Layer = 0;
    bitmap = *w->BitMap;        /* copy all info */

    if(cw)
    {
	int layerbytes;

	/* (floodrast.BitMap)->Width is byte width */
	/* of left-word-boundary adjusted layer */
	/* rounded up to high word --- */
	/* note: the +16 in the line below is intentional ! */

	bitmap.BytesPerRow = BLTBYTES(cw->bounds.MinX,cw->bounds.MaxX);

	/* (floodrast.BitMap)->Height is row height of the layer */

	bitmap.Rows = ((cw->bounds.MaxY - cw->bounds.MinY)+1);
	layerbytes = bitmap.BytesPerRow*bitmap.Rows;

#ifdef DEBUG
    printf("layerbytes == %lx\n",layerbytes);
#endif
	
	if(layerbytes > fillbytes)
	{
	    return(FALSE); /* tmpras too small */
	}
	else 
	{
	    fillbytes = layerbytes;
	}
    }

    bitmap.Depth = 1;
    bitmap.Planes[0] = w->TmpRas->RasPtr;
    floodrast.AOlPen = 1;
    /* try to use second buffer for real fill */
#ifdef DEBUG
    printf("allocate fillbytes == %lx\n",fillbytes);
#endif
    bitmap.Planes[1] = (char *)AllocMem(fillbytes,MEMF_PUBLIC|MEMF_CHIP);
    if (bitmap.Planes[1])
    {
#ifdef DEBUG
    printf("new faster filler\n");
    printf("flood fill buffers at:%lx %lx\n",bitmap.Planes[0],bitmap.Planes[1]);
    /*printf("blither");*/
#endif
	bitmap.Depth = 2;
    }
    else
    {
#ifdef DEBUG
	printf("newfill: couldn't allocate bitmap.planes[1]...\n");
#endif
	return(FALSE);
    }

#ifdef DEBUG
    printf("newfill: about to call setrast\n");
#endif
    SETRAST(&floodrast,0);
    waitblitdone();

/* bart - start of new blitmask generation code to correct problem in old genblit code */

#ifdef DEBUG
        printf("newfill: beginning to generate blitmask using newGenBlit...\n");
#endif

    if (bitmap.Planes[1])
    {
	/* make sure we don't get the rug pulled out from under us */

	savebitmap = *(w->BitMap); /* copy all fields */
	savedepth = bitmap.Depth;

	/* genblit only one plane at at time */
	w->BitMap->Depth = 1;
	bitmap.Depth = 1;

	for (planecount=0; planecount<savebitmap.Depth; planecount++)
	{
	    /* move planecount planepointer to zeroth plane pointer, for this blit */
	    w->BitMap->Planes[0] = savebitmap.Planes[planecount];

	    /* now, generate blitmask in floodrast.BitMap->Planes[0] */
	    /* clipping to SOURCE cliprects */

	    if((pen>>planecount)&1)
	    {

#ifdef DEBUG
        printf("newfill: generating blitmask for plane[%lx]=%lx ...\n",planecount,w->BitMap->Planes[0]);
        printf("newfill: (pen>>planecount)&1=%lx...\n",(pen>>planecount)&1);
#endif
		newGenBlit(w,0,0,
				&floodrast,0,0,
			        (bitmap.BytesPerRow<<3),
			        (bitmap.Rows),
			        ((planecount == 0)?(ABNC):(ABC)),
			        planecount);
	    }
	    else
	    {

#ifdef DEBUG
        printf("newfill: generating blitmask for plane[%lx]=%lx ...\n",planecount,w->BitMap->Planes[0]);
        printf("newfill: (pen>>planecount)&1=%lx...\n",(pen>>planecount)&1);
#endif
		newGenBlit(w,0,0,
				&floodrast,0,0,
			        (bitmap.BytesPerRow<<3),
			        (bitmap.Rows),
			        ((planecount == 0)?(ANBNC):(ANBC)),
			        planecount);
	    }
	}

#ifdef DEBUG
        printf("newfill: generated blitmask for all planes...\n");
#endif

	/* remember to clear floodrast.BitMap->Planes[1] to zero !!! */
	/* because of it's use as a temporary raster by newGenBlit!!! */

#ifdef DEBUG
        printf("newfill: bltclear bitmap.Planes[1] = %lx; fillbytes = %lx...\n",
		bitmap.Planes[1],fillbytes);
#endif

	BLTCLEAR(bitmap.Planes[1],fillbytes,1);

#ifdef DEBUG
        printf("newfill: returned from bltclear...\n");
#endif

#ifdef DEBUG
        printf("newfill: restore *(w->BitMap)...\n");
#endif
	*(w->BitMap) = savebitmap;  /* restore all fields */

	/* ok, critical stuff done */

	if (mode == 1)
	{

#ifdef DEBUG
	    printf("newfill: mode == 1... invert blitmask\n");
#endif

	    gfx_BltBitMap(&bitmap,0,0,&bitmap,0,0,
		      (bitmap.BytesPerRow<<3),
		      (bitmap.Rows),
		      (NANBNC|ANBNC|NANBC|ANBC),
		      1,bitmap.Planes[1]);
	}

#ifdef MASKDEBUG

	/* for mask debugging ... copy mask into all planes of rastport w then return */

	{
	    savebitmap = *(floodrast.BitMap); /* copy all fields */

	    /* duplicate bitmap data from w->Bitmap */
	    (floodrast.BitMap)->Depth  = (w->BitMap)->Depth;

	    /* duplicate bitMap.Planes[0] (mask data pointer) for all planes of floodrast */
	    for (planecount=0; planecount<floodrast.BitMap->Depth; planecount++)
	    {
		floodrast.BitMap->Planes[planecount] = savebitmap.Planes[0];
	    }

#ifdef DEBUG
        printf("newfill: about to clipblit blitmask for maskdebugging purposes...\n");
#endif
	    /* now copy floodrast to w rastport using clipblit */
	    clipbltrastport(floodrast.BitMap, 0,0, w,
			0, 0,
		        (((floodrast.BitMap->BytesPerRow)<<3)),
		        ((floodrast.BitMap->Rows)),
		        (NABNC|NABC|ABNC|ABC),
			savebitmap.Planes[1]);
#ifdef DEBUG
        printf("newfill: returned from clipblit blitmask for maskdebugging purposes...\n");
#endif

	    *(floodrast.BitMap) = savebitmap; /* restore all fields */
	}
#endif

	bitmap.Depth = savedepth;   /* restore depth */ 
    }
    
#ifdef DEBUG
    printf("newfill: generated blitmask using newGenBlit...\n");
#endif

#ifdef MASKDEBUG
    FreeMem(bitmap.Planes[1],fillbytes);
    return(TRUE);
#endif

/* bart - end of new blitmask generation code */

    /*p = w->BitMap->Planes;*/

    /* now use processor to do flood fill */

    waitblitdone();

#ifdef DEBUG
    printf("flood rast at %lx\n",&floodrast);
#endif

    /* try to use second buffer for real fill */

#ifdef DEBUG
    printf("now call fillfast\n");
	/*Debug();*/
#endif

    if (bitmap.Planes[1] == 0) {
        if (!(fillfast_region(&floodrast,w,x,y))) error = TRUE;
    }
    else
    {
	if (!(fillfast_region(&floodrast,&floodrast,x,y))) error = TRUE;
#ifdef DEBUG
	printf("call blit plate\n");
#endif
        if (!error)
	gfx_BltPattern(w,bitmap.Planes[1],0,0,
			(bitmap.BytesPerRow<<3)-1,
			bitmap.Rows-1, 
			bitmap.BytesPerRow
			);
#ifdef DEBUG
#endif
	waitblitdone();
	FreeMem(bitmap.Planes[1],fillbytes);
#ifdef DEBUG
        printf("newfill: returning from newfill...\n");
#endif
    }

    if (error) return(FALSE);
    else return(TRUE);
}

