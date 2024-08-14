**
**	$Header: Work:src/cerial/RCS/device_abs.asm,v 1.1 91/08/09 21:06:12 bryce Exp $
**
**	Generic device driver code: Startup and ROMTag
**
**	(C) Copyright 1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION main,CODE

;---------- Includes --------------------------------------------------------
	IFND	PREASSEMBLED_INCLUDES
	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/initializers.i"
	LIST
	ENDC

	INCLUDE "device_base.i"

;---------- Exports ---------------------------------------------------------
	XDEF funcTable
	XDEF dataTable
	XDEF deviceName

;---------- Domestic imports ------------------------------------------------
	XREF Device_ColdInit
	XREF Device_Open
	XREF Device_Close
	XREF Device_Expunge
	XREF Device_Null

;---------- Foreign imports -------------------------------------------------
	XREF _MD_BeginIO
	XREF _MD_AbortIO



;----------------------------------------------------------------------------
; This is the first executable location.  This must return -1 so that
; we will not crash if executed directly.
;
		IFNE	DISK_DEVICE
returnNeg1:
		moveq	#-1,d0	    ;dump out if directly executed
		rts		    ;-1 indicates "not really an executable"
		ENDC


;----------------------------------------------------------------------------
; This is a "ROMTag".  The system will search memory or disk for the magic
; cookie (RT_MATCHWORD = $4AFC).
; If found, the ROMTag will be initialized and made available to the
; system.  See exec/resident.i
;
initTable:	dc.w	RTC_MATCHWORD	;Magic cookie
		dc.l	initTable	;back pointer
		dc.l	endMarker	;Must point forward & in same hunk
		dc.b	RTF_AUTOINIT
		dc.b	VERSION
		dc.b	NT_DEVICE
		dc.b	ROMTAG_PRI
		dc.l	deviceName
		dc.l	idText
		dc.l	Device_Init_Table
endMarker:


;----------------------------------------------------------------------------
; Text area
;
deviceName:	DEVICE_NAME
idText: 	DEVICE_ID
		CNOP 0,2


;----------------------------------------------------------------------------
; Table for RTF_AUTOINIT type ROMTags.
;
Device_Init_Table:
		dc.l	MultiDev_SIZE	;size of library base
		dc.l	funcTable	;function for library base
		dc.l	dataTable	;init table for library base
		dc.l	Device_ColdInit ;callback point


;----------------------------------------------------------------------------
; Long format table of library vector entries
;
funcTable:	dc.l Device_Open
		dc.l Device_Close
		dc.l Device_Expunge
		dc.l Device_Null
		dc.l _MD_BeginIO
		dc.l _MD_AbortIO
		dc.l -1 	    ;Long format end marker


;----------------------------------------------------------------------------
;Initialization for the libray base data area.	This will be passed
;to exec/InitStruct().
;
dataTable:	INITBYTE LN_TYPE,NT_DEVICE
		INITLONG LN_NAME,deviceName
		INITBYTE LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD LIB_VERSION,VERSION
		INITWORD LIB_REVISION,REVISION
		INITLONG LIB_IDSTRING,idText
		dc.w	 0

		END
