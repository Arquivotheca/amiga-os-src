/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: blitplate.c,v 39.2 92/05/05 11:47:58 chrisg Exp $
*
******************************************************************************/

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
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <hardware/intbits.h>
#include <hardware/dmabits.h>
#include "/sane_names.h"
#include "gfxblit.h"
#include "/gfxpragmas.h"
#include "/macros.h"
#include <exec/memory.h>
#include "c.protos"

extern struct Custom custom;

#define DOWNCODE

#define	TRAP	( TRUE )

/*#define DEBUG*/
/*#define DEBUG1*/
/*#define DEBUGFILL*/
/*#define DEBUGDEBUG*/
#define BUGFIXED

extern USHORT fwmaskTable[16];
extern USHORT lwmaskTable[16];

/*
unsigned short fwmaskTable[]=
{
    0xffff,0x7fff,0x3fff,0x1fff,
    0xfff,0x7ff,0x3ff,0x1ff,
    0xff,0x7f,0x3f,0x1f,
    0xf,0x7,0x3,0x1
};
*/

char * __regargs makemask(char *ras,short bpr, short x,short width,
			  short height,int *newras)
{
	int tot_bytes;
	char *p;
	struct BitMap sbm,dbm;
//	int to_x;	not referenced CHG

	tot_bytes = smuls(bpr,height);
#ifdef DEBUG
	printf("makemask(%lx,bpr=%ld,%ld,h=%ld ",ras,bpr,height);
#endif
	p = (char *)getmustmem(tot_bytes,MEMF_CHIP);
#ifdef DEBUG
	printf("%lx\n",p);
#endif
	if (p)	/* getmustmem() now returns NULL instead of GURUing - spence Jan 18 1991 */
	{
		BLTCLEAR(p,tot_bytes,0);
		INITBITMAP(&sbm,1,bpr<<3,height);
		INITBITMAP(&dbm,1,bpr<<3,height);
		sbm.Planes[0] = ras;
		dbm.Planes[0] = p;
		gfx_BltBitMap(&sbm,x,0,&dbm,x,0,width,height,NANBC|NABC|ABNC|ABC,1,0);
#ifdef DEBUGDEBUG
	Debug();
#endif
		*newras = tot_bytes;
	}
	return(p);
}

void oldbltpattern(w,ras,cr,rect,fillbytes,fudge,rx,offsetx,offsety)
register struct RastPort *w;
char *ras;
int fillbytes;
struct Rectangle rect;
struct ClipRect *cr;
short fudge,rx;
short offsetx,offsety;
{
    short shift;
	short height;
	short width;
    int rasdx;
    char **pptr;
    short fillbytes_needed;
#ifndef DOWNCODE
    struct AmigaRegs *io;
#endif
    register struct gfxbltnd *l;
	int newras;

    struct gfxbltnd lptmp;

    l = &lptmp;
#ifndef DOWNCODE
    io = &custom;
#endif
	height = rect.MaxY - rect.MinY + 1;
	width = rect.MaxX - rect.MinX + 1;
	fillbytes_needed = BLTBYTES(rect.MinX,rect.MaxX);
	/* Dale's new blither */
	fillbytes_needed = MAX(fillbytes_needed,BLTBYTES(offsetx,offsetx+width-1));
	newras = 0;
	l->bltcon1 = 0;
#ifdef DEBUG
	if TRAP printf("height=%ld,width=%ld\n",height,width);
#endif

    pptr = w->BitMap->Planes;
    rasdx = smuls(rect.MinY , w->BitMap->BytesPerRow) + (rect.MinX>>3);
#ifdef DEBUG
	if TRAP
	{
    printf("blitplate(%lx,%lx,%lx,%d,%d,%d,%d,%lx,%lx)\n",w,ras,cr,rect,offsetx,offsety);
#ifdef DEBUGDEBUG
    Debug();
#endif
	}
#endif
    if (ras == 0)
    {
		shift = 0;
		l->fwmask = fwmaskTable[0xf & rect.MinX];
		l->lwmask = fwmaskTable[0xf & (rect.MaxX +1)];
		if (l->lwmask != 0xffff) l->lwmask = ~l->lwmask;
		l->bltcon0 = DEST|SRCC;
		fillbytes = fillbytes_needed;
    }
    else
    {	/* real areafill here */
	/* Dale's blither */
		shift = (rect.MinX&15) - (offsetx & 15);
		l->bltcon0 = DEST|SRCA|SRCC|(shift<<ASHIFTSHIFT);
		if (cr == 0)
		{
			/*l->bltpta = (char *)ras;*/
			l->fwmask = 0xffff;
	    	if (fillbytes == fillbytes_needed)
			{
				/* l->lwmask = 0xffff; */
				l->lwmask = lwmaskTable[(width - 1) & 0xf];
			}
	    	else
				l->lwmask = 0;
				/*l->lwmask = lwmaskTable[shift & 15];*/
		}
		else
		{
#ifdef DEBUG
		if TRAP
			printf("offsetx = %lx y=%lx",offsetx,offsety);
    /*printf("clipping(%lx,%lx,%ld,%ld,%ld,%ld)",ras,cr,xl,yl,maxx,maxy);*/
#endif
			/* clipping */
			/* marker */
			if (shift < 0)
			{
				/* set up new mask */
				ras = makemask(ras,fillbytes,offsetx,width,height+offsety,
								&newras);
			}
			if (newras == 0)
			{
				/* HERE */
				if (fillbytes != fillbytes_needed)
				{
#ifdef DEBUG
					printf(" here you dork\n");
#endif
					l->fwmask = fwmaskTable[offsetx & 15];
					l->lwmask = 0;
					if (fillbytes == 2)
					{
						/* use fwmask to trim end as well */
						if ( width >= (fillbytes<<3) ) ;
						else l->fwmask &= lwmaskTable[(offsetx+width-1) & 15];
					}
					else
					{
							/* have to use another temp buffer to do this */
						ras = makemask(ras,fillbytes,offsetx,width,height+offsety,
								&newras);
					}
				}
				else
				{
#ifdef DEBUG
					printf("no asshole, over here\n");
#endif
					l->fwmask = fwmaskTable[offsetx & 15];
					if ( width >= (fillbytes<<3) ) l->lwmask = 0xffff;
					else
					{
						ras = makemask(ras,fillbytes,offsetx,width,height+offsety,
								&newras);
						l->lwmask = 0xffff;
					}
				}
			}
			if (newras)
			{
				if (shift < 0)
				{
					l->lwmask = 0xffff;
					l->fwmask = 0xffff;
					l->bltcon1 = BLITREVERSE;
					l->bltcon0 = (l->bltcon0 & 0xfff) | ((-shift)<<12);
				}
				else
				{
					if (fillbytes >= fillbytes_needed)	l->lwmask = 0xffff;
					else					l->lwmask = 0;
					l->fwmask = 0xffff;
				}
			}
		}
    }
	if (ras)	l->bltpta = (char *)ras + (offsetx>>3) + smuls(offsety,fillbytes);
	if (l->bltcon1 & BLITREVERSE)
	{
		if (w->AreaPtrn == 0)
		{
	   		l->bltmdc = -w->BitMap->BytesPerRow;
	   		l->bltmda = -fillbytes;
		}
		else
		{
	    	l->bltmdc = -(w->BitMap->BytesPerRow<<ABS(w->AreaPtSz));
	    	l->bltmda = -(fillbytes<<ABS(w->AreaPtSz));
		}
		if (ras)	l->bltpta += fillbytes_needed - 2;
		rasdx += fillbytes_needed - 2;
	}
	else
	{
		if (w->AreaPtrn == 0)
		{
	   		l->bltmdc = w->BitMap->BytesPerRow;
	   		l->bltmda = fillbytes;
		}
		else
		{
	    	l->bltmdc = (w->BitMap->BytesPerRow<<ABS(w->AreaPtSz));
	    	l->bltmda = (fillbytes<<ABS(w->AreaPtSz));
		}
	}
	l->bltmdc -= fillbytes_needed;
	l->bltmda -= fillbytes_needed;
#ifdef DEBUG
	if TRAP
	{
		printf("pta = %lx fwm=%lx lwm=%lx\n",l->bltpta,l->fwmask,l->lwmask);
		printf("con0=%lx fbn=%lx shift=%ld\n",l->bltcon0,fillbytes_needed,shift);
		printf("ras=%lx rasdx=%lx\n",ras,rasdx);
		Debug();
	}
#endif

	l->bltsize = (height<<6)|(0x3f&(fillbytes_needed>>1));
	/* additions for new chips V1.4 */
	l->blitrows = height;
	l->blitwords = fillbytes_needed>>1;
    /*l->bltmdc = i;*/	/* to be changed to l->bltmdc */
	if (w->AreaPtrn)
	{
    	l->lminy = rect.MinY;
    	l->lmaxy = rect.MaxY;
    	l->fillbytes = fillbytes;
		l->fudge = fudge;
		l->rx = rx;
	}
#ifdef DOWNCODE
	bp_downcode(rasdx,l,w,pptr);
#else
	/* now only do if AreaPtrn */
    l->bpw = w->BitMap->BytesPerRow;
    l->j = 0;
    l->filln = ABS(w->AreaPtSz);
    l->bltptb = w->AreaPtrn;
    l->jstop = 1<<l->filln;
    OwnBlitter();
    waitblitdone();
    io->adata = 0xffff;
    io->bdata = 0xffff;
    io->fwmask = l->fwmask;
    io->lwmask = l->lwmask;
    io->bltmda = TOBB(l->bltmda);
    io->bltmdc = TOBB(l->bltmdc);
    io->bltmdd = TOBB(l->bltmdc);
    io->bltcon1 = l->bltcon1;
    if (w->AreaPtrn != 0)   /* pattern */
    {
#ifdef DEBUG
			printf("fudge=%lx\n",fudge);
#endif
	l->bltsize &= 0x3f;
	if (w->AreaPtSz >= 0 )
	{
#ifdef BUGFIXED
	    for(i=0;i<w->BitMap->Depth;i++) /* do all rasters */
	    {
		if (1 & (w->Mask>>i))   /* only if write mask allows */
		{
		    l->bltcon = w->minterms[i]|l->bltcon0;
		    l->bltptr = (int)(*pptr + rasdx);
		    do
		    {
			waitblitdone();
		    } while (patblit(l,&custom));
		}
		pptr++;
	    }
#endif
	}
	else
	    for(i=0;i<w->BitMap->Depth;i++) /* do all rasters */
	    {
		if (1 & (w->Mask>>i))   /* only if write mask allows */
		{
		    l->bltcon = w->minterms[i]|l->bltcon0;
		    l->bltptr = (int)(*pptr + rasdx);
		    do
		    {
			waitblitdone();
		    } while (clrblit(l,&custom));
		}
		pptr++;
	    }

    }
    else
	for(i=0;i<w->BitMap->Depth;i++) /* do all rasters */
	{
	    if (1 & (w->Mask>>i))   /* only if write mask allows */
	    {
		waitblitdone();
		io->bltpta = (UBYTE *)TOBB(l->bltpta);
		io->bltcon0 = w->minterms[i]|l->bltcon0;
		io->bltptc = (UBYTE *)(*pptr + rasdx);
		io->bltptd = (UBYTE *)(*pptr + rasdx);
		/* io->bltsize = l->bltsize; */
		start_bltnd(l,&custom);
	    }
	    pptr++;
	}
    DisownBlitter();
#endif
	if (newras)
	{
		waitblitdone();
		freemustmem(ras,newras);
	}
}

#ifndef DOWNCODE
start_blit(height,width)
UWORD height,width;
{
    register struct AmigaRegs *io = &custom;

    if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
    {
	if( (height < (1<<15)) && (width < ( 1<<11)) )
	{
	    io->bltsizv= (height & 0x7fff);
	    io->bltsizh= (width & 0x07ff);
	    return(TRUE);
	}
    }
    else
    {
	if( (height < (1<<10)) && (width < ( 1<<6)) )
	{
	    io->bltsize= (height << 6) + width;
	    return(TRUE);
	}
    }
    return(FALSE);
}

start_bltnd(bn)
register struct gfxbltnd *bn;
{
    register struct AmigaRegs *io = &custom;

    if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
    {
	start_blit(bn->blitrows,bn->blitwords);
    }
    else
    {
	io->bltsize= bn->bltsize;
    }
}
#endif

patblit(fl)
register struct gfxbltnd *fl;
{
	register struct AmigaRegs *io = (struct AmigaRegs *) (&custom);
    register int t;
	UWORD	mypat;
    mypat = *(fl->bltptb + ((fl->fudge+fl->lminy+fl->j)&(0xff>>(8-fl->filln))));
	/* must prerotate pattern by rx amount to the right */
	/*mypat = rotate(mypat,rx);*/
    io->bdata = rotate(mypat,fl->rx);
    t = umuls(fl->j,fl->fillbytes);
    io->bltpta = (UBYTE *)TOBB((int)fl->bltpta + t);
    io->bltcon0 = fl->bltcon0;
    t = umuls(fl->j,fl->bpw);
    io->bltptc = (UBYTE *)TOBB(fl->bltptr+t);
    io->bltptd = (UBYTE *)TOBB(fl->bltptr+t);
	if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
	/*if (0)*/
	{
	    io->bltsizv=((((fl->lmaxy-fl->lminy-fl->j)>>fl->filln)+1) & 0x7fff);
	    io->bltsizh=(fl->blitwords & 0x07ff);
	}
	else
    		io->bltsize = fl->bltsize +
			 ((((fl->lmaxy-fl->lminy-fl->j)>>fl->filln)+1)<<6);

    fl->j++;
    if  ((fl->j == fl->jstop)||(fl->lminy + fl->j > fl->lmaxy))
    {
	fl->j = 0;
	return(FALSE);  /* goto next bitplane */
    }
    return(TRUE);
}

clrblit(fl)
register struct gfxbltnd *fl;
{
	register struct AmigaRegs *io = (struct AmigaRegs *) (&custom);
    register int t;
	UWORD	mypat;
    mypat = *(fl->bltptb + ((fl->fudge+fl->lminy+fl->j)&(0xff>>(8-fl->filln))));
    io->bdata = rotate(mypat,fl->rx);
    t = umuls(fl->j,fl->fillbytes);
    io->bltpta = (UBYTE *)TOBB((int)fl->bltpta + t);
    io->bltcon0 = fl->bltcon0;
    t = umuls(fl->j,fl->bpw);
    io->bltptc = (UBYTE *)TOBB(fl->bltptr+t);
    io->bltptd = (UBYTE *)TOBB(fl->bltptr+t);
	if (GBASE->ChipRevBits0 & GFXF_BIG_BLITS)
	{
	    io->bltsizv=((((fl->lmaxy-fl->lminy-fl->j)>>fl->filln)+1) & 0x7fff);
	    io->bltsizh=(fl->blitwords & 0x07ff);
	}
	else
    		io->bltsize=fl->bltsize+
			((((fl->lmaxy-fl->lminy-fl->j)>>fl->filln)+1)<<6);
    fl->j++;
    if ( (fl->j == fl->jstop) || (fl->lminy + fl->j > fl->lmaxy))
    {
	fl->j = 0;
	fl->bltptb += 1<<(fl->filln);
	return(FALSE);  /* goto next bitplane */
    }
    return(TRUE);
}

