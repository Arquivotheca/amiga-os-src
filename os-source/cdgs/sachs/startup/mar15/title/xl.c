/* :ts=4
*
*	xl.c
*
*	William A. Ware							B605
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright (C) 1991, Silent Software Incorporated.						*
*   All Rights Reserved.													*
****************************************************************************/
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <libraries/dos.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>


#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>
#include <cdtv/cdtv.h>
#include <cdtv/playerprefsproto.h>

#include <ss/sslib.h>
#include <ss/talloc.h>

extern struct Custom __far	custom;


char Copyright[] = "\n\
[1m[33mFullXL V0.1 MC6800x0[0m - XL Full screen player\n\
Written By William A. Ware,  Designed by R. K. Von Wolfsheild\n\
Copyright © 1991, Silent Software, Incorporated.\n\n\
[1m*** This program may [4mNOT[0m[1m be freely distributed ***[0m\n\n";


char Usage[] = "\
[1mUSAGE:[0m	xlplay <file>\n\n\
	<file>	a .XL file from the \"[3mCDXLDEMO4:[0m\" disk.\n\n";


#define READ_PAD_BYTES		8

struct Panimage
{
	UBYTE	Type;
	UBYTE	SubType;
	ULONG	Frame;
	UWORD	XSize;
	UWORD	YSize;
	UWORD	PSize;
	UWORD	Colors;
	UWORD	AudioSize;
	UBYTE	DMACBugPad[ READ_PAD_BYTES ];
};

#define PAN_SIZE	(sizeof(struct Panimage))


struct DisplayFrame
{
	struct BitMap 		*Bm;
	struct View			*View;
	struct ViewPort		*Vp;
	UWORD				*ColorTable;
	struct Cel			*Cel;
	ULONG				Flags;
	UBYTE				*bufdata,*debuf;
	ULONG				bufsize;
	UWORD				*RGBloc[32];
	UWORD				*cmap;
};



struct XLLoad
{
	struct CDXL			xl;
	struct Panimage		pimg;
	UWORD				cm[16];
};

/* Data follows it. */


struct DisplayFrame *WorkF,*CurrentF;

struct GfxBase *GfxBase;
struct Library *PlayerPrefsBase;
struct Library *DeBoxBase;

struct LibInfo LibInit[] = {
	{ "graphics.library",	0L,(struct Library **)&GfxBase },
	{ "playerprefs.library",0L,(struct Library **)&PlayerPrefsBase },
	{ "debox.library",		0L,(struct Library **)&DeBoxBase },
	{ NULL, 0L, 0L }
};

/*
 * Disbale SASttice's miserable ^C checking.
 */
chkabort () { return (0); }
CXBRK () { return (0); }


static UWORD tftable_data[ 256 ] =
{
	0x0000,0x0003,0x000c,0x000f,0x0030,0x0033,0x003c,0x003f, 
	0x00c0,0x00c3,0x00cc,0x00cf,0x00f0,0x00f3,0x00fc,0x00ff, 
	0x0300,0x0303,0x030c,0x030f,0x0330,0x0333,0x033c,0x033f, 
	0x03c0,0x03c3,0x03cc,0x03cf,0x03f0,0x03f3,0x03fc,0x03ff, 
	0x0c00,0x0c03,0x0c0c,0x0c0f,0x0c30,0x0c33,0x0c3c,0x0c3f, 
	0x0cc0,0x0cc3,0x0ccc,0x0ccf,0x0cf0,0x0cf3,0x0cfc,0x0cff, 
	0x0f00,0x0f03,0x0f0c,0x0f0f,0x0f30,0x0f33,0x0f3c,0x0f3f,
	0x0fc0,0x0fc3,0x0fcc,0x0fcf,0x0ff0,0x0ff3,0x0ffc,0x0fff,
	0x3000,0x3003,0x300c,0x300f,0x3030,0x3033,0x303c,0x303f,
	0x30c0,0x30c3,0x30cc,0x30cf,0x30f0,0x30f3,0x30fc,0x30ff,
	0x3300,0x3303,0x330c,0x330f,0x3330,0x3333,0x333c,0x333f,
	0x33c0,0x33c3,0x33cc,0x33cf,0x33f0,0x33f3,0x33fc,0x33ff,
	0x3c00,0x3c03,0x3c0c,0x3c0f,0x3c30,0x3c33,0x3c3c,0x3c3f,
	0x3cc0,0x3cc3,0x3ccc,0x3ccf,0x3cf0,0x3cf3,0x3cfc,0x3cff,
	0x3f00,0x3f03,0x3f0c,0x3f0f,0x3f30,0x3f33,0x3f3c,0x3f3f,
	0x3fc0,0x3fc3,0x3fcc,0x3fcf,0x3ff0,0x3ff3,0x3ffc,0x3fff,
	0xc000,0xc003,0xc00c,0xc00f,0xc030,0xc033,0xc03c,0xc03f,
	0xc0c0,0xc0c3,0xc0cc,0xc0cf,0xc0f0,0xc0f3,0xc0fc,0xc0ff,
	0xc300,0xc303,0xc30c,0xc30f,0xc330,0xc333,0xc33c,0xc33f,
	0xc3c0,0xc3c3,0xc3cc,0xc3cf,0xc3f0,0xc3f3,0xc3fc,0xc3ff,
	0xcc00,0xcc03,0xcc0c,0xcc0f,0xcc30,0xcc33,0xcc3c,0xcc3f,
	0xccc0,0xccc3,0xcccc,0xcccf,0xccf0,0xccf3,0xccfc,0xccff,
	0xcf00,0xcf03,0xcf0c,0xcf0f,0xcf30,0xcf33,0xcf3c,0xcf3f,
	0xcfc0,0xcfc3,0xcfcc,0xcfcf,0xcff0,0xcff3,0xcffc,0xcfff,
	0xf000,0xf003,0xf00c,0xf00f,0xf030,0xf033,0xf03c,0xf03f,
	0xf0c0,0xf0c3,0xf0cc,0xf0cf,0xf0f0,0xf0f3,0xf0fc,0xf0ff,
	0xf300,0xf303,0xf30c,0xf30f,0xf330,0xf333,0xf33c,0xf33f,
	0xf3c0,0xf3c3,0xf3cc,0xf3cf,0xf3f0,0xf3f3,0xf3fc,0xf3ff,
	0xfc00,0xfc03,0xfc0c,0xfc0f,0xfc30,0xfc33,0xfc3c,0xfc3f,
	0xfcc0,0xfcc3,0xfccc,0xfccf,0xfcf0,0xfcf3,0xfcfc,0xfcff,
	0xff00,0xff03,0xff0c,0xff0f,0xff30,0xff33,0xff3c,0xff3f,
	0xffc0,0xffc3,0xffcc,0xffcf,0xfff0,0xfff3,0xfffc,0xffff
};
UWORD *tftable = tftable_data;



VOID FastTFold( UBYTE *data, struct BitMap *bm, int planesize )
{
	int i;
	register WORD  j;
	register UWORD *ptr;
	
	for(i=0;i<bm->Depth;i++)
	{
		for(j = planesize,ptr = (UWORD *)bm->Planes[i];
			j-->0;*ptr++ = tftable_data[ *data++ ]);
	}
}

VOID __asm DecompTFAST( register __a0 UBYTE *data, 
						register __a2 struct BitMap *);

VOID __asm DecompTFASTER( register __a0 UBYTE *data, 
						  register __a1 struct BitMap *,
						  register __d0 WORD  planesize);



/*********************************************************************/

struct CDXL		*NewXL( struct MinList *lst, void *buf, LONG len,
						VOID (*code)() )
{
	struct CDXL *x;
	
	if (!(x = TAllocMem( sizeof(struct CDXL), MEMF_PUBLIC | MEMF_CLEAR)))
		return(NULL);
	
	x->Buffer 	= (char *)buf;
	x->Length 	= len;
	x->DoneFunc = code;
	AddTail((struct List *)lst,(struct Node *)x);
	return(x);
}


struct List 		xlist;
struct List 		xready;
struct XLLoad 	*	xrem;
LONG				hits;
LONG				fc;

VOID __saveds framedone()
{
	register struct XLLoad	*xl;
	
	fc++;
	if (xrem)
	{			
		xl = (struct XLLoad *)RemHead( &xlist );
		AddTail( &xready, (struct Node *)xl );
	}
	xl = xrem = (struct XLLoad *)xlist.lh_Head;
	
	if (!xl->xl.Node.mln_Succ->mln_Succ)
	{
		hits++;
		AddTail(&xlist,RemHead( &xready ));
	}
}

struct XLLoad	*NewXLLoad( LONG len )
{
	struct XLLoad *xl;
	
	len += PAN_SIZE + 32;
	
	if (!(xl = TAllocMem( len+sizeof(struct CDXL), 
						(MEMF_PUBLIC | MEMF_CLEAR)))) return(NULL);
	
	xl->xl.Buffer 	= (char *)(&xl->pimg);
	xl->xl.Length 	= len;
	xl->xl.DoneFunc	= framedone;
	return(xl);
}

	
	

VOID LoopList( struct MinList *lst )
{
	struct MinNode *head = lst->mlh_Head;
	struct MinNode *tail = lst->mlh_TailPred;
	
	if (!head->mln_Succ) return;
	head->mln_Pred = tail;
	tail->mln_Succ = head;
}



FFInfo( char *name, ULONG *sector, ULONG *size, ULONG *nob  )
{
	BPTR 									lock;
	static struct FileInfoBlock __aligned	fib;
	int										ss = 0;

	if (sector) *sector = -1;
	if (size)   *size = -1;
	if (nob)	*nob = -1;

	if (lock = Lock(name,ACCESS_READ))
	{
		*sector = ((struct FileLock *)(lock << 2))->fl_Key;
		if (Examine( lock, &fib ))
		{
			*size = fib.fib_Size;
			*nob  = fib.fib_NumBlocks;
			ss = 1;
		}
		UnLock(lock);
	}
	return(ss);
}


/************************ Display frame *************************/

void FreeDisplayFrame( register struct DisplayFrame *f )
{
	if (f)
	{
		if (f->View)	DeleteView( (struct SuperView *)f->View );
		if (f->Bm) 		FreeBitMap( f->Bm );
		if (f->bufdata)	FreeMem(f->bufdata,f->bufsize);
		FreeMem( f, sizeof( struct DisplayFrame ));
	}
}

struct DisplayFrame *AllocDisplayFrame( int w,int h, int d, int m)
{
	register struct DisplayFrame *f;
	if (!(f = AllocMem( sizeof( struct DisplayFrame), MEMF_CLEAR )))
		return(NULL);
	
	if (!(f->Bm = AllocBitMap( d,w,h ))) 				goto mdf_err;
	if (!(f->View = CreateView( NULL,f->Bm,w,h*2,m)))	goto mdf_err;
	f->Vp = f->View->ViewPort;
	
	return(f);
mdf_err:
	FreeDisplayFrame( f );
	return(NULL);
}


VOID ToggleFrame() 
{
	register struct DisplayFrame *df;
	
	df = CurrentF;CurrentF = WorkF; WorkF = df;
}

/*********************************************************************/

struct UCopList		ucop,*uptr = NULL;

main(int argc,char **argv )
{
	struct 	Panimage	pimg;
	UWORD				colormap[64];
	char				*temp;
	int					planesize,i,readsize;
	struct View			*oldview = NULL;
	int					width,height;

	struct MinList		xferlist;
	ULONG				xsector,xsize,xblocks;
	struct DisplayFrame *tdf;
	struct IOStdReq		*cdio;
	struct XLLoad		*xl;
	WORD				offset = 0,skip;

	printf(Copyright);

	if (argc != 2) 
	{
		printf(Usage);
		exit(0);
	}
	
	if (temp = OpenLibs( LibInit ))
	{
		printf("Error: Cannot open file '%s' for reading.\n",temp);
		goto xit;
	}
	if (!FFInfo( argv[1],&xsector,&xsize,&xblocks ))
	{
		printf("Error: Unable to get file information\n");
		goto xit;
	}
	/*
	printf("File Info -- sector: %ld   size: %ld   blocks: %ld\n",
			xsector,xsize,xblocks);
	*/

	if (!(cdio = AllocIORequest("cdtv.device",0,0, sizeof (struct IOStdReq))))
	{
		printf("Error: Cannot open CDTV Device");
		goto xit;
	}
	NewList( (struct List *)&xferlist );
	NewXL( &xferlist, &pimg, PAN_SIZE, NULL );


	cdio->io_Command = CD_READXL;
	cdio->io_Offset  = xsector;
	cdio->io_Length  = 2;
	cdio->io_Data    = (APTR)&xferlist;
	if (SafeDoIO( cdio )) 
	{
		printf("Error: Initial xl read failed.\n");
		goto xit;
	}

	/* 
	 * Try to Stablize the cd 
	 * ( A seek to posision zero seems to do the trick )
	 */
	cdio->io_Command = CD_SEEK;
	cdio->io_Offset  = 0;
	cdio->io_Length  = 0;
	cdio->io_Data    = 0;
	SafeDoIO( cdio );

	width = pimg.XSize * 2;
	height = pimg.YSize;
	planesize = RASSIZE(pimg.XSize,pimg.YSize);
	readsize = planesize * pimg.PSize;

	printf("[1mInformation:[0m\n");
	printf("\n	Width: %d   Height: %d   Depth: %d\n",
			pimg.XSize,pimg.YSize,pimg.PSize) ;

	skip = 0;
	if (width > 352) 
	{
		skip = ((((width+15)>>3)&0xfffe)-(352>>3))/2;
		width = 352;
	}
	printf("	Expanding to %d by %d.\n\n",width,height );

	WorkF = AllocDisplayFrame(width,height,pimg.PSize,HAM);
	CurrentF = AllocDisplayFrame(width,height,pimg.PSize,HAM);

	CenterCDTVView( NULL,CurrentF->View,width,height * 2);
	CenterCDTVView( NULL,WorkF->View,width,height * 2);
	if (CurrentF->Vp->DxOffset < -16)   
		WorkF->Vp->DxOffset = CurrentF->Vp->DxOffset = -16;
	
	MakeVPort( CurrentF->View,CurrentF->Vp );
	MrgCop( CurrentF->View );
	MakeVPort( WorkF->View,WorkF->Vp );
	MrgCop( WorkF->View );

	for(i=0;i<CurrentF->View->LOFCprList->MaxCount;i++)
	{
		if (CurrentF->View->LOFCprList->start[i*2] == 0x108)
		{
			offset = CurrentF->View->LOFCprList->start[(i*2)+1];
			break;
		}
	}

	CINIT( &ucop, (height*3+40) );
	for(i=0;i<CurrentF->Vp->DHeight;i+=2)
	{
		CWAIT( &ucop, i,0 );
		CMOVE( &ucop, custom.bpl1mod, (offset-CurrentF->Bm->BytesPerRow) );
		CMOVE( &ucop, custom.bpl2mod, (offset-CurrentF->Bm->BytesPerRow) );
		CWAIT( &ucop, (i+1),0);
		CMOVE( &ucop, custom.bpl1mod, (offset) );
		CMOVE( &ucop, custom.bpl2mod, (offset) );
	}
	CEND ( &ucop );
	uptr = &ucop;
	
	
	CurrentF->Vp->UCopIns = WorkF->Vp->UCopIns = &ucop;
	MakeVPort( CurrentF->View,CurrentF->Vp );
	MrgCop( CurrentF->View );
	MakeVPort( WorkF->View,WorkF->Vp );
	MrgCop( WorkF->View );


	FindViewRGB( CurrentF->View,CurrentF->RGBloc, 32 );
	FindViewRGB( WorkF->View,WorkF->RGBloc, 32 );


	oldview = GfxBase -> ActiView;	

	NewList( &xlist );
	NewList( &xready );

	for(i=0;i<50;i++)
	{
		if (!(xl = NewXLLoad( readsize ))) break;
		AddHead( &xlist, (struct Node *)xl );
	}

	/* Now lest try an xl test */
	cdio->io_Command = CD_READXL;
	cdio->io_Offset  = xsector;
	cdio->io_Length  = xblocks;
	cdio->io_Data    = (APTR)&xlist;

	/* 
	 * Every Thing set up - Hold your breath and here we go. 
	 */
	SendIO(cdio);
	
	while( 1 )
	{
		if (xready.lh_Head->ln_Succ->ln_Succ )
		{
			Disable();
			xl = (struct XLLoad *)RemHead( &xready );
			Enable();
			DecompTFASTER( (UBYTE *)&xl[1],WorkF->Bm,skip );
			LoadFoundRGB( WorkF->RGBloc,xl->cm,16 );
			ToggleFrame();
			LoadView(CurrentF->View);
			AddTail( &xlist, (struct Node *)xl);
		}
		else
		{
			if (CheckIO( cdio )) break;
		}
	}
					
	AbortIO( cdio );
	SafeWaitIO( cdio );

	printf("	Total Frames in animation: %4d\n",fc);
	if (hits) printf("	Skipped:                   %4d\n",hits);

	printf("\n");
xit:
	if (cdio)		FreeIORequest(cdio);
	if (oldview)	LoadView(oldview);
	WaitTOF();
	
	if (uptr) FreeCopList (  ucop.FirstCopList );
	if (CurrentF)	
	{
		CurrentF->Vp->UCopIns = NULL;
		FreeDisplayFrame(CurrentF);
	}
	if (WorkF)		
	{
		WorkF->Vp->UCopIns = NULL;
		FreeDisplayFrame(WorkF);
	}
	FreeTAllocALL();
	CloseLibs( LibInit );
	exit(0);
}