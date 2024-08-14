/* $$TABS=4 */

#include <stdio.h>

#include "ifflib.pragmas"	/* pragmas for iff.library */

#include "iff.h"
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <intuition/intuition.h>

/*

	This program will open a window on the workbench screen, and display
	any color-mapped IFF picture in it.

	V39 graphics features, including palette allocation, 32 bit RGB colors,
	and bitmap allocation are used.

*/

struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *IFFBase;
struct BitMap *mybitmap;
struct Window *mywindow;
struct Library *OpenLibrary();
struct Screen *wbscreen;
struct IntuiMessage *mymsg;
IFFFILE pichandle;
ULONG wbdepth,pic_width,pic_height,pic_depth;
ULONG pic_ncolors;

struct BitMap *AllocBitMap();
void FreeBitMap();

void cleanup()
{
	if (mywindow) CloseWindow(mywindow);
	if (mybitmap) FreeBitMap(mybitmap);
	if (pichandle) CloseIFF(pichandle);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IFFBase) CloseLibrary(IFFBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);
}

void abort(message)
{
	printf("%s\x07\n",message);
	cleanup();
	exit(1);
}


void redraw_window()
{
		BltBitMapRastPort(mybitmap,0,0,mywindow->RPort,
			mywindow->BorderLeft,mywindow->BorderTop,
			min(mywindow->Width - mywindow->BorderRight - mywindow->BorderLeft,pic_width),
			min(mywindow->Height - mywindow->BorderTop - mywindow->BorderBottom,pic_height),
			0xc0);
}

struct BitMapHeader *bmh;

int done=0;

ULONG desired_colors[256][3];
ULONG got_colors[512][3];
struct ColorMap *wbcm;
struct ViewPort *wbvp;
ULONG wbcolor[3];
char color_table[256];
char color_table2[256];
char allocated_colors[512];
int n_allocated=0;

void __asm XLateBitMap(register __a0 struct BitMap * sbm,
						register __a1 struct BitMap * dbm,
						register __a2 char * table1,
						register __a3 char * table2);

#define SQ(x) ((x)*(x))
#define AVGC(i1,i2,c) ((got_colors[i1][c]>>1)+(got_colors[i2][c]>>1))

unsigned long color_error(r1,r2,g1,g2,b1,b2)
unsigned long r1,g1,b1,r2,g2,b2;
{
	r1 >>= 24; g1 >>= 24; b1 >>= 24;
	r2 >>= 24; g2 >>= 24; b2 >>= 24;
	return(SQ(r1-r2)+SQ(g1-g2)+SQ(b1-b2));
}

main(argc,argv)
int argc;
char **argv;
{
	char *cmap_adr;
	int i,gotcolor;
	if (argc != 2)
		abort("format : wbpic <filename>\n");
	if (! (GfxBase=OpenLibrary("graphics.library",37l)))
		abort("Can't open V39 graphics");
	if (! (IntuitionBase=OpenLibrary("intuition.library",37l)))
		abort("Can't open intuition");
	if (! (IFFBase=OpenLibrary("iff.library",0l)))
		abort("Can't open iff.library");

	if (! (wbscreen=LockPubScreen("Workbench"))) abort("can't find workbench");

	wbdepth=wbscreen->BitMap.Depth;	/* don't use screen->bitmap!! */

	wbcm=wbscreen->ViewPort.ColorMap;
	wbvp=&(wbscreen->ViewPort);

	UnlockPubScreen(NULL,wbscreen);
	
	if (! (pichandle=OpenIFF(argv[1]))) abort("Can't open picture file");
	if (! (bmh=GetBMHD(pichandle))) abort("Can't find BitMap header");

	pic_width=bmh->w; pic_height=bmh->h;
	pic_depth=bmh->nPlanes;
	pic_ncolors=1<<pic_depth;
			
	mybitmap=AllocBitMap(pic_width,pic_height,max(wbdepth,pic_depth),BMF_CLEAR,NULL);

	if (! mybitmap) abort("couldnt allocate bitmap space");

	if (! DecodePic(pichandle,mybitmap)) abort("couldn't decode picture");

	/* now, let's remap the picture */

	cmap_adr=FindChunk(pichandle,ID_CMAP);
	if (! cmap_adr) abort("No CMAP found");
	cmap_adr += 8;
	for(i=0;i<pic_ncolors;i++)
	{
		desired_colors[i][0]=*(cmap_adr++)<<24;
		desired_colors[i][1]=*(cmap_adr++)<<24;
		desired_colors[i][2]=*(cmap_adr++)<<24;
		gotcolor=ObtainBestPen(wbcm,desired_colors[i][0],desired_colors[i][1],
								desired_colors[i][2],0);
		GetRGB32(wbcm,gotcolor,1,&(got_colors[n_allocated][0]));

		color_table[i]=gotcolor;
		color_table2[i]=gotcolor;
		allocated_colors[n_allocated++]=gotcolor;
	}

	/* now, attempt to dither for colors that were too far off the mark */


	for(i=0;i<pic_ncolors;i++)
	{
		unsigned int testr,testg,testb,testerr1,testerr,j;

		testerr=color_error(desired_colors[i][0],got_colors[i][0],
						desired_colors[i][1],got_colors[i][1],
						desired_colors[i][2],got_colors[i][2]);
		if (testerr>2000)
		{
			for(j=0;j<n_allocated;j++)
			{
				testr=AVGC(i,j,0);
				testg=AVGC(i,j,1);
				testb=AVGC(i,j,2);
				testerr1=color_error(desired_colors[i][0],testr,
									desired_colors[i][1],testg,
									desired_colors[i][2],testb);

				if (testerr1<testerr)
				{
					testerr=testerr1;
					color_table2[i]=allocated_colors[j];
				}
			}
		}
	}

	XLateBitMap(mybitmap,mybitmap,color_table,color_table2);

	mywindow=OpenWindowTags(NULL,
				WA_SimpleRefresh,TRUE,WA_Title,argv[1],
				WA_CloseGadget,TRUE,WA_DepthGadget,TRUE,WA_DragBar,TRUE,
				WA_SizeGadget,TRUE,WA_MinWidth,20,WA_MinHeight,20,
				NULL,NULL);

	if (! mywindow) abort("Can't open window");

	ModifyIDCMP(mywindow,IDCMP_REFRESHWINDOW|IDCMP_CLOSEWINDOW);


	redraw_window();

	CloseIFF(pichandle); pichandle=NULL;

	while(! done)
	{
		Wait(1l<<(mywindow->UserPort->mp_SigBit));

		while(mymsg=(struct IntuiMessage *) GetMsg(mywindow->UserPort))
		{
			switch(mymsg->Class)
			{
					case REFRESHWINDOW:
						BeginRefresh(mywindow);
						redraw_window();
						EndRefresh(mywindow,-1);
						break;

					case CLOSEWINDOW:
						done=TRUE;
						break;
			}
			ReplyMsg(mymsg);
		}
	}
	for(i=0;i<n_allocated;i++)
		ReleasePen(wbcm,allocated_colors[i]);
	cleanup();
}
