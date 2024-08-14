/* -----------------------------------------------------------------------
 * _main.c for RSHD (Manx 36 ONLY)
 *
 * $Locker:  $
 *
 * $Id: _main.c,v 1.5 90/11/26 15:53:26 bj Exp $
 *
 * $Revision: 1.5 $
 *
 * $Header: HOG:Other/inet/src/serv/rshd/RCS/_main.c,v 1.5 90/11/26 15:53:26 bj Exp $
 *
 * $Log:	_main.c,v $
 * Revision 1.5  90/11/26  15:53:26  bj
 * Backtracked to older version.
 * 
 * Revision 1.4  90/11/20  17:20:38  bj
 * *** empty log message ***
 * 
 *
 *------------------------------------------------------------------------
 */

/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

/*
 *	This is common startup code for both the CLI and the WorkBench.
 *	When called from the WorkBench, argc is 0 and argv points to a
 *	WBStartup type of structure.
 */

#include <fcntl.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <functions.h>

extern long _savsp, _stkbase;

extern int errno;
extern int Enable_Abort;

extern int _argc, _arg_len;
extern char **_argv, *_arg_lin;
extern struct WBStartup *WBenchMsg;

extern struct _dev *_devtab;
extern short _numdev;

_main(alen, aptr)
long alen;
char *aptr;
{
	register struct Process *pp, *_FindTask();
	void *_OpenLibrary(), *_GetMsg(), *_AllocMem();
	long _Input(), _Output(), _Open();

#ifdef DETACH
	void do_detach();

	do_detach(&alen, &aptr);
#endif

	if ((_devtab = _AllocMem(_numdev*(long)sizeof(struct _dev),
													MEMF_CLEAR)) == 0) {
		Alert(AG_NoMemory, 0L);
#asm
		move.l	__savsp,sp		;get back original stack pointer
		rts						;and exit
#endasm
	}

	_devtab[0].mode = O_RDONLY;
	_devtab[1].mode = _devtab[2].mode = O_WRONLY;

	_stkbase = _savsp - *((long *)_savsp+1) + 8;
	*(long *)_stkbase = 0x4d414e58L;

	pp = _FindTask(0L);
#ifdef DETACH
	if (alen) {
#else
	if (pp->pr_CLI) {
#endif
		_cli_parse(pp, alen, aptr);
		Enable_Abort = 1;
#ifndef DETACH
		_devtab[0].mode |= O_STDIO;		/* shouldn't close if CLI */
		_devtab[1].mode |= O_STDIO;
#endif
	}
	else {
		_WaitPort(&pp->pr_MsgPort);
		WBenchMsg = _GetMsg(&pp->pr_MsgPort);
		if (WBenchMsg->sm_ArgList)
			_CurrentDir(WBenchMsg->sm_ArgList->wa_Lock);
		_wb_parse(pp, WBenchMsg);
		_argv = (char **)WBenchMsg;
	}
	_devtab[0].fd = _Input();
	_devtab[1].fd = _Output();
	_devtab[2].fd = _Output();

/*	if (_devtab[1].fd = _Output())
		_devtab[2].fd = _Open("*", MODE_OLDFILE);
*/

	main(_argc, _argv);
	exit(0);
}

#ifdef DETACH
extern long _stack, _priority, _BackGroundIO;
extern char *_procname;
BPTR _Backstdout = 0;
extern struct FileLock *_detach_curdir;
extern char *_detach_name;
static long _alen = 0;
static char *_aptr = 0;

static void
do_detach(alen, aptr)
long *alen;
char **aptr;
{
	register struct Process *pp, *_FindTask();
	void *sav, *_OpenLibrary(), *_GetMsg(), *_AllocMem();
	long _Open();
	register unsigned short c;
	register char *cp;
	register struct CommandLineInterface *cli;
	register long l;
	long *lp;
	struct MemList *mm;
	struct FileLock *CurrentDir();

	pp = _FindTask(0L);
	if (pp->pr_CLI) {			/* first time through!! */
		CurrentDir(_detach_curdir = CurrentDir(0L));
		_detach_curdir = DupLock(_detach_curdir);

		cli = (struct CommandLineInterface *) ((long)pp->pr_CLI << 2);
		l = cli->cli_Module;
		if ((sav = _OpenLibrary(DOSNAME, 33L)) == 0) {

			lp = (long *)*((long *)*((long *)*((long *)*((long *)
											_savsp+2)+1)-3)-3)+107;
			if (*lp != cli->cli_Module)
				exit(100);
		}
		else {
			_CloseLibrary(sav);
			lp = 0;
		}
		if (lp)
			*lp = 0;
		if (_stack == 0)
			_stack = cli->cli_DefaultStack * 4;
		if (_BackGroundIO)
			_Backstdout = (BPTR)_Open("*", MODE_OLDFILE);
		_alen = *alen;
		_aptr = _AllocMem(_alen, 0L);
		movmem(*aptr, _aptr, (int)_alen);
		cp = (char *)((long)cli->cli_CommandName << 2);
		_detach_name = AllocMem((long)cp[0]+1, 0L);
		movmem(cp, _detach_name, cp[0]+1);
#asm
		move.l	__savsp,-(sp)
#endasm
		CreateProc(_procname, _priority, l, _stack);
		cli->cli_Module = 0;
#asm
		move.l	(sp)+,sp
		move.l	#0,d0
		rts
#endasm
	}
	else if (strcmp(pp->pr_Task.tc_Node.ln_Name, _procname) == 0) { /* second time */
		lp = (long *)((long)pp->pr_SegList << 2);
		lp = (long *)(lp[3] << 2);
		sav = lp;
		c = 2;
		while (lp) {
			lp = (long *)(*lp << 2);
			c++;
		}
		mm = _AllocMem((long)sizeof(struct MemList)+
							(c-1)*sizeof(struct MemEntry), 0L);
		lp = sav;
		mm->ml_NumEntries = c;
		c = 0;
		while (lp) {
			mm->ml_me[c].me_Addr = (APTR)lp - 1;
			mm->ml_me[c].me_Length = lp[-1];
			lp = (long *)(*lp << 2);
			c++;
		}
		mm->ml_me[c].me_Addr = (APTR)_aptr;
		mm->ml_me[c++].me_Length = _alen;
		mm->ml_me[c].me_Addr = (APTR)_detach_name;
		mm->ml_me[c++].me_Length = _detach_name[0] + 1;

		AddTail(&((struct Task *)pp)->tc_MemEntry, mm);

		CurrentDir(_detach_curdir);

		pp->pr_COS = _Backstdout;

		*alen = _alen;
		*aptr = _aptr;
	}
}
#endif

