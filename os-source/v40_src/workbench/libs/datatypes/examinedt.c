/* examinedt.c
 *
 */

#include "datatypesbase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DN(x)	;

/*****************************************************************************/

/* A table of valid ASCII characters (7 bits). */
static BYTE ASCIITable[256] =
{
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* A table of clearly invalid ASCII characters (8 bits). */
static BYTE BinaryTable[256] =
{
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*****************************************************************************/

/* size=size of file */
static struct DataType *sniffbuffer (struct DataTypesLib *dtl, struct DTHookContext *dthc, STRPTR buffer, WORD len, LONG size, STRPTR filename)
{
    struct ClipboardHandle *clip = NULL;
    struct Token *t = dtl->dtl_Token;
    struct DataType *dtn = NULL;
    struct DataTypeHeader *dth;
    struct IFFHandle *iff;
    BOOL match = FALSE;
    BOOL reset = FALSE;
    struct List *list;
    struct Node *node;
    UBYTE ch, uch;
    ULONG head;
    ULONG next;
    WORD ascii;
    WORD type;
    WORD i, j;
    BOOL iffc;
    WORD perc;

    head = MAKE_ID (buffer[0], buffer[1], buffer[2], buffer[3]);
    next = MAKE_ID (buffer[4], buffer[5], buffer[6], buffer[7]) + 8;

    if ((iff = dthc->dthc_IFF) && (dthc->dthc_FileHandle == NULL))
    {
	type = DTF_IFF;
	list = &t->t_IFFList;
	clip = (struct ClipboardHandle *) iff->iff_Stream;
    }
    else
    {
	iffc = FALSE;
	if ((next >= (3 * (size / 4))) && (next <= (4 * (size / 3))))
	    iffc = TRUE;

	/* IFF file */
	if (iffc && (head == MAKE_ID ('F', 'O', 'R', 'M')))
	{
	    type = DTF_IFF;
	    list = &t->t_IFFList;
	}
	/* IFF CAT file */
	else if (iffc && (head == MAKE_ID ('C', 'A', 'T', ' ')))
	{
	    type = DTF_IFF;
	    list = &t->t_IFFList;
	}
	/* IFF LIST file */
	else if (iffc && (head == MAKE_ID ('L', 'I', 'S', 'T')))
	{
	    type = DTF_IFF;
	    list = &t->t_IFFList;
	}
	else
	{
	    dthc->dthc_IFF = NULL;
	    type = DTF_BINARY;
	    list = &t->t_BinaryList;
	    for (j = ascii = 0; j < len; j++)
	    {
		if (ASCIITable[buffer[j]])
		    ascii++;
		else
		{
		    if (BinaryTable[buffer[j]])
		    {
			DB (kprintf ("contains %ld : %02lx\n", (ULONG) buffer[j], (ULONG) buffer[j]));
			ascii = 0;
			break;
		    }
		}
	    }

	    perc = 3 * len / 4;

	    DB (kprintf ("ascii=%ld, len=%ld, perc=%ld\n", (LONG) ascii, (LONG) len, (LONG) perc));

	    if (ascii > perc)
	    {
		type = DTF_ASCII;
		list = &t->t_ASCIIList;
		DB (kprintf ("ascii data\n"));
	    }
	    else
	    {
		DB (kprintf ("binary data\n"));
	    }
	}
    }

    if (list)
    {
	for (node = list->lh_Head; (node->ln_Succ && (match == FALSE)); node = node->ln_Succ)
	{
	    dtn = (struct DataType *) node;
	    dth = dtn->dtn_Header;

	    DB (kprintf ("%s\n", dth->dth_Name));
	    if (dtn->dtn_Function && (dth->dth_MaskLen == 0))
	    {
		DB (kprintf (" compare with function\n"));
		match = (*dtn->dtn_Function) (dthc);
		reset = TRUE;
	    }
	    else if (len >= dth->dth_MaskLen)
	    {
		DB (kprintf (" compare with mask\n"));
		for (i = 0, match = TRUE; (i < dth->dth_MaskLen) && match; i++)
		{
		    if (dth->dth_Mask[i] >= 0)
		    {
			uch = ch = (UBYTE) dth->dth_Mask[i];
			if (!(dth->dth_Flags & DTF_CASE))
			    if ((uch = ToUpper (ch)) == ch)
				uch = ToLower (ch);

			if ((buffer[i] != ch) && (buffer[i] != uch))
			    match = FALSE;
		    }
		}

		if (match && !(dtn->dtn_SysFlags & DTSF_MATCHALL))
		{
		    DB (kprintf (" match file name\n"));
		    if (dtn->dtn_SysFlags & DTSF_ISWILD)
		    {
			if (MatchPatternNoCase (dtn->dtn_Token, filename) == 0)
			    match = FALSE;
		    }
		    else if (dth->dth_Pattern)
		    {
			if (!(Stricmp (dth->dth_Pattern, filename)))
			    match = FALSE;
		    }
		}

		if (match && dtn->dtn_Function)
		{
		    DB (kprintf (" match with function\n"));
		    match = (*dtn->dtn_Function) (dthc);
		    reset = TRUE;
		}
	    }

	    if (reset)
	    {
		if (iff)
		{
		    CloseIFF (iff);
		    /* Window of opportunity for the data to change */
		    OpenIFF (iff, IFFF_READ);
		}
		else
		    Seek (dthc->dthc_FileHandle, 0, OFFSET_BEGINNING);

		reset = FALSE;
	    }
	}
    }

    if (!match)
    {
	switch (type)
	{
	    case DTF_BINARY:
		dtn = (struct DataType *) FindNameI (list, "binary");
		break;

	    case DTF_ASCII:
		dtn = (struct DataType *) FindNameI (list, "ascii");
		break;

	    case DTF_IFF:
		dtn = (struct DataType *) FindNameI (list, "iff");
		break;
	}
    }

    return (dtn);
}

/*****************************************************************************/

static struct DataType *ASM ExamineClipboard (REG (a6) struct DataTypesLib *dtl, REG (a0) struct IFFHandle *iff)
{
    struct ClipboardHandle *clip = (struct ClipboardHandle *) iff->iff_Stream;
    struct DataType *dtn = NULL;
    struct DTHookContext dthc;
    UBYTE buffer[72];

    /* Read into the buffer */
    clip->cbh_Req.io_Offset  =
    clip->cbh_Req.io_Error   =
    clip->cbh_Req.io_ClipID  = 0;
    clip->cbh_Req.io_Command = CMD_READ;
    clip->cbh_Req.io_Data    = buffer;
    clip->cbh_Req.io_Length  = 64;

    DB (kprintf ("read 64 bytes from clipboard\n"));
    if (DoIO ((struct IORequest *) &clip->cbh_Req) == 0)
    {
	/* Reset stuff */
	clip->cbh_Req.io_Offset  =
	clip->cbh_Req.io_Error   = 0;

	/* Make sure we have enough data */
	if (clip->cbh_Req.io_Actual >= 12)
	{
	    dthc.dthc_SysBase = SysBase;
	    dthc.dthc_DOSBase = DOSBase;
	    dthc.dthc_IFFParseBase = IFFParseBase;
	    dthc.dthc_UtilityBase = UtilityBase;
	    dthc.dthc_Lock = NULL;
	    dthc.dthc_FIB = NULL;
	    dthc.dthc_FileHandle = NULL;
	    dthc.dthc_IFF = iff;
	    dthc.dthc_Buffer = buffer;
	    dthc.dthc_BufferLength = clip->cbh_Req.io_Actual;

	    DB (kprintf ("sniffbuffer\n"));
	    dtn = sniffbuffer (dtl, &dthc, buffer, clip->cbh_Req.io_Actual, 0L, "");
	}
	else
	{
	    DB (kprintf ("not enough data in clipboard\n"));
	    SetIoErr (ERROR_OBJECT_NOT_FOUND);
	}
    }
    else
    {
	SetIoErr (ERROR_OBJECT_NOT_FOUND);
	DB (kprintf ("couldn't get clipboard information\n"));
    }
    return (dtn);
}

/****i* datatypes.library/ExamineDT *******************************************
*
*    NAME
*	ExamineDT - Examine a file, and return its data type.   (V39)
*
*    SYNOPSIS
*	dtn = ExamineDT (lock, fib);
*	d0		 a0    a1
*
*	struct DataType *ExamineDT (BPTR, struct FileInfoBlock *);
*
*    FUNCTION
*	This function examines the file represented by the lock,
*	fills out the FileInfoBlock, and returns a DataType
*	record.
*
*    INPUTS
*	lock - A valid file lock.
*
*	fib - A pointer to a long-word aligned FileInfoBlock structure.
*
*    RETURNS
*	Success returns a pointer to a DataType.  Failure returns a NULL.
*	Use IoErr() to find out why.
*
*    SEE ALSO
*	ExamineDTHandleA()
*
*******************************************************************************
*
* Created:  28-Dec-91, David N. Junod
*
*/

struct DataType *ASM ExamineDT (REG (a6) struct DataTypesLib *dtl, REG (a0) BPTR lock,
				REG (a1) struct FileInfoBlock *fib)
{
    struct Token *t = dtl->dtl_Token;
    struct List *list = &t->t_AuxList;
    struct DataType *dtn = NULL;
    struct DTHookContext dthc;
    struct IFFHandle *iff;
    UBYTE nbuff[512];
    STRPTR buffer;
    WORD len;
    BPTR fh;

    DB (kprintf ("ExamineDt\n"));
    ObtainSemaphoreShared (&t->t_Lock);

    if (Examine (lock, fib))
    {
	if (fib->fib_DirEntryType > 0)
	{
	    /* Directory */
	    DB (kprintf ("directory\n"));
	    dtn = (struct DataType *) FindNameI (list, "directory");
	}
	else if (fib->fib_DirEntryType == 0)
	{
	    /* Error */
	    DB (kprintf ("error!\n"));
	}
	else if (NameFromLock (lock, nbuff, 510))
	{
	    if (fh = Open (nbuff, MODE_OLDFILE))	/* OpenFromLock (dup)) */
	    {
		len = MAX (64, t->t_MaxLen);
		if (buffer = AllocVec (len + 1, MEMF_CLEAR))
		{
		    if ((len = (WORD) Read (fh, buffer, (LONG) len)) > 0)
		    {
			/* Seek back to the beginning */
			Seek (fh, 0, OFFSET_BEGINNING);

			dthc.dthc_SysBase = SysBase;
			dthc.dthc_DOSBase = DOSBase;
			dthc.dthc_IFFParseBase = IFFParseBase;
			dthc.dthc_UtilityBase = UtilityBase;
			dthc.dthc_Lock = lock;
			dthc.dthc_FIB = fib;
			dthc.dthc_FileHandle = fh;
			dthc.dthc_Buffer = buffer;
			dthc.dthc_BufferLength = len;

			if (dthc.dthc_IFF = iff = AllocIFF ())
			{
			    iff->iff_Stream = fh;
			    InitIFFasDOS (iff);
			    if (OpenIFF (iff, IFFF_READ) == 0)
			    {
				dtn = sniffbuffer (dtl, &dthc, buffer, len, fib->fib_Size, fib->fib_FileName);
				CloseIFF (iff);
			    }
			    FreeIFF (iff);
			}
			else
			{
			    SetIoErr (ERROR_NO_FREE_STORE);
			}
		    }
		    FreeVec (buffer);
		}
		Close (fh);
	    }
	}
    }

    ReleaseSemaphore (&t->t_Lock);

    return (dtn);
}

/****** datatypes.library/ObtainDataTypeA ***********************************
*
*    NAME
*	ObtainDataTypeA - Examines a handle and return its DataType.
*                                                               (V39)
*    SYNOPSIS
*	dtn = ObtainDataTypeA (type, handle, attrs);
*	d0			d0    a0      a1
*
*	struct DataType *ObtainDataTypeA (ULONG, APTR, struct TagItem *);
*
*	dtn = ObtainDataType (type, handle, tag1, ...);
*
*	struct DataType *ObtainDataType (ULONG, APTR, Tag tag1, ...);
*
*    FUNCTION
*	This function examines the data that the handle points to,
*	and returns a DataType record that describes the data.
*
*    INPUTS
*	type - Type of handle.
*
*	handle - Handle to examine.
*	    For DTST_FILE, handle must be BPTR lock.
*	    For DTST_CLIPBOARD, handle must be struct IFFHandle *.
*
*	attrs - Additional attributes (currently none are defined).
*
*    NOTES
*	The datatypes.library maintains a sorted list of all the
*	DataType descriptors.  The descriptor can consist of a
*	function, a data mask for the first 64 bytes of the data,
*	and a name pattern.
*
*	The sort order for the list is:
*
*	    Descriptors with a function and no mask or name pattern.
*	    Descriptors with a function and a mask or name pattern.
*	    Descriptors with no function and a mask or name pattern.
*
*	Within each group, they are also sorted in descending priority
*	and descending mask length.
*
*    RETURNS
*	Success returns a pointer to a DataType.  You must call
*	FreeDataType() when you are done with the handle.
*
*	A NULL return indicates failure.  Use IoErr() to get error value.
*	Following is a summary of the error number used and there meaning
*	as it relates to DataTypes.
*
*	ERROR_NO_FREE_STORE - Not enough memory.
*	ERROR_OBJECT_NOT_FOUND - Unable to open the data object.
*	ERROR_NOT_IMPLEMENTED - Unknown handle type.
*
*    SEE ALSO
*	FreeDataType()
*
*******************************************************************************
*
* Created:  1-Apr-92, David N. Junod
*
*/

struct DataType *ASM ObtainDataTypeA (REG (a6) struct DataTypesLib *dtl,
				      REG (d0) ULONG type, REG (a0) APTR handle,
				      REG (a1) struct TagItem *attrs)
{
    struct Token *t = dtl->dtl_Token;
    struct DataType *dtn = NULL;
    struct FileInfoBlock *fib;

    ObtainSemaphoreShared (&t->t_Lock);

    switch (type)
    {
	case DTST_FILE:
	    if (fib = AllocDosObject (DOS_FIB, TAG_DONE))
	    {
		dtn = ExamineDT (dtl, (BPTR) handle, fib);
		FreeDosObject (DOS_FIB, fib);
	    }
	    break;

	case DTST_CLIPBOARD:
	    dtn = ExamineClipboard (dtl, (struct IFFHandle *) handle);
	    break;

	default:
	    SetIoErr (ERROR_NOT_IMPLEMENTED);
	    break;
    }

    if (dtn)
    {
	DN (kprintf ("0x%lx [%s]\n", dtn->dtn_Header->dth_Name, dtn->dtn_Header->dth_Name));
	dtn->dtn_UseCnt++;
    }

    ReleaseSemaphore (&t->t_Lock);

    if (IoErr () == ERROR_OBJECT_NOT_FOUND)
	SetIoErr (DTERROR_COULDNT_OPEN);

    return (dtn);
}

/****** datatypes.library/ReleaseDataType ***********************************
*
*    NAME
*	ReleaseDataType - Release a DataType structure.         (V39)
*
*    SYNOPSIS
*	ReleaseDataType (dtn);
*		      a0
*
*	VOID ReleaseDataType (struct DataType *);
*
*    FUNCTION
*	This function is used to release a DataType structure obtained
*	by ObtainDataTypeA().
*
*    INPUTS
*	dtn - DataType structure returned by ObtainDataTypeA().  NULL
*	    is a valid input.
*
*    SEE ALSO
*	ObtainDataTypeA()
*
*******************************************************************************
*
* Created:  1-Apr-92, David N. Junod
*
*******************************************************************************
*
* Should decrement a use count.
*
*/

void ASM ReleaseDataType (REG (a6) struct DataTypesLib * dtl, REG (a0) struct DataType * dtn)
{
    struct Token *t = dtl->dtl_Token;

    ObtainSemaphoreShared (&t->t_Lock);

    if (dtn)
    {
	if (dtn->dtn_UseCnt)
	    dtn->dtn_UseCnt--;
    }

    ReleaseSemaphore (&t->t_Lock);
}
