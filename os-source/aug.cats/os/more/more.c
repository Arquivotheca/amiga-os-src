/* more.c - special link - see smakefile */

#define MORE

#define SHAREDCAT

#ifdef SHAREDCAT
#define CATALOGNAME	"sys/utilities.catalog"
#define INCLUDENAME	<localestr/utilities.h>
#else
#define CATALOGNAME	"more.catalog"
#define INCLUDENAME	"morestrings.h"
#endif


/* More.c  -  Carolyn Scheppner - (c) Copyright Commodore-Amiga, Inc.
 *
 *   Reentrant and requires RWstartup.obj and at least 1.2 OS
 *   Linked with RWstartup.obj and inLine.o, LIBRARY Amiga.lib, LC.lib
 *   (Conditionally compiled freely redis. version does't need inLine.o)
 *   RWstartup opens AppWindow as Amiga std Input(),Output() on WB startup
 *
 * v3 mods - faster virtual file, works with pipes, new commands %, .
 * v3.1 mods - Made reentrant, quit from help screen, WaitForChar upped
 * v3.2 mods - Fixed FreeMem of More structure
 * v3.21 mods - Fixed setMatch() call, handling of long lines
 * v3.22 mods - Added E edit function, CLI only, window titles
 * v3.23 mods - Backed out long line handling - it messed up search.
 *              inLine.asm now null rather than \n terminated
 * v3.24 mods - Position at start of a line after editor return
 * v3.25 mods - remove LF from exit cursor turnon, extern NeedWStartup
 * v3.26 mods - dummy ConUnit for non-window More (AUX:)
 * v3.27 mods - Fix Filename ? exit problems (wrong esc seq sent)
 * v3.28 mods - convert to Lattice 5.04, add CTRL/C out of search
 * v36_11 mods - get close gadget from raw event stream, internat toUpper
 * v36_12 mods - use file requester if available
 * v36_15 mods - request in own window, work with AUX and pipe again
 * v36_16 mods - get a char before exiting if CTRL_C signal received.
 * v36_17 mods - Fix bug that opened mo-star twice if run AND piped
 * v36_18 mods - Bump version, use asl tags, link with new 2.0 startup
 * v36_19 mods - check for input redirection == pr_ConsoleTask, get filename
 * v36_20 mods - change lmkfile to make regular and FR More
 * v36_21 mods - use FILF_NEWIDCMP FuncFlag if no userport in window
 * v37_1  mods - use rf_Pat and Pattern gadget
 * v37_2  mods - try to force autocons open, fix enforcer hits in findWindow
 * v37_3  mods - alt-SPACE and alt-B 1/2 page forward/back
 * v38_1  mods - localize, pad out CON specs for filezapping
 * v38_2  mods - stop complaining 0x9b is binary char
 * v38_3  mods - use shared catalog, make sure have conid in setRawCon
 * v38_4  mods - fix printing of % of file - needed strlen for insertion
 * v38_5  mods - special conditional prompt when running on CDTV
 * v39_1  mods - get rid of extra linefeed after adding to partial buffer
 * v39_2  mods - fix forwrd/backward/refresh after stepping forward by lines
 * v39_3  mods - allow editing if started from WB under V37 and higher
 * v39_4  mods - simplify code for file editing from WB
 * v39_5  mods - add REJECTICONS Asl flag
 *               disable autoscroll during search string & percent input
 * v39_6  mods - separate no-Window handling from no-ConUnit
 * v39_7  mods - dynamic mo->maxline to fix long line wrap problems
 * v39_8  mods - fix backPage to work much more accurately with wrapped lines
 * v39_9  mods - added reverse search (\ and , and p) with no localized help
 * v40_1  mods - version bump, fix backSeek (not clear charsthisline on read)
 *               and fix termination of back seek
 * v40_2  mods - fix search - was broken for wrapped strings due to fixes
 *               for displaying long lines better; count tabs properly
 * v40_3  mods - fixed locale string search rtn, SAS 6-ified
 *
 * TODO next time: use new CatComp, update localized strings, add multifile 
 */


#define MORE

#include <exec/types.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/asl.h>
#include <intuition/intuition.h>
#include <devices/conunit.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/asl_protos.h>
#include <clib/locale_protos.h>
extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(a,b) ((a)<=(b)?(a):(b))
#endif

/* Locale stuff */
#define  STRINGARRAY
#include INCLUDENAME
extern struct AppString AppStrings[];
struct Library *LocaleBase;
UBYTE *GetString(ULONG id, APTR catalog);
#define S(i)	GetString(i,mo->catalog)
#define SNM(i)	GetString(i,catalog)

/* For reference
struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};
*/


/**********    debug macros     ***********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);
#define DEBTIME 0
#define bug kprintf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#define D2(x) ;
#else
#define D(x) ;
#define D2(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

#include "more_rev.h"

char vers[]=VERSTAG;

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */


#define EOL 0x0a
#define EOF -1

/* N lines for editable, seekable, non-seekable */
#define EHELPLINES 14
#define HELPLINES  13
#define UNHELPLINES 9

/*
#define ACTION_SCREEN_MODE  994L
#define DOSTRUE  -1L
#define DOSFALSE  0L
*/


#define VBUFSZ  4096
#define SBUFSZ  256
#define IBUFSZ  128
#define MBUFSZ  128
#define EBUFSZ  128
#define PBUFSZ  128
#define NBUFSZ  (256)
#define BUFSZ   (VBUFSZ + SBUFSZ + IBUFSZ + MBUFSZ + EBUFSZ + PBUFSZ)
#define ABSMAXLINE (160)
#define MAXGETS 100


/* for SetWindowTitle */
struct Library *IntuitionBase;

/* for file requester */
struct Library *AslBase;

/* for RWstartup */
extern UBYTE  NeedWStartup;
UBYTE  *HaveWStartup = &NeedWStartup;


char Copyright[] = 
 "(c) Copyright 1986-94 Commodore-Amiga, Inc.  All Rights Reserved";

char AppWindow[] = 
  "CON:0000/0000/0640/0200/ " VERS " (c) Copyright 1986-94 Commodore-Amiga, Inc. /close";


UBYTE winClrStr[] = "\033[0 p\033[%ld;0H\033[K";
UBYTE auxClrStr[] = "\033[099D\033[099B\033[K";
UBYTE eformat[] = "%s <* >NIL: \"%s\"";

struct More
   {
   /* Local for IntuitionBase */
   struct Library *intuitionBase;
   struct Library *localeBase;
   APTR   catalog;

   /* Initialized by findWindow() */
   struct Window  *conWindow;
   struct ConUnit *conUnit;

   /* For stdio, pipes, run'd More, etc */
   LONG  stdIn, stdOut;
   LONG  oldStdin, oldStdout;
   struct Process  *proc;
   APTR   oldConTask;

   /* File stuff */
   UBYTE *filename, *oldWTitle;
   BOOL IsCDTV, NeedFilename;
   LONG openedFile, file, star;
   int  rLen, mLen, wLen, gPos, shownPagePos, lineBump, fSize;
   LONG startLock, newLock;

   /* Flags */
   BOOL   FromWb, FormFeed, Found, Sensitive, UnSeekable, CanEdit, SearchBreak;
   BOOL	  RealConUnit,BackSearch;

   /* Virtual buffer index, end index, file pos, current pos, avail chars */
   LONG vbufi,vbpos,vbipos,vbChars,vbCharsLeft,maxline;

   struct Library *aslBase;
   struct FileRequester *freq;
   UBYTE fnamebuf[NBUFSZ];

   /* Dummy partial conunit for AUX: */
   ULONG dummyCU[16];
   /* Virtual file buffer, string buffer, input buffer, match buffer, pathbuf */
   UBYTE vbuf[VBUFSZ], sbuf[SBUFSZ], ibuf[IBUFSZ], mbuf[MBUFSZ], pbuf[PBUFSZ];
   /* Environment buffer */
   UBYTE ebuf[EBUFSZ];
   };

/* Protos */
extern int inLine(UBYTE *, UBYTE *);
BOOL search(struct More *);
void showUsage(struct More *mo);
int containsBinary(struct More *mo);
int showNext(struct More *mo);
void showPage(struct More *mo);
int showLine(struct More *mo);
int getLine(char *s, struct More *mo);
void showHelp(struct More *mo);
int input(char *s, char *buf, struct More *mo);
BOOL doSearch(struct More *mo);
void setMatch(char *sm, char *si, struct More *mo);
void prevPage(struct More *mo);
void prevHPage(struct More *mo);
void nextHPage(struct More *mo);
void lastPage(struct More *mo);
void clearPage(struct More *mo);
void backSeek(int blines, struct More *mo);
void printPrompt(char *s, struct More *mo);
void clearLast(struct More *mo);
void clearBLast(struct More *mo);
int seekToShown(struct More *mo);
LONG getFileSize(LONG fh);
LONG myRead(LONG fh, char *buf, int l, struct More *mo);
LONG fillVbuf(LONG fh, struct More *mo);
LONG mySeek(LONG fh, LONG pos, LONG mode, struct More *mo);
UBYTE mogetc(struct More *mo);
int mogets(char *s, struct More *mo);
void strCpy(char *to, char *from);
void toUppers(char *s);
int Atoi( char *s );
void wTitle(UBYTE *title, struct More *mo);
void cleanexit(char *s, LONG err, struct More *mo);
LONG setRawCon(LONG toggle);
LONG findWindow(struct More *mo);
LONG sendpkt(struct MsgPort *pid, LONG action, LONG args[], LONG nargs);
UBYTE *mogetfilename(struct More *);
BOOL  AslRequestTags( APTR request, ULONG tags, ...);

/* amiga.lib functions */
LONG sprintf(UBYTE *buffer, BYTE *fmt,...);
LONG fgetc(LONG);
LONG fprintf(LONG, BYTE *fmt,...);
struct MsgPort *CreatePort( UBYTE *name, LONG pri );
void DeletePort( struct MsgPort *io );

/* to shut up warning */
void SetWindowTitles(struct Window *, UBYTE *, LONG);


void main(int argc, char **argv)
   {
   struct More     *mo;
   struct WBStartup *WBenchMsg;
   struct WBArg     *arg;
   struct CommandLineInterface *cli=NULL;  
   struct Library *alib, *llib;
   struct Process *proc;
   APTR prwptr, catalog = NULL;

   /* Try to open possibly missing disk-based libs
    * Must do locale early so error messages will be in right language
    */
   proc = (struct Process *)FindTask(NULL);
   prwptr = proc->pr_WindowPtr;
   proc->pr_WindowPtr = (APTR)-1L;
   if(llib = OpenLibrary("locale.library",0L))	LocaleBase = llib;
   if(alib = OpenLibrary("asl.library",0L))	AslBase = alib;
   proc->pr_WindowPtr = prwptr;

   if(LocaleBase) catalog = OpenCatalogA(NULL,CATALOGNAME,NULL);

   if(!(mo=(struct More *)
     AllocMem(sizeof(struct More),MEMF_CLEAR|MEMF_PUBLIC)))
      {
      if(Output()) fprintf(Output(),SNM(MSG_MO_NOMEM));
      if(!argc)   Delay(100);
      exit(RETURN_FAIL);
      }


   if((FindName(&SysBase->DeviceList,"cdtv.device"))
		&&(SysBase->LibNode.lib_Version <= 37)) mo->IsCDTV = TRUE;

   /* NOTES:  mo->FromWb initially FALSE
    *         Rstartups pass WBenchMsg in argv
    */
   mo->proc = proc;
   mo->FromWb = (argc==0) ? TRUE : FALSE;
   if(mo->FromWb) WBenchMsg = (struct WBStartup *)argv;
   else cli  = (struct CommandLineInterface *)(mo->proc->pr_CLI << 2);

   mo->stdOut = Output();
   mo->stdIn  = Input();

   /* We will close based on whether THIS invocation got them open */
   mo->aslBase = alib;
   mo->localeBase = llib;
   mo->catalog = catalog;

   if(DOSBase->lib_Version < 33)
      cleanexit(S(MSG_MO_NODOS),RETURN_FAIL,mo);

   if(!(mo->intuitionBase=(OpenLibrary("intuition.library",0L))))
      cleanexit(S(MSG_MO_NOINTUITION),RETURN_FAIL,mo);
   IntuitionBase = mo->intuitionBase;

   if((argc>1)&&(argv[1][0]!='?'))  /* CLI filename argument */
      {
      mo->filename = argv[1];
      }
   else if((mo->FromWb)&&(WBenchMsg->sm_NumArgs > 1))
      {                        /* Passed filename via  WorkBench */
      arg = WBenchMsg->sm_ArgList;
      arg++;
      mo->filename   = (char *)arg->wa_Name;
      mo->newLock    = (LONG)arg->wa_Lock;
      mo->startLock  = (LONG)CurrentDir(mo->newLock);
      }
   else if((argc==2)&&(argv[1][0]=='?')) /* wants help */
      {
      showUsage(mo);
      cleanexit(" ",RETURN_OK,mo);
      }
   else if((mo->FromWb)||
	((cli)&&(cli->cli_StandardInput==mo->stdIn))||
	/* added so no filename with redirection from star not interpreted as pipe */
	(((struct FileHandle *)(mo->stdIn << 2))->fh_Type==mo->proc->pr_ConsoleTask))
      {
      mo->NeedFilename = TRUE;
      }


   if((cli)&&(cli->cli_Background))
      {
      if(!(mo->star = Open(AppWindow,MODE_OLDFILE)))
         cleanexit(S(MSG_MO_NOWINDOW),RETURN_FAIL,mo);
      Forbid();
      mo->oldConTask = mo->proc->pr_ConsoleTask;
      mo->proc->pr_ConsoleTask =
         (APTR)((struct FileHandle *)(mo->star << 2))->fh_Type;
      mo->oldStdin = mo->stdIn;
      mo->stdIn = mo->star;
      if(mo->stdOut == cli->cli_StandardOutput)
         {
         mo->oldStdout = mo->stdOut;
         mo->stdOut = mo->star;
         }
      Permit();
      }


   /* Find the window we are in (mo->conWindow == 0 if AUX) */
   if(! findWindow(mo))
      {
      /* failed - maybe it's an autocon...
       * force the sucker open...  grrrrr....
       */
      if(Output())	Write(Output()," \008",2);
      if(! findWindow(mo))
	  {
          cleanexit(S(MSG_MO_NOPACKET),RETURN_FAIL,mo);
	  }
      }

   if(mo->NeedFilename)
      {
      mo->filename = mogetfilename(mo);
      /* newline error message to prevent clearLast in exit code */
      if(!mo->filename) cleanexit("\n",RETURN_OK,mo);
      }

   D(bug("About to open file\n"));

   if(mo->filename)
      {
      if(!(mo->file = Open(mo->filename, MODE_OLDFILE)))
         cleanexit(S(MSG_MO_NOFILE),RETURN_WARN,mo);
      mo->openedFile = mo->file;
      if(((struct FileHandle *)(mo->file << 2))->fh_Type==mo->proc->pr_ConsoleTask)
         cleanexit(S(MSG_MO_NOSTAR),RETURN_WARN,mo);
      }
   else
      {
      if(! mo->star) mo->star = Open("*",MODE_OLDFILE);
      mo->oldStdin = mo->stdIn;
      mo->stdIn = mo->star;
      mo->file = Input();
      }

   /* Init */

   /* Set up match function */
   mo->Sensitive = TRUE;

   D(bug("About to try seek\n"));

   /* See if we can seek on this file */
   Seek(mo->file,2,OFFSET_BEGINING);
   if(Seek(mo->file,0,OFFSET_BEGINING) != 2)  mo->UnSeekable = TRUE;

   mo->fSize = mo->UnSeekable ? 0 : getFileSize(mo->file);

   setRawCon(DOSTRUE);

   D(bug("About to do title stuff\n"));

   /* Note - do not call clearPage(mo) before this! */
   if(mo->conWindow)
      {
      mo->oldWTitle = mo->conWindow->Title;
      wTitle(mo->filename,mo);
      }

   if(mo->conUnit) 	mo->RealConUnit = TRUE;
   else 		/* need dummyCU */
      {
      mo->conUnit = (struct ConUnit *)mo->dummyCU;
      mo->conUnit->cu_XMax = 80;
      mo->conUnit->cu_YMax = 24;
      mo->conUnit->cu_XCP = 1;
      mo->conUnit->cu_YCP = 1;
      }
   mo->maxline = MIN(mo->conUnit->cu_XMax + 1,ABSMAXLINE);

   clearPage(mo);  /* may not be called before above lines */

   mo->Found = TRUE;  /* Init flag that causes search "Not Found" prompt */

   if(containsBinary(mo))
      {
      printPrompt(S(MSG_MO_BINARY),mo);
      /* mogetc converts Close and CTRL-C to 'q' */
      if(mogetc(mo) == 'q') cleanexit("",RETURN_OK,mo);
      }

   /* We can edit real file from CLI, or from WB if >= V37 */
   mo->CanEdit = ((mo->openedFile)&&
		 ((!mo->FromWb)||(DOSBase->lib_Version >= 37))&&
		 (!mo->UnSeekable));

   showPage(mo);
   /* error, quit, or break sets rLen = -1 */
   while(mo->rLen > -1)
      {
      if(!mo->Found) printPrompt(S(MSG_MO_NOTFOUND),mo);
      else if(mo->rLen == 0)
         {
         printPrompt(S(MSG_MO_EOF),mo);
         }
      else
         {
         mo->gPos = mySeek(mo->file,0,OFFSET_CURRENT,mo);
         sprintf(mo->sbuf,S(MSG_MO_MORE));
         if(mo->fSize)
           sprintf(&mo->sbuf[strlen(mo->sbuf)]," (%ld%%)",(100*mo->gPos)/mo->fSize);

	 if(mo->IsCDTV)
	  sprintf(&mo->sbuf[strlen(mo->sbuf)],"   CDTV: 4=Forward 1=Back 7=Quit");

         printPrompt(mo->sbuf,mo);
         }
      showNext(mo);
      }
   cleanexit("",RETURN_OK,mo);
   }


void showUsage(struct More *mo)
   {
   int k;
   fprintf(mo->stdOut,"%s %s\n",VERS,Copyright);
   for(k=MSG_MO_USEFIRST; k<=MSG_MO_USELAST; k++) fprintf(mo->stdOut,"%s\n",S(k));
   }

int containsBinary(struct More *mo)
   {
   int k;
   UBYTE ch;

   myRead(mo->file,mo->sbuf,80,mo);
   mySeek(mo->file,0,OFFSET_BEGINING,mo);
   for (k=0; k<80; k++)
      {
      ch = mo->sbuf[k];
      if(((ch>0x00)&&(ch<0x08))||
         ((ch>0x0d)&&(ch<0x1b))||
         ((ch>0x1b)&&(ch<0x20))||
         ((ch>0x7f)&&(ch<0x9b))||
         ((ch>0x9b)&&(ch<0xa0)))  return(1);
      }
   return(0);
   }

/* showNext - sets rLen = -1 if 'q' or if ' ' on last screen */
int showNext(struct More *mo)
   {
   ULONG ppos;
   UWORD pct;
   /* for new E command */
   LONG  env, erLen, nilfh, flock;
   int k;
   UBYTE *prompt,ch;

   mo->Found = TRUE;
   while(!(WaitForChar(mo->stdIn,200000)))
      {
      if(SetSignal(0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
         {
         mo->rLen = -1;
	 if(WaitForChar(mo->stdIn,200000))  mogetc(mo);
         return(0);
         }
      }
   ch = mogetc(mo);   /* converts Close Gadget and CTRL-C to 'q', Help to 'h' */

   /* Disable some functions if piped or couldn't get file size */
   if((mo->UnSeekable)&&((ch==0x08)||(ch=='%')||(ch=='<')||(ch=='>')))
      ch = NULL;
   if((!mo->fSize)&&((ch=='%')||(ch=='>')))  ch = NULL;

   /* Note - mogetc() converted CTRL-C and Close to 'q', Help key to 'h' */ 
   switch(ch)
      {
      case ' ': case '4':               /* Page */
         if (mo->rLen==0)  mo->rLen = -1;
         else if (mo->rLen > 0)  showPage(mo);
         break;

      case 0x0d:              /* Line */
         if (mo->rLen==0)  mo->rLen = -1;
         else if (mo->rLen > 0)
            {
            clearLast(mo);
            showLine(mo);
	    mo->lineBump++;
            }
         break;

      case 0x0C:    /* CTRL/L  Refresh window */
         mo->shownPagePos = seekToShown(mo);
         showPage(mo);
         break;

      case '<':     /* First Page */
         mySeek(mo->file,0,OFFSET_BEGINING,mo);
         showPage(mo);
         break;

      case '>':     /* Last Page */
         lastPage(mo);
         showPage(mo);
         break;

      case 0x08: case 'b': case '1':  /* Backspace or b = previous page */
         printPrompt(S(MSG_MO_PREVPAGE),mo);
	 mo->shownPagePos = seekToShown(mo);
         prevPage(mo);
         showPage(mo);
         break;
      case 0xba:             /* alt-B is back 1/2 page */
	 mo->shownPagePos = seekToShown(mo);
         prevHPage(mo);
         showPage(mo);
         break;
      case 0xa0:              /* alt-SPACE is 1/2 page forward */
	 mo->shownPagePos = seekToShown(mo);
         if (mo->rLen==0)  mo->rLen = -1;
         else if (mo->rLen > 0)
		{
		nextHPage(mo);
		showPage(mo);
		}
         break;

      case '\\':  case ',':   /* Backwards search - falls through */
	 prompt = (ch =='\\') ? "<-/" : "<-."; 
      case '/':  case '.':   /* Get match string */
         mo->Sensitive = ((ch=='/')||(ch=='\\')) ? TRUE : FALSE;
	 mo->BackSearch = ((ch=='/')||(ch=='.')) ? FALSE: TRUE;
	 if(ch == '/')		prompt = "/"; /* preserves prompt set above */
	 else if(ch ==  '.')	prompt = ".";
         mo->mLen = input(prompt,mo->ibuf,mo);
         if(!mo->mLen)  break;
         setMatch(mo->mbuf,mo->ibuf,mo);
	 doSearch(mo);
	 break;
      case 'n': case 'p':     /* Search again (next or previous) */
	 mo->BackSearch = (ch=='p') ? TRUE : FALSE;
	 doSearch(mo);
         break;

      case '%':
         input("%",mo->sbuf,mo);
         pct = Atoi(mo->sbuf);
         if(pct > 100)  pct = 100;
         ppos = ((mo->fSize - 1) * pct) / 100;
         mo->shownPagePos = ppos; /* kludge to make prevPage work */
         prevPage(mo);
         showPage(mo);
         break;

      case 'E':
         if(mo->CanEdit) /* may be set if FromWb and V37 or higher */
            {
            if(env = Open(S(MSG_MO_EDITOR),MODE_OLDFILE))
               {
               for(k=0;k<EBUFSZ;k++) mo->ebuf[k] = 0;
               erLen = Read(env,mo->ebuf,EBUFSZ-2);
               Close(env);

               if(erLen > 0)
                  {
		  mo->shownPagePos = seekToShown(mo);  /* uses sbuf */

		  if(mo->FromWb) /* CanEdit if V37 or higher */
		      {
		      if(flock = Lock(mo->filename,ACCESS_READ))
		 	{
		      	NameFromLock(flock,mo->pbuf,PBUFSZ);
                      	sprintf(mo->sbuf,eformat,mo->ebuf,mo->pbuf);
			D(bug("will Execute: %s\n",mo->sbuf));
			UnLock(flock);
			}
		      }
                  else sprintf(mo->sbuf,eformat,mo->ebuf,mo->filename);


                  /* Close file so editor can have full access */
                  Close(mo->openedFile);
                  mo->openedFile = NULL;
                  clearPage(mo);
                  wTitle(mo->oldWTitle,mo);
                  Execute(mo->sbuf,0,nilfh=Open("NIL:",MODE_OLDFILE));
		  if(nilfh)	Close(nilfh);
                  Delay(25);
                  if(!(mo->file = Open(mo->filename,MODE_OLDFILE)))
                     cleanexit(S(MSG_MO_NOFILE2),RETURN_WARN,mo);
                  mo->openedFile = mo->file;
                  mo->fSize = getFileSize(mo->file);
                  mo->vbChars = 0;  /* force refill of buffer */

                  k = (mo->shownPagePos >= mo->fSize) ? 0 : mo->shownPagePos;
                  mySeek(mo->file,k,OFFSET_BEGINING,mo);
                  if(k) backSeek(1,mo);

                  wTitle(mo->filename,mo);
                  showPage(mo);
                  }
               }
            }
         break;
   
      case 'h':     /* Help */
         showHelp(mo);
         break;

      case 'q': case 'Q': case '7': /* Quit ( mogetc made CTRL-C and Close be 'q' ) */
         if(mo->SearchBreak) /* CTRL-C is quit unless from search */
	    {
	    D(bug("SearchBreak is TRUE\n"));
            mo->SearchBreak = FALSE;
            break;
            }
         mo->rLen = -1;
         break;

      default:
         if(ch)
            {
            printPrompt(S(MSG_MO_PRESSH),mo);
            Delay(50);
            }
         break;
      }
   }


BOOL doSearch(struct More *mo)
    {
    LONG winmax;

    /* save real window maxline, use bigger & search for string, reset maxline */
    winmax = mo->maxline;
    mo->maxline = ABSMAXLINE;
    mo->Found = search(mo);
    mo->maxline = winmax;

    if(mo->Found || mo->UnSeekable)  showPage(mo);
    return(mo->Found);
    }

int seekToShown(struct More *mo)
    {
    int k;

    mySeek(mo->file,mo->shownPagePos,OFFSET_BEGINING,mo);
    if(mo->lineBump)
	{
    	for(k=0; k<mo->lineBump; k++)  getLine(mo->sbuf,mo);
    	mo->lineBump = 0;
	}
    return(mySeek(mo->file,0,OFFSET_CURRENT,mo));
    }

/* showPage - exits with rLen = -1 if EOF */
void showPage(struct More *mo)
   {
   mo->rLen = 1;
   mo->FormFeed = FALSE;
   mo->shownPagePos = mySeek(mo->file,0,OFFSET_CURRENT,mo);
   mo->lineBump = 0;
   clearPage(mo);

   /* This seek would not be efficient normally
    * but allows search to work properly with pipes
    */
   if(mo->UnSeekable)  fillVbuf(mo->file,mo);

   while((mo->rLen > 0)&&(!mo->FormFeed)&&
     /* and cursor is not already on last line */
     (mo->conUnit->cu_YCP < mo->conUnit->cu_YMax)&&
     /* and cursor is not on last char of next to last line */
     (!((mo->conUnit->cu_YCP == (mo->conUnit->cu_YMax-1))&&
		(mo->conUnit->cu_XCP == mo->conUnit->cu_XMax))))
      showLine(mo);
   }


int showLine(struct More *mo)
   {
   int len;

   len = getLine(mo->sbuf,mo);

   D2(bug("Got line \"%s\" length=%ld, vbCharsLeft=%ld, vbipos=%ld, vbufi=%ld, last two chars: $%lx %lx\n",
	mo->sbuf,len,mo->vbCharsLeft,mo->vbipos,mo->vbufi,mo->sbuf[len-2], mo->sbuf[len-1]));

   mo->wLen = Write(mo->stdOut,mo->sbuf,len);
   if(!(mo->RealConUnit)) mo->conUnit->cu_YCP += 1;
   return(len);
   }


/* getLine - returns length for write */
int getLine(char *s, struct More *mo)
   {
   int linepos, l = 0, lv = 0;  /* lv is how many visible character spaces */

   D2(bug("getLine entry, vbipos=%ld, vbufi=%ld\n",mo->vbipos,mo->vbufi));

   linepos = mySeek(mo->file,0,OFFSET_CURRENT,mo);

   D2(bug("getLine after seek, vbipos=%ld, vbufi=%ld\n",mo->vbipos,mo->vbufi));

   if((mo->rLen = myRead(mo->file,s,mo->maxline,mo)) > 0)
      {
      /* Find end of line in buffer */
      while((l<mo->rLen)&&(lv<mo->maxline)&&(*s!=EOL)&&(*s!=0x0C))
	{
	if(*s == 0x09)	lv = (lv+8)&(~7);	/* TAB increases visible chars */
	else		lv++;
	l++;					/* actual string length */
	s++;					/* string pointer */
	}
      if((*s==EOL)|(*s==0x0c)) l++;  /* l now = length of string */

      D2(bug("getLine - seeking to %ld (linepos %ld + l %ld)\n",
		linepos+l,linepos,l));
      /* Seek in file to start of next line */
      mySeek(mo->file,linepos+l,OFFSET_BEGINING,mo);

      if(*s==0x0C)   /* Visualize FormFeed */
         {
         mo->FormFeed = TRUE;  /* Flag last line for this screen */
         *s++ = '^';
         *s++ = 'L';
         *s++ = '\n';
         l += 2;
         }
      s++, *s = NULL;  /* Null terminate */
      }

   D(bug("getLine - rLen was %ld, returning %ld, lv=%ld, vbipos=%ld, vbufi=%ld\n",
			mo->rLen,l,lv,mo->vbipos,mo->vbufi));
   return(l);
   }


void showHelp(struct More *mo)
   {
   int k,n;
   UBYTE c;

   n = ((mo->UnSeekable)||(!mo->fSize)) ? UNHELPLINES : HELPLINES;
   if(mo->CanEdit) n = EHELPLINES;

   for(k=0; k<n; )
      {
      clearPage(mo);
      while((mo->conUnit->cu_YCP < mo->conUnit->cu_YMax)&&(k<n))
         {
         fprintf(mo->stdOut,"   %s\n",S(k + MSG_MO_HELPFIRST));
         k++;
         }
      printPrompt(S(MSG_MO_MORE),mo);
      c = mogetc(mo);
      if(c=='q') cleanexit("",RETURN_OK,mo); /* Close and CTRL-C converted */
      }
   mo->shownPagePos = seekToShown(mo);
   showPage(mo);
   }

/* search - returns TRUE Found, FALSE not, exits with rLen = -1 if EOF */
BOOL search(struct More *mo) 
   {
   int oldpos, startpos, linepos, k;

   k=0;
   if(mo->BackSearch)
	{
	sprintf(mo->sbuf,"<- ");
	k=3;
	}

   sprintf(&mo->sbuf[k],(mo->Sensitive ? 
	S(MSG_MO_CSEARCH) : S(MSG_MO_ISEARCH)), mo->mbuf);

   printPrompt(mo->sbuf, mo);
   oldpos   = mySeek(mo->file,0,OFFSET_CURRENT,mo);
   startpos = seekToShown(mo);
   /* don't search first line of shown page if going forward */
   if(!mo->BackSearch) getLine(mo->sbuf,mo);

   mo->rLen = 1;
   while((mo->rLen > 0)&&(!(SetSignal(0,0)&SIGBREAKF_CTRL_C)))
      {
      if(mo->BackSearch) backSeek(1,mo);
      linepos = mySeek(mo->file,0,OFFSET_CURRENT,mo);
      getLine(mo->sbuf,mo);
      if(!mo->Sensitive) toUppers(mo->sbuf);

      D(bug("searching for %s in %s\n",mo->mbuf,mo->sbuf));

      if(inLine(mo->sbuf,mo->mbuf))
         {
         mySeek(mo->file,linepos,OFFSET_BEGINING,mo);
         mo->rLen = 1;
         return(TRUE);
         }
      if(mo->BackSearch)
	 {
	 if(!linepos)	break;
	 mySeek(mo->file,linepos,OFFSET_BEGINING,mo);
	 }
      }
   mySeek(mo->file,oldpos,OFFSET_BEGINING,mo);
   mo->rLen = 1;
   if((SetSignal(0,SIGBREAKF_CTRL_C))&SIGBREAKF_CTRL_C) mo->SearchBreak = TRUE;
   return(FALSE);
   }

int input(char *s, char *buf, struct More *mo)
   {
   int l;

   setRawCon(DOSFALSE);
   clearLast(mo);
   /* Normal set, string, normal, cursor on */
   fprintf(mo->stdOut,"\017\033[0m%s",s);

   /* cursor on, scroll off */
   if(mo->RealConUnit) fprintf(mo->stdOut,"\033[1 p\033[>1l");
   l = mogets(buf,mo);
   /* scroll on */
   if(mo->RealConUnit) fprintf(mo->stdOut,"\033[>1h");

   setRawCon(DOSTRUE);
   /* Turn cursor off, pos at next to last line, clear line */
   if(mo->RealConUnit) clearLast(mo);
   else clearBLast(mo);
   return(l);
   }

void setMatch(char *sm, char *si, struct More *mo)
   {
   strCpy(sm,si);
   if(!mo->Sensitive) toUppers(sm);
   }

void prevPage(struct More *mo)
   {
   seekToShown(mo);
   /* Seek back 1 screenful of lines */
   backSeek(mo->conUnit->cu_YMax,mo);
   }

void prevHPage(struct More *mo)
   {
   /* Seek to beginning of shown page */
   seekToShown(mo);
   /* Seek back 1/2 screenful of lines */
   backSeek(mo->conUnit->cu_YMax + 1 >> 1,mo);
   }

void nextHPage(struct More *mo)
   {
   /* Seek back 1/2 screenful of lines from where we are at end of page */
   backSeek(mo->conUnit->cu_YMax - (mo->conUnit->cu_YMax >> 1),mo);
   }

void lastPage(struct More *mo)
   {
   mySeek(mo->file,0,OFFSET_END,mo);
   /* Seek back 1 screenful of lines - 1 */
   backSeek((mo->conUnit->cu_YMax)-1,mo);
   }

void backSeek(int blines, struct More *mo)
   {
   int lines;
   int pos, backseek,charsthisline;
   char *s, *es;

   D(bug("backSeek %ld lines\n",blines));

   lines = charsthisline = 0;
   mo->FormFeed = FALSE;
   s = es = mo->sbuf;

   pos = mySeek(mo->file,0,OFFSET_CURRENT,mo);

   if (!(pos==0)) pos = mySeek(mo->file,-1,OFFSET_CURRENT,mo), pos--;

   mo->rLen = 1;
   while((mo->rLen>0)&&(lines<blines)&&(pos>0)&&(!mo->FormFeed))
      {
      /* Seek back a buffer's length or to beginning */
      backseek = (pos < ABSMAXLINE) ? pos : ABSMAXLINE;

      D(bug("seeking back %ld and reading %ld",-backseek,backseek));

      pos = mySeek(mo->file,-backseek,OFFSET_CURRENT,mo);
      if(!(pos>0))  mySeek(mo->file,0,OFFSET_BEGINING,mo);

      /* Get current pos, read bufferful, repos to pos */
      pos = mySeek(mo->file,0,OFFSET_CURRENT,mo);
      mo->rLen = myRead(mo->file,s,backseek,mo);

      mySeek(mo->file,pos,OFFSET_BEGINING,mo);   /* pos = current pos */

      if(mo->rLen > 0)
         {
         for(es=s+backseek-1; (es>=s)&&(lines<blines)&&(!mo->FormFeed); es--)
            {
	    D(bug("%lc",*es));
            charsthisline++;
            if(*es==EOL)
		{
		lines++;
		charsthisline=0;
		D(bug(" **+EOL** "));
		}
            else if(*es==0x0C)
		{
		mo->FormFeed = TRUE;
		charsthisline=0;
		}
            else if(*es==0x09)	/* TAB */
		{
		/* normally would be (+8)&(~7) but we already added 1 above */
		charsthisline=(charsthisline+7)&(~7);
		}
            else if(charsthisline>=mo->maxline)
		{
		lines++;
		charsthisline=0;
		D(bug(" **+maxline** "));
		}
            }
         }
      }

   D(bug("lines=%ld blines=%ld pos=%ld may seek ahead %ld\n",lines,blines,pos,es-s+2));
   if((lines==blines)||(mo->FormFeed))
	 mySeek(mo->file,es-s+2,OFFSET_CURRENT,mo);
   }



/* NormalSet ItalicsOn RvsOn --- %s --- Normal */
void printPrompt(char *s, struct More *mo)   
   {
   clearLast(mo);
   fprintf(mo->stdOut," \017\033[3m\033[7m --- %s --- \033[0m",s);
   }


void clearPage(struct More *mo)
   {
   if(mo->RealConUnit)
      {
      fprintf(mo->stdOut,"\033[0 p\014");  /* Turn off cursor, home and clear */
      }
   else
      {
      mo->conUnit->cu_YCP = 1;
      fprintf(mo->stdOut,"\033[0;0H\033[J");
      }
   mo->maxline = MIN(mo->conUnit->cu_XMax + 1,ABSMAXLINE);
   }


void clearLast(struct More *mo)
   {
   if(mo->RealConUnit)
   	fprintf(mo->stdOut,winClrStr, mo->conUnit->cu_YMax + 1);
   else fprintf(mo->stdOut,auxClrStr,"");
   }

/* Clears line before last */
void clearBLast(struct More *mo)
   {
   if(mo->RealConUnit)
    fprintf(mo->stdOut,winClrStr, mo->conUnit->cu_YMax);
   else fprintf(mo->stdOut,"%s%s",auxClrStr,"\033[A\033[K");
   }


LONG getFileSize(LONG fh)
   {
   LONG size;

   Seek(fh,0,OFFSET_END);
   size = Seek(fh,0,OFFSET_BEGINING);
   if(size < 0) size = 0;
   return(size);
   }

   
LONG myRead(LONG fh, char *buf, int l, struct More *mo)
   {
   LONG rlen;

   D2(bug("myRead entry: vbCharsLeft=%ld, l=%ld, vbufi=%ld\n",
		mo->vbCharsLeft,l,mo->vbufi));

   if(mo->vbCharsLeft < l)
      {
      rlen = fillVbuf(fh,mo);
      if(!(mo->vbCharsLeft))  return(rlen); /* 0=EOF or -1=error */
      }

   rlen = (l <= mo->vbCharsLeft) ? l : mo->vbCharsLeft;

   D2(bug("myRead - copying %ld bytes to &vbuf[%ld] (vbufi)\n",rlen,mo->vbufi));

   CopyMem(&mo->vbuf[mo->vbufi],buf,rlen);
   mo->vbufi += rlen;
   mo->vbipos += rlen;
   mo->vbCharsLeft -= rlen;

   D2(bug("myRead exit: vbCharsLeft=%ld, rlen=%ld, vbufi=%ld, vbipos=%ld\n",
		mo->vbCharsLeft,rlen,mo->vbufi,mo->vbipos));

   return(rlen);
   }


LONG fillVbuf(LONG fh, struct More *mo)
   {
   LONG rlen;

   D2(bug("fill entry, vbCharsLeft=%ld, vbipos=%ld\n",
		mo->vbCharsLeft,mo->vbipos));

   if(mo->vbCharsLeft)
      {
      /* If we've got the end of the file in buffer already, return */
      if(mo->vbChars != VBUFSZ)
	{
      	D2(bug("fill - vbCharsLeft=%ld, returning 0\n", mo->vbCharsLeft));
	return(0);
	}
      else
	{
      	D2(bug("fill - vbCharsLeft=%ld, copying them to vbuf[%ld] (vbufi)\n", 
		mo->vbCharsLeft, mo->vbufi));
	CopyMem(&mo->vbuf[mo->vbufi],mo->vbuf,mo->vbCharsLeft);
	}
      }

   mo->vbpos = mo->vbipos;

   rlen = Read(fh,&mo->vbuf[mo->vbCharsLeft],VBUFSZ - mo->vbCharsLeft);

   if(rlen > 0) mo->vbCharsLeft += rlen;

   mo->vbufi = 0;
   mo->vbChars = mo->vbCharsLeft;
   mo->vbipos = mo->vbpos;

   D2(bug("fill exit got %ld chars, vbCharsLeft=%ld last two chars: $%lx %lx\n",
	rlen, mo->vbCharsLeft,
	mo->vbuf[mo->vbCharsLeft -2],
	mo->vbuf[mo->vbCharsLeft -1]));

   return(rlen);
   }


LONG mySeek(LONG fh, LONG pos, LONG mode, struct More *mo)
   {
   LONG oldpos, reqpos, backoff;

   D2(bug("mySeek entry - pos=%ld, vbpos=%ld, vbipos=%ld, vbufi=%ld\n",
		pos,mo->vbpos, mo->vbipos,mo->vbufi));

   switch(mode)
      {
      case OFFSET_BEGINING:
         reqpos = pos;
         break;
      case OFFSET_CURRENT:
         reqpos = mo->vbipos + pos;
         break;
      case OFFSET_END:
         reqpos = mo->fSize ? mo->fSize - 1 + pos : mo->vbipos;
         break;
      default:
         return(-1);
         break;
      }

   oldpos = mo->vbipos;

   /* If Seek within vbuf, just change pointers
    */
   if((mo->vbChars)&&(reqpos>=mo->vbpos)&&(reqpos<(mo->vbpos+mo->vbChars)))
      {
      D2(bug("vbChars=%ld mo->vbCharsLeft=%ld reqpos=%ld vbpos=%ld vbipos=%ld vbufi=%ld\n",
		mo->vbChars, mo->vbCharsLeft, reqpos, mo->vbpos, mo->vbipos, mo->vbufi));
      mo->vbipos = reqpos;
      mo->vbufi  = mo->vbipos - mo->vbpos;
      mo->vbCharsLeft = mo->vbChars - mo->vbufi;
      D2(bug("mySeek - set vbCharsLeft to %ld\n",mo->vbCharsLeft));
      }
   else  /* Seek not within vbuf */
      {
      /* try to optimize backseeking a bit */
      if(reqpos<mo->vbpos)
         {
         if(mo->UnSeekable)
            {
            /* position to start of buffer */
            mo->vbufi = 0;
            mo->vbipos = mo->vbpos;
            mo->vbCharsLeft = mo->vbChars;
            return(oldpos);
            }
         else    /* Seekable Backseek */
            {
            if(reqpos > (VBUFSZ - ABSMAXLINE))  backoff = VBUFSZ - ABSMAXLINE;
            else  backoff = reqpos;
            Seek(fh,reqpos-backoff,OFFSET_BEGINING);
            mo->vbChars = 0;
            mo->vbCharsLeft = 0;
            fillVbuf(fh,mo);
            mo->vbufi = backoff;
            mo->vbipos = reqpos;
            mo->vbpos = reqpos - backoff;
            }
         }
      else   /* Forward Seek */
         {
         mo->vbChars = 0;
         mo->vbCharsLeft = 0;
         mo->vbufi = 0;
         if(mo->UnSeekable)
            {
            /* mo->vbipos += 1;  caused the extra linefeed bug */
            mo->vbpos = mo->vbipos;
            }
         else
            {
            Seek(fh,pos,mode);
            mo->vbipos = reqpos;
            mo->vbpos  = reqpos;
            }
         }
      }

   D2(bug("mySeek exit - pos=%ld, vbpos=%ld, vbipos=%ld, vbufi=%ld\n",
		pos,mo->vbpos, mo->vbipos,mo->vbufi));

   return(oldpos);
   }


/* mogetc - for character input from user in raw mode
 * converts CTRL-C and Close Gadget to 'q', Help key to 'h'
 */
UBYTE mogetc(struct More *mo)
    {
    UBYTE ch;
    int k;

    ch = fgetc(mo->stdIn);

    /* ignore by-hand escape unless ESC[ */
    if(ch==0x1b)
	{
	if(fgetc(mo->stdIn)=='[') ch=0x9b;
	else ch='q';
	}

    /* if escape string, convert to 'h' or 'q' as appropriate */
    /* and convert upcursor to backspace, downcursor to space */ 
    if(ch==0x9b)
	{
        mo->ibuf[0] = ch;
    	k=1;
    	do
     	    {
     	    ch=fgetc(mo->stdIn);
	    if(k<(IBUFSZ-1)) mo->ibuf[k++] = ch;
	    }
    	while( ((ch < 0x40)&&(ch!=0x1b)) || ((ch > 0x7e)&&(ch!=0x9b)) );

	/* raw close gadget (part of its escape sequence) */
	if((mo->ibuf[1]=='1')&&(mo->ibuf[2]=='1')&&(mo->ibuf[3]==';')) ch='q';
	/* help key */
	else if((mo->ibuf[1]=='?')&&(mo->ibuf[2]=='~')) ch='h';
	/* pressed escape again - probably trying to quit */
	else if((ch == 0x1b)||(ch == 0x9b)) ch='q';
	/* up-cursor = page up */
	else if(mo->ibuf[1]=='A') ch=0x08;
	/* down-cursor = page down */
	else if(mo->ibuf[1]=='B') ch=' ';
	/* left-cursor = top of file */
	else if(mo->ibuf[1]=='D') ch='<';
	/* right-cursor = end of file */
	else if(mo->ibuf[1]=='C') ch='>';
	else ch='\0'; /* else ignore */
	}
    else if(ch==0x03) ch='q';
    return(ch);
    }


/* mogets
 * for user input in cooked mode
 */
int mogets(char *s, struct More *mo)
   {
   LONG c;
   int l = 0;

   while(((c = fgetc(mo->stdIn)) !='\n' )&&(c != EOF))
	{
	D2(bug("c=%lx ",c));
	*s = c;
	if(l < MAXGETS) s++, l++;
	}
   *s = NULL;
   return(c == EOF ? 0 : l);
   }


void wTitle(UBYTE *title, struct More *mo)
   {
   if((title)&&(mo->conWindow))
	SetWindowTitles(mo->conWindow,title,-1);
   }

#ifdef CINSTR

instr(a,b)      /* Returns 1 if b found in a, else 0 */
char *a, *b;
   {
   int la, lb, lb2;
   char *a2, *b2, *es;    /* end search position */

   if((la = strlen(a)) < (lb = strlen(b)))  return(0);

   es = a + la - lb + 1;
   for( ; a<es; a++)
      {
      if(*a == *b)
         {
         a2 = a+1;
         b2 = b+1;
         lb2 = lb-1;
         while((lb2--)&&(*a2++ == *b2++));
         if(lb2 < 0) return(1);
         }
      }
   return(0);
   }


strlen(s)
char *s;
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

#endif  /* CINSTR */


void strCpy(char *to, char *from)
   {
   do
      {
      *to++ = *from;
      }
   while(*from++);
   }

void toUppers(char *s)
   {
   UBYTE u;
   while(u=(UBYTE)*s)
      {
      if(((u>='a')&&(u<='z'))||((u>=0xe0)&&(u<=0xfe))) *s = u & 0xDF;
      s++;
      }
   }


int Atoi( char *s )
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


void cleanexit(char *s, LONG err, struct More *mo)
   {
   wTitle(mo->oldWTitle,mo);
   if(mo->openedFile)  Close(mo->openedFile);

   if(mo->newLock)  CurrentDir(mo->startLock);

   if(*s)
	{
	fprintf(mo->stdOut,"%s\n",s);    /* Print error */
   	if((mo->star)&&(!((s[0]=='\n')&&(s[1]=='\0'))))
      	    {
   	    setRawCon(DOSTRUE);
	    fprintf(mo->stdOut,"\n%s\n",S(MSG_MO_QUIT));
      	    while(mogetc(mo) != 'q');
      	    }
	}
   else      clearLast(mo);            /* or clear prompt */

   setRawCon(DOSFALSE);
   if(mo->RealConUnit) fprintf(mo->stdOut,"\033[1 p"); /* Turn cursor on */

   Forbid();
   if(mo->oldConTask)  mo->proc->pr_ConsoleTask = mo->oldConTask;
   if(mo->oldStdin)    mo->stdIn = mo->oldStdin;
   if(mo->oldStdout)   mo->stdOut = mo->oldStdout;
   Permit();

   if(mo->freq)	       	  FreeAslRequest(mo->freq);
   if(mo->star)           Close(mo->star);
   if(mo->intuitionBase)  CloseLibrary(mo->intuitionBase);
   if(mo->aslBase)        CloseLibrary(mo->aslBase);
   /* Locale stuff */
   if(mo->localeBase)
	{
	if(mo->catalog)	CloseCatalog(mo->catalog);
	CloseLibrary(mo->localeBase);
	}

   FreeMem(mo,sizeof(struct More));

   exit(err); /* use this exit to get to startup's exit code */
   }



/* sendpkt code - A. Finkel, P. Lindsay, C. Scheppner  CBM */

LONG setRawCon(LONG toggle) /* DOSTRUE (-1L)  or  DOSFALSE (0L) */
   {
   struct MsgPort *conid;
   struct Process *me;
   LONG myargs[8] ,nargs, res1;

   me = (struct Process *) FindTask(NULL);
   conid = (struct MsgPort *) me->pr_ConsoleTask;

   if(!conid)	return(DOSFALSE);

   myargs[0]=toggle;
   nargs = 1;
   res1 = (LONG)sendpkt(conid,ACTION_SCREEN_MODE,myargs,nargs);
   return(res1);
   }


LONG findWindow(struct More *mo)  /* inits conWindow and conUnit */
   {
   struct InfoData *id;
   struct MsgPort  *conid;
   struct Process  *me;
   LONG myargs[8] ,nargs, res1;


   me = (struct Process *) FindTask(NULL);
   conid = (struct MsgPort *) me->pr_ConsoleTask;
   if(!conid)	return(0);

   /* Alloc to insure longword alignment */
   id = (struct InfoData *)AllocMem(sizeof(struct InfoData),
                                       MEMF_PUBLIC|MEMF_CLEAR);
   if(! id) return(0);

   myargs[0]=((ULONG)id) >> 2;
   nargs = 1;
   res1 = (LONG)sendpkt(conid,ACTION_DISK_INFO,myargs,nargs);

   if(res1)
	{
   	mo->conWindow = (struct Window *)id->id_VolumeNode;
   	mo->conUnit = (struct ConUnit *)
                 ((struct IOStdReq *)id->id_InUse)->io_Unit;
	}
   FreeMem(id,sizeof(struct InfoData));
   return(res1);
   }


LONG sendpkt(pid, action, args, nargs)
struct MsgPort *pid;  /* process indentifier ... (handlers message port ) */
LONG action,          /* packet type ... (what you want handler to do )   */
     args[],          /* a pointer to a argument list */
     nargs;           /* number of arguments in list  */
   {
   struct MsgPort        *replyport;
   struct StandardPacket *packet;
 
   LONG  count, *pargs, res1;

   if(!pid) return(NULL);
   replyport = (struct MsgPort *) CreatePort((UBYTE *)NULL,0L);
   if(!replyport) return(NULL);

   packet = (struct StandardPacket *) 
      AllocMem((long)sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR);
   if(!packet) 
      {
      DeletePort(replyport);
      return(NULL);
      }

   packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
   packet->sp_Pkt.dp_Link         = &(packet->sp_Msg);
   packet->sp_Pkt.dp_Port         = replyport;
   packet->sp_Pkt.dp_Type         = action;

   /* copy the args into the packet */
   pargs = &(packet->sp_Pkt.dp_Arg1);       /* address of first argument */
   for(count=0;count < nargs;count++) 
      pargs[count]=args[count];
 
   PutMsg(pid,packet); /* send packet */

   WaitPort(replyport);
   GetMsg(replyport); 

   res1 = packet->sp_Pkt.dp_Res1;

   FreeMem(packet,(long)sizeof(struct StandardPacket));
   DeletePort(replyport); 

   return(res1);
   }



UBYTE *mogetfilename(struct More *mo)
{
D(bug("In mogetfilename\n"));

/* If not AUX, AND if Wb or own window, AND have asl.library, try requester */
if((mo->aslBase)&&(mo->conWindow)&&((mo->FromWb)||(mo->star)))
    {
    /* if we don't have one, alloc it */
    if(! mo->freq) mo->freq = AllocAslRequest(ASL_FileRequest, NULL);
    if(mo->freq)
	{
	if(AslRequestTags((APTR)mo->freq,
				ASL_Hail,S(MSG_MO_FILEREQ),
				ASL_Window, mo->conWindow,
				ASL_FuncFlags,
                                 (mo->conWindow->UserPort)
					? FILF_PATGAD : FILF_PATGAD|FILF_NEWIDCMP,
				ASLFR_Flags2, FRF_REJECTICONS,
				ASL_Pattern, DOSBase->lib_Version >= 37
					? "#?" : "~(#?.info)",
				ASL_Dir, mo->FromWb ? ":" : "",
				TAG_DONE ))
      
	    {
	    strCpy(mo->fnamebuf,mo->freq->rf_Dir);
	    if(AddPart(mo->fnamebuf,mo->freq->rf_File,NBUFSZ)) return(mo->fnamebuf);
	    }
	else return(NULL);
	}
    }

    fprintf(mo->stdOut,"\n%s ",S(MSG_MO_FILEASK));
    return(mogets(mo->fnamebuf,mo) ? mo->fnamebuf : NULL);
}


/*------------------------------------------------------------------------*/

BOOL AslRequestTags(request, tags)
APTR request;
ULONG tags;
    {
    return(AslRequest(request, (struct TagItem *)&tags));
    }

UBYTE *NULLSTR="";

UBYTE *GetString(ULONG id, APTR catalog)
    {
    struct AppString *as;
    UBYTE *s = NULLSTR;
    int k,l;

    l = sizeof(AppStrings) / sizeof(AppStrings[0]);
    as = AppStrings;

    for(k=0; k<l; k++, as++)
	{
	if(as->as_ID == id)
	    {
	    s = as->as_Str;
	    break;
	    }
	}

    if((LocaleBase)&&(catalog)&&(*s))
	{
	s = GetCatalogStr(catalog,id,s);
	}
    return(s);
    }

/*------------------------------------------------------------------------*/



