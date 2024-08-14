/*
**	$Id: resources.c,v 37.6 91/03/11 14:23:42 kodiak Exp $
**
**	Fountain/resources.c -- external resource management
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include "fountain.h"

extern struct Library near *SysBase;
extern struct Library near *DOSBase;

struct Library near *AslBase = (struct Library *) 0;
struct Library near *DiskfontBase = (struct Library *) 0;
struct Library near *GadToolsBase = (struct Library *) 0;
struct GfxBase near *GfxBase = (struct Libary *) 0;
struct Library near *IconBase = (struct Library *) 0;
struct Library near *IntuitionBase = (struct Library *) 0;
struct Library near *UtilityBase = (struct Library *) 0;

union StaticPool S;

void OpenLibraries()
{
    if (SysBase->lib_Version < 36)
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "exec.library", 36);
    if (DOSBase->lib_Version < 36)
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "dos.library", 36);
    if (!(GadToolsBase = (struct Library *)
	    OpenLibrary("gadtools.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "gadtools.library", 36);
    if (!(GfxBase = (struct Library *) OpenLibrary("graphics.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "graphics.library", 36);
    if (!(IntuitionBase = (struct Library *)
	    OpenLibrary("intuition.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "intuition.library", 36);
    if (!(UtilityBase = (struct Library *) OpenLibrary("utility.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "utility.library", 36);
    if (!(AslBase = (struct Library *) OpenLibrary("asl.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "asl.library", 36);
    if (!(DiskfontBase = (struct Library *)
	    OpenLibrary("diskfont.library", 37)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "diskfont.library", 37);
    if (!(IconBase = (struct Library *) OpenLibrary("icon.library", 36)))
	EndGame(RETURN_ERROR, ENDGAME_LibraryError, "icon.library", 36);
}

void CloseLibraries()
{
    if (IconBase)
	CloseLibrary(IconBase);
    if (DiskfontBase)
	CloseLibrary(DiskfontBase);
    if (AslBase)
	CloseLibrary(AslBase);
    if (UtilityBase)
	CloseLibrary(UtilityBase);
    if (IntuitionBase)
	CloseLibrary(IntuitionBase);
    if (GfxBase)
	CloseLibrary(GfxBase);
    if (GadToolsBase)
	CloseLibrary(GadToolsBase);
}

FreeList(list)
struct MinList *list;
{
    struct MinNode *entry;

    while (entry = (struct MinNode *) RemHead((struct List *) list))
	free(entry);
}
