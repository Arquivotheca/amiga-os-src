/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * save.c
 * Written by David N. Junod
 *
 */

#include "clipview.h"

/*****************************************************************************/

VOID SaveObject (struct GlobalData *gd, ULONG mode)
{
    struct DiskObject *dob;
    struct DataType *dtn;
    struct dtWrite dtw;
    UBYTE buffer[64];
    BPTR fh;

    /* Get the object type */
    getdtattrs (gd, gd->gd_DisplayObject, DTA_DataType, (ULONG)&dtn, TAG_DONE);

    if (FileRequest (gd, 1, GetString (gd, TITLE_SELECT_FILE_TO_SAVE_TO), GetString (gd, LABEL_SAVE), gd->gd_TempBuffer))
    {
	if (fh = Open (gd->gd_TempBuffer, MODE_NEWFILE))
	{
	    dtw.MethodID       = DTM_WRITE;
	    dtw.dtw_GInfo      = NULL;
	    dtw.dtw_FileHandle = fh;
	    dtw.dtw_Mode       = mode;
	    dtw.dtw_AttrList   = NULL;

	    if (DoMethodA (gd->gd_DisplayObject, &dtw))
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
	    }
	    else
	    {
		/* Couldn't write file! */
		PrintF (gd, 0, ERROR_COULDNT_SAVE_FILE, gd->gd_TempBuffer);
	    }

	    Close (fh);
	}
    }
}