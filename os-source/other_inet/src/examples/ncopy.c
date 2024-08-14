/*
**  ncopy.c
**
**  Written by Dale Larson, Software Engineer, Commodore-Amiga, Inc.
**  Copyright 1991, Commodore-Amiga, Inc.
**  Permission to use granted provided this notice remains intact.
**
**  An example ncopy client.  This file contains only the main loop.
**  (See also transfer.c).  The ncopy client can copy files from the
**  current machine to the current machine or from any connected Amiga
**  running an ncopy server to any other Amiga running an ncopy server.
**
**  This ncopy server is somewhat deficient in that it:
**	Doesn't do wildcards or multiple or directory copies.
**	Does not handle the case "copy x ram:", must be "copy x ram:x"
**
**  The copy loop could be made more efficient, but it transparently
**  handles the different cases presented by files being on local and
**  remote machines.  Thanks to Mike Sinz for this idea.
*/


#include "ncopy.h"
#include "client.h"

struct Library *SSBase;
struct ss_info *ssinfo;


char vers[] = "\0$VER: ncopy 1.0 (25.02.91)";


/*
**  main()
**
**  Process arguments.
**
**  If they're ok, my_open() a source and destination my_file.  A my_file
**  is either a socket or an AmigaDOS file, depending on whether the file
**  is local or not (see ncopy.h and transfer.c/parse()).
**
**  If the my_open()s went ok, copy the file by my_read()ing a buffer and
**  my_write()ing the buffer until there's nothing left to my_read() or there
**  is an error.
**
**  Clean up.
*/
main(int argc, char **argv)
{
LONG opts[OPT_COUNT];		/*  For ReadArgs()  */
struct RDArgs *argsptr;		/*  for ReadArgs() return  */
char *from, *to, 		/*  file names from ReadArgs()  */
     buffer[LENGTH];		/*  buffer for the copy operation  */
my_file *source, *destination;	/*  socket/files for reading and writing  */
int n;				/*  number of bytes read  */

	/*
	** Process arguments using new (2.0) dos calls.
	*/
	argsptr = (struct RDArgs *)ReadArgs((UBYTE *)TEMPLATE, opts, NULL);
	if(argsptr == NULL)
	{
		PrintFault(IoErr(), "Command line not accepted");
		return RETURN_ERROR;
	}
	from = (char *)opts[OPT_FROM];
	to   = (char *)opts[OPT_TO];

	/*
	**  Don't need to check length of filenames under current operating
	**  system because command limited to 255 characters and our LENGTH
	**  should be bigger than that.
	*/

	/*  Open Shared Socket library:
	*/
        if(SSBase = OpenLibrary( "inet:libs/socket.library", 0L ))
        {
                ssinfo = setup_sockets( &errno );
	}else
	{
		puts("Can't open socket.library.");
		return RETURN_ERROR;
	}

	/*
	**  Open my_files (file/sockets).  If there is an error, my_open()
	**  will print an error message, so we don't worry about it.
	*/
	if( !(source = my_open(from, MODE_OLDFILE)) )
		return RETURN_ERROR;
	if( !(destination = my_open(to, MODE_NEWFILE)) )
		return RETURN_ERROR;

	/*
	**  Copy the file.
	*/
	while( (n = my_read(source, buffer, LENGTH)) > 0)
		if (!my_write(destination, buffer, n))
			/*
			**  Error.  Error message should be printed by
			**  my_read/write(), so we need only stop and clean up.
			**  We're too lazy to set the return code properly.
			*/
			break;
	/*
	**  Clean up.
	*/
	my_close(source);
	my_close(destination);
	cleanup_sockets(ssinfo);
	CloseLibrary(SSBase);
	FreeArgs(argsptr);
	return RETURN_OK;
}
