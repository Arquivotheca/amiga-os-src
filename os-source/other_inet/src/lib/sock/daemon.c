/*
 * daemon_start(fd, argcp, argvp) - used in daemons that are started from
 * inetd in order to setup socket bound to stdin.  The daemon startup
 * message format is given in inetd.h.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include "fd_to_fh.h"
#include <socket.h>
#include <stdio.h>
#include <inetd.h>

/*
 * Bind fd to socket specified by startup msg.  If specified fd == 0, then
 * stdin/stdout are setup to use socket.
 */
daemon_start(fd, argcp, argvp)
	int	*argcp;
	char	***argvp;
{
	extern int socket_sigio, socket_sigurg;
	struct MsgPort *myport, *CreatePort();
	struct inheritargs ia;
	struct INETDmsg *msg;
	int argc, s, to;
	void *GetMsg();
	char **argv;

	close(fd);
#ifndef LATTICE
	{
		struct _dev *refp;

		refp = _devtab + fd;
		refp->mode = O_RDWR;
		refp->fd = Open("nil:", MODE_OLDFILE);
		if(fd == 0){
			Cbuffs[0]._flags = Cbuffs[1]._flags = _BUSY;
			Cbuffs[0]._unit = Cbuffs[1]._unit = 0;
		}
	}
#endif	

	/*
	 * Open datagram socket to force socket library initialization
	 */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s < 0){
		perror("could not init daemon");
		return -1;
	}

	argc = *argcp; 	argv = *argvp;
	if(argc <= 1 || !argv[1]){
		close(s);
		errno = EINVAL;
		return -1;
	}

	myport = CreatePort(argv[1], 0);
	if(!myport){
		close(s);
		errno = ENOMEM;
		return -1;
	}

	to = INETD_TIMEOUT; 
	while(to > 0){
		Delay(INETD_RETRY);

		/*
		 * To avoid a race condition, we check the port for a message
		 * and delete the port within a Forbid()/Permit block.
		 */
		Forbid();
		if(msg = (struct INETDmsg *)GetMsg(myport)){
			DeletePort(myport);
			Permit();
			break;
		}
		if((to -= INETD_RETRY) <= 0){
			DeletePort(myport);
		}
		Permit();
	}

	if(!msg){
		errno = ETIMEDOUT;
		return -1;
	}

	ia.sigio = socket_sigio;
	ia.sigurg = socket_sigurg;
	ia.fp = (void *)msg->in_sock;
	ia.errno = 0;
	InheritAsm(&ia);
	if(ia.errno){
		msg->in_errno = errno = ia.errno;
		ReplyMsg(msg);
		close(s);
		return -1;
	}

	PUTSOCK(fd, (struct socket *)msg->in_sock);

	daemon_init(msg);
	ReplyMsg(msg);

	close(s);
	*argcp = --argc; *argvp = ++argv;
	return 0;
}

