/*
 * $Id: oldwbinternal.h,v 38.1 91/06/24 19:02:09 mks Exp $
 *
 * $Log:	oldwbinternal.h,v $
 * Revision 38.1  91/06/24  19:02:09  mks
 * Changed to V38 source tree - Trimmed Log
 * 
 */

#ifndef OLDWBINTERNAL_H
#define OLDWBINTERNAL_H

struct OldNewDD { /* ram drawer data */
    struct NewWindow	dd_NewWindow;	/* args to open window */
    LONG		dd_CurrentX;	/* current x coordinate of origin */
    LONG		dd_CurrentY;	/* current y coordinate of origin */
    LONG		dd_MinX;	/* smallest x coordinate in window */
    LONG		dd_MinY;	/* smallest y coordinate in window */
    LONG		dd_MaxX;	/* largest x coordinate in window */
    LONG		dd_MaxY;	/* largest y coordinate in window */
    struct Gadget	dd_HorizScroll;
    struct Gadget	dd_VertScroll;
    struct Gadget	dd_UpMove;
    struct Gadget	dd_DownMove;
    struct Gadget	dd_LeftMove;
    struct Gadget	dd_RightMove;
    struct Image	dd_HorizImage;
    struct Image	dd_VertImage;
    struct PropInfo	dd_HorizProp;
    struct PropInfo	dd_VertProp;
    struct Window *	dd_DrawerWin;	/* pointer to drawers window */
    struct WBObject *	dd_Object;	/* back pointer to drawer object */
    struct List		dd_Children;	/* where our children hang out */
    LONG		dd_Lock;
};

struct OldWBObject {
    struct Node		wo_MasterNode;	/* all objects are on this list */
    struct Node		wo_Siblings;	/* list of drawer members */
    struct Node		wo_SelectNode;	/* list of all selected objects */
    struct Node		wo_UtilityNode;	/* function specific linkages */
    struct WBObject *	wo_Parent;

    /* object flags */
#ifdef SMARTCOMPILER
    UBYTE		wo_IconDisp:1;	/* icon is currently in a window */
    UBYTE		wo_DrawerOpen:1;/* we're a drawer, and it is open */
    UBYTE		wo_Selected:1;	/* our icon is selected */
    UBYTE		wo_Background:1;/* set if icon is in background */
    UBYTE		wo_Invisible:1;	/* can't see this icon */
#else
    /* lattice is not full system V compatible (yet)... */
    UBYTE		wo_Flags;
#endif

    UBYTE		wo_Type;	/* what flavor object is this? */
    USHORT		wo_UseCount;	/* number of references to this obj */
    char *		wo_Name;	/* this object's textual name */
    SHORT		wo_NameXOffset;	/* where to put the name */
    SHORT		wo_NameYOffset;

    char *		wo_DefaultTool;
    struct NewDD *	wo_DrawerData;	/* if this is a drawer or disk */
    struct Window *	wo_IconWin; 	/* each object's icon lives here */
    LONG		wo_CurrentX;	/* virtual X in drawer */
    LONG		wo_CurrentY;	/* virtual Y in drawer */
    char **		wo_ToolTypes;	/* the types for this tool */
    struct Gadget	wo_Gadget;	/* NOT a ptr, but an instance of it */
    struct FreeList	wo_FreeList;	/* this objects free list */
    char *		wo_ToolWindow;	/* character string for tool's window */
    LONG		wo_StackSize;	/* how much stack to give to this */
    LONG		wo_Lock;	/* if this tool is in the backdrop */

};

#endif !OLDWBINTERNAL_H
