#ifndef ILBM_H
#define ILBM_H

/*  :ts=8 bk=0
 *
 * myilbm.h:	Definitions for the ILBM reader.
 *
 * Leo L. Schwab			8906.02
 */
#ifndef IFF_IFFPARSE_H
#include <libraries/iffparse.h>
#endif

/*  Masking techniques  */
#define	mskNone			0
#define	mskHasMask		1
#define	mskHasTransparentColor	2
#define	mskLasso		3

/*  Compression techniques  */
#define	cmpNone			0
#define	cmpByteRun1		1

/*  Bitmap header (BMHD) structure  */
struct BitMapHeader {
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

/*  Color register structure (not really used)  */
struct ColorRegister {
	UBYTE red, green, blue;
};

/*  GRAB chunk structure  */
struct Point2D {
	WORD x, y;
};

/*  IFF types we may encounter  */
#define	ID_ILBM		MAKE_ID('I','L','B','M')
#define	ID_BMHD		MAKE_ID('B','M','H','D')
#define	ID_BODY		MAKE_ID('B','O','D','Y')
#define	ID_CMAP		MAKE_ID('C','M','A','P')
#define	ID_CRNG		MAKE_ID('C','R','N','G')
#define	ID_GRAB		MAKE_ID('G','R','A','B')
#define	ID_SPRT		MAKE_ID('S','P','R','T')
#define	ID_DEST		MAKE_ID('D','E','S','T')
#define	ID_CAMG		MAKE_ID('C','A','M','G')

#endif
