
/*\
 *  Bugreport about ScrollRaster by Angela Schmidt
 *  EMail: Angela@rz.uni-karlsruhe.de
 *
 *  This has been compiled with DICE.
 *    dcc -mRR -proto -2.0 scrollraster.c -o scrollraster
\*/

#include <exec/types.h>
#include <intuition/intuitionbase.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>

#include <stdio.h>
#include <stdlib.h>


struct IntuitionBase *IntuitionBase;
struct Library *GfxBase;


#define R_MINX		100L							/* Rectangle min x */
#define R_MINY		30L							/* Rectangle min y */
#define R_MAXX		400L							/* Rectangle max x */
#define R_MAXY		100L							/* Rectangle max y */
#define R_DELTAX	-((R_MAXX-R_MINX)+100L)	/* This will cause trouble - positive values seem to be ok in V37 and V40. In V34 positive values were buggy, too! */
#define R_DELTAY	-((R_MAXY-R_MINY)+50L)	/* This will cause trouble, too */


int main (int argc, char **argv)
{
	struct RPort *rp;
	ULONG IVal;

	printf ("Please do not close this window until we have terminated!!!\n");
	printf ("Otherwise you will GURU :-)\n\n");
	printf ("This program will draw a rectangle into the active window.\n");
	printf ("After this, it will scroll an area within this rectangle.\n");
	printf ("There should be left about 10 pixels at each border of the\n");
	printf ("rectangle, since our scrollarea is smaller than the rectangle.\n");
	printf ("But look what happens... :-(\n\n");

	if (IntuitionBase=(struct Library *) OpenLibrary ("intuition.library", 33L)) {
		if (GfxBase=OpenLibrary ("graphics.library", 33L)) {

			/* Here we start a very, very bad hack!!! */

			IVal=LockIBase (0);
			rp=IntuitionBase->ActiveWindow ? IntuitionBase->ActiveWindow->RPort : 0;
			UnlockIBase (IVal);
			if (rp) {		/* Now we hope nobody will close this testwindow... :-) */

				SetAPen (rp, 3L);		/* Set color 3 for RectFill */
				SetBPen (rp, 2L);		/* Set color 2 for ScrollRaster */

				/*\
				 *  Draw a rectangle. After ScrollRaster, there should be
				 *  only pixels within _this_ box affected. There SHOULD stay
				 *  a border of about 10 pixels width and height!
				\*/
				RectFill (rp, R_MINX-10, R_MINY-10, R_MAXX+10, R_MAXY+10);

				/* Gives you some time to have a look at the output */
				Delay (100L);

				/* Now comes the buggy ScrollRaster routine */
				ScrollRaster (rp, R_DELTAX, R_DELTAY, R_MINX, R_MINY, R_MAXX, R_MAXY);

				/* Gives you some time to have a look at the output */
				Delay (100L);

				/*\
				 *  BTW there are missing some parameters in
				 *  AutoDocs:Libraries/graphics.doc/ScrollRaster/EXAMPLE
				\*/
			}
			CloseLibrary (GfxBase);
		}
		CloseLibrary ((struct Library *) IntuitionBase);
	}
	printf ("Sorry that I destroyed one of your windows... :-)\n\n");
	printf ("If you now remove the sign ('-') of R_DELTAX or R_DELTAY\n");
	printf ("and run at least V37, you can see how the output looks if it\n");
	printf ("is correctly!\n");

	exit (0);
}
