/*
**	$Id: req.c,v 38.2 91/03/24 17:06:31 kodiak Exp $
**
**	Fountain/req.c -- directory requester
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include "fountain.h"

extern struct Library *SysBase;
extern struct Library *AslBase;
extern struct Library *DOSBase;
extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;

extern struct Window *Window;

int DirHook(mask, object)
ULONG mask;
void *object;
{
    int reject;

    if (mask & FILF_DOWILDFUNC) {
	/* reject files */
	reject = ((struct AnchorPath *) object)->ap_Info.fib_DirEntryType < 0;
	return(reject);
    }
}

BOOL AslRequestTags(r, tag1)
APTR r;
Tag tag1;
{
    return(AslRequest(r, (struct TagItem *) &tag1));
}

int DirRequester(targetGadget, hailIndex)
struct Gadget *targetGadget;
int hailIndex;
{
    struct FileRequester *rf;
    int result;

    result = FALSE;			/* assume "Cancel" */
    rf = AllocAslRequest(ASL_FileRequest, NULL);

    if (rf) {
	strcpy(rf->rf_Dir, ((struct StringInfo *) targetGadget->SpecialInfo)->
		Buffer);
	if (AslRequestTags(rf, ASL_Hail, LzS[hailIndex], ASL_Window, Window,
		ASL_FuncFlags, FILF_DOWILDFUNC, ASL_HookFunc, DirHook,
		ASL_ExtFlags1, FIL1F_NOFILES,
		TAG_DONE)) {
	    GT_SetGadgetAttrs(targetGadget, Window, 0,
		    GTST_String, rf->rf_Dir, TAG_DONE);
	    result = TRUE;
	}
	FreeFileRequest(rf);
    }
    return(result);
}

int WarnRequester(formatIndex, args)
int formatIndex;
char *args;
{
    struct EasyStruct ez;

    ez.es_StructSize = sizeof(struct EasyStruct);
    ez.es_Flags = 0;
    ez.es_Title = LzS[TITLE_Warning];
    ez.es_TextFormat = LzS[formatIndex];
    ez.es_GadgetFormat = LzS[GADGET_ContinueCancel];

    return(EasyRequestArgs(Window, &ez, 0, &args));
}


int ErrRequester(formatIndex, args)
int formatIndex;
char *args;
{
    struct EasyStruct ez;

    ez.es_StructSize = sizeof(struct EasyStruct);
    ez.es_Flags = 0;
    ez.es_Title = LzS[TITLE_Error];
    ez.es_TextFormat = LzS[formatIndex];
    ez.es_GadgetFormat = LzS[GADGET_OK];

    return(EasyRequestArgs(Window, &ez, 0, &args));
}
