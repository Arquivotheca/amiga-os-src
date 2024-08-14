/**************

    flik.h

  FLIK Full Motion Video include file

  W.D.L 930612

***************/


/* Four-character IDentifier builder.*/
#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#define FLIK MakeID('F','L','I','K')


// W.D.L 920723. Need to add imagesize and numcolors, and maybe Extdata size

typedef struct flikhead
{
    ULONG   type;		// FLIK
    ULONG   flags;
    ULONG   modeID;
    ULONG   numframes;		// number of frames in flik
    ULONG   headsize;		// flikhead size including colortable
    ULONG   framesize;		// the uncompressed flikframe size
    UWORD   audiosize;
    UWORD   colortablesize;
    UWORD   width;
    UWORD   height;
    UBYTE   depth;
    UBYTE   compression;	// probably not usable for our purposes
    UWORD   playspeed;		// sectors (2048) per second or
				// (KBytes/2 per second)
    ULONG   reserved1;
    ULONG   reserved2;
//  ULONG * colortable;		// LoadRGD32() ready table

} FLIKHEAD;

// W.D.L 920723. Maybe and Extdata after framecolortable

typedef struct flikframe
{
    ULONG   framenum;
    UWORD   xpos;
    UWORD   ypos;

//  UWORD * video;
//  UWORD * audio;
//  UWORD * framecolortable; // Non NULL only if flikhead flags & FLIK_MULTI_PAL

} FLIKFRAME;


// flikhead flags values
#define	FLIK_MULTI_PAL		0x00000001
#define	FLIK_HEAD_RGB32		0x00000002 /* ctable in FLIKHEAD is in a format 
					    * that can be directly fed to LoadRGB32().
					    * Else it is in a 3 byte RGB form that must
					    * be concerted into LoadRGB32() Format.
					    */
#define	FLIK_FRAME_RGB32	0x00000004 // Same as above for FLIKFRAMEs



// flikhead compression values
#define FLIK_NO_COMP		0
#define FLIK_RUNLENGTH_COMP	1


/*
 * A FLIK file consists of a FLIKHEAD followed by FLIKHEAD->numframes of
 * FLIKFRAME.
 */
