/*

      ASwarm II V1.0

   Written by Markus "Ill" Illenseer and Matthias "Tron" Scheler
   Based upon Jeff Buterworths XSwarm

*/

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuitionbase.h>
#include <libraries/commodities.h>
#include <libraries/gadtools.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>

#include <clib/alib_protos.h>
#include <clib/commodities_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/macros.h>

#include <string.h>
#include <stdlib.h>

#ifdef LATTICE /* some stuff for SAS-C */

#include <pragmas/commodities_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#define VOID_INTERRUPT void __interrupt __saveds
#define REGARGS        __regargs
#define VOID_STDARGS   void __stdargs

UBYTE *VersionString = "$VER: ASwarm II 1.0 (compiled with SAS/C)";

void chkabort(void)

{}

#else /* some stuff for Dice especially for -mR */

#define VOID_INTERRUPT __stkargs __geta4 void
#define REGARGS
#define VOID_STDARGS   __stkargs void

#undef HotKey
#undef ArgArrayInit
#undef ArgInt
#undef ArgString

__stkargs CxObj *HotKey(UBYTE *,struct MsgPort *,long);
__stkargs UBYTE **ArgArrayInit(long arg1,UBYTE **arg2 );
__stkargs LONG ArgInt(UBYTE **arg1,UBYTE *arg2,long arg3 );
__stkargs UBYTE *ArgString(UBYTE **arg1,UBYTE *arg2,UBYTE *arg3);

UBYTE *VersionString = "$VER: ASwarm II 1.0 (compiled with DICE)";

#endif

/*

   Common Definitions

*/

#define custom (*((struct Custom *)0xDFF000L))

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

#define FINDPROCPORT (&((struct Process *)SysBase->ThisTask)->pr_MsgPort)

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define Move(rp,x,y) {(rp)->cp_x=(x); (rp)->cp_y=(y);}

struct Library *CxBase,*GadToolsBase,*IconBase;

/*

   Definitions for our Commodity

*/

struct NewBroker NewBroker =
 {NB_VERSION,"ASwarm II","Amiga Swarm II V1.0","Screen Blanker based on XSwarm",
  NBU_NOTIFY|NBU_UNIQUE,COF_SHOW_HIDE,0,NULL,0};
struct MsgPort *CxPort;

UBYTE *PopKey;

#define HOTKEY_OPEN_WINDOW 1L

#define DEF_CX_PRI 0
#define DEF_POPKEY "alt s"

LONG TimeOut,ClientTimeOut;

#define MAX_TIMEOUT        3600L
#define MAX_CLIENT_TIMEOUT 60L

#define DEF_TIMEOUT 60
#define DEF_CLIENT_TIMEOUT 5L

#define SERVER_PRI 5L
#define CLIENT_PRI -40L

/*

   Definitions for our Blanker Screen

*/

struct NewScreen NewBlankerScreen =
 {0,0,STDSCREENWIDTH,STDSCREENHEIGHT,2,-1,-1,0L,CUSTOMSCREEN|SCREENQUIET,NULL,NULL,NULL,NULL};

struct ModeNode
 {
  struct Node mn_Node;
  UWORD mn_Index;
  ULONG mn_DisplayID;
  char mn_Name[DISPLAYNAMELEN];
 };

struct List *ModeList;
struct ModeNode *DisplayMode;

#define FindMode(l,n) ((struct ModeNode *)FindName(l,n))

#define DEF_MODE HIRES

/*

   Definitions for our Configuration Window

*/

struct NewWindow NewBlankerWindow =
 {80,16,480,128,0,1,IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_GADGETDOWN|IDCMP_GADGETUP|
  IDCMP_VANILLAKEY,WINDOWCLOSE|WINDOWDRAG|WINDOWDEPTH|SIMPLE_REFRESH,NULL,NULL,NULL,NULL,NULL,
  0,0,0,0,WBENCHSCREEN};
struct Window *BlankerWindow;

#define GID_HIDE    1
#define GID_BLANK   2
#define GID_QUIT    3
#define GID_MODE    4
#define GID_TIMEOUT 5
#define GID_CLIENT  6
#define GID_SWARMS  7
#define GID_BEES    8

#define NUM_GADS 8

struct VisualInfo *BlankerVisualInfo;
struct Gadgets *BlankerGadgets;
struct TextAttr BlankerAttr = {"topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT};
struct NewGadget NewBlankerGadgets[NUM_GADS] =
 {16,112,48,12,"_Hide",&BlankerAttr,GID_HIDE,PLACETEXT_IN,NULL,NULL,
  80,112,48,12,"_Blank",&BlankerAttr,GID_BLANK,PLACETEXT_IN,NULL,NULL,
  144,112,48,12,"_Quit",&BlankerAttr,GID_QUIT,PLACETEXT_IN,NULL,NULL,
  208,28,248,96,"Display Mode",&BlankerAttr,GID_MODE,PLACETEXT_ABOVE,NULL,NULL,
  136,28,56,12,"Timeout",&BlankerAttr,GID_TIMEOUT,PLACETEXT_LEFT,NULL,NULL,
  136,44,56,12,"Client Timeout",&BlankerAttr,GID_CLIENT,PLACETEXT_LEFT,NULL,NULL,
  136,70,56,12,"Swarms",&BlankerAttr,GID_SWARMS,PLACETEXT_LEFT,NULL,NULL,
  136,86,56,12,"Bees per Swarm",&BlankerAttr,GID_BEES,PLACETEXT_LEFT,NULL,NULL};
UBYTE BlankerGadgetKinds[NUM_GADS] =
 {BUTTON_KIND,BUTTON_KIND,BUTTON_KIND,LISTVIEW_KIND,
  INTEGER_KIND,INTEGER_KIND,INTEGER_KIND,INTEGER_KIND};
struct TagItem BlankerGadgetTags[] =
 {GT_Underscore,(ULONG)'_',TAG_DONE,0L,
  GTLV_Labels,0L,GTLV_Selected,0L,GTLV_ShowSelected,NULL,TAG_DONE,0L,
  GTIN_Number,0L,GTIN_MaxChars,4L,TAG_DONE,0L,
  GTIN_Number,0L,GTIN_MaxChars,2L,TAG_DONE,0L,
  GTIN_Number,0L,GTIN_MaxChars,2L,TAG_DONE,0L,
  GTIN_Number,0L,GTIN_MaxChars,3L,TAG_DONE,0L};

struct TagItem *BlankerGadgetTagLists[NUM_GADS] =
 {&BlankerGadgetTags[0],
  &BlankerGadgetTags[0],
  &BlankerGadgetTags[0],
  &BlankerGadgetTags[2],
  &BlankerGadgetTags[6],
  &BlankerGadgetTags[9],
  &BlankerGadgetTags[12],
  &BlankerGadgetTags[15]};

/*

    Definitions for Server/Client Communication

*/

BYTE bsp_TimerSig,bsp_InputSig,bsp_ClientSig;
struct Task *BlankerServerProcess;

#define MASK(n) (1L<<(n))

struct BlankerClientMsg
 {
  struct Message bcm_Message;
  struct Screen *bcm_Screen;
  LONG bcm_Status;
  ULONG bcm_SigMask;
  LONG bcm_Swarms,bcm_Bees;
 };

/*

   Definitions or Swarm Movement

*/

#define BEEACC  3
#define BEEVEL  11
#define WASPACC 5
#define WASPVEL 13
#define BORDER  5

#define BEE_COL_NUM 32

UWORD BeeColors[BEE_COL_NUM] =
 {0x000F,0x004F,0x008F,0x00BF,0x00FF,0x00FB,0x00F7,0x00F3,0x00F0,0x04F0,0x08F0,
  0x09F0,0x0AF0,0x0BF0,0x0CF0,0x0DF0,0x0EF0,0x0FF0,0x0FE0,0x0FD0,0x0FC0,0x0FB0,
  0x0F90,0x0F80,0x0F70,0x0F60,0x0F50,0x0F40,0x0F30,0x0F20,0x0F10,0x0F00};

#define RAND(m) (Random(m)-(m)/2)

UWORD SwarmColors[4] = {0x0000,0x0FFF};
LONG NumSwarms,NumBees;

struct SwarmStruct /* structure for a swarm, including the wasp */
 {
  WORD ss_Width;
  WORD ss_Height;
  WORD ss_NumBees;
  WORD *ss_X[4];    /* the NumBees */
  WORD *ss_Y[4];
  WORD ss_WX[3]; /* the wasp */
  WORD ss_WY[3];
  WORD ss_WXV;
  WORD ss_WYV;
 };

#define ss_XV ss_X[3]
#define ss_YV ss_Y[3]

#define MAX_SWARMS 10L
#define MAX_BEES   100L

#define DEF_SWARMS 1
#define DEF_BEES   25

/*

    The following functions are taken from my resource tracker library for SAS/C.
    I normally use to link the programs with this library, but to make it possible
    for other people to compile ASwarm I included the required source codes.

*/

struct ToolNode
 {
  struct ToolNode *Next;
  void *Tool;
  void (*RemProc)(void *,LONG);
  LONG Size;
 } *ToolList;

void REGARGS RemTool(void *Tool)

{
 struct ToolNode **Ptr,*ToolNode;

 Ptr=&ToolList;
 while (*Ptr)
  if ((*Ptr)->Tool==Tool)
   {
    ToolNode=*Ptr;
    *Ptr=(*Ptr)->Next;
    ToolNode->RemProc(ToolNode->Tool,ToolNode->Size);
    FreeMem (ToolNode,sizeof(struct ToolNode));
   }
  else Ptr=&(*Ptr)->Next;
}

void REGARGS Quit(int ReturnCode)

{
 while (ToolList) RemTool (ToolList->Tool);

 exit (ReturnCode);
}

void REGARGS AddTool(void *NewTool,void *ProcPtr,LONG NewSize)

{
 struct ToolNode *Ptr;
 void (*NewRemProc)(void *,LONG);

 NewRemProc=ProcPtr;
 if (NewTool==NULL) Quit (10);

 if ((Ptr=AllocMem(sizeof(struct ToolNode),MEMF_CLEAR))==NULL)
  {
   NewRemProc (NewTool,NewSize);
   Quit (20);
  }
 Ptr->Next=ToolList;
 Ptr->Tool=NewTool;
 Ptr->RemProc=NewRemProc;
 Ptr->Size=NewSize;
 ToolList=Ptr;
}

/*

   Some useful functions

*/

VOID_STDARGS DeleteMsgPortSafely(struct MsgPort *AnyPort)

{
 struct Message *AnyMsg;

 while (AnyMsg=GetMsg(AnyPort)) ReplyMsg (AnyMsg);
 DeleteMsgPort (AnyPort);
}

int REGARGS ArgIntMax(char **ToolTypes,char *ID,int Default,int Max)

{
 int Val;

 Val=ArgInt(ToolTypes,ID,Default);
 if ((Val<1)||(Val>Max)) return Default;
 else return Val;
}

/*

   Our "InputHandler"

*/

VOID_INTERRUPT BlankerAction(CxMsg *CxMsg,CxObj *CO)

{
 struct InputEvent *IE;

 IE=(struct InputEvent *)CxMsgData(CxMsg);
 if (IE->ie_Class==IECLASS_TIMER) Signal (BlankerServerProcess,1L<<bsp_TimerSig);
 else Signal (BlankerServerProcess,1L<<bsp_InputSig);
}

/*

   Functions for Handling the List of the avaible Graphics Modes

*/

VOID_STDARGS DeleteModeList(struct List *ModeList)

{
 struct ModeNode *ModeNode;

 while (ModeList->lh_Head->ln_Succ)
  {
   ModeNode=(struct ModeNode *)ModeList->lh_Head;
   RemHead (ModeList);
   FreeMem (ModeNode,sizeof(struct ModeNode));
  }
 FreeMem (ModeList,sizeof(struct List));
}

struct List *CreateModeList(void)

{
 struct List *ModeList;
 UWORD Num;
 ULONG DisplayID;
 struct DimensionInfo DimInfo;
 struct NameInfo NameInfo;
 struct ModeNode *ModeNode;

 if (ModeList=AllocMem(sizeof(struct List),MEMF_PUBLIC)) NewList (ModeList);
 else return NULL;

 Num=0;
 DisplayID=INVALID_ID;
 while ((DisplayID=NextDisplayInfo(DisplayID))!=INVALID_ID)
  if ((DisplayID&MONITOR_ID_MASK)&&(ModeNotAvailable(DisplayID)==0L))
   if (GetDisplayInfoData(NULL,(UBYTE *)&DimInfo,sizeof(struct DimensionInfo),DTAG_DIMS,DisplayID))
    if (DimInfo.MaxDepth>1)
     if (GetDisplayInfoData(NULL,(UBYTE *)&NameInfo,sizeof(struct NameInfo),DTAG_NAME,DisplayID))
      if (ModeNode=AllocMem(sizeof(struct ModeNode),MEMF_PUBLIC))
       {
        ModeNode->mn_Node.ln_Name=ModeNode->mn_Name;
        ModeNode->mn_Index=Num++;
        ModeNode->mn_DisplayID=DisplayID;
        strcpy (ModeNode->mn_Name,NameInfo.Name);
        AddTail (ModeList,&ModeNode->mn_Node);
       }

 if (ModeList->lh_Head->ln_Succ) return ModeList;
 else
  {
   FreeMem (ModeList,sizeof(struct List));
   return NULL;
  }
}

struct ModeNode *GetDefaultMode(struct List *ModeList)

{
 struct NameInfo NameInfo;
 struct ModeNode *ModeNode;

 if (GetDisplayInfoData(NULL,(UBYTE *)&NameInfo,sizeof(struct NameInfo),DTAG_NAME,DEF_MODE))
  if (ModeNode=FindMode(ModeList,NameInfo.Name)) return ModeNode;

 return (struct ModeNode *)ModeList->lh_Head;
}

struct ModeNode *GetIndexMode(struct List *ModeList,UWORD Index)

{
 struct ModeNode *ModeNode;

 ModeNode=(struct ModeNode *)ModeList->lh_Head;
 while (ModeNode->mn_Node.ln_Succ)
  if (ModeNode->mn_Index==Index) return ModeNode;
  else ModeNode=(struct ModeNode *)ModeNode->mn_Node.ln_Succ;

 return (struct ModeNode *)ModeList->lh_Head;
}

/*

   Functions for Handling the Configuration Window

*/

LONG GetNum(struct Gadget *Gadget,LONG *Data,LONG Max)

{
 LONG NewData;

 NewData=((struct StringInfo *)Gadget->SpecialInfo)->LongInt;
 if ((NewData<1L)||(NewData>Max))
  {
   GT_SetGadgetAttrs (Gadget,BlankerWindow,NULL,GTIN_Number,(ULONG)*Data,TAG_DONE);
   return FALSE;
  }
 else
  {
   *Data=NewData;
   return TRUE;
  }
}

void CloseBlankerWindow(void)

{
 if (BlankerWindow)
  {
   NewBlankerWindow.LeftEdge=BlankerWindow->LeftEdge;
   NewBlankerWindow.TopEdge=BlankerWindow->TopEdge;

   RemTool (BlankerGadgets);
   RemTool (BlankerVisualInfo);
   RemTool (BlankerWindow);
   BlankerWindow=NULL;
  }
}

void OpenBlankerWindow(void)

{
 struct Gadget *Ptr;
 UWORD Index;
 static char Title[80];

 if (BlankerWindow==NULL)
  {
   strcpy (Title,"ASwarm II by Illenseer/Scheler: HotKey=");
   strcat (Title,PopKey);
   if (BlankerWindow=OpenWindowTags(&NewBlankerWindow,WA_Title,Title,WA_AutoAdjust,TRUE,TAG_DONE))
    {
     AddTool (BlankerWindow,CloseWindow,NULL);

     if ((BlankerVisualInfo=GetVisualInfo(BlankerWindow->WScreen,TAG_DONE))==NULL)
      {
       RemTool (BlankerWindow);
       return;
      }
     AddTool (BlankerVisualInfo,FreeVisualInfo,0L);

     BlankerGadgets=NULL;
     if ((Ptr=CreateContext(&BlankerGadgets))==NULL)
      {
       RemTool (BlankerVisualInfo);
       RemTool (BlankerWindow);
       return;
      }
     AddTool (Ptr,FreeGadgets,0L);

     BlankerGadgetTags[2].ti_Data=(ULONG)ModeList;
     BlankerGadgetTags[3].ti_Data=(ULONG)DisplayMode->mn_Index;
     BlankerGadgetTags[6].ti_Data=(ULONG)TimeOut;
     BlankerGadgetTags[9].ti_Data=(ULONG)ClientTimeOut;
     BlankerGadgetTags[12].ti_Data=(ULONG)NumSwarms;
     BlankerGadgetTags[15].ti_Data=(ULONG)NumBees;
     for (Index=0L; Index<NUM_GADS; Index++)
      {
       NewBlankerGadgets[Index].ng_VisualInfo=BlankerVisualInfo;
       Ptr=CreateGadgetA((ULONG)BlankerGadgetKinds[Index],Ptr,&NewBlankerGadgets[Index],
                         BlankerGadgetTagLists[Index]);
       if (Ptr==NULL)
        {
         CloseBlankerWindow();
         return;
        }
      }

     AddGList (BlankerWindow,BlankerGadgets,0L,-1L,NULL);
     RefreshGadgets (BlankerGadgets,BlankerWindow,NULL);
     GT_RefreshWindow (BlankerWindow,NULL);
    }
  }

 ScreenToFront (BlankerWindow->WScreen);
 WindowToFront (BlankerWindow);
 ActivateWindow (BlankerWindow);
}

/*

   Function to handle the Commodity Stuff

*/

void REGARGS HandleCxMsg(CxObj *Broker,CxMsg *CxMsg)

{
 ULONG MsgType,MsgID;

 MsgType=CxMsgType(CxMsg);
 MsgID=CxMsgID(CxMsg);
 ReplyMsg ((struct Message *)CxMsg);

 switch (MsgType)
  {
   case CXM_IEVENT: /* HotKey was pressed */
    OpenBlankerWindow();
    break;
   case CXM_COMMAND:
    switch (MsgID)
     {
      case CXCMD_DISABLE: /* Message created by Exchange (except CXCMD_UNIQUE) */
       (void)ActivateCxObj(Broker,FALSE);
       break;
      case CXCMD_ENABLE:
       (void)ActivateCxObj(Broker,TRUE);
       break;
      case CXCMD_UNIQUE:
      case CXCMD_APPEAR:
       OpenBlankerWindow();
       break;
      case CXCMD_DISAPPEAR:
       CloseBlankerWindow();
       break;
      case CXCMD_KILL:
       Quit (0);
     }
   }
}

/*

   Open a Screen with the supplied DisplayID

*/

void SpritesOff(void)

{
 OFF_SPRITE /* switch sprites off */
 custom.spr[0].ctl=0;
 custom.spr[1].ctl=0;
 custom.spr[2].ctl=0;
 custom.spr[3].ctl=0;
 custom.spr[4].ctl=0;
 custom.spr[5].ctl=0;
 custom.spr[6].ctl=0;
 custom.spr[7].ctl=0;
}

struct Screen *CreateScreen(struct List *ModeList,struct ModeNode *ModeNode)

{
 struct Screen *Screen;

 if (Screen=OpenScreenTags(&NewBlankerScreen,SA_DisplayID,ModeNode->mn_DisplayID,TAG_DONE))
  {
   SetRGB4 (&Screen->ViewPort,0,0,0,0);
   SetRast (&Screen->RastPort,0);

   SpritesOff();
  }
 return Screen;
}

/*

   Functions fore Creating/Deleting the Client Process

*/

VOID_STDARGS DeleteBlankerClient(struct MsgPort *BlankerClientPort)

{
 struct BlankerClientMsg BlankerClientMsg;

 BlankerClientMsg.bcm_Message.mn_ReplyPort=FINDPROCPORT;
 PutMsg (BlankerClientPort,(struct Message *)&BlankerClientMsg);

 (void)SetTaskPri(BlankerClientPort->mp_SigTask,SERVER_PRI);

 (void)WaitPort(BlankerClientMsg.bcm_Message.mn_ReplyPort);
 (void)GetMsg(BlankerClientMsg.bcm_Message.mn_ReplyPort);
}

struct MsgPort *REGARGS CreateBlankerClient(void *CodePtr,struct BlankerClientMsg *BlankerClientMsg)

{
 struct Process *BlankerClientProcess;
 struct TagItem ProcTags[3];

 ProcTags[0].ti_Tag=NP_Entry;
 ProcTags[0].ti_Data=(ULONG)CodePtr;
 ProcTags[1].ti_Tag=NP_Name;
 ProcTags[1].ti_Data=(ULONG)"BlankerClient";
 ProcTags[2].ti_Tag=TAG_DONE;

 if (BlankerClientProcess=CreateNewProc(ProcTags))
  {
   BlankerClientMsg->bcm_Message.mn_ReplyPort=FINDPROCPORT;
   PutMsg (&BlankerClientProcess->pr_MsgPort,(struct Message *)BlankerClientMsg);

   (void)WaitPort(BlankerClientMsg->bcm_Message.mn_ReplyPort);
   (void)GetMsg(BlankerClientMsg->bcm_Message.mn_ReplyPort);

   (void)SetTaskPri((struct Task *)BlankerClientProcess,CLIENT_PRI);

   if (BlankerClientMsg->bcm_Status) return &BlankerClientProcess->pr_MsgPort;
  }
 return NULL;
}

/*

   Functions for Creating/Drawing/Removing the Swarms

*/

/* Ill's strange and genius Random Function :-) */

WORD REGARGS Random(WORD Max)

{
 static ULONG Num=0L;
 ULONG Sec,Mic;

 CurrentTime((LONG *)&Sec,(LONG *)&Mic);

 Num*=Sec;
 Num+=Mic;

 while (Num>32767L) Num=Num>>1;

 return (WORD)Num%Max;
}

void REGARGS DeleteSwarms(struct SwarmStruct *Swarm,LONG NumSwarms,LONG NumBees)

{
 FreeMem (Swarm,NumSwarms*(sizeof(struct SwarmStruct)+sizeof(WORD)*8L*NumBees));
}

struct SwarmStruct *REGARGS CreateSwarms(struct Screen *SwarmScreen,LONG NumSwarms,LONG NumBees)

/* allocate Memory and initialize the Swarm(s) */

{
 ULONG Size;
 LONG Index,Index2;
 struct SwarmStruct *Swarm,*SP;
 WORD *Ptr;

 Size=NumSwarms*(sizeof(struct SwarmStruct)+sizeof(WORD)*8L*NumBees);
 if ((Swarm=AllocMem(Size,MEMF_CLEAR))==NULL) return NULL;

 for (Index=0L, SP=Swarm, Ptr=(WORD *)&Swarm[NumSwarms]; Index<NumSwarms; Index++, SP++)
  {
   SP->ss_NumBees=NumBees;
   SP->ss_Width=SwarmScreen->Width;
   SP->ss_Height=SwarmScreen->Height;

   for (Index2=0L; Index2<4L; Index2++)
    {
     SP->ss_X[Index2]=Ptr;
     Ptr+=SP->ss_NumBees;
     SP->ss_Y[Index2]=Ptr;
     Ptr+=SP->ss_NumBees;
    }

   /* Bees */

   for (Index2=0L; Index2<SP->ss_NumBees; Index2++)
    {
     SP->ss_X[1][Index2]=SP->ss_X[0][Index2]=BORDER+Random(SP->ss_Width-2*BORDER);
     SP->ss_Y[1][Index2]=SP->ss_Y[0][Index2]=BORDER+Random(SP->ss_Height-2*BORDER);
     SP->ss_XV[Index2]=RAND(BEEACC);
     SP->ss_YV[Index2]=RAND(BEEACC);
    }

   /* Wasp */

   SP->ss_WX[1]=SP->ss_WX[0]=BORDER+Random(SP->ss_Width-2*BORDER);
   SP->ss_WY[1]=SP->ss_WY[0]=BORDER+Random(SP->ss_Height-2*BORDER);
   SP->ss_WXV=RAND(WASPACC);
   SP->ss_WYV=RAND(WASPACC);
  }
 return Swarm;
}

/* move ONE Swarm and redraw it */

void REGARGS DrawSwarm(struct SwarmStruct *SP,struct RastPort *WaspRP,struct RastPort *BeeRP)

{
 LONG Index;

 /* Wasp */

 SP->ss_WX[2]=SP->ss_WX[1];
 SP->ss_WX[1]=SP->ss_WX[0];
 SP->ss_WY[2]=SP->ss_WY[1];
 SP->ss_WY[1]=SP->ss_WY[0];

 SP->ss_WXV+=RAND(WASPACC);
 SP->ss_WYV+=RAND(WASPACC);

 if (SP->ss_WXV>WASPVEL) SP->ss_WXV=WASPVEL;
 if (SP->ss_WXV<-WASPVEL) SP->ss_WXV=-WASPVEL;
 if (SP->ss_WYV>WASPVEL) SP->ss_WYV=WASPVEL;
 if (SP->ss_WYV<-WASPVEL) SP->ss_WYV=-WASPVEL;

 SP->ss_WX[0]=SP->ss_WX[1]+SP->ss_WXV;
 SP->ss_WY[0]=SP->ss_WY[1]+SP->ss_WYV;

 /* Bounce check */

 if ((SP->ss_WX[0]<BORDER)||(SP->ss_WX[0]>SP->ss_Width-BORDER-1)) 
  { 
   SP->ss_WXV=-SP->ss_WXV;
   if (SP->ss_WX[0]<BORDER) SP->ss_WX[0]=BORDER;
   else SP->ss_WX[0]=SP->ss_Width-BORDER-1;
  }
 if ((SP->ss_WY[0]<BORDER)||(SP->ss_WY[0]>SP->ss_Height-BORDER-1))
  {
   SP->ss_WYV=-SP->ss_WYV;
   if (SP->ss_WY[0]<BORDER) SP->ss_WY[0]=BORDER;
   else SP->ss_WY[0]=SP->ss_Height-BORDER-1;
  } 

 /* just a little more random Movements */

 SP->ss_XV[Random(SP->ss_NumBees)]+=RAND(BEEACC);
 SP->ss_YV[Random(SP->ss_NumBees)]+=RAND(BEEACC);

 /* Bees */

 for (Index=0; Index<SP->ss_NumBees; Index++)
  {
   WORD Distance,DX,DY;

   SP->ss_X[2][Index]=SP->ss_X[1][Index];
   SP->ss_X[1][Index]=SP->ss_X[0][Index];
   SP->ss_Y[2][Index]=SP->ss_Y[1][Index];
   SP->ss_Y[1][Index]=SP->ss_Y[0][Index];

   DX=SP->ss_WX[1]-SP->ss_X[1][Index];
   DY=SP->ss_WY[1]-SP->ss_Y[1][Index];
   Distance=ABS(DX)+ABS(DY);
   if (Distance==0) Distance=1;

   SP->ss_XV[Index]+=(DX*BEEACC)/Distance;
   SP->ss_YV[Index]+=(DY*BEEACC)/Distance;

   if (SP->ss_XV[Index]>BEEVEL) SP->ss_XV[Index]=BEEVEL;
   if (SP->ss_XV[Index]<-BEEVEL) SP->ss_XV[Index]=-BEEVEL;
   if (SP->ss_YV[Index]>BEEVEL) SP->ss_YV[Index]=BEEVEL;
   if (SP->ss_YV[Index]<-BEEVEL) SP->ss_YV[Index]=-BEEVEL;

   SP->ss_X[0][Index]=SP->ss_X[1][Index]+SP->ss_XV[Index];
   SP->ss_Y[0][Index]=SP->ss_Y[1][Index]+SP->ss_YV[Index];

   /* Bounce check */

   if ((SP->ss_X[0][Index]<BORDER)||(SP->ss_X[0][Index]>(SP->ss_Width-BORDER-1)))
    {
     SP->ss_XV[Index]=-SP->ss_XV[Index];
     SP->ss_X[0][Index]=SP->ss_X[1][Index]+SP->ss_XV[Index];
    }
   if ((SP->ss_Y[0][Index]<BORDER)||(SP->ss_Y[0][Index]>(SP->ss_Height-BORDER-1)))
    {
     SP->ss_YV[Index]=-SP->ss_YV[Index];
     SP->ss_Y[0][Index]=SP->ss_Y[1][Index]+SP->ss_YV[Index];
    }
  }
 
 /* Erase our insects */

 SetDrMd (WaspRP,JAM1);
 SetAPen (WaspRP,0);
 Move (WaspRP,SP->ss_WX[1],SP->ss_WY[1]);  /* Wasp */
 Draw (WaspRP,SP->ss_WX[2],SP->ss_WY[2]);

 SetDrMd (BeeRP,JAM1);
 SetAPen (BeeRP,0);
 for (Index=0; Index<SP->ss_NumBees; Index++) /* Bees */
  {
   Move (BeeRP,SP->ss_X[1][Index],SP->ss_Y[1][Index]);
   Draw (BeeRP,SP->ss_X[2][Index],SP->ss_Y[2][Index]);
  }
   
 /* And draw them again */

 SetAPen (WaspRP,1);
 Move (WaspRP,SP->ss_WX[0],SP->ss_WY[0]); /* Wasp */
 Draw (WaspRP,SP->ss_WX[1],SP->ss_WY[1]);

 SetAPen (BeeRP,1);
 for (Index=0; Index<SP->ss_NumBees; Index++) /* Bees */
  {
   Move (BeeRP,SP->ss_X[0][Index],SP->ss_Y[0][Index]);
   Draw (BeeRP,SP->ss_X[1][Index],SP->ss_Y[1][Index]);
  }
}

/*

   This is the Client Process's Main Loop

*/

VOID_INTERRUPT ASwarmClientProcess(void)

{
 struct BlankerClientMsg *BlankerClientMsg;
 struct MsgPort *BlankerClientPort;
 struct Task *BlankerServerTask;
 ULONG BlankerServerSigMask;
 struct Screen *SwarmScreen;
 LONG Index,NumSwarms,NumBees;
 struct SwarmStruct *Swarms;
 WORD Color,DColor;
 static struct RastPort RastPorts[2];
 static struct BitMap BitMaps[2];

 /* wait for Server's initial Message */

 BlankerClientPort=FINDPROCPORT;
 (void)WaitPort(BlankerClientPort);
 BlankerClientMsg=(struct BlankerClientMsg *)GetMsg(BlankerClientPort);

 BlankerServerTask=BlankerClientMsg->bcm_Message.mn_ReplyPort->mp_SigTask;
 BlankerServerSigMask=BlankerClientMsg->bcm_SigMask;

 NumSwarms=BlankerClientMsg->bcm_Swarms;
 NumBees=BlankerClientMsg->bcm_Bees;
 SwarmScreen=BlankerClientMsg->bcm_Screen;

 /* initialize requested Number of Swarms */

 if ((Swarms=CreateSwarms(SwarmScreen,NumSwarms,NumBees))==NULL)
  {
   BlankerClientMsg->bcm_Status=FALSE;
   Forbid();
   ReplyMsg ((struct Message *)BlankerClientMsg);
   return;
  }
 BlankerClientMsg->bcm_Status=TRUE;
 ReplyMsg ((struct Message *)BlankerClientMsg);

 /* Here we do a little trick:
    We create two RastPorts - one for the Wasp(s), one for the Bees - with one
    BitPlane. So we have only to draw in one BitPlane and are able to do it faster. */

 for (Index=0L; Index<2L; Index++)
  {
   InitRastPort (&RastPorts[Index]);
   RastPorts[Index].BitMap=&BitMaps[Index];
   InitBitMap (&BitMaps[Index],1L,SwarmScreen->Width,SwarmScreen->Height);
   BitMaps[Index].Planes[0]=SwarmScreen->BitMap.Planes[Index];
  }

 Color=-1;
 DColor=1;
 while ((BlankerClientMsg=(struct BlankerClientMsg *)GetMsg(BlankerClientPort))==NULL)
  {
   WaitTOF();

   /* Color Cycling */

   Color+=DColor;
   if (Color==BEE_COL_NUM)
    {
     Color-=2;
     DColor=-1;
    }
   else
    if (Color<0)
     {
      Color+=2;
      DColor=1;
     }
   SwarmColors[2]=BeeColors[Color];
   LoadRGB4 (&SwarmScreen->ViewPort,SwarmColors,4);

   /* Move the Swarm(s) */

   for (Index=0L; Index<NumSwarms; Index++)
    DrawSwarm (&Swarms[Index],&RastPorts[0],&RastPorts[1]);
   Signal (BlankerServerTask,BlankerServerSigMask);
  }

 /* We are requested to finish, so we do. */

 DeleteSwarms (Swarms,NumSwarms,NumBees);
 Forbid();
 ReplyMsg ((struct Message *)BlankerClientMsg);
}

/*

   The Main Loop

*/

void main(LONG argc,UBYTE *argv[])

{
 char **ToolTypes;
 CxObj *Broker,*ObjectList,*Filter;
 struct IntuiMessage *IntMsg;
 CxMsg *BlankerCxMsg;
 LONG ThisTimeOut,TimeUntilBlank,TimeUntilBlack;
 struct Screen *BlankerScreen=NULL;
 struct BlankerClientPort *BlankerClientPort=NULL;

 /* open our Libraries */

 AddTool (GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",37L),
          CloseLibrary,0L);
 AddTool (IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",37L),
          CloseLibrary,0L);
 AddTool (IconBase=OpenLibrary("icon.library",37L),CloseLibrary,0L);
 AddTool (CxBase=OpenLibrary("commodities.library",37L),CloseLibrary,0L);
 AddTool (GadToolsBase=OpenLibrary("gadtools.library",37L),CloseLibrary,0L);

 /* create List of Graphics Modes */

 AddTool (ModeList=CreateModeList(),DeleteModeList,0L);

 /* get our Arguments */

 if (ToolTypes=ArgArrayInit(argc,argv)) AddTool (ToolTypes,ArgArrayDone,0L);

 /* get some Signals */

 BlankerServerProcess=SysBase->ThisTask;
 if ((bsp_TimerSig=AllocSignal(-1L))==-1) Quit (10);
 AddTool ((void *)bsp_TimerSig,FreeSignal,0L);
 if ((bsp_InputSig=AllocSignal(-1L))==-1) Quit (10);
 AddTool ((void *)bsp_InputSig,FreeSignal,0L);
 if ((bsp_ClientSig=AllocSignal(-1L))==-1) Quit (10);
 AddTool ((void *)bsp_ClientSig,FreeSignal,0L);

 /* initialize our Broker = install us as a Commodity */

 AddTool (CxPort=CreateMsgPort(),DeleteMsgPortSafely,0L);

 NewBroker.nb_Pri=ArgInt(ToolTypes,"CX_PRIORITY",DEF_CX_PRI);
 NewBroker.nb_Port=CxPort;
 AddTool (Broker=CxBroker(&NewBroker,NULL),DeleteCxObjAll,0L);

 /* get Time Out, Client Time Out and Display mode */

 TimeOut=ArgIntMax(ToolTypes,"TIMEOUT",DEF_TIMEOUT,MAX_TIMEOUT);
 ClientTimeOut=ArgIntMax(ToolTypes,"CLIENTTIMEOUT",DEF_CLIENT_TIMEOUT,MAX_CLIENT_TIMEOUT);
 if ((DisplayMode=FindMode(ModeList,ArgString(ToolTypes,"DISPLAY","")))==NULL)
  DisplayMode=GetDefaultMode(ModeList);

 /* get Parameters for Swarm Movement */

 NumSwarms=ArgIntMax(ToolTypes,"SWARMS",DEF_SWARMS,MAX_SWARMS);
 NumBees=ArgIntMax(ToolTypes,"BEES",DEF_BEES,MAX_BEES);

 /* instal our Hot Key */

 PopKey=ArgString(ToolTypes,"CX_POPKEY",DEF_POPKEY);
 if ((Filter=HotKey(PopKey,CxPort,HOTKEY_OPEN_WINDOW))==NULL) Quit (10);
 else AttachCxObj(Broker,Filter);
 if (CxObjError(Filter)) Quit (10);

 /* install our "InputHandler" */

 ObjectList=CxCustom(BlankerAction,0L);
 AttachCxObj (Broker,ObjectList);
 if (CxObjError(ObjectList)) Quit (10);

 (void)ActivateCxObj(Broker,TRUE);
 AddTool (Broker,ActivateCxObj,FALSE);

 /* open Window on startup if not forbidden */

 if (stricmp(ArgString(ToolTypes,"CX_POPUP",""),"NO")) OpenBlankerWindow();

 /* increase our Priority */

 AddTool (FindTask(NULL),SetTaskPri,(LONG)SetTaskPri(FindTask(NULL),SERVER_PRI));

 /* start the Loop */

 TimeUntilBlank=ThisTimeOut=10L*TimeOut;
 TimeUntilBlack=10L*ClientTimeOut;
 FOREVER
  {
   ULONG Mask;

   if (BlankerWindow)
    Mask=Wait(MASK(bsp_TimerSig)|MASK(bsp_InputSig)|MASK(bsp_ClientSig)|
              MASK(CxPort->mp_SigBit)|MASK(BlankerWindow->UserPort->mp_SigBit)|
              SIGBREAKF_CTRL_C);
   else
    Mask=Wait(MASK(bsp_TimerSig)|MASK(bsp_InputSig)|MASK(bsp_ClientSig)|
              MASK(CxPort->mp_SigBit)|SIGBREAKF_CTRL_C);

   /* process Window Events */

   while ((BlankerWindow!=NULL)&&(IntMsg=GT_GetIMsg(BlankerWindow->UserPort)))
    switch (IntMsg->Class)
     {
      struct Gadget *Clicked;
      UWORD Code;

      case IDCMP_CLOSEWINDOW:
       GT_ReplyIMsg (IntMsg);
       CloseBlankerWindow();
       break;
      case IDCMP_REFRESHWINDOW:
       GT_BeginRefresh (BlankerWindow);
       GT_EndRefresh (BlankerWindow,TRUE);
       break;
      case IDCMP_GADGETUP:
       Code=IntMsg->Code;
       Clicked=(struct Gadget *)IntMsg->IAddress;
       GT_ReplyIMsg (IntMsg);
       switch (Clicked->GadgetID)
        {
         case GID_HIDE:
          CloseBlankerWindow();
          break;
         case GID_QUIT:
          Quit (0);
         case GID_BLANK:
          if (TimeUntilBlank) TimeUntilBlank=ThisTimeOut=2L;
          break;
         case GID_MODE:
          DisplayMode=GetIndexMode(ModeList,Code);
          break;
         case GID_TIMEOUT:
          if (GetNum(Clicked,&TimeOut,MAX_TIMEOUT)) TimeUntilBlank=ThisTimeOut=10L*TimeOut;
          break;
         case GID_CLIENT:
          if (GetNum(Clicked,&ClientTimeOut,MAX_CLIENT_TIMEOUT)) TimeUntilBlack=10L*ClientTimeOut;
          break;
         case GID_SWARMS:
          (void)GetNum(Clicked,&NumSwarms,MAX_SWARMS);
          break;
         case GID_BEES:
          (void)GetNum(Clicked,&NumBees,MAX_BEES);
        }
       break;
      case IDCMP_VANILLAKEY:
       Code=IntMsg->Code;
       GT_ReplyIMsg (IntMsg);
       switch ((char)Code)
        {
         case 'H':
         case 'h':
          CloseBlankerWindow();
          break;
         case 'Q':
         case 'q':
          Quit (0);
         case 'B':
         case 'b':
          if (TimeUntilBlank) TimeUntilBlank=ThisTimeOut=2L;
        }
       break;
      default:
       GT_ReplyIMsg (IntMsg);
     }

   /* process Commodity Messages */

   while (BlankerCxMsg=(CxMsg *)GetMsg(CxPort)) HandleCxMsg (Broker,BlankerCxMsg);

   /* check for <CTRL>-C */

   if (Mask&SIGBREAKF_CTRL_C) Quit (0);

   /* Input detected, unblank if necessary */

   if (Mask&MASK(bsp_InputSig))
    {
     if (TimeUntilBlank==0L)
      {
       ON_SPRITE
       if (BlankerClientPort) RemTool (BlankerClientPort);
       RemTool (BlankerScreen);
       ThisTimeOut=10L*TimeOut;
      }
     TimeUntilBlank=ThisTimeOut;
    }

   /* client has confirmed that it is still alive */

   if (Mask&MASK(bsp_ClientSig))
    {
     ON_DISPLAY
     TimeUntilBlack=10L*ClientTimeOut;
    }

   /* 1/10 sec is over */

   if (Mask&MASK(bsp_TimerSig))
    if (TimeUntilBlank)
     {
      TimeUntilBlank--;
      if (TimeUntilBlank==0L) /* Time Out reached, blank the screen */
       {
        struct BlankerClientMsg BlankerClientMsg;

        AddTool (BlankerScreen=CreateScreen(ModeList,DisplayMode),CloseScreen,0L);

        BlankerClientMsg.bcm_Screen=BlankerScreen;
        BlankerClientMsg.bcm_SigMask=1L<<bsp_ClientSig;
        BlankerClientMsg.bcm_Swarms=NumSwarms;
        BlankerClientMsg.bcm_Bees=NumBees;
        if (BlankerClientPort=CreateBlankerClient(ASwarmClientProcess,&BlankerClientMsg))
         {
          TimeUntilBlack=10L*ClientTimeOut; /* try to start Client */
          AddTool (BlankerClientPort,DeleteBlankerClient,0L);
         }
       }
     }
    else
     {
      if (IntuitionBase->FirstScreen!=BlankerScreen)
       {
        ScreenToFront (BlankerScreen);
        SpritesOff();
       }
      if (TimeUntilBlack)
       {
        TimeUntilBlack--;
        if (TimeUntilBlack==0L) OFF_DISPLAY /* Client Time Out reached, turn entire screen black */
       }
     }
  }
}
