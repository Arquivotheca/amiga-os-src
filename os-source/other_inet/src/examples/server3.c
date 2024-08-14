/*
**  server3.c
**
**  Written by Dale Larson, Software Engineer, Commodore-Amiga, Inc.
**  Copyright 1991, Commodore-Amiga, Inc.
**  Permission to use granted provided this notice remains intact.
**
**  This is a version of the ncopy server which processes multiple requests
**  simultaneously.  It utilizes selectwait() to do so (a server could also
**  spawn multiple processes to do so).  It is more complex than the simple
**  server, but is nicer under frequent and/or heavy use, and allows users to
**  copy a file from one location on a remote machine to another location on
**  the same remote machine (i.e.  "ncopy sprite!ram:x sprite!work:y").  Using
**  selectwait() (or multiple processes) also makes it reasonable to implement
**  exit-on-break-signal. A system requester will not pop up if the server
**  recieves a request to read/write to/from a non-existent device.
**
**  Note that this server really does not properly handle copying from one
**  locations on a remote machine to another location on the same remote
**  machine.  It works fine for files not more than about 8k, but will hang
**  for files larger than that.  The reason is that it sends all of a file
**  at once.  If the target is another machine, the other machine's ncopy or
**  server will read the data as this server writes it.  But if this server
**  is also supposed to read the data, 8 TCP packets will be buffered, then
**  send() will block until some of them are read and acknowledged.  This
**  problem could be solved in the server by including a select() in the
**  send loop, by not allowing ncopy to transfer to and from the same machine,
**  or by redesigning the ncopy protocol.  Any of these solutions would unduly
**  complicate and excede the scope of this example.
**
**  You should be familiar with the ncopy client (ncopy.c and transfer.c) and
**  the simple ncopy server (server1.c) before studying this server.
**
*/


#include "ncopy.h"

struct Library *SSBase;
struct ss_info *ssinfo;


char vers[] = "\0$VER: server3 1.0 (28.02.91)";

int process(int s, int state, fd_set *mask, int highSock);
int nhs(int s, fd_set *mask);
int init(void);

/*  state constants  */
#define SCLOSE 		0  /*  Need to close the socket  */
#define SINIT		1  /*  Need to initialize connection  */
#define SDATA		2  /*  Need to recieve data and write to disk  */

#include <time.h>
struct timeval NoTime = {0L, 0L};

main(int argc, char **argv)
{
int s,			/*  socket to accept connections on, new socket  */
    ns,			/*  new socket  	*/
    i,			/*  index in for loops  */
    state[FD_SETSIZE],	/*  where we are in communication on all socket  */
    highSock;		/*  the highest open socket  */
fd_set sockmask, mask; 	/*  mask of all open sockets, return val  */
long umask;

	highSock = s = init();
	FD_ZERO(&sockmask);
	FD_SET(s, &sockmask);
	while(1)
	{
		mask = sockmask;
		umask = SIGBREAKF_CTRL_C;

		if(s_selectwait(highSock+1, &mask, NULL, NULL, NULL, &umask,ssinfo) == -1)
		{
			perror("select");
			goto cleanup;
		}

		for(i=0; i <= highSock; i++)
			if(FD_ISSET(i, &mask))
				if (i == s)
				{
					ns = accept(s, NULL, NULL);
					if(ns != -1)
					{
						state[ns] = SINIT;
						FD_SET(ns, &sockmask);
						if(ns > highSock)
							highSock = ns;
					}
				} else
				{
					state[i] = process(i, state[i], &sockmask, highSock);
					if(state[i] == SCLOSE)
					{
						if(i == highSock)
							highSock = nhs(i,&sockmask);
						s_close(i, ssinfo);
						FD_CLR(i, &sockmask);
					}
				}
	}

cleanup:
	for(i=0; i<=highSock; i++)
		if( FD_ISSET(i, &sockmask) )
		{
			s_close(i, ssinfo);
		}
	cleanup_sockets(ssinfo);
	CloseLibrary(SSBase);
}


int nhs(int s, fd_set *mask)
{
	while(s--)
	{
		if(FD_ISSET(s, mask))
			return s;
	}
	return FD_SETSIZE;
}


int init(void)
{
struct sockaddr_in sin;	/*  socket address (internet) to establish  */
int s, 			/*  socket to accept connections on  */
    len = sizeof(sin);

	/*  Open Shared Socket library:
	*/
        if(SSBase = OpenLibrary( "inet:libs/socket.library", 0L ))
        {
                ssinfo = setup_sockets( &errno );
	}else
	{
		puts("Can't open socket.library.");
		exit(RETURN_ERROR);
	}

	if( (s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		goto cleanup;;
	}

	bzero(&sin, len);
	sin.sin_family = AF_INET;

	sin.sin_port = htons(6666);
	sin.sin_addr.s_addr = INADDR_ANY;
	if(bind(s, (struct sockaddr *)&sin, len) < 0)
	{
		perror("bind");
		s_close(s, ssinfo);
		goto cleanup;
	}

	listen(s,5);
	return s;

cleanup:
	cleanup_sockets(ssinfo);
	CloseLibrary(SSBase);
	exit(RETURN_ERROR);
}


int process(int s, int state, fd_set *mask, int highSock)
{
char buffer[LENGTH];
static int mode[FD_SETSIZE];	/*  AmigaDOS file open modes  */
static BPTR file[FD_SETSIZE];	/*  File handles  */
int n;				/*  number of bytes read/written  */
fd_set *mask2;			/*  clone of mask  */

	switch(state)
	{
	case SINIT:
		recv(s, (char *)&(mode[s]), sizeof(int), 0);
		recv(s, buffer, LENGTH, 0);
		if( !(file[s] = Open(buffer, mode[s])) )
		{
		int len;

			len = strlen(buffer);
			strncat(buffer, "': ", LENGTH-4);  /*  4 not 3 'cause of nul  */
			Fault(IoErr(), NULL, buffer+len+3, LENGTH-len-4);
			send(s, buffer, strlen(buffer)+1, 0);
			return SCLOSE;
		}
		send(s, "\0", 1, 0);
		if(mode[s] == MODE_NEWFILE)
		{
			return SDATA;
		}
/*  fall through if MODE_OLDFILE  */
	case SDATA:
		if(mode[s] == MODE_OLDFILE)
		{
			while(1)
			{
				n = Read(file[s], (void *)buffer, LENGTH);
		       		if(n <= 0)
					goto close;
				if(send(s, buffer, n, 0) < 0)
					goto close;
			}
		}else
		{
			n = recv(s, buffer, LENGTH, 0);
			if(n <= 0)
				goto close;
			if(Write(file[s], (void *)buffer, n) < 0)
				goto close;
			return SDATA;
		}
	default:
		;
	/*  in a real program, should do something more intellegent here  */
	}
close:
	Close(file[s]);
	return SCLOSE;
}
