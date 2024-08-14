/* bcplio.c */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include "dos/dos.h"
#include "dos/dosextens.h"

#include <string.h>

#include "libhdr.h"
#include "dos_private.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "blib_protos.h"
#include "klib_protos.h"

#define SAME		0			/* for string compares */
#define TOBPTR(x)	(((LONG) (x)) >> 2)	/* convert to bptr */

/* for doing __asm stuff */
#define ASM __asm
#define REG(x)	register __ ## x	/* ANSI concatenation */


/* BCPL dos used BPTR to the buffers in the filehandles.  I want to use CPTRs
 * for speed reasons, but am leaving them a BPTRs until I can boot the new
 * dos to reduce possible problems using handles created by the old dos.
 * NOTE: must also change setting of the buffer!!!
 * ALSO NOTE: must change cli.c also!!!!
 * Can't do it: Execute/Run/NewCLI(?) use it also.  ARGH
 */

#ifdef NEWDOS
#define BUFFER(fh)	((UBYTE *) (fh)->fh_Buf)
#define TOBUFFER(buf)	(buf)
#else
#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TOBPTR(buf))
#endif

static void __regargs common_getc (struct FileHandle *cis);

LONG ASM
FGetC (REG(d1) BPTR fh)
{
	struct FileHandle *cis;
	LONG pos,end;

	cis = BADDR(fh);
	if (!cis)
		return ENDSTREAMCH;

	/* handle all type-of-buffer issues */
	common_getc(cis);

	/* stupid old bcpl endcli just sets fh_End to 0, ignoring fh_Pos */
	/* this caused them not to be equal, and it would read -1(null)  */
	/* on EOF, replenish returns EOF, but does NOT set it to always  */
	/* return EOF - it will retry the read on the next attempt.      */
	/* this also improves handling of read errors.			 */
	pos = cis->fh_Pos;
	end = cis->fh_End;

	/* handle EOF; UnGetC(fh,-1); FGetC(fh) - should return EOF! */
	/* pos == -3, means we unread an EOF */
	/* pos == -2, means we read an EOF last */
	if (pos == -3)
	{
		cis->fh_Pos = -2;	/* only return the EOF once */
		return ENDSTREAMCH;
	}

	/* if we read an EOF last, keep trying */
	if (pos < 0 || pos >= end)
		return replenish(end,cis);

	/* first call, buffer exhausted or stream exhausted */
	/* Note: pos changed! */
	return (LONG) BUFFER(cis)[(cis->fh_Pos)++];
}

static void __regargs
common_getc (struct FileHandle *cis)
{
	/* we use fh_Func3 as a "buffer may be dirty" flag */
	/* if it might be dirty, flush it.  replenish will then be called */

	/* NIL: leaves funcs == 0 */
	if (cis->fh_Func3 && cis->fh_Func3 != (LONG) actendread)
	{

/*if (cis->fh_Func3 == (LONG) actendread)
kprintf("fgetc: switching from write to read, fh = 0x%lx!\n",cis);
else
kprintf("fgetc: initial read from 0x%lx\n",cis);
*/
		/* If func3 = actend, then this was NOT a write buffer   */
		/* Either it hasn't been touched, and pos/end are -1, or */
		/* it has had a buffer transplant from a shell, in which */
		/* case we do NOT want to call replenish (yet)! 	 */
		if (cis->fh_Func3 != (LONG) actend)
		{
			/* this writes the dirty buffer, if any */
			Flush(TOBPTR(cis));

			/* set pos/end for read (forces replenish) */
			cis->fh_Pos = cis->fh_End = -1;
		}

		/* mark the buffer as a read buffer */
		cis->fh_Func3 = (LONG) actendread;
	}
}

/* if ch == -1 (EOF), then don't change the char (kludge) */
/* we'll assume they just read a character - no checks of buffer type here */

LONG ASM
UnGetC (REG(d1) BPTR fh, REG(d2) LONG ch)
{
	struct FileHandle *cis;
	LONG pos,end;
	UBYTE *p;

	cis = BADDR(fh);
	if (!cis)
		return FALSE;

	/* handle all type-of-buffer issues */
	common_getc(cis);

	pos = cis->fh_Pos;
	end = cis->fh_End;

	/* this handles UnGetC(fh,-1) after an EOF - it should return EOF */
	/* see FGetC - pos can be -2 or -3, handle the same */
	if (ch == -1 && pos < -1)
	{
		cis->fh_Pos = -3;	/* flag to fgetc to EOF read */
		return FALSE;		/* couldn't unread */
	}

/* FIX!?? */
	/* End==0 is a flag for continual EOF from EndCli */
	/* -1 means it was never buffered before, 0 means at start of buf */
	if (pos == -1 || pos == 0)
		return (end == 0 ? DOSTRUE : FALSE);

	/* real ungetc of a character - do it, ignore EOF flag */
	if (pos < 0)		/* must be -2 or -3 */
		pos = end;	/* turn off EOF flag */

	cis->fh_Pos = --pos;	/* update ptr */

	p = &(BUFFER(cis)[pos]);
	if (ch != -1)
		return (LONG) (*p = ch);

	/* ch == -1, means unread last char read.  for compatibility, we'll  */
	/* return -1 (which the old unrdch would have done). */
	return DOSTRUE;
}

/* returns ch or EOF for error */

LONG ASM
FPutC (REG(d1) BPTR fh, REG(d2) LONG ch)
{
	struct FileHandle *cos;

	cos = BADDR(fh);
	if (!cos)
		return ENDSTREAMCH;

	/* we use fh_Func3 as a "buffer may be dirty" flag */
	/* if it's a read buffer, flush it.  We can then start filling it. */
	/* NIL: leaves funcs == 0 */
	if (cos->fh_Func3 && cos->fh_Func3 != (LONG) actendwrite)
	{
/*
if (cos->fh_Func3 == (LONG) actendread)
kprintf("switching from read to write, fh = 0x%lx!\n",cos); 
else
kprintf("initial buffered write to 0x%lx\n",cos);
*/
		/* this resets us to the correct position (pos/end == -1) */
		Flush(fh);

		/* If func3 = actend, then this was NOT a read buffer    */
		/* Either it hasn't been touched, and pos/end are -1, or */
		/* it has had a buffer transplant from a shell, in which */
		/* case we want to make fh_Pos and fh_End -1.		 */
		if (cos->fh_Func3 != (LONG) actend)
		{
			/* set the fh_End to the buffer size (FIX!!!!!) */
			cos->fh_Pos = 0;
			cos->fh_End = BufferSize((struct NewFileHandle *) cos);
		} else {
			/* force it to allocate a new buffer */
			/* just in case a shell stuffed an input buffer here */
			/* but no one ever used a buffered read on it. */
			cos->fh_Pos = cos->fh_End = -1;
		}

		/* mark the buffer as a write buffer */
		cos->fh_Func3 = (LONG) actendwrite;
	}
	/* see above comments re: endcli.  Fixed here too for safety */
	/* note that on the initial write, pos == end == -1          */

	/* flushes on full buffer */
	if (cos->fh_Pos < 0 || cos->fh_Pos >= cos->fh_End)
	{
		/* NIL: works here! */
		/* failed on error or func == 0 - return success for NIL:*/
		if (!deplete(cos))
		{
			if (cos->fh_Func2 == NULL)
				return ch;
			else
				return ENDSTREAMCH;
		}
	}

	BUFFER(cos)[cos->fh_Pos++] = ch;

	/* flush if interactive on certain control chars */
	if (((LONG) cos->fh_Flags) & FHF_NONE)
	{
		/* no buffering, flush on each character */
		goto flush_it;

	} else if (!(((LONG) cos->fh_Flags) & FHF_FULL)) {

		/* normal line buffering - the default */
		if (ch < ' ' && cos->fh_Interactive &&
		    (ch == '\n' || ch == 0 || ch == '\r' || ch == '\12'))
		{
flush_it:		if (!deplete(cos))
				return ENDSTREAMCH;
		}
	}
	/* FHF_FULL means it's all handled above in the buffer-full case */

	return ch;
}

LONG ASM
Flush (REG(d1) BPTR fh)
{
	struct FileHandle *cos;
	LONG func3;

	cos = (struct FileHandle *) BADDR(fh);
	if (!cos)
		return DOSTRUE;
	func3 = cos->fh_Func3;

	/* the first test keeps us from doing anything if it isn't buffered */
	/* we use fh_Func3 as a "buffer may be dirty" flag */
	/* not the best, but it ought to work unless people start mucking */

	if (func3 == (LONG) actend)	/* never read from */
		goto reset_fh;		/* people can put data in the buffer */

	if (cos->fh_Pos < 0)
	{
		if (cos->fh_Pos == -1)	/* nothing in the buffer */
			return DOSTRUE;
		else
			goto reset_fh;	/* fgetc or ungetc an eof */
	}

	if (func3 == (LONG) actendwrite) {
		/* leaves file positioned correctly if we switch to either */
		/* non-buffered writes/reads or to buffered reads. */
		deplete((struct FileHandle *) cos);

		/* FIX!!! should return result of deplete for writes */

	} else if (func3 == (LONG) actendread) {
		/* flushing read filehandle, buffered */
		/* try to seek to current buffered read position first */
		/* (gulp) */

		noflushseek(fh,cos->fh_Pos - cos->fh_End,OFFSET_CURRENT);
reset_fh:
		/* as if it was never buffered */
		cos->fh_Pos = cos->fh_End = -1;
	}

	return DOSTRUE;
}

LONG __regargs
replenish (end,cis)
	LONG end;
	struct FileHandle *cis;
{
	LONG __regargs (*func)(struct FileHandle *);

/*kprintf("replenish of 0x%lx, fh_Type = 0X%lx...",cis,cis->fh_Type);*/
	/* this is how EndCli forces termination of the stream!!!! */
	/* if it formerly was a write buffer, end is buffer size */
	if (end == 0)
		return ENDSTREAMCH;	/* EOF */

	func = (LONG __regargs (*)()) cis->fh_Func1;
	if (func == 0 || !((*func)(cis)))
	{
		/* read error or eof or NIL: */
		/* cis->fh_Pos = cis->fh_End = 0; */
		/* used to set fh_Pos=fh_End=0 here to force continual EOF */
		/* I fixed readitem to not unread on EOF, so it's not needed */

		/* now we set pos = -2 to flag to ungetc that we hit eof */
		cis->fh_Pos = -2;
		return ENDSTREAMCH;		/* EOF */
	}

	cis->fh_Pos = 1;

/*kprintf("done with replenish\n");*/
	return (LONG) BUFFER(cis)[0];
}

LONG __regargs
deplete (cos)
	struct FileHandle *cos;
{
	LONG res2,rc;
	LONG __regargs (*func)(struct FileHandle *);

/*kprintf("deplete of 0x%lx\n",cos);*/
	res2 = getresult2();

	func = (LONG __regargs (*)()) cos->fh_Func2;
	if (func == NULL)
	{
		/* DON'T set fh_Pos = 0 for NIL:! */
		return FALSE;
	}
	/* it sets fh_Pos to 0 on success */
	/* on initial call it also sets fh_End to SCB_BUFFERSIZE */
	/* returns number of chars written */
	rc = (*func)(cos);

	setresult2(res2);
	return (rc > 0 ? DOSTRUE : FALSE);
}

/* general.interface unneeded */
/* i.open == findstream */

void ASM
init_scb (REG(d1) struct FileHandle *fhb, REG(d2) ULONG type)
{
	struct NewFileHandle *scb = (void *) fhb;

	scb->fh_Flags = FHF_EXTEND; 	/* FHF_LINE is the default mode */
	scb->fh_Interactive = scb->fh_Type = 0;
	scb->fh_Pos = -1;
	scb->fh_End = -1;
	scb->fh_Buf = NULL;			/* paranoia */
	scb->fh_Func1 = (LONG) actreadinit;
	scb->fh_Func2 = (LONG) actwriteinit;
	scb->fh_Func3 = (LONG) actend;
	scb->fh_BufSize = SCB_BUFFERSIZE;
	scb->fh_DupBufPtr = 0xfedcba98;

/* we use fh_Func3 as a "buffer may be dirty" flag - actendwrite == dirty */
/* replenish and deplete change it as needed, FGetC/FPutC check it. */
/* actend is a "neutral" setting, to fix Bill whawes */
}

BPTR ASM
findinput (REG(d1) char *string)
{
	return findstream(string,ACTION_FINDINPUT);
}

BPTR ASM
findoutput (REG(d1) char *string)
{
	return findstream(string,ACTION_FINDOUTPUT);
}

/* aka Open() */
/* NOT in GV directly */
/* MUST be safe to call findstream("NIL:"...) from a TASK for createnewproc!! */

BPTR ASM
findstream (REG(d1) char *string,	/* aka Open() */
	    REG(d2) LONG act)
{
	UBYTE *name,*newname;
	struct FileHandle *scb;
	struct MsgPort *task;
	struct DevProc *dp = NULL;
	BPTR lock;
	LONG rc, links = MAX_LINKS;

/*kprintf("Open(\"%s\",%ld) ",string,act);*/

	/* allocate scb and name once, not each time */
	scb = AllocDosObject(DOS_FILEHANDLE,NULL);
	if (!scb)
		return NULL;	/* no space */
/*kprintf("scb = 0x%lx\n",scb);*/

	/* extra +1 for null-termination! */
	name = AllocVecPub(strlen(string)+1+1);
	if (!name)
	{
		freeVec(scb);
		return NULL;
	}
	CtoB(string,(CPTR) name);	/* open wants BSTR! */
	name[*name+1] = '\0';

	while (1) {
		/* init the scb for safety in case handler played with it */
		init_scb((struct FileHandle *) scb,act);

		task = (struct MsgPort *) (lock = NULL);

		/* check for * meaning current window */
		if (strcmp("*",(char *) name+1) == SAME ||
		    mystricmp("CONSOLE:",(char *) name+1) == SAME)
		{
			/* if we have no consoletask, silently send it	*/
			/* to the great bit-bucket in the sky.		*/
			task = consoletask();	/* lock is NULL */
			if (!task)
				goto open_nil;
		} else {
			/* check for resident or built in device */
			if (mystricmp("NIL:",(char *) name+1) == SAME)
			{
open_nil:			/* NIL: works here */
				/* leave fh_Type == 0 */
				scb->fh_Func1=scb->fh_Func2=scb->fh_Func3 = 0;
				goto ret_ok;

			}
			/* we need to really open it */
			/* dp may or may not be NULL here */
			dp = getdevproc((char *) name+1,dp);
			if (dp)
			{
				task = dp->dvp_Port;
				lock = dp->dvp_Lock;
			}
		}

		if (task == NULL)	/* device doesn't exist */
			goto ret_error;

		scb->fh_Type = task;

/*kprintf("sending pkt to handler 0x%lx, lock = 0x%lx, fh = 0x%lx\n",task,lock,
scb);*/
		rc = sendpkt3(task,act,TOBPTR(scb),lock,TOBPTR(name));

/*kprintf("packet returned, rc = %ld, ioerr = %ld\n",rc,getresult2());*/
		if (rc)
			goto ret_ok;

		/* was it a multiple assign? */
		/* FIX!!!!!! support new open modes!!!!!!!! */
		switch (getresult2()) {
		case ERROR_OBJECT_NOT_FOUND:
			if (dp && act != ACTION_FINDOUTPUT &&
			    (dp->dvp_Flags & DVPF_ASSIGN))
			{
				/* it's a multiple assign - try again */
				continue;
			}
			break;
		case ERROR_IS_SOFT_LINK:
			if (--links >= 0)
			{
			  /* for the time being, max 255 chars */
			  /* Bstr is allocated in 256 bytes */
			  newname = AllocVecPub(256);
			  if (!newname)
			    break;

			  BtoCstr((CPTR) name);

			  if (ReadLink(task,lock,name,newname,256) >= 0)
			  {
			    freeVec(name);
			    name = newname;
			    CtoBstr(name);
			    name[*name+1] = '\0';
			    goto loop;
			  } else {
			    /* we can't deal with >255 chars */
			    /* should still be null-terminated ok */
			    CtoBstr(name);
			    freeVec(newname);
			    setresult2(ERROR_INVALID_COMPONENT_NAME);
			  }
			} else {
			  setresult2(ERROR_TOO_MANY_LEVELS);
			}
default:		break;
		}

		/* open really failed */
		if (err_report(REPORT_LOCK,lock,task))
			goto ret_error;

loop:
		/* make sure we free before going around again */
		freedevproc(dp);	/* NULL is ok */
		dp = NULL;		/* so we don't free it twice */

	} /* while (1) */


ret_error:
	freeVec(scb);
	scb = NULL;		/* causes NULL return */
ret_ok:
	freeVec(name);
	freedevproc(dp);	/* NULL is ok */

	return TOBPTR(scb);	/* may be null */
}

/*
// Standard support functions for BCPL rdch and wrch

// Any buffers required are obtained here and released here.

// Initial read. Obtain buffer for rdch

EVIL!!!!! FIX!!!! if used for reading and writing, buffers are lost!!!!!
*/

char * __regargs
actinitcommon (scb)
	struct NewFileHandle *scb;
{
	char *nbuf;

	/* deal with buffer already allocated */
	if (scb->fh_Buf == 0)
	{
		/* BCPL used getvec.  This allocated 4 extra bytes, plus 4
		 * more because of granularity.  WShell counted on all these
		 * bytes being available to use, so the minimum size acceptable
		 * is 204 bytes (SCB_BUFFERSIZE+8).  This is a KLUDGE and may
		 * be removed at some time in the future. - REJ
		 */
		nbuf = AllocVecPub(SCB_BUFFERSIZE+8);
		if (!nbuf)
		{
			setresult2(ERROR_NO_FREE_STORE);
			return FALSE;
		}
		/* fh_BufSize is already set to SCB_BUFFERSIZE */

		/* only need to call init once! */
		scb->fh_Func1 = (LONG) actread;
		scb->fh_Func2 = (LONG) actwrite;

	} else
		return BADDR(scb->fh_Buf);

	/* goddamn cli and shell mess with the buffer! */
	scb->fh_Buf   = TOBUFFER(nbuf);

	/* check magic number to be doubly sure we allocated it! */
	if ((scb->fh_Flags & FHF_EXTEND) &&
	    (scb->fh_DupBufPtr = 0xfedcba98))
	{
		scb->fh_DupBufPtr = scb->fh_Buf;
	}

	return nbuf;
}

LONG __regargs
actreadinit (scb)
	struct NewFileHandle *scb;
{
	if (!actinitcommon(scb))
		return FALSE;

	/* actread sets fh_Pos and fh_End */
	return (actread((struct FileHandle *) scb));
}

/* Subsequent reads.  Refill existing buffer */
/* doesn't modify pos/end if Read() returns EOF */
LONG __regargs
actread (scb)
	struct FileHandle *scb;
{
	LONG len = BufferSize((struct NewFileHandle *) scb);

	/* FIX! use variable buffer size! */
	if (((LONG) scb->fh_Flags) & FHF_NONE)
	{
		/* no buffering, read 1 char at a time */
		len = 1;
	}
	len = read(TOBPTR(scb),BUFFER(scb),len);

	/* this may be redundant */
	if (!len)	/* BCPL used to do this always, I dunno why - FIX */
		setresult2(0);
	else {
		scb->fh_Pos = 0;
		scb->fh_End = len;
	}
	return len > 0;		/* len == 0 means EOF, <0 means error */
}

LONG __regargs
actend (scb)
	struct FileHandle *scb;
{
	struct NewFileHandle *fh = (void *) scb;
	LONG r;
	void *buf = BUFFER(fh);

/*kprintf("closing 0x%lx...",fh);*/
	// make sure we should free the buffer... (SetVBuf)
	// note: if we didn't allocate the fh, or if someone punched fh_Buf,
	// just free fh_buf as we always have.  If we allocated it AND we
	// installed the buffer, AND it's a user buffer, don't free it.
	if (!(fh->fh_Flags & FHF_EXTEND) ||
	    fh->fh_Buf != fh->fh_DupBufPtr ||
	    (buf && !(fh->fh_Flags & FHF_USERBUF)))
	{
		freeVec(buf);	// NULL is ok
	}
	while (1)
	{
		r = sendpkt1(fh->fh_Type,ACTION_END,fh->fh_Arg1);
		if (r || err_report(REPORT_STREAM,TOBPTR(fh),NULL))
			return r;
	}
	/*NOTREACHED*/
}

/* Termination.  Free the buffer and inform handler task */
LONG __regargs
actendread (scb)
	struct FileHandle *scb;
{
	return actend(scb);	/* because we use actendread as a flag! */
}

/* // Initial write. Obtain buffer for wrch */

LONG __regargs
actwriteinit (scb)
	struct NewFileHandle *scb;
{
	if (!actinitcommon(scb))
		return -1;	/* error, ala Write() */

	scb->fh_Pos   = 0;
	scb->fh_End   = BufferSize((struct NewFileHandle *) scb);

	return 1;	/* no bytes written, but 0 means an error! */
}

/* subsequent writes */
LONG __regargs
actwrite (scb)
	struct FileHandle *scb;
{
	LONG rc = 1;

	/* don't do a 0-length write if it can be avoided */
	if (scb->fh_Pos > 0)
	{
		rc = write(TOBPTR(scb),BUFFER(scb),scb->fh_Pos);
		if (rc > 0)
			scb->fh_Pos = 0;	/* FIX! handle partial flush!*/
	}
	return rc;
}

/* Termination.  Write final buffer then act as actendread  */
LONG __regargs
actendwrite(scb)
	struct FileHandle *scb;
{
	if (scb->fh_Pos > 0)	/* is it empty? */
		(void) actwrite(scb);	/* flush output */
	/* used to check if actwrite returned < 0 and not actend it */
	/* we now close it regardless of flush errors */

	return actend(scb);
}

LONG ASM
endread ()
{
	return endstream(input());
}

LONG ASM
endwrite ()
{
	return endstream(output());
}

/* aka Close() */
/* note: preserves result2 if it succeeds! */

LONG ASM
endstream (REG(d1) BPTR scb)
{
	LONG __regargs (*func)(struct FileHandle *);
	LONG res2,rc = DOSTRUE;	/* important! so if func3==0 we free fh */
	struct FileHandle *fh;

	if (!scb)
		return FALSE;
	fh = BADDR(scb);

	res2 = getresult2();
	func = (LONG __regargs (*)()) fh->fh_Func3;

	/* kill those poor people who close twice */
	/* assuming the memory hasn't been re-allocated yet, of course */
	if (((LONG) func) == -1)
		abort(AN_FileReclosed|AT_DeadEnd);

	/* usually (read always) ends up in actendread, which does close */
/*kprintf("closing 0x%lx\n",fh);*/
	if (func)
		rc = (*func)(fh);
/*kprintf("end function returned %ld\n",rc);*/

	fh->fh_Func3 = -1;

	freeVec(fh);
	if (rc)
		setresult2(res2);	/* keep IoErr on failure */

/*else kprintf("Close failed!!!!!!\n");*/

/*kprintf("close done\n");*/
	return rc;
}

LONG ASM
readn ()
{
	LONG sum = 0;
	LONG ch;
	LONG neg = FALSE;

l:	ch = rdch();
	if (!('0' <= ch && ch <= '9'))
	{
		switch (ch) {
		case ' ':
		case '\t':
		case '\n':
			goto l;
		case '-':
			neg = TRUE;
			/* fallthru */
		case '+':
			ch = rdch();
			break;
		default:
			UnGetC(input(),-1);	/* equiv to old unrdch */
			setresult2(-1);
			return 0;
		}
	}
	while ('0' <= ch && ch <= '9')
	{
		sum = 10*sum + (ch - '0');
		ch = rdch();
	}

	if (neg)
		sum = -sum;
	UnGetC(input(),-1);			/* equiv to old unrdch */

	setresult2(0);
	return sum;
}

void ASM
newline ()
{
	wrch('\n');
}

void ASM
writed (REG(d1) LONG n,		/* number */
	REG(d2) LONG d)		/* field? */
{
	char t[40];	/* way too big, but what BCPL uses */
	LONG i = 0;
	LONG k = -n;
	LONG j;

	if (n < 0)			/* make sure k is negative */
	{
		d--;
		k = n;
	}

	do {				/* do-while makes sure a 0 prints */
		k      = divrem32(k,10,&j);	/* j is unused in this loop */
		t[i++] = -(j);
	} while (k != 0);

	for (j = i+1; j <= d; j++)
		wrch(' ');
	if (n < 0)
		wrch('-');
	for (j = i-1; j >= 0; j--)
		wrch(t[j]+'0');
}

void ASM
writen (REG(d1) LONG n)
{
	writed(n,0);
}

void ASM
writehex (REG(d1) LONG n,
	  REG(d2) LONG d)
{
	if (d > 1)
		writehex(n >> 4,d-1);
	n &= 0x0f;
	if (n > 9)
		wrch(n + 'A' - 10);
	else
		wrch(n + '0');
}

void ASM
writeoct (REG(d1) LONG n,
	  REG(d2) LONG d)
{
	if (d > 1)
		writeoct(n >> 3,d-1);
	wrch((n & 0x07) + '0');
}

static void __regargs
writesnum (char *s, LONG num)
{
	short i;

	for (i = 0; i < num; i++)
		wrch(s[i]);
}

void ASM
bwrites (REG(d1) BSTR s)
{
	UBYTE *t;

	t = (UBYTE *) BADDR(s);
	writesnum(t+1,*t);
}

void ASM
writes (REG(d1) UBYTE *s)
{
	writesnum(s,strlen(s));
}

/* this is the externally visible one */
/* RETURNS 0 for ok, -1 for error! */

LONG ASM
FPuts (REG(d1) BPTR fh, REG(d2) UBYTE *str)
{
	while (*str)
		if (FPutC(fh,*str++) == ENDSTREAMCH)
			return -1;

	return 0;	/* no error */
}

/* RETURNS 0 for ok, -1 for error! */

LONG ASM
PutStr (REG(d1) UBYTE *str)
{
	return FPuts(output(),str);
}

void ASM
bwritet (REG(d1) BSTR s,
	 REG(d2) LONG n)
{
	LONG i;
	char *str;

	bwrites(s);

	str = (char *) BADDR(s);
	for (i = 1; i <= n - *str; i++)
		wrch(' ');
}

void ASM
writet (REG(d1) LONG s,		/* really char * */
	REG(d2) LONG n)
{
	char *str = (char *) s;
	LONG i,len;

	len = strlen(str);
	writesnum(str,len);

	for (i = 0; i < n - len; i++)
		wrch(' ');
}

void ASM
writeu (REG(d1) ULONG n,
	REG(d2) LONG d)
{
	ULONG m = udiv32(n,10);

	if (m != 0)
	{
		writed(m,d-1);
		d = 1;
	}
	writed ((LONG) (n-(m * (ULONG)10)),d);
}

void ASM
VFWritef (REG(d1) BPTR fh, REG(d2) UBYTE *format, REG(d3) LONG *argv)
{
	BPTR oldfh = output();

	selectoutput(fh);
	bwritef(format,argv,0);
	selectoutput(oldfh);
}

/* WARNING: VARARGS routine! No registerized params! */

void ASM
bwritef (REG(d1) char *format,
	 REG(d2) LONG *t,
	 REG(d3) LONG isbcpl)
{
	LONG k,len;
	char type;

	/* evil: damn BCPL passes '\0's to force output in strings!  */
	/* the global vector stub merely converts the BPTR to a CPTR */

	if (isbcpl)
	{
		len = *format++;
	} else
		len = strlen(format);

	while (len-- > 0)
	{
		k = *format;
		if (k == '%')
		{
			void (* ASM f)(REG(d1) LONG, REG(d2) LONG);
			LONG n,arg;

			n = 0;
			arg = *t;

			format++;
			len--;
			/* toupper */
			type = (*format >= 'a' && *format <= 'z' ?
				*format + ('A' - 'a') : *format);
			switch (type) {
			default:
				wrch(type);
				break;
			case 'S':
				f = (void (* ASM)(REG(d1) LONG,REG(d2) LONG))
				    (isbcpl ? bwrites : writes);
				goto l;
			case 'T':
				f = (void (* ASM)(REG(d1) LONG,REG(d2) LONG))
				    (isbcpl ? bwritet : writet);
				goto m;
			case 'C':
				f = (void (* ASM)(REG(d1) LONG,REG(d2) LONG))
				    wrch; goto l;
			case 'O':
				f = writeoct; goto m;
			case 'X':
				f = writehex; goto m;
			case 'I':
				f = writed; goto m;
			case 'N':
				f = (void (* ASM)(REG(d1) LONG,REG(d2) LONG))
				    writen; goto l;
			case 'U':
				f = (void (* ASM)(REG(d1) LONG,REG(d2) LONG))
				    writeu; goto m;
m:
				format++;
				len--;
				n = *format;
				n = ('0' <= n && n <= '9' ? n - '0' :
							    n - 'A' + 10);
l:
				(*f)(arg,n);
				/* fall thru */

			case '$':
				t++;	/* next longword */
				/* assumes reasonable stack setup! */
			} /* switch */
		} else 		/* if k == '%' */
			wrch(k);

		format++;	/* len-- is at top */
	} /* while len-- > 0 */
}

/* C version of writef - cstr for format, cstrs for writes and writet */

void __stdargs
writef (format,a)
	char *format;
	LONG a;		/* assumes b,c,d,... are on stack at &a+(N-1) */
{
	bwritef(format,&a,0);
}

/*
// This is a version of i.read for BCPL programmers
// be used by BCPL programs from the CLI
// These must remain UNBUFFERED for BCPL compatibility (argh) REJ
*/

LONG ASM
readbytes (REG(d1) char *v,
	   REG(d2) LONG n)
{
	return read(input(),v,n);
}

LONG ASM
readwords (REG(d1) BPTR v,
	   REG(d2) LONG n)
{
	LONG len;

	len = readbytes((char *) BADDR(v),n << 2);
	return (len > 0 ? len >> 2 : len);
}

/* Buffered external routine */
LONG ASM
FRead (REG(d1) BPTR fh, REG(d2) UBYTE *block, REG(d3) LONG blocklen,
       REG(d4) LONG number)
{
	LONG c,len,num;

	/* FIX!!! should not use single-character buffered reads! */
	num = number;
	while (num > 0)
	{
		len = blocklen;
		while (len-- > 0)
		{
			c = FGetC(fh);
			if (c == ENDSTREAMCH)
				goto eof;
			*block++ = c;
		}
		num--;
	}
eof:
	return number-num;	/* returns actual number of blocks */
}

/*
// This is a version of i.write for BCPL programs
// always be used by BCPL programs from the CLI
*/

LONG ASM
writebytes (REG(d1) char *v,
	    REG(d2) LONG n)
{
	return write(output(),v,n);
}

LONG ASM
writewords (REG(d1) BPTR v,
	    REG(d2) LONG n)
{
	return writebytes((char *) BADDR(v),n << 2);
}

LONG ASM
WriteChars (REG(d1) UBYTE *buffer, REG(d2) LONG buflen)
{
	/* Fix! inefficient (but small) */
	return FWrite(output(),buffer,1,buflen);
}

/* Buffered external routine */
LONG ASM
FWrite (REG(d1) BPTR fh, REG(d2) UBYTE *block, REG(d3) LONG blocklen,
	REG(d4) LONG number)
{
	LONG len,num;

	/* FIX!!! should not use single-character buffered writes! */
	num = number;
	while (num > 0)
	{
		len = blocklen;
		while (len-- > 0)
		{
			if (FPutC(fh,*block++) == ENDSTREAMCH)
				goto write_err;
		}
		num--;
	}
write_err:
	return number-num;	/* returns actual number of blocks written */
}

UBYTE * ASM
FGets (REG(d1) BPTR fh, REG(d2) UBYTE *buf, REG(d3) ULONG len)
{
	UBYTE *save = buf;
	LONG ch;

	/* FIX! set IoErr to 0 */
	/* must be careful to leave space for the null! */
	while (--len)
	{
		ch = FGetC(fh);
		if (ch == ENDSTREAMCH)
		{
			if (buf == save)
				return NULL;
			break;
		}

		*buf++ = ch;

		/* do \n check AFTER adding to buffer */
		if (ch == '\n')
			break;
	}
	*buf = '\0';

	return save;	/* returns buffer or NULL (eof/error) */
}

LONG ASM
VPrintf (REG(d1) UBYTE *fmt, REG(d2) LONG *argv)
{
	return VFPrintf(output(),fmt,argv);
}

LONG ASM
VFPrintf (REG(d1) BPTR fh, REG(d2) UBYTE *fmt, REG(d3) LONG *argv)
{
	LONG args[4];

	args[0] = fh;
	args[1] = 0;	/* number of bytes written */
	args[2] = 0;	/* last character I was asked to write */
	args[3] = 0;	/* error indicator */
	RawDoFmt(fmt,(APTR) argv,(void (*)()) myputch,(APTR) args);

	if (args[3])
		return args[3];	/* return EOF on failure (negative) */

	return args[1]-1;	/* don't count trailing '\0' */
}

LONG ASM
SetVBuf (REG(d1) BPTR fh, REG(d2) UBYTE *buff, REG(d3) LONG type,
	 REG(d4) LONG size)
{
	struct NewFileHandle *scb = BADDR(fh);
	char *old_buff;

/*kprintf("SetVBuf(0x%lx, 0x%lx, %ld, %ld)\n",scb,buff,type,size);*/

	/* for explanation of +8, see kludge info above re: wshell */

	if (type < BUF_LINE || type > BUF_NONE)
	{
		return 1;		/* icky */
	}

	/* set the buffer to a known state */
	Flush(fh);

	/* clear out old mode flags */
	scb->fh_Flags &= ~((1L << BUF_FULL) |
			   (1L << BUF_NONE));

	/* set new mode - default is LINE buffering */
	if (type)
		scb->fh_Flags |= (1L << type);

	/* if size is -1 and buff is NULL only change mode */
	// if fh_Type (port) is NULL then this is a NIL: filehandle, so
	//  silently ignore the call (success).  If we let this go through,
	//  it will set Func3, and actend will be called, and it will try
	//  to send a packet to NULL (very bad).
	if (size != -1 && scb->fh_Type != NULL)
	{
		/* make sure it's an extended filehandle!! */
		if (scb->fh_Flags & FHF_EXTEND)
		{
			/* must check for magic value in DupBufPtr */
			if ((scb->fh_Buf == scb->fh_DupBufPtr) ||
			    (scb->fh_Buf == NULL &&
			     scb->fh_DupBufPtr == 0xfedcba98))
			{
				UBYTE *foo;

				/* kludge for WShell, etc */
				if (size < SCB_BUFFERSIZE+8)
					size = SCB_BUFFERSIZE+8;

				/* ignore setting to same size */
				if (size == scb->fh_BufSize)
					return 0;

				foo = (buff ? buff : (UBYTE *)AllocVecPub(size));
				if (!foo)
					return buff ? 0 : 1; /* ick */

				/* replace buffer ptr */
				old_buff = BADDR(scb->fh_Buf);
				scb->fh_Buf=scb->fh_DupBufPtr = TOBUFFER(foo);

				/* free the old one if we allocated it */
				if (!(scb->fh_Flags & FHF_USERBUF) && old_buff)
					freeVec(old_buff);

				/* reset the userbuffer flags */
				scb->fh_Flags &= ~FHF_USERBUF;
				if (buff)
					scb->fh_Flags |= FHF_USERBUF;

				/* kludge for WShell, etc */
				scb->fh_BufSize = (size == SCB_BUFFERSIZE+8 ?
						   SCB_BUFFERSIZE : size);

				/* must set functions as buffer is alloced */
				scb->fh_Pos   = 0;
				scb->fh_End   = scb->fh_BufSize;
				scb->fh_Func1 = (LONG) actread;
				scb->fh_Func2 = (LONG) actwrite;
				scb->fh_Func3 = (LONG) actendwrite;

			} else {
				/* Someone yanked it from under me!!! */
				/* he'll probably restore the old ptr */
				/* too bad */
				/* FIX! should I return 0 here? */
				return 1; /* ick */
			}
		} else {
			/* can't change size of non-extended FH's */
			return 1;
		}
	} /* else we only wanted to change mode */

	return 0;	/* ick */
}

/* return the buffer size to use for this filehandle */

LONG __regargs
BufferSize (struct NewFileHandle *scb)
{
	LONG size = SCB_BUFFERSIZE;		/* the default */

	/* make sure it's extended, AND that we set it */
	/* unless all of those are true, use "standard" size */
	if ((scb->fh_Flags & FHF_EXTEND) &&
	    scb->fh_Buf == scb->fh_DupBufPtr)
	{
		size = scb->fh_BufSize;
	}
/*
else kprintf("Buffersize: flags $%lx, buffer $%lx, dupbuffer $%lx, size %ld\n",
scb->fh_Flags,BADDR(scb->fh_Buf),BADDR(scb->fh_DupBufPtr),scb->fh_BufSize);
*/
	return size;
}
