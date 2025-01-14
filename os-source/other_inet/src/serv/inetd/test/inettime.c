#include	<sys/types.h>
#include	<sys/socket.h>
#include	<stdio.h>
#include	<sys/errno.h>
#include <exec/types.h>
#include <ss/socket.h>
#include <string.h>

#define index(s,c)	strchr(s,c)

extern int	errno;

#define	MAXLINE		  512	/* max line length */
#define	TVAL_SIZE	    4	/* 4 bytes in the 32-bit binary timeval */

/*
 * Contact the time server using TCP/IP and print the result.
 */

tcp_time(host)
char	*host;
{
	int		fd, i;
	unsigned long	temptime, timeval;

	/*
	 * Initiate the connection and receive the 32-bit time value
	 * (in network byte order) from the server.
	 */

	if ( (fd = tcp_open(host, "time", 0)) < 0) {
		perror("tcp_open error");
		return;
	}

	if ( (i = readn(fd, (char *) &temptime, TVAL_SIZE)) != TVAL_SIZE)
		fprintf(stderr,"received %d bytes from server\n", i);

	timeval = ntohl(temptime);
	printf("time from host %s using TCP/IP = %lu\n", host, timeval);

	s_close(fd);
}

/*
 * Contact the daytime server using TCP/IP and print the result.
 */

tcp_daytime(host)
char	*host;
{
	int	fd;
	char	*ptr, buff[MAXLINE];

	/*
	 * Initiate the session and receive the netascii daytime value
	 * from the server.
	 */

	if ( (fd = tcp_open(host, "daytime", 0)) < 0) {
		perror("tcp_open error");
		return;
	}
	if(readline(fd, buff, MAXLINE) < 0)
		fprintf(stderr,"readline error\n");

	if ( (ptr = index(buff, '\n')) != NULL)
		*ptr = 0;
	printf("daytime from host %s using TCP/IP = %s\n", host, buff);

	s_close(fd);
}

/*
 * Contact the time server using UDP/IP and print the result.
 */

udp_time(host)
char	*host;
{
	int		fd, i;
	unsigned long	temptime, timeval;

	/*
	 * Open the socket and send an empty datagram to the server.
	 */

	if ( (fd = udp_open(host, "time", 0, 0)) < 0) {
		perror("udp_open error");
		return;
	}

	/*
	 * Send a datagram, and read a response.
	 */

	if ( (i = dgsendrecv(fd, (char *) &temptime, 1, (char *) &temptime,
			TVAL_SIZE, (struct sockaddr *) 0, 0)) != TVAL_SIZE)
		if (errno == EINTR) {
			perror("udp_time: no response from server");
			s_close(fd);
			return;
		} else
			fprintf(stderr,"received %d bytes from server\n", i);

	timeval = ntohl(temptime);
	printf("time from host %s using UDP/IP = %lu\n", host, timeval);

	s_close(fd);
}

/*
 * Contact the daytime server using UDP/IP and print the result.
 */

udp_daytime(host)
char	*host;
{
	int	fd, i;
	char	*ptr, buff[MAXLINE];

	/*
	 * Open the socket and send an empty datagram to the server.
	 */

	if ( (fd = udp_open(host, "daytime", 0, 0)) < 0) {
		perror("udp_open error");
		return;
	}

	if ( (i = dgsendrecv(fd, buff, 1, buff, MAXLINE,
				(struct sockaddr *) 0, 0)) < 0)
		if (errno == EINTR) {
			perror("udp_daytime: no response from server");
			s_close(fd);
			return;
		} else {
			fprintf(stderr,"read error\n");
			return;
		}

	buff[i] = 0;		/* assure its null terminated */
	if ( (ptr = index(buff, '\n')) != NULL)
		*ptr = 0;
	printf("daytime from host %s using UDP/IP = %s\n", host, buff);

	s_close(fd);
}









