/* ---------------------------------------------------------------------------------
 * RSH.C
 *
 * $Locker:  $
 *
 * $Id: rsh.c,v 1.1 92/04/06 14:21:02 dlarson Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: hog:Other/inet/src/c/rcmd/RCS/rsh.c,v 1.1 92/04/06 14:21:02 dlarson Exp $
 *
 *-----------------------------------------------------------------------------------
 */

/* totally rewritten rsh for 2.0.  This version does not do
** stdin redirection.
*/

#define FD_SETSIZE	32

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

#include <stddef.h>
#include <string.h>


/* shared library stuff */
#include <ss/socket.h>

#define RSH_PORT 514		/* XXXX - should getservent */

#define DOSLIB	   "dos.library"
#define DOSVER	   36L

#include "rcmd_rev.h"

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

int rsh (void)
{
	struct DosLibrary *DOSBase;
	struct Library *SockBase ;
	char *host;
	char *remote_user, *local_user;
	int s, cc, num, errno, one=1;
	struct Task *mytask;

	long opts[OPT_COUNT];
	struct RDargs *rdargs;

	u_long readers, rd;
	char buf[1024];

	if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))==NULL) {
		return (10);
	}
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

	s = rcmd(&host, RSH_PORT, local_user, remote_user, (char *)opts[OPT_CMD] , 0);

	if(s < 0){
		PutStr(strerror(errno));
		PutStr("\n");
		goto exit3;
	}

	mytask = FindTask(0L);
	s_ioctl(s, FIONBIO, (char *)&one);

	readers=1<<s;

	/*
	 * Let remote know we're not sending any data.
	 */

	shutdown(s, 1);


	do {
		rd = readers;
		num = select(s+1, (fd_set *)&rd, 0L, 0L, 0L);
		
		if(num < 0 && errno == EINTR) {	 /* someone hit ^C */
			PutStr(strerror(errno));
			PutStr("\n");
			break;
		}

		if(rd&readers) {		 /* stdout reader */
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

	} while(1);

    /* if we got a control C, reset signal and print a nice " ^C" */
	if(mytask->tc_SigRecvd & SIGBREAKF_CTRL_C) {
		SetSignal(0L,SIGBREAKF_CTRL_C);
		PutStr(" ^C\n");
	}

exit3:
	cleanup_sockets();
exit2:
	CloseLibrary(SockBase);
exit1:
	if(rdargs)
		FreeArgs(rdargs);
	CloseLibrary((struct Library *)DOSBase);

}

