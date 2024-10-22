	ifnd LIBRARIES_APPOBJECTS_I
LIBRARIES_APPOBJECTS_I	EQU	1

* appobjects.i
*
*

	ifnd EXEC_TYPES_I
	include "exec/types.i"
	endc
	include "intuition/intuition.i"
	include "intuition/gadgetclass.i"
	include "intuition/imageclass.i"
	include "utility/tagitem.i"

	ifnd LIBRARIES_APPSHELL_I
	include "libraries/appshell.i"
	endc

* Normal low-overhead IDCMP messages *

TempFlags	SET  CLOSEWINDOW|RAWKEY|MOUSEBUTTONS
TempFlags	SET  TempFlags|GADGETDOWN|GADGETUP|MENUPICK
TempFlags	SET  TempFlags|ACTIVEWINDOW|INACTIVEWINDOW
IDCMP_flagF 	EQU  TempFlags

* IDCMP messages used when a hold or drag gadget is active *

TempFlags	SET  RAWKEY|MOUSEMOVE|MOUSEBUTTONS
TempFlags	SET  TempFlags|INTUITICKS|GADGETUP
TempFlags	SET  TempFlags|ACTIVEWINDOW|INACTIVEWINDOW
IDCMP_flagS	EQU  TempFlags

* Normal window flags *

TempFlags	SET  WINDOWCLOSE|WINDOWDRAG|WINDOWDEPTH|ACTIVATE
TempFlags	SET  TempFlags|REPORTMOUSE|SMART_REFRESH
DEFWINFLAGS	EQU  TempFlags

* Often used qualifier pairs *

SHIFTED		EQU  IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT
ALTED		EQU  IEQUALIFIER_LALT|IEQUALIFIER_RALT

*--------------*
* Object Types *
*--------------*

* GadTools gadgets *
OBJ_Generic	EQU	1
OBJ_Button	EQU	2
OBJ_Checkbox	EQU	3
OBJ_Integer	EQU	4
OBJ_Listview	EQU	5
OBJ_MX		EQU	6
OBJ_Number	EQU	7
OBJ_Cycle	EQU	8
OBJ_Palette	EQU	9
OBJ_Scroller	EQU	10
OBJ_reserved1	EQU	11		; reserved for system use 
OBJ_Slider	EQU	12
OBJ_String	EQU	13
OBJ_Text	EQU	14

* other gadgets *
OBJ_Display	EQU	30
OBJ_Select	EQU	31
OBJ_Dropbox	EQU	32
OBJ_GImage	EQU	33
OBJ_MultiText	EQU	34
OBJ_reserved2	EQU	35
OBJ_DirString	EQU	36
OBJ_DirNumeric	EQU	37

* images *
OBJ_Image	EQU	50

* borders *
OBJ_reserved3	EQU	60
OBJ_BevelIn	EQU	61
OBJ_BevelOut	EQU	62
OBJ_DblBevelIn	EQU	63
OBJ_DblBevelOut	EQU	64

* other object types *
OBJ_Screen	EQU	70		; screen information 
OBJ_Window	EQU	71		; window information 
OBJ_Group	EQU	72		; Layout group 
OBJ_VFill	EQU	73		; Vertical fill 
OBJ_HFill	EQU	74		; Horizontal fill 
OBJ_VGroup	EQU	75		; Vertical layout group 
OBJ_HGroup	EQU	76		; Horizontal layout group 

* Intuition user-interface object (gadget, border, text, etc... Currently
* the AppShell has stolen the UserData fields of both gadgets and windows.
* Hopefully the next release of AppShell will give these fields back,
* or provide a way to use them.  o_Key is used to assign a key to a
* Gadget Toolkit (GadTools) object.  Our boopsi classes understand the
* @ sign in a label as indicating the key assigned to a gadget.

	STRUCTURE Object,0
	APTR	o_NextObject 	; next object in array 
    	WORD	o_Group		; Object group 
    	WORD	o_Priority	; Inclusion priority of object 
    	ULONG	o_Type		; type 
    	ULONG	o_ObjectID	; ID 
    	ULONG	o_Flags		; see defines below 
    	UWORD	o_Key		; hotkey 
    	APTR	o_Name		; name 
    	ULONG	o_LabelID	; label index into text catalogue 
    	STRUCT	o_Outer,ibox_SIZEOF	; size w/label 
    	APTR	o_Tags		; tags for object 
    	APTR	o_UserData	; user data for object 
	LABEL	o_SIZEOF

* Use this for a system computed o_Outer edge, such as the left edge of
* all buttons in a horizontal group that get evenly spaced relativity. 

FLOATING_EDGE	EQU	(~0)

* To indicate that this gadget is the default gadget to activate when the
* window gets activated. 

APSH_OBJF_ACTIVATE	EQU	(1<<0)

* To indicate that a gadget is used to close a window, set the following
* flag.  The window is closed after any functions for the gadget have
* completed. 

APSH_OBJF_CLOSEWINDOW	EQU	(1<<1)

* Don't adjust the gadget rectangle.  Don't use this flag unless your
* absolutely sure you know what you're doing.  Don't use it. 

APSH_OBJF_NOADJUST	EQU	(1<<2)

* Used to indicate that the object is draggable, like an icon.  This flag
* should only applied to the OBJ_GImage object type.  Any other object type
* will provide unpredictable results

APSH_OBJF_DRAGGABLE	EQU	(1<<3)

* Inicate that the text is to come from the local text table

APSH_OBJF_LOCALTEXT	EQU	(1<<4)

* This is used to describe the environment to the layout engine 

	STRUCTURE ObjectInfo,0

	; The following fields must be supplied initially 

    	APTR 	oi_Screen		; Screen that object resides in 
	APTR	oi_TextAttr		; Text attribute of font to use 
	APTR	oi_NewWindow		; Pointer to NewWindow 
	APTR	oi_WindowAttrs		; NewWindow attributes 
	APTR	oi_ZoomBox		; Pointer to ZoomBox (IBOX) 
	APTR	oi_TextTable		; Pointer to text table to use 
	APTR	oi_LocalText		; Pointer to the local text table
	APTR	oi_Objects		; List of objects 

	; The remaining fields are maintained by the system 

	APTR	oi_Gadgets		; Gadget list to AddGList to window 
	APTR	oi_GList		; PRIVATE Gadget work list 
	APTR	oi_Active		; Active/Activate gadget 
	APTR	oi_DrInfo		; Rendering information (DrawInfo) 
	APTR	oi_VisualInfo		; GadTools visual information 
	APTR	oi_TextFont		; Font to use 
	APTR	oi_Window		; Window that object resides in 
    	STRUCT	oi_RastPort,rp_SIZEOF	; PRIVATE RastPort for rendering 
	UWORD	oi_PriorKey		; Prior keystroke 
	APTR	oi_PriorObj		; Prior keystroke object 
	STRUCT	oi_ObjList,LH_SIZE	; List of objects 
	ULONG	oi_Flags		;
	STRUCT	oi_WinBox,ibox_SIZEOF	; Window Rectangle
	STRUCT	oi_Zoom,ibox_SIZEOF	; Zoom Rectangle
	APTR	oi_UserData		; Extra user information
	APTR	oi_SystemData		; Extra system information
	LABEL	oi_SIZEOF

* This environment contains relative GadTool objects
	BITDEF	AOI,REMOVE,0

* Gadget list has been removed
	BITDEF	AOI,REMOVED,1

* This environment contains GadTool objects
	BITDEF	AOI,GADTOOLS,2

* State information: we're working on objects in an opened window
	BITDEF	AOI,REFRESH,3
	BITDEF	AOI,TAILOR,4

* Information required for each Intuition object in the list. 

	STRUCTURE ObjectNode,0
	STRUCT	on_Node,LN_SIZE		; Node in the object list 
    	STRUCT	on_Object,o_SIZEOF	; Embedded object structure
	APTR	on_OTags		; Reserved for system use
	STRUCT	on_OBox,ibox_SIZEOF	; Reserved for system use 
    	APTR	on_Gadget		; Main gadget for object 
	ULONG	on_Flags		; Extended flags 
	STRUCT	on_Funcs,4*10		; Function ID's 
	ULONG	on_Current		; Current function ID 
	APTR	on_Image		; Image for object 
	APTR	on_ObjData		; Object data 
	APTR	on_ExtraData		; Extra information for object
	APTR	on_UserData		; Extra user information
	APTR	on_SystemData		; Extra system information
	LABEL	on_SIZEOF

* Gadget is selected by keystroke *
	BITDEF ON,KEYSELECT,0

* Reserved for system use
	BITDEF ON,CONVERTED,1
	BITDEF ON,DELETED,2

* Extended Gadget function array pointers *
EG_DOWNPRESS	EQU	0
EG_HOLD		EQU	1
EG_RELEASE	EQU	2
EG_DBLCLICK	EQU	3
EG_ABORT	EQU	4
EG_ALTHIT	EQU	5
EG_SHIFTHIT	EQU	6
EG_CREATE	EQU	7
EG_DELETE	EQU	8

* Attributes for our gadgets *
CGTA_Dummy		EQU	(TAG_USER+32100H)
CGTA_reserved1		EQU	(CGTA_Dummy+1)
CGTA_reserved2		EQU	(CGTA_Dummy+2)
CGTA_HighPens		EQU	(CGTA_Dummy+3)	; Highlight pens 
CGTA_NextField		EQU	(CGTA_Dummy+4)	; TAB to 
CGTA_PrevField		EQU	(CGTA_Dummy+5)	; SHIFT-TAB to 
CGTA_Total		EQU	(CGTA_Dummy+6)	; Maximum rows 
CGTA_Visible		EQU	(CGTA_Dummy+7)	; Visible rows 
CGTA_Top		EQU	(CGTA_Dummy+8)	; Top row 
CGTA_List		EQU	(CGTA_Dummy+9)	; History list 
CGTA_Increment		EQU	(CGTA_Dummy+23)	; Increment value 
CGTA_Decrement		EQU	(CGTA_Dummy+24)	; Decrement value 
CGTA_LabelInfo		EQU	(CGTA_Dummy+100)	; Label information 
CGTA_Keystroke		EQU	(CGTA_Dummy+101)	; Inquire keystroke 
CGTA_Borderless		EQU	(CGTA_Dummy+102)	; No border 
CGTA_DisplayOnly	EQU	(CGTA_Dummy+103)	; Display only 
CGTA_MinWidth		EQU	(CGTA_Dummy+104)	; Minimum width 
CGTA_MinHeight		EQU	(CGTA_Dummy+105)	; Minimum height 
CGTA_MaxWidth		EQU	(CGTA_Dummy+106)	; Maximum width 
CGTA_MaxHeight		EQU	(CGTA_Dummy+107)	; Maximum height 
CGTA_FrameInfo		EQU	(CGTA_Dummy+108)	; Frame information 
CGTA_Constraint		EQU	(CGTA_Dummy+109)	; Sizing contraints 
CGTA_Nominal		EQU	(CGTA_Dummy+110)	; Nominal size 
CGTA_TextAttr		EQU	(CGTA_Dummy+111)	; Font to use for label 
CGTA_Master		EQU	(CGTA_Dummy+112)	; Main gadget in group 
CGTA_Transparent	EQU	(CGTA_Dummy+113)	; Not hit-able 
CGTA_InBorder		EQU	(CGTA_Dummy+114)	; IMAGE is in border

* CGTA_LabelInfo flags *
LABEL_RIGHT		EQU	0001H
LABEL_BOTTOM		EQU	0002H
LABEL_VCENTER		EQU	0004H
LABEL_HCENTER		EQU	0008H
LABEL_CENTER		EQU	(LABEL_VCENTER|LABEL_HCENTER)
LIBITS			EQU	(LABEL_RIGHT|LABEL_BOTTOM|LABEL_CENTER)

* Use the text highlight pen for text labels *
LABEL_HIGHLIGHT		EQU	0010H

* Don't add a border to the gadget *
CGTF_BORDERLESS		EQU	0020H

* Make the gadget readonly *
CGTF_READONLY		EQU	0040H

* CGTA_FrameInfo flags *
FRAME_RIGHT		EQU	0100H
FRAME_BOTTOM		EQU	0200H
FRAME_VCENTER		EQU	0400H
FRAME_HCENTER		EQU	0800H
FRAME_CENTER		EQU	(FRAME_VCENTER|FRAME_HCENTER)
FIBITS			EQU	(FRAME_RIGHT|FRAME_BOTTOM|FRAME_CENTER)

CGTF_TRANSPARENT	EQU	1000H

NOT_SET			EQU	(~0)

	endc 	;LIBRARIES_APPOBJECTS_I
