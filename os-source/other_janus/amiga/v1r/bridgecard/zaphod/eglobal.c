
/* ***************************************************************************
 * 
 * This file contains the global external variable definitions
 * A copy of this file is kept in global.c, where the variables are
 * actually declared.
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 22 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/


	       
						       
/* === System Variables ==================================================== */
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct LayersBase *LayersBase;
extern struct JanusBase *JanusBase;
extern struct DiskfontBase *DiskfontBase;
extern UBYTE *NormalFont;
extern UBYTE *UnderlineFont;
extern UBYTE *FontData;
extern struct Remember *GlobalKey;
extern UBYTE MonoActiveTitle[];
extern UBYTE MonoInactiveTitle[];
extern UBYTE ColorActiveTitle[];
extern UBYTE ColorInactiveTitle[];
extern UBYTE SidecarSettingsFile[];
extern UBYTE SidecarKeysFile[];
extern struct TextAttr PCFont;
extern UBYTE AllSetPlane[80 * CHAR_HEIGHT];
extern UBYTE AllClearPlane[80 * CHAR_HEIGHT];

extern USHORT TextOnePlaneColors[2];
extern USHORT TextTwoPlaneColors[4];
extern USHORT TextThreePlaneColors[8];
extern USHORT TextFourPlaneColors[16];
extern USHORT HighGraphicsColors[2];
extern USHORT LowGraphicsColors[2][2][4];

extern struct DisplayList ZaphodDisplay;
extern struct NewWindow NewWindow;
extern struct NewScreen NewScreen;
extern struct PropInfo VertInfo, HorizInfo;
extern struct Gadget VertGadget, HorizGadget;
extern SHORT AuxToolUsers;
extern BOOL AbandonBlink;
extern struct SetupSig *PCKeySig;
extern SHORT CursorPriority;
extern SHORT DisplayPriority;
extern SHORT ColorIntenseIndex;
extern struct Menu MenuHeaders[MENU_HEADERS_COUNT];
extern struct TextAttr SafeFont;
extern struct MsgPort *AuxToolsFinalPort;
extern struct IntuiText OKText;
extern SHORT DefaultColorTextDepth;




/* === Keyboard Translation Stuff ====================================== */
extern BYTE PCRawKeytable[];
extern BYTE PCAltCode;
extern BYTE PCCapsLockCode;
extern BYTE PCCtrlCode;
extern BYTE PCLeftShiftCode;
extern BYTE PCNumLockCode;
extern BYTE PCPlusCode;
extern BYTE PCPtrScrCode;
extern BYTE PCRightShiftCode;
extern BYTE PCScrollLockCode;

extern BYTE AmigaCapsLockCode;
extern BYTE AmigaLeftAltCode;
extern BYTE AmigaNCode;
extern BYTE AmigaPCode;
extern BYTE AmigaPlusCode;
extern BYTE AmigaRightAltCode;
extern BYTE AmigaSCode;
	    




/* === Keyboard Events Variables ======================================== */
extern SHORT KeyBufferNextSend;
extern SHORT KeyBufferNextSlot;
extern UBYTE KeyBuffer[KEYBUFFER_SIZE];
extern BOOL PCWantsKey;
extern UBYTE PCRawKeyTable[];
extern SHORT KeyFlags;



/* === Assorted Task Variables ============================================= */
extern struct Task *CursorTask, *InputTask, *SL_Task;
extern ULONG SL_SuicideSignal;
extern ULONG CursorSuicideSignal;
extern ULONG InputSuicideSignal;




