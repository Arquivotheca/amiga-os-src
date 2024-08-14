/* filerequester.c
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
