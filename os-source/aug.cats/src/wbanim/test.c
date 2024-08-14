/*
 * wbanim.c by Christian Ludwig (CATS)
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <graphics/rastport.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <utility/hooks.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdio.h>
#include <stdlib.h>

#include "iffp/ilbm.h"
#include "iffp/anim.h"

#include "iffp/ilbmapp.h"
#include "iffp/packer.h"

struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
struct Library *IFFParseBase = NULL;


#define MINARGS 2
char *vers = "\0$VER: wbanim 37.4";
char *Copyright = "wbanim v37.4 (c) 1993 Commodore International Services Co., Ltd.";
char *usage = "Usage: wbanim animname";


#ifdef LATTICE
int  CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


/* ILBM Property chunks to be grabbed
 * List BMHD, CMAP and CAMG first so we can skip them when we write
 * the file back out (they will be written out with separate code)
 */
LONG	ilbmprops[] = {
		ID_ILBM, ID_BMHD,
		ID_ILBM, ID_ANHD,
		ID_ILBM, ID_CMAP,
		ID_ILBM, ID_CAMG,
		ID_ILBM, ID_AUTH,
		ID_ILBM, ID_Copyright,
		TAG_DONE
		};

/* ILBM Collection chunks (more than one in FORM) to be gathered */
/* None in this case, example would be CRNG chunks */
LONG	ilbmcollects[] = { TAG_DONE };

/* ILBM Chunks to stop on */
LONG	ilbmstops[] = {
		ID_ILBM, ID_BODY,
		ID_ILBM, ID_DLTA,
		TAG_DONE
		};



/* For our ILBM/ANIM frame */
struct ILBMInfo  *ilbm;

BOOL FromWb;

UBYTE nomem[]  = "Not enough memory\n";
UBYTE noiffh[] = "Can't alloc iff\n";

#define MAXBUFFS 4

UBYTE c,numbuffs = 2;

struct Window *playbackwindow;

struct Screen *screen, *bufferscreen[MAXBUFFS];

static LONG *dltabuffer;

static WORD *colortable;
static USHORT ncolors;

void Quit(char *whytext, int failcode)
{
	free_ytable();

	if (dltabuffer) FreeMem(dltabuffer,48000);

	if (colortable) FreeMem(colortable, (ULONG) (ncolors * sizeof(ColorRegister)));

	for(c=0;c<numbuffs;++c)
	{
		if (bufferscreen[c]) CloseScreen(bufferscreen[c]);
	}

	if (playbackwindow) CloseWindow(playbackwindow);

	if (ilbm)
	{
		unloadbrush(ilbm);
		FreeMem(ilbm,sizeof(struct ILBMInfo));
	}

	if (screen) UnlockPubScreen(NULL,screen);

	if (IFFParseBase) 	CloseLibrary(IFFParseBase);
	if (IntuitionBase) 	CloseLibrary(IntuitionBase);
	if (GfxBase) 		CloseLibrary(GfxBase);

	if(*whytext)	printf("%s (return code %ld)\n",whytext,failcode);

	exit(failcode);
}




void main(int argc, char **argv)
{
	struct DrawInfo *drawinfo;

	UWORD pubscreendepth, depth, width, height;

	struct BitMap *bufferbitmap[MAXBUFFS];

	struct RastPort *windowrastport;
	struct ContextNode     *cn;

	UBYTE displaybuffer = 0, destbuffer = 1, temp = 0;

	UBYTE *animname=NULL;
	LONG error = 0L;

	struct StoredProperty *sp;
	BitMapHeader *bmhd;
	AnimHeader *anhd;

	BOOL done = FALSE;


	FromWb = argc ? FALSE : TRUE;

	if((argc<MINARGS)||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
		Quit("",RETURN_OK);
	}

	animname = argv[1];


	if ((GfxBase = OpenLibrary("graphics.library",37))==NULL)
		Quit("graphics.library is too old <V37",RETURN_WARN);

	if ((IntuitionBase = OpenLibrary("intuition.library",37))==NULL)
		Quit("intuition.library is too old <V37",RETURN_WARN);

	if ((IFFParseBase = OpenLibrary("iffparse.library",37))==NULL)
		Quit("Can't open iffparse library",RETURN_WARN);

	if (!(screen = LockPubScreen(NULL)))
		Quit("Can't lock pubscreen",RETURN_FAIL);

	// Here's where we'll ask Intuition about the screen.
	if(!(drawinfo=GetScreenDrawInfo(screen)))
		Quit("Can't get DrawInfo",RETURN_FAIL);

	pubscreendepth=drawinfo->dri_Depth;

	// We have to tell intuition when we're done with the DrawInfo.
	FreeScreenDrawInfo(screen, drawinfo);


	/* 
	 * Alloc our ILBMInfo struct
	 */
    	if(!(ilbm = (struct ILBMInfo *)
	   AllocMem(sizeof(struct ILBMInfo),MEMF_PUBLIC|MEMF_CLEAR))) 
		Quit(nomem,RETURN_FAIL);

	/*
	 * Here we set up ILBMInfo fields for our application.
	 * Above we have defined the propery and collection chunks
	 * we are interested in (some required like BMHD and ANHD)
	 */
	ilbm->ParseInfo.propchks	= ilbmprops;
    	ilbm->ParseInfo.collectchks	= ilbmcollects;
    	ilbm->ParseInfo.stopchks	= ilbmstops;

	/* 
	 * Alloc IFF handle for frame
	 */
        if(!(ilbm->ParseInfo.iff = AllocIFF()))
		Quit("Can't alloc IFFHandle",RETURN_FAIL);

	// Open the anim file and load the initial ILBM into a bitmap
    	if(error = loadbrush(ilbm,animname))
		Quit(IFFerr(error),RETURN_WARN);


	if(!(sp = FindProp(ilbm->ParseInfo.iff, ID_ILBM, ID_BMHD)))
		Quit("No BMHD, no ANIM",RETURN_FAIL);

	bmhd = (BitMapHeader *) sp->sp_Data;
	depth = bmhd->nPlanes;
	width = bmhd->w;
	height = bmhd->h;

	if (!(playbackwindow = OpenWindowTags(NULL,
					WA_PubScreen	,screen,
					WA_InnerWidth	,width,
					WA_InnerHeight	,height,
					WA_DragBar	,TRUE,
					WA_DepthGadget	,TRUE,
					WA_CloseGadget	,TRUE,
					WA_Activate	,TRUE,
					WA_SimpleRefresh,TRUE,
					WA_NoCareRefresh,TRUE,
					WA_Title	,animname,
					WA_GimmeZeroZero,TRUE,
					TAG_END)))
	{
		Quit("Can't open window",RETURN_FAIL);
	}

	windowrastport = playbackwindow->RPort;

	for(c=0;c<numbuffs;++c)
	{
		if (!(bufferscreen[c] = OpenScreenTags(NULL,
					SA_Width	,width,
					SA_Height	,height,
					SA_Depth	,depth,
					SA_Quiet	,TRUE,
					SA_Behind	,TRUE,
					SA_FullPalette	,TRUE,
					TAG_END)))
		{
			Quit("Can't open buffer screen",RETURN_FAIL);
		}

		bufferbitmap[c] = bufferscreen[c]->RastPort.BitMap;
	}


	BltBitMap(ilbm->brbitmap,0,0,bufferbitmap[displaybuffer],0,0,width,height,0xC0,0xFF,NULL);

	BltBitMapRastPort(bufferbitmap[displaybuffer],0,0,windowrastport,0,0,width,height,0xC0);

	setcolors(ilbm,&(screen->ViewPort));

	if (!(dltabuffer = AllocMem(48000,0L)))
		Quit("Can't allocate dlta buffer",RETURN_FAIL);

	if (!(make_ytable((bufferbitmap[destbuffer]->BytesPerRow)<<3,bufferbitmap[destbuffer]->Rows)))
		Quit("Can't allocate ytable space",RETURN_FAIL);


	BltBitMap(bufferbitmap[displaybuffer],0,0,bufferbitmap[destbuffer],0,0,width,height,0xC0,0xFF,NULL);


	while(!done)
	{
                if((error = ParseIFF(ilbm->ParseInfo.iff, IFFPARSE_SCAN))==IFFERR_EOF)
                        Quit("",RETURN_OK);

		if(error = getcontext(ilbm->ParseInfo.iff))
		{
			if(error == IFFERR_EOF) Quit("",RETURN_OK);
		}

		if (sp = FindProp(ilbm->ParseInfo.iff, ID_ILBM, ID_ANHD))
		{
			anhd = (AnimHeader *) sp->sp_Data;
		}
		else
		{
       			if (cn = (CurrentChunk (ilbm->ParseInfo.iff)))
			{
				cn = ParentChunk(cn);
        			printf("Error - This is a %.4s %.4s\n",&cn->cn_Type,&cn->cn_ID);
			}
			Quit("No ANHD, No ANIM",RETURN_FAIL);

		}

		// printf("op = %ld, interleave = %ld, bits = $%08lx\n",anhd->operation,anhd->interleave,anhd->bits);

		if (error = loaddlta(ilbm->ParseInfo.iff,bufferbitmap[destbuffer],anhd,dltabuffer))
			Quit("Can't loaddlta",RETURN_FAIL);

		BltBitMapRastPort(bufferbitmap[destbuffer],0,0,windowrastport,0,0,width,height,0xC0);

		if(anhd->interleave == 0)  /* we only handle 0 and 1 */
		{
			temp=displaybuffer;
			displaybuffer=destbuffer;
			destbuffer=temp;
		}
	}

	Quit("Done.",0);
}
