;/* devmon.c - Execute me to compile me with Lattice 5.04
if not exists devmona.o
Asm -iH:include/ -odevmona.o devmona.asm
endif
if not exists devmon.o
LC -b0 -cfistq -v -j73 devmon.c
endif
Blink FROM LIB:astartup.obj,devmon.o,devmona.o TO devmon LIBRARY LIB:Amiga.lib,LIB:LC.lib,LIB:debug.lib
quit
*/

/* Devmon.c --- v37.1
 *
 * Copyright (c) 1990  Commodore Business Machines - All Rights Reserved
 *
 */

/*
#define MYDEBUG
*/

char *vers = "\0$VER: devmon 37.1";
char Copyright[] =
 "devmon v37.1\nCopyright (c) 1990  Commodore Business Machines  All Rights Reserved";
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
extern void  printf __ARGS((char *,...));

/* the assembler entries in devmona.asm */
extern VOID  mySendIO();  
extern VOID  myDoIO();    
extern VOID  myWaitIO();
extern VOID  myAbortIO();
extern VOID  myReplyMsg();
extern VOID  myBEGIN(); 
extern VOID  myABORT(); 
extern VOID  myOPEN();    
extern VOID  myCLOSE();   
extern VOID  myEXPUNGE(); 

extern ULONG RealOPEN, RealCLOSE, RealEXPUNGE, RealBEGIN, RealABORT;
extern ULONG RealSendIO, RealDoIO, RealReplyMsg, RealWaitIO, RealAbortIO;

extern LONG usecnt;

extern struct ExecBase  *SysBase;

/* exec wedges */
void MySendIO(struct IOStdReq *ior);
void MyDoIO(struct IOStdReq *ior);
void MyDoIORts(struct IOStdReq *ior);
void MyReplyMsg(struct IOStdReq *ior);
void MyWaitIO(struct IOStdReq *ior);
void MyAbortIO(struct IOStdReq *ior);
/* device wedges */
void MyBEGIN(struct IOStdReq *ior);
void MyABORT(struct IOStdReq *ior);
void MyOPEN(struct IOStdReq *ior, LONG unitn);
void MyCLOSE(struct IOStdReq *ior);
void MyEXPUNGE(void);

void genericOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd);
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

#define WBUFLEN   40000L
#define EXTRALEN   256L

#define REQSIZE    120L    /* should be big enough to open any OpenDevice */

#define DEV_OPEN     LIB_OPEN
#define DEV_CLOSE    LIB_CLOSE
#define DEV_EXPUNGE  LIB_EXPUNGE
/*      DEV_BEGINIO  (-30)       defined in exec/io.h */

#define EXEC_SENDIO	(-462)
#define EXEC_DOIO	(-456)
#define EXEC_REPLYMSG	(-378)
#define EXEC_WAITIO	(-474)
#define EXEC_ABORTIO	(-480)

ULONG  NewOPEN, NewCLOSE, NewEXPUNGE, NewBEGIN, NewABORT;
ULONG  NewSendIO, NewDoIO, NewReplyMsg, NewWaitIO, NewAbortIO;

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

UBYTE *key1="KEY 1: @ior Command Flags Error Actual Length Data Offset (caller)\n";
UBYTE *key2="KEY 2: @ior Command Flags Error tvSecs tvMics ...  ...    (caller)\n";

UBYTE *bfmt;
UBYTE *bfmth= "%s: @$%lx C=%2ld F=%ld E=%ld A=$%lx L=$%lx D=$%lx O=$%lx (%s)\n";
UBYTE *bfmtd= "%s: @$%lx C=%2ld F=%ld E=%ld A=%ld L=%ld D=%ld O=%ld (%s)\n";
UBYTE *ofmt = "OPEN : @$%lx %s unit #%ld  (%s)\n";
UBYTE *efmt = "EXPGE: %s Unit=$%lx (caused by %s)\n";
UBYTE *gfmt = "%s: @$%lx %s Unit=$%lx  (%s)\n";

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
	    sprintf(&wbuf[wi], ofmt, ior, deviceName,unitn,name);
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
	kprintf(ofmt, ior, deviceName,unitn,name);
	}
   }


void MyCLOSE(struct IOStdReq *ior)
    {
    closecnt++;
    genericOut(ior, gfmt, "CLOSE");
    }


void MyABORT(struct IOStdReq *ior)
    {
    abortcnt++;
    genericOut(ior, gfmt, "ABORT");
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
	    sprintf(&wbuf[wi], efmt, deviceName,name);
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
	kprintf(efmt, deviceName,name);
	}
   }


/* the optional exec wedges */

void MySendIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "Send ");
    }

void MyDoIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "DoIO ");
    }

void MyAbortIO(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "AbtIO");
    }

void MyDoIORts(struct IOStdReq *ior)
    {
    iorOut(ior, bfmt, "DoRts");
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
	kprintf(fmt,cmd,ior,
		ior->io_Command,ior->io_Flags,ior->io_Error,
		ior->io_Actual,ior->io_Length,ior->io_Data,ior->io_Offset,
		name);
	}
    }

void genericOut(struct IOStdReq *ior,UBYTE *fmt,UBYTE *cmd)
    {
    UBYTE *name;

    if(!Remote)
	{
	if(wi < WBUFLEN)
	    {
    	    name = taskname(SysBase->ThisTask);
	    sprintf(&wbuf[wi], fmt, cmd, ior,deviceName,ior->io_Unit,name);
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
	kprintf(fmt, cmd, ior,deviceName,ior->io_Unit,name);
	}
   }


UBYTE *taskname(struct Task *task)
    {
    struct CommandLineInterface *cli;
    struct Process *pr;
    UBYTE  *s, *name;
    APTR stack;

    stack = (APTR)&stack;

    if((stack > SysBase->SysStkLower)&&(stack < SysBase->SysStkUpper))
		name = intname;
    else
	{
    	pr = (struct Process *)task;
    	name = task->tc_Node.ln_Name;
    	if((task->tc_Node.ln_Type == NT_PROCESS)&&(pr->pr_CLI))
	    {
	    cli = (struct CommandLineInterface *)(BADDR(pr->pr_CLI));
	    if(cli->cli_CommandName)
		{
	    	s = (UBYTE *)(BADDR(cli->cli_CommandName)); 
	    	if(s[1]) name = &s[1];
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
   RealDoIO     = (ULONG)SetFunction(SysBase, EXEC_DOIO,    (PFU)myDoIO);
   RealReplyMsg = (ULONG)SetFunction(SysBase, EXEC_REPLYMSG,(PFU)myReplyMsg);
   RealWaitIO   = (ULONG)SetFunction(SysBase, EXEC_WAITIO,  (PFU)myWaitIO);
   RealAbortIO  = (ULONG)SetFunction(SysBase, EXEC_ABORTIO, (PFU)myAbortIO);
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
            	while(OpenDevice(deviceName, unitnum, myReq, 0L))  Delay(50L);

            	/* If it's been re-loaded, we can leave            */
            	/* Shouldn't be possible since we disabled Expunge */
            	if((ULONG)myReq->io_Device != (ULONG)TheDevice)
               	    {
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
               	    NewDoIO = (ULONG)
			SetFunction(SysBase, EXEC_DOIO,(PFU)RealDoIO);
               	    NewReplyMsg = (ULONG)
			SetFunction(SysBase, EXEC_REPLYMSG,(PFU)RealReplyMsg);
               	    NewWaitIO = (ULONG)
			SetFunction(SysBase, EXEC_WAITIO,(PFU)RealWaitIO);
               	    NewAbortIO = (ULONG)
			SetFunction(SysBase, EXEC_ABORTIO,(PFU)RealAbortIO);
		    }

               	    if((((ULONG)NewBEGIN != (ULONG)myBEGIN)
                      ||((ULONG)NewABORT != (ULONG)myABORT)
                      ||((ULONG)NewOPEN != (ULONG)myOPEN)
                      ||((ULONG)NewCLOSE != (ULONG)myCLOSE)
                      ||((ULONG)NewEXPUNGE != (ULONG)myEXPUNGE))||

                      ((Full)&&(((ULONG)NewSendIO != (ULONG)mySendIO)
                              ||((ULONG)NewDoIO != (ULONG)myDoIO)
                              ||((ULONG)NewWaitIO != (ULONG)myWaitIO)
                              ||((ULONG)NewAbortIO != (ULONG)myAbortIO)
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
                  	SetFunction(SysBase, EXEC_DOIO,    (PFU)NewDoIO);
                  	SetFunction(SysBase, EXEC_WAITIO,  (PFU)NewWaitIO);
                  	SetFunction(SysBase, EXEC_ABORTIO, (PFU)NewAbortIO);
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
            	if(!Done)  printf("Vectors changed - can't restore yet\n");
		else printf("\nDevmon monitor of %s unit %ld removed\n", 
				deviceName,unitnum);
                }


	    minutes = seconds / 60;
	    seconds -= minutes * 60;

	    sprintf(tbuf,
	 "\nTIME : %s unit %ld monitored for %ld mins %ld secs %ld vblanks\n\n",
		deviceName, unitnum, minutes, seconds, vblanks);

	    if(Remote) kprintf(tbuf);	

again:
	    if(!Remote)
		{
	    	printf("\nFilename for output (<RET> for stdout): ");
	    	if(getS(sbuf))
		    {
		    if(file = Open(sbuf,MODE_NEWFILE))
		    	{
		    	Write(file,wbuf,wi);
			Write(file,tbuf,strLen(tbuf));
			Write(file,key1,strLen(key1));
			Write(file,key2,strLen(key2));
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
		    Write(Output(),key1,strLen(key1));
		    Write(Output(),key2,strLen(key2));
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

