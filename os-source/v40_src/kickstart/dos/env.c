/* env.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include "dos/dos.h"
#include "dos/var.h"
#include "dos/dosextens.h"
#include <string.h>

#include "libhdr.h"

#include "env_protos.h"
/*#include <clib/alib_protos.h>*/
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"

#define SAME 0

LONG ASM
DeleteVar (REG(d1) UBYTE *name, REG(d2) ULONG flags)
{
	return SetVar(name,NULL,0,flags);
}

/* flags is type or'ed with local/global flags */

LONG ASM
SetVar (REG(d1) UBYTE *name, REG(d2) UBYTE *buffer, REG(d3) LONG size,
	REG(d4) ULONG flags)
{
	struct LocalVar *lv;
	BPTR fh;
	BPTR dirlock;
	BPTR olddir;
	LONG rc = FALSE, err;

	/* handle null-terminated input */
	if (buffer && size == -1)
		size = strlen(buffer);		/* DON'T include NULL */

	if (!(flags & GVF_GLOBAL_ONLY))
	{
		UBYTE *newvalue = NULL;

		/* don't try to allocate 0 bytes, just use a NULL ptr */
		if (buffer && size)
			if (!(newvalue = AllocMem(size,0L)))
				return FALSE;

		/* local var - use list at end of process structure */
		for (lv = (struct LocalVar *) MYPROC->pr_LocalVars.mlh_Head;
		     lv->lv_Node.ln_Succ;
		     lv = (struct LocalVar *) lv->lv_Node.ln_Succ)
		{
			if (mystricmp(name,lv->lv_Node.ln_Name) == SAME &&
			    ((flags & 0xff) == lv->lv_Node.ln_Type))
			{
				/* got it! First free the old value */
				/* safe because we already alloced the new */
				if (lv->lv_Value)
					FreeMem((char *) lv->lv_Value,
						lv->lv_Len);

				if (buffer == NULL)
				{
					/* remove it */
					Remove(&(lv->lv_Node));
					freeVec(lv);

					return DOSTRUE;
				}
				/* replace old value */
fill_in:			lv->lv_Value = newvalue;
				lv->lv_Len   = size;
				lv->lv_Flags = flags & GVF_BINARY_VAR;
				if (size)
					copyMem(buffer,newvalue,size);

				return DOSTRUE;
			}
		}

		/* if buffer is NULL, then he wanted to delete it */
		if (buffer == NULL)
		{
			/* no local var of that name, delete the global (?!) */
			goto do_global;
		}

		/* doesn't exist - create it as local - may override global */
		if (!(lv = AllocVecPubClear(sizeof(*lv) + strlen(name) + 1)))
		{
			if (newvalue)
				FreeMem(newvalue,size);
			return FALSE;
		}

		lv->lv_Node.ln_Type = flags & 0xff;
		lv->lv_Node.ln_Name = ((char *) lv) + sizeof(*lv);
		strcpy(lv->lv_Node.ln_Name,name);

		addvar((struct List *) &(MYPROC->pr_LocalVars),
		       lv);
		goto fill_in;

	}

	/* either global was specified, or delete of var that doesn't exist */
	/* as a local variable.  */

do_global:
	/* FIX! currently doesn't search disk except for vars */
	if ((flags & GVF_LOCAL_ONLY) ||
	    ((flags & 0xff) != LV_VAR))
	{
		setresult2(ERROR_OBJECT_NOT_FOUND);
		return FALSE;
	}

	if (!(dirlock = locateobj("ENV:")))
		return FALSE;

	err = 0;
	while (dirlock)
	{
		olddir = CurrentDir(dirlock);

		if (buffer == NULL)	/* means delete */
		{
			/* delete the variable */
			rc = deletefile(name);
			goto free_dir;
		}

		/* now open relative to that.  */
		/* MODE_READWRITE creates if non-existant!!!! */
		/* MODE_READWRITE is not exclusive */
		if (fh = findstream(name,MODE_READWRITE))
		{
			/* file is open.  Write, then truncate. */
			/* Write of 0 bytes should return 0 */
			if (write(fh,buffer,size) == size)
			{
				/* make sure the var ends here */
				if (SetFileSize(fh,0,OFFSET_CURRENT) != -1)
					rc = DOSTRUE;
			}
			if (!endstream(fh))
				rc = FALSE;	/* ERROR on close(!) */
		} else {
			err = getresult2();
			if (err != ERROR_OBJECT_NOT_FOUND)
			{
write_err:			freeobj(CurrentDir(olddir));
				break;		// leave while (dirlock)
			}

			// missing directory.  Create it and try again.
			if (makedir(name))
			{
				CurrentDir(olddir);
				continue;	// retry the create
			} else
				goto write_err;	// can't make it, return err
		}
free_dir:
		freeobj(CurrentDir(olddir));
		dirlock = NULL;
		if (flags & GVF_SAVE_VAR)
		{
			// Make sure we don't try this again...
			flags &= ~GVF_SAVE_VAR;
			dirlock = locateobj("ENVARC:");
			// If this fails, we'll return success I guess
		}
	} // while (dirlock)

	setresult2(err);	// reset ioerr...
	return rc;
}

// given a pathname, build all needed subdirs to get there...
static LONG __regargs
makedir (char *path)
{
	char *newname = NULL, *end;
	BPTR lock;
	LONG i, rc = FALSE;;

	end = PathPart(path);
	if (end == path)
		return rc;	// at root of path - we failed

	newname = AllocVec((end+1) - path,0);
	if (!newname)
		return rc;

	// copy up to, but not including, *end - gives us parent
	for (i = 0; i < end-path; i++)
		newname[i] = path[i];
	newname[i] = '\0';

	if (lock = createdir(newname))
	{
		freeobj(lock);
		rc = DOSTRUE;
	} else {
		// if we couldn't make it because of higher dirs needed,
		// recurse...
		if (getresult2() == ERROR_OBJECT_NOT_FOUND)
			rc = makedir(newname);
	}
	if (newname)
		FreeVec(newname);
	return rc;
}

void __regargs
addvar (struct List *list, struct LocalVar *nlv)
{
	struct LocalVar *lv;
	char *name = nlv->lv_Node.ln_Name;

	for (lv = (struct LocalVar *) list->lh_Head;
	     lv->lv_Node.ln_Succ;
	     lv = (struct LocalVar *) lv->lv_Node.ln_Succ)
	{
		/* is name > lv->lv_Name? */
		if (mystricmp(name,lv->lv_Node.ln_Name) < 0)
		{
			/* place before this node */
			Insert(list,&(nlv->lv_Node),lv->lv_Node.ln_Pred);
			return;
		}
	}
	AddTail(list,&(nlv->lv_Node));
}

struct LocalVar * ASM
FindVar (REG(d1) UBYTE *name, REG(d2) ULONG type)
{
	struct LocalVar *lv;

	/* local var - use list at end of process structure */
	for (lv = (struct LocalVar *) MYPROC->pr_LocalVars.mlh_Head;
	     lv->lv_Node.ln_Succ;
	     lv = (struct LocalVar *) lv->lv_Node.ln_Succ)
	{
		/* make sure we only compare the right bits of type! */
		if (mystricmp(name,lv->lv_Node.ln_Name) == SAME &&
		    (type & 0xff) == lv->lv_Node.ln_Type)
		{
			return lv;
		}
	}

	setresult2(ERROR_OBJECT_NOT_FOUND);
	return NULL;
}

/* size MUST be at least 1!! */
/* returns the number of characters read into the buffer. */
/* for success, IoErr has the size of the file. */
LONG ASM
GetVar (REG(d1) UBYTE *name, REG(d2) UBYTE *buffer, REG(d3) LONG size,
	REG(d4) ULONG flags)
{
	struct LocalVar *lv;
	BPTR fh;
	BPTR dirlock;
	BPTR olddir;
	LONG len,err,fsize;

	if (size == 0)
	{
		setresult2(ERROR_BAD_NUMBER);	/* not good, fix */
		return -1;
	}

	if (!(flags & GVF_GLOBAL_ONLY))
	{
		/* ignore aliases and already-expanded variables */
		lv = FindVar(name,flags & 0xff);
		if (lv)
		{
			/* doesn't work for size==0! */
			/* note: for xfer_text, lv_Len+1 means it will only */
			/* copy a max of lv_Len characters, then null-term. */
			/* note that lv_Len may be 0, and lv_Value == NULL. */
			len = size < lv->lv_Len+1 ? size : lv->lv_Len+1;
			if (!(flags & GVF_BINARY_VAR))
			{
				/* ignore binary, return 1 line, strip \n */
				/* set len so we can return the right value */
				len = xfer_text(buffer,lv->lv_Value,len) + 1;
			} else {
				/* Note: len includes space for NULL */
				/* note: we don't deref value if len=1 */
				if (flags & GVF_DONT_NULL_TERM)
					len++;
				else
					buffer[len-1] = '\0';

				if (len > 1)
					copyMem(lv->lv_Value,buffer,len-1);
			}

			setresult2(lv->lv_Len);
			return (len-1);
		}

		/* didn't find it - currently, don't search ramdisk - FIX! */
		if (flags & GVF_LOCAL_ONLY ||
		    ((flags & 0xff) != LV_VAR))
		{
			/* result2 set by FindVar */
			return -1;
		}
	}

	/* not found locally (or we didn't look), try global */

	if (!(dirlock = locateobj("ENV:")))
		return -1;				/* doesn't exist */

	olddir = CurrentDir(dirlock);		/* from here on, use error: */

	/* now open relative to that. */
	len = -1;
	if (fh = findstream(name,MODE_OLDFILE)) /* FIX! use readonly! */
	{
		// size can't be 0 here
		buffer[0] = '\0';		// in case we read 0 bytes
		len = read(fh,buffer,size);
		if (len > 0)
		{
			// DONT_NULL_TERM is ONLY valid for binary!
			if (!(flags & GVF_DONT_NULL_TERM)) {
				// We null-terminate unless otherwise specified
				// don't overrun buffer
				if (len == size)
					len--;
				buffer[len] = '\0';
			}
			/* find the first \n and terminate if text */
			if (!(flags & GVF_BINARY_VAR))
			{
				char *s = buffer;

				while (*s)
				{
					if (*s++ == '\n')
						*--s = '\0';
					/* will cause a break */
				}
				// return number of bytes not including null
				len = s - buffer;
			}
		} // else buffer[0] = 0

		/* get the file size, unless there was an error */
		if (len >= 0 && seek(fh,0L,OFFSET_END) >= 0) /* go to end */
		{
			fsize = seek(fh,0L,OFFSET_END);
			/* returns current pos of end of file */
		} else
			len = -1;	/* error */
	}

	/* don't kill ioerr */
	err = getresult2();
	if (fh)
		endstream(fh);
	freeobj(CurrentDir(olddir));

	if (len >= 0)		/* was there an error? */
		err = fsize;	/* no - IoErr is size of variable */

	setresult2(err);	/* restore IoErr (or size) */
	return len;		/* number of characters transferred */
}

/* transfer, ignoring control chars, one line, strip \n */

LONG __regargs
xfer_text (char *buffer, char *source, LONG len)
{
	char *buf = buffer;

	// must check len first to avoid dereferencing past end
	// use --len to reserve space for null at end
	while (--len > 0 &&
	       *source && *source != '\n' && *source != '\r')
	{
		*buf++ = *source++;
	}
	*buf = '\0';

	return (LONG) (buf-buffer);
}

LONG __regargs
copyvars (struct Process *np, struct Process *oldp)
{
	struct LocalVar *lv, *nlv;

	for (lv = (struct LocalVar *)
		  oldp->pr_LocalVars.mlh_Head;
	     lv->lv_Node.ln_Succ;
	     lv = (struct LocalVar *) lv->lv_Node.ln_Succ)
	{
		if (!(nlv = AllocVecPub(sizeof(*lv) +
				        strlen(lv->lv_Node.ln_Name) + 1)))
		{
			/* failure! */
			return FALSE;
		}
		/* copy name, etc */
		nlv->lv_Node.ln_Type = lv->lv_Node.ln_Type;
		nlv->lv_Node.ln_Name = ((char *) nlv) + sizeof(*nlv);
		strcpy(nlv->lv_Node.ln_Name,lv->lv_Node.ln_Name);

		nlv->lv_Flags = lv->lv_Flags;
		nlv->lv_Value = NULL;
		if (lv->lv_Len)
		{
			if (!(nlv->lv_Value = AllocMem(lv->lv_Len,0)))
			{
				/* failure! */
				freeVec(nlv);
				return FALSE;
			}
			copyMem(lv->lv_Value,nlv->lv_Value,lv->lv_Len);
		}
		nlv->lv_Len = lv->lv_Len;

		AddTail((struct List *) &(np->pr_LocalVars),
			(struct Node *) nlv);
	}

	return TRUE;
}

void __regargs
freevars (struct Process *proc)
{
	struct LocalVar *lv, *next;
	struct MinList *lv_head;

	lv_head = &(proc->pr_LocalVars);
	for (lv = (struct LocalVar *) lv_head->mlh_Head;		  
	     lv->lv_Node.ln_Succ;
	     lv = next)
	{
/*kprintf("Freeing var %s (%ld), type = %ld\n",lv->lv_Node.ln_Name,lv->lv_Len,
lv->lv_Node.ln_Type);*/
		next = (struct LocalVar *) lv->lv_Node.ln_Succ;
		if (lv->lv_Value)
			FreeMem(lv->lv_Value,lv->lv_Len);
		freeVec(lv);
	}

	/* it's empty now */
	NewList((struct List *) lv_head);
}
