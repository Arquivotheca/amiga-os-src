/* lvmclass.h
 * ListView attributes
 * Copyright (C) 1991 Commodore-Amiga, Inc.
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <utility/tagitem.h>

/* Attached to the SpecialInfo field of the ListView gadget.  This is
 * READONLY, all changes MUST be done with SetGadgetAttrs() */
struct LVSpecialInfo
{
    struct List *si_List;	/* Pointer to list of structures */
    struct IBox si_View;	/* View rectangle */
    ULONG si_Flags;

    UWORD si_UnitWidth;		/* Width of each horizontal unit */
    UWORD si_UnitHeight;	/* Height of each vertical unit */

    LONG si_TopVert;		/* Vertical top */
    LONG si_VisibleVert;	/* Number of visible vertical units */
    LONG si_TotalVert;		/* Total number of vertical units */

    LONG si_TopHoriz;		/* Horizontal top */
    LONG si_VisibleHoriz;	/* Number of visible horizontal units */
    LONG si_TotalHoriz;		/* Total number of horizontal units */

    /* Anchor point */
    struct
    {
	LONG X;
	LONG Y;
    } si_Anchor;

    /* Point that the mouse is currently over */
    struct
    {
	LONG X;
	LONG Y;
    } si_Over;

};

/* These are global flags */
#define	LVSF_HORIZONTAL	  0x0002	/* Horizontal scroll bar */
#define	LVSF_VERTICAL	  0x0004	/* Vertical scroll bar */
#define	LVSF_BORDERLESS	  0x0008	/* No frame around view */
#define	LVSF_READONLY	  0x0010	/* Listview is read-only */
#define	LVSF_MULTISELECT  0x0020	/* Listview is multiselect. Sets
					 * LNF_SELECTED if item is selected. */
#define	LVSF_SIZABLE	  0x0040	/* Columns can be resized */
#define	LVSF_DRAGGABLE	  0x0080	/* Items can be dragged */
#define	LVSF_DRAGGING	  0x0100	/* MultiSelect dragging */
#define	LVSF_SELECT	  0x0200	/* Turn line on */
#define	LVSF_HIGHLIGHT	  0x0400	/* Toggled (PRIVATE) */

#define	LNF_SELECTED	  0x01		/* Set in lvn_Node.ln_Type if selected */

/* Tags used by the ListView class */
#define	AOLV_Dummy		(TAG_USER + 0x32500)

#define	AOLV_Borderless		(AOLV_Dummy + 1)
#define	AOLV_Freedom		(AOLV_Dummy + 2)
#define	AOLV_UnitWidth		(AOLV_Dummy + 3)
#define	AOLV_UnitHeight		(AOLV_Dummy + 4)
#define	AOLV_ReadOnly		(AOLV_Dummy + 5)
#define	AOLV_MultiSelect	(AOLV_Dummy + 6)
#define	AOLV_SpecialInfo	(AOLV_Dummy + 7)
#define	AOLV_TextAttr		(AOLV_Dummy + 8)

#define	AOLV_TopVert		(AOLV_Dummy + 10)
#define	AOLV_VisibleVert	(AOLV_Dummy + 11)
#define	AOLV_TotalVert		(AOLV_Dummy + 12)
#define	AOLV_VertArrowSize	(AOLV_Dummy + 13)
#define	AOLV_VertArrow		(AOLV_Dummy + 14)

#define	AOLV_TopHoriz		(AOLV_Dummy + 20)
#define	AOLV_VisibleHoriz	(AOLV_Dummy + 21)
#define	AOLV_TotalHoriz		(AOLV_Dummy + 22)
#define	AOLV_HorizArrowSize	(AOLV_Dummy + 23)
#define	AOLV_HorizArrow		(AOLV_Dummy + 24)

/* The following tags are for ListViewImages (columns) */
#define	AOLV_List		(AOLV_Dummy + 30)
#define	AOLV_Position		(AOLV_Dummy + 31) /* column position */
#define	AOLV_ColumnWidth	(AOLV_Dummy + 32) /* % of view */
#define	AOLV_Title		(AOLV_Dummy + 33) /* Title of column */
#define	AOLV_Justification	(AOLV_Dummy + 34) /* Use flags */
#define	AOLV_ControlHeight	(AOLV_Dummy + 35)
#define	AOLV_LabelHeight	(AOLV_Dummy + 36)

/* AOLV_Justification flags */
#define	AOLVF_LEFT		0x00 /* Left justify within column */
#define	AOLVF_RIGHT		0x10 /* Right justify within column */
#define	AOLVF_CENTER		0x20 /* Center within column. Not Impl. */

/* The following two tags are used to display fields from a structure.  The
 * very first field MUST be an embedded struct Node. */
#define	AOLV_FieldOffset	(AOLV_Dummy + 35) /* Offset of field in structure */
#define	AOLV_FieldType		(AOLV_Dummy + 36) /* Field type */

/* Field types, for use with AOLV_FieldType */
#define	FT_NODENAME	0
#define	FT_STRING	1
#define	FT_STRPTR	2
#define	FT_LONG		3
#define	FT_ULONG	4
#define	FT_SHORT	5
#define	FT_USHORT	6
#define	FT_BYTE		7
#define	FT_UBYTE	8

/* Additional messages */
#define	LV_ADDCOLUMN		(TAG_USER + OM_ADDMEMBER)
#define	LV_REMCOLUMN		(TAG_USER + OM_REMMEMBER)

/* LV_ADDCOLUMN */
struct opAddColumn
{
    ULONG MethodID;
    Object *opac_Object;	/* Column to add */
    Object *opac_Control;	/* Control gadget */
};

#define	LV_DRAWCOLUMN		(TAG_USER + IM_DRAWFRAME)
struct impDrawColumn
{
    ULONG MethodID;
    struct RastPort *imp_RPort;

    /* Upper left of gadget that column belongs to */
    struct
    {
	WORD X;
	WORD Y;
    } imp_Offset;

    ULONG imp_State;
    struct DrawInfo *imp_DrInfo;

    struct
    {
	WORD Width;
	WORD Height;
    } imp_Dimensions;

    /* Anchor point */
    struct
    {
	LONG X;
	LONG Y;
    } imp_Anchor;

    /* Point that the mouse is currently over */
    struct
    {
	LONG X;
	LONG Y;
    } imp_Over;

};

/* This is ORed into the imp_State to carry through the gpr_Redraw field */
#define	IDSF_UPDATE	0x4000000
#define	IDSF_REDRAW	0x2000000
#define	IDSF_TOGGLE	0x1000000

/* Class prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);

/* Misc. prototypes */
ULONG PointInBox (struct IBox * point, struct IBox * box);
VOID gadgetBox (struct Gadget * g, struct IBox * domain, struct IBox * box);
VOID computeDomain (Class * cl, Object * o, struct gpRender * gpr, struct IBox * box);
BOOL compareRect (struct IBox * b1, struct IBox * b2);
ULONG notifyAttrChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...);
LONG CountEntries (struct List *list);
