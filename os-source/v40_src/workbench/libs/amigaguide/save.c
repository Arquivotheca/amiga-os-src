/* save.c
 * amigaguide.library
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

VOID SaveObject (struct AmigaGuideLib *ag, struct Client *cl, ULONG mode)
{
    struct DataType *dtn;
    struct dtWrite dtw;
    BPTR fh;

    /* Get the object type */
    GetDTAttrs (cl->cl_DataObject, DTA_DataType, (ULONG)&dtn, TAG_DONE);

    cl->cl_TempBuffer[0] = 0;

    if (FileRequest (ag, cl, 1, GetAmigaGuideStringLVO (ag, TITLE_SELECT_FILE_TO_SAVE_TO), GetAmigaGuideStringLVO (ag, LABEL_SAVE), cl->cl_TempBuffer))
    {
	if (fh = Open (cl->cl_TempBuffer, MODE_NEWFILE))
	{
	    dtw.MethodID       = DTM_WRITE;
	    dtw.dtw_GInfo      = NULL;
	    dtw.dtw_FileHandle = fh;
	    dtw.dtw_Mode       = mode;
	    dtw.dtw_AttrList   = NULL;

	    if (DoMethodA (cl->cl_DataObject, (Msg) &dtw))
	    {
#ifdef IconBase
		struct DiskObject *dob;
		UBYTE buffer[64];

		/* Get the an Icon for the file */
		sprintf (buffer, "ENV:Sys/def_%s", dtn->dtn_Header->dth_BaseName);
		if ((dob = GetDiskObject (buffer)) == NULL)
		     dob = GetDefDiskObject (WBPROJECT);

		/* If we have an icon, then write it out */
		if (dob)
		{
		    PutDiskObject (cl->cl_TempBuffer, dob);
		    FreeDiskObject (dob);
		}
#endif
	    }
	    else
	    {
		/* Couldn't write file! */
		PrintF (ag, cl, 1, ERR_COULDNT_SAVE_FILE, cl->cl_TempBuffer);
	    }

	    Close (fh);
	}
    }
}
