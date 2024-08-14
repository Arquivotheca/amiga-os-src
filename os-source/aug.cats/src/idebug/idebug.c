;/* idebug.c - Execute me to compile me with SAS C 5.10
LC -b0 -cfistq -v -j73 idebug.c
Blink FROM LIB:astartup.obj,idebug.o,handint.o TO idebug LIBRARY LIB:Amiga.lib,LIB:LC.lib,LIB:debug.lib ND
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <exec/devices.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>

extern struct Library *SysBase, *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <string.h>

#define MINARGS 1

UBYTE *vers = "\0$VER: idebug 39.1";
UBYTE *Copyright = 
  "idebug v39.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: idebug";


/**********    debug macros     **********
#define MYDEBUG	 1
*/

void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);

#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/


extern void   HandlerInterface(void);  /* assembler interface */

int    openedID;
struct MsgPort   *inputPort;
struct IOStdReq  *inputReq;

struct Interrupt handlerStuff;

void cleanexit(char *s, int e);
void unInstall(void);
struct InputEvent *myhandler(struct InputEvent *ev1, void *nothing);


/* C handler routine */
struct InputEvent *myhandler(struct InputEvent *ev1, void *nothing)
   {
   struct InputEvent *ev;

   for (ev=ev1; ev; ev=ev->ie_NextEvent)
      	{
	if(ev->ie_Class == IECLASS_TIMER)
		kputchar('.');
	else
		kprintf("\nC=%02lx s=%02lx c=%04lx q=%04lx ",
	   ev->ie_Class,ev->ie_SubClass,ev->ie_Code,ev->ie_Qualifier);
      	}
   return(ev1);
   }

void main(int argc, char **argv)
    {
    int error;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	cleanexit("",RETURN_OK);
	}


   /* Get ready to add our handler */
   if(!(inputPort=(struct MsgPort *)CreatePort(0,0)))
      cleanexit("Can't CreatePort",RETURN_FAIL);
   if(!(inputReq=(struct IOStdReq *)CreateStdIO(inputPort)))
      cleanexit("Can't CreateStdIO", RETURN_FAIL);

   handlerStuff.is_Data = NULL;
   handlerStuff.is_Code = HandlerInterface;  /* assem entry */
   handlerStuff.is_Node.ln_Pri = 51;         /* above Intuition */

   if(error = OpenDevice("input.device",0,inputReq,0))
      cleanexit("Can't open input device",RETURN_FAIL);

   openedID = 1;

   inputReq->io_Command = IND_ADDHANDLER;
   inputReq->io_Data = (APTR)&handlerStuff;
      
   DoIO(inputReq);

   printf("idebug installed - CTRL-C to exit\n");

   Wait(SIGBREAKF_CTRL_C);

   cleanexit("",RETURN_OK);
   }


void cleanexit(char *s, int e)
   {
   if(*s) printf("%s\n",s);
   unInstall();
   exit(e);
   }

void unInstall(void)
   {
   if (openedID)
      {
      /* remove the handler from the chain */
      inputReq->io_Command = IND_REMHANDLER;
      inputReq->io_Data = (APTR)&handlerStuff;
      DoIO(inputReq);
      /* close the input device */
      CloseDevice(inputReq);
      }

   /* delete the IO request, port, FreeSignal */
   if (inputReq)     DeleteStdIO(inputReq);
   if (inputPort)    DeletePort(inputPort);
   printf("idebug removed\n");
   }




