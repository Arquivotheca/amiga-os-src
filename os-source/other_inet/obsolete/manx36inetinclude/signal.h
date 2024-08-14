/*
** Some signal definitions for our emulator.  
*/

#ifndef SIGNAL_H
#define SIGNAL_H

#define SIGINT	1
#define SIGTERM	2
#define SIGABRT	3
#define SIGFPE	4
#define SIGILL	5
#define SIGSEGV	6
#define SIGPIPE	7
#define SIGQUIT	8
#define SIGKILL	9
#define SIGALRM	10
#define SIGHUP	11
#define SIGCHLD	12
#define SIGSYS	13
#define SIGIOT	14
#define SIGEMT	15
#define SIGUSR1	16
#define SIGUSR2	17
#define SIGURG	18
#define SIGIO	19
#define SIGTSTP	20

#define _NUMSIG	20
#define _FSTSIG	1

#define SIG_DFL	((void (*)())0)
#define SIG_IGN	((void (*)())1)
#define SIG_ERR	((void (*)())-1)
#define BADSIG	((void (*)())-1)

extern int (*signal())();
extern int raise();

#endif
