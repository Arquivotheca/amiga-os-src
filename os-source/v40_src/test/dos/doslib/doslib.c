#include "exec/types.h" 
#include "exec/memory.h" 
#include "exec/io.h"
#include "exec/libraries.h"
#include "exec/execbase.h"
#include "libraries/dos.h" 
#include "libraries/dosextens.h"
#include "workbench/startup.h"
#include "workbench/workbench.h"
#include "rexx:storage.h"
#include "rexx:rxslib.h" 
#define HOST_PORT_NAME   "dos_Lib"

#define F_OPEN 		8
#define F_CLOSE		0
#define QUIT		10
#define F_DELETE	3
#define F_LOCK		7
#define F_UNLOCK	9
#define F_CREATEDIR	1
#define F_DUPLOCK	4
#define F_CURRENTDIR	2
#define F_EXAMINE	5
#define F_EXNEXT	6
#define F_READ		11
#define F_WRITE		12
#define F_SEEK		13
#define F_SETFILESIZE	14
#define F_FGETC		15
#define F_FPUTC		16
#define F_LOCKRECORD	17
#define F_UNLOCKRECORD	18
#define F_INFO		19



struct MsgPort 		*setup_rexx_port();
struct MsgPort 		*rexx_port = NULL;
struct RxsLib *RexxSysBase = NULL;   /* this is the rexx library base */
void reply_rexx_command();
char rx_string[256];
struct MyFileNode
	{
	struct Node		s_node;
	struct FileHandle	*OpenFileHandle;
	struct FileInfoBlock	*fib;
	};
	
	struct List		MyFileList =
		{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
		};




char *AC[] =
	{
	"CLOSE",
	"CREATEDIR",
	"CURRENTDIR",
	"DELETEFILE",
	"DUPLOCK",
	"EXAMINE",
	"EXNEXT",
	"GETLOCK",
	"OPEN",
	"UNLOCK",
	"QUIT",
	"READ",
	"WRITE",
	"SEEK",
	"SETFILESIZE",
	"FGETC",
	"FPUTC",
	"LOCKRECORD",
	"UNLOCKRECORD",
	"INFO"
	};

	



main()
{
	BOOL result;
	long 	rexxbit;
	struct RexxMsg *rexxmessage;   /* incoming rexx messages */

	if ((rexx_port = setup_rexx_port()) == NULL)
	   {
	     byebye(10);
	   }

	NewList( &MyFileList);


	{
	int quit;
	for(quit = 0;!quit;)
		{
		Wait(1L<<rexx_port->mp_SigBit); 
		/* printf("done waiting \n"); */
		while(rexxmessage = (struct RexxMsg *)GetMsg(rexx_port))
			{
/* printf("we got it \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n"); */
			quit = execute_command(rexxmessage);
/* printf(" quit %ld \n",quit); */
			}
		}
	byebye(0);
	}
}

struct MsgPort *setup_rexx_port()
{
   struct MsgPort *CreatePort();
   struct MsgPort *FindPort();

   struct MsgPort *the_port;

   Forbid();

   /* look for someone else that looks just like us! */
   if (FindPort(HOST_PORT_NAME))
   {
     Permit();
     printf("A public port called '%s' already exists!\n",HOST_PORT_NAME);
     return(NULL);
   }

   /* allocate the port */
   the_port = CreatePort(HOST_PORT_NAME,0L);

   Permit();

   return(the_port);
}

shutdown_rexx_port(rexx_port)
struct MsgPort *rexx_port;
{
   DeletePort(rexx_port);
}

/* Replies a REXX message, filling in the appropriate codes.  If the macro
 * program has requested a result string, the return argstring is allocated
 * and installed in the rm_Result2 slot.
 *
 * A result is returned ONLY IF REQUESTED AND THE PRIMARY RESULT == 0.
 */

void reply_rexx_command(rexxmessage,primary,secondary,result)
struct RexxMsg *rexxmessage;
long           primary,secondary;
char           *result;
{
   /* set an error code */
   if (primary == 0 && (rexxmessage->rm_Action & 1L<<RXFB_RESULT)) {
      secondary = result ? (long) CreateArgstring(result,strlen(result))
                         : (long) NULL;
      }
   rexxmessage->rm_Result1 = primary;
   rexxmessage->rm_Result2 = secondary;
   ReplyMsg(rexxmessage);
}



execute_command(rexxmessage)
struct RexxMsg *rexxmessage;
{
   long primary=0,secondary=0;
   int  done = 0;
   long s_length;
   char *rexxvar;
   long r_variable;
   char *argstring1;
   char *argstring2;
   ULONG arg1;
   ULONG arg2;
   ULONG arg3;
   ULONG arg4;
   ULONG arg5;
   struct Lock *Llock;
   struct Lock *Llock1;
   struct MyFileNode *fn;
   int i;

/* printf("got '%s' from rexx\n",rexxmessage->rm_Args[0]); */
	
	

for(i=0; (i < (sizeof(AC)/sizeof(*AC))) && (strcmp(rexxmessage->rm_Args[0],AC[i])) ; i++);
if(i == sizeof(AC))
	{
	primary = 30;
	}
else


	switch(i)
		{
		case QUIT:
			done = 1;		
			break;
		case F_OPEN:
			primary = GetRexxVar(rexxmessage,"MODE",&argstring1);
			primary |= GetRexxVar(rexxmessage,"NAME",&argstring2);
			stcd_l(argstring1,&arg1); /* access mode */
			if (!(fn = AllocMem((LONG)sizeof (struct MyFileNode), 0L )) )
				{
				primary = 10;
				break;
				}
			s_length = sprintf(&rx_string[0],"%d", 0 );
			primary |= SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
			if(!primary)
				{
				if(!(fn->OpenFileHandle = Open(argstring2,arg1)))
					{
					FreeMem( fn, (long) sizeof *fn );
					arg1 = IoErr();
					s_length = sprintf(&rx_string[0],"%d", arg1 );
					SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
					primary = 10;
					break;
					}

				AddTail(&MyFileList, fn);
				s_length = sprintf(&rx_string[0],"%d", fn->OpenFileHandle );
				primary = SetRexxVar(rexxmessage,"FILEHANDLE",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d", fn );
				primary |= SetRexxVar(rexxmessage,"FILENUMBER",&rx_string[0],s_length);
				}
			break;

		case F_CLOSE:
			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			Close(((struct MyFileNode *) arg1)->OpenFileHandle);
			Remove((struct Node *) arg1);
			FreeMem( (struct Node *) arg1, (long) sizeof *fn );
			break;			

		case F_READ:
			{
			char *RWbuffer;
			LONG ActualLength;

			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"LENGTH",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* length  */
			
			if(!(RWbuffer = AllocMem(arg2,MEMF_PUBLIC|MEMF_CLEAR)))
				{
				primary = 30;
				break;
				}
			
			ActualLength = Read(((struct MyFileNode *) arg1)->OpenFileHandle,RWbuffer,arg2);
			if (ActualLength == -1)
				{
				arg1 = IoErr();
				s_length = sprintf(&rx_string[0],"%d", arg1 );
				SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
				primary = 10;
				FreeMem(RWbuffer,arg2);
				break;
				}
			
			primary = SetRexxVar(rexxmessage,"BUFFER",RWbuffer,ActualLength);
			s_length = sprintf(&rx_string[0],"%d", ActualLength );
			primary |= SetRexxVar(rexxmessage,"RES_LENGTH",&rx_string[0],s_length);
			FreeMem(RWbuffer,arg2);
			break;
			}
		case F_SEEK:
			{
			LONG oldPosition;

			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"POSITION",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* length  */
			primary = GetRexxVar(rexxmessage,"MODE",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg3); /* length  */
			oldPosition = Seek(((struct MyFileNode *) arg1)->OpenFileHandle,arg2,arg3);
			if (oldPosition == -1)
				{
				arg1 = IoErr();
				s_length = sprintf(&rx_string[0],"%d", arg1 );
				SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
				primary = 10;
				break;
				}
			s_length = sprintf(&rx_string[0],"%d", oldPosition );
			primary |= SetRexxVar(rexxmessage,"RES_POSITION",&rx_string[0],s_length);
			break;
			}
		case F_LOCKRECORD:
			{
			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"OFFSET",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* offset  */
			primary = GetRexxVar(rexxmessage,"LENGTH",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg3); /* lemgth  */
			primary = GetRexxVar(rexxmessage,"MODE",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg4); /* mode  */
			primary = GetRexxVar(rexxmessage,"TIMEOUT",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg4); /* timeout  */

					
			arg1 = LockRecord(((struct MyFileNode *) arg1)->OpenFileHandle,arg2,arg3,arg4,arg5);
			s_length = sprintf(&rx_string[0],"%d", arg1 );
			SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
			if(!arg1)
				primary = 10;
			break;
			}
		case F_UNLOCKRECORD:
			{
			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"OFFSET",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* offset  */
			primary = GetRexxVar(rexxmessage,"LENGTH",&argstring1);
					
			arg1 = UnLockRecord(((struct MyFileNode *) arg1)->OpenFileHandle,arg2,arg3);
			s_length = sprintf(&rx_string[0],"%d", arg1 );
			SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
			if(!arg1)
				primary = 10;
			break;

			}

		case F_SETFILESIZE:

			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"OFFSET",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* offset  */
			primary = GetRexxVar(rexxmessage,"MODE",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg3); /* mode  */
			arg1 = SetFileSize(((struct MyFileNode *) arg1)->OpenFileHandle,arg2,arg3);
			s_length = sprintf(&rx_string[0],"%d", arg1 );
			SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
			if(!arg1)
				primary = 10;
			break;

		case F_WRITE:
			{
			LONG returnedLength;

			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"LENGTH",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg2); /* length  */
			primary = GetRexxVar(rexxmessage,"BUFFER",&argstring2);
			if (primary)
				break;
			
			returnedLength = Write(((struct MyFileNode *) arg1)->OpenFileHandle,argstring2,arg2);
			if (returnedLength == -1)
				{
				arg1 = IoErr();
				s_length = sprintf(&rx_string[0],"%d", arg1 );
				SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
				primary = 10;
				break;
				}

			s_length = sprintf(&rx_string[0],"%d", returnedLength );
			primary |= SetRexxVar(rexxmessage,"RES_LENGTH",&rx_string[0],s_length);
			break;

			}

		case F_DELETE:
			primary = GetRexxVar(rexxmessage,"NAME",&argstring1);
			if(!primary)
				{
				if(!(DeleteFile(argstring1)))
					{
					arg1 = IoErr();
					s_length = sprintf(&rx_string[0],"%d", arg1 );
					SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
					primary = 10;
					break;
					}
				}
			break;
		case F_LOCK:
			primary = GetRexxVar(rexxmessage,"MODE",&argstring1);
			primary |= GetRexxVar(rexxmessage,"NAME",&argstring2);
			stcd_l(argstring1,&arg1); /* access mode */
			s_length = sprintf(&rx_string[0],"%d", 0 );
			primary |= SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
			if(!primary)
				{
				if(!(Llock = Lock(argstring2,arg1)))
					{
					s_length = sprintf(&rx_string[0],"%d", 1 );
					SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
					primary = 10;
					break;
					}

				s_length = sprintf(&rx_string[0],"%d", Llock );
				primary |= SetRexxVar(rexxmessage,"RES_LOCK",&rx_string[0],s_length);
				}
			break;

		case F_INFO:
			{
			struct InfoData *parameterBlock = NULL;
			struct FileLock *testlock = NULL;

			if(!(parameterBlock =  AllocMem(sizeof(*parameterBlock), MEMF_PUBLIC)))
				{
				primary = 30;
				break;
				}

			
			if(primary = GetRexxVar(rexxmessage,"NAME",&argstring2))
				break;
			testlock = Lock(argstring2,ACCESS_READ);
			if(!testlock) 
				{
				printf("Could not get Lock on %s \n",argstring2);
				FreeMem(parameterBlock,sizeof(*parameterBlock));

				primary = 30;
				break;
				}
			
			if(!Info(testlock,parameterBlock))
				{
				printf("Could not get Info on %s \n",argstring2);
				FreeMem(parameterBlock,sizeof(*parameterBlock));
				s_length = sprintf(&rx_string[0],"%d", 0 );
				primary = SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
				UnLock(testlock);
				break;
				}



			s_length = sprintf(&rx_string[0],"%d",parameterBlock->id_NumBlocks);
			primary |= SetRexxVar(rexxmessage,"NUM_BLOCKS",&rx_string[0],s_length);
			s_length = sprintf(&rx_string[0],"%d",parameterBlock->id_NumBlocksUsed);
			primary |= SetRexxVar(rexxmessage,"NUM_BLOCKS_USED",&rx_string[0],s_length);
			s_length = sprintf(&rx_string[0],"%d",parameterBlock->id_BytesPerBlock);
			primary |= SetRexxVar(rexxmessage,"BYTES_BLOCK",&rx_string[0],s_length);

			UnLock(testlock);
			FreeMem(parameterBlock,sizeof(*parameterBlock));
			break;
			}



		case F_UNLOCK:
			primary = GetRexxVar(rexxmessage,"LOCK",&argstring1);
			stcd_l(argstring1,&Llock); /* Lock */
			UnLock(Llock);
			break;
	
		case F_CREATEDIR:
			primary = GetRexxVar(rexxmessage,"NAME",&argstring1);
			if(!primary)
				{
				if(!(Llock = CreateDir(argstring1)))
					{
					arg1 = IoErr();
					s_length = sprintf(&rx_string[0],"%d", arg1 );
					SetRexxVar(rexxmessage,"FUNCTION_ERROR",&rx_string[0],s_length);
					primary = 10;
					break;
					}
				else
					{
					s_length = sprintf(&rx_string[0],"%d", Llock );
					primary |= SetRexxVar(rexxmessage,"RES_LOCK",&rx_string[0],s_length);
					}

				}
			break;


		case F_DUPLOCK:
			primary = GetRexxVar(rexxmessage,"LOCK",&argstring1);
			stcd_l(argstring1,&Llock); /* Lock */
			if(Llock = DupLock(Llock))
				{
				s_length = sprintf(&rx_string[0],"%d", Llock );
				primary |= SetRexxVar(rexxmessage,"RES_LOCK",&rx_string[0],s_length);
				}
			else
				primary = 30;
			break;
			
		case F_CURRENTDIR:
			primary = GetRexxVar(rexxmessage,"LOCK",&argstring1);
			stcd_l(argstring1,&Llock); /* Lock */
			Llock = CurrentDir(Llock)
			s_length = sprintf(&rx_string[0],"%d", Llock );
			primary |= SetRexxVar(rexxmessage,"RES_LOCK",&rx_string[0],s_length);
			break;
			
		case F_EXAMINE:
			primary = GetRexxVar(rexxmessage,"LOCK",&argstring1);
			primary |= GetRexxVar(rexxmessage,"LOCKQUIT",&argstring2);
			stcd_l(argstring1,&arg1); /* lock */
			stcd_l(argstring2,&arg2); /* quit */
			if (!(fn = AllocMem((LONG)sizeof (struct MyFileNode), 0L )) )
				{
				primary = 10;
				break;
				}
			if (!(fn->fib = AllocMem((LONG)sizeof (struct FileInfoBlock), 0L )) )
				{
				primary = 10;
				break;
				}
			if(!primary)
				{
				if(!(arg1 = Examine(arg1,fn->fib)))
					{
					FreeMem( fn->fib, (long) sizeof (struct FileInfoBlock) );
					FreeMem( fn, (long) sizeof (struct MyFileNode) );
					primary = 30;
					break;
					}

				if(arg2)
					AddTail(&MyFileList, fn);
				s_length = sprintf(&rx_string[0],"%d", fn->fib );
				primary = SetRexxVar(rexxmessage,"RES_INFOBLOCK",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d", fn );
				primary |= SetRexxVar(rexxmessage,"EXAMINE_NUM",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d", (fn->fib)->fib_DirEntryType );
				primary |= SetRexxVar(rexxmessage,"RES_TYPE",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d",(fn->fib)->fib_Protection  );
				primary |= SetRexxVar(rexxmessage,"RES_PROTECTION",&rx_string[0],s_length);
				s_length = strcpy(&rx_string[0],(fn->fib)->fib_FileName);
				primary |= SetRexxVar(rexxmessage,"RES_FILENAME",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d",(fn->fib)->fib_Size  );
				primary |= SetRexxVar(rexxmessage,"RES_SIZE",&rx_string[0],s_length);
				if(!arg2)
					{
					FreeMem( fn->fib, (long) sizeof (struct FileInfoBlock) );
					FreeMem( fn, (long) sizeof (struct MyFileNode) );
					}
				}
				break;
		

		case F_EXNEXT:
			primary = GetRexxVar(rexxmessage,"LOCK",&argstring1);
			primary |= GetRexxVar(rexxmessage,"QUIT",&argstring2);
			primary |= GetRexxVar(rexxmessage,"EXNUM",&argstring2);
			stcd_l(argstring1,&arg1); /* lock */
			stcd_l(argstring2,&arg2); /* quit */
			stcd_l(argstring2,&arg3); /* exnum */
			fn = arg3;
		
			if(!(arg1 = ExNext(arg1,fn->fib)))
				{
				s_length = sprintf(&rx_string[0],"%d", fn->fib );
				primary = SetRexxVar(rexxmessage,"RES_INFOBLOCK",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d", fn );
				primary |= SetRexxVar(rexxmessage,"EXAMINE_NUM",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d", (fn->fib)->fib_DirEntryType );
				primary |= SetRexxVar(rexxmessage,"RES_TYPE",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d",(fn->fib)->fib_Protection  );
				primary |= SetRexxVar(rexxmessage,"RES_PROTECTION",&rx_string[0],s_length);
				s_length = strcpy(&rx_string[0],(fn->fib)->fib_FileName);
				primary |= SetRexxVar(rexxmessage,"RES_FILENAME",&rx_string[0],s_length);
				s_length = sprintf(&rx_string[0],"%d",(fn->fib)->fib_Size  );
				primary |= SetRexxVar(rexxmessage,"RES_SIZE",&rx_string[0],s_length);

				}
			else
				primary = 30;


			if(!arg2)
				{
				FreeMem( fn->fib, (long) sizeof (struct FileInfoBlock) );
				FreeMem( fn, (long) sizeof (struct MyFileNode) );
				}
			break;

		case F_FGETC:
			{
			LONG ReadChar;
			char TheChar;
			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			ReadChar = FGetC(((struct MyFileNode *) arg1)->OpenFileHandle);
			if (ReadChar == -1)
				{
				primary = 10;
				break;
				}
			TheChar = (char) ReadChar;
			primary = SetRexxVar(rexxmessage,"RCHAR",&TheChar,1);
			break;
			}
		case F_FPUTC:
			{
			BOOL result;
			primary = GetRexxVar(rexxmessage,"FILENUMBER",&argstring1);
			if (primary)
				break;
			stcd_l(argstring1,&arg1); /* file number */
			primary = GetRexxVar(rexxmessage,"WCHAR",&argstring1);
			if (primary)
				break;
			result = FPutC(((struct MyFileNode *) arg1)->OpenFileHandle,*argstring1);
			if (result != TRUE)
				{
				primary = 10;
				break;
				}
			break;
			}


				
	}	

	

   reply_rexx_command(rexxmessage,primary,secondary,NULL);
   return(done);
}



byebye(rcode)
long rcode;
	{
	struct MyFileNode *fn;

	printf("later....");
	if (rexx_port)
		shutdown_rexx_port(rexx_port);

	while (fn = (struct MyFileNode *) RemHead( &MyFileList ))
			{
		
			FreeMem( fn, (long) sizeof *fn );
			}
	

	exit(rcode);
	}
