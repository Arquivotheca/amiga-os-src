/* WBTricks.c : Use the new VideoControl() tags to control features of
 * intuition's sprite on the Workbench screen.
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct IntuitionBase *IntuitionBase = NULL ;
struct GfxBase *GfxBase = NULL ;

/**********************************************************************/
/*                                                                    */
/* void Error (char *String)                                          */
/* Print string and exit                                              */
/*                                                                    */
/**********************************************************************/

void Error (char *String)
{
	void CloseAll (void) ;
	
	printf (String) ;
	CloseAll () ;
	exit(0) ;
}


/**********************************************************************/
/*                                                                    */
/* void Init ()                                                       */
/*                                                                    */
/* Opens all the required libraries                                   */
/* allocates all memory, etc.                                         */
/*                                                                    */
/**********************************************************************/

void Init ()
{
	/* Open the intuition library.... */
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", 37)) == NULL)
		Error ("Could not open the Intuition.library") ;

	/* Open the graphics library.... */
	if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", 39)) == NULL)
		Error ("Could not open the Graphics.library") ;

	return ;
}

/**********************************************************************/
/*                                                                    */
/* void CloseAll ()                                                   */
/*                                                                    */
/* Closes and tidies up everything that was used.                     */
/*                                                                    */
/**********************************************************************/

void CloseAll ()
{
	/* Close everything in the reverse order in which they were opened */

	/* Close the Graphics Library */
	if (GfxBase)
		CloseLibrary ((struct Library *) GfxBase) ;

	/* Close the Intuition Library */
	if (IntuitionBase)
		CloseLibrary ((struct Library *) IntuitionBase) ;

	return ;
}

/***************************************************************************/

void main (int argc, char *argv[])
{
	#define TEMPLATE "B=BORDERSPRITE/S,R=RESOLUTION/N,PF1/N,PF2/N,SPODD/N,SPEVEN/N,BASE1/N,BASE2/N,S=SHOW/S"
	#define OPT_BSPRITE 0
	#define OPT_RESOLUTION 1
	#define OPT_PF1 2
	#define OPT_PF2 3
	#define OPT_SPODD 4
	#define OPT_SPEVEN 5
	#define OPT_BASE1 6
	#define OPT_BASE2 7
	#define OPT_SHOW 8
	#define OPT_COUNT 9

	LONG result[OPT_COUNT];
	LONG *val;
	struct Screen *wb;
	struct RDArgs *rdargs;
	struct TagItem ti[] =
	{
		{VTAG_BORDERSPRITE_GET, TRUE},
		{VTAG_SPRITERESN_GET, SPRITERESN_ECS},
		{VTAG_PF1_TO_SPRITEPRI_GET, 0},
		{VTAG_PF2_TO_SPRITEPRI_GET, 0},
		{VTAG_SPODD_BASE_GET, 0},
		{VTAG_SPEVEN_BASE_GET, 0},
		{VTAG_PF1_BASE_GET, 0},
		{VTAG_PF2_BASE_GET, 0},
		{TAG_DONE, NULL},
	};

	Init () ;

	result[OPT_BSPRITE] = FALSE;
	result[OPT_RESOLUTION] = 0;
	result[OPT_PF1] = 0;
	result[OPT_PF2] = 0;
	result[OPT_SPODD] = 0;
	result[OPT_SPEVEN] = 0;
	result[OPT_BASE1] = 0;
	result[OPT_BASE2] = 0;
	result[OPT_SHOW] = FALSE;

	/* We are going to hijack the Workbench ColorMap for this. Not very
	 * friendly, but it's just to show you what the new AA features can do.
	 */

	if (wb = LockPubScreen("Workbench"))
	{
		/* Get all the current settings */
		if ((VideoControl(wb->ViewPort.ColorMap, ti)) == NULL)
		{
			if (rdargs = ReadArgs(TEMPLATE, result, NULL))
			{
				if (result[OPT_BSPRITE])
				{
					/* Enable the sprite in the border */
					ti[OPT_BSPRITE].ti_Tag = ((ti[OPT_BSPRITE].ti_Tag == VTAG_BORDERSPRITE_SET) ?
					                VTAG_BORDERSPRITE_CLR :
				        	        VTAG_BORDERSPRITE_SET);
				}

				if (val = (LONG *)result[OPT_RESOLUTION])
				{
					/* set the sprite pixel speed */
					switch (*val)
					{
						case 140 : ti[OPT_RESOLUTION].ti_Data = SPRITERESN_140NS; break;
						case 70  : ti[OPT_RESOLUTION].ti_Data = SPRITERESN_70NS; break;
						case 35  : ti[OPT_RESOLUTION].ti_Data = SPRITERESN_35NS; break;
						case 0   : ti[OPT_RESOLUTION].ti_Data = SPRITERESN_ECS; break;
						case -1 :
						default  : ti[OPT_RESOLUTION].ti_Data = SPRITERESN_DEFAULT; break;
					}
				}

				/* Sprite-to-Pfd1 priority. This has always been settable
				 * on the Amiga, but this is the first time a
				 * friendly programmable interface has been given.
				 */
				if (val = (LONG *)result[OPT_PF1])
				{
					ti[OPT_PF1].ti_Data = *val;
				}
				/* Sprite-to-pfd2 priority */
				if (val = (LONG *)result[OPT_PF2])
				{
					ti[OPT_PF2].ti_Data = *val;
				}

				/* AA lets you set the colour-offset for the odd and
				 * even sprite numbers. The default is  colour 16-31.
				 * You can set only multiples of 16 for AA.
				 */
				if (val = (LONG *)result[OPT_SPODD])
				{
					ti[OPT_SPODD].ti_Data = *val;
				}
				if (val = (LONG *)result[OPT_SPEVEN])
				{
					ti[OPT_SPEVEN].ti_Data = *val;
				}

				/* This controls the base colour of the playfields in weird and
				 * wacky ways.
				 */
				if (val = (LONG *)result[OPT_BASE1])
				{
					ti[OPT_BASE1].ti_Data = *val;
				}
				if (val = (LONG *)result[OPT_BASE2])
				{
					ti[OPT_BASE2].ti_Data = *val;
				}

				/* Now go do it */
				VideoControl(wb->ViewPort.ColorMap, ti);
				MakeScreen(wb);
				RethinkDisplay();

				if (result[OPT_SHOW])
				{
					/* Read everything back again. */
					ti[OPT_BSPRITE].ti_Tag = VTAG_BORDERSPRITE_GET;
					ti[OPT_RESOLUTION].ti_Tag = VTAG_SPRITERESN_GET;
					ti[OPT_PF1].ti_Tag = VTAG_PF1_TO_SPRITEPRI_GET;
					ti[OPT_PF2].ti_Tag = VTAG_PF2_TO_SPRITEPRI_GET;
					ti[OPT_SPODD].ti_Tag = VTAG_SPODD_BASE_GET;
					ti[OPT_SPEVEN].ti_Tag = VTAG_SPEVEN_BASE_GET;
					ti[OPT_BASE1].ti_Tag = VTAG_PF1_BASE_GET;
					ti[OPT_BASE2].ti_Tag = VTAG_PF2_BASE_GET;
					if ((VideoControl(wb->ViewPort.ColorMap, ti)) == NULL)
					{
						UBYTE *res;
						printf("BorderSprite = %s\n", ((ti[OPT_BSPRITE].ti_Tag == VTAG_BORDERSPRITE_SET) ? "On" : "Off"));
						switch (ti[OPT_RESOLUTION].ti_Data)
						{
							case SPRITERESN_140NS : res = "140ns"; break;
							case SPRITERESN_70NS : res = "70ns"; break;
							case SPRITERESN_35NS : res = "35ns"; break;
							case SPRITERESN_ECS : res = "ECS compatible"; break;
							default : res = "Default"; break;
						}
						printf("Resolution = %s\n", res);
						printf("Playfield 1 to Sprite Priority = %ld\n", ti[OPT_PF1].ti_Data);
						printf("Playfield 2 to Sprite Priority = %ld\n", ti[OPT_PF2].ti_Data);
						printf("Odd Sprite base = %ld\n", ti[OPT_SPODD].ti_Data);
						printf("Even Sprite base = %ld\n", ti[OPT_SPEVEN].ti_Data);
						printf("Playfield 1 base = %ld\n", ti[OPT_BASE1].ti_Data);
						printf("Playfield 2 base = %ld\n", ti[OPT_BASE2].ti_Data);
					}
				}
			}
			FreeArgs(rdargs);
		}
		UnlockPubScreen(NULL, wb);
	}

	CloseAll () ;
}
