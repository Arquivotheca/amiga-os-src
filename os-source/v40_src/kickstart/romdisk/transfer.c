/* transfer.c */

#include "ram_includes.h"
#include "ram.h"

LONG read(struct lock *lock,CPTR buf,LONG bsize)
{
struct data *pos;
LONG offset;
LONG count=0;

	/* get data about the file */
	offset = lock->offset;		/* offset of next valid position */

	if (pos=(struct data *)(lock->block))
	{
		count = pos->size - offset;
		if (count > bsize) count=bsize;
		CopyMem(((char *)pos) + offset,(char *)buf,count);
		lock->offset += count;
		lock->cpos +=count;
	}

	return count;	/* number of bytes read/written */
}
