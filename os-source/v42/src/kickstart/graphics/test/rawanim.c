#include <exec/types.h>
#include <clib/exec_protos.h>
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

struct Library *GfxBase,*IntuitionBase,*IFFBase;
struct Screen *myscreen;
struct BitMap *extrabitmap;
ULONG *infile;
struct DBufInfo *myDBI;

struct DBufInfo *AllocDBufInfo(struct ViewPort *vp);
VOID FreeDBufInfo(struct DBufInfo *dbi);

struct MsgPort *myport;
struct MsgPort *myport1;

ULONG *framep[300];
ULONG nframes=0;

/*#define DEBUG printf */
#define DEBUG if (0) printf

void Fail(char *msg)
{
	int i;
	if (msg) printf("%s\n",msg);
	DEBUG("freeing frames\n");
	for(i=0;i<nframes;i++)
		if (framep[i])	FreeMem(framep[i],64000);
	if (myport) DeletePort(myport);
	if (myport1) DeletePort(myport1);
	DEBUG("freeing 2nd buffer");
	if (extrabitmap) FreeBitMap(extrabitmap);
	DEBUG("freedbi\n");
	FreeDBufInfo(myDBI);	/* null free OK */
	DEBUG("closing screen\n");
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


void __asm copylongs(register __a0 ULONG *src, register __a1 ULONG * dst,register __d0 UWORD cnt);

copyframe(int fnum, struct BitMap *bm)
{
	int i;
	if (fnum >nframes) fnum=0;
	for(i=0;i<8;i++)
		CopyMemQuick(framep[fnum]+i*2000,bm->Planes[i],8000);
/*		copylongs(framep[fnum]+i*2000,bm->Planes[i],2000); */
}


int abort_flag=0;

break_handler()
{
	abort_flag=1;
	return(0);	/* tell to continue */
}



main(argc,argv)
int argc;
char **argv;
{
	int fnum;
	ULONG handle;
	ULONG safetowrite=-1;
	ULONG safetochange=-1;
	GfxBase=openlib("graphics.library",39);
	IFFBase=openlib("iff.library",0);
	IntuitionBase=openlib("intuition.library",37);
	onbreak(break_handler);
	if (argc!=4)
		Fail("format is Rawanim palettefile nametemplate skip\n");
	else {
		if (infile=OpenIFF(argv[1]))
		{
			ULONG scrwidth,scrheight,scrdepth,scrmode;
			ULONG *chunk;
			ULONG count;
			UBYTE *ptr;
			ULONG i,j;
			char filename[80];
			struct BitMap *bptrs[2];

			for(fnum=000; fnum<300; fnum+=atoi(argv[3]))
			{
				sprintf(filename,argv[2],fnum);
				DEBUG("fnum=%d attempting to open %s... ",fnum,filename);
				handle=Open(filename,MODE_OLDFILE);
				if (! handle) fnum=1000; 
				else {
					framep[nframes]=AllocMem(64000,0);
					if (! framep[nframes]) Fail("couldn't allocate memory");
					Read(handle,framep[nframes],64000);
					DEBUG("success\n");
					Close(handle);
					nframes++;
				}
			}


			DEBUG("nframes=%d\n",nframes);
			myscreen=OpenScreenTags(0l,SA_Width,320,SA_Height,200,SA_Depth,8,
				SA_DisplayID,0x800);
			if (! myscreen) Fail("couldn't open screen");
			myDBI=AllocDBufInfo(&(myscreen->ViewPort));
			if (! myDBI) Fail("Coudn't allocate a DBufInfo");

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

			CloseIFF(infile); infile=0;


			if (! (extrabitmap=AllocBitMap(320,200,8,BMF_DISPLAYABLE,0l)))
				Fail("couldn't allocate second bitmap");

			copyframe(0,&myscreen->BitMap);
			copyframe(1,extrabitmap);
			myport=CreatePort(0l,0l);
			myport1=CreatePort(0l,0l);
			if (! (myport && myport1)) Fail("Can't create port");

			bptrs[0]=&(myscreen->BitMap);
			bptrs[1]=extrabitmap;
			i=1;
			myDBI->dbi_SafeMessage.mn_ReplyPort=myport;
/*			myDBI->dbi_DispMessage.mn_ReplyPort=myport1; */
			while ( *((UBYTE *) 0xbfe001) & 128)
				for(j=0;(j<nframes) && (*((UBYTE *) 0xbfe001) & 128) ;j++)
				{
					if (! safetowrite)
					{
						while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
						safetowrite=-1;
					}
					copyframe(j,bptrs[i]);
/*					if (! safetochange)
					{
						while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit));
						safetochange=-1;
					} */
					ChangeVPBitMap(&(myscreen->ViewPort),bptrs[i],myDBI);
					safetochange=0;
					safetowrite=0;
					i^=1;
				}
			DEBUG("click\n");
				if (! safetowrite)
				{
					while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
					safetowrite=-1;
				}
/*				if (! safetochange)
				{
					while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit));
					safetochange=-1;
				} */
				ChangeVPBitMap(&(myscreen->ViewPort),bptrs[count & 1],myDBI);			
				while(! GetMsg(myport)) Wait(1l<<(myport->mp_SigBit));
/*				while(! GetMsg(myport1)) Wait(1l<<(myport1->mp_SigBit)); */
			Fail("done");
		}
	}
}
