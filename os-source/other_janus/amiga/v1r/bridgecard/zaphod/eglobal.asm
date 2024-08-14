
* ***************************************************************************
* 
* This file contains the global external variable definitions
* A copy of this file is kept in global.c, where the variables are
* actually declared.
*
* Copyright (C) 1986, Commodore-Amiga, Inc.
* 
* CONFIDENTIAL and PROPRIETARY
*
* **************************************************************************/




* === System Variables ==================================================== */
	XREF _IntuitionBase
	XREF _GfxBase
	XREF _LayersBase
	XREF _JanusBase
	XREF _DiskfontBase
	XREF _NormalFont
	XREF _UnderlineFont
	XREF _FontData
	XREF _GlobalKey
	XREF _PCFont
	XREF _AllClearPlane
	XREF _AllSetPlane
	XREF _AuxToolUsers
	XREF _custom
	XREF _CursorPriority
	XREF _DisplayPriority
	XREF _ColorIntenseIndex



* === Keyboard Events Variables ========================================== */
	XREF _KeyBufferNextSend
	XREF _KeyBufferNextSlot
	XREF _KeyBuffer
	XREF _PCWantsKey
	XREF _PCRawKeyTable
	XREF _KeyFlags



* === Assorted Task Variables ============================================= */
	XREF _CursorTask,_InputTask,_SL_Task
	XREF _SL_SuicideSignal
	XREF _CursorSuicideSignal
	XREF _InputSuicideSignal



