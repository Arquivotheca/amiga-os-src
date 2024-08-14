

/************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
************************************************************************/


/************************************************************************
*
* track.c
*
* Source Control
* ------ -------
* 
* $Header: fsup.c,v 27.1 85/06/24 13:36:59 neil Exp $
*
* $Locker: neil $
*
* $Log:	fsup.c,v $
* 
* 
************************************************************************/


#include "exec/types.h"
#include "exec/memory.h"
#include "exec/io.h"
#include "libraries/dos.h"
#include "../trackdisk.h"
#include "internal.h"

findsecs( buffer, length )
ULONG buffer;
ULONG length;
{
    ULONG buffend;
    ULONG shiftval;

    buffend = buffer + length;

    while( buffer < buffend ) {
	buffer =  FindSector( buffer, buffend - buffer, &shiftval );

	if( buffer == ~0 ) break;

	printf( "findsecs: buffer 0x%lx, shift %ld, id 0x%0lx\n",
	    buffer, shiftval, ShiftLongDecode( buffer +8, shiftval ) );
	buffer += 4;
    }
}

FindSector( buffer, length, shiftp )
UWORD *buffer;
LONG length;
ULONG *shiftp;
{
    LONG result;

    result = RawFindSector( buffer, length, shiftp );

    if( result != ~0 ) {
	/* adjust the buffer to where it should be... */
	if( *shiftp == 0 ) {
	    result -= 4;
	} else {
	    result -= 4;
	}
    }
    return( result );
}

RawFindSector( buffer, length, shiftp )
UWORD *buffer;
LONG length;
ULONG *shiftp;
{
    int shiftval;

    while( length > 0 ) {
	if( *buffer == 0xAAAA ) {
	    do {
		length -= 2;
		if( length <= 0 ) goto fail;
	    } while( *++buffer == 0xAAAA );

	    shiftval = funnyA1zeroMatch( buffer );
	    if( shiftval != -1 ) {
success:
		*shiftp = shiftval;
		return( (ULONG)buffer );
	    }
	}
	if( *buffer == 0x5555 ) {
	    do {
		length -= 2;
		if( length <= 0 ) goto fail;
	    } while( *++buffer == 0x5555 );

	    shiftval = funnyA1oneMatch( buffer );
	    if( shiftval != -1 ) goto success;
	}
	buffer++;
	length -= 2;
    }
fail:
    return( ~0 );
}

ULONG FindOne_Table[] = {
    0x2244A244
    0x48912891
    0x52244A24
    0x54891289
    0x552244A2
    0x55489128
    0x5552244A
    0x55548912
};

ULONG FindZero_Table[] = {
    0x91225122
    0xA4489448
    0xA9122512
    0xAA448944
    0xAA912251
    0xAAA44894
    0xAAA91225
    0x44894489
};

funnyA1zeroMatch( buffer )
UWORD *buffer;
{
    ULONG *table = FindZero_Table;
    int index;
    ULONG buffvalue = *(ULONG *)buffer;

    for( index = 14; index >= 0; index -= 2 ) {
	if( buffvalue == *table++ ) return( index );
    }

    return( -1 );
}

funnyA1oneMatch( buffer )
UWORD *buffer;
{
    ULONG *table = FindOne_Table;
    int index;
    ULONG buffvalue = *(ULONG *)buffer;

    for( index = 15; index >= 0; index -= 2 ) {
	if( buffvalue == *table++ ) return( index );
    }

    return( -1 );
}


LongDecode( buffer )
ULONG *buffer;
{
    return( (buffer[0] & 0x55555555)<< 1 | (buffer[1] & 0x55555555) );
}

ShiftLongDecode( buffer, shift )
ULONG *buffer;
LONG shift;
{
    ULONG mybuff[2];

    mybuff[0] = ShiftGetLong( &buffer[0], shift );
    mybuff[1] = ShiftGetLong( &buffer[1], shift );

    return( LongDecode( mybuff ) );
}

ShiftGetLong( buffer, shift )
ULONG *buffer;
LONG shift;
{
    if( shift == 0 ) return( buffer[0] );

    return( (buffer[0] << (16 - shift)) | (buffer[1] >> (16 + shift) ) );
}

aligntrack( buffer, size )
ULONG buffer;
LONG size;
{
    LONG shiftval;
    ULONG buffend = buffer + size;
    ULONG id;
    LONG length;
    ULONG sector;
    struct MfmSector *ms;
    LONG remaining;
    int i;
    int secnum;
    int tmpsector;

    sector = FindSector( buffer +6, size - 6 - sizeof( struct MfmSector ),
	&shiftval );

    if( sector == ~0 ) {
	printf( "aligntrack: can't find a sector header!\n" );
	return( 0 );
    }

    id = ShiftLongDecode( sector + 8, shiftval );
    remaining = IDTOSECOFFSET( id );

/*printf( "id 0x%08lx, remaining %ld\n", id, remaining );*/

    length = remaining * sizeof( struct MfmSector );
    if( length + sector >= buffend ) {
	printf( "aligntrack: not enough room for first chunk\n" );
	return( 0 );
    }

    /* make sure that the other sectors are well placed */
    secnum = IDTOSECNUM( id );
    for( i = remaining, tmpsector = sector; i > 0; i-- ) {
	id = ShiftLongDecode(
	    tmpsector + 8, shiftval );

	if( IDTOSECTYPE( id ) != 0xff ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector type (0x%lx)\n",
		tmpsector, shiftval, id, 0xff );
	    return( 0 );
	}
	if( IDTOSECNUM( id ) != secnum ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector number (0x%lx)\n",
		tmpsector, shiftval, id, secnum );
	    return( 0 );
	}
	if( IDTOSECOFFSET( id ) != i ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector offset (0x%lx)\n",
		tmpsector, shiftval, id, remaining );
	    return( 0 );
	}

	if( ++secnum >= NUMSECS ) secnum = 0;
	tmpsector += sizeof( struct MfmSector );
    }

    ShiftCopy( buffer, sector, shiftval, length );

    buffer += length;
    sector += length;
    remaining = NUMSECS - remaining;
    if( remaining == 0 ) {
	/* we have already done everything on the track */
	return( 1 );
    }

    sector = FindSector( sector, buffend - sector, &shiftval );

    if( sector == ~0 ) {
	printf( "aligntrack: can't find the second half of the track!\n" );
	return( 0 );
    }

    id = ShiftLongDecode( sector + 8, shiftval );

/*printf( "    2nd id 0x%08lx\n", id );*/

    length = remaining * sizeof( struct MfmSector );
    if( length + sector >= buffend ) {
	printf( "aligntrack: not enough room for second chunk\n" );
	return( 0 );
    }

    /* make sure that the other sectors are well placed */
    secnum = IDTOSECNUM( id );
    for( i = remaining, tmpsector = sector; i > 0; i-- ) {
	id = ShiftLongDecode(
	    tmpsector + 8, shiftval );

	if( IDTOSECTYPE( id ) != 0xff ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector type (0x%lx)\n",
		tmpsector, shiftval, id, 0xff );
	    return( 0 );
	}
	if( IDTOSECNUM( id ) != secnum ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector number (0x%lx)\n",
		tmpsector, shiftval, id, secnum );
	    return( 0 );
	}
#ifdef undef
/* need to figure the right clause for this... */
	if( IDTOSECOFFSET( id ) != remaining ) {
	    printf( "sector 0x%lx/%ld: (0x%lx) invalid sector offset (0x%lx)\n",
		tmpsector, shiftval, id, remaining );
	    return( 0 );
	}
#endif undef

	if( ++secnum >= NUMSECS ) secnum = 0;
	tmpsector += sizeof( struct MfmSector );
    }

    ShiftCopy( buffer, sector, shiftval, length );

    return( 1 );
}


ShiftCopy( to, from, shift, length )
ULONG *to;
ULONG *from;
LONG shift;
LONG length;
{
    for( ; length > 0; length -= 4 ) {
	*to++ = ShiftGetLong( from++, shift );
    }
}


validatetrack( buffer, length, track )
ULONG buffer;
LONG length;
LONG track;
{
    int offset;
    int secnum;
    int i;
    ULONG id;
    int error = 0;

    id = LongDecode( buffer + 8 );

    offset = IDTOSECOFFSET( id );
    secnum = IDTOSECNUM( id );

    for( i = 0; i < NUMSECS; i++ ) {
	error += validatesector( buffer, offset, secnum, track );
	if( testflags( SIGBREAKF_CTRL_C ) ) break;
	if( testflags( SIGBREAKF_CTRL_E ) ) break;
	if( --offset == 0 ) offset = NUMSECS;
	if( ++secnum >= NUMSECS ) secnum = 0;
	buffer += sizeof( struct MfmSector );
    }

    return( error );
}

validatesector( buffer, offset, secnum, track )
UBYTE *buffer;
LONG offset;
LONG secnum;
LONG track;
{
    ULONG id;
    ULONG sum;
    int error = 0;

    id = LongDecode( buffer + 8 );

    if( IDTOSECOFFSET( id ) != offset ) {
	printf( "sector 0x%lx: wrong offset (expected %ld, got %ld)\n",
	    buffer, offset, IDTOSECOFFSET( id ) );
	error++;
    }

    if( IDTOSECNUM( id ) != secnum ) {
	printf( "sector 0x%lx: wrong sector (expected %ld, got %ld)\n",
	    buffer, secnum, IDTOSECNUM( id ) );
	error++;
    }

    if( track != -1 && IDTOTRACKNUM( id ) != track ) {
	printf( "sector 0x%lx: wrong track (expected %ld, got %ld)\n",
	    buffer, track, IDTOTRACKNUM( id ) );
	error++;
    }

    if( IDTOSECTYPE( id ) != 0xff ) {
	printf( "sector 0x%lx: wrong sector type (expected 0x%lx, got 0x%lx)\n",
	    buffer, 0xff000000, id );
	error++;
    }

    if( (*(ULONG *)buffer != 0xAAAAAAAA && *(ULONG *)buffer != 0x2AAAAAAA ) ||
	*(ULONG *)(buffer +4) != 0x44894489 )
    {
	printf( "sector 0x%lx: bad sec header: 0x%lx 0x%lx 0x%lx 0x%lx\n",
	    buffer,
	    *(UWORD *) (buffer),
	    *(UWORD *) (buffer + 2),
	    *(UWORD *) (buffer + 4),
	    *(UWORD *) (buffer + 6) );
	error++;
    }

    /* check header checksum */
    id = LongDecode( buffer + 48 );
    sum = SumBuffer( buffer + 8, TD_LABELSIZE * 2 + 8 );
    if( sum != id ) {
	printf( "sector 0x%lx: bad header sum: (expected 0x%lx, got 0x%lx)\n",
	    buffer, id, sum );
	error++;
    }

    /* check data checksum */
    id = LongDecode( buffer + 56 );
    sum = SumBuffer( buffer + 64, TD_SECTOR * 2 );
    if( sum != id ) {
	printf( "sector 0x%lx: bad data sum: (expected 0x%lx, got 0x%lx)\n",
	    buffer, id, sum );
	error++;
    }


    /* check valid mfm encoding */
    error += CheckBufLegalMfm( buffer + 8, 0x440 - 8);

    return( error );
	
}


SumBuffer( buffer, length )
ULONG *buffer;
LONG length;
{
    ULONG sum = 0;

    while( length > 0 ) {
	length -= 4;
	sum ^=	(*buffer++ & 0x55555555);
    }
    return( sum );
	
}


CheckLongLegalMfm( buffer )
ULONG *buffer;
{
    ULONG data;
    ULONG clock;

    data = (*buffer & 0x55555555);
    clock = (~data << 1) & ((~data >> 1) | 0x80000000 ) & 0xAAAAAAAA;

    if( buffer[-1] & 0x1 ) clock &= 0x7fffffff;

    if( *buffer != (data | clock) ) {
	printf(
	    "coding error @ 0x%lx (0x%08lx != 0x%08lx | 0x%08lx [0x%08lx])\n",
	    buffer, *buffer, data, clock, data | clock );
	return( 1 );
    }
    return( 0 );
}

CheckBufLegalMfm( buffer, size )
ULONG *buffer;
LONG size;
{
    int errors = 0;

    for( ; size > 0; size -= 4 ) {
	errors += CheckLongLegalMfm( buffer++ );
	if( errors ) break;
	if( testflags( SIGBREAKF_CTRL_C ) ) break;
	if( testflags( SIGBREAKF_CTRL_E ) ) break;
    }

    return( errors );
}


validatedisk( st )
struct store *st;
{
    int track;
    int error;
    int totalerrors = 0;
    int size = 0x4000;
    int numtracks;
    
    if( ! st->buffer ) {
	st->buffer = (ULONG *) AllocMem( size, MEMF_CHIP );
	if( st->buffer ) {
	    printf( "buffer of %ld bytes is at 0x%lx\n", size, st->buffer );
	    st->buffsize = size;
	} else {
	    printf( "could not allocate %ld bytes of CHIP memory\n", size );
	}
    }

    numtracks = GetNumTracks( st->unit );

    for( track = 0; track < numtracks; track++ ) {
	if( testflags( SIGBREAKF_CTRL_C ) ) break;
	clearflags( SIGBREAKF_CTRL_E );

	error = RawCommon( TD_RAWREAD, st->buffer, st->buffsize,
	    track, st->unit, 0 );

	if( error ) {
	    printf( "Error %ld on TD_RAWREAD track %ld\n", error, track );
	    totalerrors++;
	    continue;
	}

	/* align the track */
	if( ! aligntrack( st->buffer, st->buffsize ) ) {
printerror:
	    totalerrors++;
	    printf( "error on track %ld\n", track );
	    continue;
	}

	/* check for errors */
	if( validatetrack( st->buffer, st->buffsize, track ) ) {
	    goto printerror;
	}

    }

    printf( "total of %ld tracks had errors\n", totalerrors );

    Motor( 0, st->unit );
    clearflags( SIGBREAKF_CTRL_C );
}


checkpad( buffer, size, track )
ULONG buffer;
LONG size;
LONG track;
{
    LONG shiftval, newshiftval;
    ULONG buffend = buffer + size;
    ULONG id;
    LONG length;
    ULONG sector, newsector;
    struct MfmSector *ms;
    LONG remaining;
    int i;
    int secnum;
    int offset;

    sector = FindSector( buffer +6, size - 6 - sizeof( struct MfmSector ),
	&shiftval );

    if( sector == ~0 ) {
	printf( "checkpad: can't find a sector header!\n" );
	return( 0 );
    }

    id = ShiftLongDecode( sector + 8, shiftval );
    remaining = IDTOSECOFFSET( id );

printf( "\ncheckpad: id 0x%08lx, remaining %ld, track %ld\n",
id, remaining, track );

    length = remaining * sizeof( struct MfmSector );
    if( length + sector >= buffend ) {
	printf( "checkpad: not enough room for first chunk\n" );
	return( 0 );
    }

    sector += length;

    newsector = FindSector( sector, buffend - sector, &newshiftval );

    if( newsector == ~0 ) {
	printf( "checkpad: can't find the second half of the track!\n" );
	return( 0 );
    }

    for( offset = 0; sector < newsector; sector += 4, offset += 4 ) {
	NullCheck( sector, newshiftval, offset );
    };

    return( 1 );
}

NullCheck( addr, shift, offset )
ULONG *addr;
ULONG shift;
int offset;
{
    ULONG val;

    val = ShiftGetLong( addr, shift );
    if( val != 0xaaaaaaaa ) {
	printf( " %6lx/%08lx", offset, val );
    }
}

validateea( st )
struct store *st;
{
    int track;
    int error;
    int totalerrors = 0;
    int size = 0x4000;
    int numtracks;
    
    if( ! st->buffer ) {
	st->buffer = (ULONG *) AllocMem( size, MEMF_CHIP );
	if( st->buffer ) {
	    printf( "buffer of %ld bytes is at 0x%lx\n", size, st->buffer );
	    st->buffsize = size;
	} else {
	    printf( "could not allocate %ld bytes of CHIP memory\n", size );
	}
    }

    numtracks = GetNumTracks( st->unit );

    for( track = 0; track < numtracks; track++ ) {
	if( testflags( SIGBREAKF_CTRL_C ) ) break;
	clearflags( SIGBREAKF_CTRL_E );

	error = RawCommon( TD_RAWREAD, st->buffer, st->buffsize,
	    track, st->unit, 0 );

	if( error ) {
	    printf( "Error %ld on TD_RAWREAD track %ld\n", error, track );
	    totalerrors++;
	    continue;
	}

	/* check the track padding */
	checkpad( st->buffer, st->buffsize, track );

#ifdef undef
	/* align the track */
	if( ! aligntrack( st->buffer, st->buffsize ) ) {
printerror:
	    totalerrors++;
	    printf( "error on track %ld\n", track );
	    continue;
	}

	/* check for errors */
	if( validatetrack( st->buffer, st->buffsize, track ) ) {
	    goto printerror;
	}
#endif undef

    }

    printf( "total of %ld tracks had errors\n", totalerrors );

    Motor( 0, st->unit );
    clearflags( SIGBREAKF_CTRL_C );
}

