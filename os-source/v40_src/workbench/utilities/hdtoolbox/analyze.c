/* Analyze.c */
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/filehandler.h>

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/* for lattice */
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>

#include "refresh.h"
#include "protos.h"

#include "global.h"

#include "gadgetids.h"
#include "gadgets.h"
#include "windows.h"

/* analyze which partitions will get tromped */
/* return TRUE if any will, else false       */

int
AnalyzePartitions (newrdb,oldrdb)
	struct RigidDiskBlock *newrdb,*oldrdb;
{
	register struct PartitionBlock *newp,*oldp;
	int retval = FALSE;
	short commitline = FIRST_LINE;	/* start parts here */

	*CommitText[commitline] = '\0';

	if (!newrdb || !oldrdb)
		return retval;

	newp = newrdb->rdb_PartitionList;
	oldp = oldrdb->rdb_PartitionList;

	/* partition lists are sorted by cyl number */
	while (oldp) {
		/* find partition that might match by cyl number */
		while (newp &&
		       (newp->pb_Environment[DE_LOWCYL] <
		        oldp->pb_Environment[DE_LOWCYL]))
		    newp = newp->pb_Next;

		if (newp)	// 39.9 --- fix enforcer hit
		{
			if (CheckParts(newp,oldp))
			{
				AddToCommit(oldp,&commitline);
				retval = TRUE;
			}
		}
		else	/* 39.17 --- Well, if "newp" is NULL, absolutely partition
			infomation is changed. */
		{
			AddToCommit(oldp,&commitline);
			retval = TRUE;
		}
		oldp = oldp->pb_Next;
	}

/* FIX  do we need to check bad blocks? */

	return retval;
}

int
CheckParts (newp,oldp)
	register struct PartitionBlock *newp,*oldp;
{
	if ((newp->pb_Environment[DE_LOWCYL] !=
	     oldp->pb_Environment[DE_LOWCYL])	 	||
	    (newp->pb_Environment[DE_UPPERCYL] !=
	     oldp->pb_Environment[DE_UPPERCYL])	 	||
	    (newp->pb_Environment[DE_DOSTYPE] !=
	     oldp->pb_Environment[DE_DOSTYPE])	 	||
	    (newp->pb_Environment[DE_RESERVEDBLKS] !=
	     oldp->pb_Environment[DE_RESERVEDBLKS])	||
	    (newp->pb_Environment[DE_PREFAC] !=
	     oldp->pb_Environment[DE_PREFAC])		||
	    (newp->pb_Environment[DE_SIZEBLOCK] !=
	     oldp->pb_Environment[DE_SIZEBLOCK])	||
	    (newp->pb_Environment[DE_SECSPERBLK] !=
	     oldp->pb_Environment[DE_SECSPERBLK])	||
	    (newp->pb_Environment[DE_INTERLEAVE] !=
	     oldp->pb_Environment[DE_INTERLEAVE])	||
	    (newp->pb_Environment[DE_NUMHEADS] !=
	     oldp->pb_Environment[DE_NUMHEADS])		||
	    (newp->pb_Environment[DE_BLKSPERTRACK] !=
	     oldp->pb_Environment[DE_BLKSPERTRACK])	||
	    (newp->pb_Environment[DE_SECORG] !=
	     oldp->pb_Environment[DE_SECORG]))
		return TRUE;

	return FALSE;
}

void
AddToCommit (p,line)
	struct PartitionBlock *p;
	register short *line;
{
	register int len;

	len = strlen(p->pb_DriveName) + 1;
	if (strlen(CommitText[*line]) > (70 - len))
	{
		if (*line < LAST_LINE - 1)
		{
			*line += 1;
			*(CommitText[*line + 1]) = '\0';
			strcpy(CommitText[*line],p->pb_DriveName);
			strcat(CommitText[*line],":");
		}
	} else {
		len = strlen(CommitText[*line]);
		*(CommitText[*line] + len) = ' ';
		strcpy(CommitText[*line] + len + 1,p->pb_DriveName);
		strcat(CommitText[*line] + len + 1,":");
	}
}
