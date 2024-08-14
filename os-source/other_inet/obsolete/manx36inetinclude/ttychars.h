/*
** ttychars.h - emulate Berkeley ttychars structs.  Mostly psyched out
**		from telnet/sys_bsd.c
*/

#ifndef SYS_TTYCHARS_H
#define SYS_TTYCHARS_H

struct ltchars {
	char	t_flushc;
	char	t_dsuspc;
	char	t_suspc;
	char	t_brkc;
	char	t_intrc;
	char	t_quitc;
};

struct tchars {
	char	t_flushc;
	char	t_dsuspc;
	char	t_suspc;
	char	t_brkc;
	char	t_intrc;
	char	t_quitc;
	char	t_startc;
	char	t_stopc;
	char	t_syspc;
	char	t_eofc;
};

#endif
