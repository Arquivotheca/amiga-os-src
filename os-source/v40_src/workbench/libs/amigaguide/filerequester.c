/* filerequester.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	PATHNAMESIZE	300

/*****************************************************************************/

BOOL FileRequest (struct AmigaGuideLib * ag, struct Client *cl, ULONG mode, STRPTR title, STRPTR posbut, STRPTR buffer)
{
    char path[PATHNAMESIZE];
    BOOL success;

    strcpy (path, buffer);
    *PathPart (path) = 0;

    if (!cl->cl_FR)
    {
	if (!(cl->cl_FR = AllocAslRequest (ASL_FileRequest, NULL)))
	{
	    return (FALSE);
	}
    }

    success = aslrequesttags (ag, cl->cl_FR,
				  ASLFR_InitialDrawer,	path,
				  ASLFR_InitialFile,	FilePart (buffer),
				  ASLFR_InitialLeftEdge,cl->cl_Prefs.p_FileReq.Left,
				  ASLFR_InitialTopEdge,	cl->cl_Prefs.p_FileReq.Top,
				  ASLFR_InitialWidth,	cl->cl_Prefs.p_FileReq.Width,
				  ASLFR_InitialHeight,  cl->cl_Prefs.p_FileReq.Height,
				  ASLFR_Window,		cl->cl_Window,
				  ASLFR_SleepWindow,	TRUE,
				  ASLFR_TitleText,	title,
				  ASLFR_PositiveText,	posbut,
				  ASLFR_DoSaveMode,	mode,
				  ASLFR_DoPatterns,	TRUE,
				  ASLFR_RejectIcons,	TRUE,
				  TAG_DONE);

    /* Always remember the file requester position */
    cl->cl_Prefs.p_FileReq = *((struct IBox *)(&cl->cl_FR->fr_LeftEdge));

    if (success)
    {
	stccpy  (buffer, cl->cl_FR->rf_Dir,  512);
	AddPart (buffer, cl->cl_FR->rf_File, 512);
    }

    return (success);
}
