/* readdatatype.c
 *
 */

#include "dtdesc.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct DataType *ReadDataType (struct AppInfo *ai, STRPTR name)
{
    struct DataType *dtn = NULL;
    struct IFFHandle *iff;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = Open (name, MODE_OLDFILE))
	{
	    InitIFFasDOS (iff);
	    dtn = GetDataType (ai, iff);
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
struct DataType *GetDataType (struct AppInfo *ai, struct IFFHandle * iff)
{
    struct DataType *dtn = NULL;

    if (OpenIFF (iff, IFFF_READ) == 0L)
    {
	PropChunks (iff, dtyp_props, 2);
	CollectionChunks (iff, dtyp_collections, 1);
	if (StopOnExit (iff, ID_DTYP, ID_FORM) == 0)
	    if (ParseIFF (iff, IFFPARSE_SCAN) == IFFERR_EOC)
		dtn = GetDTYPForm (ai, iff);
	CloseIFF (iff);
    }

    return (dtn);
}

/*****************************************************************************/

struct DataType *GetDTYPForm (struct AppInfo *ai, struct IFFHandle * iff)
{
    struct StoredProperty *sp;
    struct DataType *dtn;
    struct MyFileHandle fh;
    LONG stacksize = 4096;
    LONG fa[4];
    ULONG *ptr;

    if (dtn = GetDTHD (ai, iff))
    {
	DB (kprintf ("%s\n", dtn->dtn_Header->dth_Name));
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
		    dtn->dtn_Function = (LONG (*ASM) (struct DTHookContext *))++ ptr;
		}
	    }
	    DB (kprintf (" code=%ld : 0x%lx\n", sp->sp_Size, dtn->dtn_Function));
	}
    }

    return (dtn);
}

/*****************************************************************************/

struct DataType *GetDTHD (struct AppInfo *ai, struct IFFHandle * iff)
{
    struct DataTypeHeader *dth;
    struct StoredProperty *sp;
    struct DataType *dtn;
    ULONG msize;

    if (sp = FindProp (iff, ID_DTYP, ID_DTHD))
    {
	msize = DTNSIZE + sp->sp_Size;
	if (dtn = AllocVec (msize, MEMF_CLEAR))
	{
	    dtn->dtn_Header = MEMORY_FOLLOWING (dtn);
	    dth = dtn->dtn_Header;

	    CopyMem (sp->sp_Data, dth, sp->sp_Size);

	    DB (kprintf (" dth  = 0x%08lx\n", dth));
	    DB (kprintf (" name = 0x%08lx : 0x%08lx\n", dth->dth_Name, (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Name)));
	    DB (kprintf (" base = 0x%08lx : 0x%08lx\n", dth->dth_BaseName, (STRPTR) ((ULONG) dth + (ULONG) dth->dth_BaseName)));
	    DB (kprintf (" ptrn = 0x%08lx : 0x%08lx\n", dth->dth_Pattern, (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Pattern)));
	    DB (kprintf (" mask = 0x%08lx : %ld\n", dth->dth_Mask, dth->dth_MaskLen));
	    DB (kprintf ("size=%ld\n", sp->sp_Size));

	    dth->dth_Name     = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Name);
	    dth->dth_BaseName = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_BaseName);
	    dth->dth_Pattern  = (STRPTR) ((ULONG) dth + (ULONG) dth->dth_Pattern);
	    dth->dth_Mask     = (WORD *) ((ULONG) dth + (ULONG) dth->dth_Mask);
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

LONG ReadFunc (struct MyFileHandle * fh, STRPTR buff, ULONG len, struct Library * dosbase)
{
    LONG retval;

    retval = MIN (len, fh->size - fh->ofs);
    CopyMem (fh->data + fh->ofs, buff, retval);
    fh->ofs += retval;
    return (retval);
}

/*****************************************************************************/

#undef	SysBase

VOID * AllocFunc (ULONG size, ULONG flags, struct Library * SysBase)
{
    return (AllocMem (size, flags));
}

/*****************************************************************************/

VOID FreeFunc (APTR mem, ULONG size, struct Library * SysBase)
{
    FreeMem (mem, size);
}
