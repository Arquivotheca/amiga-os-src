/*
 *  $Header$
 */
/*
 * Amiga specific routines
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <socket.h>

#include "ring.h"
#include "defines.h"
#include "externs.h"
#include "types.h"

#include <libraries/dos.h>

static fd_set ibits, obits, xbits;
static console_up;
int net;
void *IntuitionBase;

memset(buf, val, len)
	register char *buf;
	register char val;
	register int len;
{
	while(len--){
		*buf++ = val;
	}
}

memcpy(to, from, len)
	char	*to, *from;
	int	len;
{
	bcopy(from, to, len);
}

TerminalSpecialChars(c)
	char	c;
{
}

/*
 * netin	- if set, include net input in select
 * netout	- if set, include net output in select
 * netin	- if set, include net exceptions in select
 * ttyin	- if set, include tty input in event mask
 * ttyout	- if set, include tty output in event mask
 */
process_rings(netin, netout, netex, ttyin, ttyout, poll)
	int	netin, netout, netex;
	int	ttyin, ttyout;
	int	poll;
{
    	static struct timeval TimeValue = { 0 };
	extern int cons_kybd_sigF;
	struct timeval *tvp;
    	int returnValue = 0;
	register int c;
	long event;

	event = 0L;
    	if (netout) {
		FD_SET(net, &obits);
    	} 
    	if (netin) {
		FD_SET(net, &ibits);
    	}
    	if (netex) {
		FD_SET(net, &xbits);
    	}
/*	if(ttyin){ */
		event |= cons_kybd_sigF;
/*	}*/

	tvp = (poll == 0) ? 0 : &TimeValue;
	event |= SIGBREAKF_CTRL_C;
    	c = selectwait(net+1, &ibits, &obits, &xbits, tvp, &event);

    	/*
     	 * Any urgent data?
     	 */
    	if (FD_ISSET(net, &xbits)) {
		FD_CLR(net, &xbits);
		SYNCHing = 1;
		ttyflush(1);	/* flush already enqueued data */
    	}

    	/*
     	 * Something to read from the network...
     	 */
    	if (FD_ISSET(net, &ibits)) {
		int canread;

		FD_CLR(net, &ibits);
		canread = ring_empty_consecutive(&netiring);
		c = recv(net, netiring.supply, canread, 0);
		if (c < 0 && errno == EWOULDBLOCK) {
	    		c = 0;
		} else if (c <= 0) {
	    		return -1;
		}
		if(netdata) {
	    		Dump('<', netiring.supply, c);
		}
		if(c){
	    		ring_supplied(&netiring, c);
		}

		returnValue = 1;
    	}

    	/*
     	 * Something to read from the tty...
     	 */
	if(event & cons_kybd_sigF){
		c = TerminalRead(ttyiring.supply, 
					ring_empty_consecutive(&ttyiring));
		if (c < 0){
	    		c = 0;
		} else {
		    	if((c == 0) && MODE_LOCAL_CHARS(globalmode)){
				*ttyiring.supply = termEofChar;
				c = 1;
		    	}
		    	if (c <= 0) {
				return -1;
		    	}
		    	ring_supplied(&ttyiring, c);
		}
		returnValue = 1;		/* did something useful */
    	}

	returnValue |= ttyflush(0);

    	if (FD_ISSET(net, &obits)) {
		FD_CLR(net, &obits);
		returnValue |= netflush();
    	}

    	return returnValue;
}

void
NetNonblockingIO(fd, onoff)
	int	fd, onoff;
{
    	ioctl(fd, FIONBIO, (char *)&onoff);
}

void
sys_telnet_init()
{
    	setconnmode();
    	NetNonblockingIO(net, 1);
    	if(SetSockOpt(net, SOL_SOCKET, SO_OOBINLINE, 1) == -1) {
		perror("SetSockOpt");
    	}
}

void
TerminalSaveState()
{
}

NetClose(fd)
{
	return close(fd);
}

init_sys()
{
	if(!console_up){
		console_init("telnet");
		console_up++;
	}
    	FD_ZERO(&ibits);
    	FD_ZERO(&obits);
    	FD_ZERO(&xbits);
    	errno = 0;
}

TerminalAutoFlush()
{
}

void
TerminalFlushOutput()
{
}

TerminalWrite(buf, n)
	char	*buf;
	int	n;
{
	console_write(buf, n);
	return n;
}

TerminalRead(buf, n)
	char	*buf;
	int	n;
{
	char c;

	*buf = console_getchar();
	return 1;
}

void
TerminalNewMode(mode)
	int	mode;
{
}

printf(fmt, a, b, c, d, e, f, g)
	char	*fmt;
	unsigned a, b, c, d, e, f, g;
{
	static char buf[2048];
	int rtn;

	rtn = sprintf(buf, fmt, a, b, c, d, e, f, g);
	TerminalWrite(buf, strlen(buf));
	return rtn;
}

char *
gets(buf)
	char	*buf;
{
	char 	*p;

	for(p = buf; ; p++){
		TerminalRead(p, 1);
		if(*p == '\r' || *p == '\n'){
			TerminalWrite("\r\n", 2);
			break;
		} else {
			TerminalWrite(p, 1);
		}
	}
	*p = 0;
	return buf;	
}

void
init_3270()
{
}

MyExit(code)
{
	if(console_up){
		console_up = 0;
		console_close();
	}
	exit(code);
}

SetIn3270()
{
}

main(argc, argv)
	char	**argv;
{
	return tmain(argc, argv);
}

