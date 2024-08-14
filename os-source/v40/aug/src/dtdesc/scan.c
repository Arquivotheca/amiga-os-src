/* scan.c
 *
 */

#include "dtdesc.h"

VOID SetDataType (struct AppInfo *ai)
{
    STRPTR type;

    switch (ai->ai_Flags & DTF_TYPE_MASK)
    {
	case DTF_BINARY:
	    type = "Binary";
	    break;

	case DTF_ASCII:
	    type = "ASCII";
	    break;

	case DTF_IFF:
	    type = "IFF";
	    break;

	default:
	    type = "";
	    break;
    }

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_TYPE], ai->ai_Window, NULL,
		       GTTX_Text, type, TAG_DONE);
}

VOID ScanFiles (struct AppInfo * ai)
{
    struct List *list = &ai->ai_Files;
    struct FileNode *sfn;
    struct FileNode *dfn;
    struct Node *dnode;
    UBYTE ifftype[5];
    WORD ascii;
    WORD same;
    WORD iff;
    WORD ch;
    WORD i;

    ai->ai_Flags &= ~DTF_TYPE_MASK;
    if (list->lh_TailPred != (struct Node *) list)
    {
	for (i = 0; i < ai->ai_BufSize; i++)
	{
	    ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = (-1);
	    same = 1;

	    sfn = (struct FileNode *) list->lh_Head;
	    if (sfn->fn_Len > i)
	    {
		ch = (WORD) sfn->fn_Buffer[i];
		for (dnode = list->lh_Head; dnode->ln_Succ; dnode = dnode->ln_Succ)
		{
		    dfn = (struct FileNode *) dnode;
		    if ((dfn != sfn) && (dfn->fn_Len > i))
		    {
			if ((WORD) dfn->fn_Buffer[i] == ch)
			{
			    same++;
			}
		    }
		}
	    }

	    if (same == ai->ai_NumFiles)
		ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = ch;
	}

	for (dnode = list->lh_Head, ascii = iff = 0; dnode->ln_Succ; dnode = dnode->ln_Succ)
	{
	    switch (((struct FileNode *)dnode)->fn_Flags & DTF_TYPE_MASK)
	    {
		case DTF_ASCII:
		    ascii++;
		    break;

		case DTF_IFF:
		    iff++;
		    break;
	    }
	}
	if (ascii == ai->ai_NumFiles)
	{
	    ai->ai_Flags |= DTF_ASCII;
	}
	else if (iff == ai->ai_NumFiles)
	{
	    ai->ai_Flags |= DTF_IFF;
	    if (!(ai->ai_Flags & AIF_CHANGED))
	    {
		for (i = 0; i < ai->ai_BufSize; i++)
		{
		    ai->ai_DBuffer[i] = (-1);
		}

		ai->ai_DBuffer[ 0] = ai->ai_SBuffer[ 0];
		ai->ai_DBuffer[ 1] = ai->ai_SBuffer[ 1];
		ai->ai_DBuffer[ 2] = ai->ai_SBuffer[ 2];
		ai->ai_DBuffer[ 3] = ai->ai_SBuffer[ 3];

		ifftype[0] = ai->ai_DBuffer[ 8] = ai->ai_SBuffer[ 8];
		ifftype[1] = ai->ai_DBuffer[ 9] = ai->ai_SBuffer[ 9];
		ifftype[2] = ai->ai_DBuffer[10] = ai->ai_SBuffer[10];
		ifftype[3] = ai->ai_DBuffer[11] = ai->ai_SBuffer[11];
		ifftype[4] = 0;

		strcpy (ai->ai_Name, ifftype);
		GT_SetGadgetAttrs (ai->ai_WinGad[AIG_DATATYPE], ai->ai_Window, NULL,
				   GTST_String, ifftype, TAG_DONE);
	    }
	}
    }

    SetDataType (ai);

    ShowMask (ai);

    SetAppAttrs (ai);
}
