/* exall.c */

#include "ram_includes.h"
#include "ram.h"
#include "dos/exall.h"

/* also works for examinefh, since fh_Arg1 == lock for me (object = 0) */

LONG
exnext (pkt,object)
	struct DosPacket *pkt;
	LONG object;			/* 0 = examine object, 1 = exnext */
					/* 2 = examine all */
{
	struct FileInfoBlock *ivec;
	struct node *node;
	struct node *ptr;
	struct lock *lock;

	lock = (struct lock *) (pkt->dp_Arg1 << 2);
	ptr  = checklock(lock);
	if (ptr == NULL)
		return FALSE;

	if (object == 2)		/* ExAll */
		return exall(ptr,pkt);

	ivec = (struct FileInfoBlock *) (pkt->dp_Arg2 << 2);
	node = (struct node *) ivec->fib_DiskKey;

	return setinfo(ivec,object == 0 ? ptr :
				          ptr == node ? node->list :
						        node->next);
}

LONG
exall (struct node *node, struct DosPacket *dp)
{
	struct ExAllData *ed	= (void *) dp->dp_Arg2;
	struct ExAllData *lasted = NULL;
	LONG size		= dp->dp_Arg3;
	LONG data		= dp->dp_Arg4;
	struct ExAllControl *ec = (void *) dp->dp_Arg5;
	struct node *curr;
	LONG savesize;

/*kprintf("in exall for %s (0x%lx, %ld, %ld, 0x%lx)\n",node->name,
dp->dp_Arg2,dp->dp_Arg3,dp->dp_Arg4,dp->dp_Arg5);
*/
	if (node->type != ST_USERDIR)
	{
		fileerr = ERROR_OBJECT_WRONG_TYPE;
		return FALSE;
	}

	/* check the data value */
	if (data < ED_NAME || data > ED_COMMENT)
	{
		fileerr = ERROR_BAD_NUMBER;	/* FIX!? */
		return FALSE;
	}
	/* we leave pointer to next node in ec->eac_LastKey */
	/* If it has been deleted, we're in deep shit. FIX! */
	/* I COULD temporarily lock it, since ExAlls should */
	/* always be finished.				    */

	ec->eac_Entries = 0;	/* no entries */

	for (curr = ec->eac_LastKey ? (struct node *) ec->eac_LastKey :
				      node->list;
	     curr;
	     curr = curr->next)
	{
/*kprintf("Looking at %s\n",curr->name);*/
		/* ok, we got here, fill in the buffer, check for overrun */
		savesize = size;
		if (!fill_data(ed,data,curr,&size))
		{
/*kprintf("Out of space\n");*/
			/* out of space, return partial */
			if (lasted)
				lasted->ed_Next = NULL; /* last entry */
			ec->eac_LastKey = (LONG) curr;
			return DOS_TRUE;
		}

		/* first do pattern matching */

		if (ec->eac_MatchString)
		{
			/* use default match func */
			if (!MatchPatternNoCase(ec->eac_MatchString,
						curr->name))
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
	fileerr = ERROR_NO_MORE_ENTRIES;
	return FALSE;
}

/* offsets for the first string for the different settings of data */
static UBYTE __far str_offset[ED_COMMENT] = {
	8,12,16,20,32,36
};

LONG
fill_data (struct ExAllData *ed, ULONG data, struct node *node, LONG *size)
{
	UBYTE *p = ((UBYTE *) ed) + str_offset[data-1];
	LONG mysize = 0;

	/* first take off required size */
	/* initial ed MUST be on multiple of 2 boundary!!!! */
	switch (data) {
	case ED_COMMENT:
		mysize += 4 + (node->comment ? strlen(node->comment) : 0) + 1;
	case ED_DATE:
		mysize += 3*4;
	case ED_PROTECTION:
		mysize += 4;
	case ED_SIZE:
		mysize += 4;
	case ED_TYPE:
		mysize += 4;
	case ED_NAME:
		mysize += 2*4 + strlen(node->name) + 1;
		if (mysize > *size)
			return FALSE;
	}
	if (mysize & 1)			/* if would put next on odd boundary */
		mysize++;

	*size -= mysize;

	/* we now know we have enough space - fill it in */
	switch (data) {
	case ED_COMMENT:
		ed->ed_Comment = p;
		*p = '\0';
		if (node->comment)
			strcpy(p,node->comment);
		p += strlen(p) + 1;		/* point to space for name */
	case ED_DATE:
		copydate(&(ed->ed_Days),&(node->date[0]));
	case ED_PROTECTION:
		ed->ed_Prot = node->prot;
	case ED_SIZE:
		ed->ed_Size = node->size;
	case ED_TYPE:
		ed->ed_Type = node->type;
	case ED_NAME:
		ed->ed_Name = p;
		strcpy(p,node->name);
		ed->ed_Next = (struct ExAllData *) (((LONG) ed) + mysize);
	}

	return TRUE;
}

/* this does the dirty work of examine, exnext */

LONG
setinfo (ivec,ptr)
	struct FileInfoBlock *ivec;
	struct node *ptr;
{
	LONG type;

	if (ptr == NULL)
	{
		fileerr = ERROR_NO_MORE_ENTRIES;
		return FALSE;
	}

	ivec->fib_DiskKey	= (LONG) ptr;	/* magic to user! */
	type                    =
	ivec->fib_DirEntryType	=
	ivec->fib_EntryType	= ptr->type;	/* out of order for speed */

	CtoBcpy(ivec->fib_FileName,ptr->name);		/* C strings to Bstr */

	/* report most info from the thing hard-linked to */
	if (type == ST_LINKFILE || type == ST_LINKDIR)
		ptr = ptr->list;

	ivec->fib_Protection	= ptr->prot;
	ivec->fib_Size		= ptr->size;
	ivec->fib_NumBlocks	= (ptr->size >> BLKSHIFT) + 1;
	ivec->fib_Date.ds_Days	= ptr->date[0];
	ivec->fib_Date.ds_Minute = ptr->date[1];
	ivec->fib_Date.ds_Tick	= ptr->date[2];
	ivec->fib_Comment[0]    = '\0';		/* BSTR! */

	/* handle null comments */
	if (ptr->comment)
		CtoBcpy(ivec->fib_Comment,ptr->comment);  /* C strings to bstr*/

	return DOS_TRUE;
}
