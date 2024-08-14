/* Translations for C I/O routines to AMIGA I/O routines */

#include "exec/types.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "amigaio.h"
#include "serial.h"

/* #define DEBUG 1 /* */

extern strlen();

static int seconds;

Exists(name)
char *name;
{
int *lock;
	UnLock((lock = (int *)Lock(name,ACCESS_READ)));
	if (lock!=0) return(TRUE);
	else {
		UnLock((lock = (int *)Lock(name,ACCESS_WRITE)));
		if (lock!=0) return(TRUE);
	}
	return(FALSE);
}

/*
Try to create a new file or open an existing file.
Return a file_handle if the create succeeds
and -1 if not
*/
creat(name,mode) /* create file for read or write but not r/w */
char *name;
int mode;
{
int fh;
/*
	if (Exists(name)) mode = MODE_OLDFILE;
	else mode = MODE_NEWFILE;
(omitted due to append bug Mar 14/86) */

	fh = Open(name,mode);
	return((fh ? fh : -1));
}

open(name,mode) /* open a new file for read(0) or write(1) but not r/w */
char *name;
int mode;
{
int fh;
	mode = mode ? MODE_NEWFILE : MODE_OLDFILE; /* select write(1) or read(0) */
	fh = Open(name,mode); /* try to open it */
	return((fh ? fh : -1)); /* return ptr or -1 */
}
	
close(fh) /* close */
int fh;
{
	Close(fh);
}

write(device,data,length)
int device, length;
char *data;
{
	if (device==1)	{ /* if stdout use serial */
		WriteSerial(data,length);
		return(length);
	}
	else /* disk */
		return(Write(device,data,length));
}

alarm(time)
int time;
{
	seconds = time;
}

read(device,data,length)
int device, length;
char *data;
{
int jiffies = (seconds * 50), inChar, len = 0;
	if (device==0) { /* if stdin use serial */
		while (jiffies>0 && len<length) { /* while not timed out or not all read */
			if ((inChar=SDMayGetChar())!=-1) { /* if there is a char */
				*data++ = inChar; /* save it */
				len++; /* got one more char */
				queue_readSerial(); /* queue up another read */
			}
			else { /* no char yet */
				Delay(1); /* wait a 50th */
				jiffies--; /* bump counter */
			}
		}
		return(len ? len : -1); /* return # of chars read or error */
	}
	else /* disk */
		return(Read(device,data,length));
}

sleep(seconds)
int seconds;
{
	Delay(seconds * 50);
}

stat(name,buffer) /* put info on file 'name' in buffer 'buffer' */
char *name;
struct stat *buffer;
{
struct FileInfoBlock *dbuf;
int *lock, error = FALSE;
	dbuf = (struct FileInfoBlock *)AllocMem(sizeof(*dbuf),MEMF_PUBLIC);
	lock = (int *)Lock(name,ACCESS_READ);
	if (lock!=0) { /* if no error */
		Examine(lock,dbuf);
		UnLock(lock);
		buffer->st_size = dbuf->fib_Size;
	}
	FreeMem(dbuf,sizeof(*dbuf));
}

fopen(name,mode)
char *name, *mode;
{
int fh = 0;
	if (*mode=='a') { /* append */
		if (Exists(name)) { /* if it exits */
			fh = Open(name,MODE_OLDFILE); /* open it */
			Seek(fh, OFFSET_END, 0); /* posn to end */
		}
		else fh = Open(name,MODE_NEWFILE); /* open a new file */
	}
	else if (*mode=='w') /* write */
		fh = Open(name,MODE_NEWFILE); /* open a new file */
	else if (*mode=='r') /* read */
		fh = Open(name,MODE_OLDFILE); /* open it */
	return(fh);
}

fprintf(fh,s) /* file printf */
int fh; /* the file-handle (ptr) */
char *s; /* ptr to the string to write */
{
	if (fh) Write(fh,s,strlen(s));
}

fclose(fh) /* file close */
int fh;
{
	Close(fh);
}

