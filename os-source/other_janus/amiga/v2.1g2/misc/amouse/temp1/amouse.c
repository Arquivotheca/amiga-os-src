

/*
      copyright (c) 1988 Commodore Business Machines
*/



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
#include <intuition/intuitionbase.h>
#include <janus/setupsig.h>

void SetUpMouse();
void MaybeMouseMoved();
void SetMousePort();
void SetPortText();

#ifdef   AZTEC_C

/*
   variables for detach

*/

   long   _stack = 0;      /* use CLI stack size */
   long   _priority = 0;      /* process priority */
   long   _BackGroundIO = 0;   /* no output to CLI window */
   char   *_procname = "AMouseProcess";

#endif


#define TOGGLEPORT   0x19   /* RAWCODE for P switch Mouse Ports */

   UBYTE      AmigaMousePort   =   0;   /* Port 1, left */

#define XMOVE 1
#define YMOVE 1

struct   GamePortTrigger   MouseTrigger =
   {
   GPTF_UPKEYS+GPTF_DOWNKEYS,
   60,      /* short - 'cos of outstanding IOReq at toggle */
   XMOVE,
   YMOVE,
   };

struct   Task              *MyTask;
long                       MouseSignal;
struct   InputEvent        *GameEventData = NULL;
            /* pointer into the returned data area   *
             * where input event has been sent   */
static   short             Error;
struct   IOStdReq          *GameIOMsg = NULL;
static   LONG              GameDevice = 0;
static   BYTE              GameBuffer[sizeof(struct InputEvent)],
                           *GameData = NULL;
struct   MsgPort           *GameMsgPort = NULL;
extern   struct MsgPort    *CreatePort();
static   struct MsgPort    *BlockPort=0;
extern   struct IOStdReq   *CreateStdIO();
static   short             CodeVal;
      
static   char              PCTitle[]     = "«« %s on %s port »» ";   
static   struct   Window   *Window       = NULL;
static   char              VersionId[]   = "AMouse v1.5";

/*
static   struct   NewWindow   NewWindow =
   {
   30,         \* Left      *\
   30,         \* Top      *\
   268,         \* Width   *\
   10,         \* Height   *\
   0,         \* DetailPen   *\
   1,         \* BlockPen   *\
   NULL,         \* IDCMPFlags   *\
\*
   CLOSEWINDOW,      IDCMPFlags
   WINDOWCLOSE   |
*\
   WINDOWDRAG   |
   WINDOWDEPTH   |
   SMART_REFRESH   |
   NOCAREREFRESH,      \* Flags   *\
   NULL,         \* Gadgets   *\
   NULL,         \* CheckMark   *\
   Title1,         \* Text      *\
   NULL,         \* Screen   *\
   NULL,         \* BitMap   *\
   0,0,0,0,      \* Min\Max sizes*\
   WBENCHSCREEN,      \* Type      *\
   };

*/



extern struct MsgPort *FindPort();
extern APTR AllocMem();
extern long LoadSeg();
extern void myHandlerStub();

#define INTUITION_REV   0L
#define LAYERS_REV      0L

struct IntuitionBase  *IntuitionBase  = NULL;
extern struct SysBase *SysBase;

struct GfxBase        *GfxBase        = 0;
struct SetupSig       *JanusSigPtr    = 0;
char                  *DualPortParams = NULL;
                                  /* Port used to talk to Input.Device */
struct MsgPort        *InputPort      = NULL; 
                                  /* request block used with Input.Device */
struct IOStdReq       *InputBlock     = NULL;
                                  /* flag whether Input.Device is open */
static   LONG         InputDevice     = 0;
struct Interrupt      Handler;         /* input device handler data */
static   short        GotHandler      = FALSE;

/************************************************************************
 *
 *  DoExit()
 *
 *  General purpose exit routine.  If 's' is not NULL, then print an
 *  error message with up to three parameters.  Free any memory, close
 *  any open device, delete any ports, close any libraries, etc.
 *
 ************************************************************************/
void DoExit(s,x1,x2,x3)
char *s, *x1, *x2, *x3;
   {
   void DeleteHandler();
   long status = RETURN_OK;
   char Buffer[128];
   if (s != NULL)
      {
      sprintf(Buffer,s,x1,x2,x3);
      TellUser(Buffer);
      status = RETURN_ERROR;
      }
   if( GotHandler )    DeleteHandler();
   if( InputDevice )   CloseDevice(InputBlock);
   if( InputBlock )    DeleteStdIO(InputBlock);
   if( BlockPort)      DeletePort(BlockPort);
   if( InputPort)      DeletePort(InputPort);
   if( JanusSigPtr )   CleanUpJanus();
         
   if(   GameIOMsg != NULL
      &&
      GameMsgPort != NULL
      )
      {
      WaitPort(GameMsgPort);   /* got an outstanding IOReq */
      GetMsg( GameMsgPort );   /* tried AbortIO and CMD_CLEAR; but.. */
      SetControllerType(GPCT_NOCONTROLLER);
      }
   if( GameDevice )    CloseDevice( GameIOMsg );
   if( GameIOMsg )     DeleteStdIO(GameIOMsg);
   if( GameMsgPort )   DeletePort(GameMsgPort);
/*
   if( Window )        CloseWindow(Window);
*/
   if( IntuitionBase ) CloseLibrary(IntuitionBase);
   if( GfxBase )       CloseLibrary(GfxBase);
   exit(status);
   }   /* end of DoExit */


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
 
void TellInputDevice(function)
int function;
   {
   long status;

   if( !(InputPort = CreatePort("AMouseKeyWatch",0)) )
      DoExit("Can't Create Port");
   if( !(InputBlock = CreateStdIO(InputPort)) )
      DoExit("Can't Create Standard IO Block");
   InputDevice = (OpenDevice("input.device",0,InputBlock,0) == 0);
   if( !InputDevice )
      DoExit("Can't Open 'input.device'");
   
   InputBlock->io_Command = (long) function;
   InputBlock->io_Data    = (APTR) &Handler;
   if( (status = DoIO(InputBlock)) )
      DoExit("Error from DoIO:  %ld",status);
   CloseDevice(InputBlock);
   InputDevice = 0;
   DeleteStdIO(InputBlock);
   InputBlock = NULL;
   DeletePort(InputPort);
   InputPort = NULL;
   }   /* end of TellInputDevice */


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

#ifdef MANX

#define DATA_DEF &_H1_org
int   _H1_org();

#else

#define   DATA_DEF 0

#endif

void CreateHandler()
   {

   Handler.is_Data    = DATA_DEF;
   Handler.is_Code    = myHandlerStub;
   Handler.is_Node.ln_Pri   = 51;
   TellInputDevice(IND_ADDHANDLER);
   GotHandler = TRUE;

   }   /* end of CreateHandler */


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

   }   /* end of DeleteHandler */


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
   if( !(IntuitionBase = 
               (struct IntuitionBase *)OpenLibrary("intuition.library",0)) )
      DoExit("unable to open intuition library");
   if(FindPort("AMousePort") != NULL)
      {
      DoExit("AMouse is already active");
      }

   if( (BlockPort = CreatePort("AMousePort",0)) == NULL)
      {
      DoExit("Unable to create AMouse Port");
      }

   MyTask = (struct Task *)FindTask(0);
   MouseSignal = AllocSignal(-1);
   if( MouseSignal == -1 )
      DoExit("Mouse signal allocation failure");
   MouseSignal = 1 << MouseSignal;
   CreateHandler();

/*
   based on source in ROM Kernel : Lib&Devs Pages 349 to 355
*/

   if( !(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0)) )
      DoExit("unable to open graphics library");

/*
   if( !(Window = (struct Window *) OpenWindow(&NewWindow)) )
      DoExit("Open Window Failure");
*/

   if( !(JanusSigPtr = (struct SetupSig *)SetUpJanus()) )
      DoExit("Janus setup failure");
   DualPortParams = (char *)JanusSigPtr->ss_ParamPtr;
   GameData = &GameBuffer[0];
   NotClose      =   1;

   SetUpMouse();
   GameSignal   = 1<<GameMsgPort->mp_SigBit;
/*
   WindowSignal = 1<<Window->UserPort->mp_SigBit;
*/
   AllSignals   = 
         MouseSignal
         |
/*
         WindowSignal
         |
*/
         GameSignal
         ;

   while( NotClose )
      {


      MsgBits = Wait(   AllSignals );
/*
      if( MsgBits & WindowSignal )
         {
         ReplyMsg(GetMsg(Window->UserPort));
         NotClose = 0;
         }
*/
      if( MsgBits & MouseSignal )
         {
         SetUpMouse();
         SetMousePort();
         MsgBits &= ~GameSignal;   /* in case both came together */
                                   /* needed 'cos device closed and re-opened */
/*
         WindowToFront(Window);
*/
         }
      if ( MsgBits & GameSignal )
         {
         GetMsg(GameMsgPort);
         CodeVal   =   GameEventData->ie_Code;
         MaybeMouseMoved();
         switch( CodeVal )
            {
case IECODE_LBUTTON:
            Left        = 1;
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
            }  /* end of switch */
         if( LeftPressed || LeftReleased || RightPressed || RightReleased )
            {
            SetMousePressed(
                        LeftPressed,
                        RightPressed,
                        LeftReleased,
                        RightReleased,
                        Left+Right);
            LeftPressed   = 0;
            RightPressed  = 0;
            RightReleased = 0;
            LeftReleased  = 0;
            }  /* end of if something pressed or released */
         GameIOMsg->io_Length =
               sizeof(struct InputEvent);
         SendIO(GameIOMsg);
    	 }  /* end of if GameMsg */
      }   /* end of while( NotClose ) */

      
   DeleteHandler();
   DoExit(NULL);
   }   /* end of main */


static   int   XPos = 0,
               YPos = 0;

/************************************************************************
 *
 * SetUpMouse
 *
 ************************************************************************/
void SetUpMouse()
   {
   long   x;
   if( GameDevice )
      {   /* already got an AMouse set up */
      if( !(x=CheckIO(GameIOMsg)) )
         WaitPort(GameMsgPort);   /* wait for timeout !!! */

      while( x=GetMsg(GameMsgPort) );

      GameIOMsg->io_Command = CMD_CLEAR;
      GameIOMsg->io_Flags   = 0;
      x = DoIO(GameIOMsg);
      SetControllerType(GPCT_NOCONTROLLER);
      CloseDevice(GameIOMsg);
      GameDevice = 0;
      }
   else
      {
      if( ! (GameMsgPort = CreatePort("AMouseGamePort",0)) )
         DoExit("CreatePort for Mouse failed");
      if( !(GameIOMsg = CreateStdIO(GameMsgPort)) )
         DoExit("CreateStdIO for Mouse failed");
      }
   if( !(GameDevice =
      (OpenDevice(
         "gameport.device",
         1-AmigaMousePort,   /* use ´other´ port */
         GameIOMsg,
         0) == 0) )
      )
      DoExit("OpenDevice for gameport failed");
/*
   SetWindowTitles(Window, (AmigaMousePort ? Title0 : Title1), -1);
*/
   SetPortText((AmigaMousePort ? "left" : "right"));

   GameIOMsg->io_Length   =   sizeof(struct InputEvent);
   GameIOMsg->io_Data     =   (APTR)GameBuffer;
   GameEventData          =   (struct InputEvent *)GameBuffer;
   SetControllerType(GPCT_MOUSE);
   GameIOMsg->io_Command  =   GPD_SETTRIGGER;
   GameIOMsg->io_Data     =   (APTR)&MouseTrigger;
   DoIO(GameIOMsg);
   
   GameIOMsg->io_Command  =   GPD_READEVENT;
   GameIOMsg->io_Data     =   (APTR)GameBuffer;

   GameIOMsg->io_Length   =   sizeof(struct InputEvent);
   SendIO(GameIOMsg);
   }   /* end of SetUpMouse */

/************************************************************************
 *
 * MaybeMouseMoved
 *
 ************************************************************************/
void MaybeMouseMoved()
   {
   SetMouseXY(GameEventData->ie_X,GameEventData->ie_Y);
   }   /* end of MaybeMouseMoved */
   
/************************************************************************
 *
 * SetControllerType
 *
 ************************************************************************/
SetControllerType(Type)
short   Type;
   {
   int res;
   GameIOMsg->io_Command   =   GPD_SETCTYPE;
   *GameData               =   Type;
   res=DoIO(GameIOMsg);
   return(0);
   }   /* end of SetControllerType */
   
   

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


static   struct IOStdReq IOReq;
static   struct MsgPort  IOReqPort;

void SetMousePort()
   {
   UBYTE   Port = AmigaMousePort;
   long   DoIOError;

    /* Open the input device */
   if (OpenDevice("input.device", 0, &IOReq, 0) != 0)
      {
      DoExit("can´t open input.device for mouse change");
      }

    /* set up the message Port in the I/O request */
   IOReqPort.mp_Node.ln_Type = NT_MSGPORT;
   IOReqPort.mp_Flags = 0;
   if( (IOReqPort.mp_SigBit = AllocSignal(-1)) < 0 )
      {
      CloseDevice(&IOReq);
      DoExit("can´t get Signal for mouse change");;
      }
   IOReqPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
   NewList(&IOReqPort.mp_MsgList);
   IOReq.io_Message.mn_ReplyPort = &IOReqPort;

    /* initiate the I/O request to change the mouse Port */
   IOReq.io_Command = IND_SETMPORT;
   IOReq.io_Data    = (APTR)&Port;
   IOReq.io_Length  = 1;
   DoIOError        = DoIO(&IOReq);

    /* clean up */
   CloseDevice(&IOReq);
   FreeSignal(IOReqPort.mp_SigBit);
   if( DoIOError )
      DoExit("can't switch Intuition mouse port");
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

struct InputEvent *myHandler(EventList,data)
struct InputEvent *EventList;
APTR data;
   {

   register struct InputEvent *theEvent  = EventList;
   register struct InputEvent *prevEvent = NULL;
   register int  PChanged = 0;
   Forbid();
   while(theEvent)
      {
      if(   (theEvent->ie_Class == IECLASS_RAWKEY)
            &&
            (theEvent->ie_Qualifier & IEQUALIFIER_LCOMMAND)
            &&
            (theEvent->ie_Code == TOGGLEPORT)
         )
         {
         PChanged       = 1 - PChanged;
         AmigaMousePort = 1 - AmigaMousePort;   /* toggle */
         if( !prevEvent )
            EventList =   theEvent->ie_NextEvent;
         else
            prevEvent->ie_NextEvent =
               theEvent->ie_NextEvent;
         }
      else
         prevEvent = theEvent;
      theEvent  = theEvent->ie_NextEvent;
      }
   Permit();
   if( PChanged )
      Signal(MyTask,MouseSignal);
   return(EventList);
   }   /* end of myHandler */

/************************************************************************
 *
 * SetPortText
 *
 ************************************************************************/
static   struct IntuiText   Text =
   {
   0,1,      /* Pens */
   JAM2,
   0,0,      /* left, top */
   NULL,     /* font */
   NULL,     /* text */
   NULL,     /* next */
   };

static   char   Title[] = " PC ";

void SetPortText(PortText)
char   *PortText;
   {
   struct Screen      *WorkScreen;
   struct Window      *WorkWindow;
   char               Where[128];
   sprintf(Where,PCTitle,VersionId,PortText);
   Text.IText = Where;
   Forbid();

   for(   WorkScreen = IntuitionBase->FirstScreen;
          WorkScreen;
          WorkScreen = WorkScreen->NextScreen)
      {
      for(   WorkWindow = WorkScreen->FirstWindow;
             WorkWindow;
             WorkWindow = WorkWindow->NextWindow)
         {
         if( strncmp(
                  WorkWindow->Title,
                  Title,
                  strlen(Title)
               ) == 0 )
            {
            PrintIText(
                     WorkWindow->RPort,
                     &Text,
                     (strlen(WorkWindow->Title)<<3)+30,
                     1);
            }
         }
      }
   
   Permit();
   }   /* end of SetPortText */
