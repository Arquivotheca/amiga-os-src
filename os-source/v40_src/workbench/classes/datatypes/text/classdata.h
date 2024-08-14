/* classdata.h
 *
 */

/*****************************************************************************/

#define	DBS_DOWN	0
#define	DBS_MOVE	1
#define	DBS_UP		2
#define	DBS_RENDER	3
#define	DBS_SCROLL1	4
#define	DBS_SCROLL2	5

/*****************************************************************************/

struct localData
{
    struct TextAttr	*lod_TextAttr;
    struct TextFont	*lod_Font;
    STRPTR		 lod_Buffer;
    ULONG		 lod_BufferLen;
    struct MinList	 lod_LineList;
    ULONG		 lod_Flags;

    /* Word selection */
    UBYTE		*lod_WordDelim;			/* Word deliminators */
    LONG		 lod_SSec, lod_SMic;		/* Previous stamp */
    WORD		 lod_WX, lod_WY;		/* Current X, Y */
    WORD		 lod_OX, lod_OY;		/* Previous X, Y */
    ULONG		 lod_SWord;			/* Previous word */
    ULONG		 lod_Word;			/* Pointer to the word */
    UBYTE		 lod_WordBuffer[128];		/* Buffer containing the word that was selected */
    ULONG		 lod_AWord;			/* Anchor buffer pointer */
    ULONG		 lod_EWord;			/* End buffer pointer */
    struct IBox		 lod_Select;			/* Current select box */

    /* Selection */
    LONG		 lod_OTopVert;
    LONG		 lod_OTopHoriz;
    ULONG		*lod_Selected;

    /* Temporary Display Information */
    struct RastPort	 lod_Render;
    struct RastPort	 lod_Highlight;
    struct RastPort	 lod_Clear;
    struct TmpRas	 lod_TmpRas;
    PLANEPTR		 lod_TmpPlane;
    UWORD		 lod_TmpWidth, lod_TmpHeight;
    struct Region	*lod_Region;

    /* Caching information */
    struct IBox		 lod_Domain;
    ULONG		 lod_UsefulHeight;
};

#define	LODF_REDRAW	(1L<<0)
#define	LODF_CHANGED	(1L<<1)
#define	LODF_HIGHLIGHT	(1L<<2)
#define	LODF_WORDSELECT	(1L<<3)
#define	LODF_ALEFT	(1L<<4)
#define	LODF_ARIGHT	(1L<<5)
#define	LODF_TOGGLE	(1L<<6)

#define	LODF_WORDWRAP	(1L<<7)

#define	LODF_LAYOUT	(1L<<8)
    /* We need to layout! */
