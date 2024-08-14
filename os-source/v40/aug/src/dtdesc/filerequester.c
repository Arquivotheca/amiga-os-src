/* filerequester.c
 *
 */

#include "dtdesc.h"

/*****************************************************************************/

APTR allocaslrequest (struct AppInfo *ai, ULONG type, ULONG tag1, ...)
{
    return (AllocAslRequest(type,(struct TagItem *) &tag1));
}

/*****************************************************************************/

BOOL FileRequest (struct AppInfo * ai, BOOL mode, STRPTR title, STRPTR dir, STRPTR name)
{
    BOOL success;
    ULONG flags;

    if (!ai->ai_FR)
    {
	ai->ai_FRMemory.Width = 320;
	ai->ai_FRMemory.Height = 200;

	if (!(ai->ai_FR = allocaslrequest (ai, ASL_FileRequest,
					   ASLFR_InitialDrawer,		dir,
					   ASLFR_InitialFile,		name,
					   ASLFR_InitialLeftEdge,	ai->ai_Window->LeftEdge + 12,
					   ASLFR_InitialTopEdge,	ai->ai_Window->TopEdge + 12,
					   ASLFR_Window,		ai->ai_Window,
					   ASLFR_SleepWindow,		TRUE,
					   ASLFR_RejectIcons,		TRUE,
					   TAG_DONE)))
	{
	    return (FALSE);
	}
    }

    switch (mode)
    {
	case FR_SAVE:
	    flags = FILF_DOMSGFUNC | FILF_SAVE;
	    break;

	case FR_OPEN:
	    flags = FILF_DOMSGFUNC;
	    break;

	case FR_MULTIOPEN:
	    flags = FILF_DOMSGFUNC | FILF_MULTISELECT;
	    break;
    }

    success = AslRequestTags (ai->ai_FR,
				  ASL_Dir,	dir,
				  ASL_File,	name,
				  ASL_LeftEdge,	ai->ai_FRMemory.Left,
				  ASL_TopEdge,	ai->ai_FRMemory.Top,
				  ASL_Width,	ai->ai_FRMemory.Width,
				  ASL_Height,	ai->ai_FRMemory.Height,
				  ASL_FuncFlags,flags,
				  ASL_Hail,	title,
				  ASL_Window,	ai->ai_Window,
				  TAG_DONE);
    if (success)
    {
	ai->ai_FRMemory.Left   = ai->ai_FR->rf_LeftEdge;
	ai->ai_FRMemory.Top    = ai->ai_FR->rf_TopEdge;
	ai->ai_FRMemory.Width  = ai->ai_FR->rf_Width;
	ai->ai_FRMemory.Height = ai->ai_FR->rf_Height;

	stccpy (ai->ai_Buffer,  ai->ai_FR->rf_Dir,  512);
	AddPart (ai->ai_Buffer, ai->ai_FR->rf_File, 512);

	return (TRUE);
    }

    return (FALSE);
}
