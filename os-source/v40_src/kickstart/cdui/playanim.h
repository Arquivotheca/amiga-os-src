struct BitMapHeader 
{
	UWORD	w, h;		/*  Width, height in pixels */
	WORD	x, y;		/*  x, y position for this bitmap  */
	UBYTE	nplanes;	/*  # of planes  */
	UBYTE	Masking;
	UBYTE	Compression;
	UBYTE	pad1;
	UWORD	TransparentColor;
	UBYTE	XAspect, YAspect;
	WORD	PageWidth, PageHeight;
};

#define SECTORSIZE 128
#define DATASIZE ((sizeof(struct Frame1Head)) + (ncolours * 3))
#define BYTES_TO_SECTORS(x) (((x) + DATASIZE + (SECTORSIZE - 1)) / SECTORSIZE)
#define BYTES_TO_WHOLE_SECTORS(x) (BYTES_TO_SECTORS(x) * SECTORSIZE)
#define WHOLE_SECTORS(x) (((x) + (SECTORSIZE - 1)) / SECTORSIZE)

/* Frame1 is the format of the 1st frame of the sequence.
 * This cannot be defined in C, because the structure is of a variable length.
 *
 *
 * struct Frame1Head
 * {
 *	ULONG f1_MaxSectorSize;	 max number of sectors in a frame
 * 	ULONG f1_ByteSize;	 number of bytes in this block
 *	ULONG f1_Sectors;	 number of sectors in this block
 *	UWORD f1_ImageWidth;	 number of pixels accross the image
 *	UWORD f1_ImageHeight;	 number of rows in the image
 *	UWORD f1_ImageDepth;	 depth of the image
 *	UWORD f1_ScreenWidth;	 number of pixels accross the screen
 *	UWORD f1_ScreenHeight;
 *	UWORD f1_ScreenDepth;
 *	ULONG f1_ScreenModeID;
 *	UWORD f1_Colours;	 number of colours in the CMAP
 *	UBYTE f1_CMAP[];	 full CMAP structure.
 *	UBYTE f1_ILBM;		 ILBM of frame 1
 *	ULONG f1_NextByteSize;	 number of bytes in the next frame
 *	ULONG f1_NextSectors;	 number of sectors in the next frame
 *	UBYTE f1_pads[];	 pad to the next whole sector
 * };
 */

/* Frame is the format of the other frames in the sequence.
 *
 * struct Frame
 * {
 *	UBYTE f1_DLTA[];	 DLTA of this frame
 *	ULONG f1_NextByteSize;	 number of bytes in the next frame
 *	ULONG f1_NextSectors;	 number of sectors in the next frame
 * };
 */


struct Frame1Head
{
    ULONG f1_MaxSectorSize;
    ULONG f1_ByteSize;
    ULONG f1_Sectors;
    UWORD f1_ImageWidth;	/* number of pixels across the image */
    UWORD f1_ImageHeight;	/* number of rows in the image */
    UWORD f1_ImageDepth;	/* depth of the image */
    UWORD f1_ScreenWidth;	/* number of pixels across the screen */
    UWORD f1_ScreenHeight;
    UWORD f1_ScreenDepth;
    ULONG f1_ScreenModeID;
    UWORD f1_Colours;
};

/* redefine these macros from the playraw.c code. */
#define OPEN(f, m, b) (f)
#define CLOSEFILE(f) 
#define READ(f, b, s) {(b) = (f); (f) += (s);}
typedef UBYTE * FILETYPE;
