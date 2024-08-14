/* ---------------------------------------------------------------------------------
 * RSH.C
 *
 * $Locker:  $
 *
 * $Id: rsh.c,v 1.1 92/04/06 14:19:55 dlarson Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: hog:Other/inet/src/c/rsh/RCS/rsh.c,v 1.1 92/04/06 14:19:55 dlarson Exp $
 *
 *-----------------------------------------------------------------------------------
 */

#include "aio.h"
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>


/* shared library stuff */
#include <ss/socket.h>

#define RSH_PORT 514		/* XXXX - should getservent */

#define DOSLIB	   "dos.library"
#define DOSVER	   36L

#include "rsh_rev.h"

/* This is used with the templates to cause the version string to be */
/* stored in the command.  It depends upon the command having a #include */
/* of the appropriate version file to provide the value for this string */
#define CMDREV  "\0$VER: " VSTRING

#define bzero(a,n)	memset(a,'\0',n)

#define TEMPLATE    "HOST/A,-L/K,CMD/F" CMDREV
#define OPT_HOST    0
#define OPT_L		1
#define OPT_CMD     2
#define OPT_COUNT   3


int main()
{
	long opts[OPT_COUNT];
	struct RDargs *rdargs;

	register struct Library *SockBase ;
	char *host;
	char *remote_user, *local_user;
	int s, cc, rdfd2, num, errno, one;
	struct Task *mytask;
	u_char in_char[256];
	register AFH *in_fh;
	LONG input_signal; 
	LONG insig;

	fd_set rd;
	char buf[1024];

	memset((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts, NULL);

	if ((SockBase = OpenLibrary( "inet:libs/socket.library", 1L ))==NULL) {
		PutStr("Opening of socket library failed.\n");
		goto exit1;
	}

	
	setup_sockets( 5, &errno );

	local_user=getlogin();

	if(opts[OPT_CMD]==NULL)
		goto exit3;

	if(opts[OPT_L]) 
		remote_user = (char *)opts[OPT_L];
	else
		remote_user = local_user;

	if(opts[OPT_HOST])
		host = (char *)opts[OPT_HOST];
	else {
		PutStr ("usage: rsh host [ -l username ] command\n"); 
		goto exit3;
	}

	s = rcmd(&host, RSH_PORT, local_user, remote_user, (char *)opts[OPT_CMD] , &rdfd2);


	if(s < 0){
		PutStr(strerror(errno));
		PutStr("\n");
		goto exit3;
	}

	one = 1 ;
	mytask = FindTask(0L);
	s_ioctl(rdfd2, FIONBIO, &one);
	s_ioctl(s, FIONBIO, &one);

	/* set up input signals */
	if((in_fh=AOpen("console:",MODE_READWRITE))==NULL) {
		PutStr("Error opening console\n");
		goto exit3;
	}
	input_signal = 1 << (LONG)ASignal(in_fh);
	ARead(in_fh,in_char,1);


	do {
		FD_ZERO(&rd);
		FD_SET(s, &rd);
		FD_SET(rdfd2, &rd);

		insig = input_signal;

		num = selectwait(MAX(rdfd2, s)+1, &rd, 0L, 0L, 0L, &insig);

		if(FD_ISSET(rdfd2, &rd)) {	 /* stderr reader */
			do {
				cc = recv(rdfd2, buf, sizeof(buf), 0);
				if(cc > 0){
                	WriteChars(buf, cc);
				}
			} while(cc > 0);
			if(cc == 0){
				break;
			}
		}

		if(FD_ISSET(s, &rd)) {		 /* stdout reader */
			do {
				cc = recv(s, buf, sizeof(buf), 0);
				if(cc > 0){
					WriteChars(buf, cc);
				}
			} while(!(mytask->tc_SigRecvd & SIGBREAKF_CTRL_C) && cc > 0);
			if(cc == 0){
				break;
			}
		}

		if(insig & input_signal) {
			while(GetMsg(in_fh->rp)) {	/* read all the messages */
				if( ARes1(in_fh) == 1) {
					Write(Output(),in_char,1);
					send(s,in_char,1,0);
					ARead(in_fh,in_char,1); /* queue another read request */
				} 
			}
		} else {
			if(num < 0 && errno == EINTR) {	   /* someone hit ^C */
				PutStr(strerror(errno));
				PutStr("\n");
				break;
			}
		}

	} while(1);

    /* if we got a control C, reset signal and print a nice " ^C" */
	if(mytask->tc_SigRecvd & SIGBREAKF_CTRL_C) {
		SetSignal(0L,SIGBREAKF_CTRL_C);
		PutStr(" ^C\n");
	}

	AClose(in_fh);
exit3:
	cleanup_sockets();
exit2:
	CloseLibrary(SockBase);
exit1:
	if(rdargs)
		FreeArgs(rdargs);
}

