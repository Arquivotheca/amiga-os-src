/*************

 cdxlob.h

 W.D.L 930330

**************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

#ifndef	CDXLOB_H	// [
#define	CDXLOB_H


#define	BITMAP	struct BitMap
#define	RECT	struct Rectangle
#define	BUF_NUM	2

typedef int __asm (*PF)( register __a1 APTR, register __a2 struct CDXL *);
typedef int (*PFI)( struct MinList * first, UBYTE * buf, ULONG len, PF code );

//typedef int __asm (*PF)( APTR, struct CDXL *);
//typedef int (*PFI)( struct MinList * first, UBYTE * buf, ULONG len, PF code );

// CDXL control object
typedef struct cdxl_ob
{
	UBYTE	* Buffer[BUF_NUM];
	UBYTE	* Video[BUF_NUM];
	UWORD	* CMap[BUF_NUM];
	UBYTE	* audio[BUF_NUM];
	ULONG 	  StartSector;
	ULONG	  NumSectors;
	ULONG	  NumFrames;
	ULONG	  flags;
	ULONG	  PlaneSize;
	ULONG	  ImageSize;
	ULONG	  AudioSize;
	ULONG	  FrameSize;
	ULONG	  BufSize;
	ULONG	  CMapSize;
	ULONG	  FibSize;
	ULONG	  KillSig;
	ULONG	  ReadXLSpeed;
	RECT	  rect;
	BITMAP    bm[BUF_NUM];
	SHORT	  xoff,yoff;
	BYTE	  Volume;
	BYTE	  curAudio;
	BYTE	  curVideo;
	BYTE	  loops;
} CDXLOB;

//CDXLDOB Flags
#define	CDXL_BLIT		0x00000001
#define	CDXL_MULTI_PALETTE	0x00000002
#define	CDTV_DEVICE		0x00000004
#define	CDXL_XLEEC		0x00000008
#define	CDXL_DOSXL		0x00000010



// Display definition and control structure
typedef	struct DisplayDefinition
{
	SHORT			  Left;
	SHORT			  Top;
	SHORT			  Width;
	SHORT			  Height;
	SHORT			  Depth;
	SHORT			  NominalWidth;
	SHORT			  NominalHeight;
	ULONG			  Flags;
	ULONG			  ModeID;
	struct BitMap	* bm[2];
	struct ViewPort	* vp;
	struct DBufInfo * dbuf;

} DISP_DEF;

//DISP_DEF Flags
#define		DISP_OVERSCAN		0x00000003
#define		DISP_OVERSCANX		0x00000001
#define		DISP_OVERSCANY		0x00000002
#define		DISP_SCREEN		0x10000000
#define		DISP_INTERLEAVED	0x20000000
#define		DISP_ALLOCBM		0x40000000
#define		DISP_BACKGROUND		0x80000000
#define		DISP_XLPALETTE		0x01000000
#define		DISP_NOPOINTER		0x02000000
#define		DISP_XLMODEID		0x04000000



// Use this when using cdtv.device instead of struct CDXL for cd.device
typedef struct cdtv_cdxl
{
	struct	MinNode Node;	/* double linkage	*/
	char	*Buffer;	/* data (word aligned)	*/
	long	Length;		/* must be even # bytes	*/
	void	(*DoneFunc)();	/* called when done	*/
	long	Actual;		/* bytes transferred	*/

} CDTV_CDXL;


//----
#define SCREEN_WIDTH		640
#define NTSC_HEIGHT		400
#define PAL_HEIGHT		512

//----
#define	SYSTEM_PAL		(GfxBase->DisplayFlags & REALLY_PAL)
#define	CD_NAME			"cd.device"
#define	CDTV_NAME		"cdtv.device"
#define	CDTV_SEEK		10
#define	CDTV_READXL		42
#define	DEFAULT_XLSPEED	75
#define	DEFAULT_SECTOR_SIZE	2048
#define	DATA_TRANS_RATE		((CDXL_ob->flags & CDTV_DEVICE) ? (DEFAULT_XLSPEED * DEFAULT_SECTOR_SIZE) : (cdinfo.ReadXLSpeed * cdinfo.SectorSize) )


// Audio has different periods on NTSC vs PAL
#define		NTSC_FREQ		3579545
#define		PAL_FREQ		3546895

#define		INTDIV( a, b )		( ( (a) + ( (b) / 2 ) ) / (b) )
#define	MAX_CMAP	256

IMPORT	struct	CDInfo cdinfo;

#endif				// ]
