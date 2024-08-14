/**************

    ILBMSupport.c

***************/

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

#include "iffp/ilbmapp.h"

#include "cdxl/cdxlob.h"
#include "cdxl/debugsoff.h"

/* // Uncomment to get debug output turned on
#define KPRINTF
#include "cdxl/debugson.h"
*/


#define	ILBMINFO struct ILBMInfo

/* ILBM Property chunks to be grabbed
 * List BMHD, CMAP and CAMG first so we can skip them when we write
 * the file back out (they will be written out with separate code)
 */
LONG ilbmprops[] = 
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_CAMG,
    ID_ILBM, ID_CCRT,
    ID_ILBM, ID_AUTH,
    ID_ILBM, ID_Copyright,
    TAG_DONE
};

// ILBM Collection chunks (more than one in file) to be gathered
LONG ilbmcollects[] =
{
    ID_ILBM, ID_CRNG,
    TAG_DONE
};

// ILBM Chunk to stop on
LONG ilbmstops[] =
{
    ID_ILBM, ID_BODY,
    TAG_DONE
};


/* queryilbm
 *
 * Passed an initilized ILBMInfo with a not-in-use IFFHandle,
 *   and a filename,
 *   will open an ILBM, fill in ilbm->camg and ilbm->bmhd,
 *   and close the ILBM.
 *
 * This allows you to determine if the ILBM is a size and
 *   type you want to deal with.
 *
 * Returns 0 for success or an IFFERR (libraries/iffparse.h)
 */

// query just wants these chunks
LONG queryprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CAMG,
    TAG_DONE
};

// scan can stop when a BODY is reached
LONG querystops[] =
{
    ID_ILBM, ID_BODY,
    TAG_DONE
};


LONG
queryilbm( ILBMINFO *ilbm, UBYTE *filename )
{
    LONG error = 0L;
    BitMapHeader *bmhd;

    if ( !ilbm->ParseInfo.iff )
	return(CLIENT_ERROR);

    if ( !(error = openifile((struct ParseInfo *)ilbm, filename, IFFF_READ)) ) {
	D(PRINTF("queryilbm: openifile successful\n"));

	error = parseifile((struct ParseInfo *)ilbm,
	    ID_FORM, ID_ILBM,
	    ilbm->ParseInfo.propchks,
	    ilbm->ParseInfo.collectchks,
	    ilbm->ParseInfo.stopchks);

	D(PRINTF("queryilbm: after parseifile, error = %ld\n",error));

	if ( (!error) || (error == IFFERR_EOC) || (error == IFFERR_EOF) ) {
	    if ( contextis(ilbm->ParseInfo.iff,ID_ILBM,ID_FORM) ) {

		if(bmhd = (BitMapHeader*) findpropdata(ilbm->ParseInfo.iff,ID_ILBM,ID_BMHD)) {
		    *(&ilbm->Bmhd) = *bmhd;
		    ilbm->camg = getcamg(ilbm);
		} else {
		    error = NOFILE;
		}
	    } else {
		message(SI(MSG_ILBM_NOILBM));
		error = NOFILE;
	    }
	}
	closeifile( (struct ParseInfo *)ilbm );
    }

    return(error);

} // queryilbm()


DoQuery( UBYTE * filename, DISP_DEF * disp_def )
{
    ILBMINFO	* ilbm;
    LONG	  error = FALSE;


    if (ilbm = (ILBMINFO *)AllocMem(sizeof(ILBMINFO),MEMF_PUBLIC|MEMF_CLEAR)) {

	ilbm->ParseInfo.propchks	= ilbmprops;
	ilbm->ParseInfo.collectchks	= ilbmcollects;
	ilbm->ParseInfo.stopchks	= ilbmstops;

	if( ilbm->ParseInfo.iff = AllocIFF() ) {
	    if( !(error = queryilbm(ilbm,filename))) {

		D(PRINTF("DoQuery: after query, this ILBM is %ld x %ld x %ld, modeid=$%lx\n",
		ilbm->Bmhd.w, ilbm->Bmhd.h, ilbm->Bmhd.nPlanes, ilbm->camg);)

		disp_def->Width = disp_def->NominalWidth = MAX(ilbm->Bmhd.pageWidth, ilbm->Bmhd.w);
		disp_def->Height = disp_def->NominalHeight = MAX(ilbm->Bmhd.pageHeight,ilbm->Bmhd.h);
		disp_def->Depth = MIN(ilbm->Bmhd.nPlanes,MAXAMDEPTH);

		if ( !(disp_def->Flags & DISP_XLMODEID) )
		    disp_def->ModeID = ilbm->camg;
	    }
	    FreeIFF(ilbm->ParseInfo.iff);
	}

	FreeMem( ilbm, sizeof(ILBMINFO) );
    }

    return( !error );

} // DoQuery()


DoILBM( UBYTE * filename, DISP_DEF * disp_def )
{
    ILBMINFO		* ilbm;
    BitMapHeader	* bmhd;
    LONG		  error = FALSE;

    D(PRINTF("DoILBM 1 bm[0]= 0x%lx, bm[1]= 0x%lx\n",disp_def->bm[0],disp_def->bm[1]);)

    if (ilbm = (ILBMINFO *)AllocMem(sizeof(ILBMINFO),MEMF_CLEAR)) {

	D(PRINTF("DoILBM 2\n");)

	ilbm->vp = disp_def->vp;
	ilbm->ParseInfo.propchks    = ilbmprops;
	ilbm->ParseInfo.collectchks = ilbmcollects;
	ilbm->ParseInfo.stopchks    = ilbmstops;

	if( ilbm->ParseInfo.iff = AllocIFF() ) {

	    D(PRINTF("DoILBM 3\n");)

	    if ( !(error = openifile((struct ParseInfo *)ilbm, filename, IFFF_READ)) ) {

		D(PRINTF("DoILBM 4\n");)

		error = parseifile((struct ParseInfo *)ilbm,
		    ID_FORM, ID_ILBM,
		    ilbm->ParseInfo.propchks,
		    ilbm->ParseInfo.collectchks,
		    ilbm->ParseInfo.stopchks);

		if ( (!error) || (error == IFFERR_EOC) || (error == IFFERR_EOF) ) {

		    D(PRINTF("DoILBM 5\n");)

		    if ( contextis(ilbm->ParseInfo.iff,ID_ILBM,ID_FORM) ) {

			D(PRINTF("DoILBM 6\n");)

			if(bmhd = (BitMapHeader*) findpropdata(ilbm->ParseInfo.iff,ID_ILBM,ID_BMHD)) {
			    D(PRINTF("DoILBM 7\n");)

			    *(&ilbm->Bmhd) = *bmhd;
			    ilbm->camg = getcamg(ilbm);
			    error = loadbody(ilbm->ParseInfo.iff, disp_def->bm[0], &ilbm->Bmhd);
			} else  {
			    error = NOFILE;
			}

			D(PRINTF("DoILBM 8 error= %ld\n",error);)

			if(!error) {
			    D(PRINTF("DoILBM 9\n");)

			    if( !(disp_def->Flags & DISP_XLPALETTE) && !getcolors(ilbm) ) {
				D(PRINTF("DoILBM A\n");)

				setcolors(ilbm,disp_def->vp);
				freecolors(ilbm);
			    }

			    if ( disp_def->bm[0] && disp_def->bm[1] ) {
				BltBitMap( disp_def->bm[0],0,0,disp_def->bm[1],
				 0,0,disp_def->Width,disp_def->Height,0xC0,0xFF,NULL);
			    }

			}

		    } else {
			message(SI(MSG_ILBM_NOILBM));
			error = NOFILE;
		    }
    		}
		    closeifile( (struct ParseInfo *)ilbm );
	    }

	    FreeIFF(ilbm->ParseInfo.iff);
	}

	FreeMem( ilbm, sizeof(ILBMINFO) );
    }

    return( error );

} // DoILBM()

