/* adddatatype.c
 *
 */

#include "adddatatypes.h"

/*******************************************************************************/

#define	DB(x)	;

/*******************************************************************************/

LONG AddDataTypeA (struct GlobalData * gd, struct DataType * dtn, struct TagItem * attrs)
{
    struct Token *t = gd->gd_Token;
    struct DataTypeHeader *odth;
    struct DataTypeHeader *dth;
    struct DataType * odtn;
    struct List *list;
    LONG retval = 0L;
    LONG test;

    if (dtn)
    {
	dth = dtn->dtn_Header;

	/* Get the appropriate list to add the datatype to */
	switch (dth->dth_Flags & DTF_TYPE_MASK)
	{
	    case DTF_BINARY:
		list = &t->t_BinaryList;
		break;

	    case DTF_ASCII:
		list = &t->t_ASCIIList;
		break;

	    case DTF_IFF:
		list = &t->t_IFFList;
		break;

	    case DTF_MISC:
		list = &t->t_AuxList;
		break;

	    default:
		list = NULL;
		break;
	}

	if (list)
	{
	    dtn->dtn_Node1.ln_Name = dtn->dtn_Header->dth_Name;
	    dtn->dtn_Node2.ln_Name = dtn->dtn_Header->dth_Name;

	    if ((Stricmp (dth->dth_Pattern, "#?") == 0) || (strlen (dth->dth_Pattern) == 0))
	    {
		dtn->dtn_SysFlags |= DTSF_MATCHALL;
	    }
	    else
	    {
		dtn->dtn_SysFlags &= ~DTSF_MATCHALL;
		dtn->dtn_TLength = strlen (dth->dth_Pattern) * 2 + 2;

		if (!(dtn->dtn_Token = AllocVec (dtn->dtn_TLength, MEMF_CLEAR)))
		{
		    return (0L);
		}

		test = ParsePatternNoCase (dth->dth_Pattern, dtn->dtn_Token, dtn->dtn_TLength);

		if (test == 1)
		{
		    dtn->dtn_SysFlags |= DTSF_ISWILD;
		}
		else
		{
		    dtn->dtn_SysFlags &= ~DTSF_ISWILD;
		    FreeVec (dtn->dtn_Token);
		    dtn->dtn_TLength = 0;
		    dtn->dtn_Token = NULL;
		    if (test == (-1))
		    {
			return (0L);
		    }
		}
	    }

	    retval = 2L;

	    if ((odtn = (struct DataType *) FindNameI (list, dtn->dtn_Node1.ln_Name)) &&
		(odtn->dtn_UseCnt == 0))
	    {
		BOOL remove = FALSE;

		odth = odtn->dtn_Header;

		if ((odth->dth_Flags & DTF_TYPE_MASK) != (dth->dth_Flags & DTF_TYPE_MASK))
		    remove = TRUE;

		if (Stricmp (odth->dth_Name, dth->dth_Name) != 0)
		    remove = TRUE;

		if (Stricmp (odth->dth_BaseName, dth->dth_BaseName) != 0)
		    remove = TRUE;

		if (Stricmp (odth->dth_Pattern, dth->dth_Pattern) != 0)
		    remove = TRUE;

		if (odth->dth_Flags != dth->dth_Flags)
		    remove = TRUE;

		if (odth->dth_Priority != dth->dth_Priority)
		    remove = TRUE;

		if (odth->dth_MaskLen != dth->dth_MaskLen)
		    remove = TRUE;
		else
		    memcpy (odth->dth_Mask, dth->dth_Mask, dth->dth_MaskLen);

		odth->dth_GroupID = dth->dth_GroupID;
		odth->dth_ID = dth->dth_ID;

		if (remove)
		{
		    DB (kprintf ("major change in %s\n", dtn->dtn_Header->dth_Name));
		    RemoveDataType (gd, odtn);
		    odtn = NULL;
		}
		else
		{
		    DB (kprintf ("minor or no change in %s\n", dtn->dtn_Header->dth_Name));
		}
	    }

	    if (odtn == NULL)
	    {
		if (dtn->dtn_FunctionName)
		{
		    struct MyFileHandle mfh;
		    LONG stacksize = 4096;
		    LONG fa[4];
		    ULONG *ptr;
		    ULONG len;
		    BPTR fh;

		    if (fh = Open (dtn->dtn_FunctionName, MODE_OLDFILE))
		    {
			if (Seek (fh, 0L, OFFSET_END) >= 0L)
			{
			    if ((len = Seek (fh, 0L, OFFSET_BEGINNING)) >= 0)
			    {
				if (dtn->dtn_Executable = AllocVec (len, MEMF_PUBLIC))
				{
				    if (Read (fh, dtn->dtn_Executable, len) == len)
				    {
					dtn->dtn_ExecSize = len;

					mfh.data = (UBYTE *) dtn->dtn_Executable;
					mfh.size = dtn->dtn_ExecSize;
					mfh.ofs = 0;

					fa[0] = (LONG)ReadFunc;
					fa[1] = (LONG)AllocFunc;
					fa[2] = (LONG)FreeFunc;

					if (dtn->dtn_SegList = InternalLoadSeg ((BPTR)&mfh, NULL, fa, ((LONG *) (&stacksize))))
					{
					    ptr = (ULONG *) (dtn->dtn_SegList << 2);
					    dtn->dtn_Function = (LONG (*ASM) (REG (a0) struct DTHookContext *))++ ptr;
					}
				    }
				    else
				    {
					FreeVec (dtn->dtn_Executable);
					dtn->dtn_Executable = NULL;
				    }
				}
			    }
			}
			Close (fh);
		    }
		}

		/* Once the executable has been loaded, we don't need a function name */
		dtn->dtn_FunctionName = NULL;

		t->t_MaxLen = MAX (t->t_MaxLen, dth->dth_MaskLen);

		DB (kprintf ("%s added\n", dtn->dtn_Header->dth_Name));
		EnqueueAlphaMask (gd, list, dtn);
		EnqueueAlpha (gd, &t->t_AlphaList, &dtn->dtn_Node2);
		retval = 1L;
	    }
	}
    }
    return (retval);
}
