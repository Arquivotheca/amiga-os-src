/*
** Some signal definitions most will do nothing on the Amiga
** If your compiler has a signal function, you will want to 
** check these values for compatibility.  
*/

#ifndef SIGNAL_H
#define SIGNAL_H

#define SIGINT	2	/* Lattice needs SIGINT to be 1 */
#define SIGTERM	1
#define SIGABRT	3
#define SIGFPE	8	/* Lattice */
#define SIGILL	5
#define SIGSEGV	6
#define SIGPIPE	7
#define SIGQUIT	4
#define SIGKILL	9
#define SIGALRM	10
#define SIGHUP	11
#define SIGCHLD	12
#define SIGSYS	13
#define SIGIOT	14
#define SIGEMT	15
#define SIGUSR1	16
#define SIGUSR2	17
#define SIGURG	18	/* used by socket library */
#define SIGIO	19      /* used by socket library */
#define SIGTSTP	20

#define _NUMSIG	20
#define _FSTSIG	1

#define SIG_DFL	((void (*)(int))0)
#define SIG_IGN	((void (*)(int))1)
#define SIG_ERR	((void (*)(int))-1)
#define BADSIG	((void (*)(int))-1)

void (*signal(int, void (*)(int)))(int);

#endif
