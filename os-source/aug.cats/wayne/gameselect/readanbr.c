/*************
  ReadAnbr.c

 W.D.L 930818

**************/


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

#include <graphics/modeid.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>


#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>

#include "iffp/ilbmapp.h"

#include "disp_def.h"
#include "anbr.h"
#include "retcodes.h"

#include "debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "debugson.h"


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

	/* ANHD Chunk */
typedef struct {
    UBYTE operation;	/* Compression method: 5-Byte Vertical Delta */
	UBYTE mask;			/* Not used */
	UWORD w,h;			/* Not used */
	WORD  x,y;			/* Not used */
	ULONG abstime;		/* Not used */
	ULONG reltime;		/* Time to the next frame */
	UBYTE interleave;	/* Must be 0: 2 frame back */
	UBYTE pad0;			/* Not used */
	ULONG bits;			/* Should be 0 */
	UBYTE pad[16];		/* Not used */
} AnimHeader;


typedef struct {
  UWORD loopframenum;	/* Frame to loop back to at end of anim? */
  UWORD numframes;		/* Total number of frames */
  UBYTE animfps;		/* Playback speed for entire anim in 1/60ths */
  UBYTE brushfps;		/* Playback speed for animbrushs? */
  UBYTE pad0;			/* Never seen this used */
  UBYTE extrafps;		/* Another speed seen occasionally? */
} DPANChunk;


#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )

#define ID_ANIM MAKE_ID('A','N','I','M')
#define ID_CAMG MAKE_ID('C','A','M','G')
#define ID_BODY MAKE_ID('B','O','D','Y')
#define ID_CRNG MAKE_ID('C','R','N','G')
#define ID_ANHD MAKE_ID('A','N','H','D')
#define ID_DLTA MAKE_ID('D','L','T','A')
#define ID_DPAN MAKE_ID('D','P','A','N')
#define ID_CCRT MAKE_ID('C','C','R','T')
#define ID_BMHD MAKE_ID('B','M','H','D')
#define ID_CMAP MAKE_ID('C','M','A','P')
#define ID_BODY	MAKE_ID('B','O','D','Y')


VOID
FreeAnimFrames( ANIMFRAME * frame )
{
    D(PRINTF("FreeAnimFrames() ENTERED with frame= 0x%lx\n",frame);)

    if ( !frame )
	return;

    D(PRINTF("FreeAnimFrames() Recursing with frame->next= 0x%lx\n",frame->next);)

    FreeAnimFrames( frame->next );

    D(PRINTF("FreeAnimFrames() Calling FreeVec for frame= 0x%lx\n",frame);)

    FreeVec( frame );

    D(PRINTF("FreeAnimFrames() END\n");)

} // FreeAnimFrames()


VOID
FreeAnbr( ANBR * anbr )
{
    D(PRINTF("FreeAnbr() ENTERED with anbr= 0x%lx\n",anbr);)

    if ( anbr ) {
	if ( anbr->bitmap ) {
	    D(PRINTF("FreeAnbr() anbr->bitmap= 0x%lx\n",anbr->bitmap);)
	    FreeBitMap( anbr->bitmap );
	}
        D(PRINTF("FreeAnbr() animframes= 0x%lx\n",anbr->animframes);)

	if ( anbr->multTable )
	    FreeVec( anbr->multTable );

	if ( anbr->mask )
	    FreeVec( anbr->mask );

	FreeAnimFrames( anbr->animframes );

        D(PRINTF("FreeAnbr() Calling FreeVec( anbr )\n");)

	FreeVec( anbr );
    }

    D(PRINTF("FreeAnbr() END\n");)

} // FreeAnbr()


ANBR *
AllocAnbr( ULONG width, ULONG height, ULONG depth )
{
    ANBR * anbr;

    if ( anbr = AllocVec( sizeof (*anbr), MEMF_CLEAR ) ) {
	// Note that I do not want to use BMF_DISPLAYABLE.
	if (anbr->bitmap = AllocBitMap(width, height, depth, BMF_CLEAR, NULL)) {
	    anbr->width = width;
	    anbr->height = height;
	    return( anbr );
	}
	FreeVec( anbr );
    }

    return( NULL );

} // AllocAnbr()


CreateAnbrMask( ANBR * anbr )
{
    register int i;

    UBYTE * mask;
    struct BitMap bm;
    register int w,h;
    int size;

    if ( !(anbr && anbr->bitmap) )
	return( RC_FAILED );

    if ( anbr->mask )
	FreeVec( anbr->mask );

 
    w = GetBitMapAttr( anbr->bitmap, BMA_WIDTH );
    h = GetBitMapAttr( anbr->bitmap, BMA_HEIGHT );

    size = ((w+7)/8) * h;

//    size = anbr->bitmap->BytesPerRow * anbr->bitmap->Rows;
    mask = (char *) AllocVec( size, MEMF_CHIP|MEMF_CLEAR );

    if ( !(anbr->mask = mask) )
	return( RC_NO_MEM );

//    D(PRINTF("\n\nCreateAnbrMask() w= %ld, BPR= %ld\n",w,anbr->bitmap->BytesPerRow);)

    /*
	OK, now actually create the mask from the real bitmap.
	This is done by ORing the bit planes together.
    */
    /* Setup a temporary bitmap structure to use */
    InitBitMap(&bm,anbr->bitmap->Depth,w,h);

    for ( i = 0; i < anbr->bitmap->Depth; i++ )
	bm.Planes[i] = (PLANEPTR)mask;

    BltBitMap(anbr->bitmap,0,0,&bm,0,0,w,h,0xe0,0xff,NULL);

    return( RC_OK );

} // CreateAnbrMask()


ReadInitialILBM( BPTR fh, int ckSize, ANBR **anbr )
{
    Chunk_Header	chunk_hd;
    BitMapHeader	bmhd;
    AnimHeader		anhd;
    DPANChunk		dpan;
    LONG		pos,maxpos,reltime = 0;
    ULONG		ModeID;
    int			ret;

    *anbr = NULL;

    if ( (pos = Seek( fh, 0, OFFSET_CURRENT )) == -1)
	return( RC_READ_ERROR );

    maxpos = pos + ckSize;

    while ( pos < maxpos ) {

	/* Make sure we are at the next chunks start position */
	if ( Seek( fh, pos, OFFSET_BEGINNING ) == -1 ) {
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	if ( Read( fh, &chunk_hd, sizeof (chunk_hd)) != sizeof (chunk_hd) ) {
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	/* Calculate the next Seek position */
	pos += sizeof (chunk_hd)+((chunk_hd.ckSize+1) & ~1);

	switch (chunk_hd.ckID) {
	    case ID_CCRT:
		D(PRINTF("RII() ID_CCRT\n");)
		break;

	    case ID_CRNG:
		D(PRINTF("RII() ID_CRNG\n");)
		break;

	    case ID_DPAN:
		if ( Read( fh, &dpan, sizeof (dpan) ) != sizeof (dpan) ) {
		    ret = RC_READ_ERROR;
		    goto exit;
		}
		if ( dpan.animfps )
		    reltime = 60/dpan.animfps;

		D(PRINTF("RII() ID_DPAN dpan.animfps= %ld, dpan.brushfps= %ld, dpan.extrafps= %ld\n",
		    dpan.animfps,dpan.brushfps,dpan.extrafps);)
		break;

	    case ID_ANHD:
		if ( Read( fh, &anhd, sizeof (anhd) ) != sizeof (anhd) ) {
		    ret = RC_READ_ERROR;
		    goto exit;
		}
		if ( anhd.reltime )
		    reltime = anhd.reltime;

		D(PRINTF("RII() ID_ANHD anhd.reltime= %ld\n",anhd.reltime);)
		break;

	    case ID_BMHD:
		D(PRINTF("RII() ID_BMHD\n");)
		if ( Read( fh, &bmhd, sizeof (bmhd)) != sizeof (bmhd) ) {
		    ret = RC_READ_ERROR;
		    goto exit;
		}

		if ( bmhd.compression > 1 ) {
		    ret = RC_BAD_COMP;
		    goto exit;
		}

		if ( bmhd.masking == 1 ) {
		    D(PRINTF("bmhd.masking!!!!!\n");)
		}

		if ( !(*anbr = AllocAnbr( bmhd.w, bmhd.h, bmhd.nPlanes )) ) {
		    ret = RC_NO_MEM;
		    goto exit;
		}
		break;

	    case ID_CAMG:
		if ( Read( fh, &ModeID, 4 ) != 4 ) {
		    ret = RC_READ_ERROR;
		    goto exit;
		}
		break;

	    case ID_CMAP:
		D(PRINTF("RII() ID_CMAP\n");)
		break;

	    case ID_BODY:
		D(PRINTF("RII() ID_BODY\n");)
		if ( !*anbr ) {
		    ret = RC_BAD_ILBM;
		    goto exit;
		}
		if (bmhd.compression) {
		    UBYTE * compdata;

		    D(PRINTF("\nCOMPRESSION!!\n");)

		    /* Allocate memory to read the compressed image into */
		    if ( !(compdata = AllocVec( chunk_hd.ckSize, MEMF_CLEAR )) ) {
			ret = RC_NO_MEM;
			goto exit;
		    }

		    /* Read the compressed data in */
		    if ( Read( fh, compdata, chunk_hd.ckSize) != chunk_hd.ckSize) {
			ret = RC_READ_ERROR;
			FreeVec( compdata );
			goto exit;
		    }
		    // Uncompress the data;
		    decode_ilbm_data( (*anbr)->bitmap, (*anbr)->bitmap->Depth, compdata );
//		    decode_interleaved((*anbr)->bitmap,(*anbr)->bitmap->Depth, compdata);

		    FreeVec( compdata );
		} else {
		    int i,j,k,l,len;
		    /* No compression so actually allocate the bitmaps planes */

		    k = (*anbr)->bitmap->Depth;

		    /* Get the plane length */
		    len = (*anbr)->bitmap->BytesPerRow;

		    D(PRINTF("\n NO COMPRESSION!! Depth= %ld, BPR= %ld\n",
			k,len);)

		    /* Read the data into the bitmap */
		    l = 0;
		    for ( j = 0; j < (*anbr)->bitmap->Rows; j++ ) {
			for ( i = 0; i < k; i++ ) {
			    if ( Read( fh, (*anbr)->bitmap->Planes[i]+l,len) != len) {
				ret = RC_READ_ERROR;
				goto exit;
			    }
			}
			l += len;
		    }
		}
		break;

	    default:
		D(PRINTF("RII() Unrecognized chunk_hd.ckID: 0x%lx\n",chunk_hd.ckID);)
		break;
	}

    }

    /* Seek to the last position */
    if ( Seek( fh, maxpos, OFFSET_BEGINNING ) == -1 ) {
	ret = RC_READ_ERROR;
	goto exit;
    }

    ret = RC_OK;

exit:

    if ( ret && *anbr ) {
	FreeAnbr( *anbr );
	*anbr = NULL;

    } else if ( *anbr ) {
	if ( ((*anbr)->reltime = reltime) < 1 )
	    (*anbr)->reltime = 4;

    } else {
	ret = RC_BAD_ANIM;
    }

    return( ret );

} // ReadInitialILBM()


ReadAnimDelta( BPTR fh, int ckSize, ANBR *anbr, ANIMFRAME **retanimframe )
{
    ANIMFRAME		* frame;
    Chunk_Header	  chunk_hd;
    AnimHeader		  anhd;
    LONG		  pos,maxpos,reltime = 0;
    int			  ret;

    *retanimframe = NULL;

    if ( (pos = Seek( fh, 0, OFFSET_CURRENT )) == -1)
	return( RC_READ_ERROR );

    maxpos = pos + ckSize;

    while ( pos < maxpos ) {

	/* Make sure we are at the next chunks start position */
	if ( Seek( fh, pos, OFFSET_BEGINNING ) == -1 ) {
	    ret = RC_READ_ERROR;
	    D(PRINTF("RAD RC_READ_ERROR 1\n");)
	    goto exit;
	}

	if ( Read( fh, &chunk_hd, sizeof (chunk_hd)) != sizeof (chunk_hd) ) {
	    ret = RC_READ_ERROR;
	    D(PRINTF("RAD RC_READ_ERROR 2\n");)
	    goto exit;
	}

	/* Calculate the next Seek position */
	pos += sizeof (chunk_hd)+((chunk_hd.ckSize+1) & ~1);

	switch (chunk_hd.ckID) {
	    case ID_ANHD:
		if ( Read( fh, &anhd, sizeof (anhd) ) != sizeof (anhd) ) {
		    ret = RC_READ_ERROR;
		    D(PRINTF("RAD RC_READ_ERROR 3\n");)
		    goto exit;
		}
		reltime = anhd.reltime;
		D(PRINTF("RAD() ID_ANHD anhd.reltime= %ld\n",anhd.reltime);)
		break;

	    case ID_DLTA:
		D(PRINTF("RAD() ID_DLTA\n");)
		if ( !(frame = AllocVec( sizeof (ANIMFRAME) + chunk_hd.ckSize, MEMF_CLEAR )) ) {
		    ret = RC_NO_MEM;
		    goto exit;
		}
		frame->delta = (UBYTE *)(frame+1);
		*retanimframe = frame;
		/* Read the compressed data in */
		if ( Read(fh, frame->delta, chunk_hd.ckSize) != chunk_hd.ckSize) {
		    ret = RC_READ_ERROR;
		    D(PRINTF("RAD RC_READ_ERROR 4\n");)
		    goto exit;
		}
		break;

	    case ID_CMAP:
		D(PRINTF("RAD() ID_CMAP\n");)
		break;

	    case ID_DPAN:
		D(PRINTF("RAD() ID_DPAN\n");)
		break;

	    default:
		D(PRINTF("RAD() Unrecognized chunk_hd.ckID: 0x%lx\n",chunk_hd.ckID);)
		break;
	}
    }

    /* Seek to the last position */
    if ( Seek( fh, maxpos, OFFSET_BEGINNING ) == -1 ) {
	ret = RC_READ_ERROR;
	D(PRINTF("RAD RC_READ_ERROR 5\n");)
	goto exit;
    }

    ret = RC_OK;

exit:

    if ( ret && *retanimframe ) {
	FreeVec( *retanimframe );
	*retanimframe = NULL;

    } else if ( *retanimframe ) {
	if ( ((*retanimframe)->reltime = reltime) < 1 )
	    (*retanimframe)->reltime = anbr->reltime;
    } else {
	ret = RC_BAD_ANIM;
    }

    return( ret );

} // ReadAnimDelta()


ReadAnbr( UBYTE * filename, ANBR **anbr )
{
    ANIMFRAME		**retanimframe,**next,*prev,*frame,* tmp;
    LONG		* mptr;
    BPTR		  fh;
    FORM_Header		  form_hd;
    Chunk_Header	  chunk_hd;
    LONG		  asize,apos,pos,maxpos,type,numframes;
    int			  i,j,k,l,ret;

    *anbr = NULL;

    if ( !(fh = Open( filename, MODE_OLDFILE )) )
	return( RC_CANT_OPEN );

    if ( Read( fh, &form_hd, sizeof (form_hd)) != sizeof (form_hd) ) {
	ret = RC_READ_ERROR;
	goto exit;
    }

    if ( (form_hd.ckID != ID_FORM) || (form_hd.formtype != ID_ANIM) ) {
	ret = RC_BAD_ANIM;
	goto exit;
    }

    asize = form_hd.ckSize;
    apos = Seek( fh, 0, OFFSET_CURRENT );

    if ( Read( fh ,&form_hd, sizeof (form_hd)) != sizeof (form_hd) ) {
	ret = RC_BAD_ANIM;
	goto exit;
    }

    if ( (form_hd.ckID != ID_FORM) || (form_hd.formtype != ID_ILBM) ) {
	ret = RC_BAD_ANIM;
	goto exit;
    }

    if ( (ret = ReadInitialILBM( fh, form_hd.ckSize-4, anbr )) || !*anbr )
	goto exit;


    maxpos = apos + asize - 4;
    if ( (pos = Seek( fh, 0, OFFSET_CURRENT )) == -1 ) {
	ret = RC_READ_ERROR;
	goto exit;
    }

    D(PRINTF("apos= %ld, asize= %ld, maxpos= %ld, pos= %ld\n",
	apos,asize,maxpos,pos);)

    frame = NULL;
    retanimframe = &frame;
    next = (ANIMFRAME **)&(*anbr)->animframes;
    numframes = 0;

    D(PRINTF("ReadAnbr() A next= 0x%lx\n",next);)

    while ( pos < maxpos ) {

	/* Make sure we are at the next chunks start position */
	if ( Seek( fh, pos, OFFSET_BEGINNING ) == -1 ) {
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	if ( Read( fh, &chunk_hd, sizeof (chunk_hd)) != sizeof (chunk_hd) ) {
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	/* Calculate the next Seek position */
	pos += sizeof (chunk_hd)+((chunk_hd.ckSize+1) & ~1);

	if (chunk_hd.ckID != ID_FORM)
	    continue;

	if ( Read( fh, &type, 4 ) != 4 ) {
	    ret = RC_READ_ERROR;
	    goto exit;
	}

	if ( type != ID_ILBM )
	    continue;


	if ( ret = ReadAnimDelta( fh, chunk_hd.ckSize-4, *anbr, retanimframe ) ) {
	    D(PRINTF("ReadAnbr() ReadAnimDelta() returned ret= %ld\n",ret);)
	    goto exit;
	}

	numframes++;
	*next = frame;
	next = &frame->next;

	D(PRINTF("ReadAnbr() B next= 0x%lx, frame= 0x%lx\n",next,frame);)
	D(
	    for ( tmp = (*anbr)->animframes; tmp; tmp = tmp->next ) {
		D(PRINTF("tmp= 0x%lx\n",tmp);)
	    }
	)
    }

    D(PRINTF("ReadAnbr() RETURNING with numframes= %ld\n",numframes);)

    (*anbr)->numframes = numframes;

    if ( !((*anbr)->multTable = AllocVec( (*anbr)->height << 2, MEMF_CLEAR )) ) {
	ret = RC_NO_MEM;
	goto exit;
    }

    (*anbr)->bwidth = ((*anbr)->width+7) >> 3;

    l = 0;
    j = (*anbr)->height;
    k = (*anbr)->bitmap->BytesPerRow;
    mptr = (*anbr)->multTable;

    for ( i = 0; i < j; i++ ) {
	*(mptr++) = l;
	l += k;
    }

    if ( ret = CreateAnbrMask( *anbr ) )
	goto exit;

    ret = RC_OK;

exit:
    Close( fh );

    if ( ret && *anbr ) {
	D(PRINTF("ReadAnbr() Calling FreeAnbr() ret= %ld\n",ret);)
	FreeAnbr( *anbr );
	*anbr = NULL;
    }

    D(PRINTF("ReadAnbr() returning ret= %ld\n",ret);)

    return( ret );

} // ReadAnbr()




