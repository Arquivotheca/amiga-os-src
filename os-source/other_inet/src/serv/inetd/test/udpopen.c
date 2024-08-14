/*
 * Establish a UDP socket and optionally call connect() to set up
 * the server's address for future I/O.
 */

#include <ss/socket.h>
#include <netdb.h>
#include	"netdefs.h"

#include	<netinet/in.h>
#include	<arpa/inet.h>
#include <errno.h>
#include <string.h>

#define close(x) s_close(x)

#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff	/* should be in <netinet/in.h> */
#endif

/*
 * The following globals are available to the caller, if desired.
 */

struct sockaddr_in	udp_srv_addr;	/* server's Internet socket addr */
struct sockaddr_in	udp_cli_addr;	/* client's Internet socket addr */
struct servent		udp_serv_info;	/* from getservbyname() */
struct hostent		udp_host_info;	/* from gethostbyname() */

int			/* return socket descriptor if OK, else -1 on error */
udp_open(host, service, port, dontconn)
char	*host;		/* name of other system to communicate with */
char	*service;	/* name of service being requested */
			/* can be NULL, iff port > 0 */
int	port;		/* if == 0, nothing special - use port# of service */
			/* if < 0, bind a local reserved port */
			/* if > 0, it's the port# of server (host-byte-order) */
int	dontconn;	/* if == 0, call connect(), else don't */
{
	int		fd;
	unsigned long	inaddr;
	struct servent	*sp;
	struct hostent	*hp;

	/*
	 * Initialize the server's Internet address structure.
	 * We'll store the actual 4-byte Internet address and the
	 * 2-byte port# below.
	 */

	bzero((char *) &udp_srv_addr, sizeof(udp_srv_addr));
	udp_srv_addr.sin_family = AF_INET;

	if (service != NULL) {
		if ( (sp = getservbyname(service, "udp")) == NULL) {
			fprintf(stderr,"udp_open: unknown service: %s/udp\n", service);
			return(-1);
		}
		udp_serv_info = *sp;			/* structure copy */
		if (port > 0)
			udp_srv_addr.sin_port = htons(port);
							/* caller's value */
		else
			udp_srv_addr.sin_port = sp->s_port;
							/* service's value */
	} else {
		if (port <= 0) {
			fprintf(stderr,"udp_open: must specify either service or port\n");
			return(-1);
		}
		udp_srv_addr.sin_port = htons(port);
	}

	/*
	 * First try to convert the host name as a dotted-decimal number.
	 * Only if that fails do we call gethostbyname().
	 */

	if ( (inaddr = inet_addr(host)) != INADDR_NONE) {
						/* it's dotted-decimal */
		bcopy((char *) &inaddr, (char *) &udp_srv_addr.sin_addr,
			sizeof(inaddr));
		udp_host_info.h_name = NULL;

	} else {
		if ( (hp = gethostbyname(host)) == NULL) {
			fprintf(stderr,"udp_open: host name error: %s %s\n",
						host,strerror(errno));
			return(-1);
		}
		udp_host_info = *hp;	/* found it by name, structure copy */
		bcopy(hp->h_addr, (char *) &udp_srv_addr.sin_addr,
			hp->h_length);
	}

	if (port < 0)
		fprintf(stderr,"udp_open: reserved ports not implemeneted yet\n");

	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr,"udp_open: can't create UDP socket\n");
		return(-1);
	}

	/*
	 * Bind any local address for us.
	 */

	bzero((char *) &udp_cli_addr, sizeof(udp_cli_addr));
	udp_cli_addr.sin_family      = AF_INET;
	udp_cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	udp_cli_addr.sin_port        = htons(0);
	if (bind(fd, (struct sockaddr *) &udp_cli_addr,
						sizeof(udp_cli_addr)) < 0) {
		fprintf(stderr,"udp_open: bind error\n");
		close(fd);
		return(-1);
	}

	/*
	 * Call connect, if desired.  This is used by most caller's,
	 * as the peer shouldn't change.  (TFTP is an exception.)
	 * By calling connect, the caller can call send() and recv().
	 */

	if (dontconn == 0) {
		if (connect(fd, (struct sockaddr *) &udp_srv_addr,
						sizeof(udp_srv_addr)) < 0) {
			fprintf(stderr,"udp_open: connect error\n");
			return(-1);
		}
	}

	return(fd);
}
