	IFND	WORKBENCHBASE_H
WORKBENCHBASE_H	EQU	1

*********************************************************************
*
* $Id: workbenchbase.i,v 38.11 93/01/21 14:49:08 mks Exp $
*
* $Log:	workbenchbase.i,v $
* Revision 38.11  93/01/21  14:49:08  mks
* Cleaned up the disk init code...  (Removed much redundant code)
* 
* Revision 38.10  92/08/20  09:58:34  mks
* Added wb_Copyright4
*
* Revision 38.9  92/07/13  18:34:37  mks
* changed to contain a config lock
*
* Revision 38.8  92/06/11  09:25:24  mks
* Renamed some of the bits as unused
*
* Revision 38.7  92/04/16  09:16:30  mks
* Added the copyright string pointers
*
* Revision 38.6  92/04/14  13:21:27  mks
* Removed the WaitPointer field
*
* Revision 38.5  92/02/18  10:20:49  mks
* Fixed it...
*
* Revision 38.4  92/01/07  16:09:50  mks
* Major rework of the workbenchbase for the new config stuff.
*
* Revision 38.3  91/11/12  18:41:37  mks
* Updated to support even better locale stuff
*
* Revision 38.2  91/09/12  10:44:27  mks
* Added two long words to the library base for locale support...
*
* Revision 38.1  91/06/24  11:40:02  mks
* Initial V38 tree checkin - Log file stripped
*
*********************************************************************

	IFND	EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC	!EXEC_TYPES_I

	IFND	EXEC_LISTS_I
	INCLUDE	"exec/lists.i"
	ENDC	!EXEC_LISTS_I

	IFND	EXEC_PORTS_I
	INCLUDE	"exec/ports.i"
	ENDC	!EXEC_PORTS_I

	IFND	EXEC_LIBRARIES_I
	INCLUDE	"exec/libraries.i"
	ENDC	!EXEC_LIBRARIES_I

	IFND	LIBRARIES_DOS_I
	INCLUDE	"libraries/dos.i"
	ENDC	!LIBRARIES_DOS_I

	IFND	LIBRARIES_DOSEXTENS_I
	INCLUDE	"libraries/dosextens.i"
	ENDC	!LIBRARIES_DOSEXTENS_I

	IFND	GRAPHICS_RASTPORT_I
	INCLUDE	"graphics/rastport.i"
	ENDC	!GRAPHICS_RASTPORT_I

	IFND	DEVICES_TIMER_I
	INCLUDE	"devices/timer.i"
	ENDC	!DEVICES_TIMER_I

	IFND	INTUITION_INTUITION_I
	INCLUDE	"intuition/intuition.i"
	ENDC	!INTUITION_INTUITION_I

	IFND	WORKBENCH_WORKBENCH_I
	INCLUDE	"workbench.i"
	ENDC

	INCLUDE	"exec/semaphores.i"
	INCLUDE	"utility/hooks.i"

; a pointer to the workbenchbase is kept in A6
WBBASE	EQUR	A6

VIEWBYTEXTLEFTOFFSET	EQU	4

WBBUFSIZE	EQU	260	; sizeof (struct FileInfoBlock) ???

 STRUCTURE SimpleMenu,0
    ULONG sm_text
    UWORD sm_cmd
    UWORD sm_flags
    ULONG sm_mutualexclude
    UBYTE sm_toolmenu
    LABEL SimpleMenu_SIZEOF


 STRUCTURE IconMsg,0
    STRUCT im_Message,IOSTD_SIZE	; msg struct
    LONG im_dobj			; flag (Delete or Put)
    APTR im_lock			; lock on parentdir of name
    UBYTE im_name			; storage for name starts here
    LABEL IconMsg_SIZEOF

 STRUCTURE TypedIconMsg,0
    WORD tim_type			; msg type
    STRUCT tim_IconMsg,IconMsg_SIZEOF	; IconMsg struct
    LABEL TypedIconMsg_SIZEOF

 STRUCTURE WBConfig,0
	LONG	wbc_Version	;	/* This needs to be n... */
	LONG	wbc_Reserved	;	/* This is currently 0 */
	WORD	wbc_LeftEdge	;	/* These values are the left/top/width/height */
	WORD	wbc_TopEdge	;	/* of the backdrop window if it is not a */
	WORD	wbc_Width	;	/* backdrop.  These values *MUST* remain in */
	WORD	wbc_Height	;	/* This location if Version > 0 */
	WORD	wbc_Backdrop	;	/* Backdrop flag */
	WORD	wbc_Doubleclick	;	/* Not used currently... */
	LABEL	WBConfig_SIZEOF

 STRUCTURE PatternBitMap,0
	APTR	pbm_BitMap
	UWORD	pbm_Width
	UWORD	pbm_Height
	LABEL	pbm_SIZEOF

 STRUCTURE BitMap_Sem,0
	STRUCT	bms_Semaphore,SS_SIZE
	APTR	bms_PatternBitMap
	UWORD	bms_ToMinX
	UWORD	bms_ToMinY
	UWORD	bms_FromMinX
	UWORD	bms_FromMinY
	UWORD	bms_FromMaxX
	UWORD	bms_FromMaxY
	UWORD	bms_SaveMinX
	UWORD	bms_SaveMaxX
	UWORD	bms_SaveMinX1
	UWORD	bms_pad		; Longword allign...
	LABEL	bms_SIZEOF

 STRUCTURE WorkbenchFont,0
	APTR	wf_Attr
	APTR	wf_Font
	UWORD	wf_DrawMode
	UBYTE	wf_FrontPen
	UBYTE	wf_BackPen
	LABEL	wf_SIZEOF

 STRUCTURE WB_Semaphore,0
	STRUCT	wbs_Semaphore,SS_SIZE
	STRUCT	wbs_RootPattern,bms_SIZEOF
	STRUCT	wbs_WindowPattern,bms_SIZEOF
	APTR	wbs_Icon
	APTR	wbs_Text
	LABEL	wbs_SIZEOF

 STRUCTURE WorkbenchBase,0

    STRUCT	wb_Library,LIB_SIZE

    BYTE	wb_UpdateNestCnt
    BYTE	wb_DiskIONestCnt
    ; now longword aligned

    ; ptr to wb's seglist
    APTR	wb_SegList

    ; the workbench task
    ULONG	wb_Task

    ; the work bench status flags
    UBYTE	wb_Flags1
    UBYTE	wb_Flags2
    UWORD	wb_LastFlags	; Magic stuff!!

    ; three message ports -- two for intuition, and one for all reasonable
    ; applications that let you allocate your messages yourself....
    STRUCT	wb_WorkbenchPort,MP_SIZE

    STRUCT	wb_IntuiPort,MP_SIZE
    STRUCT	wb_SeenPort,MP_SIZE
    STRUCT	wb_NotifyPort,MP_SIZE

    APTR	wb_CopyReaderPort	; Reader port for Copy
    APTR	wb_CopyWriterPort	; Writer port for Copy

    ; all of our library bases
    APTR	wb_SysBase
    APTR	wb_GfxBase
    APTR	wb_IntuitionBase
    APTR	wb_IconBase
    APTR	wb_DOSBase
    APTR	wb_LayersBase
    APTR	wb_GadToolsBase
    APTR	wb_UtilityBase
    APTR	wb_TimerBase

    ; The three copyright pointers
    APTR	wb_Copyright1
    APTR	wb_Copyright2
    APTR	wb_Copyright3
    APTR	wb_Copyright4

    ; our three major lists.  Every object is on the master list.
    ; The highlighted (selected) icons are on the select list.
    ; all disks that we have "active" are on the active list.
    STRUCT	 wb_MasterList,LH_SIZE
    STRUCT	 wb_SelectList,LH_SIZE
    STRUCT	 wb_ActiveDisks,LH_SIZE
    STRUCT	 wb_UtilityList,LH_SIZE

    STRUCT	wb_AppWindowList,LH_SIZE	; list of App Windows
    STRUCT	wb_AppIconList,LH_SIZE		; list of App Icons
    STRUCT	wb_AppMenuItemList,LH_SIZE	; list of App MenuItems

    STRUCT	wb_BobList,LH_SIZE	; all bobs are on this list

    STRUCT	wb_InfoList,LH_SIZE	; list of .info files
    STRUCT	wb_NonInfoList,LH_SIZE	; list of non .info files

    ULONG	wb_OldPath		; Old path
    LONG	wb_OldPathTick		; Tick count on old path

    LONG	wb_ByteCount		; Count bytes used in copy...

    ; the mouse position of the last select press
    UWORD	wb_XOffset
    UWORD	wb_YOffset

    ; For locale....
    APTR	wb_LocaleBase
    APTR	wb_Catalog

    ; a pointer to a timer request, so we can free it later
    APTR	wb_TimerRequest

    ; the base of the workbench object tree
    APTR	wb_RootObject

    ; some current pointers -- some of these should be deleted
    APTR	wb_CurrentGadget
    APTR	wb_CurrentWindow
    APTR	wb_CurrentObject

    ; Last IDCMPWindow to get a CloseWindow event.
    APTR	wb_LastCloseWindow

    ; images for arrow gadgets
    APTR	wb_LeftImage
    APTR	wb_RightImage
    APTR	wb_UpImage
    APTR	wb_DownImage

    ;
    ; Now, we need the two call-back hooks here...
    ;
    STRUCT	wb_WBHook,h_SIZEOF	; WB Window Hook
    STRUCT	wb_WinHook,h_SIZEOF	; All other windows Hook

    ; two string buffers to save us from allocating memory
    ; THESE TWO BUFFERS MUST BE LONGWORD ALLIGNED
    STRUCT	wb_Buf0,WBBUFSIZE
    STRUCT	wb_Buf1,WBBUFSIZE

    ; misc stuff
    ULONG	wb_LastFreeMem
    APTR	wb_IconFont
    APTR	wb_IconAttr
    APTR	wb_TextFont
    APTR	wb_TextAttr

    ; the last time we saw a key-down
    STRUCT	wb_Tick,TV_SIZE

    ; what we need to paint text
    STRUCT	wb_IconRast,rp_SIZEOF
    STRUCT	wb_TextRast,rp_SIZEOF

    ; the argument that we were initialized with
    ULONG	wb_Argument

    ; stuff for moving bobs dynamically
    STRUCT	wb_BobMem,FreeList_SIZEOF	; all bob mem is here
    WORD	wb_LastX		; last X of mouse was here
    WORD	wb_LastY		; last Y of mouse was here

    APTR	wb_Screen		; pointer to workbench screen
    APTR	wb_BackWindow		; pointer to workbench window
    APTR	wb_GelsInfo		; pointer to screen gels info

    UWORD	wb_IconFontHeight	; height of icon font

    ; these two lists are used for 'Show All'
    UWORD	wb_MaxTextWidth		; used by viewbytext for drawers

    APTR	wb_MenuStrip		; main menu
    APTR	wb_ToolMenu		; tool menu
    APTR	wb_HiddenMenu		; debug menu

    APTR	wb_LayerDemonRequest	; timer request for locked layer checking
    STRUCT	wb_LayerPort,MP_SIZE	; msgs from locked layer request

    APTR	wb_VisualInfo		; GadTools VisualInfo

    UWORD	wb_AsyncCopyCnt		; # of outstanding async copies going on
    UWORD	wb_Word2		; used during testing

    STRUCT	wb_Config,WBConfig_SIZEOF
    ULONG	wb_ConfigLock		; Lock count of config...

    ULONG	wb_ToolExitCnt		; # of outstanding launched programs
    UWORD	wb_AppMenuItemCnt	; # of app menu items
    UBYTE	wb_shinePen		; shinePen for embosing icons
    UBYTE	wb_shadowPen		; shadowPen for embosing icons

    UBYTE	wb_EmboseBorderTop	; height of embose top border
    UBYTE	wb_EmboseBorderBottom	; height of embose bottom border
    UBYTE	wb_EmboseBorderLeft	; width of embose left border
    UBYTE	wb_EmboseBorderRight	; width of embose right border

    UBYTE	wb_APen			; foreground color for text
    UBYTE	wb_BPen			; background color for text
    UBYTE	wb_DrawMode		; draw mode for text

    ; the screen title
    STRUCT	wb_ScreenTitle,190
    STRUCT	wb_CurrentError,190

    STRUCT	wb_ExecuteBuf,WBBUFSIZE	; one line history for execute (Amiga-E)

    LABEL	WorkbenchBase_SIZEOF

;*********************************************************************
;*********************************************************************
;*********************************************************************

    ; flags in wb_Flags1
    BITDEF	WB1,unused_0,7		; Unused bit...
    BITDEF	WB1,Dragging,6		; on when dragging icon(s)
    BITDEF	WB1,DoubleClick,5	; we just double clicked
    BITDEF	WB1,unused_1,4		; Unused bit...
    BITDEF	WB1,InputTrashed,3	; Don't believe SELECTUP
    BITDEF	WB1,ErrorDisplayed,2	; ErrorTitle is up
    BITDEF	WB1,NameChanged,1	; NameChange win is up
    BITDEF	WB1,Closed,0		; all windows closed

    ; flags in wb_Flags2
    BITDEF	WB2,KPrintfOK,7		; OK to send kprintfs to serial port
    BITDEF	WB2,unused_2,6		; Unused bit...
    BITDEF	WB2,unused_3,5		; Unused bit...
    BITDEF	WB2,DragSelect,4	; drag selecting
    BITDEF	WB2,Quit,3		; quit WB
    BITDEF	WB2,Backdrop,2		; backdrop disk window is enabled
    BITDEF	WB2,StartWorkbench,1	; workbench has been started
    BITDEF	WB2,unused_4,0		; Unused bit...

    ; wb_Argument bits
    BITDEF	WBARG,DEBUGON,1		; turn on debugging menus

 STRUCTURE	ActiveDisk,0
    STRUCT	ad_Node,LN_SIZE
    UBYTE	ad_Active
    UBYTE	ad_Flags
    APTR	ad_Handler
    APTR	ad_Name
    APTR	ad_Object
    BPTR 	ad_Volume
    STRUCT	ad_Info,id_SIZEOF
    STRUCT	ad_CreateTime,ds_SIZEOF
    LABEL	ad_SIZEOF

; defines for ad_Flags
ADB_FILLBACKDROP	EQU	0	/* FillBackdrop has been called (bit posn) */
ADF_FILLBACKDROP	EQU	1	/* FillBackdrop has been called (flag) */

  STRUCTURE	CreateToolMsg,0
    STRUCT	ctm_Message,IOSTD_SIZE		; message structure itself
    APTR	ctm_Handler			; ptr to the cleanup function
    APTR	ctm_StartupMsg			; ptr to tool's startup message
    ULONG	ctm_Lock			; a lock for the tool
    APTR	ctm_Name			; ptr to tool's name
    ULONG	ctm_Priority			; tool's priority
    ULONG	ctm_StackSize			; tool's stacksize
    APTR	ctm_Cli				; ptr to wb cli (used to search path)
    BPTR	ctm_LoadLock			; Lock tool was loaded from
    LABEL	ctm_SIZEOF

	ENDC	WORKBENCHBASE_H

