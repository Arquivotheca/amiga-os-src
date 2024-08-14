#include <utility/tagitem.h>

/*****************************************************************************/

#define	WOA_Dummy	(TAG_USER + 1000)

#define	WOA_IDCMP	(WOA_Dummy + 1)
#define	WOA_IDCMPPort	(WOA_Dummy + 2)
#define	WOA_AWPort	(WOA_Dummy + 3)
#define	WOA_Title	(WOA_Dummy + 4)
#define	WOA_UserData	(WOA_Dummy + 5)
#define	WOA_Window	(WOA_Dummy + 6)

/* Slider attributes */
#define	WOA_TopVert	(WOA_Dummy + 20)
#define	WOA_VisVert	(WOA_Dummy + 21)
#define	WOA_TotVert	(WOA_Dummy + 22)
#define	WOA_IncVert	(WOA_Dummy + 23)
#define	WOA_DecVert	(WOA_Dummy + 24)
#define	WOA_VertInc	(WOA_Dummy + 25)
    /* LONG: Amount to adjust when receive WOA_IncVert or WOA_DecVert */

#define	WOA_TopHoriz	(WOA_Dummy + 30)
#define	WOA_VisHoriz	(WOA_Dummy + 31)
#define	WOA_TotHoriz	(WOA_Dummy + 32)
#define	WOA_IncHoriz	(WOA_Dummy + 33)
#define	WOA_DecHoriz	(WOA_Dummy + 34)
#define	WOA_HorizInc	(WOA_Dummy + 35)
    /* LONG: Amount to adjust when receive WOA_IncHoriz or WOA_DecHoriz */

/*****************************************************************************/

#define	LEFT_GID	100
#define	UP_GID		101
#define	RIGHT_GID	102
#define	DOWN_GID	103
#define	HORIZ_GID	110
#define	VERT_GID	111

/*****************************************************************************/

#define	HORIZ_SLIDER	0
#define	VERT_SLIDER	1
#define	NUM_SLIDERS	2

#define	LEFT_ARROW	0
#define	UP_ARROW	1
#define	RIGHT_ARROW	2
#define	DOWN_ARROW	3
#define	NUM_ARROWS	4
