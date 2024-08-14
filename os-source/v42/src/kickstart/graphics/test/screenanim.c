#include <exec/types.h>
#include <clib/exec_protos.h>
#include <graphics/gfxbase.h>
#include "/view.h"
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <stdio.h>
#include <dos/dos.h>
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

struct Library *IntuitionBase,*IFFBase;
struct GfxBase *GfxBase;
struct Screen *myscreen;
struct BitMap *extrabitmap;
ULONG *infile;
struct DBufInfo *myDBI;

struct DBufInfo *AllocDBufInfo(struct ViewPort *vp);
VOID FreeDBufInfo(struct DBufInfo *dbi);

struct MsgPort *myport;
struct MsgPort *myport1;

void Fail(char *msg)
{
	if (msg) printf("%s\n",msg);
	if (myport) DeletePort(myport);
	if (myport1) DeletePort(myport1);
	if (extrabitmap) FreeBitMap(extrabitmap);
	FreeDBufInfo(myDBI);	/* null free OK */
	if (myscreen) CloseScreen(myscreen);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);
	if (infile) CloseIFF(infile);
	if (IFFBase) CloseLibrary(IFFBase);
	exit(0);
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

#define BOTHSYNCS 0

main(argc,argv)
int argc;
char **argv;
{
	ULONG safetowrite=-1;
	ULONG safetochange=-1;
	ULONG nframes=0;
	ULONG vbcount;
	GfxBase=(struct GfxBase *) openlib("graphics.library",39);
	IFFBase=openlib("iff.library",0);
	IntuitionBase=openlib("intuition.library",37);
	if (argc==2)
	{
		if (infile=OpenIFF(argv[1]))
		{
			ULONG scrwidth,scrheight,scrdepth,scrmode;
			ULONG *form,*chunk,*loopform;
			ULONG count;
			UWORD colortable[256];
			UBYTE *ptr;
			ULONG i;
			struct IFFL_BMHD *bmhd;
			struct BitMap *bptrs[2];
			if(infile[2] != ID_ANIM) Fail("Not an ANIM file");
			form=infile+3;
			if(!(bmhd=GetBMHD(form))) Fail("BitMapHeader not found");

			scrwidth = bmhd->w;
			scrheight = bmhd->h;
			scrdepth = bmhd->nPlanes;
			chunk=FindChunk(form,ID_CAMG);
			if (! chunk) Fail("no CAMG");
			scrmode=*(chunk+2);
			if ((scrmode & 0xffff0000)==0)
			{
				scrmode &= ~(SPRITES | PFBA | GENLOCK_VIDEO | GENLOCK_AUDIO);
			}
			myscreen=OpenScreenTags(0l,SA_Width,scrwidth,SA_Height,scrheight,SA_Depth,scrdepth,
				SA_DisplayID,scrmode);
			if (! myscreen) Fail("couldn't open screen");
			myDBI=AllocDBufInfo(&(myscreen->ViewPort));
			if (! myDBI) Fail("Coudn't allocate a DBufInfo");

			chunk=FindChunk(form,ID_CMAP);
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

			if (! (extrabitmap=AllocBitMap(scrwidth,scrheight,scrdepth,BMF_DISPLAYABLE,0l)))
				Fail("couldn't allocate second bitmap");

			if(!DecodePic(form,&myscreen->BitMap)) Fail("Can't decode picture");
			DecodePic(form,extrabitmap);
			myport=CreatePort(0l,0l);
			myport1=CreatePort(0l,0l);
			if (! (myport && myport1)) Fail("Can't create port");

			bptrs[0]=&(myscreen->BitMap);
			bptrs[1]=extrabitmap;
			form=FindChunk(infile+3,0L);		/* First FORM containing a DLTA */
			if(!ModifyFrame(form,bptrs[0])) Fail("Can't decode frame");
			loopform=FindChunk(form,0l);
			i=1;
			myDBI->dbi_SafeMessage.mn_ReplyPort=myport;
#if BOTH_SYNCS
			myDBI->dbi_DispMessage.mn_ReplyPort=myport1;
#endif
			vbcount=GfxBase->VBCounter;
			while (! (SetSignal(0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C))
				for(form=loopform; *form==ID_FORM; form=FindChunk(form,0l))
				{
					if (! form) Fail("can't find form");
					if (! safetowrite)
					{
						while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
						safetowrite=-1;
					}
					if(!ModifyFrame(form,bptrs[i])) Fail("Can't decode frame");
					nframes++;
#if BOTH_SYNCS
					if (! safetochange)
					{
						while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit));
						safetochange=-1;
					}
#endif
					ChangeVPBitMap(&(myscreen->ViewPort),bptrs[i],myDBI);
					safetochange=0;
					safetowrite=0;
					i^=1;
				}
				if (! safetowrite)
				{
					while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
					safetowrite=-1;
				}
#if BOTH_SYNCS
				if (! safetochange)
				{
					while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit));
					safetochange=-1;
				}
#endif
				nframes++;
				ChangeVPBitMap(&(myscreen->ViewPort),bptrs[count & 1],myDBI);			
				while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
#if BOTH_SYNCS
				while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit));
#endif
				printf("frames=%ld\n vbc=%ld\n",nframes,GfxBase->VBCounter-vbcount);
			Fail(0);
		}
	} else Fail("can't open file");
}
