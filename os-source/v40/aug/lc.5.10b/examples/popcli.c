/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors.                                                    */
/* | o  | ||    Dave Baker     Ed Burnette  Stan Chow    Jay Denebeim        */
/* |  . |//     Gordon Keener  Jack Rouse   John Toebes  Doug Walker         */
/* ======          BBS:(919)-471-6436      VOICE:(919)-469-4210              */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * VERY loosely based on the input.device example by Rob Peck, 12/1/85
 *
 * Compile as
 *   lc -v -Lcd -tb -O popcli
 */

/* * * * * * * * * INCLUDE FILES * * * * * * * * * * * */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/devices.h>
#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <string.h>
typedef void (*vfunc)();

/* * * * * * * * * * * CONSTANTS * * * * * * * * * * * * */
#define PORTNAME     "POPCLI_III.port"
#define TIMEINTERVAL 1L      /* in seconds */
#define DEFTIME      300     /* two minute timeout */
#define MAXCMD       200
#define DEFKEY    0x45
#define DEFCMD    "NEWCLI >NIL: <NIL:"
#define KILLMSG "\x9B""1mPOPCLI III\x9B""0m Terminating\n"
#define BANNER "\x9B""0;33mPOPCLI III\x9B""0m by John Toebes - Copyright \xA9 1987 The Software Distillery\n 207 Livingstone Dr, Cary NC 27513   BBS:(919)-460-7430\n"
#define BANNER1 "Usage: \x9B""1mPOPCLI\x9B""0m [secs [command]]\nsecs is number of seconds before blanking screen\ncommand is to be executed when Left Amiga-Escape is pressed\n"
/* * * * * * * * * * * GLOBAL VARIABLES * * * * * * * * * */
typedef struct
   {
   struct Task          *buddy;
   ULONG                 creatclisig;
   ULONG                 unblanksig;
   ULONG                 noevents;
   short                 creatsignum;
   short                 blanksignum;
   short                 key;
   struct Screen        *blankscreen;
   } GLOBAL_DATA;

struct MsgPort *FindPort(), *CreatePort();
void DeletePort();

struct OURMSG {
 struct Message msgpart;
 short key;
 short interval;
 char cmd[MAXCMD];
 };

/* Declarations for CBACK */
extern BPTR _Backstdout;         /* standard output when run in background */
long _BackGroundIO = 1;          /* Flag to tell it we want to do I/O      */
long _stack = 4000;              /* Amount of stack space our task needs   */
char *_procname = "PopCLI III";  /* The name of the task to create         */
long _priority = 20;             /* The priority to run us at              */

/************************************************************************/
/* the handler subroutine - called through the handler stub             */
/************************************************************************/
struct InputEvent * __asm HandlerInterface(register __a0 struct InputEvent *ev, 
                                    register __a1 GLOBAL_DATA *gptr)
   {
   register struct InputEvent *ep, *laste;

   /* run down the list of events to see if they pressed the magic button */
   for (ep = ev, laste = NULL; ep != NULL; ep = ep->ie_NextEvent)
      {
      if ((ep->ie_Class == IECLASS_RAWKEY)    &&
          (ep->ie_Code  == gptr->key)         &&
          (ep->ie_Qualifier & IEQUALIFIER_LCOMMAND))
         {
         /* we can handle this event so take it off the chain */
         if (laste == NULL)
            ev = ep->ie_NextEvent;
         else
            laste->ie_NextEvent = ep->ie_NextEvent;
         /* now tell him to create the new cli */
         Signal(gptr->buddy, gptr->creatclisig);
         }
      else
         laste = ep;

      if (ep->ie_Class != IECLASS_TIMER)
         {
         gptr->noevents = 0;
         if (gptr->blankscreen != NULL)
            Signal(gptr->buddy, gptr->unblanksig);
         }
      }

   /* pass on the pointer to the event */
   return(ev);
   }

/* * * * * * * * * * * EXTERNAL ROUTINES * * * * * * * * * */
struct IntuitionBase *IntuitionBase;
struct GfxBase       *GfxBase;
struct DosLibrary    *DosBase;

extern struct Custom custom;

struct Custom *customptr = &custom;
#define custom (*customptr)

struct NewScreen      NewScreen = 
   { 0, 0, 320, 30, 1, 0, 1, NULL, CUSTOMSCREEN, NULL, NULL, NULL, NULL };

extern struct MsgPort  *CreatePort();
struct IOStdReq *CreateIOReq(struct MsgPort *, int);
void DeleteIOReq(struct IOStdReq *);

/************************************************************************/
/* Queue a timer to go off in a given number of seconds                 */
/************************************************************************/
void QueueTimer(tr,seconds)
struct timerequest *tr;
ULONG seconds;
   {
   tr->tr_node.io_Command = TR_ADDREQUEST;   /* add a new timer request */
   tr->tr_time.tv_secs =  seconds;        	/* seconds */
   tr->tr_time.tv_micro = 0;
   SendIO( (struct IORequest *)tr );
   }

/************************************************************************/
/* the main program to do the popcli stuff                              */
/************************************************************************/
void _main(cmd)
char *cmd;
   {
   struct MsgPort *port;
   int stay = 0;
   struct OURMSG *msg;

   char cmdstr[MAXCMD];
   short key, timeout;
   BPTR  nullfh;
   ULONG sig, timersig;
   struct timerequest *timerreq;
   struct MsgPort     *timerport;
   struct MsgPort     *inputDevPort;
   struct IOStdReq    *inputRequestBlock;
   struct Interrupt      handlerStuff;
   GLOBAL_DATA global;

   global.creatsignum  = -1;
   global.blanksignum  = -1;
   timerreq            = NULL;
   timerport           = NULL;
   inputDevPort        = NULL;
   inputRequestBlock   = NULL;

   /* now see if we are already installed */
   if ((port = FindPort(PORTNAME)) == NULL)
      {
      stay = 1; /* remember to hang around when we are done */
      /* not installed, we need to install our own port */
      if ((port = CreatePort(PORTNAME,0)) == NULL)
         goto abort;
      }

   /* now send the parameter to the waiting program */
   if ((msg = (struct OURMSG *)
              AllocMem(sizeof(struct OURMSG), MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
      goto abort;

   /* fill in the message information */
   msg->msgpart.mn_Length = sizeof(struct OURMSG);

   strcpy(cmdstr, DEFCMD);

   /* if we were run from CLI then output our banner and process parameters */
   if (cmd && *cmd)
      {
      /* display our copyright */
      if (stay && _Backstdout)
         Write(_Backstdout, BANNER, sizeof(BANNER));

      /* skip over any leading spaces in the command line */
      while(*cmd != ' ')
         cmd++;
      while(*cmd == ' ')
         cmd++;

      /* see if they are trying to kill us. */
      if (!stricmp(cmd, "QUIT\n"))
         {
         timeout = -1;
         if (_Backstdout)
            Write(_Backstdout, KILLMSG, sizeof(KILLMSG));
         }
      else if ((*cmd < ' ') && _Backstdout)
         {
         Write(_Backstdout, BANNER1, sizeof(BANNER1));
         key = DEFKEY;
         timeout = DEFTIME;
         msg->cmd[0] = 0;  /* don't change the command string */
         }
      else
         {
         /* see if they gave us a number to control the interval of checking */
         timeout = 0;
         key = 0;

         while ((*cmd >= '0') && (*cmd <= '9'))
            /* Multiply it by 10 without using a subroutine */
            timeout = (timeout*10) + *cmd++ - '0';

         if (timeout <= 0)
            timeout = DEFTIME;

         while (*cmd == ' ') cmd++;

         /* see if they gave us a number to control the interval of checking */
         while ((*cmd >= '0') && (*cmd <= '9'))
            /* Multiply it by 10 without using a subroutine */
            key = (key*10) + *cmd++ - '0';

         if (key == 0)
            key = DEFKEY;

         while (*cmd == ' ') cmd++;
         strcpy(msg->cmd, cmd);
         msg->cmd[strlen(cmd)-1] = 0;  /* wipe out the EOL character */
         }
      }
   else
      {
      timeout = DEFTIME;
      key = DEFKEY;
      msg->cmd[0] = 0;
      }

   msg->interval = timeout;
   msg->key = key;

   PutMsg(port,(struct Message *)msg);

   if (!stay) goto abort;

   /* Lst the original window go away */
   if (_Backstdout)
      Close(_Backstdout);

   _Backstdout = 0;

   global.blankscreen = NULL;

   global.buddy = FindTask(0);
   global.noevents = 0;

   /* set the input and output streams to 0 so execute doen't complain */
   nullfh = Open("NIL:", MODE_NEWFILE);

   if (((inputDevPort = CreatePort(0,0)) == NULL)                          ||

      ((inputRequestBlock =
          CreateIOReq(inputDevPort, sizeof(struct IOStdReq))) == NULL)     ||

      ((timerport = CreatePort(0,0)) == NULL)                              ||

      ((timerreq  = (struct timerequest *)
          CreateIOReq(timerport, sizeof(struct timerequest))) == NULL)     ||

      ((global.creatsignum = AllocSignal(-1)) == -1)                       ||

      ((global.blanksignum = AllocSignal(-1)) == -1)                       ||

      ((GfxBase = (struct GfxBase *)
                  OpenLibrary("graphics.library", 0)) == NULL)             ||

      ((IntuitionBase = (struct IntuitionBase *)
                        OpenLibrary("intuition.library", 0)) == NULL)      ||
      OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timerreq, 0)  ||

      OpenDevice("input.device",0,(struct IORequest *)inputRequestBlock,0))

      goto abort;
   handlerStuff.is_Data = (APTR)&global;
   handlerStuff.is_Code = (vfunc)HandlerInterface;
   handlerStuff.is_Node.ln_Pri = 51;

   timersig            = (1 << timerport->mp_SigBit);
   global.creatclisig  = 1 << global.creatsignum;
   global.unblanksig   = 1 << global.blanksignum;

   inputRequestBlock->io_Command = IND_ADDHANDLER;
   inputRequestBlock->io_Data    = (APTR)&handlerStuff;

   DoIO((struct IORequest *)inputRequestBlock);

   QueueTimer(timerreq, TIMEINTERVAL);

   for(;;)         /* FOREVER */
      {
      sig = Wait( global.creatclisig | global.unblanksig | timersig );

      /* see if they asked us to change the interval */
      if ((msg = (struct OURMSG *)GetMsg(port)) != NULL)
         {
         if (msg->cmd[0]) strcpy(cmdstr, msg->cmd);
         global.key = msg->key;
         timeout    = msg->interval;
         FreeMem((char *)msg, msg->msgpart.mn_Length);

         if (global.key == 0) goto abort;
         }

      if ((sig & global.unblanksig) && global.blankscreen)
         {
         CloseScreen(global.blankscreen);
	 ON_DISPLAY
         global.blankscreen = NULL;
         }

      if (sig & global.creatclisig)
         {
         WBenchToFront();
         (void)Execute(cmdstr,nullfh,nullfh);
         }

      if (sig & timersig)
         {
         /* get rid of the message */
         (void)GetMsg(timerport);
         QueueTimer(timerreq, TIMEINTERVAL);

         if ((global.noevents++ >= timeout) && (global.blankscreen == NULL))
            {
            if ( (global.blankscreen = OpenScreen(&NewScreen)) != NULL)
               {
               SetRGB4(&(global.blankscreen->ViewPort), 0, 0, 0, 0);
               OFF_DISPLAY
               }
            }
         }

      /* Force our screen to front on a regular basis to handle something   */
      /* that might put their stuff in front of us while we are supposed to */
      /* Be blanking the screen.                                            */
      if (global.blankscreen)
         {
#if 0
         /* Unfortunately doing this causes the screen to flash momentarily */
         /* Which makes it look ugly.  Perhaps someone can come up with a   */
         /* Better way to force us upfront..                                */
         ScreenToFront(global.blankscreen);
#endif
         OFF_DISPLAY
         }
      }

abort:
   if (timerreq != NULL)
      {
      if (timerreq->tr_node.io_Device != NULL)
         CloseDevice((struct IORequest *)timerreq);
      DeleteIOReq((struct IOStdReq *)timerreq);
      }
   if (inputRequestBlock != NULL)
      {
      if (inputRequestBlock->io_Device != NULL)
         {
         inputRequestBlock->io_Command = IND_REMHANDLER;
         inputRequestBlock->io_Data = (APTR)&handlerStuff;
         DoIO((struct IORequest *)inputRequestBlock);

         CloseDevice((struct IORequest *)inputRequestBlock);
         }
      DeleteIOReq(inputRequestBlock);
      }
   if (timerport != NULL)          DeletePort(timerport);
   if (global.creatsignum != -1)   FreeSignal(global.creatsignum);
   if (global.blanksignum != -1)   FreeSignal(global.blanksignum);
   if (global.blankscreen != NULL) CloseScreen(global.blankscreen);
   if (IntuitionBase != NULL)      CloseLibrary((struct Library *)IntuitionBase);
   if (GfxBase != NULL)            CloseLibrary((struct Library *)GfxBase);
   if (inputDevPort != NULL)       DeletePort(inputDevPort);
   if (_Backstdout)                Close(_Backstdout);
   if (stay && (port != NULL))     DeletePort(port);
   if (nullfh)                     Close(nullfh);
   }

void MemCleanup(){}

struct MsgPort *CreatePort(name, pri)
char *name;
int pri;
{
   UBYTE sigbit;
   register struct MsgPort *port;

   if ((sigbit = AllocSignal(-1)) == -1)
      return((struct MsgPort *)0);

   if ((port = (struct MsgPort *)AllocMem(sizeof(struct MsgPort),
                        MEMF_CLEAR|MEMF_PUBLIC)) == 0)
      {
      FreeSignal(sigbit);
      return((struct MsgPort *) (0));
      }
   port->mp_Node.ln_Name = name;
   port->mp_Node.ln_Pri = pri;
   port->mp_Node.ln_Type = NT_MSGPORT;
   port->mp_Flags = PA_SIGNAL;
   port->mp_SigBit = sigbit;
   port->mp_SigTask = (struct Task *)FindTask(0);
   AddPort(port);
   return(port);
}

void DeletePort(port)
struct MsgPort *port;
{
RemPort(port);
FreeSignal(port->mp_SigBit);
FreeMem((char *)port,sizeof(struct MsgPort));
}

struct IOStdReq *
CreateIOReq(port, size)
struct MsgPort *port;
int size;
{
   struct IOStdReq *ioReq;

   if ((ioReq = (struct IOStdReq *)
                AllocMem(size, MEMF_CLEAR | MEMF_PUBLIC)) != NULL)
      {
      ioReq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
      ioReq->io_Message.mn_Node.ln_Pri  = 0;
      ioReq->io_Message.mn_Length       = size;
      ioReq->io_Message.mn_ReplyPort    = port;
      }
   return(ioReq);
}

void DeleteIOReq(ioReq)
struct IOStdReq *ioReq;
{
ioReq->io_Message.mn_Node.ln_Type = 0xff;
ioReq->io_Device = (struct Device *) -1;
ioReq->io_Unit = (struct Unit *) -1;

FreeMem( (char *)ioReq, ioReq->io_Message.mn_Length);
}
