#include <exec/types.h>
#include <exec/devices.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/gameport.h>
#include <libraries/dos.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#define LINT_ARGS
#include <janus/janus.h>

#include <stdio.h>
#include <string.h>
#include <proto/all.h>

#include "mouseserv.h"

#define D(x) ;
#define printf kprintf
void kprintf(char *,...);

#define TOGGLEPORT   0x19   /* RAWCODE for P switch Mouse Ports */

static struct IntuiText   MyText =
   {
   0,1,      /* Pens */
   JAM2,
   0,0,      /* left, top */
   NULL,     /* font */
   NULL,     /* text */
   NULL,     /* next */
   };

static   char   Title[] = " PC ";

UBYTE      AmigaMousePort   =   0;   /* Port 1, left */

long                       KeySignal;
struct   InputEvent        *GameEventData = NULL;
                           /* pointer into the returned data area   *
                            * where input event has been sent   */
static   short             Error;
struct   IOStdReq          *GameIOMsg = NULL;
static   LONG              GameDevice = 0;
static   BYTE              GameBuffer[sizeof(struct InputEvent)],
                           *GameData = NULL;
struct   MsgPort           *GameMsgPort = NULL;
static   short             CodeVal;
      
static   char              PCTitle[]     = "++ %s on %s port ++ ";   
static   char              VersionId[]   = "MouseServ 2.0";

/*****************************/

struct MsgPort        *BlockPort      = NULL;

struct IntuitionBase  *IntuitionBase  = NULL;
struct JanusBase      *JanusBase      = NULL;
struct GfxBase        *GfxBase        = NULL;

struct Task           *MyTask         = NULL;

long                  KeySignal       = -1;
long                  KeySignalMask   = NULL;

#if 0
long                  SetupSigSignal  = -1;

APTR                  BufMemPtr       = NULL;

struct SetupSig       *JanusSigPtr    = NULL;

struct AMouseParamMem *DualPortParams = NULL;
#endif
/*****************************/
long   ServiceSignal     = -1;
long   ServiceSignalMask = 0;
struct ServiceData *SDb  = NULL;
struct ServiceData *SDw  = NULL;
struct MouseServReq *MSb = NULL;
struct MouseServReq *MSw = NULL;
UBYTE  ChangeCount;
/*****************************/

                                  /* Port used to talk to Input.Device */
struct MsgPort        *InputPort      = NULL; 
                                  /* request block used with Input.Device */
struct IOStdReq       *InputBlock     = NULL;
                                  /* flag whether Input.Device is open */
static   LONG         InputDevice     = 0;
struct Interrupt      Handler;         /* input device handler data */
static   short        GotHandler      = FALSE;

static   int   XPos = 0,
               YPos = 0;

void DoExit(char *s);
void TellInputDevice(int function);
void CreateHandler(void);
void DeleteHandler(void);
void main(void);
void SetUpMouse(void);
void ShutDownMouse(void);
void SetControllerType(short Type);
void SetTrigger(void);
void SetMousePort(void);
struct InputEvent * __asm myHandler(register __a0 struct InputEvent *EventList,register __a1 APTR data);
void SetPortText(char *PortText);
void SetMouseXY(short x,short y);
void SetMousePressed(short LeftP,short RightP,short LeftR,short RightR,short Status);

/************************************************************************
 *
 *  DoExit()
 *
 *  General purpose exit routine.  If 's' is not NULL, then print an
 *  error message with up to three parameters.  Free any memory, close
 *  any open device, delete any ports, close any libraries, etc.
 *
 ************************************************************************/
void DoExit(char *s)
{
   void DeleteHandler();
   long status = RETURN_OK;
   char Buffer[128];
   if (s != NULL)
   {
      sprintf(Buffer,s);
      TellUser(Buffer);
      status = RETURN_ERROR;
   }
   if( GotHandler )    DeleteHandler();
   if( InputDevice )   CloseDevice((struct IORequest *)InputBlock);
   if( InputBlock )    DeleteStdIO(InputBlock);
   if( InputPort)      DeletePort(InputPort);
         
   if(   GameIOMsg != NULL && GameMsgPort != NULL )
   {
      WaitPort(GameMsgPort);   /* got an outstanding IOReq */
      GetMsg( GameMsgPort );   /* tried AbortIO and CMD_CLEAR; but.. */
      SetControllerType(GPCT_NOCONTROLLER);
   }
   if( GameDevice )    CloseDevice((struct IORequest *)GameIOMsg );
   if( GameIOMsg )     DeleteStdIO(GameIOMsg);
   if( GameMsgPort )   DeletePort(GameMsgPort);


#if 0
   if(JanusSigPtr)   CleanupJanusSig(JanusSigPtr);

   if(BufMemPtr) FreeJanusMem(BufMemPtr,sizeof(struct AMouseBufMem));

   if(SetupSigSignal != -1) FreeSignal(SetupSigSignal);
#endif
   if(SDb) DeleteService(SDb);
   if(ServiceSignal!=-1) FreeSignal(ServiceSignal);

   if(KeySignal      != -1) FreeSignal(KeySignal);

   if(GfxBase)       CloseLibrary((struct Library *)GfxBase);
   if(JanusBase)     CloseLibrary((struct Library *)JanusBase);
   if(IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);

   if( BlockPort)      DeletePort(BlockPort);

   exit(status);
}
/************************************************************************
 *
 *  TellInputDevice()
 *
 *  Create a port and I/O block, and open the input device.  Set up the
 *  I/O block to add or remove the input handler, and send the request
 *  to the input device.  Finally, close the device and delete the
 *  I/O block and port.
 *
 ************************************************************************/
void TellInputDevice(int function)
{
   long status;

   if( !(InputPort = CreatePort("AMouseKeyWatch",0)) )
      DoExit("Can't Create Port");

   if( !(InputBlock = CreateStdIO(InputPort)) )
      DoExit("Can't Create Standard IO Block");

   InputDevice = (OpenDevice("input.device",0,(struct IORequest *)InputBlock,0) == 0);
   if( !InputDevice )
      DoExit("Can't Open 'input.device'");
   
   InputBlock->io_Command = (long) function;
   InputBlock->io_Data    = (APTR) &Handler;

   if( (status = DoIO((struct IORequest *)InputBlock)) )
      DoExit("Error from DoIO");

   CloseDevice((struct IORequest *)InputBlock);
   InputDevice = 0;

   DeleteStdIO(InputBlock);
   InputBlock = NULL;

   DeletePort(InputPort);
   InputPort = NULL;

}
/************************************************************************
 *
 *  CreateHandler()
 *
 *  Set the handler priority to 51 so that it is ahead of Intuition.
 *
 *  Finally, add the handler in the chain and tell the user that the
 *  handler has been installed.
 *
 ************************************************************************/
void CreateHandler()
{
   Handler.is_Data        = NULL;
   Handler.is_Code        = (VOID *)myHandler;
   Handler.is_Node.ln_Pri = 51;
   TellInputDevice(IND_ADDHANDLER);
   GotHandler = TRUE;
}
/************************************************************************
 *
 *  Delete Handler()
 *
 *  Retreive the library pointers from the HandlerInfo structure where
 *  we stored them when we originally installed the handler, then remove
 *  the handler from the input handler chain.  Tell the user that the
 *  handler is gone and then close the libraries that are no longer needed.
 *
 ************************************************************************/
void DeleteHandler()
{
   TellInputDevice(IND_REMHANDLER);
   GotHandler = FALSE;
}
/************************************************************************
 *
 *  main()
 *
 ************************************************************************/

void main()
{
   register   int     MsgBits;
   register   short   NotClose;
              short   LeftPressed   = 0,
                      RightPressed  = 0,
                      RightReleased = 0,
                      LeftReleased  = 0,
                      Left          = 0,
                      Right         = 0;
   int                GameSignal,
                      AllSignals;

   D( kprintf("MS: main(enter)\n"); )

   if(FindPort("AMousePort") != NULL)
   {
      DoExit("AMouse is already active");
   }

   if( (BlockPort = CreatePort("AMousePort",0)) == NULL)
   {
      DoExit("Unable to create AMouse Port");
   }
   D( kprintf("MS: BlockPort @ 0x%lx\n",BlockPort); )

   if(!(IntuitionBase=(struct IntuitionBase *)
                       OpenLibrary("intuition.library",0)))
   {
      DoExit("unable to open intuition library");
   }
   D( kprintf("MS: IntuitionBase @ 0x%lx\n",IntuitionBase); )

   if(!(JanusBase=(struct JanusBase *)OpenLibrary("janus.library",0)))
   {
      DoExit("unable to open Janus library");
   }
   D( kprintf("MS: JanusBase @ 0x%lx\n",JanusBase); )

   if(!(GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",0)))
   {
      DoExit("unable to open graphics library");
   }
   D( kprintf("MS: GfxBase @ 0x%lx\n",GfxBase); )

   MyTask = (struct Task *)FindTask(0);
   D( kprintf("MS: MyTask @ 0x%lx\n",MyTask); )

   KeySignal = AllocSignal(-1);
   if( KeySignal == -1 )
   {
      DoExit("KeySignal allocation failure");
   }
   KeySignalMask = 1 << KeySignal;

#if 0
   SetupSigSignal = AllocSignal(-1);
   if( SetupSigSignal == -1 )
   {
      DoExit("SetupSigSignal allocation failure");
   }

   BufMemPtr=AllocJanusMem(sizeof(struct AMouseBufMem),MEMF_BUFFER|MEM_WORDACCESS);
   if(BufMemPtr==0)
   {
      DoExit("BufMemPtr allocation failure");
   }
   D( printf("AMouseBufMem @ 0x%lx\n",BufMemPtr); )

   if( !(JanusSigPtr = (struct SetupSig *)SetupJanusSig(JSERV_AMOUSE,(USHORT)SetupSigSignal,sizeof(struct AMouseParamMem),MEMF_PARAMETER|MEM_WORDACCESS)) )
   {
      DoExit("Janus setup failure");
   }
   ((struct AMouseParamMem *)JanusSigPtr->ss_ParamPtr)->PCParam=0;
   ((struct AMouseParamMem *)JanusSigPtr->ss_ParamPtr)->BufMemOffset=JanusMemToOffset(BufMemPtr);
   JanusUnlock(MakeBytePtr((APTR)&((struct AMouseParamMem *)JanusSigPtr->ss_ParamPtr)->WriteLock));

   DualPortParams = (struct AMouseParamMem *)MakeBytePtr(JanusSigPtr->ss_ParamPtr);
#endif
   ServiceSignal=AllocSignal(-1);
   if( ServiceSignal == -1 )
   {
      DoExit("ServiceSignal allocation failure");
   }
   ServiceSignalMask=1<<ServiceSignal;

   if(AddService(&SDb,1L,2,sizeof(struct MouseServReq),
                  MEMF_PARAMETER|MEM_BYTEACCESS,ServiceSignal,
				  ADDS_LOCKDATA)!=JSERV_OK)
   {
      DoExit("AddService() failure");
   }
   D( kprintf("MS: ServiceData @ 0x%lx\n",SDb); )

   SDb=(struct ServiceData  *)MakeBytePtr((APTR)SDb);
   SDw=(struct ServiceData  *)MakeWordPtr((APTR)SDb);
   MSb=(struct MouseServReq *)MakeBytePtr((APTR)SDw->sd_AmigaMemPtr);
   MSw=(struct MouseServReq *)MakeWordPtr((APTR)SDw->sd_AmigaMemPtr);
   MSw->AmigaPCX=0;
   MSw->AmigaPCY=0;
   MSw->AmigaPCLeftP=0;
   MSw->AmigaPCRightP=0;
   MSw->AmigaPCLeftR=0;
   MSw->AmigaPCRightR=0;
   MSw->AmigaPCStatus=0;
   JanusInitLock(&MSb->WriteLock);
   MSb->ChangeCount=0;
   ChangeCount=0;
   UnlockServiceData(SDb);
   D( kprintf("MS: ServiceData->Unlocked\n"); )

   GameData       = &GameBuffer[0];
   NotClose       = 1;

   CreateHandler();

   if( ! (GameMsgPort = CreatePort("AMouseGamePort",0)) )
      DoExit("CreatePort for Mouse failed");

   if( !(GameIOMsg = CreateStdIO(GameMsgPort)) )
      DoExit("CreateStdIO for Mouse failed");

   SetUpMouse();

   GameSignal   = 1<<GameMsgPort->mp_SigBit;

   AllSignals   = KeySignalMask | GameSignal | ServiceSignalMask;

   while( NotClose )
   {
      MsgBits = Wait(   AllSignals );

	  if( MsgBits & ServiceSignalMask )
	  {
	     if(SDw->sd_Flags & EXPUNGE_SERVICE)
            NotClose=0;
	  }

      if( MsgBits & KeySignalMask )
      {
         D( printf("amouse.c: main() Got a mouse signal\n"); )
         ShutDownMouse();
         SetMousePort();
         SetUpMouse();
         MsgBits &= ~GameSignal;   /* in case both came together */
                                   /* needed 'cos device closed  */
								   /* and re-opened */
      }

      if ( MsgBits & GameSignal )
      {
         D( printf("amouse.c: main() Got a game signal\n"); )
         while(GetMsg(GameMsgPort));
         CodeVal   =   GameEventData->ie_Code;
         SetMouseXY(GameEventData->ie_X,GameEventData->ie_Y);
         switch( CodeVal )
         {
            case IECODE_LBUTTON:
                  Left         = 1;
                  LeftPressed  = 1;
                  break;
            case IECODE_RBUTTON:
                  Right        = 2;
                  RightPressed = 1;
                  break;
            case IECODE_LBUTTON+IECODE_UP_PREFIX:
                  Left         = 0;
                  LeftReleased  = 1;
                  break;
            case IECODE_RBUTTON+IECODE_UP_PREFIX:
                  Right         = 0;
                  RightReleased = 1;
                  break;
            case IECODE_NOBUTTON:
            default:
                  break;
         }
         if( LeftPressed || LeftReleased || RightPressed || RightReleased )
         {
            D( printf("amouse.c: main() buttons pressed\n"); )
            SetMousePressed(LeftPressed,RightPressed,LeftReleased,
                            RightReleased,(short)(Left+Right));
            LeftPressed   = 0;
            RightPressed  = 0;
            RightReleased = 0;
            LeftReleased  = 0;
         } 
         GameIOMsg->io_Length = sizeof(struct InputEvent);
         SendIO((struct IORequest *)GameIOMsg);
      }
   }  
   DeleteHandler();
   DoExit(NULL);
}
/************************************************************************
 *
 * SetUpMouse
 *
 ************************************************************************/
void SetUpMouse()
{
   D( printf("amouse.c: SetUpMouse() enter\n"); )

   if( GameDevice )
   {
      D( printf("amouse.c: SetUpMouse() calls ShutDownMouse()\n"); )
      ShutDownMouse();
   }

   if( !(GameDevice=(OpenDevice("gameport.device",1-AmigaMousePort,
         (struct IORequest *)GameIOMsg,0) == 0) ) )
   {
      DoExit("OpenDevice for gameport failed");
   }

   SetPortText((AmigaMousePort ? "first" : "second"));

   GameEventData          =   (struct InputEvent *)GameBuffer;
   SetControllerType(GPCT_MOUSE);
   SetTrigger();
   
   GameIOMsg->io_Command  =   GPD_READEVENT;
   GameIOMsg->io_Data     =   (APTR)GameBuffer;
   GameIOMsg->io_Length   =   sizeof(struct InputEvent);
   SendIO((struct IORequest *)GameIOMsg);

   D( printf("amouse.c: SetUpMouse() returns Mport=%ld\n",1-AmigaMousePort); )
}
void ShutDownMouse()
{
   long x;

   D( printf("amouse.c: ShutDownMouse() enter GameDevice=0x%lx\n",GameDevice); )
   if( GameDevice )
   {   /* already got an AMouse set up */
      if( !(x=CheckIO((struct IORequest *)GameIOMsg)) )
         WaitPort(GameMsgPort);

      while( x=GetMsg(GameMsgPort) );

      GameIOMsg->io_Command = CMD_CLEAR;
      GameIOMsg->io_Flags   = 0;
      x = DoIO((struct IORequest *)GameIOMsg);
      D( printf("amouse.c: ShutDownMouse() DoIO returns 0x%lx\n",x); )
/*
      AbortIO(GameIOMsg);
      WaitIO(GameIOMsg);
*/
      SetControllerType(GPCT_NOCONTROLLER);
      CloseDevice((struct IORequest *)GameIOMsg);
      GameDevice = 0;
   }
   D( printf("amouse.c: ShutDownMouse() returns GameDevice=0x%lx\n",GameDevice); )
}
/************************************************************************
 *
 * SetControllerType
 *
 ************************************************************************/
void SetControllerType(short Type)
{
   int  res;
   char gpType;

   D( printf("amouse.c: SetControllerType() enter\n"); )

   gpType=(char)Type;
   GameIOMsg->io_Command   =   GPD_SETCTYPE;
   GameIOMsg->io_Length    =   1;
   GameIOMsg->io_Data      =   (APTR)&gpType;
   res=DoIO((struct IORequest *)GameIOMsg);

   D( printf("amouse.c: SetControllerType() DoIO returns 0x%lx\n",res); )
}
void SetTrigger()
{
   int res;
   struct GamePortTrigger gpt;

   D( printf("amouse.c: SetTrigger() enter\n"); )


   gpt.gpt_Keys    = GPTF_DOWNKEYS | GPTF_UPKEYS;
   gpt.gpt_Timeout = 60;
   gpt.gpt_XDelta  = 1;
   gpt.gpt_YDelta  = 1;

   GameIOMsg->io_Command   =   GPD_SETTRIGGER;
   GameIOMsg->io_Length    =   sizeof(struct GamePortTrigger);
   GameIOMsg->io_Data      =   (APTR)&gpt;
   res=DoIO((struct IORequest *)GameIOMsg);

   D( printf("amouse.c: SetTrigger() DoIO returns 0x%lx\n",res); )
}
/************************************************************************
 *
 *  ProgramName:   SetMousePort
 *
 *  Purpose:       To indicate that the mouse is connected to a
 *                 specific game port
 *
 *  Programmer:    Kodiak, Commodore-Amiga Inc.
 *
 *  PlagiarPolicy: You got it, use it however you want.
 *
 ************************************************************************/
void SetMousePort()
{
   UBYTE           Port       = AmigaMousePort;
   long            DoIOError;
   struct MsgPort  *ReqPort   = 0;
   struct IOStdReq *IOReq     = 0;

   D( printf("amouse.c: SetMousePort() enter\n"); )

   if((ReqPort=CreatePort(NULL,NULL))==0)
   {
      D( printf("amouse.c: SetMousePort() can't get port\n"); )
      DoExit("can4t get port for mouse change");
      return;
   }
   if((IOReq=CreateStdIO(ReqPort))==0)
   {
      DeletePort(ReqPort);
      D( printf("amouse.c: SetMousePort() can't CreateStdIO\n"); )
      DoExit("can4t CreateStdIO for mouse change");
      return;
   }

   if (OpenDevice("input.device", 0, (struct IORequest *)IOReq, 0) != 0)
   {
      DeleteStdIO(IOReq);
      DeletePort(ReqPort);
      D( printf("amouse.c: SetMousePort() can't open input.device\n"); )
      DoExit("can4t open input.device for mouse change");
   }

   IOReq->io_Command = IND_SETMPORT;
   IOReq->io_Data    = (APTR)&Port;
   IOReq->io_Length  = 1;
   DoIOError        = DoIO((struct IORequest *)IOReq);

   CloseDevice((struct IORequest *)IOReq);
   DeleteStdIO(IOReq);
   DeletePort(ReqPort);

   if( DoIOError )
   {
      D( printf("amouse.c: SetMousePort() can't get switch port\n"); )
      DoExit("can't switch Intuition mouse port");
   }

   D( printf("amouse.c: SetMousePort() returns IPort=%ld\n",Port); )
}
/************************************************************************
 *
 *  myHandler()
 *
 *  This is the input handler.  Whenever a keyboard event occurs check
 *  to see if it was the AMouse toggle key combination (Left-Amiga + P,
 *  see #define for TOGGLEPORT ) and if so then ......
 *
 ************************************************************************/
struct InputEvent * __asm myHandler(register __a0 struct InputEvent *EventList,register __a1 APTR data)
{
   register struct InputEvent *theEvent  = EventList;
   register struct InputEvent *prevEvent = NULL;
   register int  PChanged = 0;

   /* D( printf("amouse.c: myHandler() enter\n"); ) */

   Forbid();
   while(theEvent)
   {
      if((theEvent->ie_Class == IECLASS_RAWKEY)&&
         (theEvent->ie_Qualifier & IEQUALIFIER_LCOMMAND)&&
         (theEvent->ie_Code == TOGGLEPORT))
      {
         PChanged       = 1 - PChanged;
         AmigaMousePort = 1 - AmigaMousePort;   /* toggle */
         if( !prevEvent )
            EventList =   theEvent->ie_NextEvent;
         else
            prevEvent->ie_NextEvent = theEvent->ie_NextEvent;
      }
      else
         prevEvent = theEvent;
      theEvent  = theEvent->ie_NextEvent;
   }
   Permit();
   if( PChanged )
      Signal(MyTask,KeySignalMask);

   return(EventList);
}   /* end of myHandler */
/************************************************************************
 *
 * SetPortText
 *
 ************************************************************************/
void SetPortText(char *PortText)
{
   struct Screen      *WorkScreen;
   struct Window      *WorkWindow;
   char               Where[128];

   sprintf(Where,PCTitle,VersionId,PortText);
   MyText.IText = Where;
   Forbid();

   for(WorkScreen=IntuitionBase->FirstScreen;
          WorkScreen;
          WorkScreen=WorkScreen->NextScreen)
   {
      for(WorkWindow=WorkScreen->FirstWindow;
          WorkWindow;
          WorkWindow=WorkWindow->NextWindow)
      {
	     if(WorkWindow->Title)
		 {
            if(strncmp(WorkWindow->Title,Title,strlen(Title)) == 0 )
            {
               PrintIText(
                        WorkWindow->RPort,
                        &MyText,
                        (strlen(WorkWindow->Title)<<3)+30,
                        1);
            }
	     }
      }
   }
   Permit();
}
void SetMouseXY(short x,short y)
{

/*
   D( printf("amouse.c: SetMouseXYC(x=%ld,y=%ld)\n",x,y); )
   D( printf("amouse.c: SetMouseXYC() AmigaPCX =%ld\n",ab->AmigaPCX); )
   D( printf("amouse.c: SetMouseXYC() AmigaPCY =%ld\n",ab->AmigaPCY); )
   D( printf("amouse.c: SetMouseXYC() WriteLock=%lx\n",ap->WriteLock); )
   D( printf("amouse.c: SetMouseXYC() ap       =%lx\n",ap); )
   D( printf("amouse.c: SetMouseXYC() ab       =%lx\n",ab); )
*/

   if(++ChangeCount==255)
      ChangeCount=0;
   JanusLock(&MSb->WriteLock);
   MSw->AmigaPCX += x;
   MSw->AmigaPCY += y;
   MSb->ChangeCount=ChangeCount;
   JanusUnlock(&MSb->WriteLock);

   /*D( printf("amouse.c: SetMouseXYC() returns\n",x,y); )*/
}
void SetMousePressed(short LeftP,short RightP,short LeftR,short RightR,short Status)
{
   D( printf("amouse.c: SetMousePressed() enter\n"); )

   if(++ChangeCount==255)
      ChangeCount=0;
   JanusLock(&MSb->WriteLock);
   MSw->AmigaPCLeftP  += LeftP;
   MSw->AmigaPCRightP += RightP;
   MSw->AmigaPCLeftR  += LeftR;
   MSw->AmigaPCRightR += RightR;
   MSw->AmigaPCStatus  = Status;
   MSb->ChangeCount=ChangeCount;
   JanusUnlock(&MSb->WriteLock);

   D( printf("amouse.c: SetMousePressed() returns\n"); )
}
