;/* cmd.c - Execute me to compile me with Lattice 5.04
asm -iinclude: -ocmda.o cmda.asm
LC -b0 -cfistq -v -y -j73 cmd.c
Blink FROM LIB:astartup.obj,cmd.o,cmda.o TO cmd LIBRARY LIB:amiga.lib,LIB:LC.lib
quit
*/

/* 
#define DEBUG 
*/

#include <exec/types.h>
#include <libraries/dos.h>
#include <exec/memory.h> 
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/errors.h>
#include <libraries/dos.h> 
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <devices/serial.h> 
#include <devices/parallel.h> 
#include <devices/printer.h>
#include <hardware/cia.h>


extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include <stdlib.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

#include "cmd_rev.h"

char *verstag = VERSTAG;
char *Copyright = 
  VERS "\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";


/* Cmd.c ------ Carolyn Scheppner  CBM  11/90
 *
 * Copyright (c) 1990  Commodore Business Machines - All Rights Reserved
 *
 * Redirects exec serial or parallel device CMD_WRITEs to a file
 *  (for the purpose of capturing printer output in a file)
 * Built upon fragments of Read (author?) and NoFastMem (Andy Finkel)
 *
 * CLI Usage:  [run] cmd devicename filename [opt [s] [m] [n]]
 *   devicename serial or parallel
 *   s (Skip) skips any short initial write (usually a Reset if screendump)
 *   m (Multiple) causes cmd to remain installed for multiple files
 *   n (Notify) enables helpful progress messages
 *
 * WB Usage:  Just doubleclick.
 *            Specify the args in your icon's ToolTypes (use WB Info)
 *            Built-in defaults are:
 *               DEVICE=parallel
 *               FILE=ram:CMD_file
 *               SKIP=FALSE
 *               MULTIPLE=FALSE
 *               NOTIFY=FALSE
 *
 *   Note: On a screen dump, first CMD_WRITE is usually a printer RESET.
 *         The printer device then delays long enough for the reset
 *         to complete, to prevent the loss of subsequent output data.
 *         When the dump is instead captured in a file, this delay
 *         is of course lost.  If your printer driver outputs a reset
 *         at the start of a dump (as ours do), use the -s (SKIP) option
 *         to keep the initial CMD_WRITE out of the file.
 *
 * Sorry about the busywait synchronization of the device wedge
 * and the main process.  The purpose was to avoid unnecessary
 * meddling with the message structures and the device's signals.
 * I had to add a conditional kludge in MyBeginIO to allow Cmd
 * to work with our HPLaser drivers which do PWrites within a Forbid
 * in their Close logic to print/eject last sheet, and also apparently
 * during their open logic if drivers are resident.
 *
 * v2 mods: changes to MyBeginIO for -1 and 0 length CMD_WRITES, usage
 * v3 mods: added buffering of small writes to speed file IO
 * v4 mods: Conditional kludges added to MyBeginIO for HPLaser
 *          (if Forbidden, sneaks the data into main's write buffer)
 *          (EXTRALEN added to wbuf size to allow extra room for this) 
 *          myWrite now doesn't Write if len = 0
 *          MyClose now conditional on writecnt, not reqcnt
 * v5 mods: Minor cleanup, check for fh in myWrite, flags changed to opts
 * v6 mods: Now sets Status to 'CIAF_PRTRSEL' if io_Command is 'PDCMD_QUERY'
 *          or 'SDCMD_QUERY' (since they are the same).  Now sets io_Data
 *          to 'CIAF_PRTRSEL' and io_Actual to 1 if io_Command is
 *          'PRD_QUERY'.  ie.  It assumes that a parallel printer is in use
 *          (for now anyway).  Now sets io_Error to 'IOERR_NOCMD' if an
 *          unknown command is encountered.
 *
 * 36_10 mods: Latticized and used alloc'd signals for all but BREAK
 * 36_11 mods: use cmd_rev.h, bump version, new startup, remove unimp. i opt
 * 36_12 mods: screen for user passing something ending in a colon (:)
 * 36_13 mods: use pragmas, use WaitTOF() in former busy-waits 
 * 37_2  mods: fix crash from WB (move new OpenLibrary of graphics)
 * 37_3  mods: check for devname ending in ":" from ToolType also, bigger win
 * 37_4  mods: don't return NOCMD error for writes of length zero ! 
 *
 */

 
#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c)) 
#define HIGHER(x,y)     ((x)>(y)?(x):(y))

#define WBUFLEN   2048L
#define EXTRALEN   256L

#define INBUFLEN    40L
#define REQSIZE    120L    /* should be big enough or any OpenDevice */

#define DEV_CLOSE    LIB_CLOSE
#define DEV_EXPUNGE  LIB_EXPUNGE
/*      DEV_BEGINIO  (-30)       defined in exec/io.h */


typedef VOID (*PFV)();
typedef ULONG (*PFU)();

LONG  allocsigs[3] = { -1 };
ULONG OpenSig, CloseSig, WriteSig;
#define BREAK_SIG (SIGBREAKF_CTRL_C)

#define SHORT_WRITE (8L)

/* amiga.lib functions */
LONG printf(BYTE *fmt,...);
LONG sprintf(UBYTE *buffer, BYTE *fmt,...);
LONG fgetc(LONG);
LONG fprintf(LONG, BYTE *fmt,...);

void strCat( char *to, char *from );
void strCpy(char *p, char *q);
int strLen(char *s);
BOOL strEqu(TEXT *p, TEXT *q);
void getWbArgs(struct WBStartup *wbMsg);
void message(char *s);
void cleanup(void);
void cleanexit(char *s,int e);
void usageExit(void);
void helpExit(void);
int myWrite(LONG fh, char *data, int len);
int bufOrWrite(LONG fh, char *data, int len);
void MyClose(struct IOStdReq *ior);
void MyBeginIO(struct IOStdReq *ior);

extern VOID  myBeginIO();  /* The assembler entry */
extern VOID  myClose();    /* The assembler entry */
extern VOID  myExpunge();  /* The assembler routine */


extern struct MsgPort   *CreatePort(); 
extern struct WBStartup *WBenchMsg;

ULONG  RealBeginIO, NewBeginIO;
ULONG  RealClose,   NewClose;
ULONG  RealExpunge, NewExpunge;

char *noMem      = "Out of memory\n";
char *portName   = "cas_TMP_CMD_PORT";
char *conSpec    = "CON:20/20/600/80/ " VERS;

char u1[]=
  {"\nCLI Usage " VERS ": [run] Cmd devicename filename [opt s m n]\n"};
char u2[]={"  devicename = serial or parallel\n"};
char u3[]={"  s = SKIP any short initial write (usually a reset if screendump)\n"};
char u4[]={"  m = installed for MULTIPLE files until Break or CTRL_C\n"};
char u5[]={"  n = enables NOTIFY (helpful progress messages)\n\n"};
char u6[]={"WB Tooltypes: DEVICE, FILE, and booleans SKIP,MULTIPLE,NOTIFY\n"};
char u7[]={"   Cancel installation for multiple files by reclicking\n\n"};
char *us[7] = {u1,u2,u3,u4,u5,u6,u7};
char *morehelp = "Type  cmd help  for more help\n\n";

char *prevTaskName = NULL;
char *outFileName, *deviceName;
char mainTaskName[40];
char wbDev[INBUFLEN], wbFile[INBUFLEN];
char sbuf[120], *wbuf = 0;

struct Device *TheDevice;
struct Task   *otherTask, *mainTask;

struct IOStdReq *myReq, *ioR;
struct MsgPort  *port; 

struct Library *IconBase = NULL;
struct Library *GfxBase = NULL;

LONG  wLen = 1, outFile = NULL;
ULONG total = 0;

BOOL  Error1 = TRUE, Skip = FALSE, Multiple = FALSE, Notify = FALSE;
BOOL  Done = FALSE, FromWb = FALSE, MainBusy = FALSE;
int   reqcnt = 0, writecnt = 0, filecnt = 0; fnLen, wi = 0;

union parserioreq {
    struct IOExtPar par;
    struct IOExtSer ser;
};


void MyBeginIO(struct IOStdReq *ior)
   {
   BOOL   Forbidden;
   char   *data;
   int    k;
   union parserioreq *ioreq = (union parserioreq *)ior;

#ifdef DEBUG
   kprintf("caller pri=%ld FC=%ld DC=%ld, mainpri = %ld, GfxBase=$%lx\n",
		SysBase->ThisTask->tc_Node.ln_Pri,
		SysBase->ThisTask->tc_TDNestCnt,
		SysBase->ThisTask->tc_IDNestCnt,
		mainTask->tc_Node.ln_Pri,
		GfxBase);
#endif
   /* The code conditional on Forbidden is needed to work with
    * HPLaser drivers which PWrite during a Forbid in their Close
    * logic to print and eject last sheet, and also apparently
    * during the initial write if drivers are resident.
    */
   Forbidden = (SysBase->TDNestCnt >= 0) ? TRUE : FALSE;

   reqcnt += 1;

   /* David Berezowski / Sept 6, 1988 */
   if (ior->io_Command == PDCMD_QUERY) { /* parallel port query */
	ioreq->par.io_Status = CIAF_PRTRSEL; /* printer is selected */
   }
   else if (ior->io_Command == PRD_QUERY) { /* printer port query */
	*(UBYTE *)(ior->io_Data) = CIAF_PRTRSEL; /* printer is selected */
	ior->io_Actual = 1; /* flag parallel */
   }
   else if((ior->io_Command == CMD_WRITE)&&(ior->io_Length)) {

      writecnt += 1;

      if(writecnt==1)
         {
         if(!Forbidden)  while(MainBusy) { WaitTOF(); }
         MainBusy = TRUE;
         Signal(mainTask,OpenSig);
         if(!Forbidden)   while(MainBusy) { WaitTOF(); }
         }

      /* If device CMD_WRITE uses length -1, convert to actual length */
      if(ior->io_Length==-1)  ior->io_Length = strLen(ior->io_Data);

      /* If not a short initial write with Skip TRUE
       * then have main write it out to file.
       */
      if((!Skip)||(writecnt>1)||(ior->io_Length > SHORT_WRITE))
         {
         /* This conditional kludge needed to work with HPLaser
          * drivers which PWrite during a Forbid in their
          * Close logic to print/eject last sheet
          */
         if(Forbidden)
            {
            if(ior->io_Length < (WBUFLEN + EXTRALEN - wi))
               {
               data = (char *)ior->io_Data;
               for(k=0; k<ior->io_Length; k++, wi++)  wbuf[wi]=data[k];
               }
            }
         else
            {
            while(MainBusy)  WaitTOF();
            MainBusy = TRUE;
            ioR = ior;
            Signal(mainTask,WriteSig);  /* Signal write */
            while(MainBusy)  WaitTOF();
            }
         }
      ior->io_Actual = ior->io_Length;
   }
   /* 37_4 - added this else to not return NOCMD if a CMD_WRITE of length zero
    */
   else if(ior->io_Command == CMD_WRITE) {
      ior->io_Actual = 0;
   }
   else { /* not a command handled by cmd */
      ior->io_Error = IOERR_NOCMD;
   }
   if(!(ior->io_Flags & IOF_QUICK))  ReplyMsg(ior);
   }


void MyClose(struct IOStdReq *ior)
   {
   /* Note - Exec has us in a forbid here */
   if(writecnt) /* Ignores DOS's initial Open/Close/Open */
      {
      Signal(mainTask,CloseSig);  /* Signal Close */
      }
   }

void main(int argc, char **argv) 
   { 
   ULONG signals;
   int k,j;
   char opt;

   FromWb = (argc==0) ? TRUE : FALSE;

   if(!(GfxBase = (struct Library *)OpenLibrary("graphics.library",0L)))
	cleanexit("Can't open graphics.library\n",RETURN_FAIL);

   if(FromWb)
      {
      getWbArgs(WBenchMsg);
      deviceName  = wbDev;
      outFileName = wbFile;
      }
   else
      {
      if((argc>1)&&(strEqu(argv[1], "?")))     usageExit();
      if((argc>1)&&(strEqu(argv[1], "help")))  helpExit();
      if(argc<3) usageExit();

      deviceName  = argv[1];
      outFileName = argv[2];

      if((strEqu(outFileName,"opt")) || 
	 (strEqu(deviceName,"opt"))) usageExit();

      if((argc>4)&&(strEqu(argv[3],"opt")))
         {
         for(k=4; k<argc; k++)
            {
            for(j=0; j < strLen(argv[k]); j++)
               {
               opt = argv[k][j];
               if(opt == 's')  Skip = TRUE;
               if(opt == 'm')  Multiple = TRUE;
               if(opt == 'n')  Notify = TRUE;
               }
            }
         }
      }

   if(deviceName[strlen(deviceName)-1] == ':')
	cleanexit("Cmd: Device must be \"serial\" or \"parallel\"\n",RETURN_FAIL);

   fnLen = strLen(outFileName); /* Used if Multiple extension added */

   /* Result will be mainTaskName = "cas_CMD_whatever.device"
    *   with deviceName pointing to the eighth character
    */
   strCpy(&mainTaskName[0],"cas_CMD_");
   strCat(mainTaskName,deviceName);
   strCat(mainTaskName,".device");
   deviceName = &mainTaskName[8];

   for(k=0; k<3; k++)
	{
	if((allocsigs[k] = AllocSignal(-1)) == -1)
		cleanexit("No free signals\n",RETURN_FAIL);
	}
   OpenSig  = 1 << allocsigs[0];
   CloseSig = 1 << allocsigs[1];
   WriteSig = 1 << allocsigs[2];

   Forbid();
   if(otherTask = (struct Task *)FindTask(mainTaskName))
      {
      Permit();
      if(FromWb) Signal(otherTask,BREAK_SIG);
      else printf("Device already redirected... exiting\n");
      cleanexit("",RETURN_WARN);
      }

   mainTask = (struct Task *)FindTask(NULL);
   prevTaskName = mainTask->tc_Node.ln_Name;
   mainTask->tc_Node.ln_Name = mainTaskName;
   Permit();
     
   /* initialize */
   if(!(wbuf = (char *)AllocMem(WBUFLEN+EXTRALEN,MEMF_PUBLIC|MEMF_CLEAR)))
      cleanexit("Can't allocate write buffer\n",RETURN_FAIL);
   wi = 0;    /* index into wbuf */

   if(!(port = CreatePort(portName, 0)))
	cleanexit("Can't open port\n",RETURN_FAIL);
 
   myReq = (struct IOStdReq *)AllocMem(REQSIZE,MEMF_CLEAR|MEMF_PUBLIC);
   if (!myReq)  cleanexit(noMem,RETURN_FAIL);

   myReq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
   myReq->io_Message.mn_ReplyPort = port;

   if(OpenDevice(deviceName, 0, myReq, 0))
      {
      sprintf(sbuf,"Can't open %s\n",deviceName);
      cleanexit(sbuf,RETURN_FAIL);
      }
   TheDevice = myReq->io_Device;

   /* Install device IO redirection */

   Forbid();
   RealBeginIO = (ULONG)SetFunction(TheDevice, DEV_BEGINIO, (PFU)myBeginIO);
   RealClose   = (ULONG)SetFunction(TheDevice, DEV_CLOSE,   (PFU)myClose);
   RealExpunge = (ULONG)SetFunction(TheDevice, DEV_EXPUNGE, (PFU)myExpunge);
   Permit();

   /* Expunge disabled, CloseDevice so another can open it */
   CloseDevice(myReq);

   if(Notify)
      {
      sprintf(sbuf,"Cmd redirection of %s installed\n",deviceName);
      message(sbuf);
      }

   while(!Done)
      {
      signals = Wait(OpenSig|WriteSig|CloseSig|BREAK_SIG);

      if(signals & OpenSig)   /* Open */
         {
         if(!outFile)   /* No output file currently open */
            {
            if(Multiple)  /* If Multiple, add .n extension to filename */
               {
               filecnt++;
               sprintf(&outFileName[fnLen],".%ld",filecnt);
               }
             /* open output file */
            outFile = Open(outFileName, MODE_NEWFILE);
            wLen = 1;
            total = 0;
            /* This moved due to sneak-into-buffer HP kludge */
            /* wi = 0;     Init now at Alloc, and each Close */
            Error1 = TRUE;

            if(Notify)
               {
               sprintf(sbuf,"Redirecting %s to %s\n",
                         deviceName,outFileName);
               message(sbuf);
               }

            }
#ifdef DEBUG
      printf("Processed OpenSig, file %s, handle $%lx\n",
                 outFileName,outFile);
#endif
         }

      if(signals & WriteSig)   /* Write */
         {
         if((outFile)&&(wLen > -1))
            {
            wLen = bufOrWrite(outFile,ioR->io_Data,ioR->io_Length);
            }
         else if(Error1)
            {
            message("Cmd file error: Cancel device output\n");
            Error1 = FALSE;
            }
#ifdef DEBUG
      printf("Processed WriteSig, ioLen %ld, wLen %ld, Error1 = %ld\n",
                 ioR->io_Length, wLen, Error1);
#endif
         }

      if(signals & (CloseSig|BREAK_SIG))
         {
         /* Close file now so user can copy even if something is wrong */
         /* Null the handle - to prevent Write or re-Close */
         if(!Multiple)  signals |= BREAK_SIG;
         if(outFile)
            {
            /* Write buffer contents */
            if((wi>0)&&(wLen>-1)) wLen = myWrite(outFile,wbuf,wi);
            wi = 0;    /* moved from Open logic */

            Forbid();
            Close(outFile);
            outFile = NULL;
            writecnt = 0;
            reqcnt = 0;
            Permit();

            if((!Multiple)||(Notify))
               {
               sprintf(sbuf,"Redirected %ld bytes from %s to %s\n",
                          total,deviceName,outFileName);
               message(sbuf);
               }
            }
#ifdef DEBUG
      printf("Processed CloseSig, total %ld\n", total);
#endif
         }

      if(signals & BREAK_SIG)
         {
#ifdef DEBUG
      printf("Got BREAK_SIG\n");
#endif
         while(!Done)
            {
            /* Wait till we can reopen the device */
            while(OpenDevice(deviceName, 0L, myReq, 0L))  Delay(50L);

            /* If it's been re-loaded, we can leave            */
            /* Shouldn't be possible since we disabled Expunge */
            if((ULONG)myReq->io_Device != (ULONG)TheDevice)
               {
               Done = TRUE;
               }
            else
               {
               Forbid();

               NewBeginIO = (ULONG)SetFunction(TheDevice, DEV_BEGINIO, (PFU)RealBeginIO);
               NewClose   = (ULONG)SetFunction(TheDevice, DEV_CLOSE,   (PFU)RealClose);
               NewExpunge = (ULONG)SetFunction(TheDevice, DEV_EXPUNGE, (PFU)RealExpunge);

               if((NewBeginIO != (ULONG)myBeginIO)
                   ||(NewClose != (ULONG)myClose)
                     ||(NewExpunge != (ULONG)myExpunge))
                  {
                  /* Someone else has changed the vectors */
                  /* We put theirs back - can't exit yet  */
                  SetFunction(TheDevice, DEV_BEGINIO, (PFU)NewBeginIO);
                  SetFunction(TheDevice, DEV_CLOSE,   (PFU)NewClose);
                  SetFunction(TheDevice, DEV_EXPUNGE, (PFU)NewExpunge);
                  }
               else
                  {
                  Done = TRUE;
                  }
               Permit();
               }
            CloseDevice(myReq);
            if(!Done)  message("Vectors have changed - can't restore\n");
            }
         }
      MainBusy = FALSE;
      }

   sprintf(sbuf,"\nCmd redirection of %s removed\n", deviceName);
   cleanexit(sbuf,RETURN_OK);
   }


/* Output buffering */

int bufOrWrite(LONG fh, char *data, int len)
   {
   int k, wlen;

   wlen = len;

   /* If possible, just buffer the output data */
   if(len <  WBUFLEN - wi)
      {
      for(k=0; k<len; k++, wi++)  wbuf[wi] = data[k];
      }
   else
      {
      /* Else output any buffered data to the file */
      if(wi>0)  wlen = myWrite(fh,wbuf,wi);
      wi = 0;

      /* Then either buffer or write out current request */
      if(wlen > -1)
         {
         if(len < WBUFLEN)
            {
            for(k=0; k<len; k++, wi++)  wbuf[wi] = data[k];
            wlen = len;
            }
         else
            {
            wlen = myWrite(fh,data,len);
            }
         }
      }
   return(wlen);
   }


/* myWrite also updates total */
int myWrite(LONG fh, char *data, int len)
   {
   int wlen = 0;

   if((fh)&&(len))
      {
      wlen = Write(fh,data,len);
      if (wlen > -1) total += wlen;
      }
   return(wlen);
   }


/* Cleanup and exits */

void helpExit()
   { 
   int k;
   for(k=0; k<6; k++) printf(us[k]);
   exit(RETURN_OK);
   } 

void usageExit()
   { 
   printf(u1);
   printf(morehelp);
   exit(RETURN_OK);
   } 

void cleanexit(char *s, int e)
   {
   message(s);
   cleanup();
   exit(e);
   }

void cleanup()
   {
   int k;

   if(myReq)   FreeMem(myReq,REQSIZE);
   if(port)    DeletePort(port);
   if(outFile) Close(outFile);
   if(wbuf)    FreeMem(wbuf,WBUFLEN+EXTRALEN);
   for(k=0; k<3; k++) if(allocsigs[k] != -1) FreeSignal(allocsigs[k]);
   Forbid();
   if(prevTaskName) mainTask->tc_Node.ln_Name = prevTaskName;
   Permit();
   if(GfxBase) CloseLibrary(GfxBase);
   }


void message(char *s)
   {
   LONG con;

   if((!FromWb)&&(*s)) printf(s);
   if((FromWb)&&(*s)&&(con = Open(conSpec,MODE_OLDFILE)))
      {
      Write(con,s,strLen(s));
      Delay(120L);
      Close(con);
      }
   }


void getWbArgs(struct WBStartup *wbMsg)
   {
   struct WBArg  *wbArg;
   struct DiskObject *diskobj;
   char **toolarray;
   char *s;

   /* Defaults */
   strCpy(wbDev,"parallel");
   strCpy(wbFile,"ram:CMD_file");
   Skip = FALSE;
   Multiple = FALSE;
   Notify = FALSE;

   wbArg = wbMsg->sm_ArgList;

   if((IconBase = OpenLibrary("icon.library", 0)))
      {
      diskobj=(struct DiskObject *)GetDiskObject(wbArg->wa_Name);
      if(diskobj)
         {
         toolarray = (char **)diskobj->do_ToolTypes;

         if(s=(char *)FindToolType(toolarray,"DEVICE"))strCpy(wbDev,s);
         if(s=(char *)FindToolType(toolarray,"FILE"))strCpy(wbFile,s);
         if(s=(char *)FindToolType(toolarray,"SKIP"))
            {
            if(strEqu(s,"TRUE"))  Skip = TRUE;
            }
         if(s=(char *)FindToolType(toolarray,"MULTIPLE"))
            {
            if(strEqu(s,"TRUE"))  Multiple = TRUE;
            }
         if(s=(char *)FindToolType(toolarray,"NOTIFY"))
            {
            if(strEqu(s,"TRUE"))  Notify = TRUE;
            }
         FreeDiskObject(diskobj);
         }
      CloseLibrary(IconBase);
      }
   }


/* String functions */

BOOL strEqu(TEXT *p, TEXT *q) 
   { 
   while(TOUPPER(*p) == TOUPPER(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 


int strLen(char *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

void strCpy(char *p, char *q)
{
    while (*p++ = *q++);
}

void strCat( char *to, char *from )
{
    while( *to ) to++;
    strCpy( to, from );
}
