��INTUITION_INTUITIONBASE_H�INTUITION_INTUITIONBASE_H�EXEC_TYPES_H�<exec/types.h>��EXEC_LIBRARIES_H�<exec/libraries.h>��INTUITION_INTUITION_H�<intuition/intuition.h>��EXEC_INTERRUPTS_H�<exec/interrupts.h>��INTUITIONPRIVATE�DMODECOUNT 2�HIRESPICK 0�LOWRESPICK 1�EVENTMAX 10�RESCOUNT 2�HIRESGADGET 0�LOWRESGADGET 1�GADGETCOUNT 8�UPFRONTGADGET 0�DOWNBACKGADGET 1�SIZEGADGET 2�CLOSEGADGET 3�DRAGGADGET 4�SUPFRONTGADGET 5�SDOWNBACKGADGET 6�SDRAGGADGET 7�ISTATELOCK 0�LAYERINFOLOCK 1�GADGETSLOCK 2�LAYERROMLOCK 3�VIEWLOCK 4�IBASELOCK 5�RPLOCK 6�NUMILOCKS 7
�FatIntuiMessage{
�IntuiMessage;
�PrevKeys;
};
�IBox{
�Left;
�Top;
�Width;
�Height;
};
�Point{
�X;
�Y;
};
�PenPair{
�DetailPen;
�BlockPen;
};
�GListEnv{
�Screen*ge_Screen;
�Window*ge_Window;
�Requester*ge_Requester;
�RastPort*ge_RastPort;
�Layer*ge_Layer;
�Layer*ge_GZZLayer;
�PenPair ge_Pens;
�IBox ge_Domain;
�IBox ge_GZZdims;
};
�GadgetInfo{
�GListEnv*gi_Environ;
��*gi_Gadget;
�IBox gi_Box;
�IBox gi_Container;
�Layer*gi_Layer;
�IBox gi_NewKnob;
};�
��
{
�Library LibNode;
�View ViewLord;
�Window*ActiveWindow;
�Screen*ActiveScreen;
�Screen*FirstScreen;
��;
�MouseY,MouseX;
�Seconds;
�Micros;�INTUITIONPRIVATE
�MinXMouse,MaxXMouse;
�MinYMouse,MaxYMouse;
�StartSecs,StartMicros;
�SysBase;
��*�;
��;
�ConsoleDevice;
�*APointer;
�APtrHeight;
�APtrWidth;
�AXOffset,AYOffset;
�MenuDrawn;
�MenuSelected;
�OptionList;
�RastPort MenuRPort;
�TmpRas MenuTmpRas;
�ClipRect ItemCRect;
�ClipRect SubCRect;
�BitMap IBitMap;
�BitMap SBitMap;
�IOStdReq InputRequest;
�Interrupt InputInterrupt;
�Remember*EventKey;
�InputEvent*IEvents;�NUM_IEVENTS 4
�EventCount;
�InputEvent IEBuffer[NUM_IEVENTS];
��*ActiveGadget;
�PropInfo*ActivePInfo;
�Image*ActiveImage;
�GListEnv GadgetEnv;
�GadgetInfo GadgetInfo;
�Point KnobOffset;
�Window*getOKWindow;
�IntuiMessage*getOKMessage;
�setWExcept,GadgetReturn,StateReturn;
�RastPort*RP;
�TmpRas ITmpRas;
�Region*OldClipRegion;
�Point OldScroll;
�IBox IFrame;
�hthick,vthick;
VOID(*frameChange)();
VOID(*sizeDrag)();
�Point FirstPt;
�Point OldPt;
��*SysGadgets[RESCOUNT][GADGETCOUNT];
�Image*CheckImage[RESCOUNT],*AmigaIcon[RESCOUNT];�OLDPATTERN
�apattern[3],bpattern[4];�
�apattern[8],bpattern[4];�
�*IPointer;
�IPtrHeight;
�IPtrWidth;
�IXOffset,IYOffset;
�DoubleSeconds,DoubleMicros;
�WBorLeft[DMODECOUNT];
�WBorTop[DMODECOUNT];
�WBorRight[DMODECOUNT];
�WBorBottom[DMODECOUNT];
�BarVBorder[DMODECOUNT];
�BarHBorder[DMODECOUNT];
�MenuVBorder[DMODECOUNT];
�MenuHBorder[DMODECOUNT];
�color0;
�color1;
�color2;
�color3;
�color17;
�color18;
�color19;
�TextAttr SysFont;
�Preferences*Preferences;
�DistantEcho*Echoes;
�ViewInitX,ViewInitY;
�CursorDX,CursorDY;
�KeyMap*KeyMap;
�MouseYMinimum;
�ErrorX,ErrorY;
�timerequest IOExcess;
�HoldMinYMouse;
��*WBPort,*iqd_FNKUHDPort;
�IntuiMessage WBMessage;
�Screen*HitScreen;
�SimpleSprite*SimpleSprite;
�SimpleSprite*AttachedSSprite;
BOOL GotSprite1;
��SemaphoreList;
�SignalSemaphore ISemaphore[NUMILOCKS];
�MaxDisplayHeight;
�MaxDisplayRow;
�MaxDisplayWidth;
�Reserved[7];�
};�