;/* devmon.c - Execute me to compile me with Lattice 5.04
if not exists devmona.o
HX68 -a devmona.asm -i INCLUDE: -o devmona.o
;Asm -iH:include/ -odevmona.o devmona.asm
endif
if not exists devmon.o
LC -b0 -cfistq -v -j73 devmon.c
endif
Blink FROM LIB:astartup.obj,devmon.o,devmona.o TO devmon LIBRARY LIB:Amiga.lib,LIB:LC.lib,LIB:debug.lib
quit
*/

/* Devmon.c
 *
 * (c) Copyright 1990-92  Commodore-Amiga, Inc.  All Rights Reserved
 *
 */

/*
#define MYDEBUG
*/

char *vers = "\0$VER: devmon 37.12";
char Copyright[] =
 "devmon v37.12\n(c) Copyright 1990-92 Commodore-Amiga, Inc.  All Rights Reserved";
char *usage = "USAGE: devmon name.device unitnum [remote] [hex] [allunits] [full]\n";

#include <exec/types.h>
#include <exec/memory.h> 
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/execbase.h>
#include <libraries/dos.h> 
#include <libraries/dosextens.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

typedef ULONG (*PFU)();

extern int  sprintf __ARGS((char *,char *,...));
extern void kprintf __ARGS((char *,...));
extern void kputs   __ARGS((char *));
extern void printf  __ARGS((char *,...));

/* the assembler entries in devmona.asm */
extern VOID  mySendIO();
extern VOID  myCheckIO();
extern VOID  myDoIO();    
extern VOID  myWaitIO();
extern VOID  myAbortIO();
extern VOID  myReplyMsg();
extern VOID  myOpenDevice();
extern VOID  myCloseDevice();

extern VOID  myBEGIN(); 
extern VOID  myABORT(); 
extern VOID  myOPEN();    
extern VOID  myCLOSE();   
extern VOID  myEXPUNGE(); 

extern ULONG RealOPEN, RealCLOSE, RealEXPUNGE, RealBEGIN, RealABORT;
extern ULONG RealSendIO, RealCheckIO, RealDoIO, RealReplyMsg, RealWaitIO;
extern ULONG RealAbortIO, RealOpenDevice, RealCloseDevice;

extern LONG usecnt;

extern struct ExecBase  *SysBase;

/* exec wedges */
void MySendIO(struct IOStdReq *ior);
void MyCheckIO(struct IOStdReq *ior);
void MyCheckIORts(struct IOStdReq *ior);
void MyDoIO(struct IOStdReq *ior);
void MyDoIORts(struct IOStdReq *ior);
void MyReplyMsg(struct IOStdReq *ior);
void MyWaitIO(struct IOStdReq *ior);
void MyAbortIO(struct IOStdReq *ior);
void MyOpenDevice(struct IOStdReq *ior, LONG unitn);
void MyDoOpenDevRts(struct IOStdReq *ior,LONG result);
void MyCloseDevice(struct IOStdReq *ior);
void MyCloseDevRts(struct IOStdReq *ior);

/* device wedges */
void MyBEGIN(struct IOStdReq *ior);
void MyABORT(struct IOStdReq *ior);
void MyOPEN(struct IOStdReq *ior, LONG unitn);
void MyCLOSE(struct IOStdReq *ior);
void MyEXPUNGE(void);

void genericOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd, UBYTE *res);
void iorOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd);

int atoi( char *s );
void bye(UBYTE *s, LONG err);
void cleanup(void);
BOOL strEqu(UBYTE *p, UBYTE *q); 
int strLen(UBYTE *s);
void strCpy(UBYTE *to,UBYTE *from);
LONG getS(UBYTE *s);
LONG getchar(void);
UBYTE *taskname(struct Task *task);
LONG MyVBlank(void);
void StartVBlank(void);
void StopVBlank(void);

#define BREAKSIG	SIGBREAKF_CTRL_C
#define WFULLSIG	SIGBREAKF_CTRL_E
#define FLUSHSIG	SIGBREAKF_CTRL_F

#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c)) 
#define HIGHER(x,y)     ((x)>(y)?(x):(y))

#define WBUFLEN   64000L
#define EXTRALEN    512L

#define REQSIZE    120L    /* should be big enough to open any OpenDevice */

#define DEV_OPEN     LIB_OPEN
#define DEV_CLOSE    LIB_CLOSE
#define DEV_EXPUNGE  LIB_EXPUNGE
/*      DEV_BEGINIO  (-30)       defined in exec/io.h */

#define EXEC_SENDIO	(-462)
#define EXEC_CHECKIO	(-468)
#define EXEC_DOIO	(-456)
#define EXEC_REPLYMSG	(-378)
#define EXEC_WAITIO	(-474)
#define EXEC_ABORTIO	(-480)
#define EXEC_OPENDEV	(-444)
#define EXEC_CLOSEDEV	(-450)

ULONG  NewOPEN, NewCLOSE, NewEXPUNGE, NewBEGIN, NewABORT;
ULONG  NewSendIO, NewCheckIO, NewDoIO, NewReplyMsg, NewWaitIO, NewAbortIO;
ULONG  NewOpenDevice, NewCloseDevice;

char *noMem      = "Out of memory\n";
char *portName   = "cas_DEVMON_PORT";

char *prevTaskName = NULL;
char *deviceName;

LONG unitnum;
struct Unit *unitptr;
ULONG allunits = 0;

char mainTaskName[40];
char *wbuf = 0;

struct Library *TheDevice;
struct Task   *otherTask, *mainTask;

struct IOStdReq *myReq, *ioR;
struct MsgPort  *port; 

BOOL  Remote=FALSE, Told=FALSE, Full = FALSE, Done = FALSE;

#define NCOM  47
ULONG cmdcnt[NCOM] = {0}, othercnt, opencnt, closecnt, abortcnt, expungecnt;
ULONG wi;

UBYTE *intname = "<int/except>";

UBYTE *keys[] = {
"Key to output:\n",
"IOStdReq: @ior Command Flags Error Actual Length Data Offset (caller)\n",
"TimerReq: @ior Command Flags Error tvSecs tvMics ...  ...    (caller)\n",
"UPPERCASE command indicates entry to a device vector\n"
"MixedCase (FULL option) indicates entry or Rts from Exec function\n",
"... indicates additional information for previous command\n"
"NOTE: Audio requests do not match IOStdReq\n",
NULL };

UBYTE *bfmt;
UBYTE *bfmth= "%s: @$%lx C=%2ld F=$%lx E=%ld A=$%lx L=$%lx D=$%lx O=$%lx (%s)\n";
UBYTE *bfmtd= "%s: @$%lx C=%2ld F=%ld E=%ld A=%ld L=%ld D=%ld O=%ld (%s)\n";
UBYTE *ofmt = "%s: @$%lx %s unit #%ld prevOpCnt=%ld (%s $%lx)\n";
UBYTE *odfmt= "%s: @$%lx %s unit #%ld (%s $%lx)\n";
UBYTE *efmt = "EXPGE: %s Unit=$%lx (caused by %s $%lx)\n";
UBYTE *gfmt = "%s: @$%lx %s Unit=$%lx%s(%s $%lx)\n";

ULONG vblanks, seconds, minutes;
UBYTE vblankfreq;

/* the device wedges */

void MyBEGIN(struct IOStdReq *ior)
    {
    UWORD  cmdn;

    cmdn = ior->io_Command;
    if(cmdn < NCOM) cmdcnt[cmdn] += 1;
    else othercnt++;

    iorOut(ior,bfmt,"BEGIN");
    }


void MyOPEN(struct IOStdReq *ior, LONG unitn)
    {
    UBYTE *name;

    opencnt++;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	    sprintf(&wbuf[wi], ofmt, "OPEN ", ior,deviceName,unitn,
			ior->io_Device->dd_Library.lib_OpenCnt,name,SysBase->ThisTask);
	    wi += strLen(&wbuf[wi]);
	    }
	else
	    {
   	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(SysBase->ThisTask);

/*	kprintf(ofmt, "OPEN ", ior, deviceName,unitn,name); */
	sprintf(wbuf,ofmt, "OPEN ", ior, deviceName,unitn,
		ior->io_Device->dd_Library.lib_OpenCnt,name,SysBase->ThisTask);
	kputs(wbuf);
	}
   }


void MyCLOSE(struct IOStdReq *ior)
    {
    closecnt++;
    genericOut(ior, gfmt, "CLOSE", " ");
    }

void MyABORT(struct IOStdReq *ior)
    {
    abortcnt++;
    genericOut(ior, gfmt, "ABORT", " ");
    }

void MyEXPUNGE(void)
    {
    UBYTE *name;

    expungecnt++;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	    sprintf(&wbuf[wi], efmt, deviceName,name,SysBase->ThisTask);
	    wi += strLen(&wbuf[wi]);
	    }
	else
	    {
   	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(SysBase->ThisTask);

/*	kprintf(efmt, deviceName,name);*/
	sprintf(wbuf,efmt, deviceName,name,SysBase->ThisTask);
	kputs(wbuf);
	}
   }


/* the optional exec wedges */

void MyOpenDevice(struct IOStdReq *ior, LONG unitn)
    {
    UBYTE *name;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	    sprintf(&wbuf[wi], odfmt, "OpenD", ior, deviceName,unitn,name,SysBase->ThisTask);
	    wi += strLen(&wbuf[wi]);
    	    iorOut(ior, bfmt, "Op...");
	    }
	else
	    {
   	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(SysBase->ThisTask);
/*	kprintf(ofmt, "OpenD", ior, deviceName,unitn,name); */
	sprintf(wbuf,odfmt, "OpenD", ior, deviceName,unitn,name,SysBase->ThisTask);
	kputs(wbuf);
    	iorOut(ior, bfmt, "Op...");
	}
   }


void MyOpenDevRts(struct IOStdReq *ior, LONG result)
    {
    char sbuf[36];
    if(result)
	{
	if(ior->io_Device)
		{
		sprintf(sbuf," ERROR=%ld  OpCnt=%ld ",
			result, ior->io_Device->dd_Library.lib_OpenCnt);
		}
	else
		{
		sprintf(sbuf," ERROR=%ld ",result);
		}
	}
    else sprintf(sbuf," SUCCESSFUL OpCnt=%ld ",
			ior->io_Device->dd_Library.lib_OpenCnt);
    genericOut(ior, gfmt, "OpRts", sbuf);
    iorOut(ior, bfmt, "OpR..");
    }

void MyCloseDevice(struct IOStdReq *ior)
    {
    char sbuf[36];

    sprintf(sbuf," (prevOpCnt=%ld) ",
			ior->io_Device->dd_Library.lib_OpenCnt);
    genericOut(ior, gfmt, "Close", sbuf);
    }

void MyCloseDevRts(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "ClRts");
    }

void MySendIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "Send ");
    }

void MyDoIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "DoIO ");
    }

void MyDoIORts(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "DoRts");
    }

void MyCheckIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "ChkIO");
    }

void MyCheckIORts(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "ChRts");
    }

void MyAbortIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "AbtIO");
    }
  
void MyReplyMsg(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "Reply");
    }

void MyWaitIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "WaitI");
    }

void iorOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd)
    {
    UBYTE *name;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	     sprintf(&wbuf[wi],fmt,cmd,ior,
	     ior->io_Command,ior->io_Flags,ior->io_Error,
	     ior->io_Actual,ior->io_Length,ior->io_Data,ior->io_Offset,
	     name);
	    wi += strLen(&wbuf[wi]);
	    }
	else
	    {
    	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(SysBase->ThisTask);
/*
	kprintf(fmt,cmd,ior,
		ior->io_Command,ior->io_Flags,ior->io_Error,
		ior->io_Actual,ior->io_Length,ior->io_Data,ior->io_Offset,
		name);
*/
	sprintf(wbuf,fmt,cmd,ior,
		ior->io_Command,ior->io_Flags,ior->io_Error,
		ior->io_Actual,ior->io_Length,ior->io_Data,ior->io_Offset,
		name);
	kputs(wbuf);
	}
    }

void genericOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd, UBYTE *res)
    {
    UBYTE *name;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	    sprintf(&wbuf[wi], fmt, cmd, ior,deviceName,
		ior->io_Unit,res,name,SysBase->ThisTask);
	    wi += strLen(&wbuf[wi]);
	    }
	else
	    {
   	    if(!Told) Told=TRUE,Signal(mainTask,WFULLSIG);
	    }
	}
    else
	{
    	name = taskname(SysBase->ThisTask);

/*	kprintf(fmt, cmd, ior,deviceName,ior->io_Unit,res,name);*/
	sprintf(wbuf,fmt, cmd, ior,deviceName,ior->io_Unit,res,
			name,SysBase->ThisTask);
	kputs(wbuf);
	}
   }


UBYTE *taskname(struct Task *task)
    {
    struct CommandLineInterface *cli;
    UBYTE  *s,*name;

    /* if our stack is in system stack, then we are an int or execption */
    if((((APTR)&name) > SysBase->SysStkLower)&&
	(((APTR)&name) < SysBase->SysStkUpper))
		name = intname;
    else
	{
    	name = task->tc_Node.ln_Name;
    	if((task->tc_Node.ln_Type == NT_PROCESS)
		&&(((struct Process *)task)->pr_CLI))
	    {
	    cli = (struct CommandLineInterface *)
			(BADDR(((struct Process *)task)->pr_CLI));
	    if(cli->cli_CommandName)
		{
	    	s = (UBYTE *)(BADDR(cli->cli_CommandName));
	    	if(s[0]) name = &s[1];
		}
	    }
	}
    return(name);
    }



void main(int argc, char **argv) 
   {
   ULONG signals;
   LONG file = NULL;
   UBYTE sbuf[128],tbuf[128];
   int k;

   if(argc<3)
	{
   	printf("%s\n%s\n",Copyright,usage), exit(0L);
	}

   deviceName  = argv[1];
   unitnum = atoi(argv[2]);

   bfmt = bfmtd;
   if(argc>3)
   	{
   	for(k=3; k<argc; k++)
   	    {
	    if(strEqu(argv[k],"remote")) Remote = TRUE;
	    else if(strEqu(argv[k],"hex")) bfmt = bfmth;
	    else if(strEqu(argv[k],"allunits")) allunits = 1;
	    else if(strEqu(argv[k],"full")) Full = TRUE;
	    else printf("unknown option %s\n",argv[k]);
            }
        }


    vblankfreq = SysBase->VBlankFrequency;

   /* Result will be mainTaskName = "cas_DEVMON_whatever.device"
    *   with deviceName pointing to the eighth character
    */
   strCpy(&mainTaskName[0],"cas_DEVMON_");
   strCpy(&mainTaskName[strLen(mainTaskName)],deviceName);

   Forbid();
   if(otherTask = (struct Task *)FindTask(mainTaskName))
      	{
      	Permit();
      	printf("Device %s unit %ld already being monitored... exiting\n",
		deviceName, unitnum);
      	bye("",RETURN_WARN);
      	}

   mainTask = (struct Task *)FindTask(NULL);
   prevTaskName = mainTask->tc_Node.ln_Name;
   mainTask->tc_Node.ln_Name = mainTaskName;
   Permit();
     
   /* initialize */
   if(!(wbuf = (char *)AllocMem(WBUFLEN+EXTRALEN,MEMF_PUBLIC|MEMF_CLEAR)))
      bye("Can't allocate buffer\n",RETURN_FAIL);
   wi = 0;    /* index into wbuf */

   if(!(port = CreatePort(portName, 0)))  bye("Can't open port\n",RETURN_FAIL);
 
   myReq = (struct IOStdReq *)AllocMem(REQSIZE,MEMF_CLEAR|MEMF_PUBLIC);
   if (!myReq)  bye(noMem,RETURN_FAIL);

   myReq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
   myReq->io_Message.mn_ReplyPort = port;

   if(OpenDevice(deviceName, unitnum, myReq, 0))
      	{
      	printf("Can't open %s unit %ld\n",deviceName,unitnum);
      	bye(usage,RETURN_FAIL);
      	}
   TheDevice = (struct Library *)(myReq->io_Device);
   unitptr = myReq->io_Unit;

   /* Install device IO monitoring */

   Forbid();
   RealBEGIN = (ULONG)SetFunction(TheDevice, DEV_BEGINIO, (PFU)myBEGIN);
   RealABORT = (ULONG)SetFunction(TheDevice, DEV_ABORTIO, (PFU)myABORT);
   RealOPEN    = (ULONG)SetFunction(TheDevice, DEV_OPEN,    (PFU)myOPEN);
   RealCLOSE   = (ULONG)SetFunction(TheDevice, DEV_CLOSE,   (PFU)myCLOSE);
   RealEXPUNGE = (ULONG)SetFunction(TheDevice, DEV_EXPUNGE, (PFU)myEXPUNGE);

   if(Full) {
   RealSendIO   = (ULONG)SetFunction(SysBase, EXEC_SENDIO,  (PFU)mySendIO); 
   RealCheckIO  = (ULONG)SetFunction(SysBase, EXEC_CHECKIO, (PFU)myCheckIO); 
   RealDoIO     = (ULONG)SetFunction(SysBase, EXEC_DOIO,    (PFU)myDoIO);
   RealReplyMsg = (ULONG)SetFunction(SysBase, EXEC_REPLYMSG,(PFU)myReplyMsg);
   RealWaitIO   = (ULONG)SetFunction(SysBase, EXEC_WAITIO,  (PFU)myWaitIO);
   RealAbortIO  = (ULONG)SetFunction(SysBase, EXEC_ABORTIO, (PFU)myAbortIO);
   RealOpenDevice = (ULONG)SetFunction(SysBase, EXEC_OPENDEV,  (PFU)myOpenDevice);
   RealCloseDevice= (ULONG)SetFunction(SysBase, EXEC_CLOSEDEV, (PFU)myCloseDevice);
   }
   Permit();

   /* Expunge disabled, CloseDevice so another can open it */
   CloseDevice(myReq);

   StartVBlank();

#ifdef MYDEBUG
	printf("BEGIN was $%08lx now $%08lx\n",RealBEGIN,myBEGIN);
	printf("ABORT was $%08lx now $%08lx\n",RealABORT,myABORT);
	printf("OPEN  was $%08lx now $%08lx\n",RealOPEN,myOPEN);
	printf("CLOSE was $%08lx now $%08lx\n",RealCLOSE,myCLOSE);
	printf("EXPNG was $%08lx now $%08lx\n",RealEXPUNGE,myEXPUNGE);
#endif

   printf("%s Devmon monitoring of %s installed\n",
		Remote ? "Remote" : "Local", deviceName);
   
   printf("CTRL-C to remove monitor%s\n",
		Remote ? " " : ", CTRL-F to flush local buffer");

   while(!Done)
	{
      	signals = Wait(WFULLSIG | BREAKSIG | FLUSHSIG);

      	if(signals & WFULLSIG)   /* Local buffer full */
            {
	    printf("Devmon buffer full.  CTRL-C to quit, CTRL-F to restart: ");
	    signals=Wait(BREAKSIG | FLUSHSIG);
	    }

	if(signals & FLUSHSIG)
	    {
	    wi = 0;
	    Told = FALSE;
	    seconds = 0;
	    vblanks = 0;
	    if(!Remote) printf("Devmon local buffer flushed\n");
	    }

	if(signals & BREAKSIG)
	    {

#ifdef MYDEBUG
      printf("Got BREAKSIG\n");
#endif

	    StopVBlank();

    	    while(!Done)
            	{
            	/* Wait till we can reopen the device */
            	while(OpenDevice(deviceName, unitnum, myReq, 0L))
			{
			printf("Devmon: can't remove wedge because device is open !\n");
			Delay(250L);
			}
            	/* If it's been re-loaded, we can leave            */
            	/* Shouldn't be possible since we disabled Expunge */
            	if((ULONG)myReq->io_Device != (ULONG)TheDevice)
               	    {
		    printf("Device pointer has changed !\n");
               	    Done = TRUE;
               	    }
            	else
               	    {
               	    Forbid();

               	    NewBEGIN = (ULONG)
			SetFunction(TheDevice, DEV_BEGINIO,(PFU)RealBEGIN);
               	    NewABORT = (ULONG)
			SetFunction(TheDevice, DEV_ABORTIO,(PFU)RealABORT);
               	    NewOPEN    = (ULONG)
			SetFunction(TheDevice, DEV_OPEN,(PFU)RealOPEN);
               	    NewCLOSE   = (ULONG)
			SetFunction(TheDevice, DEV_CLOSE,(PFU)RealCLOSE);
               	    NewEXPUNGE = (ULONG)
			SetFunction(TheDevice, DEV_EXPUNGE,(PFU)RealEXPUNGE);
		
		    if(Full) {
		
               	    NewSendIO = (ULONG)
			SetFunction(SysBase, EXEC_SENDIO,(PFU)RealSendIO);
               	    NewCheckIO = (ULONG)
			SetFunction(SysBase, EXEC_CHECKIO,(PFU)RealCheckIO);
               	    NewDoIO = (ULONG)
			SetFunction(SysBase, EXEC_DOIO,(PFU)RealDoIO);
               	    NewReplyMsg = (ULONG)
			SetFunction(SysBase, EXEC_REPLYMSG,(PFU)RealReplyMsg);
               	    NewWaitIO = (ULONG)
			SetFunction(SysBase, EXEC_WAITIO,(PFU)RealWaitIO);
               	    NewAbortIO = (ULONG)
			SetFunction(SysBase, EXEC_ABORTIO,(PFU)RealAbortIO);
               	    NewOpenDevice = (ULONG)
			SetFunction(SysBase, EXEC_OPENDEV,(PFU)RealOpenDevice);
               	    NewCloseDevice = (ULONG)
			SetFunction(SysBase, EXEC_CLOSEDEV,(PFU)RealCloseDevice);
		    }

               	    if((((ULONG)NewBEGIN != (ULONG)myBEGIN)
                      ||((ULONG)NewABORT != (ULONG)myABORT)
                      ||((ULONG)NewOPEN != (ULONG)myOPEN)
                      ||((ULONG)NewCLOSE != (ULONG)myCLOSE)
                      ||((ULONG)NewEXPUNGE != (ULONG)myEXPUNGE))||

                      ((Full)&&(((ULONG)NewSendIO != (ULONG)mySendIO)
                              ||((ULONG)NewCheckIO != (ULONG)myCheckIO)
                              ||((ULONG)NewDoIO != (ULONG)myDoIO)
                              ||((ULONG)NewWaitIO != (ULONG)myWaitIO)
                              ||((ULONG)NewAbortIO != (ULONG)myAbortIO)
                              ||((ULONG)NewOpenDevice != (ULONG)myOpenDevice)
                              ||((ULONG)NewCloseDevice != (ULONG)myCloseDevice)
                              ||((ULONG)NewReplyMsg != (ULONG)myReplyMsg))))

                  	{
                  	/* Someone else has changed the vectors */
                  	/* We put theirs back - can't exit yet  */
                  	SetFunction(TheDevice, DEV_BEGINIO, (PFU)NewBEGIN);
                  	SetFunction(TheDevice, DEV_ABORTIO, (PFU)NewABORT);
                  	SetFunction(TheDevice, DEV_OPEN,    (PFU)NewOPEN);
                  	SetFunction(TheDevice, DEV_CLOSE,   (PFU)NewCLOSE);
                  	SetFunction(TheDevice, DEV_EXPUNGE, (PFU)NewEXPUNGE);

			if(Full) {
                  	SetFunction(SysBase, EXEC_SENDIO,  (PFU)NewSendIO);
                  	SetFunction(SysBase, EXEC_CHECKIO, (PFU)NewCheckIO);
                  	SetFunction(SysBase, EXEC_DOIO,    (PFU)NewDoIO);
                  	SetFunction(SysBase, EXEC_WAITIO,  (PFU)NewWaitIO);
                  	SetFunction(SysBase, EXEC_ABORTIO, (PFU)NewAbortIO);
                  	SetFunction(SysBase, EXEC_OPENDEV, (PFU)NewOpenDevice);
                  	SetFunction(SysBase, EXEC_CLOSEDEV, (PFU)NewCloseDevice);
                  	SetFunction(SysBase, EXEC_REPLYMSG,(PFU)NewReplyMsg);
			}
                  	}
               	    else
                  	{
                  	Done = TRUE;
                  	}
               	    Permit();
               	    }
            	CloseDevice(myReq);
            	if(!Done)
		    {
		    printf("Vectors changed - can't restore yet\n");
		    printf("CTRL-C again to try exiting when ready...\n");
		    Wait(BREAKSIG);
		    }
		else printf("\nDevmon monitor of %s unit %ld removed\n", 
				deviceName,unitnum);
                }


	    minutes = seconds / 60;
	    seconds -= minutes * 60;

	    sprintf(tbuf,
	 "\nTIME : %s unit %ld monitored for %ld mins %ld secs %ld vblanks\n\n",
		deviceName, unitnum, minutes, seconds, vblanks);

	    if(Remote) kputs(tbuf);	

again:
	    if(Remote)
		{
		for(k=0; keys[k]; k++) kputs(keys[k]);
		}
	    else
		{
	    	printf("\nFilename for output (<RET> for stdout): ");
	    	if(getS(sbuf))
		    {
		    if(file = Open(sbuf,MODE_NEWFILE))
		    	{
		    	Write(file,wbuf,wi);
			Write(file,tbuf,strLen(tbuf));
			for(k=0; keys[k]; k++) Write(file,keys[k],strlen(keys[k]));
		    	Close(file);
		        }
		    else
		    	{
		    	printf("Can't open %s\n",sbuf);
		    	goto again;
		    	}
		    }
	    	else
		    {
		    Write(Output(),wbuf,wi);
		    Write(Output(),tbuf,strLen(tbuf));
		    for(k=0; keys[k]; k++) Write(Output(),keys[k],strlen(keys[k]));
		    }
		}
            }
	}

   if(usecnt) printf("Devmon: Waiting for final requests to complete...\n");
   while(usecnt) Delay(20);

   bye("",RETURN_OK);
   }


void bye(UBYTE *s, LONG err)
   {
   if(*s) printf(s);
   cleanup();
   exit(err);
   }

void cleanup()
   {
   if(myReq)   FreeMem(myReq,REQSIZE);
   if(port)    DeletePort(port);
   if(wbuf)    FreeMem(wbuf,WBUFLEN+EXTRALEN);

   Forbid();
   if(prevTaskName) mainTask->tc_Node.ln_Name = prevTaskName;
   Permit();
   }


/* String functions */

BOOL strEqu(UBYTE *p, UBYTE *q)
   { 
   while(TOUPPER(*p) == TOUPPER(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

int strLen(UBYTE *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

void strCpy(UBYTE *to,UBYTE *from)
   {
   do
      {
      *to++ = *from;
      }
   while(*from++);
   }


LONG getS(UBYTE *s)
    {
    int i=0;

    while((s[i] = getchar()) != '\n') i++;
    s[i] = '\0';
    return(i);
    }

int atoi( char *s )
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }



LONG MyVBlank()
   {

#ifdef IS_AZTEC
#asm
       movem.l  a2-a7/d2-d7,-(sp)
       move.l   a1,a4
#endasm
#endif

   if(++vblanks > vblankfreq)
      {
      vblanks  = 0L;
      seconds++;
      }

#ifdef IS_AZTEC
      ;   /* this is necessary */
#asm
      movem.l  (sp)+,a2-a7/d2-d7
#endasm
#endif

   return(0);  /* interrupt routines have to do this */
   }



/*
 *  Code to install/remove cycling interrupt handler
 */

char myname[] = "CAS_devmon_VB";  /* Name of interrupt handler */
struct Interrupt intServ;

void StartVBlank()  {
#ifdef IS_AZTEC
   intServ.is_Data = GETAZTEC();  /* returns contents of register a4 */
#else
   intServ.is_Data = NULL;
#endif
   intServ.is_Code = (APTR)&MyVBlank;
   intServ.is_Node.ln_Succ = NULL;
   intServ.is_Node.ln_Pred = NULL;
   intServ.is_Node.ln_Type = NT_INTERRUPT;
   intServ.is_Node.ln_Pri  = 0;
   intServ.is_Node.ln_Name = myname;
   AddIntServer(5,&intServ);
   }

void StopVBlank() { RemIntServer(5,&intServ); }


/* end */

