ÄàINTUITION_INTUITIONBASE_HÄINTUITION_INTUITIONBASE_HàEXEC_TYPES_Hå<exec/types.h>áàEXEC_LIBRARIES_Hå<exec/libraries.h>áàINTUITION_INTUITION_Hå<intuition/intuition.h>áàEXEC_INTERRUPTS_Hå<exec/interrupts.h>á∞INTUITIONPRIVATEÄDMODECOUNT 2ÄHIRESPICK 0ÄLOWRESPICK 1ÄEVENTMAX 10ÄRESCOUNT 2ÄHIRESGADGET 0ÄLOWRESGADGET 1ÄGADGETCOUNT 8ÄUPFRONTGADGET 0ÄDOWNBACKGADGET 1ÄSIZEGADGET 2ÄCLOSEGADGET 3ÄDRAGGADGET 4ÄSUPFRONTGADGET 5ÄSDOWNBACKGADGET 6ÄSDRAGGADGET 7ÄISTATELOCK 0ÄLAYERINFOLOCK 1ÄGADGETSLOCK 2ÄLAYERROMLOCK 3ÄVIEWLOCK 4ÄIBASELOCK 5ÄRPLOCK 6ÄNUMILOCKS 7
ÉFatIntuiMessage{
ÉIntuiMessage;
óPrevKeys;
};
ÉIBox{
ïLeft;
ïTop;
ïWidth;
ïHeight;
};
ÉPoint{
ïX;
ïY;
};
ÉPenPair{
äDetailPen;
äBlockPen;
};
ÉGListEnv{
ÉScreen*ge_Screen;
ÉWindow*ge_Window;
ÉRequester*ge_Requester;
ÉRastPort*ge_RastPort;
ÉLayer*ge_Layer;
ÉLayer*ge_GZZLayer;
ÉPenPair ge_Pens;
ÉIBox ge_Domain;
ÉIBox ge_GZZdims;
};
ÉGadgetInfo{
ÉGListEnv*gi_Environ;
Éª*gi_Gadget;
ÉIBox gi_Box;
ÉIBox gi_Container;
ÉLayer*gi_Layer;
ÉIBox gi_NewKnob;
};á
Éñ
{
ÉLibrary LibNode;
ÉView ViewLord;
ÉWindow*ActiveWindow;
ÉScreen*ActiveScreen;
ÉScreen*FirstScreen;
ó¶;
òMouseY,MouseX;
óSeconds;
óMicros;∞INTUITIONPRIVATE
òMinXMouse,MaxXMouse;
òMinYMouse,MaxYMouse;
óStartSecs,StartMicros;
îSysBase;
Éè*è;
îß;
îConsoleDevice;
ô*APointer;
öAPtrHeight;
öAPtrWidth;
öAXOffset,AYOffset;
ôMenuDrawn;
ôMenuSelected;
ôOptionList;
ÉRastPort MenuRPort;
ÉTmpRas MenuTmpRas;
ÉClipRect ItemCRect;
ÉClipRect SubCRect;
ÉBitMap IBitMap;
ÉBitMap SBitMap;
ÉIOStdReq InputRequest;
ÉInterrupt InputInterrupt;
ÉRemember*EventKey;
ÉInputEvent*IEvents;ÄNUM_IEVENTS 4
ïEventCount;
ÉInputEvent IEBuffer[NUM_IEVENTS];
Éª*ActiveGadget;
ÉPropInfo*ActivePInfo;
ÉImage*ActiveImage;
ÉGListEnv GadgetEnv;
ÉGadgetInfo GadgetInfo;
ÉPoint KnobOffset;
ÉWindow*getOKWindow;
ÉIntuiMessage*getOKMessage;
ôsetWExcept,GadgetReturn,StateReturn;
ÉRastPort*RP;
ÉTmpRas ITmpRas;
ÉRegion*OldClipRegion;
ÉPoint OldScroll;
ÉIBox IFrame;
ïhthick,vthick;
VOID(*frameChange)();
VOID(*sizeDrag)();
ÉPoint FirstPt;
ÉPoint OldPt;
Éª*SysGadgets[RESCOUNT][GADGETCOUNT];
ÉImage*CheckImage[RESCOUNT],*AmigaIcon[RESCOUNT];∞OLDPATTERN
ôapattern[3],bpattern[4];ù
ôapattern[8],bpattern[4];á
ô*IPointer;
öIPtrHeight;
öIPtrWidth;
öIXOffset,IYOffset;
íDoubleSeconds,DoubleMicros;
öWBorLeft[DMODECOUNT];
öWBorTop[DMODECOUNT];
öWBorRight[DMODECOUNT];
öWBorBottom[DMODECOUNT];
öBarVBorder[DMODECOUNT];
öBarHBorder[DMODECOUNT];
öMenuVBorder[DMODECOUNT];
öMenuHBorder[DMODECOUNT];
ôcolor0;
ôcolor1;
ôcolor2;
ôcolor3;
ôcolor17;
ôcolor18;
ôcolor19;
ÉTextAttr SysFont;
ÉPreferences*Preferences;
ÉDistantEcho*Echoes;
òViewInitX,ViewInitY;
ïCursorDX,CursorDY;
ÉKeyMap*KeyMap;
ïMouseYMinimum;
ïErrorX,ErrorY;
Étimerequest IOExcess;
ïHoldMinYMouse;
É©*WBPort,*iqd_FNKUHDPort;
ÉIntuiMessage WBMessage;
ÉScreen*HitScreen;
ÉSimpleSprite*SimpleSprite;
ÉSimpleSprite*AttachedSSprite;
BOOL GotSprite1;
ÉÆSemaphoreList;
ÉSignalSemaphore ISemaphore[NUMILOCKS];
òMaxDisplayHeight;
òMaxDisplayRow;
òMaxDisplayWidth;
óReserved[7];á
};á