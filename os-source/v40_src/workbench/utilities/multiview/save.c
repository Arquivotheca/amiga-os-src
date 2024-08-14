/* save.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

ULONG ASM saveasFunc (REG (a0) struct Hook * h, REG (a2) struct Cmd * c, REG (a1) Msg msg)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    struct DiskObject *dob;
    struct DataType *dtn;
    struct dtGeneral dtg;
    ULONG retval = FALSE;
    struct dtWrite dtw;
    BOOL save = FALSE;
    UBYTE buffer[64];
    ULONG mode;
    BPTR fh;

    if (gd->gd_DataObject)
    {
	/* Get the object type */
	getdtattrs (gd, gd->gd_DataObject, DTA_DataType, (ULONG) & dtn, TAG_DONE);
	gd->gd_TempBuffer[0] = 0;

	/* Get the file name */
	if (c->c_Options[0])
	{
	    strcpy (gd->gd_TempBuffer, (STRPTR) c->c_Options[0]);
	    save = TRUE;
	}
	else
	{
	    if (FileRequest (gd, 1, GetString (gd, MV_TITLE_SELECT_FILE_TO_SAVE), GetString (gd, MV_LABEL_SAVE), gd->gd_TempBuffer))
	    {
		save = TRUE;
	    }
	}

	/* Set the mode */
	mode = DTWM_RAW;
	if (c->c_Options[1])
	    mode = DTWM_IFF;

	if (save)
	{
	    /* Create the file */
	    if (fh = Open (gd->gd_TempBuffer, MODE_NEWFILE))
	    {
		/* Fill in the Write method */
		dtw.MethodID       = DTM_WRITE;
		dtw.dtw_GInfo      = NULL;
		dtw.dtw_FileHandle = fh;
		dtw.dtw_Mode       = mode;
		dtw.dtw_AttrList   = NULL;

		/* Perform the Write method */
		if (DoMethodA (gd->gd_DataObject, &dtw))
		{
		    /* Get the an Icon for the file */
		    sprintf (buffer, "ENV:Sys/def_%s", dtn->dtn_Header->dth_BaseName);
		    if ((dob = GetDiskObject (buffer)) == NULL)
			dob = GetDefDiskObject (WBPROJECT);

		    /* If we have an icon, then write it out */
		    if (dob)
		    {
			PutDiskObject (gd->gd_TempBuffer, dob);
			FreeDiskObject (dob);
		    }

		    /* Clear the selected block */
		    dtg.MethodID = DTM_CLEARSELECTED;
		    DoDTMethodA (gd->gd_DataObject, gd->gd_Window, NULL, &dtg);

		    /* Indicate success */
		    retval = TRUE;
		}
		else
		{
		    /* Couldn't write file! */
		    PrintF (gd, 1, MV_ERROR_COULDNT_SAVE_FILE, gd->gd_TempBuffer);
		}

		Close (fh);
	    }
	}
    }

    return (retval);
}
