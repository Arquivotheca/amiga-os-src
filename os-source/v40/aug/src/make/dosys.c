#include "defs.h"
#include <signal.h>
#include <proto/all.h>
#include <rexx/storage.h>
#include <libraries/dos.h>
#include <exec/ports.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <stdio.h>

dosys(comstring,nohalt)
register char *comstring;
int nohalt;
{
    register int status;
		status = doshell(comstring,nohalt);
    return(status);
}



metas(s)   /* Are there are any	 Shell meta-characters? */
register char *s;
{
    register char c;

    while( (funny[c = *s++] & META) == 0 )
	;
    return((int) c );
}

extern	struct List	PortList;
extern	struct MsgPort	*MyPort, *REXXPort;

doshell(comstring)
char	*comstring;
{
	struct	RexxMsg		*theRexxMsg, *CreateRexxMsg();
	int					 i;
	char				*TempBuffer;
	ULONG				 BufferLen;
	

		if(theRexxMsg = CreateRexxMsg(MyPort, "", "COMMAND")) {
			BufferLen = strlen(comstring) + 5L;
			if ( !(TempBuffer = (char *)AllocMem(BufferLen, MEMF_PUBLIC | MEMF_CLEAR))) {
				DeleteRexxMsg(theRexxMsg);
				fatal("Could not allocate room for expansion");
				}
			*((ULONG *)TempBuffer) = BufferLen;
			TempBuffer += 4;
			strcat (TempBuffer, comstring);
			theRexxMsg->rm_Args[0] = TempBuffer;
			TempBuffer -= 4;
			for (i = 1; i < 16; i++)
				theRexxMsg->rm_Args[i] = 0L;
			if(FillRexxMsg(theRexxMsg, 1, 0x0)) {
				theRexxMsg->rm_Action |= RXCOMM | RXFF_STRING;
				PutMsg(REXXPort, (struct Message *)theRexxMsg);
				}
			else {
				FreeMem(TempBuffer, *((ULONG *)TempBuffer));
				DeleteRexxMsg(theRexxMsg);
				fatal("Could not fill rexx message");
				}
			}

    return (await(MyPort,theRexxMsg, TempBuffer) );
    
}



await(thePort, theMessage, theBuffer)
struct	MsgPort	*thePort;
struct	RexxMsg	*theMessage;
char			*theBuffer;
	{
	
	struct	RexxMsg *ReturnedMessage;
	int				 returncode;
	
	while (1) {
		while (ReturnedMessage = (struct RexxMsg *)GetMsg(thePort))
			if (ReturnedMessage == theMessage) {
				returncode = ReturnedMessage->rm_Result1;
				ClearRexxMsg(theMessage, 1);
				DeleteRexxMsg(theMessage);
				FreeMem(theBuffer, *((ULONG *)theBuffer));
				return(returncode);
				}
			else
				ReplyMsg((struct Message *)ReturnedMessage);
		WaitPort(MyPort);
		}
}




doclose()   /* Close open directory files before exec'ing */
{
    int i;
    for( i = 3 ; i < 32 ; i++ ) {
	close( i );
    }
	return(0);
}



#define MAXARGV 400
#include <errno.h>

touch(force, name)
int force;
char *name;
{
	BPTR					FileLock;
	struct FileInfoBlock	*data;
	int						fd;
	char					junk[1];
	
	if(FileLock = Lock(name, SHARED_LOCK)) {
		if(!(data = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR | MEMF_PUBLIC))) {
			UnLock(FileLock);
			fatal("Out of memory");
			}
		if(!Examine(FileLock, data))
			goto create;
		if (data->fib_Size == 0)
		goto create;
    if( (fd = open(name, 2)) < 0)
		goto bad;

    if( read(fd, junk, 1) < 1) {
		close(fd);
		goto bad;
	    }

    lseek(fd, 0L, 0);

    if( write(fd, junk, 1) < 1 ) {
		close(fd);
		goto bad;
	    }

    close(fd);
    return(0);

bad:
    fprintf(stderr, "Cannot touch %s\n", name);
    goto common;

create:
    if( (fd = creat(name, 0666)) < 0)
		goto bad;
    close(fd);

common:
		UnLock(FileLock);
		FreeMem(data, sizeof(struct FileInfoBlock));
		return(0);
		}
	else
		goto create;
}
