
/* *** eglobal.c *************************************************************
 * 
 * This file contains the global external variable definitions
 * A copy of this file is kept in global.c, where the variables are
 * actually declared.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY   Name      Description
 * -------   ------      --------------------------------------------
 * 22 Feb 86   =RJ Mical=   Created this file
 *
 * **************************************************************************/


          
                         
/* === System Variables ==================================================== */

extern   struct   Preferences *SavePrefs;

extern   struct   IntuitionBase *IntuitionBase;
extern   struct   GfxBase *GfxBase;
extern   struct   LayersBase *LayersBase;
extern   struct   JanusBase *JanusBase;
extern   struct   JanusAmiga *JanusAmigaBA;
extern   struct   JanusAmiga *JanusAmigaWA;
/*extern   struct   DiskfontBase *DiskfontBase; */
extern   UBYTE   *NormalFont;
extern   UBYTE   *UnderlineFont;
extern   UBYTE   *FontData;
extern   struct   Remember *GlobalKey;
extern   SHORT   GlobalScreenHeight;
extern   UBYTE   MonoActiveTitle[];
extern   UBYTE   MonoInactiveTitle[];
extern   UBYTE   MonoFreezeTitle[];
extern   UBYTE   ColorActiveTitle[];
extern   UBYTE   ColorInactiveTitle[];
extern   UBYTE   ColorFreezeTitle[];
extern   UBYTE   ScreenTitle[];
extern   UBYTE   ScreenUnsharedTitle[];
extern   UBYTE   OldSidecarSettingsFile[];
extern   UBYTE   SidecarSettingsFile[];
extern   UBYTE   OldSidecarKeysFile[];
extern   UBYTE   SidecarKeysFile[];
extern   UBYTE   OldScanCodeFile[];
extern   UBYTE   ScanCodeFile[];
extern   struct   TextAttr PCFont;
extern   UBYTE   *AllSetPlane;
extern   UBYTE   *AllClearPlane;

extern   USHORT   TextOnePlaneColors[2];
extern   USHORT   TextTwoPlaneColors[4];
extern   USHORT   TextThreePlaneColors[8];
extern   USHORT   TextFourPlaneColors[16];
extern   USHORT   HighGraphicsColors[2];
extern   USHORT   LowGraphicsColors[2][2][4];

extern   struct   DisplayList ZaphodDisplay;
extern   struct   NewWindow NewWindow;
extern   struct   NewScreen NewScreen;
extern   struct   PropInfo VertInfo, HorizInfo;
extern   struct   Gadget VertGadget, HorizGadget;
extern   SHORT   AuxToolUsers;
extern   BOOL   AbandonBlink;
extern   BOOL   DeadenBlink;
extern   struct   SetupSig *PCKeySig;
extern   SHORT   CursorPriority;
extern   SHORT   DisplayPriority;
extern   SHORT   ColorIntenseIndex;
extern   struct   Menu MenuHeaders[MENU_HEADERS_COUNT];
extern   struct   TextAttr SafeFont;
extern   struct   MsgPort *AuxToolsFinalPort;
extern   struct   IntuiText OKText;
extern   SHORT   DefaultColorTextDepth;
extern   SHORT   LeftOfCloseGadget;
extern   SHORT   NullPointer[];
extern   LONG   (*KeyQueuer)();
/*???extern   SHORT   *PCMouseFlags;*/
/*???extern   SHORT   *PCMouseX;*/
/*???extern   SHORT   *PCMouseY;*/



/* === Keyboard Translation Stuff ====================================== */
extern   UBYTE   PCRawKeytable[];

extern   UBYTE   PCAltCode;
extern   UBYTE   PCCapsLockCode;
extern   UBYTE   PCCtrlCode;
extern   UBYTE   PCLeftShiftCode;
extern   UBYTE   PCNumLockCode;
extern   UBYTE   PCPlusCode;
extern   UBYTE   PCPtrScrCode;
extern   UBYTE   PCRightShiftCode;
extern   UBYTE   PCScrollLockCode;
extern   UBYTE   PCTildeCode;
extern   UBYTE   PCBarCode;
extern   UBYTE   PCBackDashCode;
extern   UBYTE   PCBackSlashCode;
extern   UBYTE   PCKeypad0Code;
extern   UBYTE   PCKeypad1Code;
extern   UBYTE   PCKeypad2Code;
extern   UBYTE   PCKeypad3Code;
extern   UBYTE   PCKeypad4Code;
extern   UBYTE   PCKeypad5Code;
extern   UBYTE   PCKeypad6Code;
extern   UBYTE   PCKeypad7Code;
extern   UBYTE   PCKeypad8Code;
extern   UBYTE   PCKeypad9Code;
extern   UBYTE   PCKeypadDotCode;

extern   UBYTE   AmigaCapsLockCode;
extern   UBYTE   AmigaLeftAltCode;
extern   UBYTE   AmigaNCode;
extern   UBYTE   AmigaPCode;
extern   UBYTE   AmigaPlusCode;
extern   UBYTE   AmigaRightAltCode;
extern   UBYTE   AmigaSCode;
extern   UBYTE   AmigaPrtScrCode;
extern   UBYTE   AmigaTildeBackDash;
extern   UBYTE   AmigaBarBackSlash;
extern   UBYTE   AmigaCursorUp;
extern   UBYTE   AmigaCursorLeft;
extern   UBYTE   AmigaCursorRight;
extern   UBYTE   AmigaCursorDown;
extern   UBYTE   AmigaDelCode;



/* === Keyboard Events Variables ======================================== */
extern   SHORT   KeyBufferNextSend;
extern   SHORT   KeyBufferNextSlot;
extern   UBYTE   KeyBuffer[];
extern   BOOL   PCWantsKey;
extern   UBYTE   PCRawKeyTable[];
extern   SHORT   KeyFlags;



/* === Clip Area Variables =============================================== */
extern   SHORT   ClipStartX, ClipStartY;
extern   SHORT   ClipCurrentX, ClipCurrentY;



/* === Assorted Task Variables ============================================= */
extern   struct   Task *CursorTask, *InputTask, *MainTask;
extern   ULONG   SL_SuicideSignal;
extern   ULONG   CursorSuicideSignal;
extern   ULONG   InputSuicideSignal;

extern   SHORT   TopBorderOn;
extern   SHORT   TopBorderOff;
/*BBBextern   SHORT   BottomBorderOn;*/
/*BBBextern   SHORT   BottomBorderOff;*/

