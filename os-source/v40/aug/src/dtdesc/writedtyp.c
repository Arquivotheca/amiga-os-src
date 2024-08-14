/* writedtyp.c
 *
 */

#include "dtdesc.h"

BOOL WriteDTYP (struct AppInfo * ai, STRPTR fname)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    BOOL retval = FALSE;
    STRPTR pattern;
    STRPTR basename;
    STRPTR name;
    ULONG msize;
    WORD *buf;
    UBYTE idstr[5];
    WORD i;

    name     = (STRPTR) ((struct StringInfo *) ai->ai_WinGad[AIG_DATATYPE]->SpecialInfo)->Buffer;
    basename = (STRPTR) ((struct StringInfo *) ai->ai_WinGad[AIG_BASENAME]->SpecialInfo)->Buffer;
    pattern  = (STRPTR) ((struct StringInfo *) ai->ai_WinGad[AIG_PATTERN] ->SpecialInfo)->Buffer;

    msize = DTSIZE + DTHSIZE;
    msize += strlen (name) + 1;
    msize += strlen (basename) + 1;
    msize += strlen (pattern) + 1;
    msize += (FNBUF_SIZE * sizeof (LONG)) + 1;

    if (dtn = AllocVec (msize, MEMF_CLEAR))
    {
	dth               = MEMORY_FOLLOWING (dtn);
	dtn->dtn_Header   = dth;
	dth->dth_Name     = MEMORY_FOLLOWING (dth);
	strcpy (dth->dth_Name, name);
	dth->dth_GroupID  = ai->ai_GroupID;

	for (i = 0; i < 5; i++)
	    idstr[i] = 0;
	strncpy (idstr, name, 4);
	dth->dth_ID       = MAKE_ID (ToLower(idstr[0]), ToLower (idstr[1]), ToLower (idstr[2]), ToLower (idstr[3]));

	dtn->dtn_FunctionName = (STRPTR) ((struct StringInfo *) ai->ai_WinGad[AIG_FUNCTION]->SpecialInfo)->Buffer;

	dth->dth_BaseName = dth->dth_Name + strlen (name) + 1;
	strcpy (dth->dth_BaseName, basename);

	dth->dth_Pattern  = dth->dth_BaseName + strlen (basename) + 1;
	strcpy (dth->dth_Pattern, pattern);

	dth->dth_MaskLen  = 0;
	dth->dth_Mask = (WORD *) (dth->dth_Pattern + strlen (pattern) + 1);
	for (i = 0, buf = (WORD *) dth->dth_Mask; i < FNBUF_SIZE; i++)
	{
	    buf[i] = ai->ai_DBuffer[i];
	    if (ai->ai_DBuffer[i] >= 0)
	    {
		dth->dth_MaskLen = (i + 1);
	    }
	}

	dth->dth_Flags = ai->ai_Flags & 0xFFFF;

	dth->dth_Priority = (UWORD) ((struct StringInfo *) ai->ai_WinGad[AIG_PRIORITY]->SpecialInfo)->LongInt;

	retval = (BOOL) WriteDataType (ai, fname, dtn);

	FreeVec (dtn);
    }
    return (retval);
}

BOOL AddDTYP (struct AppInfo * ai)
{
    return (FALSE);
}
