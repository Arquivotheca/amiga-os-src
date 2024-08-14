/* -----------------------------------------------------------------------
 * _mymain.c  inetd  manx36
 *
 * $Locker:  $
 *
 * $Id: _mymain.c,v 1.2 91/04/26 16:42:14 dlarson Exp $
 *
 * $Revision: 1.2 $
 *
 * $Header: HOGNET:src/serv/inetd/RCS/_mymain.c,v 1.2 91/04/26 16:42:14 dlarson Exp $
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

APTR old_pr_WindowPtr;

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
	old_pr_WindowPtr = pp->pr_WindowPtr;
	pp->pr_WindowPtr = -1;

	if (pp->pr_CLI) {
		_cli_parse(pp, alen, aptr);
		Enable_Abort = 0 ;

		_devtab[0].mode |= O_STDIO;		/* shouldn't close if CLI */
		_devtab[1].mode |= O_STDIO;

	}
	else {
		exit(5) ;  /* not from wb ! */
	}
	_devtab[0].fd = _Input();
	if (_devtab[1].fd = _Output())
/*		_devtab[2].fd = _Open("*", MODE_OLDFILE) ;  */
		_devtab[2].fd = _Output() ;
	main(_argc, _argv);
	exit(0);
}

