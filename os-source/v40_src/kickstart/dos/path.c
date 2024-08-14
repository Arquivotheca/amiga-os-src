/* path.c */

#include <exec/types.h>
#include <exec/memory.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include <string.h>
#include <stddef.h>	/* for offsetof */

#include "libhdr.h"
#include "cli_init.h"

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "path_protos.h"
#include "klib_protos.h"
#include "blib_protos.h"
#endif

static LONG __regargs common_get (LONG offset, UBYTE *buffer, LONG len);
static LONG __regargs uncommon_get (UBYTE *name, UBYTE *buffer, LONG len);
static LONG __regargs common_set (LONG offset, UBYTE *newname, LONG maxsize);


/* cribbed from SeaShell - REJ */

LONG ASM
NameFromLock (REG(d1) BPTR lock, REG(d2) char *buffer, REG(d3) LONG buflen)
{
	BPTR fl_lock;
	BPTR fl_lock2;
	char *name;
	char *vl;
	struct FileInfoBlock *fib;
	LONG rc = FALSE;
	LONG len;

	if (buflen == 0)
	{
		setresult2(ERROR_LINE_TOO_LONG);
		return FALSE;
	}

	*buffer = '\0';

	if (!lock)
	{
		/* null lock - return sys: */
		/* FIX! does this work right for bufferlen < 4? */
		/* FIX! really boot volume */
		/* FIX! get from rn_Boot... */
		return AddPart(buffer,"SYS:",buflen);
	}

	/* save original lock - don't free! */
	/* we don't want to Examine() the lock - it causes a reset of */
	/* FS information for ExNext under FFS */
	fl_lock = copydir(lock);
	if (!fl_lock)
		return FALSE;

	fib = AllocDosObject(DOS_FIB,NULL);
	if (!fib)
	{
		freeobj(fl_lock);
		return FALSE;
	}

	/* null name.  We start at end of buffer and work backwards */
	name  = &buffer[buflen-1];
	*name = '\0';

	/* is fl_lock the root? */
	while (fl_lock2 = parent(fl_lock)) 	/* while fl_lock != root */
	{
		if (examine(fl_lock,fib))
		{
			/* make sure we have enough space */
			len = strlen(fib->fib_FileName);
			/* 1 for null, 1 for /, 1 for :, and 1 for dev = 4 */
			if (name-(len+4) < buffer)
			{
				setresult2(ERROR_LINE_TOO_LONG);
				freeobj(fl_lock2);
				goto cleanup;
			}

			/* add seperator if needed */
			if (*name)
				*--name = '/';
			copyMem(fib->fib_FileName,name - len,len);
			name -= len;
		} else {
			/* error! exit with failure! */
			goto cleanup;
		}
		freeobj(fl_lock);

		fl_lock = fl_lock2;
	}

	if (getresult2())	/*  == ERROR_DEVICE_NOT_MOUNTED) */
	{
		/* error! disk isn't in (ioerr should be 0 for parent(root)!) */
		/* or some other error */
		goto cleanup;
	}

	*--name = ':';

	/* device names must be null-terminated */
	vl = (char *) ((long) (((struct DeviceList *) 
                      ((long) (((struct FileLock *)
	              ((long) fl_lock << 2))->fl_Volume) << 2))->dl_Name) << 2)
		      + 1;

	/* do we have space? */
	len = strlen(vl);
	if (name-len < buffer)
	{
		setresult2(ERROR_LINE_TOO_LONG);
		goto cleanup;
	}
	copyMem(vl,buffer,len);
	/* Danger! counts on strcpy to work forward! */
	strcpy(buffer+len,name);

	rc = DOSTRUE;

cleanup:
	freeobj(fl_lock);
	freeVec(fib);

	return rc;
}

static LONG __regargs
common_get (LONG offset, UBYTE *buffer, LONG len)
{
	struct CommandLineInterface *clip;

	clip = cli();
	if (!clip)
	{
		if (len)
			*buffer = '\0';
		setresult2(ERROR_OBJECT_WRONG_TYPE);
		return FALSE;
	}

	return uncommon_get((UBYTE *)BADDR(*((LONG *) (((LONG) clip)+offset))),
			  buffer,len);
}

LONG ASM
NameFromFH (REG(d1) BPTR fh, REG(d2) char *buffer, REG(d3) LONG buflen)
{
	BPTR lock;
	struct FileInfoBlock *fib = NULL;
	LONG rc = FALSE;
	LONG res2;

	/* kind of ugly coding style here */

	if ((!(lock = ParentOfFH(fh))) ||
	    (!(fib  = AllocDosObject(DOS_FIB,NULL))) ||
	    (!(ExamineFH(fh,fib))) ||
	    (!(NameFromLock(lock,buffer,buflen))))
		goto cleanup;

	rc = AddPart(buffer,fib->fib_FileName,buflen);

cleanup:
	res2 = getresult2();	/* don't lose error code */

	if (fib)
		FreeDosObject(DOS_FIB,fib);
	if (lock)
		freeobj(lock);

	setresult2(res2);

	return rc;
}

static LONG __regargs
uncommon_get (UBYTE *name, UBYTE *buffer, LONG len)
{
	LONG end;

	setresult2(0);
	if (*name < len-1)
		end = *name;
	else {
		end = len - 1;
		setresult2(ERROR_LINE_TOO_LONG);
	}

	if (len != 0) {
		copyMem(name+1,buffer,end);
		buffer[end] = '\0';
	}
	return DOSTRUE;
}

LONG ASM
GetCurrentDirName (REG(d1) UBYTE *buffer, REG(d2) LONG len)
{
	struct CommandLineInterface *clip;

	clip = cli();
	if (!clip)
	{
		/* do a NameFromLock on pr_CurrentDir */
		return NameFromLock(currentdir(FALSE,NULL),buffer,len);
	}

	return uncommon_get((UBYTE *) BADDR(clip->cli_SetName),buffer,len);
}

LONG ASM
GetPrompt (REG(d1) UBYTE *buffer, REG(d2) LONG len)
{
	return common_get(offsetof(struct CommandLineInterface,cli_Prompt),
			  buffer,len);
}

LONG ASM
GetProgramName (REG(d1) UBYTE *buffer, REG(d2) LONG len)
{
	return common_get(offsetof(struct CommandLineInterface,cli_CommandName),
			  buffer,len);
}

/*************************************************************************/

static LONG __regargs
common_set (LONG offset, UBYTE *newname, LONG maxsize)
{
	struct CommandLineInterface *clip;
	UBYTE *name;
	LONG len;

	clip = cli();
	if (!clip)
		return FALSE;

	/* assumes the things are AllocVec()ed! */
	name = (UBYTE *) BADDR(*((LONG *) (((LONG) clip) + offset)));
	/* FIX!!!!!! */
	/*maxsize = *(((LONG *) name)-1) - 4;*/

	/* leave space for NULL and length byte */
	len = strlen(newname);
	if (maxsize-2 < len)
		len = maxsize-2;

	copyMem(newname,name+1,len);
	*name = len;
	name[len+1] = '\0';	/* null-terminate */

	return DOSTRUE;
}

LONG ASM
SetCurrentDirName (REG(d1) UBYTE *dirname)
{
	return common_set(offsetof(struct CommandLineInterface,cli_SetName),
			  dirname,SETMAX*4);
}

LONG ASM
SetPrompt (REG(d1) UBYTE *prompt)
{
	return common_set(offsetof(struct CommandLineInterface,cli_Prompt),
			  prompt,PROMPTMAX*4);
}

LONG ASM
SetProgramName (REG(d1) UBYTE *name)
{
	return common_set(offsetof(struct CommandLineInterface,cli_CommandName),
			  name,NAMEMAX*4);
}

