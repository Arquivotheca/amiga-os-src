/* readdtyp.c
 *
 */

#include "dtdesc.h"

struct DataType *ReadDTYP (struct AppInfo *ai, STRPTR name)
{
    struct DataTypeHeader *dth;
    struct DataType *dtn;
    WORD i;

    if (dtn = ReadDataType (ai, name))
    {
	removeallfunc (ai);

	dth = dtn->dtn_Header;

	ai->ai_Flags &= ~0xFFFF;
	ai->ai_Flags |= (dth->dth_Flags & 0xFFFF);
	for (i = 0; i < FNBUF_SIZE; i++)
	{
	    if (i < dth->dth_MaskLen)
		ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = dth->dth_Mask[i];
	    else
		ai->ai_SBuffer[i] = ai->ai_DBuffer[i] = (-1);
	}
	GT_SetGadgetAttrs (ai->ai_WinGad[AIG_DATATYPE], ai->ai_Window, NULL,
			   GTST_String, dth->dth_Name,
			   TAG_DONE);
	GT_SetGadgetAttrs (ai->ai_WinGad[AIG_BASENAME], ai->ai_Window, NULL,
			   GTST_String, dth->dth_BaseName,
			   TAG_DONE);
	GT_SetGadgetAttrs (ai->ai_WinGad[AIG_PATTERN], ai->ai_Window, NULL,
			   GTST_String, dth->dth_Pattern,
			   TAG_DONE);
	GT_SetGadgetAttrs (ai->ai_WinGad[AIG_CASE], ai->ai_Window, NULL,
			   GTCB_Checked, ((ai->ai_Flags & DTF_CASE) ? TRUE : FALSE),
			   TAG_DONE);
	GT_SetGadgetAttrs (ai->ai_WinGad[AIG_PRIORITY], ai->ai_Window, NULL,
			   GTIN_Number, dth->dth_Priority,
			   TAG_DONE);

	SetGroupID (ai, dth->dth_GroupID);

	SetDataType (ai);
	ShowMask (ai);
	FreeDataTypeNode (ai, dtn);
    }

    return (dtn);
}

void SetGroupID (struct AppInfo *ai, ULONG gid)
{
    struct MenuItem *mi;
    struct Menu *m = ai->ai_Menu->NextMenu;

    ai->ai_GroupID = gid;

    for (mi = m->FirstItem; mi; mi = mi->NextItem)
    {
	mi->Flags &= ~CHECKED;
	if ((ULONG)MENU_USERDATA (mi) == gid)
	{
	    mi->Flags |= CHECKED;
	}
    }
}
