
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#include <stdio.h>

struct GfxBase *GfxBase = NULL;

int CXBRK (void) { return (0) ; }	/* disable ctrl-c */

void main (int argc, char *argv[])
{
	#define TEMPLATE "C=COMMENTS/S,D=DATA/S,N=NOCOLOURS/S,I=NOCOPINIT/S"
	#define OPT_COMMENT 0
	#define OPT_DATA 1
	#define OPT_NOCOLOURS 2
	#define OPT_NOCOPINIT 3
	#define OPT_COUNT 4

	struct RDArgs *rdargs;
	LONG result[OPT_COUNT];

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L); 

	if (GfxBase == NULL)
	{
		fprintf(stderr,"couldn't open graphics.library\n");
		exit(1);
	}
	result[OPT_COMMENT] = FALSE;
	result[OPT_DATA] = FALSE;
	result[OPT_NOCOLOURS] = FALSE;
	result[OPT_NOCOPINIT] = FALSE;
	if (rdargs = ReadArgs(TEMPLATE, result, NULL))
		FreeArgs(rdargs);

	printf("\nCurrently active LOF copper list:\n");
	copdis(GfxBase->ActiView->LOFCprList->start, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS]);

	if (GfxBase->ActiView->Modes & LACE)
	{
		printf("\nCurrently active SHF copper list:\n");
		copdis(GfxBase->ActiView->SHFCprList->start, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS]);
	}

	if (result[OPT_NOCOPINIT] == FALSE)
	{
		printf("\nGfxBase's copinit list:\n");
		copdis(GfxBase->copinit, result[OPT_COMMENT], result[OPT_DATA], result[OPT_NOCOLOURS]);
	}

}
