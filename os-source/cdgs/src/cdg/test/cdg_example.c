/*
 *
 * cdg.c - V38 cdg.library example
 *
 * Compiled with SAS C 5.10: LC -L -b1 -cfistq -v -y -icdtv:include cdgtest.c
 *
 *
 */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dosextens.h>

#include <cdtv/cdtv.h>
#include <cdtv/cdgprefs.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/input_protos.h>

#include <pragmas/input_pragmas.h>

#include <cdtv/cdg_cr_pragmas.h>
#include <cdtv/cdg_cr_protos.h>

#include <stdio.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }     /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

struct Library *CDGBase;
struct Library *InputBase;

int main(int argc, char **argv[])
{
struct CDGPrefs *cdp;
int status;
int track;

	track = 0;

	if(argc > 1)
	{
		track = atoi(argv[1]);

	}
	if(argc <= 1 || track == 0)
	{
	    printf("Usage: TRACK NUMBER\nClick B Button to Quit\n");
	    return(RETURN_WARN);
	}

	status = RETURN_FAIL;

	if(CDGBase = OpenLibrary(CDGNAME,38L))
	{
		if(CDGBase->lib_Version >= 38)
		{
			if(cdp = CDGAllocPrefs())
			{
				
				/* initialize CDGPrefs structure */

				cdp->cdgp_DisplayX = 0;
				cdp->cdgp_DisplayY = 0;

				cdp->cdgp_Reserved0 = 0;
				cdp->cdgp_Reserved1 = 0;

				/* Disable +MIDI and Enable Error correction */

				cdp->cdgp_Control = CDGF_NOMIDI;

				MakeSprites( cdp, track );

				CDGFreePrefs(cdp);
			}
		}
		CloseLibrary(CDGBase);
	}
	return(status);
}

#define SPRITEHEIGHT 32

/**
 ** For our test purposes, we won't really be using any sprites, but
 ** we will allocate a single blank image at least, and initialize
 ** the CDGPrefs structure with it
 **/

int MakeSprites( struct CDGPrefs *cdp, int track );
{
UWORD *spritedata;
int status;
register int loop;

	status = RETURN_FAIL;

	if(spritedata = (UWORD *)
		AllocMem(((SPRITEHEIGHT+2)*4)L,MEMF_PUBLIC|MEMF_CLEAR))
	{

		for(loop=0;loop<64;loop++)
		{
			cdp->cdgp_ChannelSprites[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_PAUSESprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_NTrackSprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_PTrackSprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_FFSprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_RWSprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_STOPSprite[loop] = spritedata;
		}

		for(loop=0;loop<4;loop++)
		{
			cdp->cdgp_PLAYSprite[loop] = spritedata;
		}

		/* Initialize sprite colors */

		for(loop=0;loop<8;loop++)
		{
			cdp->cdgp_SpriteColors[loop] = 0x0000;
		}

		/* And sprite height */

		cdp->cdgp_SpriteHeight = 32;

		status = StartDevices(cdp, track);

		FreeMem(spritedate,((SPRITEHEIGHT+2)*4)L);
	}
	return(status);
}

/**
 ** Open devices so we can start disk play
 **/

int StartDevices( struct CDGPrefs *cdp, int track );
{
int status;
struct MsgPort *mp;
struct IOStdReq *ior;

struct MsgPort *imp;
struct IOStdReq *iior;

	/* Open the cdtv.device [use 2.0 exec functions - we know we are
	 * running 2.0 since we opened the cdg.library successfully]
	 */

	status = RETURN_FAIL;

	if(mp=CreateMsgPort())
	{
		if(ior=CreateIORequest(mp,sizeof(struct IOStdReq)))
		{
			if(!(OpenDevice("cdtv.device",0L,ior,0L))
			{
				if(imp=CreateMsgPort())
				{
					if(iior=CreateIORequest(imp,sizeof(struct IOStdReq)))
					{
						if(!(OpenDevice("input.device",0L,iior,0L)))
						{

							InputBase = (struct Library *)iior->io_Device;

							status = StartPlay(cdp,track,ior,mp);

							CloseDevice(iior);
						}
						DeleteIORequest(iior);
					}
					DeleteMsgPort(imp);
				}				
				CloseDevice(ior);
			}
			DeleteIORequest(ior);
		}
		DeleteMsgPort(mp);
	}
	return(status);
}

/**
 ** Start CD+G, and disk play
 **/

int StartPlay( struct CDGPrefs *cdp, int track, struct IOStdReq *ior, struct MsgPort *mp );
{

int status;
BYTE signal;
UWORD buttons;
ULONG sigmask;
BOOL loop;

	status = RETURN_FAIL;

	cdp->cdgp_SigTask = FindTask(0L);

	if(signal = AllocSignal(-1L))
	{

		cdp->cdgp_SigMask = (1L<<signal);

		if(CDGBegin(cdp))
		{

			status = RETURN_OK;

			/* bring CDG+G screen to front */

			CDGFront();

			/* Enable subcode processing */

			CDGPlay(FALSE);

			ior->io_Command = CDTV_PLAYTRACK;
			ior->io_Data = NULL;
			ior->io_Offset = track;
			ior->io_Length = track+1;

			SendIO(ior);

			sigmask = (1L<<signal)|(1L<<mp->mp_SigBit);

			loop = TRUE;


			/***************************************************
			 *
			 * This is the main loop!  For the sake of simplicity
			 * it simply checks for the B button every time
			 * we return from Wait() -- which happens to be
			 * at least 75x per second!!!  More than fast enough
			 * for this example.
			 *
			 ***************************************************/

			while(loop)
			{
				Wait(sigmask);
				if(CheckIO(ior)) loop = FALSE;

				buttons = PeekQualifier();
				if(buttons & IEQUALIFIER_LEFTBUTTON) loop = FALSE;

				/* process pending PACKETS */

				CDGDraw(CDGF_GRAPHICS);
			}

			/*
			 * Indicate disk is no longer playing, and stop the CD
			 */

			CDGStop();

			AbortIO(ior);
			WaitIO(ior);

			ior->io_Command = CDTV_STOPPLAY;
			ior->io_Data = NULL;
			ior->io_Length = 0L;
			ior->io_Offset = 0L;

			DoIO(ior);

			/* close down CD+G */

			CDGEnd();
		}
		FreeSignal(signal);
	}
	return(status);
}

