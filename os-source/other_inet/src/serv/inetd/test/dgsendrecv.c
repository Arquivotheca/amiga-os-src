/*
 * Send a datagram to a server, and read a response.
 * Establish a timer and resend as necessary.
 * This function is intended for those applications that send a datagram
 * and expect a response.
 * Returns actual size of received datagram, or -1 if error or no response.
 */

#include <stdio.h>
#include <ss/socket.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<signal.h>
extern int	errno;

int
dgsendrecv(fd, outbuff, outbytes, inbuff, inbytes, destaddr, destlen)
int		fd;		/* datagram socket */
char		*outbuff;	/* pointer to buffer to send */
int		outbytes;	/* #bytes to send */
char		*inbuff;	/* pointer to buffer to receive into */
int		inbytes;	/* max #bytes to receive */
struct sockaddr *destaddr;	/* destination address */
				/* can be 0, if datagram socket is connect'ed */
int		destlen;	/* sizeof(destaddr) */
{
	int	n;
	/*
	 * Send the datagram.
	 */

	if (sendto(fd, outbuff, outbytes, 0, destaddr, destlen) != outbytes) {
		fprintf(stderr,"dgsendrecv: sendto error on socket");
		return(-1);
	}


	n = recvfrom(fd, inbuff, inbytes, 0,
			(struct sockaddr *) 0, (int *) 0);
	if (n < 0) {
			fprintf(stderr,"dgsendrecv: no response from server");
	}

	return(n);		/* return size of received datagram */
}

