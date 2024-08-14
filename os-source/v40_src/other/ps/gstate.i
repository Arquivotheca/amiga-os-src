/* The main gstate structure for holding the current graphics state */

typedef struct __gstate {
	struct __gstate *next;		// next in list for gsave/grestore
	float	CTM[6];				// current transformation matrix
	float	ICTM[6];			// inverse of current transformation matrix
	float	color[3];			// current RGB color
	float	CurrentPoint[2];	// current position in user space
	float	LineWidth;
	float	MiterLimit;
	float	Flatness;
	float	Greyness;
	char *	Dash;				// ptr to dashInfo & Dash
	uchar	LineCap;
	uchar	LineJoin;
	uchar	ICTMValid;			// flag to validate ICTM
	uchar	__padding;
} gstate;
