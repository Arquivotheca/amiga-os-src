/******************

     disp_def.h

 Created    W.D.L 930507

*******************/

#include	<iffp/ilbm.h>

//----
#define SCREEN_WIDTH		640
#define NTSC_HEIGHT		400
#define PAL_HEIGHT		512

#define	MAX_CRNG		8

// Display definition and control structure
typedef	struct DisplayDefinition
{
	SHORT		  Left;
	SHORT		  Top;
	SHORT		  Width;
	SHORT		  Height;
	SHORT		  Depth;
	SHORT		  NominalWidth;
	SHORT		  NominalHeight;
	ULONG		  Flags;
	ULONG		  ModeID;
	struct BitMap	* bm[2];
	struct ViewPort	* vp;
	struct DBufInfo * dbuf;
	struct Screen	* screen;
	struct Window	* window;
	WORD		* colorrecord;	/* Passed to LoadRGB32 (ncolors,firstreg,table) */
	ULONG		  crecsize;	/* Bytes allocated including extra WORDs        */
	CRange		  crng[MAX_CRNG];
	SHORT		  cyclecount[MAX_CRNG];

} DISP_DEF;


//DISP_DEF Flags
#define		DISP_OVERSCAN		0x00000003
#define		DISP_OVERSCANX		0x00000001
#define		DISP_OVERSCANY		0x00000002
#define		DISP_CENTER		0x00000030
#define		DISP_CENTERX		0x00000010
#define		DISP_CENTERY		0x00000020
#define		DISP_SCREEN		0x10000000
#define		DISP_INTERLEAVED	0x20000000
#define		DISP_ALLOCBM		0x40000000
#define		DISP_BACKGROUND		0x80000000
#define		DISP_XLPALETTE		0x01000000
#define		DISP_NOPOINTER		0x02000000
#define		DISP_XLMODEID		0x04000000
#define		DISP_COP_INT		0x00100000
#define		DISP_OPEN		0x00200000
#define		DISP_DBUF		0x00010000
