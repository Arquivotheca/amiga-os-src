/* -----------------------------------------------------------------------
 * _myexit.c  manx36 for inetd
 *
 * $Locker:  $
 *
 * $Id: _myexit.c,v 1.1 90/11/12 17:08:28 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/serv/inetd/RCS/_myexit.c,v 1.1 90/11/12 17:08:28 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

/* this is for PORTMAPD & Manx 3.6 *only* !!!!!!  */
/* Does *NOT* close fd #2 as that is already closed  */

#include <fcntl.h>

extern int _argc, _arg_len;
extern char **_argv, *_arg_lin;
extern void *MathBase, *MathTransBase;
extern void *MathIeeeDoubBasBase, *MathIeeeDoubTransBase;
extern long _detach_curdir;
extern void *_oldtrap, **_trapaddr;
extern struct MsgPort *tagport ;

_exit(code)
{
	long ret = code;
	register int fd;

	if( tagport ) 
		DeletePort( tagport ) ;

	if (_devtab) {
		for (fd = 0 ; fd < _numdev ; fd++) {
			if( fd != 2) {
				close(fd) ;        /** <<-- NOTE!!  */
			}
		}
		_FreeMem(_devtab, _numdev*(long)sizeof(struct _dev));
	}
	if (_detach_curdir)				/* for DETACHed programs */
		UnLock(_detach_curdir);
	if (_trapaddr)					/* clean up signal handling */
		*_trapaddr = _oldtrap;
	if (MathTransBase)
		_CloseLibrary(MathTransBase);
	if (MathBase)
		_CloseLibrary(MathBase);
	if (MathIeeeDoubBasBase)
		_CloseLibrary(MathIeeeDoubBasBase);
	if (MathIeeeDoubTransBase)
		_CloseLibrary(MathIeeeDoubTransBase);
	{
#asm
	mc68881
	move.l	4,a6				;get ExecBase
	btst.b	#4,$129(a6)			;check for 68881 flag in AttnFlags
	beq		1$					;skip if not
	move.l	a5,-(sp)
	lea		2$,a5
	jsr		-30(a6)				;do it in supervisor mode
	move.l	(sp)+,a5
	bra		1$
2$
	clr.l	-(sp)
	frestore (sp)+				;reset the ffp stuff
	rte							;and return
1$
#endasm
	}
	if (_arg_lin) {
		_FreeMem(_arg_lin, (long)_arg_len);
		_FreeMem(_argv, (long)(_argc+1)*sizeof(*_argv));
	}
	{
#asm
		move.l	-4(a5),d0		;pick up return exit code
		move.l	__savsp#,sp		;get back original stack pointer
		rts						;and exit
#endasm
	}
}

