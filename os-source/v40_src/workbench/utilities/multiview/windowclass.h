
#include <utility/tagitem.h>
#include <datatypes/datatypes.h>
#include <workbench/workbench.h>

/*****************************************************************************/

#define	WOA_Dummy		(TAG_USER + 1000)

#define	WOA_DrawInfo		(WOA_Dummy + 1)
#define	WOA_Window		(WOA_Dummy + 2)
#define	WOA_VSlider		(WOA_Dummy + 3)
#define	WOA_HSlider		(WOA_Dummy + 4)
#define	WOA_ObjectID		(WOA_Dummy + 5)
#define	WOA_UserData		(WOA_Dummy + 6)
#define	WOA_DisplayObj		(WOA_Dummy + 7)
#define	WOA_Title		(WOA_Dummy + 8)
#define	WOA_Sync		DTA_Sync	/* (WOA_Dummy + 9) */

/* Shared Intuition message port */
#define	WOA_IDCMPPort		(WOA_Dummy + 10)

/* AppWindow message port */
#define	WOA_AWPort		(WOA_Dummy + 11)

#define	WOA_Backdrop		(WOA_Dummy + 12)

#define	WOA_Flags		(WOA_Dummy + 13)

/* Slider attributes */
#define	WOA_TopVert		DTA_TopVert		/* (WOA_Dummy + 20) */
#define	WOA_VisibleVert		DTA_VisibleVert		/* (WOA_Dummy + 21) */
#define	WOA_TotalVert		DTA_TotalVert		/* (WOA_Dummy + 22) */
#define	WOA_IncVert		(WOA_Dummy + 23)
#define	WOA_DecVert		(WOA_Dummy + 24)

#define	WOA_TopHoriz		DTA_TopHoriz		/* (WOA_Dummy + 30) */
#define	WOA_VisibleHoriz	DTA_VisibleHoriz	/* (WOA_Dummy + 31) */
#define	WOA_TotalHoriz		DTA_TotalHoriz		/* (WOA_Dummy + 32) */
#define	WOA_IncHoriz		(WOA_Dummy + 33)
#define	WOA_DecHoriz		(WOA_Dummy + 34)

/*****************************************************************************/

#define	WOAF_BACKDROP	(1L<<0)
#define	WOAF_BITMAP	(1L<<1)

/*****************************************************************************/

#define	LEFT_GID	100
#define	UP_GID		101
#define	RIGHT_GID	102
#define	DOWN_GID	103
#define	HORIZ_GID	110
#define	VERT_GID	111

/*****************************************************************************/

#define	WOM_ADDVIEW	1000
#define	WOM_REMVIEW	1001

struct womView
{
    ULONG		 MethodID;
    Object		*wom_Object;
};
