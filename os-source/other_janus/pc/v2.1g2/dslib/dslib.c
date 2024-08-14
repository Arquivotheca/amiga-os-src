/*
 * dslib.c
 *
 * a collection of amigados-like functions for doing things through DOSServ
 * and things to set up for them.
 *
 * 12/12/90		rsd		created
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <exec/types.h>
#include <janus/services.h>
#include <janus/dosserv.h>
#include "dslib.h"

/* #define PRINTING */
/* #define WAIT */

static UBYTE myCallService(struct dslib_struct *ds)
{
UBYTE rval;

#ifdef PRINTING
	printf("myCallService (pre): dsr_Function = %d\n", ds->ds_rq->dsr_Function);
	printf("myCallService (pre): dsr_Buffer_Seg:Off = %p\n",
	 (((ULONG)ds->ds_rq->dsr_Buffer_Seg) << 16) | ds->ds_rq->dsr_Buffer_Off);
	printf("myCallService (pre): dsr_Buffer_Size = %d\n", ds->ds_rq->dsr_Buffer_Size);
	printf("myCallService (pre): dsr_Arg1 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l);
	printf("myCallService (pre): dsr_Arg2 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg2_h) << 16) + ds->ds_rq->dsr_Arg2_l);
	printf("myCallService (pre): dsr_Arg3 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg3_h) << 16) + ds->ds_rq->dsr_Arg3_l);
#ifdef WAIT
	puts("hit return: ");
	getch();
#endif
#endif

	rval = CallService(ds->sd_ds);
#ifdef PRINTING
	printf("myCallService: call complete, waiting for service\n");
#endif
	WaitService(ds->sd_ds);

#ifdef PRINTING
	printf("myCallService (post): retval = %d\n", rval);
	printf("myCallService (post): dsr_Err = %d\n", ds->ds_rq->dsr_Err);
	printf("myCallService (post): dsr_Arg1 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l);
	printf("myCallService (post): dsr_Arg2 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg2_h) << 16) + ds->ds_rq->dsr_Arg2_l);
	printf("myCallService (post): dsr_Arg3 = %ld\n",
	 (((long)ds->ds_rq->dsr_Arg3_h) << 16) + ds->ds_rq->dsr_Arg3_l);
#ifdef WAIT
	puts("hit return: ");
	getch();
#endif
#endif

	return rval;
}

static void setcd(struct dslib_struct *ds, ULONG newcd)
{
int err;

	newcd--;

	/* arg1 gets lock */
	ds->ds_rq->dsr_Arg1_l = newcd & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (newcd >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_SETCURRENTDIR;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
		} else {
			/* createdir failed */
		}
	} else {
		/* service failed */
	}
}

static void getcurrentdir(struct dslib_struct *ds)
{
int err;

	ds->ds_rq->dsr_Function = DSR_FUNC_GETCURRENTDIR;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			ds->oldcd = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			ds->oldcd++;
		} else {
			/* createdir failed */
		}
	} else {
		/* service failed */
	}
}

static void endcurrentdir(struct dslib_struct *ds)
{
int err;
ULONG dir;

	dir = ds->oldcd - 1;

	ds->ds_rq->dsr_Arg1_h = (dir >> 16) & 0xffff;
	ds->ds_rq->dsr_Arg1_l = dir & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_ENDCURRENTDIR;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
		} else {
			/* createdir failed */
		}
	} else {
		/* service failed */
	}
}

static void unlock(struct dslib_struct *ds, ULONG lock)
{
int err;

	/* arg1 gets lock */
	lock--;
	ds->ds_rq->dsr_Arg1_l = lock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (lock >> 16) & 0xffff;
	ds->ds_rq->dsr_Function = DSR_FUNC_UNLOCK;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* all ok */
		} else {
			/* createdir failed */
		}
	} else {
		/* service failed */
	}
}


/****** dslib.lib/j_tickle_janus *******************************************
*
*	NAME	
*		j_tickle_janus -- determine if janus handler is installed
*
*	SYNOPSIS
*		UBYTE success = j_tickle_janus()
*
*	FUNCTION
*		Determines if the janus handler is present in the system.
*
*	INPUTS
*
*	RESULT
*		UBYTE success
*			FALSE (0) value indicates failure to locate the
*				janus handler.
*			TRUE (nonzero) value indicates that DOSServ has been
*				opened and the structure filled in correctly.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
UBYTE j_tickle_janus()
{
UBYTE error;
USHORT j_param_seg, j_param_offset, j_buffer_seg;
int i;

	j_param_offset = 0xffff;
	
	/* first wake up janus */
	for (i = 0; i < 2; i++) {
		error = GetBase(JSERV_AMIGASERVICE, &j_param_seg,
			        &j_param_offset, &j_buffer_seg);
	}

	if (j_param_offset == 0xffff)
		error = TRUE;

	return error; 
}


/****** dslib.lib/j_open_dosserv *******************************************
*
*	NAME	
*		j_open_dosserv -- open DOSServ service
*
*	SYNOPSIS
*		UBYTE success = j_open_dosserv(struct dslib_struct *ds)
*
*	FUNCTION
*		Opens the DOSServ service provided by the Amiga, paving the
*		way for j_fxxx() functions.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure isi
*			filled in by j_open_dosserv() if DOSServ can be opened.
*
*	RESULT
*		UBYTE success
*			FALSE (zero)
*				Indicates that DOSServ has been	opened and the
*				structure filled in correctly.
*			TRUE (nonzero)
*				Indicates failure to open DOSServ.  The
*				structure should not be used, and no other
*				calls to other j_fxxx() functions should be
*				performed.
*
*		struct j_dosserv *ds
*			If there was no error, this structure is filled in
*			with pointers needed by other j_fxxxx() functions.
*
*	EXAMPLE
*
*	NOTES
*		Before calling, you should insure that the janus handler is
*		present by calling j_tickle_janus().
*
*		This function may take up to 30 seconds to complete (assuming
*		that a GetService() with GETS_WAIT and GETS_ALOAD times out
*		in 30 seconds if the service won't or can't start).
*
*		If this function fails, you should not call j_close_dosserv().
*
*	BUGS
*		Currently the synchronization of autoloaded services with
*		GetService() isn't working right, so this call can sometimes
*		appear to work when it really failed.
*
*	SEE ALSO
*		j_tickle_janus(), j_close_dosserv(), j_fopen(), f_fclose(),
*		j_fread(), j_fwrite(), j_fseek()
*
****************************************************************************/

UBYTE j_open_dosserv(struct dslib_struct *ds)
{
UBYTE error;
ULONG cd;
void *sig;

	/* now try to get DOSServ */
	error = GetService(&ds->sd_ds, (ULONG) DOSSERV_APPLICATION_ID,
			   (UWORD) DOSSERV_LOCAL_ID, (ULONG) 0,
			   (UWORD) GETS_WAIT | GETS_ALOAD_A);

/* kludge until GetService() finishes all init before returning */
JanusLock(&ds->sd_ds->sd_ServiceDataLock);
JanusUnlock(&ds->sd_ds->sd_ServiceDataLock);

	/* did it work? */
	if (error == JSERV_OK) {
		/* yes! */
		ds->ds_rq = (struct DOSServReq *) ds->sd_ds->sd_PCMemPtr;
		ds->ds_buf = (UBYTE *)
 ((((ULONG) ds->ds_rq->dsr_Buffer_Seg) << 16) + ds->ds_rq->dsr_Buffer_Off);
		ds->ds_buf_len = ds->ds_rq->dsr_Buffer_Size;
		error = FALSE;

		/* find out what the current dir is */
		getcurrentdir(ds);
		ds->currentdir = ds->oldcd;
	} else {
		/* no.  oh well. */
		ds->sd_ds = 0;
		ds->ds_rq = 0;
		ds->ds_buf = 0;
		ds->ds_buf_len = 0;
		error = TRUE;
	}
#ifdef PRINTING
printf("j_open_dosserv: ds = %p, sd_ds = %p, ds_rq = %p, ds_buf = %p, ds_buf_len = %d, retval = %d\n",
	ds, ds->sd_ds, ds->ds_rq, ds->ds_buf, ds->ds_buf_len, error);
#endif

	/* return error code from GetService() */
	ds->errno = error;
	return error;
}


/****** dslib.lib/j_close_dosserv ******************************************
*
*	NAME	
*		j_close_dosserv -- closes DOSServ service
*
*	SYNOPSIS
*		UBYTE Error = j_close_dosserv(struct dslib_struct *ds)
*
*	FUNCTION
*		Closes the DOSServ service (opposite of j_open_dosserv()).
*		If j_open_dosserv() succeeded, you should call this function
*		when you are finished with j_fxxx() calls.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*	RESULT
*		Error
*			FALSE (zero)
*				Indicates that DOSServ has been closed.
*			TRUE (nonzero)
*				Indicates failure to close DOSServ.  This
*				should never happen.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*		Ideally, all the functions in this library that open files
*		or create locks should register these things in a list
*		attached to the dslib_struct, so that j_close_dosserv()
*		can get rid of them in case of programmer error.
*
*	SEE ALSO
*		j_open_dosserv()
*
****************************************************************************/
UBYTE j_close_dosserv(struct dslib_struct *ds)
{
UBYTE retval;

	endcurrentdir(ds);
	retval = ReleaseService(ds->sd_ds);
	return retval;
}

/****** dslib.lib/j_CreateDir **********************************************
*
*	NAME	
*		j_CreateDir - create a directory on the amiga
*
*	SYNOPSIS
*		ULONG Lock = j_CreateDir(struct dslib_struct *ds, char *name)
*
*	FUNCTION
*		Creates a directory on the amiga using the name supplied.
*		Returns 0 if failed, or a lock value if succeeded.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *name
*			Name of directory to create.
*
*	RESULT
*		Lock
*			zero
*				j_CreateDir failed for some reason
*			nonzero
*				The lock for the new directory.  The lock must
*				be freed using j_UnLock() eventually.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_CreateDir(struct dslib_struct *ds, char *name)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(name) + 1 >= ds->ds_buf_len) {
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* install filename in buffer */
	strcpy(ds->ds_buf, name);

	ds->ds_rq->dsr_Function = DSR_FUNC_CREATE_DIR;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			retval++;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_CurrentDir *********************************************
*
*	NAME	
*		j_CurrentDir - change the current directory
*
*	SYNOPSIS
*		ULONG Lock = j_CurrentDir(struct dslib_struct *ds,
*					  ULONG newlock)
*
*	FUNCTION
*		Change the current directory to that attached to the given
*		lock.  returns the lock for the "old" current directory.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG newlock
*			lock obtained via j_Lock() or j_CreateDir.
*
*	RESULT
*		ULONG Lock
*			lock for previous current directory
*
*	EXAMPLE
*
*	NOTES
*		The lock you pass in *must* be a directory (not a file) lock.
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_CurrentDir(struct dslib_struct *ds, ULONG newlock)
{
ULONG retval, oldcd;
int err;

	if (newlock == 0)
		return ds->currentdir;

	/* set the cd */
	retval = ds->currentdir;
	ds->currentdir = newlock;
	setcd(ds, ds->oldcd);

	return retval;
}

/****** dslib.lib/j_DeleteFile *********************************************
*
*	NAME	
*		j_DeleteFile - delete a file
*
*	SYNOPSIS
*		ULONG success = j_DeleteFile(struct dslib_struct *ds,
*					     char *name)
*
*	FUNCTION
*		Delete an amiga file
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *name
*			The name of the file to delete.
*
*	RESULT
*		ULONG success
*			nonzero if file was deleted.
*			zero if file was not deleted.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_DeleteFile(struct dslib_struct *ds, char *name)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(name) + 1 >= ds->ds_buf_len) {
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);

	/* install filename in buffer */
	strcpy(ds->ds_buf, name);

	ds->ds_rq->dsr_Function = DSR_FUNC_DELETEFILE;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_DupLock ************************************************
*
*	NAME	
*		j_DupLock - duplicate a lock
*
*	SYNOPSIS
*		ULONG newlock = j_DupLock(struct dslib_struct *ds,
*					  ULONG oldlock)
*
*	FUNCTION
*		Duplicate a lock on a file or directory.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG oldlock
*			The lock to be duplicated
*
*	RESULT
*		ULONG newlock
*			A lock on the same object as oldlock
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_DupLock(struct dslib_struct *ds, ULONG oldlock)
{
ULONG retval;
int err;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets lock */
	oldlock--;
	ds->ds_rq->dsr_Arg1_l = oldlock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (oldlock >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_DUPLOCK;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			retval++;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

static void fixlong(char *p)
{
char t;

	t = p[0];
	p[0] = p[3];
	p[3] = t;

	t = p[1];
	p[1] = p[2];
	p[2] = t;
}

static void fixdatestamp(struct DateStamp *ds)
{
	fixlong((char *)&ds->ds_Days);
	fixlong((char *)&ds->ds_Minute);
	fixlong((char *)&ds->ds_Tick);
}

static void fixfib(struct FileInfoBlock *fib)
{
	fixlong((char *)&fib->fib_DiskKey);
	fixlong((char *)&fib->fib_DirEntryType);
	fixlong((char *)&fib->fib_Protection);
	fixlong((char *)&fib->fib_EntryType);
	fixlong((char *)&fib->fib_Size);
	fixlong((char *)&fib->fib_NumBlocks);
	fixdatestamp(&fib->fib_Date);
}

/****** dslib.lib/j_Examine ************************************************
*
*	NAME	
*		j_Examine - get information about a lock
*
*	SYNOPSIS
*		ULONG success = j_Examine(struct dslib_struct *ds, ULONG lock,
*					  struct FileInfoBlock *infoblock)
*
*	FUNCTION
*		Get information about a lock.  Call this before calling
*		j_ExNext() when scanning a directory.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG lock
*			Lock on an object to be examined.
*
*		struct FileInfoBlock *infoblock
*			Pointer to structure to be filled in by j_Examine().
*
*	RESULT
*		ULONG success
*			nonzero if succeeded.
*			zero if failed.
*
*		struct FileInfoBlock *infoblock
*			if success is nonzero, structure has been filled in.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_Examine(struct dslib_struct *ds, ULONG lock, struct FileInfoBlock *infoblock)
{
ULONG retval;
int err;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets lock */
	lock--;
	ds->ds_rq->dsr_Arg1_l = lock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (lock >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_EXAMINE;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			memcpy(infoblock, ds->ds_buf, sizeof(struct FileInfoBlock));
			fixfib(infoblock);
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_ExNext *************************************************
*
*	NAME	
*		j_ExNext - get information about a lock
*
*	SYNOPSIS
*		ULONG success = j_ExNext(struct dslib_struct *ds, ULONG lock,
*					 struct FileInfoBlock *infoblock)
*
*	FUNCTION
*		Get information about a lock.  Call this after j_Examine()
*		repeatedly to scan a directory.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG lock
*			Lock on an object to be examined.
*
*		struct FileInfoBlock *infoblock
*			Pointer to structure to be filled in by j_ExNext().
*			Must have already been filled in by j_Examine().
*
*	RESULT
*		ULONG success
*			nonzero if succeeded.
*			zero if failed.
*
*		struct FileInfoBlock *infoblock
*			if success is nonzero, structure has been filled in.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_ExNext(struct dslib_struct *ds, ULONG lock, struct FileInfoBlock *infoblock)
{
ULONG retval;
int err;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets lock */
	lock--;
	ds->ds_rq->dsr_Arg1_l = lock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (lock >> 16) & 0xffff;

	memcpy(ds->ds_buf, infoblock, sizeof(struct FileInfoBlock));
	fixfib((struct FileInfoBlock *)ds->ds_buf);

	ds->ds_rq->dsr_Function = DSR_FUNC_EXNEXT;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			memcpy(infoblock, ds->ds_buf, sizeof(struct FileInfoBlock));
			fixfib(infoblock);
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_Lock ***************************************************
*
*	NAME	
*		j_Lock - obtain a lock on a file or directory
*
*	SYNOPSIS
*		ULONG lock = j_Lock(struct dslib_struct *ds, char *name,
*				    LONG accessmode)
*
*	FUNCTION
*		Obtain a lock on an object.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *name
*			Name of object to lock.
*
*		LONG accessmode
*			Type of lock.  Can be one of:
*				ACCESS_READ
*					shared read lock
*				ACCESS_WRITE
*					exclusive read/write lock
*
*	RESULT
*		ULONG lock
*			nonzero - Lock on object (lock succeeded).
*			zero - lock failed.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_Lock(struct dslib_struct *ds, char *name, LONG accessmode)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(name) + 1 >= ds->ds_buf_len) {
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* install filename in buffer */
	strcpy(ds->ds_buf, name);

	ds->ds_rq->dsr_Arg1_l = accessmode & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (accessmode >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_LOCK;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			retval++;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);
	
	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_ParsePattern *******************************************
*
*	NAME	
*		j_ParsePattern - convert an amigados regular expression to
*				 a parsed regular expression for use by
*				 j_MatchPattern().
*
*	SYNOPSIS
*		LONG wild = j_ParsePattern(struct dslib_struct *ds,
*					   char *sourcepat, char *destpat,
*					   LONG destlen)
*
*	FUNCTION
*		Create a parsed regular expression for j_MatchPattern().
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *sourcepat
*			Amigados regular expression string.
*
*		char *destpat
*			Place to store result.
*
*	RESULT
*		LONG wild
*			 1 = pattern contains wildcards (success)
*			 0 = pattern does not contain wildcards (success)
*			-1 = conversion error (syntax or destpat to small)
*
*		char *destpat
*			If wild indicates success, contains parsed regexp.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*		j_MatchPattern()
*
****************************************************************************/
LONG j_ParsePattern(struct dslib_struct *ds, char *sourcepat,
				    char *destpat, LONG destlen)
{
ULONG retval;
int err;
ULONG t;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets length of input pattern (including \0 on end) */
	t = strlen(sourcepat) + 1;
	memcpy(&(ds->ds_buf[0]), sourcepat, t);
	ds->ds_rq->dsr_Arg1_l = t & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (t >> 16) & 0xffff;

	/* arg2 gets length of output pattern */
	ds->ds_rq->dsr_Arg2_l = destlen & 0xffff;
	ds->ds_rq->dsr_Arg2_h = (destlen >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_PARSEPATTERN;
	
	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the wild flag */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			/* and get result pattern */
			if (retval != -1)
				strcpy(destpat, &(ds->ds_buf[t]));
			else
				strcpy(destpat, "");
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_MatchPattern *******************************************
*
*	NAME	
*		j_MatchPattern - compare a string with a preparsed regular
*				 expression.
*
*	SYNOPSIS
*		ULONG match = j_MatchPattern(struct dslib_struct *ds,
*					     char *pat, char *str)
*
*	FUNCTION
*		Determine if a string (typically a filename) matches a
*		preparsed amigados regular expression.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *pat
*			Preparsed amigados regular expression.
*
*		char *name
*			Name to check against regular expression.
*
*	RESULT
*		ULONG match
*			nonzero - name matches pattern
*			zero - name does not match pattern
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*		j_ParsePattern()
*
****************************************************************************/
ULONG j_MatchPattern(struct dslib_struct *ds, char *pat, char *str)
{
ULONG retval;
int err;
ULONG t;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets length of pattern (including \0 on end) */
	strcpy(&(ds->ds_buf[0]), pat);
	t = strlen(pat) + 1;
	ds->ds_rq->dsr_Arg1_l = t & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (t >> 16) & 0xffff;

	/* arg2 gets length of string (including \0 on end) */
	strcpy(&(ds->ds_buf[t]), str);

	ds->ds_rq->dsr_Function = DSR_FUNC_MATCHPATTERN;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_ParentDir **********************************************
*
*	NAME	
*		j_ParentDir - get the parent of a directory
*
*	SYNOPSIS
*		ULONG plock = j_ParentDir(struct dslib_struct *ds,
*					  ULONG lock)
*
*	FUNCTION
*		Get a lock on the directory above the given one.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG lock
*			lock obtained via j_Lock() or j_CreateDir.
*
*	RESULT
*		ULONG plock
*			lock for parent of the lock attached to lock.
*
*	EXAMPLE
*
*	NOTES
*		The lock you pass in *must* be a directory (not a file) lock.
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_ParentDir(struct dslib_struct *ds, ULONG oldlock)
{
ULONG retval;
int err;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets lock */
	oldlock--;
	ds->ds_rq->dsr_Arg1_l = oldlock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (oldlock >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_PARENTDIR;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			retval++;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_Rename *************************************************
*
*	NAME	
*		j_Rename - rename a file or directory
*
*	SYNOPSIS
*		ULONG success = j_Rename(struct dslib_struct *ds,
*					 char *oldname, char *newname)
*
*	FUNCTION
*		Rename an amiga file or directory.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *oldname
*			The name of the file to rename.
*
*		char *newname
*			The new name of the file.
*
*	RESULT
*		ULONG success
*			nonzero if file was renamed.
*			zero if file was not renamed.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_Rename(struct dslib_struct *ds, char *oldname, char *newname)
{
ULONG retval;
int err;
ULONG t;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* arg1 gets length of oldname (including \0 on end) */
	strcpy(&(ds->ds_buf[0]), oldname);
	t = strlen(oldname) + 1;
	ds->ds_rq->dsr_Arg1_l = t & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (t >> 16) & 0xffff;

	/* arg2 gets length of newname (including \0 on end) */
	strcpy(&(ds->ds_buf[t]), newname);
	ds->ds_rq->dsr_Arg2_l = t & 0xffff;
	ds->ds_rq->dsr_Arg2_h = (t >> 16) & 0xffff;
	t += strlen(newname) + 1;

	ds->ds_rq->dsr_Function = DSR_FUNC_RENAME;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);
	
	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_UnLock *************************************************
*
*	NAME	
*		j_UnLock - free a lock obtained via j_Lock(), j_CreateDir,
*			   j_ParentDir, or j_DupLock().
*
*	SYNOPSIS
*		j_UnLock(struct dslib_struct *ds, ULONG lock)
*
*	FUNCTION
*		Get rid of a lock.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG lock
*			The lock to be freed.
*
*	RESULT
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
void j_UnLock(struct dslib_struct *ds, ULONG lock)
{
int err;

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets lock */
	lock--;
	ds->ds_rq->dsr_Arg1_l = lock & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (lock >> 16) & 0xffff;
	ds->ds_rq->dsr_Function = DSR_FUNC_UNLOCK;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* all ok */
		} else {
			/* createdir failed */
		}
	} else {
		/* service failed */
	}

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);
}

/****** dslib.lib/j_Close **************************************************
*
*	NAME	
*		j_Close - close a file.
*
*	SYNOPSIS
*		j_Close(struct dslib_struct *ds, ULONG afile)
*
*	FUNCTION
*		Close a file.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG afile
*			File handle of file to be closed, obtained via
*			j_Open().
*
*	RESULT
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
void j_Close(struct dslib_struct *ds, ULONG afile)
{
	/* normalize */
	afile--;

  /* grab DOSServReq */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* do a close */
	ds->ds_rq->dsr_Function = DSR_FUNC_CLOSE;

	/* arg1 gets filehandle */
	ds->ds_rq->dsr_Arg1_l = afile & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

	/* who *CARES* if a close fails? */
	ds->errno = myCallService(ds);

	/* someone else can have it now */
	JanusUnlock(&ds->ds_rq->dsr_Lock);
}

/****** dslib.lib/j_Read ***************************************************
*
*	NAME	
*		j_Read - read data from a file.
*
*	SYNOPSIS
*		LONG bytesread = j_Read(struct dslib_struct *ds,
*					ULONG afile, void *bufin, LONG n_bytes)
*
*	FUNCTION
*		Read from a file.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG afile
*			File handle of file to read from.
*
*		void *bufin
*			Pointer to where to store data.
*
*		LONG n_bytes
*			Number of bytes to read.
*
*	RESULT
*		LONG bytesread
*			-1 = error occurred, no data read
*			 0 = EOF (no data read)
*			 otherwise number of bytes read.  Note that ds->errno
*				should be checked in this case, since multiple
*				reads can be performed for each call, and if
*				some succeed but a later read fails, you can
*				get a positive value but still have had an
*				error.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
LONG j_Read(struct dslib_struct *ds, ULONG afile, void *bufin, LONG n_bytes)
{
LONG retval, nread;
int length;
UBYTE *buf;

	/* normalize */
	afile--;

	/* grab DOSServReq */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets filehandle */
	ds->ds_rq->dsr_Arg1_l = afile & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

	/* set function */
	ds->ds_rq->dsr_Function = DSR_FUNC_READ;

	buf = bufin;
	retval = 0;
	ds->errno = FALSE;

	/* loop until all bytes transferred or error occurs */
	while (n_bytes) {
		if (n_bytes > ds->ds_buf_len)
			length = ds->ds_buf_len;
		else
			length = n_bytes;

		/* arg2 gets num of bytes to read */
		ds->ds_rq->dsr_Arg2_l = length & 0xffff;
		ds->ds_rq->dsr_Arg2_h = (length >> 16) & 0xffff;

		/* try to call the service */
		if (myCallService(ds) == JSERV_OK) {
			/* service worked.  did the read work? */
			if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
				/* read worked */
				nread = (((ULONG) ds->ds_rq->dsr_Arg3_h) << 16) + ds->ds_rq->dsr_Arg3_l;

				/* if got -1, error */
				if (nread == -1) {
					ds->errno = TRUE;
					break;
				}

				/* if we got 0 bytes, we're at EOF */
				if (nread == 0)
					break;

				memcpy(&buf[retval], ds->ds_buf, nread);
				retval += nread;
				n_bytes -= nread;
			} else {
				/* read failed */
				ds->errno = TRUE;
				break;
			}
		} else {
			/* service failed */
			ds->errno = TRUE;
			break;
		}
	}

	/* someone else can have it now */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_Write **************************************************
*
*	NAME	
*		j_Write - write data to a file.
*
*	SYNOPSIS
*		LONG byteswritten = j_Write(struct dslib_struct *ds,
*					    ULONG afile, void *bufin,
*					    LONG n_bytes)
*
*	FUNCTION
*		Write to a file.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG afile
*			File handle of file to write to.
*
*		void *bufin
*			Pointer to where to fetch data.
*
*		LONG n_bytes
*			Number of bytes to write.
*
*	RESULT
*		LONG byteswritten
*			-1 = error occurred, no data written
*			 otherwise number of bytes written.  Note that
*			 	ds->errno should be checked in this case, since
*			 	multiple writes can be performed for each call,
*				and if some succeed but a later write fails,
*				you can get a positive value but still have
*				had an error.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
LONG j_Write(struct dslib_struct *ds, ULONG afile, void *bufin, LONG n_bytes)
{
LONG retval, nwritten;
int length;
UBYTE *buf;

	/* normalize */
	afile--;

	/* grab DOSServReq */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets filehandle */
	ds->ds_rq->dsr_Arg1_l = afile & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

	/* set function */
	ds->ds_rq->dsr_Function = DSR_FUNC_WRITE;

	buf = bufin;
	retval = 0;
	ds->errno = FALSE;

	/* loop until all bytes transferred or error occurs */
	while (n_bytes) {
		if (n_bytes > ds->ds_buf_len)
			length = ds->ds_buf_len;
		else
			length = n_bytes;

		/* arg2 gets num of bytes to write */
		ds->ds_rq->dsr_Arg2_l = length & 0xffff;
		ds->ds_rq->dsr_Arg2_h = (length >> 16) & 0xffff;

		/* put bytes in buffer */
		memcpy(ds->ds_buf, &buf[retval], length);

		/* try to call the service */
		if (myCallService(ds) == JSERV_OK) {
			/* service worked.  did the write work? */
			if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
				/* write worked */
				nwritten = (((ULONG) ds->ds_rq->dsr_Arg3_h) << 16) + ds->ds_rq->dsr_Arg3_l;

				/* if got an error, stop */
				if (nwritten == -1) {
					ds->errno = TRUE;
					break;
				}

				/* we did this many */
				retval += nwritten;
				n_bytes -= nwritten;

				/* if everything didn't make it, stop. */
				if (nwritten != length) {
					break;
				}
	
			} else {
				/* read failed */
				ds->errno = TRUE;
				break;
			}
		} else {
			/* service failed */
			ds->errno = TRUE;
			break;
		}
	}

	/* someone else can have it now */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_Seek ***************************************************
*
*	NAME	
*		j_Seek - change file position pointer.
*
*	SYNOPSIS
*		LONG oldpos = j_Seek(struct dslib_struct *ds, ULONG afile,
*				     LONG offset, LONG mode)
*
*	FUNCTION
*		Seek within a file
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		ULONG afile
*			File handle of file to seek.
*
*		LONG offset
*			Offset in bytes within file.  Actual position depends
*			on mode.
*
*		LONG mode
*			How to interpret offset.  One of the following:
*				OFFSET_BEGINNING
*					offset = number of bytes from start
*					of file.
*				OFFSET_CURRENT
*					offset = number of bytes from current
*					position (may be negative).  a zero
*					value doesn't move the file pointer,
*					but has the effect of returning the
*					current file position.
*				OFFSET_END
*					offset = number of bytes from end
*					of file.  this value should be negative
*					to seek backwards from the end, or
*					zero to seek to the end.
*
*	RESULT
*		LONG oldpos
*			the file pointer value before this call completes.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
LONG j_Seek(struct dslib_struct *ds, ULONG afile, LONG offset, LONG mode)
{
LONG retval;

	/* normalize */
	afile--;

	/* grab DOSServReq */
	JanusLock(&ds->ds_rq->dsr_Lock);

	/* arg1 gets filehandle */
	ds->ds_rq->dsr_Arg1_l = afile & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

	/* arg2 gets seek offset */
	ds->ds_rq->dsr_Arg2_l = offset & 0xffff;
	ds->ds_rq->dsr_Arg2_h = (offset >> 16) & 0xffff;

	/* set function */
	if (mode == OFFSET_BEGINNING) {
		ds->ds_rq->dsr_Function = DSR_FUNC_SEEK_BEGINING;
	} else if (mode == OFFSET_CURRENT) {
		ds->ds_rq->dsr_Function = DSR_FUNC_SEEK_CURRENT;
	} else if (mode == OFFSET_END) {
		ds->ds_rq->dsr_Function = DSR_FUNC_SEEK_END;
	} else {
		ds->errno = TRUE;
		return 0;
	}
	
	/* try to call the service */
	if (myCallService(ds) == JSERV_OK) {
		/* service worked.  did the seek work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* seek worked */
			retval = (((ULONG) ds->ds_rq->dsr_Arg3_h) << 16) + ds->ds_rq->dsr_Arg3_l;
			ds->errno = FALSE;
		} else {
			/* seek failed */
		retval = 0;
			ds->errno = TRUE;
		}
	} else {
		/* service failed */
		retval = 0;
		ds->errno = TRUE;
	}

	/* someone else can have it now */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}

/****** dslib.lib/j_Open ***************************************************
*
*	NAME	
*		j_Open - open a file.
*
*	SYNOPSIS
*		ULONG handle = j_Open(struct dslib_struct *ds,
*				      char *fname, LONG mode)
*
*	FUNCTION
*		Open an amiga file.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *fname
*			Name of file to open.
*
*		LONG mode
*			How to open the file.  One of the following:
*				MODE_OLDFILE
*					The file must exist, and will be
*					opened in shared read mode.
*				MODE_NEWFILE
*					If the file exists, it is cleared.
*					If it doesn't exist, it is created.
*					The file is opened in exclusive write
*					mode.
*				MODE_READWRITE
*					If the file exists, it is opened.
*					If it doesn't exist, it is created.
*					The file is opened in shared read mode.
*
*	RESULT
*		LONG handle
*			The file handle needed to read/write/seek/close this
*			file.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_Open(struct dslib_struct *ds, char *fname, LONG mode)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(fname) + 1 >= ds->ds_buf_len) {
		ds->errno = TRUE;
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* install filename in buffer */
	strcpy(ds->ds_buf, fname);

	/* figure out flavor of open */
	if (mode == MODE_OLDFILE) {
		/* exclusive lock, file must exist */
		ds->ds_rq->dsr_Function = DSR_FUNC_OPEN_OLD;
	} else if (mode == MODE_NEWFILE) {
		/* exclusive lock, file is created or truncated to 0 length */
		ds->ds_rq->dsr_Function = DSR_FUNC_OPEN_NEW;
	} else if (mode == MODE_READWRITE) {
		/* shared lock, create if doesn't exist */
		ds->ds_rq->dsr_Function = DSR_FUNC_OPEN_READ_WRITE;
	} else {
		/* unsupported mode */
		JanusUnlock(&ds->ds_rq->dsr_Lock);
		ds->errno = TRUE;
		return 0;
	}

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the open work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the file handle + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
			retval++;
		} else {
			/* open failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);
	
	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	if (retval)
		ds->errno = FALSE;
	else
		ds->errno = TRUE;

	return retval;
}

#if 0
/*xxxxxxxxx***** dslib.lib/j_IsFileSystem *******************************************
*
*	NAME	
*		j_IsFileSystem - determine if the given name indicates a file
*				 on a filesystem.
*
*	SYNOPSIS
*		ULONG truth = j_IsFileSystem(struct dslib_struct *ds,
*					     char *name)
*
*	FUNCTION
*		Determine if a name references a legal filesystem device or
*		volume.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *name
*			Name to test.
*
*	RESULT
*		ULONG truth
*			nonzero - the file does refer to a filesystem device.
*			zero - the file does NOT refer to a filesystem device.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_IsFileSystem(struct dslib_struct *ds, char *name)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(name) + 1 >= ds->ds_buf_len) {
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);

	/* install filename in buffer */
	strcpy(ds->ds_buf, name);

	ds->ds_rq->dsr_Function = DSR_FUNC_ISFILESYSTEM;

	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the createdir work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the lock index + 1 */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* createdir failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);

	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	return retval;
}
#endif

/****** dslib.lib/j_SetProtection ******************************************
*
*	NAME	
*		j_SetProtection - change a file's protection bits
*
*	SYNOPSIS
*		ULONG success = j_SetProtection(struct dslib_struct *ds,
*						char *fname, LONG bits)
*
*	FUNCTION
*		Alter a file's protection bits.
*
*	INPUTS
*		struct j_dosserv *ds
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *fname
*			Name of file whose bits to change.
*
*		LONG bits
*			New bits to install in the file's fib.fib_Protection
*			field.  Possible values are listed in dslib.h.
*
*	RESULT
*		ULONG success
*			TRUE if the bits were modified.
*			FALSE if the bits were not modified.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_SetProtection(struct dslib_struct *ds, char *fname, LONG bits)
{
ULONG retval;
int err;

	/* go poof if filename is huge */
	if (strlen(fname) + 1 >= ds->ds_buf_len) {
		ds->errno = TRUE;
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* install filename in buffer */
	strcpy(ds->ds_buf, fname);

	/* bits go in arg1 */
	ds->ds_rq->dsr_Arg1_l = bits & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (bits >> 16) & 0xffff;

	ds->ds_rq->dsr_Function = DSR_FUNC_SETPROTECTION;
	
	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the open work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the success flag */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* open failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);
	
	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	if (retval)
		ds->errno = FALSE;
	else
		ds->errno = TRUE;

	return retval;
}


/****** dslib.lib/j_SetFileDate ********************************************
*
*	NAME	
*		j_SetFileDate - change the time/date stamp of a file
*
*	SYNOPSIS
*		ULONG success = j_SetFileDate(struct dslib_struct *ds,
*						char *fname, struct DateStamp *d_s)
*
*	FUNCTION
*		Alter a file's timestamp
*
*	INPUTS
*		struct j_dosserv *d_s
*			Pointer to a j_dosserv struct.  This structure was
*			filled in by j_open_dosserv().
*
*		char *fname
*			Name of file whose bits to change.
*
*		stuct DateStamp *ds
*			Pointer to a DateStamp struct containing the new timestamp
*			value.
*
*	RESULT
*		ULONG success
*			TRUE if the timestamp was modified.
*			FALSE if the timestamp was not modified.
*
*	EXAMPLE
*
*	NOTES
*
*	BUGS
*
*	SEE ALSO
*
****************************************************************************/
ULONG j_SetFileDate(struct dslib_struct *ds, char *fname, struct DateStamp *d_s)
{
ULONG retval;
int err;
int len;

	/* go poof if filename is huge */
	if (strlen(fname) + 1 >= ds->ds_buf_len) {
		ds->errno = TRUE;
		return 0;
	}

	/* lock the buffer */
	JanusLock(&ds->ds_rq->dsr_Lock);

	setcd(ds, ds->currentdir);
	
	/* install filename in buffer */
	len = strlen(fname) + 1;
	memcpy(&ds->ds_buf[0], fname, len);

	/* strlen goes in arg1 */
	ds->ds_rq->dsr_Arg1_l = len & 0xffff;
	ds->ds_rq->dsr_Arg1_h = (len >> 16) & 0xffff;

	/* datestamp struct goes after name */
	memcpy(&ds->ds_buf[len], d_s, sizeof(struct DateStamp));
	fixdatestamp((struct DateStamp *)&ds->ds_buf[len]);
		
	ds->ds_rq->dsr_Function = DSR_FUNC_SETFILEDATE;
	
	/* try to call the service */
	if ((err = myCallService(ds)) == JSERV_OK) {
		/* service call worked.  did the open work? */
		if (ds->ds_rq->dsr_Err == DSR_ERR_OK) {
			/* yes - return the success flag */
			retval = (((ULONG) ds->ds_rq->dsr_Arg1_h) << 16) + ds->ds_rq->dsr_Arg1_l;
		} else {
			/* open failed */
			retval = 0;
		}
	} else {
		/* service failed */
		retval = 0;
	}

	setcd(ds, ds->oldcd);
	
	/* unlock the buffer */
	JanusUnlock(&ds->ds_rq->dsr_Lock);

	if (retval)
		ds->errno = FALSE;
	else
		ds->errno = TRUE;

	return retval;
}
