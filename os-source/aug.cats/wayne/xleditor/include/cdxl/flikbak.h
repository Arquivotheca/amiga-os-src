/**************

    flik.h

  FLIK Full Motion Video include file

  W.D.L 930612

***************/


/* Four-character IDentifier builder.*/
#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )

#define FLIK MakeID('F','L','I','K')


typedef struct flikhead
{
    ULONG   type;		// FLIK
    ULONG   flags;
    ULONG   modeID;
    ULONG   numframes;		// number of frames in flik
    ULONG   headsize;		// flikhead size including colortable
    ULONG   framesize;		// the uncompressed flikframe size
    ULONG   maxcompsize;	// largest compressed flikframe size (only if compression != 0)
    ULONG   firstcompsize;	// first compressed flikframe size (only if compression != 0)
    UWORD   audiosize;
    UWORD   colortablesize;
    UWORD   width;
    UWORD   height;
    UBYTE   depth;
    UBYTE   compression;
    UWORD   playspeed;		// sectors (2048) per second or (KBytes/2 per second)
    ULONG   reserved1;
    ULONG   reserved2;
    ULONG   reserved3;
    ULONG   reserved4;
//  ULONG * colortable;		// LoadRGD32() ready table

} FLIKHEAD;


typedef struct flikframe
{
    ULONG   framenum;
    ULONG   nextcompsize;	// next compressed flikframe size (ONLY if flikhead->compression != 0)

//  UWORD * video;
//  UWORD * audio;
//  UWORD * framecolortable; // Non NULL only if flikhead flags & FLIK_MULTI_PAL

} FLIKFRAME;



// flikhead compression values
#define FLIK_NO_COMP		0
#define FLIK_RUNLENGTH_COMP	1

// flikhead flags values
#define	FLIK_MULTI_PAL	0x00000001








