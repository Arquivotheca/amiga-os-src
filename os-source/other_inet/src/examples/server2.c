/*
**  server2.c
**
**  Written by Dale Larson, Software Engineer, Commodore-Amiga, Inc.
**  Copyright 1991, Commodore-Amiga, Inc.
**  Permission to use granted provided this notice remains intact.
**
**  This is a version of the ncopy server which processes multiple requests
**  simultaneously.  It spawns multiple processes to do so (it could also
**  use selectwait() to do [see server3.c]).  It is more complex than the
**  simple server, but not much, and it is nicer under frequent and/or heavy
**  use.  It also allows users to copy a file from one location on a remote
**  machine to another location on the same remote machine (i.e.
**  "ncopy sprite!ram:x sprite!work:y").  A system requester will not
**  pop up if the server recieves a request to read/write to/from a
**  non-existent device.
**
**  This server has no provision for exiting.
**
**  You should be familiar with the ncopy client (ncopy.c and transfer.c) and
**  the simple ncopy server (server1.c) before studying this server.
**
**  Thanks to Randell Jessup for his excellent idea on how to easily
**  arbitrate access to the global Ns (which is used to pass info to child
**  processes).
*/


#include "ncopy.h"

struct Library *SSBase;
struct ss_info *ssinfo;


char vers[] = "\0$VER: server2 1.0 (25.02.91)";

void send_file(int s, BPTR file);
void get_file(int s, BPTR file);
void handle_request(void);
int  init(void);

/*  Not in current SAS 5.10 or Manx5.0d includes  */
struct Process *CreateNewProcTags( unsigned long tag1type, ... );

/*  global variable for New socket read by new processes:  */
int Ns = 0;


/*
**  main()
**
**  Call init() then wait to accept connections. Once a connection has been
**  accepted, start a new process to execute the handler function with the
**  socket which accept() connected to a remote machine.  Go back to accepting
**  connections.
*/
main(int argc, char **argv)
{
int s; 			/*  socket to accept connections on  */
struct Process *pid;

	s = init();

	/*
	**  Accept connections from s and process them on ns.
	*/
	while(1)
	{
		/*
		**  Make sure that there is not a child process which hasn't
		**  had a chance to copy its socket.  Ns must be initialized
		**  to zero before this while() and children must set Ns
		**  to zero when they have copied it.
		*/
		if(Ns)
		{
			Delay(1);
			continue;
		}
		if( (Ns = accept(s,NULL,NULL)) >= 0 )
		{
		static char proc_name[20];

			sprintf(proc_name, "ncopy server #%2d", Ns);
			pid = CreateNewProcTags(NP_Entry, (LONG)handle_request,
						NP_Name, (LONG)proc_name,
						NP_WindowPtr, -1L,
						TAG_DONE);
			if(!pid)
			{
				close(Ns);
				Ns = 0;
			}
		}
	}
}


/*
**  init()
**
**  Get a socket, bind a name to it, express a willingness to accept
**  connections to it (listen) and return the socket.  Exit on error.
*/
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
	bzero(&sin, len);
	sin.sin_family = AF_INET;
	/*
	**  While prototyping a program, pick an unused port number above
	**  1024 like I did here.  For a real program, assign a name and a
	**  port number above 1024 and stick it in inet:db/servers on all
	**  machines which will use the application.  That way the user can
	**  reconfigure port numbers used by new applications if necessary.
	**  You can modify the server to get the port number from the
	**  database by using this code (instead of "sin.sin_port=htons(6666)"
	**  below):
	**
	**  struct servent *servptr;
	**  char serv[] = "servicename";
	**
	**  if( (servptr = getservbyname(serv, "tcp")) == NULL)
	**  {
	**  	printf("Unknown service %s.  Check inet:db/services.\n",serv);
	**	return RETURN_ERROR;
	**  }
	**  sin.sin_port = servptr->s_port;
	*/
	sin.sin_port = htons(6666);
	sin.sin_addr.s_addr = INADDR_ANY;
	if(bind(s, &sin, len) < 0)
	{
		perror("bind");
		exit(RETURN_ERROR);
	}

	/*
	**  Put socket into "listen" state (ready to accept connections).
	*/
	listen(s,5);
	return s;
}


/*
**  handle_request()
**
**  Handles a request to send or receive a file on a given socket.
**  Handling a request consists of the following:
**  	1)  receive the mode (send or receive -- MODE_OLDFILE or MODE_NEWFILE)
**	2)  receive the filename
**	3)  attempt to open the file
**	4)  return "\0" for succesful open,
**	    specially formated informative error message otherwise
**	5)  call function to send or receive the file
**	6)  closes the file we opened and the socket we were passed
*/
void handle_request(void)
{
int s;		/*  socket passed by parent  */
char buffer[LENGTH];
int mode;	/*  AmigaDOS file open mode (MODE_OLDFILE or MODE_NEWFILE)  */
BPTR file;	/*  BCPL pointer to an AmigaDOS file handle  */

	geta4();

        s = Ns;
	Ns = 0;  /*  So that parent may feel free to use it again  */

	/*  Get the mode.
	**  Don't worry about making sure that we got what we wanted because we
	**  can't really screw anything up and we can't send an error message
	**  if our communcications are error-prone.
	*/
	recv(s, (char *)&mode, sizeof(int), 0);

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
