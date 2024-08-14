;******************************************************************************
*
* $Id: quotes.asm,v 39.3 93/02/15 15:06:33 mks Exp $
*
* $Log:	quotes.asm,v $
* Revision 39.3  93/02/15  15:06:33  mks
* Cleaned up and added what was needed
* 
* Revision 39.2  93/02/15  14:44:15  mks
* Added some more global strings
*
* Revision 39.1  93/01/11  16:53:30  mks
* Now supports a string filter for the RENAME requester...
* No longer needs to check for ":" or "/" after the fact.
*
* Revision 38.13  92/05/30  15:54:41  mks
* Added a label on the physical string called Information
*
* Revision 38.12  92/05/12  16:11:48  mks
* Added upper case %d to the value display formats for locale and
* the locale format...
* Requires the V39 EXEC now...
*
* Revision 38.11  92/05/11  18:40:19  mks
* Added "?" as default keyboard shortcut for About...
*
* Revision 38.10  92/03/18  16:09:00  mks
* Changed the case of some of the words...
* Also added "." as keyboard shortcut on CleanUp
*
* Revision 38.9  92/03/10  10:41:44  mks
* Added support for the NULL string returns
*
* Revision 38.8  92/02/18  14:54:50  mks
* Fixed cataloge closing
*
* Revision 38.7  92/02/17  19:26:05  mks
* Moved some quotes from this file to the abs.asm file...
*
* Revision 38.6  92/02/17  15:40:42  mks
* Fixed the comments for VersionFormat such that it does not look like
* an autodoc.
*
* Revision 38.5  92/01/21  13:59:51  mks
* Updated the Copyright date...
*
* Revision 38.4  92/01/07  13:53:25  mks
* Removed strings not needed...
*
* Revision 38.3  91/11/12  18:44:57  mks
* Changed the way locale works in that it always shuts down completely
* between catalogs.  (More localized changes and better support for
* dynamic language changes)
*
* Revision 38.2  91/09/12  11:39:58  mks
* Added all of the support to handle Locale.library directly...
*
* Revision 38.1  91/06/24  11:37:59  mks
* Initial V38 tree checkin - Log file stripped
*
;******************************************************************************
;
; The strings stored here are stored in a set such that they are separated
; by an individual NULL byte.  If the string is just a NULL byte, it will
; be returned as a NULL pointer.
;
; Other strings usefull in the system...
;
	XDEF	_NumbersString
	XDEF	_MM_String
	XDEF	_M_String
	XDEF	_OutputWindow1
	XDEF	_OutputWindow2
	XDEF	_TopazName
	XDEF	_DiskIconName
	XDEF	_InfoSuf
	XDEF	_SystemDiskCopyName
	XDEF	_SystemFormatName
	XDEF	_SystemWorkbenchName
	XDEF	_InformationString
	XDEF	_CopyTaskName

;	xref	_LVOGetCatalogStr
;	xref	_LVOOpenCatalogA
;	xref	_LVOCloseCatalog
*
* These are here just until the amiga.lib and includes are updated for V38
* and locale.library
*
_LVOCloseCatalog	EQU	-$24
_LVOGetCatalogStr	EQU	-$48
_LVOOpenCatalogA	EQU	-$96

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"workbench.i"
	INCLUDE	"workbenchbase.i"

******i workbench.library/QuoteWorkbench *************************************
*
*   NAME
*	QuoteWorkbench - Return a text string from workbench
*
*   SYNOPSIS
*	string = QuoteWorkbench(num)
*         D0                    D0
*	char *QuoteWorkbench(ULONG);
*
*   FUNCTION
*	Return a pointer to the ASCII text string based on the string
*	number given.  This function is private and is here such that
*	strings may be replaced via a SetFunction() type of feature in
*	a special version of LoadWB.
*
*   INPUTS
*	num - Text string ID number
*
*   RESULTS
*	NULL if the string is empty.  (ie: "")
*	Pointe to a standard C string.
*
*   SEE ALSO
*
*   BUGS
*	This command does *NO* error checking and this string numbers
*	outside the range may cause unpredictable results.  This is an
*	internal-use only function and thus should not be a problem.
*
******************************************************************************
*
* Code to return the strings in workbench...
*
	SECTION	code,code
	XDEF	GetQuote
GetQuote:
	move.l	d0,a1			; Save number in a1...
	lea	Quotes,a0		; Point at my strings...
Loop:	tst.b	(a0)+			; Test the next byte...
	bne.s	Loop			; Loop back until we find a null...
	dbra	d0,Loop			; Continue until we get our string...
	move.l	a0,d0			; Get address of string to return...
	move.l	wb_Catalog(a6),d1	; Get the catalog...
	beq.s	Found			; If we don't have one, we are done
	move.l	d1,a0			; Put catalog into a0...
	exg	d0,a1			; Put string in a1 and number in d0
	move.l	a6,-(sp)		; Save WB Base
	move.l	wb_LocaleBase(a6),a6	; Get locale base
	jsr	_LVOGetCatalogStr(a6)	; Get the catalog string
	move.l	(sp)+,a6		; Restore WB Base
*
* Now check for NULL string
*
Found:	move.l	d0,a0			; Get string pointer
	move.b	(a0),d1			; Is it NULL?
	bne.s	Done			; If not, we exit
	moveq.l	#0,d0			; If NULL string, we return NULL...
Done:	rts				; Return with D0 set...
*
******************************************************************************
*
* These routine are the catalog quoting routines...
*
		xref	_OpenLibraryStub
_OpenWBCat:	xdef	_OpenWBCat
		clr.l	-(sp)			; Any version...
		pea	LocName			; Point at string...
		bsr	_OpenLibraryStub	; Call open library
		addq.l	#8,sp			; Restore the stack...
		move.l	d0,wb_LocaleBase(a6)	; Store it...
		beq.s	OpenDone		; Not open, so exit...
GotLocale:	movem.l	a6/a2,-(sp)		; Save a6 and a2
		sub.l	a0,a0			; Clear a0
		move.l	a0,a2			; Clear a2
		lea	CatName,a1		; Get catalog name
		move.l	d0,a6			; Get locale base
		jsr	_LVOOpenCatalogA(a6)	; Open the catalog...
		movem.l	(sp)+,a6/a2		; Restore a6/a2
OpenDone:	move.l	d0,wb_Catalog(a6)	; Store result
		rts
*
******************************************************************************
*
		xref	_CloseLibraryStub
_CloseWBCat:	xdef	_CloseWBCat
		move.l	wb_Catalog(a6),d1	; Check for catalog...
		beq.s	CloseDone		; No catalog...
		move.l	a6,-(sp)		; Save WB base...
		move.l	wb_LocaleBase(a6),a6	; Get locale base...
		move.l	d1,a0			; Get catalog
		jsr	_LVOCloseCatalog(a6)	; Close the catalog...
		move.l	(sp)+,a6		; Get WB base back...
CloseDone:	clr.l	wb_Catalog(a6)		; Clear catalog pointer...
		move.l	wb_LocaleBase(a6),d0	; Get locale base...
		beq.s	CloseDone2		; If no locale base...
		move.l	d0,-(sp)		; Set up for call
		bsr	_CloseLibraryStub	; Close the library
		addq.l	#4,sp			; Clean up stack
CloseDone2:	clr.l	wb_LocaleBase(a6)	; Clear locale base...
		rts
*
******************************************************************************
*
;
	SECTION	data,data
;
; These are external definitions used elsewhere in the code...
;
LocName:		dc.b	'locale.library',0
CatName:		dc.b	'sys/workbench.catalog',0
_NumbersString:		dc.b	'123,456,789'	; Continues...
_MM_String:		dc.b	'm'		; below...
_M_String:		dc.b	'm',0		; for shared strings...
_OutputWindow1		dc.b	'CON:0/25/640/150/',0
_OutputWindow2		dc.b	'/AUTO/WAIT',0
_DiskIconName:		dc.b	'Disk',0
_InfoSuf:		dc.b	'.info',0
_TopazName:		dc.b	'topaz.font',0
_SystemDiskCopyName:	dc.b	'SYS:System/Diskcopy',0
_SystemFormatName:	dc.b	'SYS:System/Format'		; 0 is below
Quotes:
 dc.b 0		; Start of quote table...
;
; ErrorTitle() strings
;
 dc.b "This drawer is not really a directory",0			; Q_DRW_IS_NOT_A_DIR
 dc.b "Cannot rename this disk",0				; Q_CANT_RENAME_DISK
 dc.b "This drawer does not have a parent to open",0		; Q_DRW_HAS_NO_PARENT
 dc.b "Icons cannot be moved into this window",0		; Q_ICON_CANT_B_IN_WIN
 dc.b "'%s' cannot be moved out of its window",0		; Q_CANT_MOV_OUT_OF_WIN
 dc.b "Cannot copy '%s' to itself",0				; Q_CANT_COPY_TO_ITSELF
 dc.b "Cannot diskcopy, source disk is not a DOS disk",0	; Q_CANT_DISKCOPY_SRC
 dc.b "Info failed.",0						; Q_INFO_FAILED
 dc.b 0	; Obsolete: "Do not use '%lc' in names",0		; Q_DO_NOT_USE_IN_NAMES
 dc.b "Ran out of memory.  Please free some and try again.",0	; Q_OUT_OF_MEM
 dc.b "The icon(s) have no default tool",0			; Q_ICON_HAS_NO_DEF_TOOL
 dc.b "Cannot launch this program",0				; Q_CANT_LAUNCH_PROGRAM
 dc.b "Not enough memory to run '%s'",0				; Q_NO_MEM_TO_RUN
 dc.b "Unable to open your tool '%s'",0				; Q_CANT_OPEN_TOOL
 dc.b "This drawer cannot be opened",0				; Q_DRW_CANT_BE_OPENED

; format strings for errors while WB was doing some DOS thing
 dc.b "Error while %s: (%ld) %s",0				; Q_ERROR_WHILE
 dc.b "examining drawer",0					; Q_EXAMINING_DIR
 dc.b "accessing %s",0						; Q_ACCESSING_STRING
 dc.b "opening %s",0						; Q_OPENING_STRING
 dc.b "reading",0						; Q_READING
 dc.b "writing",0						; Q_WRITING
 dc.b "moving %s",0						; Q_MOVING_STRING
 dc.b "removing %s",0						; Q_REMOVING_STRING
 dc.b "examining %s",0						; Q_EXAMINING_STRING
 dc.b "writing %s",0						; Q_WRITING_STRING
 dc.b "creating %s",0						; Q_CREATING_STRING

; format string for workbench screen title
 dc.b "Amiga Workbench  %lD graphics mem  %lD other mem",0	; Q_SCREEN_TITLE

; The string "version" used in the strings "Kickstart version 36.1" etc.
 dc.b "version",0						; Q_VERSION_TITLE

; misc. requester(s) strings
 dc.b "Retry",0							; Q_RETRY_TEXT
 dc.b "Cancel",0						; Q_CANCEL_TEXT
 dc.b "Ok",0							; Q_OK_TEXT
 dc.b "Save",0							; Q_SAVE_TEXT
 dc.b "Ok|Cancel",0						; Q_OK_CANCEL_TEXT
 dc.b "Retry|Cancel",0						; Q_RETRY_CANCEL_TEXT
 dc.b "Remove|Cancel",0						; Q_REMOVE_CANCEL_TEXT
 dc.b "Replace|Replace All|Cancel",0				; Q_REPLACE_CANCEL_TEXT
 dc.b "Information",0						; Q_INFO_TITLE

; protection bit descriptions (used by Info)
 dc.b "Script",0						; Q_SCRIPT_BIT
 dc.b "Pure",0							; Q_PURE_BIT
 dc.b "Archived",0						; Q_ARCHIVED_BIT
 dc.b "Deletable",0						; Q_DELETE_BIT
 dc.b "Readable",0						; Q_READ_BIT
 dc.b "Writable",0						; Q_WRITE_BIT
 dc.b "Executable",0						; Q_EXECUTE_BIT

; various strings used by Info
 dc.b "Stack:",0						; Q_STACK
 dc.b "Volume:",0						; Q_VOLUME
 dc.b "Blocks:",0						; Q_BLOCKS
 dc.b "Used:",0							; Q_USED
 dc.b "Free:",0							; Q_FREE
 dc.b "Block size:",0						; Q_BLOCK_SIZE
 dc.b "Bytes:",0						; Q_BYTES
 dc.b "Last Changed:",0						; Q_LAST_CHANGED
 dc.b "Created:",0						; Q_CREATED
 dc.b "Comment:",0						; Q_COMMENT
 dc.b "Default Tool:",0						; Q_DEFAULT_TOOL
 dc.b "Tool Types:",0						; Q_TOOL_TYPES

; volume string and volume status (in Info)
 dc.b "Volume is",0						; Q_VOLUME_TEXT
 dc.b "Read Only",0						; Q_VOL_IS_READ_ONLY
 dc.b "Validating",0						; Q_VOL_IS_VALIDATING
 dc.b "Read/Write",0						; Q_VOL_IS_READ_WRITE
 dc.b "???",0							; Q_VOL_IS_UNKNOWN

; ToolTypes gadget strings (used in Info)
 dc.b "New",0							; Q_NEW
 dc.b "Del",0							; Q_DELETE

; icon types (used in Info)
 dc.b "Volume",0						; Q_IDISK
 dc.b "Drawer",0						; Q_IDRAWER
 dc.b "Tool",0							; Q_ITOOL
 dc.b "Project",0						; Q_IPROJECT
 dc.b "Trashcan",0						; Q_ITRASHCAN

; file type when in View By Name/Date/Size
; length MUST NOT change!  (Must be 10 characters)
 dc.b "    Drawer",0						; Q_WBDRAWER

; Start of workbench menu strings
 dc.b 0								; Q_END_MENU
 dc.b 0								; Q_END_MENU_KEY

_SystemWorkbenchName:	; This one is used elsewhere too...
 dc.b "Workbench",0						; Q_WORKBENCH_MENU
 dc.b 0								; Q_WORKBENCH_MENU_KEY
 dc.b "Backdrop",0						; Q_FLIP_WB_MENU
 dc.b "B",0							; Q_FLIP_WB_MENU_KEY
 dc.b "Execute Command...",0					; Q_EXECUTE_MENU
 dc.b "E",0							; Q_EXECUTE_MENU_KEY
 dc.b "Redraw All",0						; Q_REDRAW_ALL_MENU
 dc.b 0								; Q_REDRAW_ALL_MENU_KEY
 dc.b "Update All",0						; Q_RESCAN_ALL_MENU
 dc.b 0								; Q_RESCAN_ALL_MENU_KEY
 dc.b "Last Message",0						; Q_LAST_ERROR_MENU
 dc.b 0								; Q_LAST_ERROR_MENU_KEY
 dc.b "About...",0						; Q_VERSION_MENU
 dc.b "?",0							; Q_VERSION_MENU_KEY
 dc.b "Quit...",0						; Q_QUIT_MENU
 dc.b "Q",0							; Q_QUIT_MENU_KEY

 dc.b "Window",0						; Q_WINDOW_MENU
 dc.b 0								; Q_WINDOW_MENU_KEY
 dc.b "New Drawer",0						; Q_NEW_DRAWER_MENU
 dc.b "N",0							; Q_NEW_DRAWER_MENU_KEY
 dc.b "Open Parent",0						; Q_PARENT_MENU
 dc.b 0								; Q_PARENT_MENU_KEY
 dc.b "Close",0							; Q_CLOSE_MENU
 dc.b "K",0							; Q_CLOSE_MENU_KEY
 dc.b "Update",0						; Q_RESCAN_MENU
 dc.b 0								; Q_RESCAN_MENU_KEY
 dc.b "Select Contents",0					; Q_SELECT_ALL_MENU
 dc.b "A",0							; Q_SELECT_ALL_MENU_KEY
 dc.b "Clean Up",0						; Q_CLEAN_UP_MENU
 dc.b ".",0							; Q_CLEAN_UP_MENU_KEY
 dc.b "Snapshot",0						; Q_SNAPSHOT_MENU
 dc.b 0								; Q_SNAPSHOT_MENU_KEY
 dc.b "Show",0							; Q_SHOW_MENU
 dc.b 0								; Q_SHOW_MENU_KEY
 dc.b "View By",0						; Q_VIEW_BY_MENU
 dc.b 0								; Q_VIEW_BY_MENU_KEY

 dc.b "Icons",0							; Q_ICON_MENU
 dc.b 0								; Q_ICON_MENU_KEY
 dc.b "Open",0							; Q_OPEN_MENU
 dc.b "O",0							; Q_OPEN_MENU_KEY
_CopyTaskName:
 dc.b "Copy",0							; Q_COPY_MENU
 dc.b "C",0							; Q_COPY_MENU_KEY
 dc.b "Rename...",0						; Q_RENAME_MENU
 dc.b "R",0							; Q_RENAME_MENU_KEY
_InformationString:
 dc.b "Information...",0					; Q_INFO_MENU
 dc.b "I",0							; Q_INFO_MENU_KEY
 dc.b "Snapshot",0						; Q_SNAPSHOTI_MENU
 dc.b "S",0							; Q_SNAPSHOTI_MENU_KEY
 dc.b "UnSnapshot",0						; Q_UNSNAPSHOT_MENU
 dc.b "U",0							; Q_UNSNAPSHOT_MENU_KEY
 dc.b "Leave Out",0						; Q_LEAVE_OUT_MENU
 dc.b "L",0							; Q_LEAVE_OUT_MENU_KEY
 dc.b "Put Away",0						; Q_PUT_AWAY_MENU
 dc.b "P",0							; Q_PUT_AWAY_MENU_KEY
 dc.b 0								; Q_SEPARATOR_MENU
 dc.b 0								; Q_SEPARATOR_MENU_KEY
 dc.b "Delete...",0						; Q_DISCARD_MENU
 dc.b 0								; Q_DISCARD_MENU_KEY
 dc.b "Format Disk...",0					; Q_FORMAT_DISK_MENU
 dc.b 0								; Q_FORMAT_DISK_MENU_KEY
 dc.b "Empty Trash",0						; Q_EMPTY_TRASH_MENU
 dc.b 0								; Q_EMPTY_TRASH_MENU_KEY

 dc.b "Tools",0							; Q_TOOLS_MENU
 dc.b 0								; Q_TOOLS_MENU_KEY
 dc.b "ResetWB",0						; Q_RESETWB_MENU
 dc.b 0								; Q_RESETWB_MENU_KEY

; sub-menu item strings for 'Snapshot'
 dc.b "Window",0						; Q_SNAPSHOT_WINDOW_MENU
 dc.b 0								; Q_SNAPSHOT_WINDOW_MENU_KEY
 dc.b "All",0							; Q_SNAPSHOT_ALL_MENU
 dc.b 0								; Q_SNAPSHOT_ALL_MENU_KEY

; sub-menu item strings for 'View By '
 dc.b "Icon",0							; Q_VIEW_BY_ICON_MENU
 dc.b 0								; Q_VIEW_BY_ICON_M_KEY
 dc.b "Name",0							; Q_VIEW_BY_NAME_MENU
 dc.b 0								; Q_VIEW_BY_NAME_M_KEY
 dc.b "Date",0							; Q_VIEW_BY_DATE_MENU
 dc.b 0								; Q_VIEW_BY_DATE_M_KEY
 dc.b "Size",0							; Q_VIEW_BY_SIZE_MENU
 dc.b 0								; Q_VIEW_BY_SIZE_M_KEY

; sub-menu item strings for 'Show'
 dc.b "Only Icons",0						; Q_SHOW_ONLY_ICONS_MENU
 dc.b 0								; Q_SHOW_ONLY_ICONS_MK
 dc.b "All Files",0						; Q_SHOW_ALL_FILES_MENU
 dc.b 0								; Q_SHOW_ALL_FILES_MK
; end of workbench menu strings

								; Q_DISCARD_REQ_TEXT
 dc.b "Warning: you cannot get back",10,"what you delete!  Ok to delete:",10,10,"%lD file(s) and",10,"%lD drawer(s) (and their contents)?",0

; Quit wb requester strings
 dc.b "Do you really want",10,"to quit workbench?",0		; Q_QUIT_REQ_TEXT

; this is the name 'New Drawer' uses for the new drawer
 dc.b "Unnamed",0						; Q_NEW_DRAWER_NAME

; format string for window title of disks (volumes)
 dc.b "  %ld%% full, %lD%lc free, %lD%lc in use",0		; Q_VOLUME_TITLE_FORMAT

; Rename requester strings
 dc.b "Enter a new name for '%s'.",0				; Q_RENAME_GREETING
 dc.b "Rename",0						; Q_RENAME_REQ_TITLE
 dc.b "New Name:",0						; Q_RENAME_LABEL
 dc.b "Rename failed.",0					; Q_RENAME_FAILED

; misc. entries
 dc.b "Enter Command Arguments:",0				; Q_EXECUTE_CMD
 dc.b "Enter Command and its Arguments:",0			; Q_EXECUTE_CMD_AN_ARGS
 dc.b "Execute Failed",0					; Q_EXECUTE_FAILED
 dc.b "Output Window",0						; Q_WB_OUTPUT_TITLE
 dc.b "Command:",0						; Q_COMMAND_TEXT
 dc.b "Execute a File",0					; Q_EXECUTE_REQ_TITLE
 dc.b "Cannot Quit yet, there are %lD WB launched program(s)",0	; Q_TOOL_EXIT
 dc.b "Cannot Quit yet, open count = %lD",0			; Q_LIBRARY_OPEN

 dc.b "Cannot 'Leave Out' this icon",0				; Q_CANNOT_LEAVE_OUT
 dc.b "Cannot 'Put Away' this icon",0				; Q_CANNOT_PUT_AWAY

 dc.b "Disk is busy, cannot format",0				; Q_CANNOT_FORMAT

 dc.b "This icon has already been left out",0			; Q_ALREADY_LEFT_OUT

 dc.b "Cannot rename a left out icon",0				; Q_RENAME_LEFTOUT

 dc.b "Icon '%s' cannot be deleted",0				; Q_CANNOT_DELETE

 dc.b "Attempting to load program '%s'...",0			; Q_LOADING

								; Q_STARTUP_WAIT
 dc.b "Program '%s'",10,"has not yet returned.",10,"Should I wait some more?",0

; Copy error texts...
 dc.b "Error while copying '%s'",10,"%s%s",0			; Q_COPY_ERROR_TEXT
 dc.b 10,10,"Remove incomplete object?",0			; Q_COPY_DELETE_TEXT
 dc.b "Cannot copy '%s'",0					; Q_CANT_COPY

;
; * Moved the debug stuff to the end such that it can be removed
; * via a simple change...
;

 dc.b "Debug",0							; Q_DEBUG_MENU
 dc.b 0								; Q_DEBUG_MENU_KEY
 dc.b "ROMWack",0						; Q_ROMWACK_MENU
 dc.b 0								; Q_ROMWACK_MENU_KEY
 dc.b "flushlibs",0						; Q_FLUSHLIBS_MENU
 dc.b 0								; Q_FLUSHLIBS_MENU_KEY

	END
