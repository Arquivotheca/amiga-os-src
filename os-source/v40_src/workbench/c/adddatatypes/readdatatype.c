/* readdatatype.c
 *
 */

#include "adddatatypes.h"

/*****************************************************************************/

struct DataType *ASM ReadDataType (REG (a6) struct GlobalData *gd, REG (d1) STRPTR name)
{
    struct DataType *dtn = NULL;
    struct IFFHandle *iff;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = Open (name, MODE_OLDFILE))
	{
	    InitIFFasDOS (iff);
	    dtn = GetDataType (gd, iff);
	    Close (iff->iff_Stream);
	}
	FreeIFF (iff);
    }
    else
	SetIoErr (ERROR_NO_FREE_STORE);
    return (dtn);
}

/*****************************************************************************/

/* Properties that we can deal with */
static LONG dtyp_props[] =
{
    ID_DTYP, ID_DTHD,
    ID_DTYP, ID_CODE,
};

static LONG dtyp_collections[] =
{
    ID_DTYP, ID_TOOL,
};

/*****************************************************************************/

/* IFFHandle could be a file or clipboard at this point... */
struct DataType *ASM GetDataType (REG (a6) struct GlobalData *gd, REG (a0) struct IFFHandle * iff)
{
    struct DataType *dtn = NULL;

    if (OpenIFF (iff, IFFF_READ) == 0L)
    {
	PropChunks (iff, dtyp_props, 2);
	CollectionChunks (iff, dtyp_collections, 1);
	if (StopOnExit (iff, ID_DTYP, ID_FORM) == 0)
	    if (ParseIFF (iff, IFFPARSE_SCAN) == IFFERR_EOC)
		dtn = GetDTYPForm (gd, iff);
	CloseIFF (iff);
    }

    return (dtn);
}

/*****************************************************************************/

struct DataType *GetDTYPForm (struct GlobalData *gd, struct IFFHandle * iff)
{
    struct StoredProperty *sp;
    struct DataType *dtn;
    struct MyFileHandle fh;
    LONG stacksize = 4096;
    LONG fa[4];
    ULONG *ptr;

    if (dtn = GetDTHD (gd, iff))
    {
	if (sp = FindProp (iff, ID_DTYP, ID_CODE))
	{
	    if (dtn->dtn_Executable = AllocVec (sp->sp_Size, MEMF_PUBLIC))
	    {
		dtn->dtn_ExecSize = sp->sp_Size;
		CopyMem (sp->sp_Data, dtn->dtn_Executable, dtn->dtn_ExecSize);

		fh.data = (UBYTE *) dtn->dtn_Executable;
		fh.size = dtn->dtn_ExecSize;
		fh.ofs = 0;

		fa[0] = (LONG) ReadFunc;
		fa[1] = (LONG) AllocFunc;
		fa[2] = (LONG) FreeFunc;

		if (dtn->dtn_SegList = InternalLoadSeg ((BPTR) & fh, NULL, fa, ((LONG *) (&stacksize))))
		{
		    ptr = (ULONG *) (dtn->dtn_SegList << 2);
		    dtn->dtn_Function = (LONG (*ASM) (REG (a0) struct DTHookContext *))++ ptr;
		}
	    }
	}
    }

    return (dtn);
}

/*****************************************************************************/

struct DataType *GetDTHD (struct GlobalData *gd, struct IFFHandle * iff)
{
    struct DataTypeHeader *dth;
    struct StoredProperty *sp;
    struct DataType *dtn;
    ULONG msize;

    if (sp = FindProp (iff, ID_DTYP, ID_DTHD))
    {
	msize = DTSIZE + DTHSIZE + sp->sp_Size - DTHSIZE;
	if (dtn = AllocVec (msize, MEMF_CLEAR))
	{
	    dtn->dtn_Header = MEMORY_FOLLOWING (dtn);
	    dth = dtn->dtn_Header;

	    CopyMem (sp->sp_Data, dth, sp->sp_Size);

	    dth->dth_Name = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Name);
	    dth->dth_BaseName = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_BaseName);
	    dth->dth_Pattern = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Pattern);
	    dth->dth_Mask = (WORD *) ((ULONG) dth + (ULONG) dth->dth_Mask);
	    dtn->dtn_Node1.ln_Name = dth->dth_Name;
	    dtn->dtn_Node2.ln_Name = dth->dth_Name;

	    NewList (&dtn->dtn_ToolList);
	    dtn->dtn_Length = msize;
	}
	else
	    SetIoErr (ERROR_NO_FREE_STORE);
    }

    return (dtn);
}

/*****************************************************************************/

LONG ASM ReadFunc (REG (d1) struct MyFileHandle * fh, REG (d2) STRPTR buff, REG (d3) ULONG len, REG (a6) struct Library * dosbase)
{
    LONG retval;

    retval = MIN (len, fh->size - fh->ofs);
    CopyMem (fh->data + fh->ofs, buff, retval);
    fh->ofs += retval;
    return (retval);
}

/*****************************************************************************/

#undef	SysBase

VOID * ASM AllocFunc (REG (d0) ULONG size, REG (d1) ULONG flags, REG (a6) struct Library * SysBase)
{
    return (AllocMem (size, flags));
}

/*****************************************************************************/

VOID ASM FreeFunc (REG (a1) APTR mem, REG (d0) ULONG size, REG (a6) struct Library * SysBase)
{
    FreeMem (mem, size);
}
