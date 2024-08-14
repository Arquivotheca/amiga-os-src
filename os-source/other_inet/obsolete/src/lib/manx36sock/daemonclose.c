/*
 * "close" a socket that will be passed to a daemon.  All we really
 * do is massage the file table.
 */

#include "fd_to_fh.h"
#include <exec/types.h>
#include <exec/memory.h>
#include <inetd.h>
#include <errno.h>

/*
 * "close" a socket that was handed off to a daemon.  We want to close
 * the local nil: device that was opened, but leave open the actual
 * socket.
 */
static
daemonclose(fd)
	int	fd;
{
	if(fd < 0 || fd > NUMFD || !FD_ISSET(fd,&_is_socket)){
		errno = EBADF;		
		return -1;
	}

	FD_CLR(fd, &_is_socket);
	_socks[fd] = 0;
	close(fd);

	return 0;
}

/*
 * Send the inetd startup message to the given port.  Timeout if port
 * doesn't appear after TIMEOUT seconds.  This procedure returns on
 * timeout OR when daemon returns startup message.
 */

daemon_startupmsg(port, s)
	char	*port;
	int	s;
{
	struct MsgPort *msgport, *FindPort(), *CreatePort();
	struct INETDmsg *msg, *rmsg;
	void *AllocMem(), *sock;
	void *GetMsg();
	int to;

	if(s < 0 || s > NUMFD || !FD_ISSET(s, &_is_socket)){
		errno = EBADF;
		return -1;
	}

	msg = (struct INETDmsg *)AllocMem(sizeof(*msg), MEMF_PUBLIC|MEMF_CLEAR);
	if(!msg){
		errno = ENOMEM;
		close(s);
		return -1;
	}

	msg->in_sock = _socks[s];
	msg->in_errno = 0;
	msg->in_msg.mn_Node.ln_Type = NT_MESSAGE;
	msg->in_msg.mn_ReplyPort = CreatePort(0L, 0L);
	if(!msg->in_msg.mn_ReplyPort){
		errno = ENOMEM;
		close(s);
		FreeMem(msg, sizeof(*msg));
		return -1;
	}

	for(to = INETD_TIMEOUT; to > 0; to -= INETD_RETRY){
		Forbid();
		if(msgport = FindPort(port)){
			break;
		}
		Permit();
		Delay(INETD_RETRY);
	}

	if(msgport){
		PutMsg(msgport, msg);
		Permit();
		for(to = INETD_TIMEOUT; to > 0; to -= INETD_RETRY){
			rmsg = (struct INETDmsg *)
					GetMsg(msg->in_msg.mn_ReplyPort);
			if(rmsg == msg){
				daemonclose(s);
				errno = msg->in_errno;
				DeletePort(msg->in_msg.mn_ReplyPort);
				FreeMem(msg, sizeof(*msg));
				return 0;
			}
			Delay(INETD_RETRY);
		}
	}

	/* 
	 * leave port+msg allocated if get here since server may get unhung
	 * much later, eg if it is waiting for requester and user isn't
	 * around to release it.
	 */
	close(s);
	errno = ETIMEDOUT;
	return -1;	
}

