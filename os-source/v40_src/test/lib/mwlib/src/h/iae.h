/*	IAE.h - structs for IAE.c
*
*	Mitchell/Ware		Version 1.01		8/31/87
*
*/

typedef	struct iae
{
	long	*Array;			/* the actual array		*/
	short	DimX, DimY;		/* x & y dimensions of the array */
	short	Width, Height;		/* Max &  initial dimensions	*/
	short	Digits;			/* Digits per number displayed 	*/
	struct	Screen	*Screen;	/* if customscreen, points to it */
	UBYTE	DetailPen, BlockPen;	/* pens	*/
	UBYTE	*Title;			/* If null, uses default title	*/
	USHORT	Command;		/* return command/status	*/
	USHORT	Initial;		/* Initial setup flags		*/
	void	(*cal)();		/* A recalculation routine	*/

	/* The rest are used internally by IAE	*/
	short	LeftX, TopY;		/* displayed portion of array	*/
	short	DispWidth, DispHeight;	/* Count of elements displayable */
	short	CurX, CurY;		/* Current pos. in array being edited */
	short	CWidth, CHeight;	/* Character width and height	*/
	short	Gx, Gy;			/* Calculated pixel position */
} IAE;
