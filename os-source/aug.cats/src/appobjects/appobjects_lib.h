#ifndef LIBRARIES_APPOBJECTS_H
#define	LIBRARIES_APPOBJECTS_H

/* appobjects.h
 *
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef	INTUITION_CLASSES_H
#include <intuition/classes.h>
#endif

#ifndef INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif

#ifndef INTUITION_GADGETCLASS_H
#include <intuition/gadgetclass.h>
#endif

#ifndef INTUITION_IMAGECLASS_H
#include <intuition/imageclass.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef LIBRARIES_APPSHELL_H
#include <libraries/appshell.h>
#endif

/* Normal low-overhead IDCMP messages */
#define	IDCMP_flagF (CLOSEWINDOW | RAWKEY | MOUSEBUTTONS \
		     | GADGETDOWN | GADGETUP | MENUPICK | ACTIVEWINDOW \
		     | INACTIVEWINDOW | IDCMPUPDATE | IDCMP_MENUHELP)

/* IDCMP messages used when a hold or drag gadget is active */
#define	IDCMP_flagS (RAWKEY | MOUSEMOVE | MOUSEBUTTONS \
		     | INTUITICKS | GADGETUP | ACTIVEWINDOW \
		     | INACTIVEWINDOW | IDCMPUPDATE | IDCMP_MENUHELP)

/* Normal window flags */
#define	DEFWINFLAGS (WINDOWCLOSE | WINDOWDRAG | WINDOWDEPTH | ACTIVATE \
		     | REPORTMOUSE | SMART_REFRESH)

/* Often used qualifier pairs */
#ifndef SHIFTED
#define SHIFTED (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define ALTED (IEQUALIFIER_LALT | IEQUALIFIER_RALT)
#endif

/*--------------*/
/* Object Types */
/*--------------*/

/* GadTools gadgets */
#define	OBJ_Generic	1L
#define	OBJ_Button	2L
#define	OBJ_Checkbox	3L
#define	OBJ_Integer	4L
#define	OBJ_Listview	5L
#define	OBJ_MX		6L
#define	OBJ_Number	7L
#define	OBJ_Cycle	8L
#define	OBJ_Palette	9L
#define	OBJ_Scroller	10L
#define	OBJ_reserved1	11L		/* reserved for system use */
#define	OBJ_Slider	12L
#define	OBJ_String	13L
#define	OBJ_Text	14L

/* other gadgets */
#define	OBJ_Display	30L
#define	OBJ_Select	31L
#define	OBJ_Dropbox	32L
#define	OBJ_GImage	33L
#define	OBJ_MultiText	34L
#define	OBJ_reserved2	35L
#define	OBJ_DirString	36L
#define	OBJ_DirNumeric	37L
#define	OBJ_boopsi	38L
#define	OBJ_View	39L		/* View gadget */
#define	OBJ_MListView	40L		/* View gadget with scrollbars */

/* images */
#define	OBJ_Image	50L
#define	OBJ_Column	51L		/* Can only be used with OBJ_View
					 * and OBJ_ListView */

/* borders */
#define	OBJ_reserved3	60L
#define	OBJ_BevelIn	61L
#define	OBJ_BevelOut	62L
#define	OBJ_DblBevelIn	63L
#define	OBJ_DblBevelOut	64L

/* other object types */
#define	OBJ_Screen	70L		/* screen information */
#define	OBJ_Window	71L		/* window information */
#define	OBJ_Group	72L		/* Layout group */
#define	OBJ_VFill	73L		/* Vertical fill */
#define	OBJ_HFill	74L		/* Horizontal fill */
#define	OBJ_VGroup	75L		/* Vertical layout group */
#define	OBJ_HGroup	76L		/* Horizontal layout group */
#define	OBJ_MGroup	77L		/* Mutual Exclude group */

/* Intuition user-interface object (gadget, border, text, etc... Currently
 * the AppShell has stolen the UserData fields of both gadgets and windows.
 * o_Key is used to assign a key to a GadTools object.  Our boopsi classes
 * understand the _ sign in a label as indicating the key assigned to a
 * gadget. */
struct Object
{
    struct Object *o_NextObject;/* next object in array */
    WORD o_Group;		/* Object group */
    WORD o_Priority;		/* Inclusion priority of object */
    ULONG o_Type;		/* type */
    ULONG o_ObjectID;		/* ID */
    ULONG o_Flags;		/* see defines below */
    UWORD o_Key;		/* hotkey */
    STRPTR o_Name;		/* name */
    ULONG o_LabelID;		/* label index into text catalogue */
    struct IBox o_Outer;	/* size w/label */
    struct TagItem *o_Tags;	/* tags for object */
    APTR o_UserData;		/* user data for object */
};

/* Use this for a system computed o_Outer edge, such as the left edge of
 * all buttons in a horizontal group that get evenly spaced relativity. */
#define	FLOATING_EDGE	(~0)

/* To indicate that this gadget is the default gadget to activate when the
 * window gets activated. */
#define	APSH_OBJF_ACTIVATE	(1L<<0)

/* To indicate that a gadget is used to close a window, set the following
 * flag.  The window is closed after any functions for the gadget have
 * completed. */
#define	APSH_OBJF_CLOSEWINDOW	(1L<<1)

/* Don't adjust the gadget rectangle.  Don't use this flag unless your
 * absolutely sure you know what you're doing.  Don't use it. */
#define	APSH_OBJF_NOADJUST	(1L<<2)

/* Used to indicate that the object is draggable, like an icon. This flag
 * should only applied to the OBJ_GImage object type.  Any other object
 * type will provide unpredictable results. */
#define	APSH_OBJF_DRAGGABLE	(1L<<3)

/* OBSOLETE!! */
#define	APSH_OBJF_LOCALTEXT	(1L<<4)

/* Don't scale the gadget rectangle.  The main use for this flag is for
 * objects that are going into the border. */
#define	APSH_OBJF_NOSCALE	(1L<<5)

/* Indicate that the text is to come from the global text table, when there
 * is a local window text table. */
#define	APSH_OBJF_GLOBALTEXT	(1L<<6)

/* Object (usually only for group objects) gets an engraved border. */
#define	APSH_OBJF_BORDER	(1L<<7)

/* Object isn't a gadget, so don't add to GList. */
#define	APSH_OBJF_NOGADGET	(1L<<8)

/* This is used to describe the environment to the layout engine */
struct ObjectInfo
{
    /* The following fields must be supplied initially */
    struct Screen *oi_Screen;		/* Screen that object resides in */
    struct TextAttr *oi_TextAttr;	/* Text attribute of font to use */
    struct NewWindow *oi_NewWindow;	/* Pointer to NewWindow */
    struct TagItem *oi_WindowAttrs;	/* NewWindow attributes */
    struct IBox *oi_ZoomBox;		/* Pointer to ZoomBox */
    STRPTR *oi_TextTable;		/* Pointer to text table to use */
    STRPTR *oi_LocalText;		/* Pointer to the local text table */
    struct Object *oi_Objects;		/* List of objects */

    /* The remaining fields are maintained by the system */
    struct Rectangle oi_View;		/* View rectangle */
    struct Gadget *oi_Gadgets;		/* Gadget list to AddGList to window */
    struct Gadget *oi_GList;		/* PRIVATE Gadget work list */
    struct Gadget *oi_Active;		/* Active/Activate gadget */
    struct DrawInfo *oi_DrInfo;		/* Rendering information */
    VOID *oi_VisualInfo;		/* GadTools visual information */
    struct TextFont *oi_TextFont;	/* Font to use */
    struct Window *oi_Window;		/* Window that object resides in */
    struct RastPort oi_RastPort;	/* PRIVATE RastPort for rendering */
    UWORD oi_Size;			/* SYSIA_Size of screen */
    UWORD oi_PriorKey;			/* Prior keystroke */
    struct ObjectNode *oi_PriorObj;	/* Prior keystroke object */
    struct List oi_ObjList;		/* List of objects */
    ULONG oi_Flags;
    struct IBox oi_WinBox;		/* Window rectangle */
    struct IBox oi_Zoom;		/* Zoom rectangle */
    APTR oi_UserData;			/* Extra user information */
    APTR oi_SystemData;			/* Extra system information */
};

/* This environment contains relative GadTool objects */
#define	AOIF_REMOVE	(1L<<0)

/* Gadget list has been removed */
#define	AOIF_REMOVED	(1L<<1)

/* This environment contains GadTool objects */
#define	AOIF_GADTOOLS	(1L<<2)

/* State information: we're working on objects in an opened window */
#define	AOIF_REFRESH	(1L<<3)
#define	AOIF_TAILOR	(1L<<4)

/* Information required for each Intuition object in the list. */
struct ObjectNode
{
    struct Node on_Node;		/* Node in the object list */
    struct Object on_Object;		/* Embedded object structure */
    struct TagItem *on_OTags;		/* Reserved for system use */
    struct IBox on_OBox;		/* Reserved for system use */
    struct Gadget *on_Gadget;		/* Main gadget for object */
    ULONG on_Flags;			/* Extended flags */
    ULONG on_Funcs[10];			/* Function ID's */
    ULONG on_Current;			/* Current function ID */
    APTR on_Image;			/* Image for object */
    APTR on_ObjData;			/* Object data */
    APTR on_ExtraData;			/* Extra information for object */
    APTR on_UserData;			/* Extra user information */
    APTR on_SystemData;			/* Extra system information */
};

/* Gadget is selected by keystroke */
#define	ONF_KEYSELECT	(1L<<0)

/* Reserved for system use */
#define	ONF_CONVERTED	(1L<<1)
#define	ONF_DELETED	(1L<<2)

/* on_SystemData points to an ObjectData structure */
#define	ONF_OBJDATA	(1L<<3)

/* Extended Gadget function array pointers */
#define	EG_DOWNPRESS	0
#define	EG_HOLD		1
#define	EG_RELEASE	2
#define	EG_DBLCLICK	3
#define	EG_ABORT	4
#define	EG_ALTHIT	5
#define	EG_SHIFTHIT	6
#define	EG_CREATE	7
#define	EG_DELETE	8
#define	EG_UPDATE	9

/* Data structure for Listview, Cycle, and MutualExclude */
struct ObjectData
{
    APTR od_Label;			/* Pointer to the label(s) */
    LONG od_Current;			/* Current value */
    LONG od_Top;			/* Top of the view */
    LONG od_View;			/* Number that fit in view */
    LONG od_Max;			/* Number of labels */
    LONG od_Flags;
    APTR od_UserData;
    APTR od_SystemData;
};

/* Attributes for our gadgets */
#define	CGTA_Dummy		(TAG_USER + 0x32100)
#define	CGTA_reserved1		(CGTA_Dummy + 1)
#define	CGTA_reserved2		(CGTA_Dummy + 2)
#define	CGTA_HighPens		(CGTA_Dummy + 3)	/* Highlight pens */
#define	CGTA_NextField		(CGTA_Dummy + 4)	/* OBSOLETE: TAB to */
#define	CGTA_PrevField		(CGTA_Dummy + 5)	/* OBSOLETE: SHIFT-TAB to */
#define	CGTA_Total		(CGTA_Dummy + 6)	/* Maximum rows */
#define	CGTA_Visible		(CGTA_Dummy + 7)	/* Visible rows */
#define	CGTA_Top		(CGTA_Dummy + 8)	/* Top row */
#define	CGTA_List		(CGTA_Dummy + 9)	/* History list */
#define CGTA_Increment		(CGTA_Dummy + 23)	/* Increment value */
#define CGTA_Decrement		(CGTA_Dummy + 24)	/* Decrement value */
#define	CGTA_LabelInfo		(CGTA_Dummy + 100)	/* Label information */
#define	CGTA_Keystroke		(CGTA_Dummy + 101)	/* Inquire keystroke */
#define	CGTA_Borderless		(CGTA_Dummy + 102)	/* No border */
#define	CGTA_DisplayOnly	(CGTA_Dummy + 103)	/* Display only */
#define	CGTA_MinWidth		(CGTA_Dummy + 104)	/* Minimum width */
#define	CGTA_MinHeight		(CGTA_Dummy + 105)	/* Minimum height */
#define	CGTA_MaxWidth		(CGTA_Dummy + 106)	/* Maximum width */
#define	CGTA_MaxHeight		(CGTA_Dummy + 107)	/* Maximum height */
#define	CGTA_FrameInfo		(CGTA_Dummy + 108)	/* Frame information */
#define	CGTA_Constraint		(CGTA_Dummy + 109)	/* Sizing contraints */
#define	CGTA_Nominal		(CGTA_Dummy + 110)	/* Nominal size */
#define	CGTA_TextAttr		(CGTA_Dummy + 111)	/* Font to use for label */
#define	CGTA_Master		(CGTA_Dummy + 112)	/* Main gadget in group */
#define	CGTA_Transparent	(CGTA_Dummy + 113)	/* Not hit-able */
#define	CGTA_InBorder		(CGTA_Dummy + 114)	/* IMAGE is in border */
#define	CGTA_Parent		(CGTA_Dummy + 115)	/* Label's parent */

/* CGTA_LabelInfo flags */
#define	LABEL_RIGHT		0x0001
#define	LABEL_BOTTOM		0x0002
#define	LABEL_VCENTER		0x0004
#define	LABEL_HCENTER		0x0008
#define	LABEL_CENTER		(LABEL_VCENTER | LABEL_HCENTER)
#define	LIBITS			(LABEL_RIGHT | LABEL_BOTTOM | LABEL_CENTER)

/* Use the text highlight pen for text labels */
#define	LABEL_HIGHLIGHT		0x0010

/* Don't add a border to the gadget */
#define	CGTF_BORDERLESS		0x0020

/* Make the gadget readonly */
#define	CGTF_READONLY		0x0040

/* CGTA_FrameInfo flags */
#define	FRAME_RIGHT		0x0100
#define	FRAME_BOTTOM		0x0200
#define	FRAME_VCENTER		0x0400
#define	FRAME_HCENTER		0x0800
#define	FRAME_CENTER		(FRAME_VCENTER | FRAME_HCENTER)
#define	FIBITS			(FRAME_RIGHT | FRAME_BOTTOM | FRAME_CENTER)

#define	CGTF_TRANSPARENT	0x1000

#define	NOT_SET			(~0)

/* Additional label placement flags */
#define	PLACETEXT_ABOVEC	0x0040
#define	PLACETEXT_ABOVEI	0x0080
#define	PLACETEXT_BELOWI	0x0100
#define	PLACETEXT_BELOWC	0x0200

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
#define	LNF_TEMPORARY	  0x02		/* (PRIVATE) */
#define	LNF_REFRESH	  0x04		/* (PRIVATE) */

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
#define	AOLV_View		(AOLV_Dummy + 9) /* View size (IBox) */

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
#define	AOLV_Sizable		(AOLV_Dummy + 25)
#define	AOLV_Draggable		(AOLV_Dummy + 26)

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
#define	FT_BOOL		9
#define	FT_IMAGE	10

/* Additional messages */
#define	LV_ADDCOLUMN		(0x1000 + OM_ADDMEMBER)
#define	LV_REMCOLUMN		(0x1000 + OM_REMMEMBER)

/* LV_ADDCOLUMN */
struct opAddColumn
{
    ULONG MethodID;
    Object *opac_Object;	/* Column to add */
    Object *opac_Control;	/* Control gadget */
};

#define	LV_DRAWCOLUMN		(0x1000 + IM_DRAWFRAME)
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

    ULONG imp_Mode;		/* Drawing mode */

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

/* This is the drawing mode */
#define	IDSM_TOGGLE	0
#define	IDSM_REDRAW	1
#define	IDSM_UPDATE	2

#endif /* LIBRARIES_APPOBJECTS_H */
