/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1986 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors.                                                    */
/* | o  | ||   Dave Baker    Ed Burnette        Stan Chow          BBS:      */
/* |  . |//    Jay Denebeim  Gordon Keener      Jack Rouse   (919)-471-6436  */
/* ======      John Toebes   Mary Ellen Toebes  Doug Walker                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* This program watches the first 0x100 bytes of memory for random trashing, */
/* attempts repair of the damage and then puts up an alert indicating the    */
/* damage that was done.  Many thanks to EA for suggesting this program at   */
/* the developer's conference.                                               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * INCLUDE FILES * * * * * * * * * * * */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/devices.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <libraries/dosextens.h>
#include <string.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/exec.h>

/* * * * * * * * * * * STRUCTURES * * * * * * * * * * * * */
typedef struct stomped
   {
   long data;           /* value that was thrown into address wildly */
   long address;        /* address that was changed */
   } STOMPED;

struct OURMSG {
 struct Message msgpart;
 int interval;
 };

/* * * * * * * * * * * CONSTANTS * * * * * * * * * * * * */
#define TIMEINTERVAL 2500L      /* in micro seconds */
#define LOWLIMIT     20L        /* smallest safe interval */
#define BANNER "\x9B0;33mMemWatch II\x9B0m by John Toebes - Copyright \xa9 1987 The Software Distillery\n 235 Trillingham Lane, Cary NC 27513   BBS:(919)-471-6436\n"
#define BANNER1 "Usage: \x9B1mMemWatch\x9B0m [n]  - set interval between watch checks (Default 2500ms)\n       \x9B1mMemWatch\x9B0m QUIT - To terminate MemWatch\n"
#define PORTNAME "MemWatch_port"
#define KILLMSG "Terminating MemWatch\n"

/* * * * * * * * * * * EXTERNAL ROUTINES * * * * * * * * * */
struct IntuitionBase *IntuitionBase = NULL;

extern void XCEXIT(long);
extern void SaveMem(void);
extern int ValidateMem(struct stomped *);

struct DosLibrary *DOSBase;
int MathTransBase, MathBase;

/* Declarations for CBACK */
extern long _Backstdout;         /* standard output when run in background */
long _BackGroundIO = 1;          /* Flag to tell it we want to do I/O      */
long _stack = 4000;              /* Amount of stack space our task needs   */
char *_procname = "MemWatch II"; /* The name of the task to create         */
long _priority = 20;             /* The priority to run us at              */

/* * * * * * * * * * * Alert definition structure* * * * * * * * * */
/* we want to display the message:                                 */
/*  MemWatch II - Copyright c 1987 By the Software Distillery      */
/* Someone stepped on Low memory $nnnnnnnn with $nnnnnnnn cccc!    */
/*                                                                 */
/* Left Button to correct location       Right button to continue  */
/* 345678901234567890123456789012345678901234567890123456789012    */
/*        11111111112222222222333333333344444444445555555555666    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ALERTHEIGHT 41          /* height of alert in pixals */
#define ADDR_OFF 96             /* location of address substitution */
#define DATA_OFF 111		/* Location of data val substitution */
#define CHAR_OFF 120		/* Location of data text substitution */
static char AlertString[] = {
0, 92,                          /* 2 byte x absolute offset of first string */
10,                             /* 1 byte y absolute offset of first string */
'M', 'e', 'm', 'W', 'a', 't', 'c', 'h', ' ', 'I', 'I', ' ',
'-', ' ',
'C', 'o', 'p', 'y', 'r', 'i', 'g', 'h', 't',
' ', '\xa9', ' ', '1', '9', '8', '7', ' ',
'B', 'y', ' ',
'T', 'h', 'e', ' ',
'S', 'o', 'f', 't', 'w', 'a', 'r', 'e', ' ',
'D', 'i', 's', 't', 'i', 'l', 'l', 'e', 'r', 'y',
0,                                              /* null terminator on string */
1,                              /* flag to indicate another alert string */
0, 84,                          /* 2 byte x absolute offset of next string  */
18,                             /* 1 byte y absolute offset of next string  */
'S', 'o', 'm', 'e', 'o', 'n', 'e', ' ',                  /* message string  */
'S', 't', 'e', 'p', 'p', 'e', 'd', ' ',
'o', 'n', ' ', 'L', 'o', 'w', ' ',
'm', 'e', 'm', 'o', 'r', 'y', ' ',
'$', 'h', 'h', 'h', 'h', 'h', 'h', 'h', 'h', ' ', /* address we substitute */
'w', 'i', 't', 'h', ' ',
'$', 'h', 'h', 'h', 'h', 'h', 'h', 'h', 'h', ' ', /* data we substitute */
'c', 'c', 'c', 'c', '!',                          /* chars we substitute */
0,                                              /* null terminator on string */
1,                              /* flag to indicate another alert string */
0, 76,                         /* 2 byte x absolute offset of second string */
34,                             /* 1 byte y absolute offset of second string */
'L', 'e', 'f', 't', ' ',                /* second message string */
'B', 'u', 't', 't', 'o', 'n', ' ',
't', 'o', ' ',
'c', 'o', 'r', 'r', 'e', 'c', 't', ' ',
'l', 'o', 'c', 'a', 't', 'i', 'o', 'n',
' ', ' ', ' ', ' ', ' ', ' ', ' ',
'R', 'i', 'g', 'h', 't', ' ',
'b', 'u', 't', 't', 'o', 'n', ' ',
't', 'o', ' ',
'c', 'o', 'n', 't', 'i', 'n', 'u', 'e', 
0,                                              /* null terminator on string */
0 };                            /* flag to indicate no more strings */

/************************************************************************/
/* The main program to watch the memory                                 */
/************************************************************************/
void _main(cmd)
char *cmd;
   {
   struct MsgPort *port;
   int stay = 0;
   struct OURMSG *msg;

   register struct timerequest *timerreq = NULL;
                                    /* request structure for timer waits*/
   struct stomped rslt;             /* information on memory stomps */
   register int i;                  /* general index variable */
   register long v;                 /* temporary for formatting display */
   register long interval = TIMEINTERVAL;
                                    /* how long to wait between checks */
   register char *p;                /* display formatting index */

   /* NOTE: The declarations for i anv v MUST come before that of interval */
   /* because DOIO trashes D6 and D7 which are the first ones selected for */
   /* register variables.  It doesn't matter if i and v are trashed but    */
   /* interval MUST be maintained across the lifetime of the loop          */

   /* now see if we are already installed */
   if ((port = FindPort(PORTNAME)) == NULL)
      {
      stay = 1; /* remember to hang around when we are done */
      /* not installed, we need to install our own port */
      if ((port = CreatePort(PORTNAME,0)) == NULL)
         goto quitit;
      }

   /* now send the parameter to the waiting program */
   if ((msg = (struct OURMSG *)
              AllocMem(sizeof(struct OURMSG), MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
      {
      if (stay) DeletePort(port);
         goto quitit;
      }

   /* fill in the message information */
   msg->msgpart.mn_Length = sizeof(struct OURMSG);

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

      /* now see if they gave us a number to control the interval of checking */
      interval = 0;

      while ((*cmd >= '0') && (*cmd <= '9'))
         /* Multiply it by 10 without using a subroutine */
         interval = (((interval << 2) + interval) << 1) + *cmd++ - '0';

      /* if they gave us nothing (or something we didn't parse well */
      /* such as a VERY large number or just plain trash, give them a */
      /* short usage message for the program */
      if (!stricmp(cmd, "QUIT\n"))
         {
         interval = -1;
         if (_Backstdout)
            Write(_Backstdout, KILLMSG, sizeof(KILLMSG));
         }
      else
         {
         if (_Backstdout && (interval <= 0))
            Write(_Backstdout, BANNER1, sizeof(BANNER1));
         /* just incase we got a very low number (or a 0) from the command   */
         /* line apply a minimum rule to keep from killing the system        */
         /* performance note that since tasks switch at about every 20mili   */
         /* seconds or so (not sure where that information is from) making it*/
         /* less than 20000 really isn't much of a gain (but it feels nice)  */
         if (interval < LOWLIMIT) interval = LOWLIMIT;
         }
      }
   else
      interval = 0;

   msg->interval = interval;

   PutMsg(port,(struct Message *)msg);

   if (!stay)
      {
quitit:
      if (_Backstdout)
         Close(_Backstdout);
      _Backstdout = 0;
      XCEXIT(1);
      }

   if (_Backstdout)
      Close(_Backstdout);

   /* open up intuition - we only use this for the alert but we must do it */
   /* anyway */
   if ((IntuitionBase = (struct IntuitionBase *)
                        OpenLibrary("intuition.library", 0)) == NULL)
      goto abort;

   /* create a request structure to send the messages with */
   if ((timerreq = (struct timerequest *)
                    AllocMem(sizeof(struct timerequest),
                             MEMF_CLEAR | MEMF_PUBLIC)) == NULL)
      goto abort;

   /* fill in the struture with what we are dealing with */
   timerreq->tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
   timerreq->tr_node.io_Message.mn_Node.ln_Pri = 0;
   timerreq->tr_node.io_Message.mn_ReplyPort =
       &(((struct Process *)FindTask(NULL))->pr_MsgPort);

   /* and open us a timer.  Note that we are using VBLANK since it is the */
   /* lowest system overhead.  We are not critical on the timing and only */
   /* want to run as often as possible without killing the system */
   if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest*)timerreq, 0))
      goto abort;

   /* let our assembler stub squirel away a copy of the low memory */
   SaveMem();

   while(1)
      {
      /* see if they asked us to change the interval */
      if ((msg = (struct OURMSG *)GetMsg(port)) != NULL)
         {
         interval = msg->interval;
         FreeMem((char *)msg, msg->msgpart.mn_Length);
         if (interval < 0) goto done;
         }

      /* Use the timer to wait our required number of seconds */
      timerreq->tr_node.io_Command = TR_ADDREQUEST;
      timerreq->tr_time.tv_secs =  0;           /* seconds */
      timerreq->tr_time.tv_micro = interval;    /* micro seconds */
      DoIO((struct IORequest *)timerreq);

      /* Now have our buddy check and fix any memory */
      /* if something was wrong, he will tell us about it */
      if (ValidateMem(&rslt))
         {
         /* format the failure data to be displayed */
         *(long *)(&AlertString[CHAR_OFF]) = rslt.data;

         /* replace any nulls so they don't screw up the message */
         for (i=CHAR_OFF; i<CHAR_OFF+4; i++)
            if (!AlertString[i]) AlertString[i] = '.';

         /* format the address for them */
         p = &AlertString[ADDR_OFF+7];
         v = rslt.address;
         for (i = 7; i>= 0; i--)
            {
            *p-- = "0123456789ABCDEF"[v&15];
            v >>= 4;
            }

         /* format the data value for them */
         p = &AlertString[DATA_OFF+7];
         v = rslt.data;
         for (i = 7; i>= 0; i--)
            {
            *p-- = "0123456789ABCDEF"[v&15];
            v >>= 4;
            }

         /* put up a requester to indicate it failed */
         if (!DisplayAlert(0xBADC0DE, AlertString, ALERTHEIGHT))
            {
            /* They want us to patch it up for them... */
            *(long *)(rslt.address) = rslt.data;
            SaveMem();     
            }
         }
      }

done:
   CloseDevice((struct IORequest *)timerreq);
abort:
   if (timerreq != NULL) FreeMem ((char *)timerreq, sizeof(struct timerequest));
   if (IntuitionBase != NULL) CloseLibrary((struct Library *)IntuitionBase);
   DeletePort(port);
   XCEXIT(-1);
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
