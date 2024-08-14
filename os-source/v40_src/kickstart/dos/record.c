/* record.c */

#include <exec/types.h>
#include <exec/memory.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/record.h"
#include <string.h>

#include "libhdr.h"

#include "clib/exec_protos.h"
#include <pragmas/exec_old_pragmas.h>

#include "record_protos.h"
#include "klib_protos.h"
#include "blib_protos.h"

/* these are the record locking functions, based on Novell record locking */
/* LockRecord and UnLockRecord are in support.asm, since they use readwrite. */

LONG ASM
LockRecords (REG(d1) struct RecordLock *array, REG(d2) ULONG timeout)
{
	/* FIX!!! should sort list of records by offset to reduce deadlocks! */
	struct RecordLock *rec = array;

	while (rec->rec_FH)
	{
		if (!LockRecord(rec->rec_FH, rec->rec_Offset, rec->rec_Length,
				rec->rec_Mode, timeout))
		{
			LONG err = getresult2();	/* save error code */

			/* failure! unwind! */
			while (rec != array)
			{
				rec--;
				UnLockRecord(rec->rec_FH,rec->rec_Offset,
					     rec->rec_Length);
			}
			setresult2(err);		/* restore error code */
			return FALSE;
		}
		rec++;
	}

	return DOSTRUE;
}

LONG ASM
UnLockRecords (REG(d1) struct RecordLock *array)
{
	struct RecordLock *rec = array;
	LONG rc = DOSTRUE;

	while (rec->rec_FH)
	{
		/* tricky way to find if any failed */
		/* FIX! how do we indicate how they failed? */
		rc &= UnLockRecord(rec->rec_FH,rec->rec_Offset,rec->rec_Length);

		rec++;	/* point to next entry in the array */
	}

	return rc ? DOSTRUE : FALSE;	/* just in case of funny returns */
}
