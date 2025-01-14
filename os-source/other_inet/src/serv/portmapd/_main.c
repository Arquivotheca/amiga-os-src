/* -----------------------------------------------------------------------
 * _main.c   portmapd  manx36
 *
 * $Locker$
 *
 * $Id$
 *
 * $Revision$
 *
 * $Header$
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
	APTR _DeviceProc() ;
	long _Input(), _Output(), _Open() ;
    
#ifdef DEBUG    
	BPTR win_fh = NULL ;
    char msg_buffer[256] ;
    int isout = 1 ;
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
	if (pp->pr_CLI) {
		_cli_parse(pp, alen, aptr);
		Enable_Abort = 1;

		_devtab[0].mode |= O_STDIO;		/* shouldn't close if CLI */
		_devtab[1].mode |= O_STDIO;
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
	if (_devtab[1].fd = _Output())
		_devtab[2].fd = _Output() ;
		
#ifdef DEBUG
	else isout = 0 ;	
	win_fh = _Open("CON:0/20/400/100/PORTMAPD MESSAGES:", MODE_NEWFILE) ;	
	if( win_fh) {
		if( isout == 0 ) Write( win_fh , "OUTPUT = NULL!!!!\N",18L ) ; 
		sprintf( msg_buffer, "in = %08x  out = %08x err = %08x\n",
							_devtab[0].fd, _devtab[1].fd, _devtab[2].fd ) ;
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
		sprintf( msg_buffer, "pp->pr_COS = %08x\n", pp->pr_COS) ;		
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
		sprintf( msg_buffer, "pp->pr_CIS = %08x\n", pp->pr_CIS) ;		
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
		sprintf( msg_buffer, "devtab[0].fd = %08x   devtab[0].mode = %08x\n", 
							_devtab[0].fd,_devtab[0].mode ) ;
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
		sprintf( msg_buffer, "devtab[1].fd = %08x   devtab[0].mode = %08x\n", 
							_devtab[1].fd,_devtab[1].mode ) ;
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
		sprintf( msg_buffer, "devtab[2].fd = %08x   devtab[0].mode = %08x\n", 
							_devtab[2].fd,_devtab[2].mode ) ;
		Write( win_fh, msg_buffer, (long)strlen(msg_buffer)) ;	
        }        
#endif
	
	main(_argc, _argv);
#ifdef	DEBUG
	if( win_fh )
		_Close( win_fh ) ;
#endif		
	exit(0);
}

