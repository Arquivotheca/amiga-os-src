/* cli.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/filehandler.h"
#include <devices/timer.h>

#include <string.h>
#include <stddef.h>

#include "libhdr.h"
#include "cli_init.h"

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"
#include "cli_protos.h"

#define SAME		0		/* for string compares */

#define TOBPTR(x)	(((LONG) (x)) >> 2)


/* BCPL dos used BPTR to the buffers in the filehandles.  I want to use CPTRs
 * for speed reasons, but am leaving them a BPTRs until I can boot the new
 * dos to reduce possible problems using handles created by the old dos.
 * NOTE: must also change setting of the buffer!!! (marked by "FIX!")
 * ALSO NOTE: must change bcplio.c also!!!!
 * Can't do it: Execute/Run/NewCLI(?) use it also.  ARGH
 */

#ifdef NEWDOS
#define BUFFER(fh)	((UBYTE *) (fh)->fh_Buf)
#define TOBUFFER(buf)	(buf)
#else
#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TOBPTR(buf))
#endif


/*
// This is a totally rewritten CLI for AmigaDOS
// Modifications to make it use the Resident list added
// in conditional compilation flags - NHG 6Sep86
*/

/*
// We use five basic IO streams throughout this CLI
// 1) stderr         Always the console (standardout)
// 2) stdin          Set by <, defaults to console (CIS)
// 3) stdout         Set by >, defaults to console (COS)
// 4) cmdin          Set by EXECUTE or eof to file or console (currentin)
// 5) conin          Default console input stream (standardin)

// The initial packet contains a function to call for
// initialisation purposes; or zero if the global one
// is to be used.
*/

/* this is the only call visible from outside */

#ifdef OLD_CLI
void ASM
start (REG(d1) struct DosPacket *parm_pkt)
{
	char *prompt;
	char *commandname;
	char *commandfile;
	char *setname;
	long error = FALSE;
	long tnum;
	struct Process *proc;
	BPTR (*fn)(struct DosPacket *);
	struct Path *p;
	struct CommandLineInterface *clip;
	BPTR standardout;
	LONG init_type;

	/* strings have to be longword aligned, ARGH */
	/* cli structure has to also */

	prompt = AllocVec(PROMPTMAX*4 + NAMEMAX*4 + FILEMAX*4 + SETMAX*4 +
			   sizeof(struct CommandLineInterface),
			  MEMF_PUBLIC|MEMF_CLEAR);
	if (!prompt)
		exit();		/* kill self - FIX! maybe should alloca it */

	commandname = prompt      + PROMPTMAX*4;
	commandfile = commandname + NAMEMAX*4;
	setname     = commandfile + FILEMAX*4;
	clip	    = (struct CommandLineInterface *) (setname + SETMAX*4);

	proc = MYPROC;

	proc->pr_CLI = TOBPTR(clip);

	clip->cli_Prompt      = TOBPTR(prompt);
	clip->cli_CommandName = TOBPTR(commandname);
	clip->cli_CommandFile = TOBPTR(commandfile);
	clip->cli_SetName     = TOBPTR(setname);

	/* dp_Type will be CPTR to bcpl function!!!! */
	fn = (BPTR (*)(struct DosPacket *))
	     (parm_pkt == 0 ? NULL : parm_pkt->dp_Type);
/*
     // Call the initialisation routine.
     // Return a function, or 0 (nothing to do), 
     // or <0 (return parm.pkt at end)
*/
	/* DANGER! function returned by init CAN'T be above 2 gig!!!! FIX */

/*
	If function is 0, we assume we're booting, and call cli_init.

	We used to accept a function ptr here.  Now we look at the res 1 & 2
	fields.  0,0 -> call cli_init_run; 1,0 -> call cli_init_newcli.
	These values were used consistently in 1.3 and before BCPL versions
	of c:Run and c:NewCli/NewShell.  Other values are reserved.  The
	function ptr passed in must be non-zero, but is otherwise ignored.

	As far as I know, there are exactly 3 things that pass packets to here:
	doslib.asm, c:Run, and c:NewCli/c:NewShell.

	We handle BCPL shells by having 1.4 Run/NewShell invoke
	"Shell" from the resident list, not CLI.  The boot shell is also
	on the resident list as CLI so 1.3 BCPL NewCli will work (it's broken).
	It is also invoked for System if UserShell is specified.

	We need to not run with a bcpl shell because we need to supply a
	default input channel to commands run under System().  The BCPL shell
	would continue to read commands from the said filehandle.  We need to
	recognize System() calls and use different filehandles for shell input
	and for programs run under the shell.

*/
	/* produces values in 0..3 range */
	init_type = (parm_pkt->dp_Res1 == 0 ? 0 : 2) |
		    (parm_pkt->dp_Res2 == 0 ? 0 : 1);

	if (fn) {
#ifdef USE_FPTR
		fn = (BPTR (*)(struct DosPacket *))
		     call_bcpl_fptr(parm_pkt,(LONG) fn);
#else
		if (init_type == 0)
		{
			fn = (BPTR (*)(struct DosPacket *))
			     cli_init_run(parm_pkt);	/* Cli init for c:run */
		} else if (init_type == 2) {
			fn = (BPTR (*)(struct DosPacket *))
			     cli_init_newcli(parm_pkt);
					  /* Cli init for c:NewCli/c:NewShell */
		} else {
			abort(AN_BadInitFunc | AT_DeadEnd);
		}
#endif
	} else {
		/* we are booting! (always returns 0) */
		fn = (BPTR (*)(struct DosPacket *)) cli_init(parm_pkt);
	}

	/* if a ptr was passed back OR this is an asynch system call */
	if ((((LONG) fn) > 0) || (((LONG) fn) & 8))
	{
/*
	The only functions returned here are qpkt (for types 00 and 10) for
	an argument of parm_pkt; and deletetask for an argument of FindTask(0).
	We'll key off the value of result2.  Ugly but required.
*/
#ifdef USE_FPTR
		/* note that I must pass a CPTR to call_bcpl_fptr */
		call_bcpl_fptr((struct DosPacket *) BADDR(getresult2()),
			       (LONG) fn);
#else
		if (getresult2() == (LONG) FindTask(0))
		{
			/* FIX! cleanup! */
			deletetask();
		} else {
			qpkt(parm_pkt);
		}
#endif
	}

	tnum = proc->pr_TaskNum;

	standardout = clip->cli_StandardOutput;
	proc->pr_HomeDir = NULL;

	while (1)
	{
		LONG ch;

		do {	/* while ch != ENDSTREAMCH */
			LONG item;

			/* REALLY wierd!!! */
			/* sets interactive if using standard input AND */
			/* if background is not set 			*/

			clip->cli_Interactive  = ~(clip->cli_Background) &
			  ((clip->cli_CurrentInput == clip->cli_StandardInput) ?
			   DOSTRUE : FALSE);

			selectinput(clip->cli_CurrentInput);
			selectoutput(standardout);

			if (clip->cli_Interactive)
			{
				FPutC(standardout,'O'-'@');
						/* change to default char set */
				bwritef(prompt,&tnum,1);  /* write prompt */
				Flush(standardout);
							/* forces output(!!) */
			} else {
				/* check for "break script" signal */

				if (SetSignal(0,SIGBREAKF_CTRL_D) &
				    SIGBREAKF_CTRL_D)
				{
					error = TRUE;
					FPuts(standardout,"****BREAK - CLI\n");
				}
				/* stop script if error from command or ^D */
				if (error)
					break; /* from do {...} while (); */
			}
			error = FALSE;

			/* one for null, one for len byte */
			item  = ReadItem(commandname+1,(NAMEMAX << 2)-2,NULL);
			/* name is converted to BSTR before calling prog */

			ch = 0;
			if (item != 0)
			{
				error = TRUE;
				if (item > 0)
				{
				  /* a quoted or unquoted item */
				  LONG rc = -1;
				  LONG res2 = 0; /* error code if ~found */
				  BPTR module = 0;
				  LONG firsttime = TRUE;
				  BPTR dir;
				  struct Segment *segment;

retry:
				  /* current dir lock */
				  dir = currentdir(FALSE,NULL);
				  p = (struct Path *)
				      BADDR(clip->cli_CommandDir); /* path */

				  /* try the resident segment list */
				  Forbid();
				  segment = searchsegs(commandname+1,NULL,
						       FALSE);

				  /* ignore things marked system (uc == -1) */
				  if (segment && segment->seg_UC >= 0)
				  {
					module = segment->seg_Seg;
					segment->seg_UC++;
				  }
				  Permit();

				  /* Try current directory */
				  /* NOTE: load() sets pr_HomeDir via DupLock */
				  /* pr_HomeDir is NULL for resident mods(?) */
				  if (module == NULL)
					module = load(commandname+1,dir,&res2,
						      TRUE);

				  /* Try directories in the path list */
				  while (!module && p != NULL)
				  {
					module = load(commandname+1,
						      p->path_Lock,
						      &res2,TRUE);
					p = (struct Path *) BADDR(p->path_Next);
				  }

				  /* Try c: */
				  if (!module)
				  {
				      struct DevProc *dp = NULL;
				      struct MsgPort *fstask = filesystemtask();

				      do {
					dp = getdevproc("c:",dp);
					if (dp)
					{
					  /* lock may be NULL! */
					  setfilesys(dp->dvp_Port);
					  module = load(commandname+1,
							dp->dvp_Lock,&res2,
							FALSE);
					}
					/* repeat if multi-assign */
				      } while (!module && dp &&
					       (dp->dvp_Flags & DVPF_ASSIGN) &&
					       getresult2() ==
							ERROR_OBJECT_NOT_FOUND);

				      setfilesys(fstask);  /* reset it */

				      if (dp)
					freedevproc(dp);
				  }

			  /* now restore the current dir (modified by load()) */
				  currentdir(TRUE,dir);

/*
	      // if all that failed, try again because
	      // the user may have replace a disc and made a
	      // directory in the path list valid. Only do
    	      // this once more, but not if the error was
	      // device.not.mounted (218) which implies that
	      // the requestor was cancelled.
*/
				  if (!module && firsttime &&
				      getresult2() != ERROR_DEVICE_NOT_MOUNTED)
				  {
					firsttime = FALSE;
					goto retry;
				  }

				  /* convert commandname buffer to BSTR! */
				  *commandname = strlen(commandname+1);

				  SetSignal(0,SIGBREAKF_CTRL_C);
				  clip->cli_Module = module;
				  if (module)			/* run it! */
					rc = load_and_run(module,clip);

				  /* free home lock created by load() */
				  if (proc->pr_HomeDir)
				  {
					freeobj(proc->pr_HomeDir);
					proc->pr_HomeDir = NULL;
				  }

				  /* rc inited to -1 above */
				  if (rc == -1)
				  {
					getr2(&res2);
					/* now give Unknown command or more */
					if (!res2)
						writef("Unknown command %S\n",
						       (LONG) commandname+1);
					else {
						writef("Unable to load %S: ",
						       (LONG) commandname+1);
						fault(res2,NULL);
					}
				  } else {
					res2 = (rc == 0 ? 0 : getresult2());

					clip->cli_ReturnCode = rc;
					if (rc < clip->cli_FailLevel)
						error = FALSE;
					selectoutput(standardout);
					if (error && !clip->cli_Interactive)
					    writef("%S failed returncode %N\n",
						   (LONG) commandname+1,rc);
				  }

				  selectinput(clip->cli_CurrentInput);
				  /* unrdch returns true if it works, or */	
				  /* if there are no chars in the buffer */
				  /* and none to unread */
				  /* note that ungetc of -1 works like old */
				  ch = UnGetC(clip->cli_CurrentInput,-1) ?
				       0 : '\n';

				  if (segment == NULL)
					unloadseg(clip->cli_Module);
				  else if (segment->seg_UC > 0)
					segment->seg_UC--;

				  clip->cli_Module  = 0;
				  clip->cli_Result2 = res2;

				} /* if item > 0 */
				else
					writes((LONG)"Error in command name\n");
			} /* if item */

			while (ch != '\n' && ch != ENDSTREAMCH)
				ch = FGetC(clip->cli_CurrentInput);

		} while (ch != ENDSTREAMCH);

		/* Endcli sets fh_End = 0, which causes rdch() to call   */
		/* replenish(), which returns ENDSTREAMCH on fh_End == 0 */
		/* Endcli ALSO sets background! */

		/* if the flag is set for System(), don't continue to read */
		/* from standardinput - instead, exit. */

		if (((((LONG) fn) & 0x80000004) == 0x80000004) ||
		    (clip->cli_CurrentInput == clip->cli_StandardInput))
		{
			/* eof on Run, Execute(), System() */
			break;

		} else {
			/* end of an execute command */
			endread();

			/* commandfile is a CPTR to a BSTR!!! */
			if (commandfile[0] != 0)
			{
				BtoCstr((CPTR) commandfile);
				deleteobj(commandfile);
			}
			commandfile[0] = 0;
			clip->cli_CurrentInput = clip->cli_StandardInput;
			clip->cli_FailLevel = CLI_INITIAL_FAIL_LEVEL;
		}
	} /* while 1 */

/*
     // If EXECUTE called then pass back startup packet to caller
     // now we are finished. Note that we do not close the IO streams
     // in this case unless we opened them for the user. When fn is <0
     // it is a flag word as follows:
     // Bit 31     Set to indicate flags
     // Bit  3	   Set to indicate asynch system call
     // Bit  2     Set if this is a System() call
     // Bit  1     Set if user provided input stream
     // Bit  0     Set if RUN provided output stream

     // If asynch System call, close IO streams always!
*/

	if ((LONG) fn < 0)	/* bit 31 set */
	{
		struct FileHandle *scb;

		/* Flush buffered output */
		scb = (struct FileHandle *) BADDR(output());
		if (scb->fh_Pos > 0)
		{
			(void) deplete(scb);
			scb->fh_Pos = 0;
		}

/*      // Decode flag word. Close any io streams which the user has
        // not provided. Do not close IO streams provided by user.
*/
		if (!(((LONG) fn) & 2))
			endread();

		if (((LONG) fn) & 1)
			endwrite();

		/* return returncode of last program run */
		if (!(((LONG) fn) & 8))
			returnpkt(parm_pkt,clip->cli_ReturnCode,
				  clip->cli_Result2);

	} else {		/* Otherwise close IO streams, normal case */
		endread();
		endwrite();
	}

	freeobj(currentdir(TRUE,NULL));		/* don't leave bad currentdir */
	/* from here on the current dir is NULL! */

	proc->pr_HomeDir = NULL;		/* don't leave bad homedir */

	p = (struct Path *) BADDR(clip->cli_CommandDir);
	while (p)
	{
		struct Path *next;

		next = (struct Path *) BADDR(p->path_Next);
		freeobj(p->path_Lock);
		freeVec(p);

		p = next;
	}

	proc->pr_CLI = NULL;
	freeVec(prompt);	/* frees entire CLI structure! */

	kill_cli_num(tnum);	/* remove from cli task array */

	/* no deletetask needed - fall out */
}

BPTR __regargs
load (file,dir,codep,flag)
	char *file;
	BPTR dir;
	LONG *codep;
	LONG flag;	/* TRUE means noreq */
{
	struct Process *proc;
	BPTR seg;
	char *p,c;

	currentdir(TRUE,dir);	/* will be reset by calling routine */

	seg = flag ? noreqloadseg(file) : loadseg(file);
	if (seg) {
		proc = MYPROC;

		/* must use a copy!!! orig could be unlocked at any time! */
		/* we need a lock on the directory the program was actually */
		/* loaded from.  Look for ':' or '/'.  */
		/* copydir doesn't work on NULL (FIX??!) */
		if (!(p = strrchr(file,'/')))
		{
			/* no '/', check for ':' */
			if (!(p = strchr(file,':')))
			{
				/* no ':' - must be bare filename */
				proc->pr_HomeDir = dir ? copydir(dir) :
							 locateobj("");
				goto got_home;
			} else {
				p++;	/* point past ':' */
			}
		}
		/* p points to '/' or character after ':' */
		c = *p;		/* save char          */
		*p = '\0';	/* temporary          */
		proc->pr_HomeDir = locateobj(file);
		*p = c;		/* put character back */
got_home:
		return seg;
	}

	getr2(codep);

	return NULL;
}

void __regargs
getr2 (codep)
	LONG *codep;
{
	LONG r2 = getresult2();

	if (codep && r2 != ERROR_OBJECT_NOT_FOUND &&
		     r2 != ERROR_DEVICE_NOT_MOUNTED)
		*codep = r2;
}

/*
// Attempt to load and run the required module
// Return -2 if syntax error (having printed message)
// Else return result of runcommand
*/

LONG __regargs
load_and_run (module,clip)
	BPTR module;
	struct CommandLineInterface *clip;
{
	BPTR termin  = clip->cli_StandardInput;
	BPTR termout = output();
	BPTR ifh = input();
	LONG stdin   = -1;
	LONG stdout  = -1;
	LONG rc;
	LONG ch;

	while (1)
	{
		if (stdin == 0 || stdout == 0)		/* Error */
			return err((char *) getresult2(),stdin,stdout);

		ch = FGetC(ifh);
		switch (ch) {
		case ' ':
		case '\t':
			continue;
		case '>':
			if (stdout > 0)		/* already set */
				break;
			stdout = openf(FALSE);  /* error printed by err above */
			continue;
		case '<':
			if (stdin > 0)		/* already set */
				break;
			stdin = openf(TRUE);	/* error printed by err above */
			continue;
		default:
			goto endwhile;
		}
		return err("Too many < or >",stdin,stdout);
	} /* while */

endwhile:
/*
   // We now read the rest of the line and place it into a buffer.
   // This is passed to runcommand to be made available to the calling
   // program. We also patch up the scb so that programs calling the
   // character interface read the command tail as part of their input.
   // Note that the stdin stream is either a new input file; or it
   // is from the terminal. In either case there are no characters
   // buffered within the stream buffer at this time. Programs running
   // under the CLI must not close stdin so we can safely replace the
   // SCB buffer with a private one, filled with the command line.
   // If stdin is new then the buffer pointer will be overwritten when
   // the command line is exhausted. If stdin is the terminal then
   // reads will use the replacement buffer instead.

   EVIL! EVIL! EVIL! - REJ
*/

	{
		LONG pos = 0;
		BPTR orig_buf;
		LONG orig_pos,orig_end;
/*    // State indicates parsing state. <0 -> in quotes, >0  out of quotes,
      //                                =0 -> in comment
*/
		LONG state = 1;
		char buffer[258];	/* assume word alignment! */
		char *private_buffer = buffer;
		struct FileHandle *fh;

		/* EVIL! FIX!!! */
		if ((LONG) private_buffer & 2)
			private_buffer += 2;	/* force to lword pointer!! */

		while (1)
		{
			LONG c;			/* because of '+' */

			c = ch;
			switch (ch) {
			case ';':		/* comment character */
				if (state >= 0)
					state = 0;
				goto nextl;
			case '"':		/* quoted values */
				state = -state;
				goto nextl;
			case '+':		/* maybe a trailing plus */
				if (FGetC(ifh) == '\n')
					c = '\n';
				else
					UnGetC(ifh,-1);	/* push back last */
			default:
nextl:
				if (!state)
					break;
				/* FALL THRU */
			case '\n':
			case ENDSTREAMCH:
				/* so we don't leave 0xff's in the buffer */
				if (ch == ENDSTREAMCH)
					c = ch = '\n';
				private_buffer[pos++] = c;
				if (pos >= 256)
				    return err("Command too long",stdin,stdout);

				if (ch == '\n' || ch == ENDSTREAMCH)
					goto endwhile2;
			} /* switch */

			ch = FGetC(ifh);

		} /* while 1 */
endwhile2:
		/* terminate string as CSTR */
		private_buffer[pos] = '\0';

		/* Set up defaults for stdin and stdout */
		if (stdin < 0)
			stdin = termin;
		if (stdout < 0)
			stdout = termout;

		fh = (struct FileHandle *) BADDR(stdin);
		orig_buf = fh->fh_Buf;
		orig_pos = fh->fh_Pos;
		orig_end = fh->fh_End;

		/* we forced it to lword alignment */
		fh->fh_Buf = TOBUFFER(private_buffer);	/* FIX! */
		fh->fh_Pos = 0;
		fh->fh_End = pos;

		/* attempt to run the command */
		selectinput(stdin);
		selectoutput(stdout);
		setresult2(0);

		/* private buffer is always null-terminated */
		rc = runcommand(module,clip->cli_DefaultStack,private_buffer,
				pos);

		/* fix the filehandle */
		if (fh->fh_Buf == TOBUFFER(private_buffer))	/* FIX! */
			fh->fh_Buf = orig_buf;
		fh->fh_Pos = orig_pos;
		if (fh->fh_End != 0)
			fh->fh_End = orig_end;
	}

	if (stdin != termin)
		endstream(stdin);
	if (stdout != termout)
		endstream(stdout);

	return rc;
}

BPTR __regargs
openf (f)
	LONG f;
{
	char stdvec[NAMEMAX*4];
	BPTR s = 0;

	if (ReadItem(stdvec,(NAMEMAX << 2)-1,NULL) > 0)
	{
		if (f)
			s = findinput(stdvec);
		else
			s = findoutput(stdvec);

		if (!s)
			setresult2((LONG) "Unable to open redirection file");
	} else
		setresult2((LONG) "Syntax error");

	return s;
}

LONG __regargs
err (m,si,so)
	char *m;
	BPTR si,so;
{
	writef("CLI error: %s\n",(LONG) m);
	if (si > 0)			/* depends on using lsr to shift fh! */
		endstream(si);
	if (so > 0)
		endstream(so);
	return -2;
}
#endif /* OLD_CLI */

struct Segment * ASM
searchsegs (REG(d1) UBYTE *name,REG(d2) struct Segment *start,
	    REG(d3) LONG system)
{
	struct Segment *list = start;
	LONG len;

	Forbid();
	if (!list)
		list = (struct Segment *)
		       BADDR(((struct DosInfo *)
			      BADDR(rootstruct()->rn_Info))->di_NetHand);
	else
		list = (struct Segment *) BADDR(list->seg_Next);

	len = strlen(name);
	while (list)
	{
		if (len == list->seg_Name[0] &&
		    mystrnicmp(name,&(list->seg_Name[1]),len) == SAME)
		{
			if (system)
			{
				if (list->seg_UC < 0)
					goto ss_exit;
			} else if (list->seg_UC > 0)
				goto ss_exit;
			/* MCC uses 0 for something funny */
		}
		list = (struct Segment *) BADDR(list->seg_Next);
	}
	setresult2(ERROR_OBJECT_NOT_FOUND);
ss_exit:
	Permit();
	return list;
}

LONG ASM
RemSegment (REG(d1) struct Segment *segment)
{
	struct Segment *list;
	LONG rc = FALSE;

	setresult2(ERROR_OBJECT_IN_USE); /* default error */

	/* allow unload on 0/1 */
	Forbid();
	if (segment->seg_UC < 0 || segment->seg_UC > 1)	/* stupid tripos/MCC */
		goto done;

	/* Careful! counts on seg_next being at offset 0 */
	/* set list initially to the address of the pointer to the first seg */

	for (list = (struct Segment *)
		    &(((struct DosInfo *)
		       BADDR(rootstruct()->rn_Info))->di_NetHand);
	     list;
	     list = (struct Segment *) BADDR(list->seg_Next))
	{
		if (((LONG) BADDR(list->seg_Next)) == ((LONG) segment))
		{
			list->seg_Next = segment->seg_Next;
			unloadseg(segment->seg_Seg);
			freeVec(segment);
			rc = DOSTRUE;
			goto done;
		}
	}
done:
	Permit();
	return rc;
}

#define FREE_MEMENTRY

LONG ASM
RunCommand (REG(d1) BPTR seg, REG(d2) LONG stack, REG(d3) UBYTE *args,
	    REG(d4) LONG length)
{
	BPTR bfh = input();
	struct FileHandle *fh;
    	BPTR old_buf;
	LONG old_pos, old_end;
	LONG rc,rc2;
	char *buffer;
#ifdef FREE_MEMENTRY
	struct MinList save_mementry;
	struct MinList *task_mementry = (struct MinList *)
					&(MYPROC->pr_Task.tc_MemEntry);
#endif
	fh = BADDR(bfh);

	/* for 1.3 compatibility, buffer must be at least 208 bytes long */
	if (!(buffer = AllocVecPub(MAX(length,208))))
		return -1;

#ifdef FREE_MEMENTRY
	// save old tc_MemEntry list
	save_mementry = *task_mementry;
	NewList((struct List *) task_mementry);
#endif

	copyMem((APTR) args,buffer,length);

	old_buf = fh->fh_Buf;
	old_pos = fh->fh_Pos;
	old_end = fh->fh_End;

	fh->fh_Buf = TOBPTR(buffer);
	fh->fh_Pos = 0;
	fh->fh_End = length;

	rc  = InternalRunCommand(seg,stack,args,length);
	rc2 = getresult2();

	/* This forces the fh to think the buffer is a read buffer again */
	/* (the program might have done buffered output to Input()).	 */
	UnGetC(bfh,-1); /* let DOS adjust the filehandle */

	/* something (unknown) used to muck with the buffer (BCPL). */
	if (fh->fh_Buf == TOBPTR(buffer))
	{
		fh->fh_Buf = old_buf;
		fh->fh_Pos = old_pos;
		/* don't overwrite the signal from Endcli! (end = 0) */
		if (fh->fh_End)
			fh->fh_End = old_end;
	}

	FreeVec(buffer);

#ifdef FREE_MEMENTRY
	// free anything left on the tc_Mementry list
	{
		struct MemList *entry;

		while (entry = (struct MemList *)
			       RemHead((struct List *) task_mementry))
			FreeEntry(entry);
	}
	// and restore...
	*task_mementry = save_mementry;
#endif

	/* restore IoErr() for the caller */
	setresult2(rc2);
	return rc;
}
