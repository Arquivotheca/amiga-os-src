/*
**  Transfer.c
**
**  Written by Dale Larson, Software Engineer, Commodore-Amiga, Inc.
**  Copyright 1991, Commodore-Amiga, Inc.
**  Permission to use granted provided this notice remains intact.
**
**  The functions in this file open, close, read to and write from my_files.
**  my_files are either sockets or AmigaDOS files, depending on whether the
**  file being accessed is remote or local.  The structure includes a flag
**  indicating whether it is a file or socket (see client.h).  Filenames for
**  local files are specified exactly like they always have been.  Filenames
**  for remote files have a host name followed by a '!' followed by a local
**  (to the host) filename (i.e.  host!ram:test).
*/


#include "ncopy.h"
#include "client.h"


/*
**  my_open()
**
**  Accepts a filename (see parse()) and an AmigaDOS file Open() mode
**  (use MODE_OLDFILE to read, MODE_NEWFILE to write).  Returns a pointer
**  to a my_file or prints an error message and returns NULL.
**
**  Allocates memory for a my_file, then calls function to parse filename.
**  If file is local, attempts to open file and returns a pointer to and
**  AmigaDOS type my_file or prints an error message and returns NULL.  If
**  file is non-local, my_open() does the following:
**
**  Get an machine-readable hostname from the human-readable one.
**  Get a new socket.
**  Connect the socket to a specific port number at the remote host.
**  Send the mode for transfer and the filename to send or recieve.
**  Get back an error string (zero length if no error).
**  Return the pointer to a socket type my_file or print an error message
**	and return NULL.
**
**  NOTE:  The calling function is responsible for my_close()ing the returned
**  my_file pointer.
*/
my_file *my_open(char *filename, int mode)
{
my_file *f;
char *host, 			/*  host name parsed out of filename  */
     *file,			/*  file parsed out of filename  */
     buffer[LENGTH];    	/*  for sprintf() and for errmsg from server  */
struct sockaddr_in server;  	/*  socket address (internet) for server  */
struct hostent *hp;
size_t len = sizeof(server);

	if( !(f = (my_file *)AllocMem((LONG)sizeof(my_file), 0L)) )
	{
		puts("Unable to open file because of low memory.");
		return(NULL);
	}

	/*
	**  parse() allocates memory for the host string (if any) and
	**  we must free it later (file will point into filename).
	*/
	if(parse(filename, &host, &file))
	{
		puts("Not enough memory to parse filename.");
		return(NULL);
	}

	/*
	**  If it is a local file, open it:
	*/
	if(!host)
	{
		f->tag = dos_myfile;
		if(f->data.file = Open(filename, mode))
			return(f);
		sprintf(buffer, "Unable to open local file '%s'", filename);
		PrintFault(IoErr(), buffer);
		return(NULL);
	}

	/*
	**  It's not a local file, connect a socket and initialize the server.
	*/
	f->tag = socket_myfile;

	/*
	**  Since server came off the stack, we initialize it to zero before
	**  using.
	*/
	bzero(&server, (int)len);

	/*
	**  Find remote machine -- assume host name is dotted-decimal (you
	**  can use inet_addr() under this assumption, call gethostbyname()
	**  if that assumtion fails:
	*/
	if( (server.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		if( (hp=gethostbyname(host)) == NULL)
		{
			printf("Can't find host %s\n", host);
			return(NULL);
		}
		/*
		**  Found it by name, copy hp into server.
		*/
		bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
	}

	/*
	**  Finish filling out the server socket address by adding the
	**  protocol family to use and the port number to connect to.
	*/
	server.sin_family = AF_INET;
	server.sin_port = htons(6666);

	/*
	**  Free the host string now that we're done with it.
	*/
	FreeMem(host, strlen(host)+1);

	/*
	**  Initialize a socket.
	*/
	if( (f->data.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Can't talk to server [socket]");
		return(NULL);
	}

	/*
	**  Connect to remote socket specified in 'server'
	**  (which we filled out above).
	*/
	if(connect(f->data.socket, (struct sockaddr *)&server, (int)len) < 0)
	{
		perror("Can't talk to server [connect]");
		close(f->data.socket);
		return(NULL);
	}

	/*
	** Send mode of request (are we sending from or to?).
	**	NOTE:  Since the mode is an int, both client and server must
	**	       be compiled with the same int-size.
	*/
	send(f->data.socket, (char *)&mode, (int)sizeof(int), 0);

	/*
	**  Send remote filename (length is strlen()+1 to include '\0').
	*/
	send(f->data.socket, file, strlen(file)+1, 0);

	/*
	**  Check for errors from the other end.
	**  (could hang here if the server has serious problems)
	*/
        recv(f->data.socket, buffer, LENGTH, 0);

	if(strlen(buffer)>0)
	{
		printf("Server can't open '%s\n", buffer);
		close(f->data.socket);
		return(NULL);
	}
	return(f);
}


/*
**  parse()
**
**  Parses the filename 'string' into a hostname and real filename.
**  Modifies *host to the hostname part of a filename (or NULL if there is
**  none).  Modifies *file to point to after 'hostname!'.
**  If everything goes well, returns a RETURN_OK, else a RETURN_ERROR.
**
**  A well-formed filename must, if it contains a hostname, be a hostname
**  followed by a '!', followed a filename the remote system understands.  The
**  hostname must not contain a '/', ':', or '!'
**
**  The case "ncopy ram:x work:" is not supported (because it would require
**  the second invocation of my_open() to know about the first).
**
**  The case "ncopy ram:x sprite!test" is supported.  It copies x to the
**  file test in the directory in which the server on sprite was started.
*/

int parse(char *string, char **host, char **file)
{
int len;	/*  length of the host string (includes nul)  */

	*host = NULL;
	*file = string;

	/*
	**  Go through the string searching for important punctuation.
	*/
	while(**file != '\0')
	{
		if(**file == '/' || **file == ':')
		{
			*file = string;
			return RETURN_OK;
		}
		if(**file == '!')
			break;
		(*file)++;
	}

	/*
	**  Check to see if we reached the end of a puctuationless name.
	*/
	if(**file == '\0')
	{
		*file = string;
		return RETURN_OK;
	}

	/*
	**  '!' isn't part of the filename, so move past it.
	*/
	(*file)++;

	len = *file-string;

	if( !(*host = AllocMem(len, 0L)) )
		return RETURN_ERROR;
	CopyMem(string, *host, len-1);
	(*host)[len-1] = '\0';  /*  make it a null terminated string  */
	return RETURN_OK;
}


/*
**  my_close()
**
**  Closes the my_file file.  NULL is ok.
*/
void my_close(my_file *file)
{
	if(!file)
		return;
	if(file->tag == socket_myfile)
		s_close(file->data.socket, ssinfo);
	else
		Close(file->data.file);
	FreeMem(file, (LONG)sizeof(my_file));
}


/*
**  my_read()
**
**  Fill 'buffer' of 'length' characters from 'file.'
**  Returns number of characters actually read (-1 == error, 0 == EOF).
*/
int my_read(my_file *file, char *buffer, int length)
{
	if(file->tag == dos_myfile)
		return( Read(file->data.file, (void *)buffer, length) );
	return( recv(file->data.socket, buffer, length, 0) );
}


/*
**  my_write()
**
**  Send 'file' 'length' characters from 'buffer'.
**  Returns number of characters actually written (-1 == error).
*/
int my_write(my_file *file, char *buffer, int length)
{
	if(file->tag == dos_myfile)
		return( Write(file->data.file, (void *)buffer, length) );
	return( send(file->data.socket, buffer, length, 0) );
}
