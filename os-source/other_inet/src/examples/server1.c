/*
**  server.c
**
**  Written by Dale Larson, Software Engineer, Commodore-Amiga, Inc.
**  Copyright 1991, Commodore-Amiga, Inc.
**  Permission to use granted provided this notice remains intact.
**
**  This is a simple version of the ncopy server.  It processes only one
**  request at a time.  This is adequate for a server which sees light or
**  infrequent usage, but can lead to unacceptable delays for a server which
**  sees heavy or even moderate trafic.  It also will cause problems if the
**  user copies a file from a remote machine to the same remote machine
**  (i.e.  ncopy sprite!ram:x sprite!work:y).
**
**  You should study and understand this server before you look at
**  server2.c, which is significantly more complex.
**
**  This server has no provisions for exiting.
**  A system requester will pop up if the server recieves a request to
**  read/write to/from a device a non-existant device.
*/


#include "ncopy.h"

struct Library *SSBase;
struct ss_info *ssinfo;


char vers[] = "\0$VER: server1 1.0 (25.02.91)";

void handle_request(int s);
void send_file(int s, BPTR file);
void get_file(int s, BPTR file);


/*
**  main()
**
**  Get a socket, bind a name to it, express a willingness to accept
**  connections to it (listen) then wait to actually accept connections.
**  Once a connection has been accepted, give the handler function the
**  socket which accept() connected to a remote machine so that the handler
**  function can chat with the remote machine.  When the handler returns,
**  go back to waiting to accept connections.
*/
main(int argc, char **argv)
{
struct sockaddr_in sin;	/*  socket address (internet) to establish  */
int s, ns;		/*  socket and new socket  */
size_t len = sizeof(sin);

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

	/*
	**  Initialize a socket.
	*/
	if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(RETURN_ERROR);
	}

	/*
	**  Bind name to socket.
	**  Since we got sin off the stack, it could contain anything, and so
	**  we zero it before initializing it.
	*/
	bzero(&sin, (int)len);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(6666);
	sin.sin_addr.s_addr = INADDR_ANY;
	if(bind(s, (struct sockaddr *)&sin, len) < 0)
	{
		perror("bind");
		exit(RETURN_ERROR);
	}

	/*
	**  Put socket into "listen" state (ready to accept connections).
	*/
	listen(s,5);

	/*
	**  Accept connections from s and process them on ns.
	*/
	while(1)
	{
		if( (ns = accept(s,NULL,NULL)) >= 0)
			handle_request(ns);
	}
}


/*
**  handle_request()
**
**  Handles a request to send or receive a file over a given socket.
**  Handling a request consists of the following:
**  	1)  receive the mode (send or receive -- MODE_OLDFILE or MODE_NEWFILE)
**	2)  receive the filename
**	3)  attempt to open the file
**	4)  return "\0" for succesful open,
**	    specially formated informative error message otherwise
**	5)  call function to send or receive the file
**	6)  close the file we opened and the socket we were passed
*/
void handle_request(int s)
{
char buffer[LENGTH];
int mode;	/*  AmigaDOS file open mode (MODE_OLDFILE or MODE_NEWFILE)  */
BPTR file;	/*  BCPL pointer to an AmigaDOS file handle  */


	/*  Get the mode.
	**  Don't worry about making sure that we got what we wanted because we
	**  can't really screw anything up and we can't send an error message
	**  if our communcications are error-prone.
	*/
	recv(s, (char *)&mode, (int)sizeof(int), 0);

	/*  Get the filename which we'll read or write (depending on mode).
	**  (Assume that we get only null-terminated strings.)
	*/
	recv(s, buffer, LENGTH, 0);

	/*
	**  Attempt to open file.
	*/
	if( !(file = Open(buffer, mode)) )
	{
	int len;

		len = strlen(buffer);
		strncat(buffer, "': ", LENGTH-4);  /*  4 not 3 'cause of nul  */
		Fault(IoErr(), NULL, buffer+len+3, LENGTH-len-4);
		send(s, buffer, strlen(buffer)+1, 0);
		close(s);
		return;
	}

	/*
	**  Indicate succesful opening of file.
	*/
	send(s, "\0", 1, 0);

	/*
	**  Send or recieve the file.
	*/
	if(mode == MODE_OLDFILE)
		send_file(s, file);
	else
		get_file(s, file);

	Close(file);
	close(s);
}


void send_file(int s, BPTR file)
{
char	buffer[LENGTH];
long	actualLength;

	do
	{
		actualLength = Read(file, (void *)buffer, LENGTH);
	        if(actualLength < 0)
			return;  /*  Error, but we have no way to report it.  */
		if(send(s, buffer, actualLength, 0) < 0)
			return;  /*  Error, but we have no way to report it.  */
	}while(actualLength);
}


void get_file(int s, BPTR file)
{
char	buffer[LENGTH];
int 	n;

	do
	{
		n = recv(s, buffer, LENGTH, 0);
		if(n < 0)
			return;  /*  Error, but we have no way to report it.  */
		if(Write(file, (void *)buffer, n) < 0)
			return;  /*  Error, but we have no way to report it.  */
	}while(n);
}
