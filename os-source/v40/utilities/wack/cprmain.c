/* Simple current view copper list disassembly - Scott Evernden */
/* $Id: cprmain.c,v 1.2 91/04/24 20:51:37 peter Exp $	*/

#include <stdio.h>
#include <graphics/gfxbase.h>
#include <graphics/copper.h>
#include <graphics/view.h>

struct GfxBase *GfxBase;
extern struct GfxBase *OpenLibrary();

main()
{
	struct View *v;
	struct cprlist *lis;

	GfxBase = OpenLibrary("graphics.library", 0L);
	if (GfxBase == NULL) {
		fprintf(stderr, "You don't see this!\n");
		exit(21);
	}

	v = GfxBase->ActiView;

	lis = v->LOFCprList;
	printf("LOF:\n");
	dumpCprList(stdout, lis);

	lis = v->SHFCprList;
	printf("SHF:\n");
	dumpCprList(stdout, lis);

	CloseLibrary(GfxBase);
}
