/*
 * showanim.c by Christian Ludwig & Carolyn Schppner (CATS)
 * decompression code based on Jim Kent code
 * requires at least V39 IFF modules
 *
 * This is a preliminary example of using iffparse and the NewIFF39 modules,
 * plus some new ANIM modules, to load and display an ANIM.
 *
 * The example has many shortcomings:
 *
 *    - Since all memory allocations are done before parsing the ANIM,
 *      more memory than necessary may be allocated for XOR ANIMs
 *      (i.e. a second alternate buffer bitmap is allocated).
 *      Note also that apparently many older XOR brush ANIMs do not
 *      properly state that they are XOR.  This code kludegs by
 *      assuming that all interleave=1 bits=0 anims are actually XOR.
 *
 *    - IFFParse parsing does not allow seeking back to a file position
 *      such as would be desirable for looping an ANIM.  Looping is
 *      not implemented in this example.
 *
 *    - Playing of the the ANIM is not optimal for screens since the
 *      ANIM changes are being done in offscreen buffers and the whole
 *      resulting image is blitted to the screen for each frame.
 *      For V39, a more optimal method would be to use the V39
 *      double-buffered screen routines, and for V37 and earlier
 *      to use some older method for swappinga screen's bitmap.
 *
 *    - Playing of an ANIM in a window does not allocate or remap colors,
 *      and just shows the ANIM as-is in the screen's colors.
 *
 *    - This example streams but does not spool - i.e. it plays the ANIM
 *      from disk, but does not have any code to load up additional
 *      deltas ahead of time.
 *
 *    - This example uses no timing - it just plays the ANIM as fast
 *      as it can.
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


#include "showanim_rev.h"
UBYTE vers[] = VERSTAG;
UBYTE Copyright[] = VERS " Show ANIM in window (V37+) or screen - Freely Redistributable";
char *usage =
"Usage: showanim animname [BUFK=n (default 64)] [SCREEN] (SCREEN only if <V37)";

#define MINARGS 2

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

#define MAXBUFFS 2

struct Window *playbackwindow, *pubwindow;

struct Screen *pubscreen;

struct BitMap *bufferbitmap[MAXBUFFS] = {0};


static LONG *dltabuffer;
static LONG dltabufsize = 64 * 1024;

UBYTE destbuffer = 0, altbuffer = 1, temp = 0;


void Quit(char *whytext, int failcode)
{
struct BitMap *tmpbitmap;

	free_ytable();

	if (dltabuffer) FreeMem(dltabuffer,dltabufsize);

	if (ilbm)
	{
		if(bufferbitmap[1])
			{
			tmpbitmap = ilbm->brbitmap;
			ilbm->brbitmap = bufferbitmap[1];
			freebitmap(ilbm);
			ilbm->brbitmap = tmpbitmap;
			}

		if(ilbm->brbitmap)	unloadbrush(ilbm);

		if(ilbm->scr)		closedisplay(ilbm);

        	if(ilbm->ParseInfo.iff) FreeIFF(ilbm->ParseInfo.iff);

		FreeMem(ilbm,sizeof(struct ILBMInfo));
	}

	if(IntuitionBase->lib_Version >= 37)
	{
		if (pubwindow) CloseWindow(pubwindow);
		if (pubscreen) UnlockPubScreen(NULL,pubscreen);
	}

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
	struct RastPort *destrastport;
	struct ContextNode     *cn;
	UBYTE *animname=NULL;
	LONG error = 0L;
	struct StoredProperty *sp;
	BitMapHeader *bmhd;
	AnimHeader *anhd;
	BOOL Done = FALSE, UseScreen = FALSE;
	int k,n;

	FromWb = argc ? FALSE : TRUE;

	if((argc<MINARGS)||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
		Quit("",RETURN_OK);
	}

	animname = argv[1];

	if(argc > 2)
	{
		for(k=2; k<argc; k++)
		{
			if(!(stricmp(argv[k],"SCREEN"))) UseScreen = TRUE;
			else if(!(strnicmp(argv[k],"BUFK=",5)))
			{
				if(n = atoi(&argv[k][5]))
					dltabufsize = n * 1024; 
			}
		}
	}

	if ((GfxBase = OpenLibrary("graphics.library",34))==NULL)
		Quit("graphics.library is too old <V34",RETURN_WARN);

	if ((IntuitionBase = OpenLibrary("intuition.library",34))==NULL)
		Quit("intuition.library is too old <V34",RETURN_WARN);

	if ((IFFParseBase = OpenLibrary("iffparse.library",37))==NULL)
		Quit("Can't open iffparse library",RETURN_WARN);


	if(IntuitionBase->lib_Version < 37)
	{
		if(!UseScreen)
		{
			UseScreen = TRUE;
			printf("OS <V37 - using SCREEN option\n");
		}
	}

	if(!UseScreen)
	{	
		if (!(pubscreen = LockPubScreen(NULL)))
			Quit("Can't lock pubscreen",RETURN_FAIL);

		// Here's where we'll ask Intuition about the screen.
		if(!(drawinfo=GetScreenDrawInfo(pubscreen)))
			Quit("Can't get DrawInfo",RETURN_FAIL);

		pubscreendepth=drawinfo->dri_Depth;

		// We have to tell intuition when we're done with the DrawInfo.
		FreeScreenDrawInfo(pubscreen, drawinfo);
	}

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

	bmhd   = (BitMapHeader *) sp->sp_Data;
	depth  = bmhd->nPlanes;
	width  = bmhd->w;
	height = bmhd->h;

	/* use ILBMInfo temporarily to alloc another same-size bitmap
	 */
	bufferbitmap[0] = ilbm->brbitmap; /* loadbrush allocated */
	ilbm->brbitmap = NULL;
	error = getbitmap(ilbm);
	bufferbitmap[1] = ilbm->brbitmap;  /* second bitmap if no error */
	ilbm->brbitmap = bufferbitmap[0]; /* restore */
	if(error)
	{
		Quit("Can't allocate second bitmap",RETURN_FAIL);
	}
	
	if(UseScreen)
	{
		playbackwindow = opendisplay(ilbm,
			bmhd->pageWidth, bmhd->pageHeight, bmhd->nPlanes,
                        ilbm->camg);
		if(!playbackwindow)	Quit("Can't open screen",RETURN_FAIL);
	}
	else
	{
		pubwindow = OpenWindowTags(NULL,
				WA_PubScreen	,pubscreen,
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
				TAG_END);
		if(!pubwindow) Quit("Can't open window",RETURN_FAIL);
		else playbackwindow = pubwindow;
	}

	destrastport = playbackwindow->RPort;


	BltBitMap(bufferbitmap[0],0,0,bufferbitmap[1],0,0,width,height,0xC0,0xFF,NULL);

	BltBitMapRastPort(bufferbitmap[1],0,0,destrastport,0,0,width,height,0xC0);

	if(ilbm->scr)
	{
		setcolors(ilbm,&(ilbm->scr->ViewPort));
		ScreenToFront(ilbm->scr);
	}

	if (!(dltabuffer = AllocMem(dltabufsize,0L)))
		Quit("Can't allocate dlta buffer",RETURN_FAIL);

	if (!(make_ytable((bufferbitmap[0]->BytesPerRow)<<3,bufferbitmap[0]->Rows)))
		Quit("Can't allocate ytable space",RETURN_FAIL);

	while(!Done)
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

		if (error = loaddlta(ilbm->ParseInfo.iff,
			bufferbitmap[destbuffer],anhd,dltabuffer,dltabufsize))
			Quit("Can't loaddlta - may need higher BUFK",RETURN_FAIL);

		BltBitMapRastPort(bufferbitmap[destbuffer],0,0,destrastport,0,0,width,height,0xC0);

		if(anhd->interleave == 0)  /* we only handle 0 and 1 */
		{
			temp=altbuffer;
			altbuffer=destbuffer;
			destbuffer=temp;
		}
	}

	Quit("",0);
}
