/* videocd.c
 * Copyright (c) 1993 Commodore-Amiga, Inc.
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <devices/cd.h>
#include "videocd_lib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "libbase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DT(x)	;

/*****************************************************************************/

#define	PAD_SIZE	151

/*****************************************************************************/

#define	TEMP_SIZE	32

/*****************************************************************************/

static void MyStrToLong (STRPTR buff, ULONG * value)
{
    register ULONG i, j;

    i = j = 0;
    while (buff[i])
    {
	j = (j * 10) + (buff[i] - '0');
	i++;
    }
    *value = j;
}

/*****************************************************************************/

static struct CDText *AllocCDText (struct LibBase *lb, STRPTR text, ULONG length, BOOL unpad)
{
    struct CDText *cs = NULL;

    if (length == 0)
	length = strlen (text);
    if (length)
    {
	if (unpad)
	    while ((text[length - 1] == ' ') && (length > 1))
		length--;

	if (cs = AllocVec (sizeof (struct CDText) + length + 1, MEMF_CLEAR))
	{
	    cs->cs_Length = length;
	    cs->cs_Text = MEMORY_FOLLOWING (cs);
	    strncpy (cs->cs_Text, text, length);
	}
    }
    return cs;
}

/*****************************************************************************/

#if 0
static LONG FindDigitalVideo (struct LibBase *lb, struct IOStdReq *io, struct CDIVolume *cd, ULONG ss)
{
    UBYTE tmp[TEMP_SIZE];
    LONG l, r, x;
    BOOL found;

    /* Now find the starting track */
    found = FALSE;
    l = cd->cd_PathTableAddress + cd->cd_PathTableSize + 1;
    r = l + (cd->cd_Space / 3);
    do
    {
	/* Get the sector number */
	x = (l + r) / 2;

	/* Read the track */
	io->io_Command = CD_READ;
	io->io_Data = (APTR) tmp;
	io->io_Length = sizeof (tmp);
	io->io_Offset = x * ss;
	if (DoIO ((struct IORequest *) io) == 0)
	{
	    if ((tmp[0] == 0x00) && (tmp[1] == 0x00) && (tmp[2] == 0x01) && (tmp[3] == 0xBA))
	    {
		found = TRUE;
		r = x - 1;
	    }
	    else
		l = x + 1;
	}
	else
	    r = x - 1;
    }
    while (l <= r);

    return ((found) ? x : 0);
}
#endif

/*****************************************************************************/

BOOL IsDigitalVideo (struct LibBase * lb, struct IOStdReq * io, struct CDIVolume * cd, ULONG ss)
{
    STRPTR ptr, pathTable;
    struct PathTable *pt;
    LONG i, j, cnt;

    STRPTR dtr, dirEntry;
    struct DirRecord *dr;
    UBYTE buff[64];
    LONG o, pad;

    DB (struct Library *DOSBase = OpenLibrary ("dos.library", 0));

    cnt = 0;
    pad = PAD_SIZE * ss;

    if (pathTable = ptr = AllocVec (cd->cd_PathTableSize, NULL))
    {
	/* Read the path table block */
	io->io_Command = CD_READ;
	io->io_Data = (APTR) pathTable;
	io->io_Length = i = cd->cd_PathTableSize;
	io->io_Offset = cd->cd_PathTableAddress * ss;
	if (DoIO ((struct IORequest *) io) == 0)
	{
	    while ((i > 0) && (cnt == 0))
	    {
		pt = (struct PathTable *) ptr;
		if (dirEntry = AllocVec (ss, NULL))
		{
		    /* Read the directory block */
		    io->io_Command = CD_READ;
		    io->io_Data = (APTR) dirEntry;
		    io->io_Length = ss;
		    io->io_Offset = pt->pt_DirBlock * ss;
		    if (DoIO ((struct IORequest *) io) == 0)
		    {
			dtr = dirEntry;
			while (dtr && (cnt == 0))
			{
			    dr = (struct DirRecord *) dtr;
			    if (dr->dr_Length)
			    {
				o = (LONG) ((dtr[32] & 1) ? dtr[32] : dtr[32] + 1);
				if (!(dtr[37 + o] & 0x80) && (dr->dr_Size > pad))
				{
				    /* Read the track */
				    io->io_Command = CD_READ;
				    io->io_Data = (APTR) buff;
				    io->io_Length = (ULONG) sizeof (buff);
				    io->io_Offset = (dr->dr_Block + PAD_SIZE) * ss;
				    if (DoIO ((struct IORequest *) io) == 0)
				    {
					if ((buff[0] == 0x00) && (buff[1] == 0x00) && (buff[2] == 0x01) && (buff[3] == 0xBA))
					    cnt++;
				    }
				}
				/* Next directory entry */
				dtr += dr->dr_Length;
			    }
			    else
				break;
			}
		    }
		    FreeVec (dtr);
		}
		j = 8 + ((pt->pt_Length & 1) ? pt->pt_Length + 1 : pt->pt_Length);
		ptr += j;
		i -= j;
	    }
	}
	FreeVec (pathTable);
    }
    DB (CloseLibrary (DOSBase));
    return (BOOL) ((cnt) ? 1 : 0);
}

/*****************************************************************************/

void AddTrack (struct LibBase *lb, struct List *list, struct Node *node)
{
    struct Node *insert;

    /* We first just add it to the end */
    AddTail (list, node);

    /* Now, run through the list to find the correct place */
    insert = list->lh_Head;
    while (insert != node)
    {
	if (((struct MTrack *) insert)->mt_Block > ((struct MTrack *) node)->mt_Block)
	{
	    RemTail (list);
	    if (insert == list->lh_Head)
		AddHead (list, node);
	    else
		Insert (list, node, insert->ln_Pred);
	    insert = node;
	}
	else
	    insert = insert->ln_Succ;
    }
}

/*****************************************************************************/

static struct MinList *ObtainDigitalVideo (struct LibBase *lb, struct IOStdReq *io, struct CDIVolume *cd, ULONG ss, ULONG * cntp)
{
    STRPTR ptr, pathTable;
    struct PathTable *pt;
    struct MinList *mlh;
    struct MTrack *mt;
    LONG i, j, cnt;

    STRPTR dtr, dirEntry;
    struct DirRecord *dr;
    UBYTE buff[64];
    LONG o, pad;

    cnt = 0;
    pad = ss * PAD_SIZE;
    if (mlh = AllocVec (sizeof (struct MinList), MEMF_CLEAR))
    {
	NewList ((struct List *) mlh);
	if (pathTable = ptr = AllocVec (cd->cd_PathTableSize, NULL))
	{
	    /* Read the path table block */
	    io->io_Command = CD_READ;
	    io->io_Data = (APTR) pathTable;
	    io->io_Length = i = cd->cd_PathTableSize;
	    io->io_Offset = cd->cd_PathTableAddress * ss;
	    if (DoIO ((struct IORequest *) io) == 0)
	    {
		while (i > 0)
		{
		    pt = (struct PathTable *) ptr;
		    if (dirEntry = AllocVec (ss, NULL))
		    {
			/* Read the directory block */
			io->io_Command = CD_READ;
			io->io_Data = (APTR) dirEntry;
			io->io_Length = ss;
			io->io_Offset = pt->pt_DirBlock * ss;
			if (DoIO ((struct IORequest *) io) == 0)
			{
			    dtr = dirEntry;
			    while (dtr)
			    {
				dr = (struct DirRecord *) dtr;
				if (dr->dr_Length)
				{
				    o = (LONG) ((dtr[32] & 1) ? dtr[32] : dtr[32] + 1);

				    /* Make sure it isn't a directory and that it is big enough */
				    if (!(dtr[37 + o] & 0x80) && (dr->dr_Size > pad))
				    {
					/* Read the track */
					io->io_Command = CD_READ;
					io->io_Data = (APTR) buff;
					io->io_Length = (ULONG) sizeof (buff);
					io->io_Offset = (dr->dr_Block + PAD_SIZE) * ss;
					if (DoIO ((struct IORequest *) io) == 0)
					{
					    if ((buff[0] == 0x00) && (buff[1] == 0x00) && (buff[2] == 0x01) && (buff[3] == 0xBA))
					    {
						if (mt = AllocVec (sizeof (struct MTrack), MEMF_CLEAR))
						{
						    cnt++;
						    mt->mt_Block = dr->dr_Block;
						    mt->mt_Size = dr->dr_Size / 2048;
						    AddTrack (lb, (struct List *) mlh, (struct Node *) mt);
						}
					    }
					}
				    }
				    /* Next directory entry */
				    dtr += dr->dr_Length;
				}
				else
				    break;
			    }
			}
			FreeVec (dtr);
		    }
		    j = 8 + ((pt->pt_Length & 1) ? pt->pt_Length + 1 : pt->pt_Length);
		    ptr += j;
		    i -= j;
		}
	    }
	    FreeVec (pathTable);
	}
    }
    *cntp = cnt;
    return mlh;
}

/*****************************************************************************/

static void ReleaseDigitalVideo (struct LibBase *lb, struct MinList *mlh)
{
    struct Node *ln;

    if (mlh)
    {
	while (ln = RemHead ((struct List *) mlh))
	    FreeVec (ln);
    }
}

/****** videocd.library/GetCDTypeA *****************************************
*
*   NAME
*	GetCDTypeA -- Determine type of CD
*
*   SYNOPSIS
*	type = GetCDTypeA (path, attrs);
*			    A0	 A1
*
*	LONG GetCDTypeA (STRPTR path, struct TagItem *attrs);
*
*   FUNCTION
*	This function is used to obtain the type of CD.
*
*   INPUTS
*	path -- Must be NULL which indicates cd.device.
*
*	attrs -- pointer to an array of tags providing optional extra
*	    parameters, or NULL
*
*   TAGS
*	none are currently defined.
*
*   RESULTS
*	type - Indicates the type of disc.  If CDT_ERROR is returned,
*	    then there was error when attempting to obtain the type
*	    of CD.
*
*	    CDT_ERROR - Error when attempting to obtain type.
*	    CDT_UNKNOWN - Unknown CD type
*	    CDT_MUSIC - Music CD
*	    CDT_KARAOKE - Karaoke CD 1.0
*	    CDT_GENERIC - Generic Data
*	    CDT_VIDEOCD - Video CD 1.1
*	    CDT_CDI - CDI
*	    CDT_CDIFMV - CDI Full Motion Video
*
*   SEE ALSO
*
******************************************************************************
*
*/

LONG ASM GetCDTypeA (REG (a0) STRPTR path, REG (a1)
		     struct TagItem *attrs, REG (a6)
		     struct LibBase *lb)
{
    LONG retval = CDT_ERROR;
    UBYTE buffer[TEMP_SIZE];
    ULONG offset, oss, ss;
    struct TagItem ti[2];
    struct CDIVolume *cd;
    struct IOStdReq *io;
    struct MsgPort *mp;
    struct RMSF *msf;
    struct CDInfo cdi;
    union CDTOC toc[2];

    DB (struct Library *DOSBase = OpenLibrary ("dos.library", 0));

    if (mp = CreateMsgPort ())
    {
	if (io = (struct IOStdReq *) CreateIORequest (mp, sizeof (struct IOStdReq)))
	{
	    if ((retval = OpenDevice ("cd.device", 0L, (struct IORequest *) io, 0L)) == 0)
	    {
		/* Get the information on the CD */
		io->io_Command = CD_INFO;
		io->io_Data = (APTR) & cdi;
		io->io_Length = sizeof (struct CDInfo);
		io->io_Offset = 0;
		if (DoIO ((struct IORequest *) io) == 0)
		{
		    retval = CDT_UNKNOWN;
		    if (cdi.Status)
		    {
			/* Get the sector size */
			oss = ss = cdi.SectorSize;

			/* See if it is a music disk */
			if (cdi.Status & CDSTSF_TOC)
			{
			    retval = CDT_MUSIC;
			    if (cdi.Status & CDSTSF_CDROM)
				retval = CDT_GENERIC;

			    /* Get the table of contents */
			    io->io_Command = CD_TOCMSF;
			    io->io_Data = (APTR) & toc;
			    io->io_Length = 2;
			    io->io_Offset = 0;
			    if (DoIO ((struct IORequest *) io) == 0)
			    {
				offset = 0;
				if (io->io_Actual > 0)
				{
				    msf = (struct RMSF *) &(toc[1].Entry.Position.MSF);
				    offset = MSFtoLSNoffset (msf->Minute, msf->Second, msf->Frame, 0, ss);
				}

				/* Read the track */
				io->io_Command = CD_READ;
				io->io_Data = (APTR) buffer;
				io->io_Length = TEMP_SIZE;
				io->io_Offset = MSFtoLSNoffset (0, 4, 0, offset, ss);
				if (DoIO ((struct IORequest *) io) == 0)
				{
				    if (strncmp (buffer, "VIDEO_CD", 8) == 0)
					retval = CDT_VIDEOCD;
				    else
				    {
					DB (buffer[8] = 0);
					DB (Printf ("buffer contains '%s'\n", buffer));
				    }
				}
				else
				{
				    DB (Printf ("couldn't read 00:04:00 : error=%ld\n", (LONG) io->io_Error));
				}

				/* Read the track */
				io->io_Command = CD_READ;
				io->io_Data = (APTR) buffer;
				io->io_Length = TEMP_SIZE;
				io->io_Offset = MSFtoLSNoffset (0, 3, 0, offset, ss);
				if (DoIO ((struct IORequest *) io) == 0)
				{
				    if (strncmp (buffer, "KARINFO.BIH", 11) == 0)
					retval = CDT_KARAOKE;
				    else
				    {
					DB (buffer[11] = 0);
					DB (Printf ("buffer contains '%s'\n", buffer));
				    }
				}
				else
				{
				    DB (Printf ("couldn't read 00:03:00 : error=%ld\n", (LONG) io->io_Error));
				}

				/* See if it could be CD-I */
				if ((retval != CDT_VIDEOCD) && (retval != CDT_KARAOKE))
				{
				    /* We have to configure it to sector size 2328 */
				    ss = 2328;
				    ti[0].ti_Tag = TAGCD_SECTORSIZE;
				    ti[0].ti_Data = ss;
				    ti[1].ti_Tag = TAG_DONE;
				    io->io_Command = CD_CONFIG;
				    io->io_Data = (APTR) ti;
				    io->io_Length = 0;
				    if (DoIO ((struct IORequest *) io) == 0)
				    {
					if (cd = AllocMem (sizeof (struct CDIVolume), MEMF_CLEAR))
					{
					    /* Read the track */
					    io->io_Command = CD_READ;
					    io->io_Data = (APTR) cd;
					    io->io_Length = sizeof (struct CDIVolume);

					    io->io_Offset = 16 * ss;
					    if (DoIO ((struct IORequest *) io) == 0)
					    {
						if ((cd->cd_Type == 0x01) &&
//						    (Strnicmp (cd->cd_ID, "CD-I", 4) == 0) &&
						    (Strnicmp (cd->cd_SystemID, "CD-RTOS ", 8) == 0))
						{
						    DB (Printf ("Trying CD-I\n"));
						    retval = CDT_CDI;
						    if (Strnicmp (cd->cd_ApplicationID, "cdi_video", 9) == 0)
							retval = CDT_CDIFMV;
						    else if (IsDigitalVideo (lb, io, cd, ss))
							retval = CDT_CDIFMV;
						}
					    }
					    FreeMem (cd, sizeof (struct CDIVolume));
					}
				    }

				    /* Restore the sector size */
				    ti[0].ti_Tag = TAGCD_SECTORSIZE;
				    ti[0].ti_Data = oss;
				    ti[1].ti_Tag = TAG_DONE;
				    io->io_Command = CD_CONFIG;
				    io->io_Data = (APTR) ti;
				    io->io_Length = 0;
				    DoIO ((struct IORequest *) io);
				}
			    }
			    else
			    {
				DB (Printf ("couldn't get table of contents\n"));
			    }
			}
			else
			{
			    DB (Printf ("no table of contents\n"));
			}
		    }
		    else
		    {
			DB (Printf ("no status\n"));
		    }
		}
		CloseDevice ((struct IORequest *) io);
	    }
	    DeleteIORequest ((struct IORequest *) io);
	}
	DeleteMsgPort (mp);
    }
    DB (CloseLibrary (DOSBase));
    return retval;
}

/****** videocd.library/ObtainCDHandleA *************************************
*
*   NAME
*	ObtainCDHandleA -- Get a handle on a CD
*
*   SYNOPSIS
*	handle = ObtainCDHandleA (path, attrs);
*	d0			  a0	a1
*
*	LONG ObtainCDHandleA (STRPTR path, struct TagItem *attrs);
*
*   FUNCTION
*	This function is used to obtain a handle on a CD for use
*	with the GetVideoCDInfoA() command.
*
*   INPUTS
*	path -- Must be NULL which indicates cd.device.
*
*	attrs -- pointer to an array of tags providing optional extra
*	    parameters, or NULL.
*
*   TAGS
*	GVCD_CodeSet (ULONG) - Preferred code set for returned strings.
*	    The value supplied here comes from the VideoCD spec.  The default
*	    is ISO 646.
*
*	    VCCS_ISO_646
*	    VCCS_ISO_8859_1
*	    VCCS_JIS -- JIS Roman [14] & JIS Kanji 1990 (168)
*	    VCCS_SHIFT_JIS -- Shifted JIS Kanji, including JIS Roman [14]
*		and JIS Katakana [13]
*
*   RESULTS
*	handle - A handle to pass to GetVideoCDInfoA().
*
*   SEE ALSO
*	GetVideoCDInfoA(), ReleaseCDHandle()
*
******************************************************************************
*
*/

APTR ASM ObtainCDHandleA (REG (a0) STRPTR pathName, REG (a1) struct TagItem * attrs, REG (a6) struct LibBase * lb)
{
    struct VideoCDHandle *vh;
    struct RMSF *msf, *msf2;
    UBYTE buffer[TEMP_SIZE];
    struct CDIVolume *cd;
    struct IOStdReq *io;
    UBYTE charset;

    /* Used for obtaining the attributes */
    struct TagItem *tags = attrs;
    struct TagItem *tstate;
    struct TagItem *tag;
    struct TagItem ti[4];
    LONG ss, oss;

    /* Establish defaults */
    charset = 1;

    /* Scan tags for attributes */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	switch (tag->ti_Tag)
	{
	    case GVCD_CodeSet:
		charset = (UBYTE) tag->ti_Data;
		break;
	}
    }

    /* Allocate our handle */
    if (vh = AllocVec (sizeof (struct VideoCDHandle), MEMF_CLEAR))
    {
	/* Initialize our semaphore */
	InitSemaphore (&vh->vh_Lock);

	/* Set the default character set */
	vh->vh_CharSet = charset;

	if (vh->vh_MP = CreateMsgPort ())
	{
	    if (vh->vh_IO = (struct IOStdReq *) CreateIORequest (vh->vh_MP, sizeof (struct IOStdReq)))
	    {
		io = vh->vh_IO;
		if (OpenDevice ("cd.device", 0L, (struct IORequest *) io, 0L) == 0)
		{
		    /* Get the information on the CD */
		    io->io_Command = CD_INFO;
		    io->io_Data = (APTR) & vh->vh_CDI;
		    io->io_Length = sizeof (struct CDInfo);

		    io->io_Offset = 0;
		    if (DoIO ((struct IORequest *) io) == 0)
		    {
			/* Get the real	sector size */
			vh->vh_SectorSize = ss = oss = vh->vh_CDI.SectorSize;

			/* Make sure there is a data CD in the drive */
			if (vh->vh_CDI.Status && (vh->vh_CDI.Status & CDSTSF_TOC) && (vh->vh_CDI.Status & CDSTSF_CDROM))
			{
			    /* Get the table of	contents */
			    io->io_Command = CD_TOCMSF;
			    io->io_Data = (APTR) & vh->vh_TOC;
			    io->io_Length = NUM_TOC_ENTRIES;
			    io->io_Offset = 0;
			    if (DoIO ((struct IORequest *) io) == 0)
			    {
				/* Get the offset */
				vh->vh_Offset = 0;
				vh->vh_Tracks = (ULONG) io->io_Actual - 2;
				if (io->io_Actual > 0)
				{
				    msf = (struct RMSF *) &(vh->vh_TOC[1].Entry.Position.MSF);
				    msf2 = (struct RMSF *) &(vh->vh_TOC[2].Entry.Position.MSF);
				    vh->vh_Offset = MSFtoLSNoffset (msf->Minute, msf->Second, msf->Frame, 0, vh->vh_SectorSize);
				    vh->vh_CDILength = MSFtoLSN (msf2->Minute, msf2->Second, msf2->Frame) - MSFtoLSN (msf->Minute, msf->Second, msf->Frame);
				}

				/* See if we have an InfoVCD track */
				io->io_Command = CD_READ;
				io->io_Data = (APTR) buffer;
				io->io_Length = sizeof (buffer);
				io->io_Offset = MSFtoLSNoffset (0, 4, 0, vh->vh_Offset, vh->vh_SectorSize);
				if (DoIO ((struct IORequest *) io) == 0)
				{
				    if (strncmp (buffer, "VIDEO_CD", 8) == 0)
				    {
					if (vh->vh_IVCD = AllocVec (sizeof (struct InfoVCD), MEMF_CLEAR))
					{
					    vh->vh_Flags |= VCDF_INFOVCD;

					    /* Read the InfoVCD track */
					    io->io_Command = CD_READ;
					    io->io_Data = (APTR) vh->vh_IVCD;
					    io->io_Length = sizeof (struct InfoVCD);

					    io->io_Offset = MSFtoLSNoffset (0, 4, 0, vh->vh_Offset, vh->vh_SectorSize);
					    DoIO ((struct IORequest *) io);
					}
				    }
				}

				/* See if we have an EntriesVCD track */
				io->io_Command = CD_READ;
				io->io_Data = (APTR) buffer;
				io->io_Length = sizeof (buffer);
				io->io_Offset = MSFtoLSNoffset (0, 4, 1, vh->vh_Offset, vh->vh_SectorSize);
				if (DoIO ((struct IORequest *) io) == 0)
				{
				    if (strncmp (buffer, "ENTRYVCD", 8) == 0)
				    {
					if (vh->vh_EVCD = AllocVec (sizeof (struct EntriesVCD), MEMF_CLEAR))
					{
					    vh->vh_Flags |= VCDF_ENTRYVCD;

					    /* Read the EntriesVCD track */
					    io->io_Command = CD_READ;
					    io->io_Data = (APTR) vh->vh_EVCD;
					    io->io_Length = sizeof (struct EntriesVCD);

					    io->io_Offset = MSFtoLSNoffset (0, 4, 1, vh->vh_Offset, vh->vh_SectorSize);
					    DoIO ((struct IORequest *) io);
					}
				    }
				}

				/* See if we have an KARINFO.BIH track */
				io->io_Command = CD_READ;
				io->io_Data = (APTR) buffer;
				io->io_Length = sizeof (buffer);
				io->io_Offset = MSFtoLSNoffset (0, 3, 0, vh->vh_Offset, vh->vh_SectorSize);
				if (DoIO ((struct IORequest *) io) == 0)
				{
				    if (strncmp (buffer, "KARINFO.BIH", 11) == 0)
				    {
					vh->vh_Buffer = AllocVec (vh->vh_SectorSize, MEMF_CLEAR);
					vh->vh_Text = AllocVec (MAX_TEXT, MEMF_CLEAR);
					if (vh->vh_Buffer && vh->vh_Text)
					{
					    vh->vh_KIH = (struct KarInfoHeader *) vh->vh_Buffer;

					    /* Show that we have Karaoke information */
					    vh->vh_Flags |= VCDF_KARINFO;

					    /* Read the Karinfo track */
					    io->io_Command = CD_READ;
					    io->io_Data = (APTR) vh->vh_Buffer;
					    io->io_Length = vh->vh_SectorSize;
					    io->io_Offset = MSFtoLSNoffset (0, 3, 0, vh->vh_Offset, vh->vh_SectorSize);
					    DoIO ((struct IORequest *) io);

					    vh->vh_Tracks = vh->vh_KIH->kih_NumTracks;
					}
				    }
				}

				/* See if it could be CD-I */
				if (vh->vh_Flags == NULL)
				{
				    /* We have to configure it to sector size 2328 */
				    ss = 2328;
				    ti[0].ti_Tag = TAGCD_SECTORSIZE;
				    ti[0].ti_Data = ss;
				    ti[1].ti_Tag = TAG_DONE;
				    io->io_Command = CD_CONFIG;
				    io->io_Data = (APTR) ti;
				    io->io_Length = 0;
				    if (DoIO ((struct IORequest *) io) == 0)
				    {
					if (vh->vh_CDIV = AllocVec (sizeof (struct CDIVolume), MEMF_CLEAR))
					{
					    /* Cache a pointer */
					    cd = vh->vh_CDIV;

					    /* Read the track */
					    io->io_Command = CD_READ;
					    io->io_Data = (APTR) cd;
					    io->io_Length = sizeof (struct CDIVolume);

					    io->io_Offset = 16 * ss;
					    if (DoIO ((struct IORequest *) io) == 0)
					    {
						if ((cd->cd_Type == 0x01) &&
//						    (Strnicmp (cd->cd_ID, "CD-I", 4) == 0) &&
						    (Strnicmp (cd->cd_SystemID, "CD-RTOS ", 8) == 0))
						{

						    /* Find the start */
						    if (vh->vh_CDITrackList = ObtainDigitalVideo (lb, io, cd, ss, &vh->vh_CDITracks))
						    {
							if (vh->vh_CDITracks)
							    vh->vh_Flags |= VCDF_CDIFMV;
						    }
						}
					    }
					}
				    }

				    /* Restore the sector size */
				    ti[0].ti_Tag = TAGCD_SECTORSIZE;
				    ti[0].ti_Data = oss;
				    ti[1].ti_Tag = TAG_DONE;
				    io->io_Command = CD_CONFIG;
				    io->io_Data = (APTR) ti;
				    io->io_Length = 0;
				    DoIO ((struct IORequest *) io);
				}

				/* Make sure we have the information we need */
				if (vh->vh_Flags)
				{
				    /* Add our handle to the list */
				    ObtainSemaphore (&lb->lb_Lock);
				    AddTail ((struct List *) &lb->lb_HandleList, (struct Node *) vh);
				    ReleaseSemaphore (&lb->lb_Lock);
				    return vh;
				}
			    }
			}
		    }
		    CloseDevice ((struct IORequest *) io);
		}
		DeleteIORequest ((struct IORequest *) io);
	    }
	    DeleteMsgPort (vh->vh_MP);
	}
	FreeVec (vh->vh_CCBuff);
	FreeVec (vh->vh_Buffer);
	FreeVec (vh->vh_Text);
	FreeVec (vh->vh_IVCD);
	FreeVec (vh->vh_EVCD);
	FreeVec (vh);
    }
    return NULL;
}

/****** videocd.library/GetVideoCDInfoA **************************************
*
*   NAME
*	GetVideoCDInfoA -- Obtain information on a VideoCD standard disc.
*
*   SYNOPSIS
*	info = GetVideoCDInfoA (handle, sequenceNumber, options);
*	D0		        A0      D0              A1
*
*	struct TagItem *GetVideoCDInfoA(APTR, ULONG, struct TagItem *);
*
*	info = GetVideoCDInfo(handle,sequenceNumber,firstTag,...);
*
*	struct TagItem *GetVideoCDInfoA(APTR, ULONG, Tag, ...);
*
*   FUNCTION
*	Obtain information on a Karaoke 1.0 or VideoCD 1.1 standard disc.
*
*	Information returned by this function falls into two general
*	categories: disk-level and sequence-level. Disk-level information
*	concerns describes the VideoCD as a whole, such as the
*	title of the disk. To obtain this information, you supply a sequence
*	number of 0. Sequence-level information describes an individual
*	sequence on a VideoCD. This information is obtained by providing
*	the sequence number corresponding to the sequence of interest.
*
*	The information is returned in the form of a tag list. Many
*	pieces of information stored on a VideoCD are optional. When
*	the information is not present, the associated tag will not
*	appear in the resulting tag list.
*
*   INPUTS
*	handle - Pointer to a handle obtained by GetCDHandleA().
*
*	attrs - pointer to an array of tags providing optional extra
*	    parameters, or NULL
*
*   TAGS
*	GVCD_CodeSet (ULONG) - Preferred code set for returned strings.
*	    The value supplied here comes from the VideoCD spec.  The default
*	    is ISO 646.
*
*	    VCCS_ISO_646
*	    VCCS_ISO_8859_1
*	    VCCS_JIS -- JIS Roman [14] & JIS Kanji 1990 (168)
*	    VCCS_SHIFT_JIS -- Shifted JIS Kanji, including JIS Roman [14]
*		and JIS Katakana [13]
*
*	GVCD_GetCodeSets (BOOL) -- Set this to TRUE to obtain a list of the
*	    available character sets.  This will cause the following tags to
*	    be returned when the disc is Karaoke:
*
*	    GVCD_NumCodeSets (ULONG) -- Number of code sets.
*	    GVCD_CodeSets (struct MinList *) -- Pointer to a list of
*		CodeSetNode structures describing the available code sets.
*
*   RESULT
*	info (struct TagItem *) -- A pointer to a tag list containing one
*	    tag for every piece of information obtained from the Video CD.
*	    This pointer will be NULL if an error occurred.
*
*   RESULT TAGS
*	The following tags are returned only when the sequence number is
*	0 and the disc contains Karaoke 1.0 information.
*
*	    GVCD_DiscTitle -- title of the disc.
*	    GVCD_DiscCatalogNum -- catalog number for disc.
*	    GVCD_DiscNumSequences -- number of sequences on the disc.
*	    GVCD_DiscDescription -- description of the disc.
*
*	The following tags are returned only when the sequence number is
*	0 and the disc contains VideoCD 1.1 information.
*
*	    GVCD_InfoVersion -- INFO.VCD version.
*	    GVCD_InfoAlbumID -- Album Identification.
*	    GVCD_InfoNumberVolumes -- Number of discs in set.
*	    GVCD_InfoVolumeNumber -- Volume number of this disc.
*	    GVCD_DiscNumSequences -- Number of video tracks.
*
*	The following tags are returned only when the sequence number is
*	greater than 0 and the disc contains VideoCD information.
*
*	    GVCD_EntriesUsed -- Number of chapter marks for this sequence.
*	    GVCD_EntriesArray -- Array of chapter marks in LSN format.
*
*	The following tags are returned only when the sequence number is
*	greater than 0 and the disc contains Karaoke information.
*
*	    GVCD_SequenceStartSector - Starting LSN of the sequence.
*	    GVCD_SequenceEndSector - Ending LSN of the sequence.
*
*	The following tags are returned only when the sequence number is
*	greater than 0 and the disc contains Karaoke information and
*	the tag was specified in the attrs input.
*
*	    GVCD_SequenceISRCCode
*	    GVCD_SequenceTitle -- title of current sequence
*	    GVCD_SequenceTitleSort -- title of sequence for sorting
*	    GVCD_SequencePerformer
*	    GVCD_SequencePerformerSort
*	    GVCD_SequenceWriter
*	    GVCD_SequenceComposer
*	    GVCD_SequenceArranger
*	    GVCD_SequencePlayer
*	    GVCD_SequenceTextHeader
*	    GVCD_SequenceText -- Exec list of CDTextNode structures
*	    GVCD_SequenceKareokiKey
*	    GVCD_SequenceOriginalKey
*	    GVCD_SequenceDescription -- Exec list of CDTextNode structures
*
*	The following tags can always be returned, regardless of the sequence
*	number requested.
*
*	    GVCD_CodeSet -- Code set of strings returned
*
*
*   SEE ALSO
*	FreeVideoCDInfo()
*
******************************************************************************
*
*	GVCD_GetSequenceOffset (BOOL) -- Set this to TRUE if you wish this
*	    function to try to return the GVCD_SequenceOffset tag. This tag
*	    can only be returned if you are looking at a real VideoCD, as
*	    opposed to a directory structure made to look like a VideoCD.
*
*	    GVCD_SequenceOffset -- offset on the CD where this sequence starts.
*		This tag is only returned if the GVCD_GetSequenceOffset tag
*		was specified on input.
*
*/

struct TagItem *ASM GetVideoCDInfoA (REG (a0)
				     struct VideoCDHandle *vh, REG (d0) ULONG seqNum,
				     REG (a1)
				     struct TagItem *attrs, REG (a6)
				     struct LibBase *lb)
{
    BOOL getcodesets = FALSE;
    BOOL success = TRUE;
    UBYTE charset;

    /* Used for obtaining the attributes */
    struct TagItem *tags, *tstate, *tag;
    ULONG tagt;

    /* Return data */
    struct TagItem *retTags = NULL;	/* Return value */
    struct TagItem rta[100];	/* Working tag array */
    ULONG rto = 0;		/* Offset into the working tag array */
    struct MinList *mlh = NULL;
    struct CodeSetNode *csn;
    struct KarInfoSeq *kis;
    register UBYTE i;
    UBYTE *cptr;
    STRPTR ptr;

    struct EntriesVCD *ev = vh->vh_EVCD;
    struct InfoVCD *iv = vh->vh_IVCD;;

    /* Establish defaults */
    charset = vh->vh_CharSet;

    /* Look for control parameters first */
    tstate = tags = attrs;
    while (tag = NextTagItem (&tstate))
    {
	switch (tag->ti_Tag)
	{
		/* Get the code set */
	    case GVCD_CodeSet:
		charset = (UBYTE) tag->ti_Data;

		/* If they are changing it, then free the old information */
		if (charset != vh->vh_CharSet)
		{
		    vh->vh_CharSet = charset;
		    FreeVec (vh->vh_CCBuff);
		    vh->vh_CCBuff = NULL;
		}
		break;

	    case GVCD_GetCodeSets:
		getcodesets = (BOOL) tag->ti_Data;
		break;
	}
    }

    /* See if we are supposed to be getting code sets */
    if ((vh->vh_Flags & VCDF_KARINFO) && getcodesets)
    {
	tag = &rta[rto++];
	tag->ti_Tag = GVCD_NumCodeSets;
	tag->ti_Data = (ULONG) vh->vh_KIH->kih_NumCoded;

	tag = &rta[rto++];
	tag->ti_Tag = GVCD_CodeSets;
	tag->ti_Data = (ULONG) AllocVec (sizeof (struct MinList), MEMF_CLEAR);

	mlh = (struct MinList *) tag->ti_Data;
	NewList ((struct List *) mlh);
    }

    /* Make sure we are the only ones accessing the information */
    ObtainSemaphore (&vh->vh_Lock);

    /* See if we need to muck with the code sets */
    if ((vh->vh_Flags & VCDF_KARINFO) && (getcodesets || (vh->vh_CCBuff == NULL)))
    {
	/* Now get information on the code files */
	vh->vh_CCOffset = vh->vh_CCLength = 0;
	ptr = &vh->vh_Buffer[32];
	for (i = 0; success && (i < vh->vh_KIH->kih_NumCoded); i++, ptr += sizeof (struct KarInfoSeq))
	{
	    /* Cast it to a KarInfoSeq */
	    kis = (struct KarInfoSeq *) ptr;

	    /* Add the entry to	the list of CodeSets */
	    if (getcodesets)
	    {
		if (csn = AllocVec (sizeof (struct CodeSetNode), MEMF_CLEAR))
		{
		    CopyMem (kis, &csn->csn_CharSet, sizeof (struct KarInfoSeq));

		    AddTail ((struct List *) mlh, (struct Node *) &csn->csn_Node);
		}
		else
		{
		    /* No free store */
		    success = FALSE;
		}
	    }

	    /* See if this is the code set they	asked for */
	    if (kis->kis_CharSet == charset)
	    {
		vh->vh_CCOffset = kis->kis_Offset;
		vh->vh_CCLength = kis->kis_Length;
	    }
	}
    }

    if (seqNum > vh->vh_Tracks)
    {
	success = FALSE;
    }
    else if (seqNum == 0)
    {
	DB (struct Library *DOSBase = OpenLibrary ("dos.library", 0));

	DB (Printf ("seqNum==0\n"));

	if (vh->vh_Flags & VCDF_CDIFMV)
	{
	    struct CDIVolume *cd = vh->vh_CDIV;
	    LONG i, j, k, count, offset;
	    struct MTrack *mt;
	    struct Node *ln;
	    ULONG *array;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoAlbumID;
	    tag->ti_Data = (ULONG) AllocCDText (lb, cd->cd_AlbumID, 128, TRUE);

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoNumberVolumes;
	    tag->ti_Data = (ULONG) cd->cd_NumVolumes;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoVolumeNumber;
	    tag->ti_Data = (ULONG) cd->cd_Volume;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_DigitalVideoTrackList;
	    tag->ti_Data = (ULONG) vh->vh_CDITrackList;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_NumDigitalVideoTracks;
	    tag->ti_Data = (ULONG) vh->vh_CDITracks;

#if 0
	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_DiscNumSequences;
	    tag->ti_Data = (ULONG) vh->vh_CDITracks;
#endif

#define	TIME_INC	(5 * 60 * 75)
#define	TIME_THRESH	(10 * 75)

	    /* Count the number of chapter marks */
	    count = 0;
	    for (ln = (struct Node *) vh->vh_CDITrackList->mlh_Head; ln->ln_Succ; ln = ln->ln_Succ)
	    {
		mt = (struct MTrack *) ln;
		DB (Printf ("%ld (%ld)\n", mt->mt_Size, TIME_THRESH));
		if (mt->mt_Size > TIME_THRESH)
		    count += (mt->mt_Size / TIME_INC);
	    }
	    DB (Printf ("count=%ld\n", count));

	    if (count)
	    {
		if (array = AllocVec (sizeof (ULONG) * (count + 1), MEMF_CLEAR))
		{
		    i = 0;
		    for (ln = (struct Node *) vh->vh_CDITrackList->mlh_Head; ln->ln_Succ; ln = ln->ln_Succ)
		    {
			mt = (struct MTrack *) ln;
			if (mt->mt_Size > TIME_THRESH)
			{
			    offset = mt->mt_Block;
			    k = mt->mt_Size / TIME_INC;
			    for (j = 0; (j < k) && (offset < mt->mt_Size); j++)
			    {
				array[i++] = offset;
				offset += TIME_INC;
			    }
			}
		    }
		    array[i] = 0;

		    tag = &rta[rto++];
		    tag->ti_Tag = GVCD_EntriesArray;
		    tag->ti_Data = (ULONG) array;

		    tag = &rta[rto++];
		    tag->ti_Tag = GVCD_EntriesUsed;
		    tag->ti_Data = (ULONG) count;
		}
	    }
	}
	else
	{
	    DB (Printf ("NOT CDI FMV\n"));
	}

	if (vh->vh_Flags & VCDF_INFOVCD)
	{
	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoVersion;
	    tag->ti_Data = (ULONG) ((iv->iv_Version[0] << 8) | iv->iv_Version[1]);

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoAlbumID;
	    tag->ti_Data = (ULONG) AllocCDText (lb, iv->iv_AlbumID, 0, FALSE);

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoNumberVolumes;
	    tag->ti_Data = (ULONG) iv->iv_NumVolumes;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_InfoVolumeNumber;
	    tag->ti_Data = (ULONG) iv->iv_Volume;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_DiscNumSequences;
	    tag->ti_Data = (ULONG) vh->vh_Tracks;
	}

	if (vh->vh_Flags & VCDF_ENTRYVCD)
	{
	    UWORD i, count = 0;
	    ULONG *array;
	    ULONG e;

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_EntriesVersion;
	    tag->ti_Data = (ULONG) ((ev->ev_Version[0] << 8) | ev->ev_Version[1]);

	    if (ev->ev_NumEntries && (array = AllocVec (sizeof (ULONG) * ev->ev_NumEntries, MEMF_CLEAR)))
	    {
		for (i = count = 0; i < ev->ev_NumEntries; i++)
		{
		    e = ev->ev_Entries[i];
		    array[count++] = MSFtoLSN (((e & 0xFF0000) >> 16), ((e & 0xFF00) >> 8), (e & 0xFF));
		}
		array[count] = 0;

		tag = &rta[rto++];
		tag->ti_Tag = GVCD_EntriesArray;
		tag->ti_Data = (ULONG) array;
	    }

	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_EntriesUsed;
	    tag->ti_Data = (ULONG) count;
	}

#if 0
	if (vh->vh_Flags & VCDF_KARINFO)
	{
	    tag = &rta[rto++];
	    tag->ti_Tag = GVCD_DiscNumSequences;
	    tag->ti_Data = (ULONG) vh->vh_Tracks;
	}
#endif

	DB (CloseLibrary (DOSBase));
    }

    /* See if we have something	in our language	*/
    if ((vh->vh_Flags & VCDF_KARINFO) && success && vh->vh_CCOffset)
    {
	UWORD g, glen;
	UBYTE itype;

	if (vh->vh_CCBuff == NULL)
	{
	    ULONG size = vh->vh_SectorSize * vh->vh_CCLength;

	    if (vh->vh_CCBuff = AllocVec (size, MEMF_CLEAR))
	    {
		/* Read the track */
		vh->vh_IO->io_Command = CD_READ;
		vh->vh_IO->io_Data = (APTR) vh->vh_CCBuff;
		vh->vh_IO->io_Length = size;
		vh->vh_IO->io_Offset = MSFtoLSNoffset (0, 3, vh->vh_CCOffset, vh->vh_Offset, vh->vh_SectorSize);
		if (DoIO ((struct IORequest *) vh->vh_IO) == 0)
		{
		}
		else
		    success = FALSE;
	    }
	    else
		success = FALSE;
	}

	if (success)
	{
	    cptr = vh->vh_CCBuff;

	    g = 0;
	    do
	    {
		glen = (UWORD) ((cptr[0] << 8) + cptr[1]);

		/* Skip past the group length */
		cptr += 2;

		/* See if this is the sequence number they asked for */
		if (g == seqNum)
		{
		    if (seqNum > 0)
		    {
			struct RMSF *smsf, *emsf;
			ULONG slsn, elsn;

			smsf = (struct RMSF *) &(vh->vh_TOC[seqNum + 1].Entry.Position.MSF);
			emsf = (struct RMSF *) &(vh->vh_TOC[seqNum + 2].Entry.Position.MSF);

			if (seqNum + 2 >= vh->vh_Tracks)
			    emsf = (struct RMSF *) &(vh->vh_TOC[0].Summary.LeadOut);
			slsn = MSFtoLSN (smsf->Minute, smsf->Second, smsf->Frame);
			elsn = MSFtoLSN (emsf->Minute, emsf->Second, emsf->Frame) - 1;

			tag = &rta[rto++];
			tag->ti_Tag = GVCD_SequenceStartSector;
			tag->ti_Data = slsn;

			tag = &rta[rto++];
			tag->ti_Tag = GVCD_SequenceEndSector;
			tag->ti_Data = elsn;
		    }

		    /* Make sure we have a valid length */
		    if ((glen > 1) && (glen < 0xFFFE))
		    {
			UBYTE *tbuff = vh->vh_Text;
			register ULONG di, si;
			UBYTE *gptr = cptr;
			WORD ilen;
			BOOL out;

			di = 0;
			out = FALSE;
			do
			{
			    /* Get the item information */
			    itype = gptr[0];
			    ilen = (WORD) gptr[1];

			    /* Skip over the item information */
			    gptr += 2;

			    /* See if we have a length */
			    if (ilen)
			    {
				for (si = 0; (si < ilen) && (di < MAX_TEXT - 1); si++, di++)
				    tbuff[di] = gptr[si];
				tbuff[di] = 0;
				if (ilen != 0xFF)
				    out = TRUE;
			    }
			    else if (di)
				out = TRUE;

			    if (out)
			    {
				/* See if it is a tag that they asked for */
				tagt = GVCD_DiscTitle + (ULONG) (itype & 0x3F);
				if (FindTagItem (tagt, attrs))
				{
				    tag = &rta[rto++];
				    tag->ti_Tag = tagt;
				    switch (tag->ti_Tag)
				    {
					case GVCD_DiscNumSequences:
					    MyStrToLong (tbuff, (ULONG *) & tag->ti_Data);
					    break;

					default:
					    tag->ti_Data = (ULONG) AllocCDText (lb, tbuff, di, FALSE);
					    break;
				    }
				}

				di = 0;
				out = FALSE;
			    }

			    /* Next item */
			    gptr += ilen;
			}
			while (!(itype & 0x80));
		    }
		}
		cptr += glen;
		g++;
	    }
	    while (glen && (g <= seqNum));
	}
    }

    /* All done now */
    ReleaseSemaphore (&vh->vh_Lock);

    if (success)
    {
	tag = &rta[rto];
	tag->ti_Tag = TAG_DONE;
	retTags = CloneTagItems (rta);
    }

    return (retTags);
}

/****** videocd.library/FreeVideoCDInfo **************************************
*
*   NAME
*	FreeVideoCDInfo -- frees resources allocated by GetVideoCDInfo()
*
*   SYNOPSIS
*	FreeVideoCDInfo(info);
*		        A0
*
*	VOID FreeVideoCDInfo(info);
*
*   FUNCTION
*	FreeVideoCDInfo() frees any system resources allocated by
*	GetVideoCDInfo().
*
*   INPUTS
*	info - pointer obtained from GetVideoCDInfo(), or NULL
*	       which case this function does nothing.
*
*   SEE ALSO
*	GetVideoCDInfo()
*
******************************************************************************
*
*/

void ASM FreeVideoCDInfo (REG (a0)
			  struct TagItem *retTags, REG (a6)
			  struct LibBase *lb)
{
    /* Used for obtaining the attributes */
    struct TagItem *tags = retTags;
    struct TagItem *tstate;
    struct TagItem *tag;

    struct MinList *mlh;
    struct Node *ln;

    /* Make sure we have something to free */
    if (retTags)
    {
	/* Deallocate each individual attribute */
	tstate = tags;
	while (tag = NextTagItem (&tstate))
	{
	    switch (tag->ti_Tag)
	    {
		    /* Numeric tags go here */
		case GVCD_DiscNumSequences:
		case GVCD_NumCodeSets:
		case GVCD_SequenceStartSector:
		case GVCD_SequenceEndSector:
		    /* Nothing to do for these */
		case GVCD_InfoVersion:
		case GVCD_InfoAlbumID:
		case GVCD_InfoNumberVolumes:
		case GVCD_InfoVolumeNumber:
		case GVCD_EntriesVersion:
		case GVCD_EntriesUsed:
		case GVCD_DigitalVideoTrackList:
		case GVCD_NumDigitalVideoTracks:
		    break;

		case GVCD_CodeSets:
		    if (mlh = (struct MinList *) tag->ti_Data)
		    {
			while (ln = RemHead ((struct List *) mlh))
			    FreeVec (ln);
			FreeVec (mlh);
		    }
		    break;

		    /* CDString tags go here */
		default:
		    FreeVec ((APTR) tag->ti_Data);
		    break;
	    }
	}

	/* Now free the tag list */
	FreeTagItems (retTags);
    }
}

/****** videocd.library/ReleaseCDHandle ***********************************
*
*   NAME
*	ReleaseCDHandle -- Release a CD handle.
*
*   SYNOPSIS
*	ReleaseCDHandle (handle)
*			 a0
*
*	VOID ReleaseCDHandle (APTR handle);
*
*   FUNCTION
*	This function is used to release a handle obtained with
*	ObtainCDHandle().
*
*   INPUTS
*	handle -- Pointer returned by ObtainCDHandle().
*
*   SEE ALSO
*	GetVideoCDInfoA(), ObtainCDHandleA()
*
******************************************************************************
*
*/

VOID ASM ReleaseCDHandle (REG (a0)
			  struct VideoCDHandle *vh, REG (a6)
			  struct LibBase *lb)
{
    if (vh)
    {
	/* Remove our handle from the list */
	ObtainSemaphore (&lb->lb_Lock);
	Remove ((struct Node *) vh);
	ReleaseSemaphore (&lb->lb_Lock);

	/* Shut everything down */
	CloseDevice ((struct IORequest *) vh->vh_IO);
	DeleteIORequest ((struct IORequest *) vh->vh_IO);
	DeleteMsgPort (vh->vh_MP);
	FreeVec (vh->vh_CCBuff);
	FreeVec (vh->vh_Buffer);
	FreeVec (vh->vh_Text);
	FreeVec (vh->vh_IVCD);
	FreeVec (vh->vh_EVCD);
	FreeVec (vh->vh_CDIV);
	ReleaseDigitalVideo (lb, vh->vh_CDITrackList);
	FreeVec (vh);
    }
}
