/*
** Structures used to communicate between inet.library and the user.
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/time.h>

struct sockargs {
	short	domain;
	short	type;
	short	protocol;
	char	sigurg;
	char	sigio;
	short	errno;
	void 	*rval;
};

struct inheritargs {
	void	*fp;
	char	sigurg, sigio;
	short	errno;
};

struct closeargs {
	void 	*fp;
	short	errno;
};

struct bindargs {
	void 	*fp;
	caddr_t	name;
	short	namelen;
	short	errno;
};

struct listenargs {
	void 	*fp;
	short	backlog;
	short	errno;
};

struct acceptargs {
	void 	*fp;
	caddr_t	name;
	short	namelen;
	short	errno;
	void 	*rval;
};

struct connectargs {
	void 	*fp;
	caddr_t	name;
	short	namelen;
	short	errno;
};

struct sendtoargs {
	void 	*fp;
	caddr_t	buf;
	long	len;
	short	flags;
	caddr_t	to;
	short	tolen;
	short	errno;
	long	rval;
};

struct sendargs {
	void	*fp;
	caddr_t	buf;
	long	len;
	short	flags;
	short	errno;
	long	rval;
};

struct sendmsgargs {
	void	*fp;
	caddr_t	msg;
	short	flags;
	short	errno;
	long	rval;
};

struct recvfromargs {
	void	*fp;
	caddr_t	buf;
	long	len;
	short	flags;
	caddr_t	from;
	short	fromlen;
	short	errno;
	long	rval;
};

struct recvargs {
	void	*fp;
	caddr_t	buf;
	long	len;
	short	flags;
	short	errno;
	long	rval;
};

struct recvmsgargs {
	void	*fp;
	struct msghdr *msg;
	short	flags;
	short	errno;
	long	rval;
};

struct shutdownargs {
	void	*fp;
	short	how;
	short	errno;
};

struct setsockoptargs {
	void	*fp;
	short	level;
	short	name;
	caddr_t	val;
	short	valsize;
	short	errno;
};

struct getsockoptargs {
	void	*fp;
	short	level;
	short	name;
	caddr_t	val;
	short	valsize;
	short	errno;
};

struct getsocknameargs {
	void	*fp;
	caddr_t	asa;
	short	len;
	short	errno;
};

struct getpeernameargs {
	void	*fp;
	caddr_t	asa;
	short	len;
	short	errno;
};

struct ioctlargs {
	void	*fp;
	short	cmd;
	caddr_t	data;
	short	errno;
};

struct selectargs {
	void	**socks;		/* array of socket ptrs		*/
	fd_set	*rd_set;		/* socks considered for rd evnt */
	fd_set	*wr_set;		/* socks considered for wr evnt */
	fd_set	*ex_set;		/* socks considered for ex evnts*/
	struct Task *task;		/* task to signal when selecting*/
	short	sigbit;			/* signal to send when selecting*/
	short	numfds;			/* number of fds to consider	*/
	short	errno;			/* return error value		*/
	short	rval;			/* number of fds found ready	*/
};

#endif
