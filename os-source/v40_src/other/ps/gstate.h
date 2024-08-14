//
// Amiga Postscript (C) Commodore-Amiga 1991
// GState and Path structures definitions and related info.
//
// The structure of paths is really simple:  PathBase and PrivatePath
// simply point to an array of MAXPATH chars with MAXPATH co-ordinate
// pairs appended.  Path memory usage is MAXPATH+MAXPATH*8 bytes.
//
// PathArgs is an index (and effectively constant) to the start of
// the co-ordinate storage area.  Given an index to a path segment,
// the argument pointer can be calculated as PathBase+PathArgs+index<<3
//
// Example, a path with MAXPATH set to 4 would look like this in memory;
//
//    chars up to here | floats from now on
//					   V
//	seg1,seg2,seg3,seg4,x1,y1,x2,y2,x3,y3,x4,y4
//  ^				   ^
//  |PathBase		   |PathArgs
//
//

#ifndef PATH_H

#define PATH_H 1

// segment types stored in the first part of the path, arguments are stored
// in the float area starting at offset PathArgs
#define	SEG_MOVETO	 0
#define	SEG_LINETO	 1
#define	SEG_CURVETO	 2
#define	SEG_CLOSE	 3
#define SEG_CTRL_PT1 2
#define SEG_CTRL_PT2 4
#define SEG_CTRL_PT3 5

// as defined in Red Book implementation limits
#define	MAX_DASH 11

// maximum size of path (number of segments)
#define MAXPATH 1500

/********************************************************************/
/* The main GSTATE structure for holding the current graphics state */
/********************************************************************/

typedef struct __gstate {
	struct __gstate *next;	// next in list for gsave linking

	float	CTM[6];			// current transformation matrix
	float	ICTM[6];		// inverse of current transformation matrix

	float	color[3];		// current RGB color (HSB is derived from RGB)

	float	CurrentPoint[2];	// current position in device space

	float	LineWidth;		// path stroking attributes
	float	MiterLimit;
	float	Flatness;		// 0.2...100
	float	Greyness;

	float	ScreenFreq;		// halftone screen frequency
	float	ScreenAngle;	// halftone screen angle

	ps_obj	SpotFunc;		// halftone screen spot function
	ps_obj	CurrentFont;	// dictionary describing current font for 'show'
	ps_obj	FontMatrix;		// copy of current font's FontMatrix
	ps_obj	Encoding;		// copy of   ''		''	  Encoding array
	ps_obj	CharStrings;	// copy of   ''		''	  CharStrings dict
	ps_obj	Metrics;		// OPTIONAL copy of the Metrics dict in a font

	int		FontType;		// user or built-in font (copy of dict entry)
	float	CharWidthX;		// values passed by setcharwidth
	float	CharWidthY;
	BOOL	goodfont;		// setfont's "invalidfont" flag to show...

	ps_obj	GrayTransfer;	// user->device gray mapping function

	float	DashOffset;		// start dash offset
	ps_obj	DashArray;		// handle to array of nums

// Current Path variables...

	uchar	*PathBase;		// ptr to array of path segment records
	uchar	*PrivatePath;	// Used by stroke, strokepath and reversepath
	uchar	*ClipPathBase;	// pointer to current clippath

	short	PathStart;		// index to where the path currently starts
	short	PathNew;		// where to start path after newpath executed
	short	PathIndex;		// index to next free segment
	short	SubPath;		// index to start of current subpath
	short	PathArgs;		// maximum segments in path (index to x,y float pairs)

	short	PrivPathIndex;	// current offset in private path
	short	PrivSubPath;	// start of current subpath
	short	ClipPathIndex;	// current index into clippath

// start of GState flags
	uchar	inBuildChar;	// are we running inside a BuildChar proc ?
	uchar	SCD;			// SetCacheDevice called but not yet BuildChar
	uchar	cp_valid;		// CurrentPoint is valid
	uchar	path_open;		// done a closepath recently ?

	uchar	charpathed;		// path contains a character def ?
	uchar	LineCap;		// linecap style
	uchar	LineJoin;		// linejoin style
	uchar	ICTMValid;		// flag, inverse of CTM valid

	uchar	PathIsCurved;	// path has curves in (0=no)
	uchar	NoOutput;	// disable stroke,fill,eofill, etc.. (for stringwidth)
	uchar	chartype;		// same as PaintType font entry
	uchar	let_grow;		// charpath/show communication
} gstate;

//-------------------------------------------------------------------------
// Font Cache related info follows.

typedef struct tdef_Cache {
	ulong	CacheUpper;			// setcacheparams upper cache size limit
	ulong	CacheLower;			// setcacheparams lower cache size limit
	ulong	NumCached;			// number of characters in the cache
	ulong	MaxCached;			// maximum possible number of ...
	
} Cache;

#endif
