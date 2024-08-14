/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * windowclass.h
 *
 */

#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>

/*****************************************************************************/

#define	WOA_Dummy	(TAG_USER + 1000)

#define	WOA_DrawInfo	(WOA_Dummy + 1)
#define	WOA_Window	(WOA_Dummy + 2)
#define	WOA_VSlider	(WOA_Dummy + 3)
#define	WOA_HSlider	(WOA_Dummy + 4)
#define	WOA_ObjectID	(WOA_Dummy + 5)
#define	WOA_UserData	(WOA_Dummy + 6)
#define	WOA_DisplayObj	(WOA_Dummy + 7)
#define	WOA_Title	(WOA_Dummy + 8)
#define	WOA_Sync	(WOA_Dummy + 9)

/* Shared Intuition message port */
#define	WOA_IDCMPPort	(WOA_Dummy + 10)

/* AppWindow message port */
#define	WOA_AWPort	(WOA_Dummy + 11)

#define	WOA_Backdrop	(WOA_Dummy + 12)

/* Slider attributes */
#define	WOA_TopVert	DTA_TopVert
#define	WOA_VisVert	DTA_VisibleVert
#define	WOA_TotVert	DTA_TotalVert
#define	WOA_IncVert	(WOA_Dummy + 23)
#define	WOA_DecVert	(WOA_Dummy + 24)

#define	WOA_TopHoriz	DTA_TopHoriz
#define	WOA_VisHoriz	DTA_VisibleHoriz
#define	WOA_TotHoriz	DTA_TotalHoriz
#define	WOA_IncHoriz	(WOA_Dummy + 33)
#define	WOA_DecHoriz	(WOA_Dummy + 34)

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
    struct TagItem	*wom_Map;
};
