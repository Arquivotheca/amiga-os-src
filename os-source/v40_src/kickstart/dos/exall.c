/* exall.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include "dos/dos.h"
#include "dos/exall.h"
#include "dos/dosextens.h"
#include <string.h>

#include "libhdr.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>
#include "klib_protos.h"
#include "blib_protos.h"

#define AllocNew(x) ((struct x *) AllocMem(sizeof(struct x),\
					   MEMF_PUBLIC|MEMF_CLEAR));


/* called with args on stack! */
LONG __stdargs
fake_exall (BPTR lock, struct ExAllData *ed, LONG size, LONG data,
	    struct ExAllControl *ec)
{
	struct ExAllData *lasted = NULL;
	struct FileInfoBlock *fib;
	LONG savesize,fileerr;

	ec->eac_Entries = 0;	/* no entries */

	/* get old fib if this is a continuation */
	fib = (void *) (ec->eac_LastKey);
	if (!fib)
	{
		/* AllocDosObject makes sure OwnerUID/GID are 0 */
		fib = AllocDosObject(DOS_FIB,NULL);
		if (!fib)
			return NULL;

/*kprintf("in exall for %s (0x%lx, %ld, %ld, 0x%lx)\n",fib->fib_FileName,
	ed,size,data,ec);
*/
		/* first examine the directory (if this is the first time!) */
		if (!examine(lock,fib))
			goto cleanup;

		if (fib->fib_DirEntryType < 0)	/* file */
		{
			fileerr = ERROR_OBJECT_WRONG_TYPE;
			goto cleanup;
		}

		/* check the data value */
		if (data < ED_NAME || data > ED_OWNER)
		{
			fileerr = ERROR_BAD_NUMBER;	/* FIX!? */
			goto cleanup;
		}
	} else {
		// ugly and unstructured, but the easy and safe way to fix the
		// bug. if a previous exall didn't have enough room, fib has
		// valid data in it.
		goto have_fib;	// avoid exnext() first time through
	}

	/* we leave pointer to fib in ec->eac_LastKey       */
	/* If it has been deleted, we're in deep shit. FIX! */
	/* I COULD temporarily lock it, since ExAlls should */
	/* always be finished.				    */


	while (exnext(lock,fib))
	{
have_fib:
/*kprintf("Looking at %s\n",fib->fib_FileName);*/
		/* ok, we got here, fill in the buffer, check for overrun */
		savesize = size;
		if (!fill_data(ed,data,fib,&size))
		{
/*kprintf("Out of space\n");*/
			/* out of space, return partial */
			if (lasted)
				lasted->ed_Next = NULL;
			ec->eac_LastKey = (LONG) fib;
			/* don't free fib! */
			return DOSTRUE;
		}

		/* first do pattern matching */
		/* this is a ParsePattern()ed string! */
		if (ec->eac_MatchString)
		{
			/* use default match func */
			if (!MatchPatternNoCase(ec->eac_MatchString,
					        fib->fib_FileName))
				goto reject;
		}

		if (ec->eac_MatchFunc)
		{
			if (!do_match(ec,data,ed))
			{
reject:				size = savesize;
				continue;
			}
		}

		/* remember the last entry so we can NULL out ed_Next */
		lasted = ed;

/*kprintf("adding entry (%ld)\n",ec->eac_Entries+1);*/
		/* filled it in, bump count and move on */
		ec->eac_Entries++;
		ed = ed->ed_Next;
	}

	/* make sure last ed_Next entry is NULL */
	if (lasted)
		lasted->ed_Next = NULL;

/*kprintf("Done\n");*/
	/* finished, return that we're done (ERROR_NO_MORE_ENTRIES) */
	fileerr = getresult2();
cleanup:
	setresult2(fileerr);
	FreeDosObject(DOS_FIB,fib);
	return FALSE;
}

/* offsets for the first string for the different settings of data */
static UBYTE far str_offset[ED_OWNER] = {
	8,12,16,20,32,36,40
};

LONG __regargs
fill_data (struct ExAllData *ed, ULONG data, struct FileInfoBlock *fib,
	   LONG *size)
{
	UBYTE *p = ((UBYTE *) ed) + str_offset[data-1];
	LONG mysize = 0;

	/* first take off required size */
	/* initial ed MUST be on multiple of 2 boundary!!!! */
	switch (data) {
	case ED_OWNER:
		mysize += 4;
	case ED_COMMENT:
		mysize += 4 + strlen(fib->fib_Comment) + 1;
	case ED_DATE:
		mysize += 3*4;
	case ED_PROTECTION:
		mysize += 4;
	case ED_SIZE:
		mysize += 4;
	case ED_TYPE:
		mysize += 4;
	case ED_NAME:
		mysize += 2*4 + strlen(fib->fib_FileName) + 1;
		if (mysize > *size)
			return FALSE;
	}
	if (mysize & 1)			/* if would put next on odd boundary */
		mysize++;

	*size -= mysize;

	/* we now know we have enough space - fill it in */
	switch (data) {
	case ED_OWNER:
		ed->ed_OwnerUID = fib->fib_OwnerUID;
		ed->ed_OwnerGID = fib->fib_OwnerGID;
	case ED_COMMENT:
		ed->ed_Comment = p;
		strcpy(p,fib->fib_Comment);
		p += strlen(p) + 1;		/* point to space for name */
	case ED_DATE:
		*((struct DateStamp *) &(ed->ed_Days)) = fib->fib_Date;
	case ED_PROTECTION:
		ed->ed_Prot = fib->fib_Protection;
	case ED_SIZE:
		ed->ed_Size = fib->fib_Size;
	case ED_TYPE:
		ed->ed_Type = fib->fib_DirEntryType;
	case ED_NAME:
		ed->ed_Name = p;
		strcpy(p,fib->fib_FileName);
		ed->ed_Next = (struct ExAllData *) (((LONG) ed) + mysize);
	}

	return TRUE;
}

