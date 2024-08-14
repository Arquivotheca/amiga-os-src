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
 * filerequester.c
 * Written by David N. Junod
 *
 */

#include "clipview.h"

/*****************************************************************************/

BOOL FileRequest (struct GlobalData * gd, ULONG mode, STRPTR title, STRPTR posbut, STRPTR buffer)
{
    BOOL success;

    if (!gd->gd_FR)
    {
	if (!(gd->gd_FR = AllocAslRequest (ASL_FileRequest, NULL)))
	{
	    return (FALSE);
	}
    }

    success = aslrequesttags (gd, gd->gd_FR,
				  ASLFR_Window,		gd->gd_Window,
				  ASLFR_SleepWindow,	TRUE,
				  ASLFR_TitleText,	title,
				  ASLFR_PositiveText,	posbut,
				  ASLFR_DoSaveMode,	mode,
				  ASLFR_DoPatterns,	TRUE,
				  ASLFR_RejectIcons,	TRUE,
				  TAG_DONE);

    if (success)
    {
	stccpy  (buffer, gd->gd_FR->rf_Dir,  512);
	AddPart (buffer, gd->gd_FR->rf_File, 512);
    }

    return (success);
}
