/* writedatatype.c
 *
 */

#include "dtdesc.h"

/*******************************************************************************/

#define	DB(x)	;

extern struct DiskObject DataTypeIcon;

/*******************************************************************************/

LONG WriteDataType (struct AppInfo *ai, STRPTR name, struct DataType * dtn)
{
    struct IFFHandle *iff;
    LONG retval = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = Open (name, MODE_NEWFILE))
	{
	    InitIFFasDOS (iff);
	    if (OpenIFF (iff, IFFF_WRITE) == 0L)
	    {
		if (PutDataType (ai, iff, dtn))
		{
		    retval = TRUE;
		}
		CloseIFF (iff);
	    }
	    Close (iff->iff_Stream);

	    if (retval)
	    {
		SetProtection (name, FIBF_EXECUTE);
		PutDiskObject (name, &DataTypeIcon);
	    }
	    else
		DeleteFile (name);
	}
	FreeIFF (iff);
    }
    else
	SetIoErr (ERROR_NO_FREE_STORE);

    return (retval);
}

/*******************************************************************************/

LONG PutDataType (struct AppInfo *ai, struct IFFHandle * iff, struct DataType * dtn)
{
    LONG status = FALSE;

    if (iff)
    {
	if (!(OpenIFF (iff, IFFF_WRITE)))
	{
	    status = (LONG) WriteDTHDChunk (ai, iff, dtn);
	    CloseIFF (iff);
	}
    }

    return (status);
}

/*******************************************************************************/

#define	ISODD(x)	(((x)&1)?1:0)

/*******************************************************************************/

BOOL WriteDTHDChunk (struct AppInfo *ai, struct IFFHandle * iff, struct DataType * dtn)
{
    struct DataTypeHeader *dth = dtn->dtn_Header;
    struct DataTypeHeader *tdth;
    BOOL status = FALSE;
    ULONG msize;
    LONG retval;
    ULONG mlen;
    ULONG len;
    WORD *buf;
    char *ptr;
    WORD i;

    mlen   = dth->dth_MaskLen * sizeof (WORD);
#if 0
    mlen  += ISODD (mlen);
#endif

    msize  = DTHSIZE;
    msize += mlen;
    msize += strlen (dth->dth_Name) + 1;
    msize += strlen (dth->dth_BaseName) + 1;
    msize += strlen (dth->dth_Pattern) + 1;

    if (tdth = AllocVec (msize, MEMF_CLEAR))
    {
	/* Copy the original DataType */
	CopyMem (dth, tdth, DTHSIZE);

	tdth->dth_Mask = (WORD *) DTHSIZE;
	DB (kprintf (" mask = 0x%08lx : len=%ld (%ld)\n", tdth->dth_Mask, (ULONG)dth->dth_MaskLen, mlen));
	ptr = (char *) ((ULONG) tdth + DTHSIZE);
	for (i = 0, buf = (WORD *) ptr; i < dth->dth_MaskLen; i++)
	    buf[i] = dth->dth_Mask[i];

	tdth->dth_Name = (STRPTR) (tdth->dth_Mask + dth->dth_MaskLen);
	DB (kprintf (" name = 0x%08lx : %s\n", tdth->dth_Name, dth->dth_Name));
	ptr = (char *) ((ULONG) tdth + DTHSIZE + mlen);
	strcpy (ptr, dth->dth_Name);

	tdth->dth_BaseName = (STRPTR) (tdth->dth_Name + strlen (dth->dth_Name) + 1);
	DB (kprintf (" base = 0x%08lx : %s\n", tdth->dth_BaseName, dth->dth_BaseName));
	ptr += strlen (dth->dth_Name) + 1;
	strcpy (ptr, dth->dth_BaseName);

	tdth->dth_Pattern = (STRPTR) (tdth->dth_BaseName + strlen (dth->dth_BaseName) + 1);
	DB (kprintf (" ptrn = 0x%08lx : %s\n", tdth->dth_Pattern, dth->dth_Pattern));
	ptr += strlen (dth->dth_BaseName) + 1;
	strcpy (ptr, dth->dth_Pattern);
	DB (kprintf ("size=%ld\n", msize));

	if ((retval = PushChunk (iff, ID_DTYP, ID_FORM, IFFSIZE_UNKNOWN)) == 0)
	{
	    len = strlen (dth->dth_Name);
	    DB (kprintf ("%s\n", dth->dth_Name));
	    if ((PushChunk (iff, 0, ID_NAME, IFFSIZE_UNKNOWN)) == 0)
		if ((WriteChunkBytes (iff, dth->dth_Name, len)) == len)
		    PopChunk (iff);

	    if ((PushChunk (iff, 0, ID_DTHD, IFFSIZE_UNKNOWN)) == 0)
		if ((WriteChunkBytes (iff, tdth, msize)) == msize)
		    if ((PopChunk (iff)) == 0L)
			status = TRUE;

	    /* Write the code chunk */
	    if (status)
	    {
		if (dtn->dtn_FunctionName)
		{
		    STRPTR buffer;
		    ULONG len;
		    BPTR fh;

		    if (buffer = AllocVec (4096L, MEMF_CLEAR))
		    {
			if (fh = Open (dtn->dtn_FunctionName, MODE_OLDFILE))
			{
			    status = FALSE;
			    if ((PushChunk (iff, 0, ID_CODE, IFFSIZE_UNKNOWN)) == 0)
			    {
				status = TRUE;
				while (((len = Read (fh, buffer, 4096)) > 0L) && status)
				    status = (WriteChunkBytes (iff, buffer, len) == len);
				PopChunk (iff);
			    }
			    Close (fh);
			}

			FreeVec (buffer);
		    }
		}
		else if (dtn->dtn_Executable)
		{
		    status = FALSE;
		    if ((PushChunk (iff, 0, ID_CODE, IFFSIZE_UNKNOWN)) == 0)
			if (WriteChunkBytes (iff, dtn->dtn_Executable, dtn->dtn_ExecSize) == dtn->dtn_ExecSize)
			    if (PopChunk (iff) == 0L)
				status = TRUE;
		}
	    }
	    PopChunk (iff);
	}
	FreeVec (tdth);
    }
    return (status);
}
