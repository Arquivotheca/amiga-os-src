/* classdata.h
 *
 */

/*****************************************************************************/

#define	G(o)		((struct Gadget *)(o))

/*****************************************************************************/

struct localData
{
    ULONG			 lod_ModeID;		/* Display mode id */
    struct BitMapHeader		 lod_BMHD;		/* BitMap header */
    Point			 lod_Point;		/* Grab spot */
    struct BitMap		*lod_OurBMap;		/* Our source Bitmap that we free */
    struct BitMap		*lod_SourceBMap;	/* Source Bitmap */
    struct BitMap		*lod_BMap;		/* Bitmap */
    struct BitMap		*lod_ZoomBMap;		/* Zoom bitmap */
    struct Screen		*lod_Screen;		/* Screen to map to */

    /* Color & Remap information */
    struct ColorRegister	*lod_Colors;
    LONG			*lod_CRegs;
    LONG			*lod_GRegs;
    UBYTE			*lod_ColorTable;
    UBYTE			*lod_ColorTable2;
    UBYTE			*lod_Allocated;
    ULONG			*lod_Histogram;		/* Histogram */
    ULONG			*lod_Histogram2;	/* Sorted histogram */
    WORD			 lod_NumColors;
    WORD			 lod_NumAlloc;		/* Number of colors allocated */
    ULONG			 lod_HistoSize;		/* Size of histogram */
    struct ColorMap		*lod_ColorMap;		/* ColorMap that we attached to */
    ULONG			 lod_Flags;
    ULONG			 lod_Precision;		/* Remap precision */

    /* Sparse color information */
    UWORD			 lod_NumSparse;
    UBYTE			*lod_SparseTable;
    struct ColorMap		*lod_SparseCM;

    /* Scale information */
    LONG			 lod_OTopVert;
    LONG			 lod_OTopHoriz;
    LONG			 lod_ZoomWidth;
    LONG			 lod_ZoomHeight;
    WORD			 lod_AX, lod_AY;	/* Anchor point */
};

/*****************************************************************************/

#define	LODF_REMAP	(1L<<0)
#define	LODF_FREESRC	(1L<<1)
#define	LODF_SPARSE	(1L<<2)
#define	LODF_SAMEBM	(1L<<3)

/*****************************************************************************/
