#ifndef pcx_h
#define pcx_h

/*
 * defines for the PCX header and magic numbers
 */

typedef struct {						/*header for PCX bitmap files*/
    unsigned char	signature;			/*PCX file identifier*/
    unsigned char	version;			/*version compatibility level*/
    unsigned char	encoding;			/*encoding method*/
    unsigned char	bitsperpix;			/*bits per pixel, or depth*/
    unsigned short	Xleft;				/*X position of left edge*/
    unsigned short	Ytop;				/*Y position of top edge*/
    unsigned short	Xright;				/*X position of right edge*/
    unsigned short	Ybottom;			/*Y position of bottom edge*/
    unsigned short	Xscreensize;		/*X screen res of source image*/
    unsigned short	Yscreensize;		/*Y screen res of source image*/
    unsigned char	PCXpalette[16][3];	/*PCX color map*/
    unsigned char	reserved1;			/*should be 0, 1 if std res fax*/
    unsigned char	planes;				/*bit planes in image*/
    unsigned short	linesize;			/*byte delta between scanlines */
    unsigned short	paletteinfo;		/*0 == undef
										  1 == color
										  2 == grayscale*/
    unsigned char reserved2[58];		/*fill to struct size of 128*/
} PCX_HEADER;

#define PCX_HEADERSIZE		128

#define PCX_LAST_VER		5			/* last acceptable version number */
#define PCX_RLL				1			/* PCX RLL encoding method */

#define PCX_MAGIC			0x0a		/* first byte of a PCX image */
#define PCX_256COLORS_MAGIC	0x0c		/* first byte of a PCX 256 color map */

#endif /* pcx_h */
