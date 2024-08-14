/* funcs.c
 *
 */

#include "dtdesc.h"

/*****************************************************************************/

void FreeDataTypeNode (struct AppInfo *ai, struct DataType * dtn)
{
    if (dtn && (dtn->dtn_Node1.ln_Succ == NULL) && (dtn->dtn_Node1.ln_Pred == NULL))
    {
	if (dtn->dtn_SegList)
	    InternalUnLoadSeg (dtn->dtn_SegList, FreeFunc);
	if (dtn->dtn_Executable)
	    FreeVec (dtn->dtn_Executable);
	FreeVec (dtn);
    }
}

/*****************************************************************************/

VOID RemoveFunc (struct AppInfo *ai)
{
    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FILELIST], ai->ai_Window, NULL, GTLV_Labels, ~0, TAG_DONE);
    removefunc (ai);
    ScanFiles (ai);
}

/*****************************************************************************/

VOID RemoveAllFunc (struct AppInfo *ai)
{
    WORD stx, sty;

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FILELIST], ai->ai_Window, NULL, GTLV_Labels, ~0, TAG_DONE);
    removeallfunc (ai);
    SetAppAttrs (ai);

    stx = ai->ai_ViewBox.Left + 4;
    sty = ai->ai_ViewBox.Top + 2;

    SetAPen (ai->ai_Window->RPort, 0);
    RectFill (ai->ai_Window->RPort, stx, sty, stx + ai->ai_ViewBox.Width - 8 - 1, sty + ai->ai_ViewBox.Height - 4 - 1);
}

/*****************************************************************************/

VOID NewFunc (struct AppInfo *ai)
{
    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_DATATYPE], ai->ai_Window, NULL,
		       GTST_String, "",
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_BASENAME], ai->ai_Window, NULL,
		       GTST_String, "",
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_PATTERN], ai->ai_Window, NULL,
		       GTST_String, "#?",
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FUNCTION], ai->ai_Window, NULL,
		       GTST_String, "",
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_CASE], ai->ai_Window, NULL,
		       GTCB_Checked, FALSE,
		       TAG_DONE);

    GT_SetGadgetAttrs (ai->ai_WinGad[AIG_PRIORITY], ai->ai_Window, NULL,
		       GTIN_Number, 0,
		       TAG_DONE);

    SetGroupID (ai, GID_SYSTEM);

    RemoveAllFunc (ai);
}

/*****************************************************************************/

LONG ProcessCommand (struct AppInfo * ai, ULONG id)
{
    LONG retval = 0L;

    switch (id)
    {
	case MMC_SYSTEM_G:
	case MMC_TEXT_G:
	case MMC_DOCUMENT_G:
	case MMC_SOUND_G:
	case MMC_INSTRUMENT_G:
	case MMC_MUSIC_G:
	case MMC_PICTURE_G:
	case MMC_ANIMATION_G:
	case MMC_MOVIE_G:
	    ai->ai_GroupID = id;
	    break;

	case MMC_NEW:
	    NewFunc (ai);
	    break;

	case MMC_OPEN:
	    if (FileRequest (ai, FR_OPEN, "Open File Type", ai->ai_Dir, ai->ai_Name))
	    {
		if (ReadDTYP (ai, ai->ai_Buffer))
		{
		    strcpy (ai->ai_Dir, ai->ai_FR->rf_Dir);
		    strcpy (ai->ai_Name, ai->ai_FR->rf_File);
		}
	    }
	    break;

	case MMC_SAVE:
	case MMC_SAVEAS:
	    strcpy (ai->ai_Name, ((struct StringInfo *) ai->ai_WinGad[AIG_DATATYPE]->SpecialInfo)->Buffer);
	    if (FileRequest (ai, FR_SAVE, "Save File Type", ai->ai_Dir, ai->ai_Name))
	    {
		if (WriteDTYP (ai, ai->ai_Buffer))
		{
		    strcpy (ai->ai_Dir, ai->ai_FR->rf_Dir);
		    strcpy (ai->ai_Name, ai->ai_FR->rf_File);
		}
	    }
	    break;

	/* Project */
	case MMC_QUIT:
	    ai->ai_Done = TRUE;
	    break;

	case MMC_GRAPHIC:
	    if (ai->ai_Flags & (AIF_TEXT | AIF_HEX))
	    {
		ai->ai_Flags &= ~(AIF_TEXT | AIF_HEX);
		ShowMask (ai);
	    }
	    break;

	case MMC_TEXT:
	    if (!(ai->ai_Flags & AIF_TEXT))
	    {
		ai->ai_Flags &= ~(AIF_TEXT | AIF_HEX);
		ai->ai_Flags |= AIF_TEXT;
		ShowMask (ai);
	    }
	    break;

	case MMC_HEX:
	    if (!(ai->ai_Flags & AIF_HEX))
	    {
		ai->ai_Flags &= ~(AIF_TEXT | AIF_HEX);
		ai->ai_Flags |= AIF_HEX;
		ShowMask (ai);
	    }
	    break;

	case MMC_CASE:
	    if (ai->ai_WinGad[AIG_CASE]->Flags & GFLG_SELECTED)
		ai->ai_Flags |= DTF_CASE;
	    else
		ai->ai_Flags &= ~DTF_CASE;
	    break;

	case MMC_REMOVE:
	    RemoveFunc (ai);
	    break;

	case MMC_REMOVEALL:
	    RemoveAllFunc (ai);
	    break;

	case MMC_BASENAME:
	case MMC_DATATYPE:
	    break;

	case MMC_SAMPLE:
	    FindSelected (ai, NULL, ai->ai_IMsg->Code);
	    SetAppAttrs (ai);
	    break;

	case MMC_LOAD:
	    if (FileRequest (ai, FR_MULTIOPEN, "Load Samples", "", ""))
	    {
		AddWBArgs (ai, ai->ai_FR->rf_ArgList, ai->ai_FR->rf_NumArgs);
	    }
	    break;

	case MMC_LOADFUNC:
	    if (FileRequest (ai, FR_OPEN, "Load Function", "", ""))
	    {
		strcpy (ai->ai_Buffer, ai->ai_FR->rf_Dir);
		AddPart (ai->ai_Buffer, ai->ai_FR->rf_File, 256);
		GT_SetGadgetAttrs (ai->ai_WinGad[AIG_FUNCTION], ai->ai_Window, NULL,
				   GTST_String, ai->ai_Buffer,
				   TAG_DONE);
	    }
	    break;

	case MMC_ADDDATATYPE:
	    AddDTYP (ai);
	    break;
    }

    return (retval);
}
