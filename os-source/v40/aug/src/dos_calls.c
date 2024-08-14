#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/record.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/* normally pragmas would be included here */

#include <stdio.h>
#include <string.h>

#define SAME 0

/* so the compiler doesn't get upset */
void help(int *done);
void quit(int *done);
void source(int *done);
void showbuffer(int *done);
void open(int *done);
void close(int *done);
void showfiles(int *done);
void lock(int *done);
void unlock(int *done);
void duplock(int *done);
void showlocks(int *done);
void myread(int *done);
void mywrite(int *done);
void lockrecord(int *done);
void unlockrecord(int *done);
void myfgets(int *done);
void myfputs(int *done);
void myfgetc(int *done);
void myfputc(int *done);
void myfread(int *done);
void myfwrite(int *done);
void myseek(int *done);
void myflush(int *done);
void dumpfh(int *done);
void dumplock(int *done);
void samelock(int*done);
void execute(int *done);
void deletefile(int *done);
void splitname(int *done);
void findvar(int *done);
void duplockfromfh(int *done);
void namefromlock(int *done);
void namefromfh(int *done);
void parentdir(int *done);
void parentoffh(int *done);
void examine(int *done);
void exnext(int *done);
void parsepattern(int *done);
void matchpattern(int *done);
void parsepatternnocase(int *done);
void matchpatternnocase(int *done);
void changemode(int *done);
void openfromlock(int *done);
void setfilesize(int *done);
void isfilesystem(int *done);
void getvar(int *done);
void setvar(int *done);
void setbuffer(int *done);
void loadseg(int *done);
void unloadseg(int *done);
void relabel(int *done);
void runcommand(int *done);
void assignlock(int *done);
/* more commands can be _easily_ added */

struct commands {
	char *name;
	char *helpstr;
	void (*rtn)(int *done);
} comm[] = {
	"?"," - list all commands",help,
	"quit"," - exit this program",quit,
	"Open","<filename> <mode> - open a file",open,
	"Close","<file> - close a file",close,
	"Read","<file> <offset> <length> - read bytes from file",myread,
	"Write","<file> <offset> <length> - write bytes from file",mywrite,
	"Seek","<file> <offset> <mode> - seek to position in file",myseek,
	"SetFileSize","<file> <offset> <mode> - set the size of the file",
		setfilesize,
	"Lock","<name> <mode> - lock an object",lock,
	"UnLock","<lock> - unlock a lock",unlock,
	"DupLock","<lock> - duplicate a lock",duplock,
	"SameLock","<lock> <lock> - determine if two locks are on same object",
		samelock,
	"ParentDir","<lock> - get a lock on the parent of a lock",parentdir,
	"OpenFromLock","<lock> - open a filehandle on the locked item",
		openfromlock,
	"Examine","<lock> <offset> - examine a lock",examine,
	"ExNext","<lock> <offset> - examine next file/dir",exnext,
	"Relabel","<disk> <name> - relabel a disk",relabel,
	"ParentOfFH","<file> - returns lock on parent dir of object",parentoffh,
	"DupLockFromFH","<number> - get a lock on open file",duplockfromfh,
	"NameFromLock","<lock> <offset> <length> - get name of object",
		namefromlock,
	"NameFromFH","<file> <offset> <length> - get name of object",
		namefromfh,
	"LockRecord","<file> <offset> <length> <mode> <timeout>- lock a part of a file",
		lockrecord,
	"UnlockRecord","<file> <offset> <length> - unlock a part of a file",
		unlockrecord,
	"ChangeMode","[FH|Lock] <#> <newmode> - change mode of item",
		changemode,
	"FGets","<file> <offset> <length> - buffered read to len or \\n",
		myfgets,
	"FPuts","<file> <offset>|<string> - buffered write of string",myfputs,
	"FGetC","<file> <offset> - buffered character read",myfgetc,
	"FPutC","<file> <offset>|<'char'> - buffered character write",myfputc,
	"FRead","<file> <offset> <blocklen> <blocks> - buffered read by blocks",
		myfread,
	"FWrite","<file> <offset> <blocklen> <blocks> - buffered write by blocks",
		myfwrite,
	"Flush","<file> - flush IO buffers for file",myflush,
	"Execute","<command> <input file> <output file>",execute,
	"RunCommand","<command> <stack> <args>",runcommand,
	"DeleteFile","<filename> - delete a file or directory",deletefile,
	"SplitName","<string> <'char'> <offset> <old pos> <size>",splitname,
	"GetVar","<name> <offset> <size> <flags> - get a variable's value",
		getvar,
	"SetVar","<name> <offset> <size> <flags> - set a variable's value",
		setvar,
	"FindVar","<name> <type> - find a local variable",findvar,
	"ParsePattern","<pattern> <offset> <len> - parse a pattern",
		parsepattern,
	"MatchPattern","<offset> <string> - match against a pattern",
		matchpattern,
	"ParsePatternNoCase","<pattern> <offset> <len> - parse a pattern",
		parsepatternnocase,
	"MatchPatternNoCase","<offset> <string> - match against a pattern",
		matchpatternnocase,
	"IsFileSystem","<string> - determine if a handler is a filesystem",
		isfilesystem,
	"LoadSeg","<file> - load an executable",loadseg,
	"UnLoadSeg","0x<addr> - unload an executable",unloadseg,
	"AssignLock","<name> <lock> - make an assignment",assignlock,
	"DumpFH","<file> - show filehandle",dumpfh,
	"DumpLock","<lock> - show filelock",dumplock,
	"ShowFiles"," - show all open files",showfiles,
	"ShowLocks"," - show all locks",showlocks,
	"Source","<file> - read commands from file",source,
	"SetBuffer","[offset] [hex <hex digits>]|<string>",setbuffer,
	"ShowBuffer","[hex] [offset [length]] - show part of buffer",
		showbuffer,
};

extern struct DosLibrary *DOSBase;

/* filehandle for input */
FILE *fp = stdin;

int
main (argc,argv)
	int argc;
	char **argv;
{
	char buff[1024];
	int done = FALSE,i,len;
	char *command;

	if (argc != 1)
	{
		printf("usage: %s\n",argv[0]);
		exit(10);
	}

	if (DOSBase->dl_lib.lib_Version < 37)
	{
		printf("You need a dos.library V37 or better!\n");
		exit(20);
	}

	printf("%s 2.0 - use ? to get a list of commands\n\n",argv[0]);
	printf("\tWarning: there is little error checking in this program.\n");
	printf("You HAVE been warned!\n");

	while (!done)
	{
		if (fp == stdin)
		{
			printf("%s> ",argv[0]);
			fflush(stdout);
		}
		if (fgets(buff,sizeof(buff),fp) == NULL)
		{
			if (fp == stdin)
				break;			/* eof */
			else {
				fp = stdin;		/* end of 'source' */
				continue;
			}
		}

		len = strlen(buff);		/* kill \n */
		if (len && buff[len-1] == '\n')
			buff[len-1] = '\0';

		command = strtok(buff," ");
		if (!command || !*command)		/* no command */
			continue;

		for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
		{
			if (stricmp(command,comm[i].name) == SAME)
			{
				/* found it - execute */

				(*(comm[i].rtn))(&done);
				break;
			}
		}
		if (i >= sizeof(comm)/sizeof(comm[0]))	/* failure */
			printf("Error: command %s not known!\n",command);
	}

	return 0;
}

void
help (int *done)
{
	int i;

	for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
	{
		printf(" %s %s\n",comm[i].name,comm[i].helpstr);
	}
	printf("\n <file> or <lock> refers to an index into the arrays\n");
	printf(" <offset> refers to an offset into the buffer\n");
}

void
quit (int *done)
{
	*done = TRUE;
}

void
source (int *done)
{
	char *arg;
	FILE *myfp;

	arg = strtok(NULL," ");
	if (!arg)
	{
		printf("error: use source <filename>\n");
		return;
	}

	myfp = fopen(arg,"r");
	if (!myfp)
	{
		printf("error: Can't open %s!\n",arg);
		return;
	}

	fp = myfp;
}

unsigned char buffer[2048];

void
showbuffer (int *done)
{
	char *arg;
	int i;
	int hex = FALSE;
	long offset = 0;
	long len    = 80;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"hex") == SAME)
		{
			hex = TRUE;
			arg = strtok(NULL," ");
		}
	}

	/* optional offset */
	if (arg)
	{
		if (strlen(arg) != stcd_l(arg,&offset))
		{
err:		    printf("error: bad number (%s)\n",arg);
		    printf("error: use showbuffer [hex] [offset [length]]\n");
		    return;
		}

		arg = strtok(NULL," ");
		if (arg)
		{
			if (strlen(arg) != stcd_l(arg,&len))
			{
				goto err;
			}
		}
	}

	for (i = 0; i < len; i++)
	{
		if (!hex)
		{
			if (!buffer[offset+i])
				break;
			putchar(buffer[offset+i]);
		} else {
			if (i % 20 == 0)
			{
				printf("\n%04ld: ",i+offset);
			}
			printf("%02x ",buffer[offset+i]);
		}
	}
	putchar('\n');
}

void
setbuffer (int *done)
{
	char *str;
	long offset;
	char hexdigit[4];
	long digit;

	str = strtok(NULL," ");
	if (!str)
	{
err:	    printf("Usage: SetBuffer [offset] [hex <hex digits>]|<string>\n");
	    return;
	}
	if (strlen(str) != stcd_l(str,&offset))
	{
		offset = 0;
	} else {
		/* got offset, get next string */
		str = strtok(NULL," ");
		if (!str)
			goto err;
	}

	if (stricmp(str,"hex") == SAME)
	{
		str = strtok(NULL," ");
		if (!str)
			goto err;

		while (*str)
		{
			hexdigit[0] = *str++;
			if (!*str)
				goto err;

			hexdigit[1] = *str++;
			hexdigit[2] = 0;

			if (stch_l(hexdigit,&digit) != 2)
			{
				printf("Bad hex value '%s'!\n",hexdigit);
				return;
			}

			buffer[offset++] = digit;
		}
	} else {
		/* string */
		strcpy(&(buffer[offset]),str);
	}
}

/* end standard code */

/* data */

BPTR files[10];
BPTR locks[10];
char *lastarg;		/* for getnum failures */

int
getnum (long *num)
{
	lastarg = strtok(NULL," ");
	if (!lastarg)
		return FALSE;

	if (strlen(lastarg) != stcd_l(lastarg,num))
	{
		printf("error: bad number (%s)\n",lastarg);
		return FALSE;
	}

	return TRUE;
}

int
get2nums (long *num1, long *num2)
{
	if (!getnum(num1))
		return FALSE;

	return getnum(num2);
}

int
get3nums (long *num1, long *num2, long *num3)
{
	if (!getnum(num1) || !getnum(num2))
		return FALSE;

	return getnum(num3);
}

/* open file */
void
open (int *done)
{
	char *name,*modestr;
	LONG mode;
	BPTR fh;
	int i;

	name = strtok(NULL," ");
	if (!name)
	{
err:		printf("error: use open <filename> <mode>\n");
		return;
	}

	modestr = strtok(NULL," ");
	if (!modestr)
		goto err;

	if (stricmp(modestr,"mode_oldfile") == SAME)
		mode = MODE_OLDFILE;
	else if (stricmp(modestr,"mode_newfile") == SAME)
		mode = MODE_NEWFILE;
	else
		if (strlen(modestr) != stcd_l(modestr,&mode))
		{
			printf("error: bad number (%s)\n",modestr);
			return;
		}

	fh = Open(name,mode);
	if (!fh)
	{
		printf("error: Can't open %s, ioerr = %ld\n",name,IoErr());
		return;
	}

	for (i = 0; i < sizeof(files)/sizeof(files[0]); i++)
	{
		if (files[i] == NULL)
		{
			files[i] = fh;
			printf("Opened as file %d\n",i);
			return;
		}
	}

	printf("no space for more files!\n");
	Close(fh);
}

void
close (int *done)
{
	int num;

	if (!getnum(&num))
	{
		printf("error: use close <filenumber>\n");
		return;
	}

	Close(files[num]);
	files[num] = NULL;
}

void
openfromlock (int *done)
{
	BPTR fh;
	int i;

	if (!getnum(&i))
	{
		printf("error: use OpenFromLock <lock>\n");
		return;
	}

	fh = OpenFromLock(locks[i]);
	if (!fh)
	{
		printf("error: Can't open from lock %d (0x%lx), IoErr = %ld\n",
			i,locks[i],IoErr());
		return;
	}
	locks[i] = NULL;

	for (i = 0; i < sizeof(files)/sizeof(files[0]); i++)
	{
		if (files[i] == NULL)
		{
			files[i] = fh;
			printf("Opened as file %d\n",i);
			return;
		}
	}

	printf("no space for more files!\n");
	Close(fh);
}

void
showfiles (int *done)
{
	int i;

	printf("\tNumber\t   FH\n");
	for (i = 0; i < sizeof(files)/sizeof(files[0]); i++)
	{
		if (files[i])
			printf("\t%d\t- 0x%lx\n",i,files[i]);
	}
}

/* open file */
void
lock (int *done)
{
	char *name,*modestr;
	LONG mode;
	BPTR lock;
	int i;

	name = strtok(NULL," ");
	if (!name)
	{
err:		printf("error: use lock <name> <mode>\n");
		return;
	}

	modestr = strtok(NULL," ");
	if (!modestr)
		goto err;

	if (stricmp(modestr,"shared") == SAME)
		mode = SHARED_LOCK;
	else if (stricmp(modestr,"exclusive") == SAME)
		mode = EXCLUSIVE_LOCK;
	else
		if (strlen(modestr) != stcd_l(modestr,&mode))
		{
			printf("error: bad number (%s)\n",modestr);
			return;
		}

	lock = Lock(name,mode);
	if (!lock)
	{
		printf("error: Can't lock %s, ioerr = %ld\n",name,IoErr());
		return;
	}

	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i] == NULL)
		{
			locks[i] = lock;
			printf("Locked as lock %d\n",i);
			return;
		}
	}

	printf("no space for more locks!\n");
	UnLock(lock);
}

void
unlock (int *done)
{
	int num;

	if (!getnum(&num))
	{
		printf("error: use unlock <locknumber>\n");
		return;
	}

	UnLock(locks[num]);
	locks[num] = NULL;
}

void
duplock (int *done)
{
	int num,i;
	BPTR lock;

	if (!getnum(&num))
	{
		printf("error: use duplock <locknumber>\n");
		return;
	}

	lock = DupLock(locks[num]);
	if (!lock)
	{
		printf("error: Can't duplock 0x%lx, ioerr = %ld\n",
			locks[num],IoErr());
		printf(" (note: DupLock(0) == 0)\n");
		return;
	}


	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i] == NULL)
		{
			locks[i] = lock;
			printf("Locked as lock %d\n",i);
			return;
		}
	}

	printf("no space for more locks!\n");
	UnLock(lock);
}

void
parentdir (int *done)
{
	int num,i;
	BPTR lock;

	if (!getnum(&num))
	{
		printf("error: use ParentDir <locknumber>\n");
		return;
	}

	lock = ParentDir(locks[num]);
	if (!lock)
	{
		printf("error: Can't get parent of 0x%lx, ioerr = %ld\n",
			locks[num],IoErr());
		return;
	}


	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i] == NULL)
		{
			locks[i] = lock;
			printf("Locked as lock %d\n",i);
			return;
		}
	}

	printf("no space for more locks!\n");
	UnLock(lock);
}

void
duplockfromfh (int *done)
{
	int num,i;
	BPTR lock;

	if (!getnum(&num))
	{
		printf("error: use DupLockFromFH <filenumber>\n");
		return;
	}

	lock = DupLockFromFH(files[num]);
	if (!lock)
	{
		printf("error: Can't duplockfromfh 0x%lx, ioerr = %ld\n",
			files[num],IoErr());
		return;
	}


	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i] == NULL)
		{
			locks[i] = lock;
			printf("Locked as lock %d\n",i);
			return;
		}
	}

	printf("no space for more locks!\n");
	UnLock(lock);
}

void
parentoffh (int *done)
{
	int num,i;
	BPTR lock;

	if (!getnum(&num))
	{
		printf("error: use ParentOfFH <filenumber>\n");
		return;
	}

	lock = ParentOfFH(files[num]);
	if (!lock)
	{
		printf("error: Can't get parent of 0x%lx, ioerr = %ld\n",
			files[num],IoErr());
		return;
	}


	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i] == NULL)
		{
			locks[i] = lock;
			printf("Locked as lock %d\n",i);
			return;
		}
	}

	printf("no space for more locks!\n");
	UnLock(lock);
}

void
examine (int *done)
{
	struct FileInfoBlock *fib;
	LONG num,offset,rc;

	if (!getnum(&num))
	{
err:		printf("error: use Examine <lock> <offset>\n");
		return;
	}

	if (!getnum(&offset))
		goto err;

	fib = (void *) &buffer[offset];

	rc = Examine(locks[num],fib);
	printf("Examine returned %ld, ioerr = %ld\n",rc,IoErr());
}

void
exnext (int *done)
{
	struct FileInfoBlock *fib;
	LONG num,offset,rc;

	if (!getnum(&num))
	{
err:		printf("error: use ExNext <lock> <offset>\n");
		return;
	}

	if (!getnum(&offset))
		goto err;

	fib = (void *) &buffer[offset];

	rc = ExNext(locks[num],fib);
	printf("ExNext returned %ld, ioerr = %ld\n",rc,IoErr());
}

void
showlocks (int *done)
{
	int i;

	printf("\tNumber\t   Lock\n");
	for (i = 0; i < sizeof(locks)/sizeof(locks[0]); i++)
	{
		if (locks[i])
			printf("\t%d\t- 0x%lx\n",i,locks[i]);
	}
}

void
myread (int *done)
{
	long offset,length,file,len;

	if (!get3nums(&file,&offset,&length))
	{
		printf("error: use read <filenumber> <offset> <length>\n");
		return;
	}

	len = Read(files[file],&(buffer[offset]),length);
	if (len < 0)
	{
		printf("error on read - %ld\n",IoErr());
		return;
	}

	printf("Read %ld characters\n",len);
}

void
mywrite (int *done)
{
	long offset,length,file,len;

	if (!get3nums(&file,&offset,&length))
	{
		printf("error: use write <filenumber> <offset> <length>\n");
		return;
	}

	len = Write(files[file],&(buffer[offset]),length);
	if (len < 0)
	{
		printf("error on write - %ld\n",IoErr());
		return;
	}

	printf("Wrote %ld characters\n",len);
}

void
myseek (int *done)
{
	long offset,mode,file,oldpos;

	if (!get3nums(&file,&offset,&mode))
	{
		printf("error: use seek <filenumber> <offset> <mode>\n");
		return;
	}

	oldpos = Seek(files[file],offset,mode);
	if (oldpos < 0)
	{
		printf("error on seek - %ld\n",IoErr());
		return;
	}

	printf("old position was %ld\n",oldpos);
}

void
setfilesize (int *done)
{
	long offset,mode,file,oldpos;

	if (!get3nums(&file,&offset,&mode))
	{
		printf("error: use SetFileSize <filenumber> <offset> <mode>\n");
		return;
	}

	oldpos = SetFileSize(files[file],offset,mode);
	if (oldpos < 0)
	{
		printf("error on setfilesize - %ld\n",IoErr());
		return;
	}

	printf("old position was %ld, current position is %ld\n",oldpos,
		Seek(files[file],0,OFFSET_CURRENT));
}

void
lockrecord (int *done)
{
	long offset,length,file,res,mode,timeout = 50;

	if (!get3nums(&file,&offset,&length) ||
	    !get2nums(&mode,&timeout))
	{
		printf(
 "error: use lockrecord <filenumber> <offset> <length> <mode> <timeout>\n");
		return;
	}

	res = LockRecord(files[file],offset,length,mode,timeout);
	if (res == 0)
	{
		printf("error on lock - %ld\n",IoErr());
		return;
	}

	printf("Locked (%ld,%ld)\n",res,IoErr());
}

void
unlockrecord (int *done)
{
	long offset,length,file,res;

	if (!get3nums(&file,&offset,&length))
	{
	 printf("error: use unlockrecord <filenumber> <offset> <length>\n");
	 return;
	}

	res = UnLockRecord(files[file],offset,length);
	if (res == 0)
	{
		printf("error on unlock - %ld\n",IoErr());
		return;
	}

	printf("Unlocked\n");
}

void
myfgets (int *done)
{
	long offset,length,file;
	char *p;

	if (!get3nums(&file,&offset,&length))
	{
		printf("error: use fgets <filenumber> <offset> <length>\n");
		return;
	}

	p = (char *) FGets(files[file],&(buffer[offset]),length);
	if (p == 0)
	{
		printf("eof or error on FGets - %ld\n",IoErr());
		return;
	}

	printf("Read %ld characters\n",strlen(p));
}

void
myfputs (int *done)
{
	long offset,file;
	unsigned char *p;

	if (!getnum(&file))
	{
err:		printf("error: use fputs <filenumber> <offset>|<string>\n");
		return;
	}
	if (!(p = strtok(NULL," ")))
		goto err;

	if (strlen(p) == stcd_l(p,&offset))
	{
		p = &(buffer[offset]);
	}

	if (FPuts(files[file],p))
	{
		printf("eof or error on FPuts - %ld\n",IoErr());
		return;
	}

	printf("Wrote %ld characters\n",strlen(p));
}

void
myfgetc (int *done)
{
	long offset,file;
	long p;

	if (!get2nums(&file,&offset))
	{
		printf("error: use fgetc <filenumber> <offset>\n");
		return;
	}

	p = FGetC(files[file]);
	if (p == -1)
	{
		printf("eof on FGetC - %ld\n",IoErr());
		return;
	}

	buffer[offset] = p;

	printf("Read 0x%lx\n",p);
}

void
myfputc (int *done)
{
	long offset,file;
	unsigned char *p;

	if (!getnum(&file))
	{
err:		printf("error: use fputc <filenumber> <offset>|<'char'>\n");
		return;
	}
	if (!(p = strtok(NULL," ")))
		goto err;

	if (strlen(p) == stcd_l(p,&offset))
	{
		p = &(buffer[offset]);
	}

	FPutC(files[file],*p);

	printf("Wrote 0x%lx\n",*p);
}

void
myfread (int *done)
{
	long offset,length,file,blocks;
	LONG rc;

	if (!get3nums(&file,&offset,&length))
	{
err:		printf(
	      "error: use FRead <filenumber> <offset> <blocklen> <blocks>\n");
		return;
	}
	if (!getnum(&blocks))
		goto err;

	rc = FRead(files[file],&(buffer[offset]),length,blocks);
	printf("FRead %ld blocks\n",rc);
}

void
myfwrite (int *done)
{
	long offset,length,file,blocks;
	LONG rc;

	if (!get3nums(&file,&offset,&length))
	{
err:		printf(
	      "error: use FWrite <filenumber> <offset> <blocklen> <blocks>\n");
		return;
	}
	if (!getnum(&blocks))
		goto err;

	rc = FWrite(files[file],&(buffer[offset]),length,blocks);
	printf("FWrite %ld blocks\n",rc);
}

void
myflush (int *done)
{
	long file;

	if (!getnum(&file))
	{
		printf("error: use flush <filenumber>\n");
		return;
	}

	Flush(files[file]);

	printf("flushed\n");
}

void
dumpfh (int *done)
{
	long file;
	struct FileHandle *fh;

	if (!getnum(&file))
	{
		printf("error: use dumpfh <filenumber>\n");
		return;
	}

	fh = (void *) BADDR(files[file]);
	printf("link = 0x%lx, port = 0x%lx, type = 0x%lx\n",
		fh->fh_Link,fh->fh_Port,fh->fh_Type);
	printf("buf = 0x%lx, pos = %ld, end = %ld\n",
		fh->fh_Buf,fh->fh_Pos,fh->fh_End);
	printf("func1 = 0x%lx, func2 = 0x%lx, func3 = 0x%lx\n",
		fh->fh_Func1,fh->fh_Func2,fh->fh_Func3);
	printf("fh_Arg1 = 0x%lx, fh-Arg2 = 0x%lx\n",
		fh->fh_Arg1,fh->fh_Arg2);
}

void
dumplock (int *done)
{
	long num;
	struct FileLock *lock;

	if (!getnum(&num))
	{
		printf("error: use dumplock <locknumber>\n");
		return;
	}

	lock = (void *) BADDR(locks[num]);
	if (!lock)
	{
		printf("lock on boot fs root (NULL)\n");
		return;
	}
	printf("link = 0x%lx, key = 0x%lx, access = 0x%lx\n",
		lock->fl_Link,lock->fl_Key,lock->fl_Access);
	printf("port = 0x%lx, volume = %ld (%s)\n",
		lock->fl_Task,lock->fl_Volume, (lock->fl_Volume ?
			((char *) BADDR(((struct DosList *)
					BADDR(lock->fl_Volume))->dol_Name))+1 :
			""));
}

void
samelock (int *done)
{
	LONG rc;
	BPTR lock1,lock2;

	if (!get2nums(&lock1,&lock2))
	{
		printf("error: use samelock <lock1> <lock2>\n");
		return;
	}

	rc = SameLock(locks[lock1],locks[lock2]);

	printf("SameLock returned %ld\n",rc);
}

void
changemode (int *done)
{
	LONG rc,type,num,mode;
	char *str;

	str = strtok(NULL," ");
	if (!str)
	{
err:		printf("error: use ChangeMode FH|Lock <number> <newmode>\n");
		return;
	}

	if (stricmp(str,"fh") == SAME)
		type = CHANGE_FH;
	else if (stricmp(str,"lock") == SAME)
		type = CHANGE_LOCK;
	else {
		printf("'%s' is not a valid type\n",str);
		goto err;
	}

	if (!get2nums(&num,&mode))
		goto err;

	rc = ChangeMode(type,type == CHANGE_FH ? files[num] : locks[num],
			mode);

	printf("ChangeMode returned %ld, ioerr = %ld\n",rc,IoErr());
}

void
execute (int *done)
{
	char *command;
	long input,output;
	long rc;
	char buf[256];

	command = strtok(NULL," ");
	if (!command)
	{
err:		printf("error: use execute <command> <input fh> <output fh>\n");
		return;
	}
	if (*command == '"')
	{
		/* strip quotes */
		command++;
		if (command[strlen(command)-1] == '"')
		{
			command[strlen(command)-1] = 0;
		} else {
			strcpy(buf,command);
			do {
				command = strtok(NULL," ");
				if (!command)
					goto err;
				strcat(buf," ");
				strcat(buf,command);
			} while (command[strlen(command)-1] != '"');
			buf[strlen(buf)-1] = 0;
			command = buf;
		}
	}

	if (!get2nums(&input,&output))
		goto err;

	if (input == -1)
		input = NULL;
	else
		input = files[input];

	if (output == -1)
		output = NULL;
	else
		output = files[output];

	printf("Execute(\"%s\",0x%lx,0x%lx)\n",command,input,output);

	rc = Execute(command,input,output);

	printf("Execute returned %ld (ioerr = %ld)\n",rc,IoErr());
}

void
runcommand (int *done)
{
	char *command,*args;
	BPTR seg;
	long stack;
	long rc;
	char buf[256],argbuf[256];

	command = strtok(NULL," ");
	if (!command)
	{
err:
   printf("error: use RunCommand <command> <stack> <args>\n");
		return;
	}
	if (*command == '"')
	{
		/* strip quotes */
		command++;
		if (command[strlen(command)-1] == '"')
		{
			command[strlen(command)-1] = 0;
		} else {
			strcpy(buf,command);
			do {
				command = strtok(NULL," ");
				if (!command)
					goto err;
				strcat(buf," ");
				strcat(buf,command);
			} while (command[strlen(command)-1] != '"');
			buf[strlen(buf)-1] = 0;
			command = buf;
		}
	}

	if (!getnum(&stack))
		goto err;

	argbuf[0] = 0;
	while (args = strtok(NULL," "))
	{
		strcat(argbuf,args);
		strcat(argbuf," ");
	}
	if (argbuf[0])
		argbuf[strlen(argbuf)-1] = '\n';
	args = argbuf;

	seg = LoadSeg(command);
	if (!seg)
	{
		printf("can't load %s!\n",command);
		goto err;
	}

	printf("RunCommand(0x%lx,%ld,\"%s\",%ld)\n",seg,stack,args,
		strlen(args));

	rc = RunCommand(seg,stack,args,strlen(args));

	printf("RunCommand returned %ld (ioerr = %ld)\n",rc,IoErr());
}

void
deletefile (int *done)
{
	LONG rc;

	rc = DeleteFile(strtok(NULL," "));

	printf("DeleteFile returned %ld, ioerr = %ld\n",rc,IoErr());
}

void
splitname (int *done)
{
	char *string,*separator;
	unsigned char *buf;
	LONG size,pos,offset;

	string = strtok(NULL," ");
	if (!string)
	{
err:	printf("usage: SplitName <string> <'char'> <offset> <old pos> <size>\n");
	return;
	}

	separator = strtok(NULL," ");
	if (!separator)
		goto err;

	if (!getnum(&offset))
		goto err;

	buf = &(buffer[offset]);

	if (!getnum(&pos))
		goto err;

	if (!getnum(&size))
		goto err;

	printf("SplitName(%s,%lc,0x%lx,%ld,%ld)\n",
		string,*separator,buf,pos,size);
	pos = SplitName(string,*separator,buf,(UWORD) pos,size);
	printf("  returned %ld (ioerr = %ld)\n",pos,IoErr());

}

void
getvar (int *done)
{
	char *name;
	LONG offset,size,flags,len;

	name = strtok(NULL," ");
	if (!name)
	{
err:		printf("usage: GetVar <name> <offset> <size> <flags>\n");
		return;
	}

	if (!get3nums(&offset,&size,&flags))
		goto err;

	len = GetVar(name,&(buffer[offset]),size,flags);
	if (len == -1)
		printf("(IoErr %ld): ",IoErr());
	printf("GetVar returned %ld (string = %s)\n",len,&(buffer[offset]));
}

void
setvar (int *done)
{
	char *name;
	LONG offset,size,flags,rc;

	name = strtok(NULL," ");
	if (!name)
	{
err:		printf("usage: SetVar <name> <offset> <size> <flags>\n");
		return;
	}

	if (!get3nums(&offset,&size,&flags))
		goto err;

	rc = SetVar(name,&(buffer[offset]),size,flags);
	if (!rc)
		printf("SetVar failed!  (IoErr = %ld)\n",IoErr());
}

void
findvar (int *done)
{
	char *name;
	LONG type;
	struct LocalVar *lv;

	name = strtok(NULL," ");
	if (!name)
	{
err:		printf("usage: FindVar <name> <type>\n");
		return;
	}

	if (!getnum(&type))
		goto err;

	lv = FindVar(name,type);
	printf("FindVar(%s,%ld) returned 0x%lx\n",name,type,lv);
}

void
namefromlock (int *done)
{
	unsigned char *buf;
	LONG size,num,offset,rc;

	if (!getnum(&num))
	{
err:	printf("usage: NameFromLock <lock> <offset> <length>\n");
	return;
	}

	if (!getnum(&offset))
		goto err;

	buf = &(buffer[offset]);

	if (!getnum(&size))
		goto err;

	rc = NameFromLock(locks[num],buf,size);
	printf("NameFromLock returned %ld (ioerr = %ld)\n",rc,IoErr());

	if (rc)
		printf("\t\"%s\"\n",buf);
}

void
namefromfh (int *done)
{
	unsigned char *buf;
	LONG size,num,offset,rc;

	if (!getnum(&num))
	{
err:	printf("usage: NameFromFH <file> <offset> <length>\n");
	return;
	}

	if (!getnum(&offset))
		goto err;

	buf = &(buffer[offset]);

	if (!getnum(&size))
		goto err;

	rc = NameFromFH(files[num],buf,size);
	printf("NameFromFH returned %ld (ioerr = %ld)\n",rc,IoErr());

	if (rc)
		printf("\t\"%s\"\n",buf);
}

void
parsepattern (int *done)
{
	char *pattern;
	char *dest;
	LONG offset,len,rc;

	pattern = strtok(NULL," ");
	if (!pattern)
	{
err:		printf("usage: ParsePattern <pattern> <offset> <len>\n");
		return;
	}

	/* strip quotes if any */
	if (*pattern == '"' && *(pattern+strlen(pattern)-1) == '"')
	{
		pattern++;
		*(pattern+strlen(pattern)-1) = '\0';
	}

	if (!get2nums(&offset,&len))
		goto err;

	dest = &(buffer[offset]);

	rc = ParsePattern(pattern,dest,len);

	printf("ParsePattern returned %ld\n",rc);
}

void
matchpattern (int *done)
{
	char *pattern;
	char *string;
	LONG offset,rc;

	if (!getnum(&offset))
		goto err;

	string = strtok(NULL," ");
	if (!string)
	{
err:		printf("usage: MatchPattern <offset> <string>\n");
		return;
	}

	pattern = &(buffer[offset]);

	rc = MatchPattern(pattern,string);

	printf("MatchPattern returned %ld\n",rc);
}

void
parsepatternnocase (int *done)
{
	char *pattern;
	char *dest;
	LONG offset,len,rc;

	pattern = strtok(NULL," ");
	if (!pattern)
	{
err:		printf("usage: ParsePatternNoCase <pattern> <offset> <len>\n");
		return;
	}

	/* strip quotes if any */
	if (*pattern == '"' && *(pattern+strlen(pattern)-1) == '"')
	{
		pattern++;
		*(pattern+strlen(pattern)-1) = '\0';
	}

	if (!get2nums(&offset,&len))
		goto err;

	dest = &(buffer[offset]);

	rc = ParsePatternNoCase(pattern,dest,len);

	printf("ParsePatternNoCase returned %ld\n",rc);
}

void
matchpatternnocase (int *done)
{
	char *pattern;
	char *string;
	LONG offset,rc;

	if (!getnum(&offset))
		goto err;

	string = strtok(NULL," ");
	if (!string)
	{
err:		printf("usage: MatchPatternNoCase <offset> <string>\n");
		return;
	}

	pattern = &(buffer[offset]);

	rc = MatchPatternNoCase(pattern,string);

	printf("MatchPatternNoCase returned %ld\n",rc);
}

void
isfilesystem (int *done)
{
	char *string;
	LONG rc;

	string = strtok(NULL," ");
	if (!string)
	{
		printf("usage: IsFileSystem <device name>:\n");
		return;
	}

	rc = IsFileSystem(string);

	printf("IsFileSystem returned %ld\n",rc);
}

void
loadseg (int *done)
{
	BPTR seg;
	char *string;

	string = strtok(NULL," ");
	if (!string)
	{
		printf("usage: LoadSeg <file>\n");
		return;
	}

	seg = LoadSeg(string);
	printf("LoadSeg returned 0x%lx, ioerr = %ld\n",seg,IoErr());
}

void
unloadseg (int *done)
{
	BPTR seg;
	char *string;

	string = strtok(NULL," ");
	if (!string)
	{
err:		printf("usage: UnLoadSeg 0x<addr>\n");
		return;
	}

	if (*string++ != '0' || *string++ != 'x')
		goto err;

	if (stch_l(string,&seg) != strlen(string))
		goto err;

	UnLoadSeg(seg);

	printf("unloaded 0x%lx\n",seg);
}

void
relabel (int *done)
{
	char *disk,*name;
	LONG rc;

	disk = strtok(NULL," ");
	if (!disk)
	{
err:		printf("usage: Relabel <disk>: <name>\n");
		return;
	}
	if (*(disk + strlen(disk) - 1) != ':')
		goto err;

	name = strtok(NULL," ");
	if (!name)
		goto err;

	rc = Relabel(disk,name);
	if (!rc)
		printf("Relabel failed!  (IoErr = %ld)\n",IoErr());
}

void
assignlock (int *done)
{
	char *disk;
	LONG rc,lock;

	disk = strtok(NULL," ");
	if (!disk)
	{
err:		printf("usage: AssignLock <name> <lock>\n");
		return;
	}
	if (*(disk + strlen(disk) - 1) == ':')
		goto err;

	if (!getnum(&lock))
		goto err;

	rc = AssignLock(disk,locks[lock]);
	if (!rc)
		printf("AssignLock failed!  (IoErr = %ld)\n",IoErr());
	else {
		locks[lock] = NULL;
	}
}

