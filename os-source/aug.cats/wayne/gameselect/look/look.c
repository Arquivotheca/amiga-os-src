date ; /*  Note that this declares an int available externally
failat 1
stack 20000
LC -M  -j88i -cw -v -iinclude3.0: Look Display ILBMSupport
Blink FROM LIB:c.o Look.o Display.o CopRoutine.o ILBMSupport.o IFFP/parse.o IFFP/unpacker.o IFFP/iffpstrings.o IFFP/ilbmr.o TO LOOK LIB LIB:lc.lib RAM:amiga.lib LIB:debug.lib SMALLDATA NODEBUG
quit ;*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "disp_def.h"
#include "retcodes.h"

#include "look_rev.h"


#include "debugsoff.h"
#define		KPRINTF
#include "debugson.h"


// argument parsing 
#define TEMPLATE    "FROM/A,VIEW/S,LACE/S,NONLACE/S,HIRES/S,LORES/S,SDBL/S,NTSC/S,PAL/S,DEFMON/S,NOPOINTER/S" VERSTAG " Wayne D. Lutz"
#define OPT_FROM	0
#define	OPT_VIEW	1
#define	OPT_LACE	2
#define	OPT_NONLACE	3
#define	OPT_HIRES	4
#define	OPT_LORES	5
#define	OPT_SDBL	6
#define	OPT_NTSC	7
#define	OPT_PAL		8
#define	OPT_DEFMON	9
#define	OPT_NOPOINT	10
#define	OPT_COUNT	11

LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


//int CXBRK(void) { return(0); }		/* Disable SASC CTRL/C handling */
//int chkabort(void) { return(0); }	/* Indeed */

// Error messages.
STATIC UBYTE * XLError[] = {
    "No error",
    "Required filename missing",
    "Error while reading file",
    "Couldn't open file",
    "Not enough memory for operation",
    "Could not open cd/cdtv device",
    "Could not open audio device",
    "Could not open window",
    "Could not open screen",
    "Specified CDXL file is not a standard PAN file",
    "Operation failed"
};


struct IntuitionBase	* IntuitionBase;
struct GfxBase		* GfxBase;
struct Library		* IFFParseBase;
struct Library		* FreeAnimBase;


/*
 * Close every thing down.
 */
STATIC VOID
closedown( VOID )
{
    if ( IntuitionBase )
	CloseLibrary( (struct Library *)IntuitionBase );

    if ( GfxBase )
	CloseLibrary( (struct Library *)GfxBase );

    if ( IFFParseBase )
	CloseLibrary( IFFParseBase );

    if ( FreeAnimBase )
	CloseLibrary( FreeAnimBase );

} // closedown()


/*
 * Open all of the required libraries. If iff is TRUE, then open iffparse.
 */
STATIC
init( BOOL iff )
{
    if ( !(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39L)) ) {
	printf("Could NOT open intuition.library!\n");
	return( RC_FAILED );
    }

    if(!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",39L)) ) {
	printf("Could NOT open graphics.library!\n");
	return( RC_FAILED );
    }

    if ( iff ) {
	D(PRINTF("opening iffparse.library!\n");)
	if(!(IFFParseBase = OpenLibrary("iffparse.library",0L)) ) {
	    printf("Could NOT open iffparse.library!\n");
	    return( RC_FAILED );
	}
    }

    kprintf("Opening freeanim.library\n");
    FreeAnimBase = OpenLibrary("freeanim.library",0L);
    kprintf("After opening freeanim.library returning %ld\n",RC_OK);

    return( RC_OK );

} // init()


VOID
main( LONG argc,char * argv[] )
{
    int     	  ret;
    LONG    	  xlspeed,left,top,vol,loops;
    DISP_DEF	  disp_def ;
    UBYTE	* ilbmfile;


    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    PF("main 1");

    setmem( opts, sizeof (opts) ,0 );

    PF("main 2");

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    PF("main 3");

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	exit( RETURN_ERROR );
    }

    PF("main 4");

    setmem( &disp_def, sizeof (DISP_DEF) ,0 );

    PF("main 5");

    if ( !opts[OPT_VIEW] ) {
	disp_def.Flags |= DISP_SCREEN;
    }

    ilbmfile = (UBYTE *)opts[OPT_FROM];
    disp_def.Flags |= (DISP_INTERLEAVED|DISP_BACKGROUND);

    disp_def.Flags |= DISP_CENTERX;

    D(PRINTF("ilbmfile= '%ls'\n", PFSTR(ilbmfile) );)

    if ( ret = init( (disp_def.Flags & DISP_BACKGROUND) ? TRUE : FALSE ) ) {
	closedown();
	exit( RETURN_FAIL );
    }

    PF("main 6");

    // Query the ILBM to find what sort of display to open.
    if ( ilbmfile && !DoQuery( ilbmfile, &disp_def ) ) {
	ret = RC_CANT_FIND;
	PF("main RC_CANT_FIND");
	closedown();
	return( RC_FAILED );
    }

    PF("main 7");


    CloseLibrary( FreeAnimBase );
    FreeAnimBase = NULL;

    if ( !(ret = ScrWinOpen( &disp_def, ilbmfile, NULL ) ) ) {
	PF("main 8");

	while( (*((BYTE *) 0xbfe001) & 192)==192 );
	    WaitTOF();

	CloseDisplay( &disp_def );
    }

    PF("main 9");

    FreeArgs( rdargs );

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	printf("'%ls'\n",XLError[ret]);
	ret = RETURN_FAIL;
    }

    closedown();

    exit( ret );

} // main()
