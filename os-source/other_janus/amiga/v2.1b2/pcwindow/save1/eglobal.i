
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



   IFND   AZTEC
* If AZTEC isn't defined, use the Amiga standard declarations 

* === System Variables =================================================== */
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
   XREF _CursorTask,_InputTask
   XREF _SL_SuicideSignal
   XREF _CursorSuicideSignal
   XREF _InputSuicideSignal
   ENDC




   IFD   AZTEC
* If AZTEC is defined, use the Aztec non-Amiga-standard declarations

* === System Variables =================================================== */
   PUBLIC _IntuitionBase
   PUBLIC _GfxBase
   PUBLIC _LayersBase
   PUBLIC _JanusBase
   PUBLIC _DiskfontBase
   PUBLIC _NormalFont
   PUBLIC _UnderlineFont
   PUBLIC _FontData
   PUBLIC _GlobalKey
   PUBLIC _PCFont
   PUBLIC _AllClearPlane
   PUBLIC _AllSetPlane
   PUBLIC _AuxToolUsers
   PUBLIC _custom
   PUBLIC _CursorPriority
   PUBLIC _DisplayPriority
   PUBLIC _ColorIntenseIndex



* === Keyboard Events Variables ========================================== */
   PUBLIC _KeyBufferNextSend
   PUBLIC _KeyBufferNextSlot
   PUBLIC _KeyBuffer
   PUBLIC _PCWantsKey
   PUBLIC _PCRawKeyTable
   PUBLIC _KeyFlags



* === Assorted Task Variables ============================================= */
   PUBLIC _CursorTask,_InputTask
   PUBLIC _SL_SuicideSignal
   PUBLIC _CursorSuicideSignal
   PUBLIC _InputSuicideSignal

   ENDC


