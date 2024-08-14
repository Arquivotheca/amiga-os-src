/* readlist.c
 *
 */

#include "adddatatypes.h"

/*****************************************************************************/

/* Properties that we can deal with */
static LONG list_props[] =
{
    ID_DTYP, ID_DTHD,
    ID_DTYP, ID_CODE,
};

static LONG list_collections[] =
{
    ID_DTYP, ID_TOOL,
};

/*****************************************************************************/

static struct DataType * ASM GetLIST (REG (a6) struct GlobalData * gd, REG (a0) struct IFFHandle * iff)
{
    struct DataType *dtn = NULL;
    BOOL going = TRUE;

    if (OpenIFF (iff, IFFF_READ) == 0)
    {
	PropChunks (iff, list_props, 2);
	CollectionChunks (iff, list_collections, 1);
	if (StopOnExit (iff, ID_DTYP, ID_FORM) == 0)
	{
	    while (going)
	    {
		going = FALSE;
		switch (ParseIFF (iff, IFFPARSE_SCAN))
		{
		    case IFFERR_EOC:
			if (dtn = GetDTYPForm (gd, iff))
			{
			    if (AddDataTypeA (gd, dtn, NULL))
				going = TRUE;
			}
			break;

		    case IFFERR_EOF:
			break;

		    default:
			FreeVec (dtn);
			dtn = NULL;
			break;

		}
	    }
	}

	/* Close the IFF handle */
	CloseIFF (iff);
    }

    return (dtn);
}

/*****************************************************************************/

struct DataType * ASM ReadDataTypeList (REG (a6) struct GlobalData *gd, REG (d0) STRPTR name)
{
    struct DataType *dtn = NULL;
    struct IFFHandle *iff;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = Open (name, MODE_OLDFILE))
	{
	    InitIFFasDOS (iff);
	    dtn = GetLIST (gd, iff);
	    Close (iff->iff_Stream);
	}
	FreeIFF (iff);
    }
    else
    {
	SetIoErr (ERROR_NO_FREE_STORE);
    }
    return (dtn);
}
