#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/execbase.h>

#define DOS_PRIVATE_PRAGMAS

#include <internal/librarytags.h>

/* Private ROM routine... */
struct Library *TaggedOpenLibrary(LONG);
#pragma syscall TaggedOpenLibrary 32a 001

#include "global.h"

#include <devices/timer.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/notify.h>
#include <dos/filehandler.h>
#include <dos/var.h>
#include "dos/dostags.h"

#include "libhdr.h"
#include "cli_init.h"
#include <string.h>
#include <stddef.h>

#include "clib/macros.h"

#include "fault.h"

#include "shell_rev.h"

#define EXECBASE (*(struct ExecBase **)4)
#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))

#define NMAX 31
#define VARMAX 11

#ifdef USE_HOLD
#define FIBB_HOLD	7
#define FIBF_HOLD	(1<<FIBB_HOLD)
#endif

#define ERROR_REDIRECT  -118
#define ERROR_COMMAND_LONG -124

#define DOS_SIGNAL              8
#define DOS_MASK                ( 1 << DOS_SIGNAL )

/*
   Scb.Link       =0 = fh_Link
   Scb.Id         =1 = fh_Port
   Scb.Type       =2 = fh_Type
   Scb.Buf        =3 = fh_Buf
   Scb.Pos        =4 = fh_Pos
   Scb.End        =5 fh_End
   Scb.Funcs      =6 fh_Func1
   Scb.Func1      =6 
   Scb.Rdch       = Scb.Func1
   Scb.Func2      =7 fh_Func2
   Scb.Wrch       = Scb.Func2
   Scb.Func3      =8 fh_Func3
   Scb.Args       =9 fh_Args
   Scb.Arg1       =9 fh_Arg1
   Scb.Arg2       =10 fh_Arg2
   Scb.NFunc      =Scb.Args-Scb.Funcs
   Scb.Upb        =10
   Scb.WordSize   = 49      // SCB buffer size in lwords
   Scb.ByteSize   = 200     // SCB buffer byte upb

*/

int vsprintf(char *, char *, long []);
#pragma syscall RawDoFmt 20a ba9804
extern void __builtin_emit(UWORD);
static void __regargs prbuf(char c);

/* There are four basic IO streams used through this shell. */
/* 1 stdin, set by < , defaults to console (CIS) */
/* 2 stdout, set by >, defaults to console (COS) */
/* 3 commandin, set by Execute or EOF to file or console (currentin) */
/* 4 conin default console input stream (standardin) */

int CXBRK()
{
return(0);
}

void __asm startup( register __a0 struct DosPacket *parm_pkt)
{
   struct Global *gv;
   struct CGlobal *cgv=NULL;
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   UBYTE *prompt;
   UBYTE *commandname;
   UBYTE *commandfile;
   UBYTE *setname;
   LONG popts[8];
   struct CommandLineInterface *clip;
   struct CliProcList *cl;
   struct Process *process;
   LONG error;
   LONG tnum;
   BPTR (*fn)(struct DosPacket *) = NULL;
   BPTR standardout;
   struct Path *p;
   struct Segment *thiseg;
   LONG init_type = 3;		// shouldn't be required!!!! fix?
   struct FileHandle *fh; /* temporary filehandle */
   struct LocalVar *current_alias;
   struct DosPacket *save_pkt;
     BOOL freeCLI;
     BOOL pipe_expand;
     UBYTE *str;

     int ch;

     LONG item;
     LONG seg;
     LONG rc;
     BPTR module;
     LONG i,j;

char *internalname[]= {
".key",
".bra",
".ket",
"Why",
"Unsetenv",
"Unset",
"Unalias",

"Stack",
"Skip",
"Setenv",
"Set",

"Run",
"Resident",

"Quit",

"Prompt",
"Path",

"NewShell",
"NewCLI",

"Lab",

"If",

"Getenv",
"Get",

"Fault",
"Failat",

"EndSkip",
"EndShell",
"EndIf",
"EndCLI",
"Else",
"Echo",

"CD",

"Ask",
"Alias" };

LONG internallink[]= {
(link_endif),
(link_endif),
(link_endif),
(link_why),
(link_setenv),
(link_setenv),
(link_setenv),
(link_stack),
(link_skip),
(link_setenv),
(link_setenv),
(link_run),
(link_resident),
(link_quit),
(link_prompt),
(link_path),
(link_newshell),
(link_newshell),
(link_endif),
(link_if),
(link_getenv),
(link_getenv),
(link_fault),
(link_failat),
(link_endif),
(link_endcli),
(link_endif),
(link_endcli),
(link_else),
(link_echo),
(link_cd),
(link_ask),	
(link_setenv)
};

#ifdef pdebug
kprintf("\n----------------------\nStarting shell rev %ld\n",REVISION);
#endif

   DOSBase=(struct DOSLibrary *)TaggedOpenLibrary(OLTAG_DOS);

   if(parm_pkt == (struct DosPacket *) 1 ) {
	parm_pkt=WaitPkt();
   }

   /* dp_Type will be CPTR to bcpl function */
   if (parm_pkt)
   {
	/* these default to NULL and 0 respectively */
   	fn = (BPTR (*)(struct DosPacket *))(parm_pkt->dp_Type);
	init_type=(parm_pkt->dp_Res1 == 0 ? 0:2) |
		  (parm_pkt->dp_Res2 == 0 ? 0:1);
   }
   save_pkt=0;

   process = THISPROC; /* ProcessID(); */
   freeCLI = FALSE; /* Assume DOS will free it for us */   

   if(!(gv = (struct Global *)
	AllocMem(sizeof(struct Global),MEMF_PUBLIC|MEMF_CLEAR)))
	    goto early_error;

   gv->DOSBase=DOSBase;


   if(process->pr_CLI) { /* for 2.0 we have a CLI process structure */
	/* for a 2.0 style process, DOS nicely provides us with a */
	/* working CLI structure.  Since it will want to free it */
	/* set the flag that we won't have to */
	clip = BADDR(process->pr_CLI); 
   }
   else {
	/* we are not given a CLI structure, so this must be */
	/* an oldstyle BCPL type call.  We'd better make our own */

	/* allocate a CLI structure */
	cgv = (struct CGlobal *)
		AllocMem(sizeof(struct CGlobal),MEMF_PUBLIC|MEMF_CLEAR);
	if(!cgv) {
	   /* if we can't get this memory, might as well leave now */
early_error:
#ifdef pdebug
kprintf("Taking Early Error Exit\n");
#endif
	    /* give out of memory reply */
       	    ReplyPkt(parm_pkt,0,ERROR_NO_FREE_STORE); 
	    goto cleanup;
	}

	clip = &(cgv->Clip);
   	clip->cli_Prompt      = TO_BPTR(cgv->prompt);
   	clip->cli_CommandName = TO_BPTR(cgv->commandname);
   	clip->cli_CommandFile = TO_BPTR(cgv->commandfile);
   	clip->cli_SetName     = TO_BPTR(cgv->setname);

        process->pr_CLI = TO_BPTR(clip);
       	freeCLI = TRUE;		/* mark so these resources get freed */
   }
   prompt = BADDR(clip->cli_Prompt);
   commandname= BADDR(clip->cli_CommandName);
   commandfile=BADDR(clip->cli_CommandFile);
   setname=BADDR(clip->cli_SetName);
   gv->Clip = clip;

   /* for string compares */
   UtilityBase=TaggedOpenLibrary(OLTAG_UTILITY);
   gv->UtilityBase=UtilityBase;


#ifdef pdebug
kprintf("seg 0 is %lx, seg1 is %lx, seg2 is %lx, seg3 is %lx, seg4 is %lx\n",
((LONG *)BADDR(process->pr_SegList))[0],
((LONG *)BADDR(process->pr_SegList))[1],
((LONG *)BADDR(process->pr_SegList))[2],
((LONG *)BADDR(process->pr_SegList))[3],
((LONG *)BADDR(process->pr_SegList))[4]);
#endif
	    /* if 2.0 run have to change its seglist */
   	    if (!(((LONG *)BADDR(process->pr_SegList))[4])) {
	        ((LONG *)BADDR(process->pr_SegList))[4] = 
			((LONG *)BADDR(process->pr_SegList))[3];
	        ((LONG *)BADDR(process->pr_SegList))[3] = 0 ;
	    }

   if ((LONG)fn) {
        if (init_type == 0) { /* Cli init for c:run */
#ifdef pdebug
kprintf("\nRUN/System/Execute\n");
#endif
       	    fn = (BPTR (*)(struct DosPacket *))CliInitRun(parm_pkt);
#ifdef pdebug
kprintf("returned from CliInitRun\n");
#endif
	    if( ((LONG)fn<0) &&(LONG)fn&8 ) {/* check for async system call */
#ifdef pdebug
kprintf("async reply\n");
#endif
		 /* so reply now */
    		ReplyPkt(parm_pkt,parm_pkt->dp_Res1,parm_pkt->dp_Res2);
	    }
	}
	else if (init_type == 2) { /* Cli init for c:NewCli/c:NewShell */
#ifdef pdebug
kprintf("\nNewshell\n");
#endif
	    /* newshell case */
	    fn = (BPTR (*)(struct DosPacket *))CliInitNewcli(parm_pkt);
	}
	else goto cleanup; /* we need to clean up , maybe do an alert*/
   }
   else {
	/* we are booting! (always returns 0) */
	fn = (BPTR (*)(struct DosPacket *))CliInit(parm_pkt);
#ifdef pdebug
	kprintf("BOOT, fn returned is %lx\n",fn);
#endif
   }


	if ((LONG)fn > 0) {
/*
	The only functions returned here are qpkt (for types 00 and 10) for
	an argument of parm_pkt; and deletetask for an argument of FindTask(0).
	We'll key off the value of result2.
*/
	    if ((LONG)IoErr() != (LONG)(THISPROC) ) {
		if(init_type == 2) {
     	    	    ReplyPkt(parm_pkt,parm_pkt->dp_Res1,parm_pkt->dp_Res2);
		}
		else {
#ifdef pdebug
	    	    kprintf("delay doing the Reply\n");
#endif
	
		    /* mark this reply to be performed after we've tried to */
		    /* loadseg.  This will prevent grinding when running */
		    /* commands from scripts. */
		    save_pkt=parm_pkt;
	    	}
	    }
	    else {
#ifdef pdebug
		kprintf("bailing out!\n");
#endif
		goto cleanup;
	    }
	}

   if(thiseg=searchsegs(DOSBase,"SHELL",NULL))thiseg->seg_UC =  -1;

   process->pr_HomeDir = 0; /* initialize to sys: */
   tnum =process->pr_TaskNum;

   standardout = clip->cli_StandardOutput;

   /* add builtin commands to the resident list */
   error=0;

   Forbid();	/* add our internal commands */
   thiseg=searchsegs(DOSBase,"alias",NULL);

   while(thiseg) {
	/* set redirection special flag for set(s) commands */

	if(thiseg->seg_Seg == TO_BPTR(link_setenv)) {
	    error=TRUE; /* commands are already installed */
	    break;
	}
	thiseg=searchsegs(DOSBase,"alias",thiseg);
   }
   if(!error) {
	for(i=0; i<33; i++)
	   AddSegment(internalname[i],TO_BPTR(internallink[i]),CMD_BUILTIN);
   }

   /* the set command(s) and the run command need special treatment */
   /* for redirection purposes */
   if(thiseg=searchsegs(DOSBase,"run",0))
	gv->run_module=((struct Segment *)thiseg)->seg_Seg;
   if(thiseg=searchsegs(DOSBase,"set",0))
	gv->set_module=((struct Segment *)thiseg)->seg_Seg;
   if(thiseg=searchsegs(DOSBase,"pipe",0))
	gv->pipe_module=((struct Segment *)thiseg)->seg_Seg;

#if 0
   /* the above code compiles smaller ! */
   for(i=0, point= &gv->run_module; i < 3; i++ ) {
   if(thiseg=searchsegs(DOSBase,specialname[i],0))
	*point++ = ((struct Segment *)thiseg)->seg_Seg;
   }	
#endif

   /* ok, we're set up, and can go public now */

   Permit();
   /* set the local variables */
   vsprintf(gv->buffer,"%ld",&tnum);
   SetVar("process",gv->buffer,VARMAX,LV_VAR|GVF_LOCAL_ONLY);
   rc = 0;
   error=0;
   setv(gv,&rc); /* initialize RC and Result2 vatriables */

   do {
      do {								
    	setname[setname[0]+1]=0; /* null terminate the bstrings */
	prompt[prompt[0]+1]=0;
	commandname[0]=0; /* ready for next command */

         clip->cli_Interactive =  (!clip->cli_Background  && 
	    (clip->cli_CurrentInput == clip->cli_StandardInput) &&
	     !((((LONG)fn) & 0x80000004) == 0x80000004)) ?
		DOSTRUE : FALSE;

         SelectInput(clip->cli_CurrentInput);
         SelectOutput(standardout); /* set process outout() */

         if (clip->cli_Interactive) {
	   if(!clip->cli_Background) {
            FPutC(Output(),'O' - '@'); /* go into normal mode */
	    j=0;
	    strcpy(gv->alias,prompt+1);
            for (i = 1 ; prompt[i]; i++)if (prompt[i] == '%') {
		switch(ToUpper(prompt[i+1])) {
		case 'S':
			popts[j++]=(LONG)setname+1;
			break;
		case 'N':
			popts[j++]=tnum;
			break;
		case 'R':
			popts[j++]=clip->cli_ReturnCode;
			gv->alias[i]='n';
			break;
		case '%':
			i++;  // skip the second %
			break;
		default:
			gv->alias[i]=' ';
			break;
		case '\0':
			break;	// must not go to default! (trailing %)
	        }
	    }
	    popts[j]=tnum;
	    expand(gv,gv->alias,MAXALIAS);/* take care of variables in prompt */
	    ticks(gv,gv->alias,MAXALIAS); /* handle a command in prompt if present */
	    /* we don't care if ticks() failed here (the user will see it) */

            VFWritef(Output(), gv->alias, (LONG *)&popts );
	    Flush(Output());
	   }
         }
         else {
             if (testflags(SIGBREAKF_CTRL_D)) {
               error = DOS_TRUE;
	       PrintFault(304,"SHELL"); /* "**BREAK - SHELL\n" */
            }
            if (error) break;
         }
         error = FALSE;

	 /* check for local 'command' variables */
	 gv->echo = 0; /* assume they want quiet */
	 if(GetVar("echo",gv->buffer,NMAX,LV_VAR|GVF_LOCAL_ONLY) >0 ) {
	     if(!Strnicmp("on",gv->buffer,2)) gv->echo = TRUE;
	 }
#ifdef OLD_REDIRECT
	 gv->old_red=0; /* assume new style redirection */
	 if(GetVar("oldredirect",gv->buffer,NMAX,LV_VAR|GVF_LOCAL_ONLY) >0) {
	     if(!Strnicmp("on",gv->buffer,2)) gv->old_red = TRUE;
	 }
#endif
         item = ReadItem(commandname+1, NAMEMAX,FALSE);
 	 commandname[0] = strlen(commandname+1); /* convert to bstr */
	 commandname[commandname[0]+1]=0; /* and null terminate */
         ch = 0;
         if (item) {
            error = DOS_TRUE;
            if (item > 0) {
		rc = 0;	/* preset to can't find command */
		i = -1;
		seg = 0;
		gv->Res2 = 0;
		pipe_expand=FALSE;
		gv->cpos = 0;
		gv->alias[0]=0;
		current_alias = 0;

		/* if the item was unquoted */
		if(item == ITEM_UNQUOTED) {
		    // returns -1 if variable not found
		    i=GetVar(commandname+1,gv->cbuffer,255,LV_ALIAS);
		    /* get pointer to alias  for recursion prevention */
		    if (i >= 0)		// only if we found it...
			    current_alias = FindVar(commandname+1,LV_ALIAS);
		}
		/* or if its not an alias, try variable expansion */
		if(i<0)strcpy(gv->cbuffer,commandname+1);

		// gv->cbuffer has either command or alias in it
		/* trust the alias is null terminated */
		expand(gv,gv->cbuffer,MAXCOMMAND); /* expand variables */
		/* if its not an alias, do backticks before expansion */
		if(!current_alias) {
		    rc = ticks(gv,gv->cbuffer,MAXCOMMAND); /* handle backticks */
		}
		if(item == ITEM_UNQUOTED) { /* if the item was unquoted */
		    for(str=gv->cbuffer; *str; str++)
			if(*str == ' ') {
			    str++; /* skip the space */
		            strcpy(gv->alias,str); /* copy string into buffer */
		 	  			   /* for use later (arguments)*/
			    /* add a space to seperate from first argument */
			    strcat(gv->alias," ");
			    gv->cpos = strlen(gv->alias); /* and set pointer */
			    break;
			}
		}

		/* if the string was quoted, the entire str was command name */
		// else move until space.  FIX! should be until isspace()!
		for(i=1, str=gv->cbuffer;
		    i < NAMEMAX && ((item == ITEM_QUOTED) || (*str != ' '));
		     )
		{
		    commandname[i++]= *str++;
// FIX! should error on commandname too long!
		}
  	        commandname[i]=0;	/* null terminate */

		if(current_alias) {
		    /* to prevent recursion */
		    rc = ticks(gv,commandname+1,NAMEMAX); /* handle backticks */
		    current_alias->lv_Node.ln_Type |= LVF_IGNORE;
		}
	   	commandname[0] = strlen(commandname+1); /* set bstr length */

		module = clip->cli_Module = 0;
		if (rc >= 0)  /* any ticks() call succeeded */
		{
			rc = -9999;	/* preset to can't find command */
	                module=findLoad(gv,commandname+1,&seg,save_pkt,0);
			save_pkt=0;
		} else
			rc = -rc;	// ticks() returned negative of rc
		// if ticks() failed, we'll fall into the error-handling code
		// commandname now has the string: `<command>`

                testflags(SIGBREAKF_CTRL_C);/* clear the control C signal bit */
                if (module <  -1 ) {    /* a script needs an execute */
				 	/* of one sort or another */	
					/* and -1 is an error condition */
		     strins(gv->alias," ");
		     strins(gv->alias,commandname+1);
		     /* switch commandname so startups don't get unhappy */
		     /* cpos is going to always get reset at end */

		     /* if we have a rexx script, we have to treat both */
		     /* quoting conventions and commandline differently */
		     if(module == (-4)) {
			strcpy(commandname+1,"RX");
			/* if we have a homedir */
			/* which indicates we found it via path */
			if(process->pr_HomeDir > 0) {
			 if(NameFromLock(process->pr_HomeDir,gv->cbuffer,108)){
			    if(AddPart(gv->cbuffer,gv->alias,160)) {
/*				strcat(gv->cbuffer," "); */
			        strcpy(gv->alias,gv->cbuffer);
			    }
			 }
			}
		     }
		     else { /* RX has a special problem with "" so skip */
		         if(module == (-5))strcpy(commandname+1,"CD");
			 else if (module == (-6)) {
			    strcpy(commandname+1,"PIPE");
			    pipe_expand=TRUE; /* avoid redirection problems */
			 }
		         else {
			    strcpy(commandname+1,"EXECUTE");
			    /* if we have a homedir */
			    /* which indicates we found it via path */
			    if(process->pr_HomeDir > 0) {
			        strins(gv->alias,"PROGDIR:");
			    }
			 }
			 /* check if we have to double * */
			 for(i=0; i<strlen(gv->alias); i++) {
			     ch=gv->alias[i];
			     /* if the command wasn't quoted, */
			     /* stop at 1st space */
			     if((item != 2)&&(ch==' '))break;
			     if((ch == '*') || (ch == '$') || (ch == '\"')) {
				strins(&gv->alias[i++],"*");
			     }
			 }
			 if(item == 2) {
		     	     strins(gv->alias,"\""); /* handle spaces in arg */
		             strcat(gv->alias,"\"");
			 }
		     }
		     /* this may end up adding two spaces, if the script */
		     /* is part of an alias */
		     
		     if(gv->alias[(gv->cpos=strlen(gv->alias))-1]!=' ') {
			strcat(gv->alias," ");
			gv->cpos++;
		     }
		     commandname[0]=strlen(commandname+1); /* fix b length */

		     /* at this point, commandname contains the command */
		     /* to execute, ie RX, EXECUTE, CD, PIPE */
		     /* and the rest of any alias string is in gv->alias */

		     /* now find the actual command to execute */
                     module = findLoad(gv,commandname+1, &seg,0,TRUE);
		  }
		  if(gv->echo) {
			writef(DOSBase,"%s ",commandname+1);
		  	Flush(Output());
		  }

		  if (module > 2) {
		    clip->cli_Module = module;

		    // kludge to keep 1.3 SetClock from crashing A4000's
		    // 1.3 setclock starts with 48e78080, no later ones do.
		    // Must check for setclock and c:setclock.
#define SAME 0
		    if (((Stricmp(commandname+1,"setclock") != SAME) &&
			 (Stricmp(commandname+1,"c:setclock") != SAME)) ||
			(*(((LONG *) BADDR(module)) + 1) != 0x48e78080))
		    {
                    	if (module)rc = run(gv,module,pipe_expand);
		    } else {
			rc = RETURN_OK;
		    }
		    /* DOS promised unlocking a NULL is harmless */
		    UnLock(process->pr_HomeDir);
		    process->pr_HomeDir=0;
		  }
		 /* undo any redirection the command did */
                  SelectOutput(standardout);
#if 0
		  /* guess what ?  RunCommand returns -1 for can't */
		  /* allocate stack.  We have to use this kludge */
		  /* to attempt to tell this from programs that return -1 */
		  if(rc == (-1)	&& (gv->Res2 == ERROR_NO_FREE_STORE)) {
			rc = -9999;	/* its an internal error */
		  }				
#endif		  
 	          if ((rc == -9999) || (rc == -1)) { /* something went wrong before */
					/* we ran the command */
		      rc = RETURN_ERROR; /* failure */
			/* probably want to print out SHELL */
                      if(!gv->Res2 || (gv->Res2 == ERROR_OBJECT_NOT_FOUND) ||
			 (gv->Res2 == ERROR_DEVICE_NOT_MOUNTED)) {
			   PrintFault(STR_UNKNOWN_COMMAND,commandname+1);/* unknown command */
		      }
	              else { 
			  /* print actual error */
			  PrintFault(gv->Res2,commandname+1);
		      }
		  }
                  gv->Res2 = ((rc == 0) || (gv->Res2 < 0)) ? 0 : gv->Res2;
		  
                  if ((rc>=0) && (rc < clip->cli_FailLevel))error = FALSE;


		  /* if we have an error, and this is a script */
		  /* tell what happened */
// FIX!!!! make this string localized!!!!!!
                  if (error && !(clip->cli_Interactive))
		  {
// since it's commented out in the include files (private)...
		     extern BYTE *DosGetString(LONG num);
#pragma libcall DOSBase DosGetString 3d2 101
		     LONG args[2];

		     args[0] = (LONG) commandname+1;
		     args[1] = rc;
		     VPrintf(DosGetString(STR_FAILED_RETURNCODE),args);
		  }
                  clip->cli_ReturnCode = rc;


		  /* update current values of result variables */
		  setv(gv,&rc);
		  if(current_alias) { /* reset alias so it can be used again */
			current_alias->lv_Node.ln_Type &= ~LVF_IGNORE;
		  }

                  SelectInput (clip->cli_CurrentInput);
		  /* eat anything left over on the line */
                  ch = UnGetC(Input(),-1) ? 0 : '\n';
		  if(!seg) {
                      if(clip->cli_Module)UnLoadSeg(clip->cli_Module);
		  }
		  else {	/* resident program */
		      Forbid();
		      if (((struct Segment *)seg)->seg_UC > 0)
			((struct Segment *)seg)->seg_UC--;
		      Permit();
		  }
                  clip->cli_Module = 0;
                  clip->cli_Result2 = gv->Res2;
            }
            else {
		PrintFault(STR_ERROR_COMMAND,0); /* Error in command name */ 
	    }
         }

	 if(save_pkt) {
     	    ReplyPkt(save_pkt,save_pkt->dp_Res1,save_pkt->dp_Res2);
	    save_pkt=0;
	 }

	 /* eat any extra characters */
	 while ((ch != '\n') && (ch != END_STREAM_CH))ch = FGetC(Input());
 	/* system exit special case taken only if they have played*/
	/* around with the input stream */
         if (((((LONG) fn)& 0x80000004) == 0x80000004) && 
	     (clip->cli_CurrentInput == clip->cli_StandardInput))break;
      } while (ch != END_STREAM_CH);

      /* cleanup code */

#ifdef pdebug
kprintf("in cleanup,fn is %lx, clip is %lx\n",(LONG)fn,(LONG)clip);
#endif
      	if (clip->cli_CurrentInput == clip->cli_StandardInput) {
           /* endcli only if its an interactive CLI */
 	   if((ch == END_STREAM_CH)&&(!clip->cli_Background)) {
		do_end(DOSBase);
	   }
	   /* can we avoid leaving a temp file around ? */
           if(commandfile[0]) {
	     	DeleteFile(commandfile+1);
                commandfile[0] = 0;
	   }
	   break;
      	}
	/* if call is coming via the System() call, stop reading and */
        /* just return */
        else if ((((LONG) fn)& 0x80000004) == 0x80000004) {
            Close(clip->cli_CurrentInput);
	    break;
        }

      	else {
#ifdef pdebug
	kprintf("commandfile, closing input\n");
#endif
         Close(Input());
         if(commandfile[0]) { 
	     DeleteFile(commandfile+1);
             commandfile[0] = 0;
	 }
         clip->cli_CurrentInput = clip->cli_StandardInput;
         clip->cli_FailLevel = CLI_INITIAL_FAIL_LEVEL;
      }
   } while (TRUE);


   if ((LONG)fn < 0) { /* the function pointer is a flag word */
      fh = (struct FileHandle *)BADDR(Output());
	/* note: pos>0 test and setting it to 0 is probably not needed now */
      if ((fh) && (fh->fh_Pos > 0)) {
   	 Flush(Output());
         fh->fh_Pos = 0;
      }

      if (((LONG)fn & 2) == 0) {
#ifdef pdebug
kprintf("close read %lx (%lx)\n",(LONG)Input(),clip->cli_StandardInput);
#endif
	Close(clip->cli_StandardInput);
      }
      if (((LONG)fn & 1) == 1) {
#ifdef pdebug
kprintf("close write %lx (%lx)\n",(LONG)Output(),clip->cli_StandardOutput);
#endif
	 Close(clip->cli_StandardOutput); /* close for writing */
      }
      if ( ((LONG)fn >= 0) || !((LONG)fn&8)) {
#ifdef pdebug
kprintf("replying message, fn is %lx\n",(LONG)fn);
#endif
	ReplyPkt(parm_pkt,clip->cli_ReturnCode,clip->cli_Result2);
      }
   }
   else {
#ifdef pdebug
kprintf("Close read %lx, close write %lx\n",(LONG)Input(),(LONG)Output());
#endif
      if(clip->cli_StandardOutput) {
           Flush(clip->cli_StandardOutput);
           Close(clip->cli_StandardOutput);
      }
      if(clip->cli_StandardInput)Close(clip->cli_StandardInput);
   }
/* we only want to do this if we were started in the 1.3 manner */

cleanup:
#ifdef pdebug
kprintf("now in cleanup\n");
#endif

   if(freeCLI) {

#ifdef pdebug
kprintf("freeing the cli structure\n");
#endif
	/* just in case we got down here due to initial failure */
       tnum =process->pr_TaskNum;
       p = (struct Path *)BADDR(clip->cli_CommandDir);
       while (p) {
          struct Path *next;

      	  next = (struct Path *)BADDR(p->path_Next);
          UnLock(p->path_Lock);
          FreeVec(p);
          p = next;
         }
       for(cl=(struct CliProcList *)((struct RootNode *)
	   DOSBase->dl_Root)->rn_CliList.mlh_Head; cl;
           cl = (struct CliProcList *) cl->cpl_Node.mln_Succ) {
	   	if (tnum <= cl->cpl_First + ((LONG) cl->cpl_Array[0]) - 1) {
		    cl->cpl_Array[tnum - cl->cpl_First + 1] = NULL;
		    break;
	        }
       	   }

       process->pr_Flags =0;
       process->pr_CLI = 0;
   }

#ifdef pdebug
kprintf("currentdir is %lx\n",process->pr_CurrentDir);
#endif

 UnLock(process->pr_CurrentDir);
 process->pr_CurrentDir=NULL;
 if(UtilityBase)CloseLibrary(UtilityBase); /* close this before freeing the ram */

 if(cgv)FreeMem(cgv,sizeof(struct CGlobal));
 if(gv)FreeMem(gv,sizeof(struct Global));

 CloseLibrary((struct Library *)DOSBase);

#ifdef pdebug
kprintf("exit\n");
#endif

 return;
}


/* Find and Load a Command */
/* returns	-1 for error */
/* returns       0 for file not found
/* returns	 1 for file found, execute not set
/* returns	-3 AmigaDOS script
/* returns	-4 for Arexx script
/* returns	-5 for directory to CD into
/* returns	-6 for PIPE */
/* returns	 segment for success
/* seg is set to indicate resident , builtin, or normal commands
/* */

BPTR findLoad(gv,commandname, segment, parm_pkt,status)
struct Global *gv;
UBYTE  *commandname;
LONG *segment;
struct DosPacket *parm_pkt;
BOOL status;
{
   BPTR module = 0;
   LONG firsttime = 2;
   struct Path *p;
   struct DosLibrary *DOSBase=gv->DOSBase;
   struct Library *UtilityBase=gv->UtilityBase;
   struct CommandLineInterface *clip= gv->Clip;
   BOOL dirfound=FALSE;
   int i;
   struct Process *proc = THISPROC;

   if(i=pipecheck(gv,status,"_pchar"))return(i);
   if(i=pipecheck(gv,status,"_mchar"))return(i);

   /* try the resident segment list first */
   /* note: the value of segment is used to flag a resident */
   /* command that isn't unloaded */
 if(*segment = (LONG)useseg(DOSBase,commandname)) {
	module=((struct Segment *)*segment)->seg_Seg;
 }

 /* try the disk */
 else {
   while((--firsttime >=0) && (module==0)) {
   /* try in current directory first (actually, this is trying exactly */
   /*    where they typed */
   if (!module)module = load(gv,commandname,0,segment, (!firsttime));
   if(module==2) {
	dirfound=TRUE;
	module=0; /* keep looking just in case */
   }
   else if (module < -1) {
	/* we found a script via direct path means */
	/* unlocking a null is safe */
	UnLock(proc->pr_HomeDir);
	proc->pr_HomeDir = 0;
   }

   /* search the path */
   /* NOTE: don't want to do this if they specified : or / */

   if(FilePart(commandname) == commandname) {
     p = (struct Path *)BADDR(clip->cli_CommandDir);
     while (!module && p) {
           module = load(gv,commandname, p->path_Lock, segment, FALSE);
	   if(module==2) module=0; /* keep looking */
	   p = (struct Path *)BADDR(p->path_Next);
     }
     /* try c:  */
     if (!module) {
	struct DevProc *dp = NULL;
	struct MsgPort *ftask = GetFileSysTask();
	    do {
	      dp=GetDeviceProc("c:",dp); /* this should bring up a requester */
	      if (dp) {
		 SetFileSysTask(dp->dvp_Port);
		 module=load(gv,commandname,dp->dvp_Lock,segment,(firsttime>0));
		 if(module==2) module=0; /* keep looking */
	      }
	    } while (!module && dp && (dp->dvp_Flags & DVPF_ASSIGN) && 
		IoErr()==ERROR_OBJECT_NOT_FOUND); /* repeat if multi-assign */
	SetFileSysTask(ftask);
	if (dp)FreeDeviceProc(dp);
     }
     if ( module || (IoErr() == 218))break;
   }
   if(parm_pkt) {
#ifdef pdebug
	kprintf("doing the run delayed reply!\n");
#endif
	ReplyPkt(parm_pkt,parm_pkt->dp_Res1,parm_pkt->dp_Res2);
	parm_pkt=0;
    }
   /* special code to handle the case of the new builtins */
   if((!module) && !Strnicmp("c:",commandname,2) ) { /* call it in c: ? */
   if(*segment = (LONG)useseg(DOSBase,FilePart(commandname)))
	return(((struct Segment *)*segment)->seg_Seg);
   }
  }
 }
if(parm_pkt) {	/* handle the delayed run reply case */
#ifdef pdebug
    kprintf("doing the run delayed reply!\n");
#endif
    ReplyPkt(parm_pkt,parm_pkt->dp_Res1,parm_pkt->dp_Res2);
}
if((!module) && dirfound)return(-5); /* they want an implicit cd */
return(module);
}

/* LOAD function */
/* takes filename, directory lock, pointer to return code, quietflag */
/* returns	0 for file not found
/* returns     -1 for problems during load
/* returns     -2 or -3 for script file
/* returns      1 for file found, execute not set
/* returns      2 for directory found
/* returns      segment for success
/*
/* NOTE: a 0 for directory input indicates current directory
/* */

BPTR load(gv,file, ldir, segment, qflag)
struct Global *gv;
UBYTE *file;
BPTR ldir;
LONG *segment;
LONG qflag;
{
   BPTR seg=0;
   LONG res;
   APTR wp;
   BPTR olddir;
   struct Process *tid;
   BPTR dir;
   struct DosLibrary *DOSBase=gv->DOSBase;

   tid = THISPROC;

   wp= tid->pr_WindowPtr;
   if (!qflag)tid->pr_WindowPtr = (APTR)(-1); /* load quietly if requested */

   if(!ldir)dir=tid->pr_CurrentDir;
   else dir=ldir;
   olddir=CurrentDir(dir);
   res = testscript(gv,file,qflag);
   if ( (res == (-1)) || (res == (-2))) {
	seg = LoadSeg(file);
	if(seg) {
#ifdef USE_HOLD
	    /* test if this is a hold and execute module */
   	    if(res == (-2)) {
	    	res = -1; /* we'll return same as standard excutable */
	    	/* Is this name already on the resident list ? */
	    	if(searchsegs(DOSBase,FilePart(file),0) == 0) { /* no... */
	            /* lets add it to the resident list as well */
		    AddSegment(FilePart(file),seg,1);
 		    *segment = (LONG)useseg(DOSBase,FilePart(file)); /* set the flag */
		}
	    }
#endif
	}
	else {
	    gv->Res2=IoErr(); /* loadseg failed */
	    if(gv->Res2 == ERROR_OBJECT_WRONG_TYPE)
		gv->Res2=ERROR_FILE_NOT_OBJECT;  
	}
   }
   else if((res <= -3)) { /* -3 and -4 are scripts */
	tid->pr_HomeDir = DupLock(dir); /* set homedir to the script dir */
   }
   else  {
      /* this will preserve a file found with execute not set */
      /* error for later */
      if(!gv->Res2)gv->Res2 = IoErr();
   }
   if(seg) { /* we loaded the file */
	/* if we have a homedir already, this must be us loading */
        /* the proper script command, execute or RX */
	/* so don't set homedir */
	if(!tid->pr_HomeDir) {
	    dir=Lock(file,SHARED_LOCK);
	    if(dir) {
	        tid->pr_HomeDir = ParentDir(dir);
	        UnLock(dir);
	    }
	}
  	res=seg;
   }
   /* seg of 0 and res -1 means we tried to load */
   else if (res == -1) {
	/* set a reasonable error return */
	/* loadseg says nothing, make something up */
	if(!gv->Res2)gv->Res2=ERROR_FILE_NOT_OBJECT;  
   }

   CurrentDir(olddir);
   if (!qflag) tid->pr_WindowPtr = wp;
   if(res == 1)	{
	gv->Res2=ERROR_FILE_NOT_OBJECT;
	res=0; /* plain file, so we keep looking */ 
   }
   SetIoErr(gv->Res2); /* just in case */
   return(res);
}

/* test type of file
/* return 0 for file not found
/*        2 for directory (ie keep looking)
/*        1 for plain file
/*	 -1 executable
/*	 -2 for hold & execute
/*       -3 for script file
/*       -4 for Arexx script


/* */

int testscript (gv,file,qflag)
struct Global *gv;
UBYTE *file, qflag;
{
   struct DosLibrary *DOSBase=gv->DOSBase;
   struct Library *UtilityBase=gv->UtilityBase; 
   struct FileInfoBlock *exinfo;
   LONG res=0;
   BPTR lock;
   BPTR fh;
   UBYTE buffer[10];

   exinfo = &(gv->ExInfo);
   lock=Lock(file,ACCESS_READ);
   if (lock) {
      res = Examine(lock,exinfo);
      if (res) {
	if (exinfo->fib_DirEntryType >= 0) res = 2;	/* directory */
	else if (exinfo->fib_Protection & FIBF_SCRIPT) {
		res =  - 3;	/* assume AmigaDOS script */
		fh=Open(file,MODE_OLDFILE);
		if(fh) {
		    Read(fh,buffer,2);    
		    if(!Strnicmp(buffer,"/*",2))res = -4; /* no, its arexx */
		    Close(fh);
		}
	}
        else if ( (exinfo->fib_Protection & FIBF_EXECUTE) == 0) {
#ifdef USE_HOLD
	    /* check if this execute file is a hold and execute type */
	    if( (exinfo->fib_Protection & FIBF_PURE) &&
		(exinfo->fib_Protection & FIBF_HOLD)) res = -2;
	    else
#endif
		 res = -1; /* its a plain executable */
	}
        else res =  1;	/* plain file */
      }
      UnLock(lock);
   }
   else { /* if not already another error */
	if(gv->Res2 !=ERROR_FILE_NOT_OBJECT)gv->Res2 = IoErr();
   }
   return(res);
}

/* run a command on the current process */

LONG run(gv,module,pipe_expand)
struct Global *gv;
BPTR module; 
BOOL pipe_expand;
{
   BPTR termout;
   BPTR termin;
   BPTR stdin =  (BPTR)(-1);
   BPTR stdout =  (BPTR)(-1);

   LONG rc;
   int c;
   int ch;
   int lastch = ' ';	/* initialize to space so redirection is allowed */
/* State indicates parsing state. <0 -> in quotes, >0  out of quotes,
   =0 -> in comment */
   int state = 1;

   UBYTE stdvec[NAMEMAX+1];

   struct DosLibrary *DOSBase=gv->DOSBase;
   struct CommandLineInterface *clip= gv->Clip;
   struct Process *process= THISPROC; /* ProcessID(); */
#if 0
   struct FileHandle *fh;
   BPTR orig_buf;
   LONG orig_pos,orig_end;
#endif
   APTR ctask;
   BOOL past_begin=pipe_expand; /* flag to indicate whether the line */
				/* processing has started used to handle */
				/* the run redirect case.  If a pipe */
				/* expansion was performed, don't allow */
				/* new redirection */
   LONG pos=0;	
   UBYTE *private_buffer = gv->cbuffer;
   termin = clip->cli_StandardInput;
   termout = Output(); /* initialize to pr_COS */

   gv->count =0;	/* start processing the line at beginning */
   gv->fill = 0;

   /* take care of leading spaces and tabs */
   do {
      ch = getchar(gv,0);
      if((ch != ' ') && (ch != '\t'))break;
   } while (DOS_TRUE);

/*
   We now read the rest of the line and place it into a buffer.
   This is passed to runcommand to be made available to the calling
   program. We also patch up the scb so that programs calling the
   character interface read the command tail as part of their input.
   Note that the stdin stream is either a new input file; or it
   is from the terminal. In either case there are no characters
   buffered within the stream buffer at this time. Programs running
   under the CLI must not close stdin so we can safely replace the
   SCB buffer with a private one, filled with the command line.
   If stdin is new then the buffer pointer will be overwritten when
   the command line is exhausted. If stdin is the terminal then
   reads will use the replacement buffer instead.
   State indicates parsing state. <0 -> in quotes, >0  out of quotes,
   =0 -> in comment

*/

	while (TRUE) {
	    c=ch;
	    switch ((int)ch) {
		case ';':		/* comment character */
		    if (state > 0) state = 0;
			goto nextl;
		case '"':		/* quoted values */
			if(lastch != '*')state = -state;
			goto nextl;
		case '+':		/* maybe a trailing plus */
			if (getchar(gv,lastch) == '\n')c = '\n';
			else ungetchar(gv);
			goto nextl;

		case '>':
#ifdef OLD_REDIRECT
		    if((state <= 0) || (stdout > 0) || 
			((lastch != ' ') && !gv->old_red))goto nextl;
		    if(past_begin && ((module==gv->run_module) ||
			(module==gv->pipe_module)||(module==gv->set_module)||
			(gv->old_red)))goto nextl;
#else
		    if((state <= 0) || (stdout > 0) || (lastch != ' '))
			goto nextl;
		    if(past_begin && ((module==gv->run_module) ||
			(module==gv->pipe_module)||(module==gv->set_module)))
			goto nextl;
#endif
		    ch = getchar(gv,0);
 	  	    if (ch != '>') ungetchar(gv);
 		    if(readitem(gv,stdvec) <= 0 ) {
			ch=' ';
			goto nextl;
		    }
       		    if (ch == '>') {
	                stdout = openf(gv,stdvec,ACTION_FINDUPDATE);
		        if(stdout)Seek(stdout,0,1);
		    }
	            if(stdout <=0 ) { /* still don't have a redirect */
	                stdout = openf(gv,stdvec,ACTION_FINDOUTPUT);
		    }
	            /* need error check on open here */
		    if (stdout==0) {
			/* unable to open redirection file */
			return(Err(gv,ERROR_REDIRECT,stdin,stdout)); 
		    }
		    ch=' ';
		    goto nextch;
		case '<':
#ifdef OLD_REDIRECT
		    if( (state <= 0) || (stdin > 0) || 
			((lastch != ' ') && !gv->old_red))goto nextl;
		    if(past_begin && ((module==gv->run_module) ||
			(module==gv->pipe_module)||(module==gv->set_module)||
			(gv->old_red)))goto nextl;
#else
		    if( (state <= 0) || (stdin > 0) || (lastch != ' '))
			goto nextl;
		    if(past_begin && ((module==gv->run_module) ||
			(module==gv->pipe_module)||(module==gv->set_module)))
			goto nextl;
#endif
		    ch = getchar(gv,0);
		    if (ch != '>')ungetchar(gv);
	   	    if(readitem(gv,stdvec) <= 0 ) {
			ch=' ';
			goto nextl;
		    }
	  	    if(ch == '>') {
			stdin=openf(gv,stdvec,ACTION_FINDINPUT);
			if(stdin) {
			    ctask=process->pr_ConsoleTask;
			    process->pr_ConsoleTask=
			      (APTR)((struct FileHandle *)BADDR(stdin))->fh_Type;
			    stdout=Open("*",ACTION_FINDOUTPUT);
			    if (!stdout)
				SetIoErr(ERROR_REDIRECT);
			    process->pr_ConsoleTask=ctask;
			}
		    }
	  	    else stdin = openf(gv,stdvec,ACTION_FINDINPUT);
		    /* return unable to open redirection file */
		    if(stdin==0)return(Err(gv,ERROR_REDIRECT,stdin,stdout)); 
		    ch=' ';
		    goto nextch;

		default:
nextl:
		if (!state)break;
		if((ch != ' ')&&(ch != '\t'))past_begin=TRUE;

		case '\n':
		case ENDSTREAMCH:
		    private_buffer[pos++] = c;
		    if (pos > MAXCOMMAND) {
				/* command too long */
			return(Err(gv,ERROR_COMMAND_LONG,stdin,stdout));
		    }
		    if ((ch == '\n') || (ch == ENDSTREAMCH))goto endwhile2;
		} /* switch */
nextch:
		/* is the * escaped ? */
		if((ch == '*')&&(lastch=='*'))lastch=0;
		else lastch = ch;
		ch=getchar(gv,lastch);
	}

endwhile2:
		/* terminate string as CSTR */

		if(pos)private_buffer[pos-1]=10; /* rc requires a nl */
						 /* zap the endstream ? */
		private_buffer[pos] = '\0';

		/* Set up defaults for stdin and stdout */
		if ((LONG)stdin < 0) stdin = termin;
		if ((LONG)stdout < 0)stdout = termout;

		/* private buffer is always null-terminated */
		/* pos contains the length now */
		expand(gv,private_buffer,MAXCOMMAND); /* expand variables */
	 	/* handle embedded commands */
		pos=ticks(gv,private_buffer,MAXCOMMAND);
		if(gv->echo) {
		    writef(DOSBase,"%s",private_buffer);
		    Flush(Output());
		}
		if (pos >= 0)	// ticks() succeeded
		{
			/* attempt to run the command */
			SelectInput(stdin);
			SelectOutput(stdout);
			SetIoErr(0);
			SetSignal(0,SIGBREAKF_CTRL_C); /* clear the user sig */
 		        rc=RunCommand(module,4*(clip->cli_DefaultStack),
				private_buffer,pos);
			gv->Res2=IoErr(); /* save IoErr for later */
		} else {
			// ticks() failed
			rc = -pos;	// returned negative error code
			// gv->Res2 already set
		}
#ifdef pdebug
kprintf("after run,stdin is %lx,termin is %lx, stdout is %lx, termout is %lx\n",
	stdin,termin,stdout,termout);
#endif

		if (stdin != termin) {
#ifdef pdebug
kprintf("closing stdin\n");
#endif
		    Close(stdin);
		}
		if (stdout != termout) {
#ifdef pdebug
kprintf("closing stdout\n");
#endif
		    Close(stdout);
		}
	        return(rc);
}

BPTR openf(gv,name,act)
struct Global *gv;
UBYTE *name;
LONG act;
{

   BPTR s;
   struct DosLibrary *DOSBase=gv->DOSBase;
   expand(gv,name,NAMEMAX);
   s= Open(name,act);
   if (!s)SetIoErr(ERROR_REDIRECT); /* unable to open redirection file */
   return(s);
}

/* print the error and close the streams we opened */

LONG Err(gv,m,si, so)
struct Global *gv;
LONG m;
BPTR si, so;
{
   struct DosLibrary *DOSBase=gv->DOSBase;
   if (si > 0)Close(si);
   if (so > 0)Close(so);
   gv->Res2=m;
   return(-9999);
}

/* return seg pointer if its a resident command or builtin */

struct Segment *useseg(DOSBase,name)
struct DosLibrary *DOSBase;
UBYTE *name;
{
   struct Segment *seg = searchsegs(DOSBase,name,NULL);
   if(seg) {
	Forbid();
	// probably not needed, but for paranoia about the code generator...
	if(seg->seg_UC >=0)seg->seg_UC++;
	Permit();
	if((seg->seg_UC < 0) && ((seg->seg_UC > CMD_BUILTIN)  ||
	    (seg->seg_UC <= CMD_DISABLED)))seg=(struct Segment *)0;
   }
   return(seg);
}

struct Segment *searchsegs(DOSBase,name,startseg)
struct DosLibrary *DOSBase;
UBYTE *name;
struct Segment *startseg;
{
    struct Segment *seg;
//kprintf("searching for %s...",name);
    seg=FindSegment(name,startseg,0);	/* additional commands first */
    if((long)seg==0)seg=FindSegment(name,startseg,TRUE); /* now system stuff */
//kprintf("found $%lx\n",seg);
    return(seg);
}

/* check for control-C */

ULONG testflags(flag)
long flag;
{
return((SetSignal(0,flag) & flag));
}

void __stdargs writef( DOSBase, fmt, args )
struct DosLibrary *DOSBase; 
UBYTE *fmt, *args;
{
   VFWritef( Output(), fmt, (LONG *)&args );
}

int ungetchar(gv)
struct Global *gv;
{
struct DosLibrary *DOSBase=gv->DOSBase;
int res=FALSE;
if((gv->count >= gv->cpos) || gv->fill) {
	if(gv->fill != 1)(gv->count)--;
	res=UnGetC(Input(),-1);
#if 0
	/* check to see if we need to reset fill */
	if(gv->fill) {
            if((ch == ']') && 
		(gv->alias[gv->count] == '[') && (lastch != '*')) {
		}
	}
#endif
}
else if(gv->count > 0) {
    (gv->count)--;
    res=TRUE;
}
return(res);
}

// FIX! what is this used for?
// Alias handling.  Treat the buffer like a stream.
// look for [], when seen grab args
// gv->fill has 3 states: having started, currently sucking, done sucking

int getchar(gv,lastch)
struct Global *gv;
int lastch;
{
struct DosLibrary *DOSBase=gv->DOSBase;
int ch;

/* might want test if gv->cpos is non-zero to see if we have any alias */
if((gv->count < gv->cpos) && (gv->fill != 1)) {
    ch=gv->alias[(gv->count)++];
    if(!gv->fill) {
        if((ch == '[') && (gv->alias[gv->count] == ']') && (lastch != '*')) {
	    gv->fill=1; /* start filling in the string here */
	    gv->count++;
	    goto startfill;
	}
    }
}

else if((gv->count >= gv->cpos) || (gv->fill == 1)) {
startfill:
	ch = FGetC(Input());
	if((ch == '\n') || (ch == ENDSTREAMCH)) {
	    if(gv->fill == 1) {
		/* if there are still any characters left in alias */
		gv->fill=2; /* mark done filling in from the command line */
		if(gv->count < gv->cpos) {
		    gv->alias[gv->cpos++]=ch ; /* save char on end of alias */
		    ch = gv->alias[gv->count]; /* get the next char */
		}
	    }
	}
	if(gv->fill != 1)(gv->count)++; /* keep track for unread */
}
return(ch);
}

// FIX! what the HELL does this routine do?!!  It's totally obtuse!

int readitem(gv,string)
struct Global *gv;
UBYTE *string;
{
struct DosLibrary *DOSBase=gv->DOSBase;
int i,res=1;

/* in a readitem clone, we have to eat leading spaces */
while((gv->fill != 1) && (gv->count < gv->cpos) 
    && (gv->alias[gv->count] == ' '))gv->count++;

if((gv->count < gv->cpos) && (gv->fill != 1)) {
    for(i=0; (gv->count < gv->cpos) && (gv->alias[gv->count] != ' '); i++) {
	string[i]=gv->alias[(gv->count)++];
	if((gv->fill) || ((string[i] == '[') && (gv->alias[gv->count]==']'))) {
	    if(!gv->fill)gv->count++; /* skip the close bracket */
	    res=ReadItem(&string[i], NAMEMAX,FALSE);
	    if(res>0) {
		i +=strlen(string);
		break;
	    }
	}
    }
    string[i]=0;
}
else {
   res=ReadItem(string, NAMEMAX,FALSE);
   if((res>0) && (gv->fill != 1))gv->count +=strlen(string);
}
return(res);
}

LONG ticks(gv,string,mc)
struct Global *gv;
UBYTE *string;
LONG mc;
{
struct DosLibrary *DOSBase=gv->DOSBase;
int i,k,max,st,rc;
int ch,lastch=0;
BPTR file;
UBYTE *buf=gv->buffer;
UBYTE fname[NMAX]="T:tick$XX";
LONG len;
LONG pnum= (LONG)(((struct Process *)(THISPROC))->pr_TaskNum);
struct LocalVar *c_echo=0;
struct TagItem tags[] = {
	SYS_Input, NULL,
	SYS_Output, NULL,
	SYS_UserShell,TRUE,
	TAG_DONE,NULL};

/* need to change escaped backticks to backticks here */

if(!(len=strlen(string)))return(0); /* don't even bother */

if(len > mc)return(len); /* too long to handle */

// FIX! needs to handle more than one set of `...` per line!
for(i=0; i<len; i++) {
  ch=string[i];
  if (ch=='`') {
    if(lastch != '*') { /* we've found start of tick sequence */	
	lastch=0;	/* reset for reuse to find string end */
	st=i;	/* save start found */
        for(i=st+1; i<strlen(string); i++) { /* find other end */
	    ch=string[i];
	    if(ch=='`') {
		if(lastch != '*') {
		    memcpy(buf,&string[st+1],i-st-1); /* got other end, its real */
		    buf[i-st-1]=0;

	            /* build a unique command name */
		    vsprintf(&fname[7],"%ld",&pnum);
		    file=Open(fname,MODE_NEWFILE);
		    if(file) {
			tags[1].ti_Data = file;
			/* if they have echo on, turn it off for now */
			/* because echoing ticks doesn't work so well */
/* FIX!? is this needed? */
			if(gv->echo) {
				c_echo=FindVar("echo",LV_VAR);
			        if(c_echo)c_echo->lv_Node.ln_Type |= LVF_IGNORE;
			}
		        rc = System(buf,tags);
			/* ignore returncodes less than FailAt */
			if ((rc >= 0) && (rc < gv->Clip->cli_FailLevel))
				rc = 0;
			else {
				gv->Res2 = IoErr();
				/* we'll insert output at front later */
				vsprintf(string,"`%s`\n",(LONG *) &buf);
			}

		        Close(file);
		        if(c_echo)c_echo->lv_Node.ln_Type &= ~LVF_IGNORE;

			file=Open(fname,MODE_OLDFILE);
			if(file) {
			    k=Read(file,buf,mc);
			    Close(file);
			    buf[k]=0;
			    if(buf[k-1]=='\n')buf[--k]=0;
			    /* test for a long string */
			    if((max = (len-st+k+ 8 - mc))>0)
				buf[k-max]=0; /* cut it off here */
			    /* replace lf with spaces */
			    for(k=0; k<strlen(buf);k++)if(buf[k]==10)buf[k]=' ';
			    /* remove old string */
			    if (rc == 0)
			    {
				for(k=st; k<len; k++)
					string[k]=string[k+i-st+1];
				strins(&string[st],buf); /* add new one */
			    } else if (*buf) {
			        // write output (if any) from failed cmd
				writef(DOSBase,"%s\n",buf);
			    }
			    DeleteFile(fname);
			}
			/* what do we do about negative returncodes? Fix? */
			if (rc > 0)
			{
			    /* ticked command failed.  return failure (neg) and
			       failed command string (in buffer) */
			    return -rc;
			}
		        return(strlen(string));
		    }
		}
		else {
		    /* blank out the * */	
		    strcpy(&string[i-1],&string[i]);
		    i--;
		}
	    }
	    else if ((ch=='*')&&(lastch=='*'))lastch=0;
  	    else lastch = ch;
	}
      }
      else {
    	strcpy(&string[i-1],&string[i]);
	i--;
      }
  }
  else if ((ch=='*')&&(lastch=='*'))lastch=0;
  else lastch = ch;
}
return(len);	/* nothing to do */
}


/* check for active external pipe/multiple command processor */

int pipecheck(gv,status,type)
struct Global *gv;
BOOL status;
UBYTE *type;
{
   struct DOSLibrary *DOSBase=gv->DOSBase;
   int state = 1; /* State indicates parsing state. <0 -> in quotes, */
		  /* >0  out of quotes,    =0 -> in comment */
   int pstate=1;  /* same for parens */
   int ch;
   int lastch=' ';
   struct FileHandle *fh;
   UBYTE *string;
   UBYTE pbuffer[32];
   int i;

   /* check for pipes if pipechar is active */
   /* and this is the first time through */

   i=(int)GetVar(type,pbuffer,NMAX,LV_VAR|GVF_LOCAL_ONLY);
   if(!status && (i >0)) {
	/* hey, pipes are active */
	if(fh = (void *)BADDR(Input())) {
	    string= (UBYTE *)BADDR(fh->fh_Buf);

	    // scan the characters in the FH buffer!  Be VERY careful to track dos
	    // changes!
	    for(i=fh->fh_Pos; i< fh->fh_End; i++) {
		ch=string[i];
		switch ((int)ch) {
		    case ';':		/* comment character */
			if (state > 0) state = 0;
			break;
		    case '\"':		/* quoted values */
			if(lastch != '*')state = -state;
			break;
		    case '\(':
			if(lastch != '*')pstate = -1; /* in parens */
			break;
		    case '\)':
			if(lastch != '*')pstate = 1; /* and out again */
			break;
		    case '*':
			if(lastch == '*')ch=0;
			break;
		    case '\n':
			// break out of buffer scan if we're not in quotes
			if (state >= 0)
				return(0);
			break;
		    default:
	            if((state>0)&&(pstate>0)&&(pbuffer[0]==ch)&&(lastch==' ')) {
		        if(!pbuffer[1] || 
			   (i < (fh->fh_End-1)) && (pbuffer[1] == string[i+1]))
		           return(-6);
		    }
		}
nextch:
		if(!state)break;
		lastch=ch;
	    }
	}
   }
return(0);
}


/*==============================================================*/
/* Function: prbuf						*/
/*  Purpose: The following stub routine is called from RawDoFmt */
/*	     for each character in the string.	At invocation,	*/
/*	     we have:						*/
/*		D0 - next character to be formatted		*/
/*		A3 - pointer to data buffer			*/
/*==============================================================*/
static void __regargs prbuf(char c)
{
  __builtin_emit(0x16c0);   /* move.b D0,(A3)+ */
}

/*====================================================================*/
/* Function: vsprintf						      */
/*  Purpose: This is a tiny implementation of sprintf.	Note that it  */
/*	     assumes that the buffer is large enough to hold any      */
/*	     formatted string you ask it to generate.		      */
/*====================================================================*/
int vsprintf(char *buf, char *ctl, long args[])
{
  RawDoFmt(ctl, args, prbuf, buf);
  return(strlen(buf));
}


void setv(gv,rc)
struct Global *gv;
LONG *rc;
{
    struct DOSLibrary *DOSBase=gv->DOSBase;
    struct LocalVar *result_var;

    /* update current values of result variables */
    memset((UBYTE *)gv->buffer,0,8);
    /* first let's do RC */
    vsprintf(gv->buffer,"%ld",rc); /* print into the buffer */
    result_var=FindVar("RC",LV_VAR);
    /* if we find the variable and the space it big enough */
    if(result_var && (result_var->lv_Len >= VARMAX))
	memcpy(result_var->lv_Value,gv->buffer,VARMAX);
    else SetVar("RC",gv->buffer,VARMAX,LV_VAR|GVF_LOCAL_ONLY);

    /* now lets handle result2 */
    vsprintf(gv->buffer,"%ld",&gv->Res2);
    result_var=FindVar("Result2",LV_VAR);
    /* if we find the variable and the space it big enough */
    if(result_var && (result_var->lv_Len >= VARMAX))
        memcpy(result_var->lv_Value,gv->buffer,VARMAX);
    else SetVar("Result2",gv->buffer,VARMAX,LV_VAR|GVF_LOCAL_ONLY);
}

/* Function: getargs */
/* Purpose: Open dos.library, do a readargs, return readargs result */

struct RDargs *getargs(db,ub,template,opts,size)
struct Library **db;
struct Library **ub;
UBYTE *template;
LONG *opts[];
int size;
{
    struct Library *DOSBase=TaggedOpenLibrary(OLTAG_DOS);
    struct RDargs *rd=NULL;

    *db = DOSBase;	/* return dosbase */
    if(ub) { 		/* and UtilityBase */
        *ub = TaggedOpenLibrary(OLTAG_UTILITY);
    }
    if(size) {
       memset((char *)opts,0,size);
       if(!(rd=ReadArgs(template,(LONG *)opts,NULL))) {
	   PrintFault(IoErr(),NULL);
       }
    }
    return(rd);
}
