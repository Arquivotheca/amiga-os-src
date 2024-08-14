#include <exec/types.h>
#include <clib/exec_protos.h>
#include "/view.h"
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <stdio.h>
#define NO_PRAGMAS 1
#include "pd:ifflib/iff.h"

#pragma libcall IFFBase OpenIFF 1e 801
#pragma libcall IFFBase CloseIFF 24 901
#pragma libcall IFFBase FindChunk 2a 902
#pragma libcall IFFBase GetBMHD 30 901
#pragma libcall IFFBase GetColorTab 36 8902
#pragma libcall IFFBase DecodePic 3c 8902
#pragma libcall IFFBase SaveBitMap 42 a9804
/*#pragma libcall IFFBase SaveClip 48 210a9808*/
#pragma libcall IFFBase IFFError 4e 0
#pragma libcall IFFBase GetViewModes 54 901
#pragma libcall IFFBase NewOpenIFF 5a 802
#pragma libcall IFFBase ModifyFrame 60 8902

struct Library *GfxBase,*IntuitionBase,*IFFBase;
struct Screen *myscreen;
ULONG *infile;

void Fail(char *msg)
{
	if (msg) printf("%s\n",msg);
	if (myscreen) CloseScreen(myscreen);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);
	if (infile) CloseIFF(infile);
	if (IFFBase) CloseLibrary(IFFBase);
	Exit(0);
}


struct Library *openlib(char *name,ULONG version)
{
	struct Library *t1;
	t1=OpenLibrary(name,version);
	if (! t1)
	{
		printf("error- needs %s version %d\n",name,version);
		Fail(0l);
	}
	else return(t1);
}




ULONG lcolortab[2+256*3];

main(argc,argv)
int argc;
char **argv;
{
	ULONG count;
	GfxBase=openlib("graphics.library",39);
	IFFBase=openlib("iff.library",0);
	IntuitionBase=openlib("intuition.library",37);
	if (argc==2)
	{
		if (infile=OpenIFF(argv[1]))
		{
			ULONG scrwidth,scrheight,scrdepth,scrmode;
			ULONG *chunk;
			UBYTE *ptr;
			ULONG i;
			struct IFFL_BMHD *bmhd;
			struct BitMap *bptrs[2];
			if(!(bmhd=GetBMHD(infile))) Fail("BitMapHeader not found");

			scrwidth = bmhd->w;
			scrheight = bmhd->h;
			scrdepth = bmhd->nPlanes;
			chunk=FindChunk(infile,ID_CAMG);
			if (! chunk) Fail("no CAMG");
			scrmode=*(chunk+2);
			if ((scrmode & 0xffff0000)==0)
			{
				scrmode &= ~(SPRITES | PFBA | GENLOCK_VIDEO | GENLOCK_AUDIO);
			}
			myscreen=OpenScreenTags(0l,SA_Width,scrwidth,SA_Height,scrheight,SA_Depth,scrdepth,
				SA_DisplayID,scrmode,SA_Behind,-1,SA_Quiet,-1,TAG_END);

			chunk=FindChunk(infile,ID_CMAP);
			if (! chunk) Fail("no color table");
			chunk++;
			count=(*(chunk++))/3;
			ptr=chunk;
			if (count>256) count=256;
			lcolortab[0]=(count<<16)+0;
			for(i=0;i<count;i++)
			{
				lcolortab[i*3+1]=*(ptr++)<<24;
				lcolortab[i*3+2]=*(ptr++)<<24;
				lcolortab[i*3+3]=*(ptr++)<<24;
			}
			LoadRGB32(&(myscreen->ViewPort),lcolortab);
			if(!DecodePic(infile,myscreen->RastPort.BitMap)) Fail("Can't decode picture");
			ScreenToFront(myscreen);
			getchar();
			Fail(0);
		}
	}
}
