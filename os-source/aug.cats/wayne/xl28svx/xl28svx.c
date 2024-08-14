/***********

    Xl28SVX.c

    W.D.L 930812

************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

// Tab size is 8!

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <graphics/gfx.h>


#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()

#include "/XLUtils/Include/retcodes.h"
#include "/XLUtils/Include/cdxl/pan.h"
#include "/XLUtils/Include/iffp/8svx.h"

#include "xl28svx_rev.h"

#include "/XLUtils/Include/cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
#include "/XLUtils/Include/cdxl/debugson.h"


#define TEMPLATE    "FROM/A,TO/A" VERSTAG "Wayne D. Lutz ...In House Testing Version..."
#define OPT_FROM	0
#define OPT_TO		1
#define	OPT_COUNT	2

#define	DEFAULT_XLSPEED		75
#define	DEFAULT_SECTOR_SIZE	2048

#define		INTDIV( a, b )		( ( (a) + ( (b) / 2 ) ) / (b) )


// Error messages.
STATIC UBYTE * XLError[] = {
    "OK",
    "Required filename missing",
    "Error while reading file",
    "Couldn't open file",
    "Not enough memory for operation",
    "Could not open cd/cdtv device",
    "Could not open audio device",
    "Could not open window",
    "Could not open screen",
    "Specified CDXL file is not a FLIK file",
    "Operation failed",
    "Error while writting file",
    "Supplied filename is too long",
    "Can't open file",
    "File is not an IFF ILBM file",
    "File is not an IFF 8SVX file",
};



#define MakeID(a,b,c,d) ((LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d))

/*
#define ID_FORM MakeID('F','O','R','M')
#define ID_VHDR MakeID('V','H','D','R')
#define ID_BODY MakeID('B','O','D','Y')
#define ID_8SVX MakeID('8','S','V','X')
*/
#define ID_CHAN MakeID('C','H','A','N')

/* General IFF Structures */
typedef struct
{
	LONG ckID;
	LONG ckSize;
} Chunk_Header;

typedef struct
{
	LONG ckID;
	LONG ckSize;
	LONG formtype;
} FORM_Header;

typedef struct VOICE_HEADER
{
    ULONG oneshot;
	ULONG repeat;
	ULONG samples_per_cycle;
	UWORD samples_per_sec;
	UBYTE octives;
	UBYTE compression;
	LONG  volume;
} VOICE_HEADER;


STATIC	VOICE_HEADER	VHead;
STATIC	FORM_Header	FHead;
STATIC	Chunk_Header	CHead;

LONG		  opts[OPT_COUNT];
struct RDArgs	* rdargs;


UBYTE *
ErrorString( int rc )
{

    if ( rc >= (sizeof (XLError)/sizeof (UBYTE *)) ) {
	rc = RC_FAILED;
    }

    return( XLError[ rc ] );

} // ErrorString()


WritePad( BPTR fh )
{
    short nil = 0;
    return ( (Write(fh,&nil,1) == 1) ? 1 : 0 );

} // WritePad()



SaveAs8svx( BPTR xlfh, BPTR audiofh, LONG audiosize, LONG videosize, LONG framesize, LONG numframes )
{
    int		  i;
    LONG	  chan = 4,ret = RC_OK;
    LONG	  bodysize = audiosize * numframes,seekpos1=-1,seekpos2=-1;
    BOOL	  Pad = FALSE,FPad = FALSE;
    UBYTE	* audiodata = NULL;


    if ( Seek( xlfh, 0, OFFSET_BEGINNING ) == -1L ) {
	ret = RC_READ_ERROR;
	goto exit;
    }

    if ( !(audiodata = AllocMem( audiosize, 0L )) ) {
	ret = RC_NO_MEM;
	goto exit;
    }
		// Body   VoiceHeader	  Chan
    FHead.ckSize = bodysize + sizeof(VHead) + sizeof(LONG) + (3 * 8) + 4;
    if( FHead.ckSize & 1 ) {
	FHead.ckSize++;
	FPad = TRUE;
    }

    if ( (seekpos1 = Seek( audiofh, 0, OFFSET_CURRENT )) == -1L ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Write( audiofh, &FHead, sizeof(FHead) ) != sizeof(FHead) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    VHead.repeat = bodysize;

    CHead.ckID = ID_VHDR;
    CHead.ckSize = sizeof(VHead);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( audiofh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Write( audiofh, &VHead, sizeof(VHead) ) != sizeof(VHead) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Pad ) {
	if (!WritePad(audiofh) ) {
	    ret = RC_WRITE_ERROR;
	    goto exit;
	}
	Pad = FALSE;
    }

    CHead.ckID = ID_CHAN;
    CHead.ckSize = sizeof(LONG);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( audiofh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Write( audiofh, &chan, sizeof(LONG) ) != sizeof(LONG) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Pad ) {
	if (!WritePad(audiofh) ) {
	    ret = RC_WRITE_ERROR;
	    goto exit;
	}
	Pad = FALSE;
    }

    CHead.ckID = ID_BODY;
    CHead.ckSize = bodysize;
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if ( (seekpos2 = Seek( audiofh, 0, OFFSET_CURRENT )) == -1L ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    if( Write( audiofh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto exit;
    }

    bodysize = 0;

    for ( i = 0; i < numframes; i++ ) {

	// After the first loop, I can seek a full framesize-audiosize to
	// get to the audio space. This way if FLIK_MULTI_PAL, framesize will
	// acount for the colortable.
	if ( i ) {
	    if ( Seek( xlfh, framesize-audiosize, OFFSET_CURRENT ) == -1 ) {
		printf("Error seeking in xlfile framenum= %ld\n",i);
		ret = RC_READ_ERROR;
		goto exit;
	    }
	} else if ( Seek( xlfh, sizeof (PAN) + videosize, OFFSET_CURRENT ) == -1 ) {
	    printf("Error seeking in xlfile\n");
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	if ( Read( xlfh, audiodata, audiosize ) != audiosize ) {
	    printf("Error writing audio\n");
	    ret = RC_WRITE_ERROR;
	    goto exit;
	}

	if( Write( audiofh, audiodata, audiosize ) != audiosize ) {
	    ret = RC_WRITE_ERROR;
	    goto exit;
	}

	bodysize += audiosize;

    }

    if ( (seekpos2 = Seek( audiofh, seekpos2, OFFSET_BEGINNING)) != -1L ) {

	CHead.ckID = ID_BODY;
	CHead.ckSize = bodysize;
	if( CHead.ckSize & 1 ) {
	    CHead.ckSize++;
	    Pad = TRUE;
	} else {
	    Pad = FALSE;
	}

	kprintf("ckSize= %ld, bodysize= %ld, pad= %ld\n",
	    CHead.ckSize,bodysize,Pad);


	// Rewrite CHead accounting for actual bodysize written
	if( Write( audiofh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	    ret = RC_WRITE_ERROR;
	    goto exit;
	}


	// Rewrite FHead accounting for actual bodysize written
	if ( Seek( audiofh, seekpos1, OFFSET_BEGINNING) != -1L ) {
			// Body   VoiceHeader	  Chan
	    FHead.ckSize = bodysize + sizeof(VHead) + sizeof(LONG) + (3 * 8) + 4;
	    if( FHead.ckSize & 1 ) {
		FHead.ckSize++;
		FPad = TRUE;
	    } else {
		FPad = FALSE;
	    }

	    if( Write( audiofh, &FHead, sizeof(FHead) ) != sizeof(FHead) ) {
		ret = RC_WRITE_ERROR;
		goto exit;
	    }
	} else {
	    seekpos2 = -1;
	}
    }

    if ( (seekpos2 !=-1) && (Seek( audiofh, seekpos2, OFFSET_BEGINNING)) != -1L ) {
	if( Pad ) {
	    if (!WritePad(audiofh) ) {
		ret = RC_WRITE_ERROR;
		goto exit;
	    }
	    Pad = FALSE;
	}

	if( FPad ) {
	    if (!WritePad(audiofh) ) {
		ret = RC_WRITE_ERROR;
		goto exit;
	    }
	    Pad = FALSE;
	}

    } else {
	ret = RC_WRITE_ERROR;
    }

exit:

    if ( audiodata )
	FreeMem( audiodata, audiosize );

    return( ret );

} /* SaveAs8svx() */


VOID
main( LONG argc,char * argv[] )
{
    int			  ret;
    int			  numframes,framesize;
    LONG		  xlLock,audiosize;
    BPTR	  	  xlfh,audiofh;
    ULONG		  size,xlspeed;
    PAN			  pan;
    UBYTE		  fibbuf[ ( sizeof( struct FileInfoBlock ) + 3 ) ];
    UBYTE		* xlname,* audioname;
    struct FileInfoBlock *fib;

    // workbench
    if ( argc == 0 )
	exit( RETURN_ERROR );

    xlLock = NULL;
    xlfh = audiofh = NULL;

    setmem( opts, sizeof (opts) ,0 );

    rdargs = ReadArgs(TEMPLATE, opts, NULL);

    if ( !rdargs ) {
	PrintFault(IoErr(), NULL);
	ret = RC_FAILED;
	goto exit;
    }


    xlname = (UBYTE *)opts[OPT_FROM];
    audioname = (UBYTE *)opts[OPT_TO];


    if ( !(xlLock = Lock( xlname, ACCESS_READ ) ) ) {
	ret = RC_CANT_FIND;
	printf("Lock on '%ls' failed\n",PFSTR(xlname));
	goto exit;
    }

    // Get size of file.
    // (Force longword alignment of fib pointer.)
    fib = (struct FileInfoBlock *) ( (LONG) ( fibbuf + 3 ) & ~3L );
    Examine( xlLock, fib );
    size = fib->fib_Size;

    if ( !(xlfh = Open( xlname, MODE_OLDFILE )) ) {
	ret = RC_CANT_FIND;
	printf("Open on '%ls' failed\n",PFSTR(xlname));
	goto exit;
    }

    // Get the first PAN
    if ( Read( xlfh, &pan, PAN_SIZE ) != PAN_SIZE ) {
	ret = RC_READ_ERROR;
	goto exit;
    }

    if ( !(audiosize = pan.AudioSize) ) {
	printf("This CDXL has no audio!\n");
	ret = RC_OK;
	goto exit;
    }

    if ( !(audiofh = Open( audioname, MODE_NEWFILE )) ) {
	ret = RC_CANT_FIND;
	printf("Open on '%ls' failed\n",PFSTR(audioname));
	goto exit;
    }


    framesize = pan.Size;
    numframes = ( size / framesize );
    xlspeed = (pan.Reserved+1) * DEFAULT_XLSPEED;

    setmem( &VHead, sizeof(VHead), 0 );
    setmem( &FHead, sizeof(FHead), 0 );
    setmem( &CHead, sizeof(CHead), 0 );

    VHead.samples_per_sec = INTDIV( (( xlspeed * DEFAULT_SECTOR_SIZE) * audiosize ), framesize );

    VHead.octives = 1;
    VHead.volume = 65536;

    FHead.ckID = ID_FORM;
    FHead.formtype = ID_8SVX;

    ret = SaveAs8svx( xlfh, audiofh, audiosize, IMAGE_SIZE( &pan ), framesize, numframes );

exit:

    if ( xlfh ) {
	Close( xlfh );
	xlfh = NULL;
    }

    if ( xlLock ) {
	UnLock( xlLock );
	xlLock = NULL;
    }

    if ( audiofh ) {
	Close( audiofh );
	audiofh = NULL;
    }

    if ( rdargs ) {
	D(PRINTF("main() G\n");)
	FreeArgs( rdargs );
    }

    D(PRINTF("main() H\n");)

    if ( !ret ) {
	ret = RETURN_OK;
    } else {
	printf("'%ls'\n", ErrorString(ret) );
	ret = RETURN_FAIL;
    }

    D(PRINTF("main() END\n");)

    exit( ret );


} // main()
