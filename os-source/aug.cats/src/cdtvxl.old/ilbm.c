#define INTUI_V36_NAMES_ONLY

#include "ilbmapp.h"

extern void cleaniff(void);
extern void 	Quit(LONG Code);
extern long getplanes(struct BitMap *bitmap, LONG depth, LONG width, LONG height);
extern void freeplanes(struct BitMap *bitmap, LONG depth, LONG width, LONG height);

/* Note - these fields are also available in the ILBMInfo structure */
struct   Screen         *scr= NULL;         /* for ptr to screen structure */
struct   Window         *win= NULL;         /* for ptr to window structure */
struct   RastPort       *wrp = NULL;         /* for ptr to RastPort  */
struct   ViewPort       *vp = NULL;          /* for ptr to Viewport  */
struct 	 BitMap		*altbitmap = NULL;
struct 	 BitMap		*origbitmap;
struct   NewWindow      mynw = {
   0, 0,                                  /* LeftEdge and TopEdge */
   0, 0,                          	  /* Width and Height */
   -1, -1,                                /* DetailPen and BlockPen */
   IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS, /* IDCMP Flags with Flags below */
   WFLG_BACKDROP | WFLG_BORDERLESS |
   WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH |
   WFLG_ACTIVATE | WFLG_RMBTRAP,
   NULL, NULL,                            /* Gadget and Image pointers */
   NULL,                                  /* Title string */
   NULL,                                  /* Screen ptr null till opened */
   NULL,                                  /* BitMap pointer */
   50, 20,                                /* MinWidth and MinHeight */
   0 , 0,                                 /* MaxWidth and MaxHeight */
   CUSTOMSCREEN                           /* Type of window */
   };


/* ILBM Property chunks to be grabbed
 * List BMHD, CMAP and CAMG first so we can skip them when we write
 * the file back out (they will be written out with separate code)
 */
LONG	ilbmprops[] = {
		ID_ILBM, ID_BMHD,
		ID_ILBM, ID_CMAP,
		ID_ILBM, ID_CAMG,
		ID_ILBM, ID_CCRT,
		ID_ILBM, ID_AUTH,
		ID_ILBM, ID_Copyright,
		TAG_DONE
		};

/* ILBM Collection chunks (more than one in file) to be gathered */
LONG	ilbmcollects[] = {
		ID_ILBM, ID_CRNG,
		TAG_DONE
		};

/* ILBM Chunk to stop on */
LONG	ilbmstops[] = {
		ID_ILBM, ID_BODY,
		TAG_DONE
		};

UBYTE nomem[]  = "Not enough memory\n";
UBYTE noiffh[] = "Can't alloc iff\n";

/* our indexes to reference our frames
 * DEFault, BRUsh, and SCReen
 */
#define DEF	0
#define SCR	1
#define UICOUNT 2

/* For our ILBM frames */
struct ILBMInfo  *ilbms[UICOUNT]  = { NULL };


int swidth, sheight, sdepth;


void loadpic(UBYTE *ilbmname)
{
   LONG error;


/* 
 * Alloc two ILBMInfo structs (one each for defaults, screen) 
 */

    if(!(ilbms[0] = (struct ILBMInfo *)
	AllocMem(UICOUNT * sizeof(struct ILBMInfo),MEMF_PUBLIC|MEMF_CLEAR))) 
		Quit(RETURN_FAIL);
    else {
	ilbms[SCR] = ilbms[0] + 1;
    }

/*
 * Here we set up default ILBMInfo fields for our
 * application's frames.
 * Above we have defined the propery and collection chunks
 * we are interested in (some required like BMHD)
 * Since all of our frames are for ILBM's, we'll initialize
 * one default frame and clone the others from it.
 */
    ilbms[DEF]->ParseInfo.propchks	= ilbmprops;
    ilbms[DEF]->ParseInfo.collectchks	= ilbmcollects;
    ilbms[DEF]->ParseInfo.stopchks	= ilbmstops;

    ilbms[DEF]->windef	= &mynw;
/* 
 * Initialize our working ILBM frames from our default one
 */
    *ilbms[SCR] = *ilbms[DEF];	/* for our screen */

/* 
 * Alloc one IFF handles (one for screen frame) 
 */
    if(!(ilbms[SCR]->ParseInfo.iff = AllocIFF())) Quit(RETURN_FAIL);	/* not iff */

/* Load and display an ILBM
 */
    if(error = showilbm(ilbms[SCR],ilbmname))
	{
	printf("Can't load background \"%s\"\n",ilbmname);
	Quit(RETURN_WARN);
	}

    /* These were set up by our successful showilbm() above */
    win = ilbms[SCR]->win;	/* our window */
    wrp = ilbms[SCR]->wrp;	/* our window's RastPort */
    scr = ilbms[SCR]->scr;	/* our screen */
    vp  = ilbms[SCR]->vp;		/* our screen's ViewPort */

    origbitmap = scr->RastPort.BitMap;
    sdepth = scr->RastPort.BitMap->Depth;
    swidth = scr ->Width;
    sheight = scr->Height;

    if((altbitmap = AllocMem((LONG)sizeof(struct BitMap), MEMF_CLEAR))) {
	InitBitMap(altbitmap, sdepth, swidth, sheight);
	if(!getplanes(altbitmap, sdepth, swidth,sheight))Quit(10); /* problem */
	/* copy the bitplanes */
	BltBitMap(origbitmap,0,0,altbitmap,0,0,swidth,sheight,0xC0, 0xFF, NULL);
    }

    ScreenToFront(scr);
}

void cleaniff(void)
{
   if(ilbms[SCR]) {
   	if(ilbms[SCR]->scr)		unshowilbm(ilbms[SCR]);
   	if(ilbms[SCR]->ParseInfo.iff)	FreeIFF(ilbms[SCR]->ParseInfo.iff);
   }

   if(ilbms[0]) {
	FreeMem(ilbms[0],UICOUNT * sizeof(struct ILBMInfo));
   }
   if(altbitmap) {
	freeplanes(altbitmap, sdepth, swidth, sheight);
	FreeMem(altbitmap,sizeof(struct BitMap));
   }
}

long getplanes(struct BitMap *bitmap, LONG depth, LONG width, LONG height)
{
int plane_num;

    for(plane_num = 0; plane_num < depth; plane_num++) {
    	if((bitmap->Planes[plane_num] = (PLANEPTR)AllocRaster(width,height))) {
	    BltClear(bitmap->Planes[plane_num], (width/8)*height, 1);
	}
        else {
	    freeplanes(bitmap, depth, width, height);
            return(NULL);
	}
    }
return(TRUE);
}

void freeplanes(struct BitMap *bitmap, LONG depth, LONG width, LONG height)
{
int plane_num;

    for(plane_num = 0; plane_num < depth; plane_num++) {
	if(bitmap->Planes[plane_num]) {
	    FreeRaster(bitmap->Planes[plane_num], width, height);
	    bitmap->Planes[plane_num] = NULL;
	}
    }
}