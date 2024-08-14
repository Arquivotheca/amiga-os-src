/********************************
          SimCD32Env.c

  Simulate AmigaCD32 Envirnoment

          W.D.L 931013
*********************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/graphics_pragmas.h>

#include "fakefreeanim_pragmas.h"

#include <stdio.h>
#include <stdlib.h>

#include "SimCD32Env_rev.h"

/*
#define	SIMCD32ENV_VER	VERSTAG " Wayne D. Lutz"

UBYTE	* Version = SIMCD32ENV_VER;
*/

int __saveds StartFakeAnim( VOID );

struct GfxBase		* GfxBase;
struct Library		* FreeAnimBase;
struct Library		* LowLevelBase;
struct Library		* NVBase;


// argument parsing 
#define TEMPLATE    "CDTV.TM/S" VERSTAG " Wayne D. Lutz"
#define OPT_CDTV	0
#define	OPT_COUNT	1


LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


int __saveds StartFakeAnim( VOID );


VOID
closedown( VOID )
{
    if ( rdargs )
	FreeArgs( rdargs );

    if ( NVBase )
	CloseLibrary( NVBase );

    if ( LowLevelBase )
	CloseLibrary( LowLevelBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    exit( RETURN_FAIL );

} // closedown()


VOID
init( BOOL CDTV )
{

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L))) {
	printf("Could NOT open graphics.library V39\n");
	closedown();
    }

    if ( !CDTV )
	SetChipRev( SETCHIPREV_AA );

    if(!(FreeAnimBase = OpenLibrary("freeanim.library",40L))) {
	printf("Could NOT open freeanim.library V40\n");
	closedown();
    }

    if(!(LowLevelBase = OpenLibrary("lowlevel.library",40L))) {
	printf("Could NOT open lowlevel.library V40\n");
	closedown();
    }

    if(!(NVBase = OpenLibrary("nonvolatile.library",40L))) {
	printf("Could NOT open nonvolatile.library V40\n");
	closedown();
    }

} // init()


main()
{
    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }

    init( opts[OPT_CDTV] ? TRUE : FALSE );

    StartFakeAnim();

    FreeArgs( rdargs );

} // main()

