/* filerequester.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

#define	PATHNAMESIZE	300

/*****************************************************************************/

BOOL FileRequest (struct GlobalData * gd, ULONG mode, STRPTR title, STRPTR posbut, STRPTR buffer)
{
    char path[PATHNAMESIZE];
    BOOL success;

    strcpy (path, buffer);
    *PathPart (path) = 0;

    if (!gd->gd_FR)
    {
	if (!(gd->gd_FR = allocaslrequest (gd, ASL_FileRequest, TAG_DONE)))
	{
	    return (FALSE);
	}
    }

    success = aslrequesttags (gd, gd->gd_FR,
				  ASLFR_InitialDrawer,	path,
				  ASLFR_InitialFile,	FilePart (buffer),
				  ASLFR_InitialLeftEdge,gd->gd_Prefs.p_FileReq.Left,
				  ASLFR_InitialTopEdge,	gd->gd_Prefs.p_FileReq.Top,
				  ASLFR_InitialWidth,	gd->gd_Prefs.p_FileReq.Width,
				  ASLFR_InitialHeight,  gd->gd_Prefs.p_FileReq.Height,
				  ASLFR_Window,		gd->gd_Window,
				  ASLFR_Screen,		gd->gd_Screen,
				  ASLFR_TitleText,	title,
				  ASLFR_PositiveText,	posbut,
				  ASLFR_SleepWindow,	TRUE,
				  ASLFR_DoSaveMode,	mode,
				  ASLFR_DoPatterns,	TRUE,
				  ASLFR_RejectIcons,	TRUE,
				  TAG_DONE);

    /* Always remember the file requester position */
    gd->gd_Prefs.p_FileReq = *((struct IBox *)(&gd->gd_FR->fr_LeftEdge));

    if (success)
    {
	stccpy  (buffer, gd->gd_FR->rf_Dir,  512);
	AddPart (buffer, gd->gd_FR->rf_File, 512);
    }

    return (success);
}
