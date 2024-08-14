#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <dos/dos.h>

#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

int foo();

int
foo()
{
	struct Library *SysBase = *((struct Library **) 4);
	struct Library *IntuitionBase;
	struct Library *GfxBase;
	struct Screen *s;

	if (!(IntuitionBase = OpenLibrary("intuition.library",0)))
		return 20;

	if (!(GfxBase = OpenLibrary("graphics.library",0)))
		return 30;

	s = OpenScreenTags(NULL,TAG_DONE);

	SetAPen(&(s->RastPort),1);
	SetDrMd(&(s->RastPort),JAM1);
	Move(&(s->RastPort),40,40);
	Text(&(s->RastPort),"Hi There",8);

	Wait(SIGBREAKF_CTRL_C);
	return 0;
}
