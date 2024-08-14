/*  (C) Copyright 1979 Tripos Research Group */   
/*   University of Cambridge */
/*   Computer Laboratory */

/*   Copyright 1991 Commodore-Amiga, Inc. */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <string.h>
#include <setjmp.h>

#include <internal/commands.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "execute_rev.h"

int HandleDirective(struct Global *gv);
int substitute(struct Global *gv,int ch);
BOOL substwrites(struct Global *gv,UBYTE *s, LONG pmax, UBYTE *v, LONG *lvp);
LONG getch(struct Global *gv);
extern void reportcerr();
LONG substreaditem(struct Global *gv, UBYTE *v, LONG size);
LONG substwrch(int ch, LONG pmax, UBYTE *v, LONG *lvp);
void __stdargs writef(struct Global *gv, UBYTE *fmt, long args, ...);
void printerr(struct Library *dosbase);

#define TEMPLATE    "FILE/A" CMDREV

#define OPT_FILE	0
#define OPT_COUNT	1

#define ENDSTREAMCH (-1)
#define MAXDEF	500

struct Global {
    struct Library *UB;
    struct Library *DB;
    struct RDArgs *rd;
    UBYTE subch;
    UBYTE busch;
    UBYTE defch;
    UBYTE dirch;

    BPTR instream;
    BPTR outstream;
    BPTR sys_stream;
    BPTR par_stream;

    LONG keygiven;
    LONG defstart; /* index into default area */

    UBYTE newfile[80];

    UBYTE keyword[80];

    UBYTE rdargskey[200];	/* template used by the script itself */
    LONG parameters[100];	/* storage for variables read */
    UBYTE defbuf[MAXDEF];
    jmp_buf	save;
};

/* definitions to use global variables */

#define DOSBase gv->DB
#define UtilityBase gv->UB
#define subch gv->subch
#define busch gv->busch
#define defch gv->defch
#define dirch gv->dirch

#define instream gv->instream
#define outstream gv->outstream
#define sys_stream gv->sys_stream
#define par_stream gv->par_stream

#define keygiven gv->keygiven

#define newfile gv->newfile
#define keyword gv->keyword
#define rdargskey gv->rdargskey
#define parameters gv->parameters
#define rdargs	gv->rd
#define defstart gv->defstart
#define defbuf gv->defbuf
#define save gv->save

/* character position of file character difference */
#define SWITCHPOS 12 
#define MAXKEYCHARS 21

int cmd_execute(void)
{
    struct Library *SysBase = (*((struct Library **) 4));
    struct Global *gv;
    struct Library *dosbase;
    UBYTE cfile[80];
    LONG j;
    LONG item;
    BPTR dir;
    UBYTE *workfile;
    BPTR tlock;
    BPTR newstream;
    struct Process *tid = THISPROC;
    LONG t;
    struct Window *wp;
    UBYTE *point;
    int ch;
    LONG rc=RETURN_FAIL,res2;
    int ret;

if ((dosbase = OpenLibrary(DOSLIB, DOSVER))) {
    if(!(gv=AllocMem(sizeof(struct Global),MEMF_PUBLIC|MEMF_CLEAR)))goto err;
    DOSBase=dosbase;
    if(!(UtilityBase=OpenLibrary("utility.library",0)))goto err;
    wp = tid->pr_WindowPtr;
    t = tid->pr_TaskNum;
    sys_stream = Output();
    par_stream = Input();

    outstream = NULL;
    instream = NULL;
    keygiven = FALSE;
    subch = '<';
    busch = '>';
    defch = '$';
    dirch = '.';

    memset(newfile,0,80);


    ret=setjmp(save);
    if(ret) {
	rc=ret;
	goto err;
    }
    /* get the name of the file to execute */
    /* guess who can't use readargs here, because we want to use */
    /* it later, once we've read the .key  */
    item=ReadItem(cfile,80,NULL);
    /* so we have to do another read if they did execute ? */
    if(item && !strcmp(cfile,"?")) {
	writef(gv,"%S: %C",(LONG)TEMPLATE,0);
	Flush(Input());
        item=ReadItem(cfile,80,NULL);
    }
    if(item <=0) {
	if(item) {
	    reportcerr(gv,"Incorrect file name");
	}
	goto err;
    }
    rc=RETURN_OK;

    if(!(instream =Open(cfile,MODE_OLDFILE))) {
    	/* look other places */
      	/* like the default directory */
      	dir = CurrentDir(Lock("S:",SHARED_LOCK));
      	instream =Open(cfile,MODE_OLDFILE);
      	UnLock(CurrentDir(dir));
    }
    if (!instream) {
	reportcerr (gv,"Can\'t open %S", cfile);
    }

/*
     Have a look at the first character. If it is '.' then
     we must go ahead and handle full command handling.
     Otherwise we don't need to write the output file.
     Blank lines are ignored. Write a command file if C called
     within another C file.

     If the first char is not a dot, we cannot assume that
     the file does not contain comments just because it doesn't
     have a directive as the first line !!!
     Guess we'll leave this bug in - if the guy wants comments
     he'll have to put one on line 1 too.
*/
    SelectInput (instream);
    /* eat blank lines */
    do { ch = FGetC(Input()); } while (ch == '\n');

    /* if it doesn't begin with a . directive and we're not already */
    /* executing a script */

    if (( ch != '.') &&
	(THISCLI->cli_CurrentInput == THISCLI->cli_StandardInput)) {
      UnGetC(Input(),-1);
      if (THISCLI->cli_Interactive) CheckSignal(SIGBREAKF_CTRL_D);
      THISCLI->cli_CurrentInput = instream;
      goto err;
   }
   /* construct work file name */
   /* is there a T: defined ? */
   tid->pr_WindowPtr = (struct Window *)(-1);
   tlock = Lock("T:",SHARED_LOCK);
   tid->pr_WindowPtr=wp; /* and turn them back on */

    if (tlock == 0)workfile = ":T/Command-0-Tnn";
    else {
      UnLock (tlock);
      workfile = "T:Command-00-Tnn";
    }
    strcpy(newfile,workfile);
    newfile[14]=(t/10)+'0';
    newfile[15]=(t%10)+'0';
    point = (UBYTE *)BADDR(THISCLI->cli_CommandFile);

   if (THISCLI->cli_CurrentInput != THISCLI->cli_StandardInput) {
      if (point[0] >= SWITCHPOS)newfile[11] = (point[SWITCHPOS])^1;
   }

   outstream = Open(newfile,MODE_NEWFILE);

   /* if the problem is that :T doesn't exist, try making one */
   if (outstream == 0) {
      if (IoErr() == ERROR_OBJECT_NOT_FOUND) {
         LONG lock = CreateDir(":T");
         if(lock != 0)UnLock (lock);

         outstream = Open(newfile,MODE_NEWFILE);
      }
      else if (IoErr() == ERROR_DELETE_PROTECTED) {
	 SetProtection(newfile,0);
         outstream = Open(newfile,MODE_NEWFILE);
      }
      if (outstream == 0)
         reportcerr(gv,"Can't open work file \"%S\"", newfile);
   }
   SelectOutput(outstream);
   if (THISCLI->cli_Interactive)CheckSignal(SIGBREAKF_CTRL_D);

   while (ch != ENDSTREAMCH) {
      if (ch == dirch)ch=HandleDirective(gv);
      else ch=substitute(gv,ch);
   }
   Close(Input()); /*    endread (); */

   /* now copy the rest of the current input */
   SelectInput (THISCLI->cli_CurrentInput);
   if (THISCLI->cli_CurrentInput != THISCLI->cli_StandardInput) do {
         ch = FGetC(Input());
         if (ch == ENDSTREAMCH)break;
         FPutC(Output(),ch);
      } while (TRUE);
   Close(Output()); /* endwrite (); */

   newstream = Open(newfile,MODE_OLDFILE);
   if (THISCLI->cli_CurrentInput != THISCLI->cli_StandardInput) {

      Close(Input()); /* endread (); */
      tid->pr_WindowPtr = (struct Window *)(-1);
      if(point[0]) {
	  /* if delete fails, try again */
	  if(!DeleteFile(&point[1])) {
		SetProtection(&point[1],0);
		DeleteFile(&point[1]);
	  }
      }
      tid->pr_WindowPtr=wp; /* and turn them back on */
   }
   /* attach the stream to the shell */
   THISCLI->cli_CurrentInput = newstream;

   for (j = 1 ; j <= (strlen(newfile)+1);j++) {
	point[j] = newfile[j-1];
   }
   point[0]=strlen(newfile);

err:
	if(gv) {
	    if(rc && sys_stream)SelectOutput(sys_stream);
	    res2=IoErr();
	    if (rdargs)FreeArgs(rdargs);
	    if(rc) {
		PrintFault(res2,NULL);
		SetIoErr(res2);
	    }
	    if(UtilityBase)CloseLibrary(UtilityBase);
            FreeMem(gv,sizeof(struct Global));
	}
	else printerr(dosbase);
        CloseLibrary((struct Library *)dosbase);
    }
    else {OPENFAIL;}
    return(rc);
}


int HandleDirective(gv)
struct Global *gv;
{
    LONG item;
    int c,ch;
    LONG keyn,max;

   ch = FGetC(Input());
   if ( (ch != '\n') && (ch != 9) && (ch != ' ') && (ch != ENDSTREAMCH)) {
      UnGetC(Input(),-1);
      item = ReadItem(keyword, 80, NULL);
      if(item == 1) {
	c = FindArg("KEY,K,DEFAULT,DEF,BRA,KET,DOLLAR,DOT",keyword);
      }
      else c = -1;
      if (c < 0) reportcerr(gv,"Invalid directive");

      switch (c) {
      case 0:
      case 1:
         if (keygiven) reportcerr(gv,"More than one .KEY directive");
            item = ReadItem(rdargskey, 200, NULL);
            if (item <= 0)reportcerr(gv,"Illegal KEY directive");

            SelectInput (par_stream);
            SelectOutput(sys_stream);

            memset((UBYTE *)parameters, 0, sizeof(parameters));
	    rdargs = ReadArgs(rdargskey, parameters, rdargs);

            UnGetC(Input(),-1);
            SelectOutput(outstream);
            SelectInput (instream);
            if (rdargs == 0) {
               reportcerr(gv,"Parameters unsuitable for key \"%S\"", rdargskey);
	    }
            keygiven = TRUE;
            break;
      case 2:
      case 3:
            item = ReadItem(keyword, 80 , NULL);

            if (item < 0) reportcerr(gv,"Illegal key");
            if (item == 0)break;
            if (! keygiven)reportcerr(gv,"Missing .KEY directive");
            keyn = FindArg(rdargskey,keyword);

            if ((keyn >= 0) && (parameters[keyn] == 0)) {
		max = MAXDEF-defstart;
              	item = substreaditem(gv, &defbuf[defstart], max);
		/* if = then read again */
               if (item ==  - 2 ) {
		    item=substreaditem(gv,&defbuf[defstart],max);
	       }
               if (item <= 0) {
                  if (item != 0)reportcerr(gv,"Illegal data item");
                  break;
               }
               parameters[keyn] = (LONG)&defbuf[defstart];
               defstart = (defstart+strlen(&defbuf[defstart])) + 1;
            }
            break;
	case 4:
	  /* subch */
          subch=(UBYTE)getch(gv);
	  break;
	case 5:
	  /* busch */
          busch=(UBYTE)getch(gv);
	  break;
	case 6:
	  /* defch */
          defch=(UBYTE)getch(gv);
	  break;
	case 7:
	  /* dirch */
          dirch=(UBYTE)getch(gv);
	  break;
      default:
         break;
      }
      ch = FGetC(Input());
   }

   while ( (ch != '\n') && (ch != ENDSTREAMCH)) ch = FGetC(Input());
   ch = FGetC(Input());
   return(ch);
}

int substitute(gv,ch)
struct Global *gv;
int ch;
{
   LONG writing = TRUE;
   LONG substituting = FALSE;

   LONG keyn;
   LONG l;
   LONG cliflag;
   BOOL comline = (ch != ';');

   while (  (ch != '\n') && (ch != ENDSTREAMCH)) {
      if ((ch == subch) && writing && keygiven && comline ) {
/*         if (! keygiven)reportcerr(gv,"Missing .KEY directive"); */
         ch = FGetC(Input());
         l = 0;
         cliflag = 0;
         writing = FALSE;
         substituting = TRUE;

         if (ch == defch) {
            if (FGetC(Input()) == defch) {
               cliflag = TRUE;
	       writef(gv,"%N",THISPROC->pr_TaskNum);
            }
            else UnGetC(Input(),-1);

	 }
         if (cliflag == 0) {
            while((ch != busch) && (ch != defch) && 
	     (ch != '\n') && (ch != ENDSTREAMCH)) {
               if (l >= MAXKEYCHARS)reportcerr(gv,"Key too long\n");
               keyword[l++] = ch;
               ch = FGetC(Input());
	     }
            keyword[l] = 0;
            keyn = FindArg(rdargskey,keyword);
            if ((keyn < 0) || (parameters[keyn] == 0)) {
		/* variable not found */
		/* new, output leading keyword as well */
#if 0
/* got to think about this section...it might result in a file */
/* being overwritten, if the user is unlucky */
		if(keyn <0) {
		    FPutC(Output(),subch);
		    PutStr((UBYTE *)keyword);
		    FPutC(Output(),busch);
		}
#endif
		writing = TRUE;
	    }
            else if (parameters[keyn] ==  -1 ) {
		    /* variable found, but no definition */
		    PutStr((UBYTE *)keyword);
	    }
            else {
		PutStr((UBYTE *)parameters[keyn]);
	    }
            if (ch == defch)ch = FGetC(Input());
         }
      }
      else {
         if ((ch == busch) && substituting) {
            writing = TRUE;
            substituting = FALSE;
         }
         else if (writing) {
	    FPutC(Output(),ch);
	 }
         ch = FGetC(Input());
      }
   }
   FPutC(Output(),'\n');
   ch = FGetC(Input());
   return(ch);
}

LONG getch(gv)
struct Global *gv;
{
   int ch;
   LONG item = ReadItem(keyword, 80, NULL);

   if (item == 0) {
      ch = FGetC(Input());
      UnGetC(Input(),-1);
      if ((ch == '\n') || (ch == ENDSTREAMCH)) return  (-2);
   }
   if ((item <= 0) || ((strlen(keyword) != 1))) {
	reportcerr(gv,"Invalid directive argument");
   }
   return (LONG)(keyword[0]);
}

/*
// Read an item from command line
// returns -2    "=" Symbol
//         -1    error
//          0    *N, ;, endstreamch
//          1    unquoted item
//          2    quoted item

*/

LONG substreaditem(gv, v, size)
struct Global *gv;
UBYTE *v;
LONG size;
{
   LONG p = 0;
   int ch;
   LONG pmax = size;
   LONG quoted = FALSE;
   LONG substituting = FALSE;
   LONG writing = TRUE;
   UBYTE *s;
   LONG keyn;
   LONG l;
   LONG cliflag;

   memset(v,0,size);
   do {ch = FGetC(Input()); } while (ch == ' ');
   if (ch == '\"') {
      quoted = TRUE;
      ch = FGetC(Input());
   }
   while ( (ch != '\n') && (ch != ENDSTREAMCH)) {
      if ((ch == subch) && writing && keygiven) {
/*         if (! keygiven) reportcerr(gv,"Missing .KEY directive"); */
         ch = FGetC(Input());
	 l=0;
         cliflag = FALSE;
         substituting = TRUE;

         if (ch == defch) {
            if (FGetC(Input()) == defch) {
               cliflag = TRUE;
		writef(gv,"%N",THISPROC->pr_TaskNum);
            }
            else UnGetC(Input(),-1);
	 }

         if (cliflag == FALSE) {
            while ( (ch != busch) && (ch != defch) && 
		(ch != '\n') && (ch != ENDSTREAMCH)) {
               if (l >= MAXKEYCHARS)reportcerr(gv,"Key too long\n");
               keyword[l++] = ch;
               ch = FGetC(Input());
            }
            keyword[l] = 0;
            keyn = FindArg(rdargskey,keyword);
            if ((keyn < 0) || (parameters[keyn] == 0)) {
		writing = TRUE;
	    }
            else {
               s  = (parameters[keyn] == -1) ? keyword : parameters[keyn];
               if (! substwrites(gv,s, pmax, v,  &p)) {
		    return(-1);
	       }
            }
            if (ch == defch)ch = FGetC(Input());
         }
      }
      else {
         if ((ch == busch) && substituting) {
            writing = TRUE;
            substituting = FALSE;
	 }
         else {
            if (quoted) {
               if (ch == '\"') return 2;
               if (ch == '*') {
                  ch = FGetC(Input());
                  if (ToUpper(ch) == 'E')ch = '\033';
                  else if (ToUpper(ch) == 'N')ch = '\n';
               }
            }
            else if((ch == ';') || (ch == ' ') || (ch == '='))break;

            if (writing) {
		if (!substwrch(ch, pmax, v,  &p)) {
		    return(-1);
		}
	    }
         }
         ch = FGetC(Input());
      }
   }
   UnGetC(Input(),-1);
   if (quoted) return(-1);
   if (p == 0) {
      if (ch == '=') {
         FGetC(Input());
         return(-2);
      }
      return(0);
   }
   else return(1);
}

LONG substwrch(ch, pmax, v, lvp)
int ch;
LONG pmax; 
UBYTE *v;
LONG *lvp;
{
   LONG p = (*lvp);
   if (p > pmax) {
	return FALSE;
   }
   v[p++] = ch;
   (*lvp) = p;
   return TRUE;
}

BOOL substwrites(gv, s , pmax, v, lvp)
struct Global *gv;
UBYTE *s;
LONG pmax;
UBYTE *v;
LONG *lvp;
{
   LONG i;
   for (i = 0 ; i < strlen(s) ; i++)
      if (! substwrch(s[i], pmax, v, lvp)) {
	return FALSE;
      }
   return TRUE;
}



void __stdargs writef( gv, fmt, args )
struct Global *gv; 
UBYTE *fmt;
long args;
{
   VFWritef( Output(), fmt, (LONG *)&args );
}

void reportcerr(gv, format, parm1, parm2)
struct Global *gv;
UBYTE *format;
LONG parm1, parm2;
{
LONG res2;

   res2 = IoErr();	/* save current error */
   if(outstream) {
	Close(Output()); /* was endwrite */
      if (newfile)DeleteFile (newfile);
      SelectOutput(sys_stream);
   }
   if (instream)Close(Input()); /* was endread */
   PutStr("EXECUTE: ");
   writef(gv, format, parm1, parm2);
   FPutC(Output(),'\n');
   SetIoErr(res2);
   longjmp(save,10);
}

#undef DOSBase

void printerr(DOSBase)
struct Library *DOSBase;
{
    PrintFault(IoErr(),NULL);
}
