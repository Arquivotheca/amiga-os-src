/*
 *	MidiFile.c
 *	$Header: AVDev:Server/main/av/rcs/midifile.c,v 1.5 92/05/28 19:18:27 WAYNE Exp $
 *
 *	Modification History
 */


#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include	"retcodes.h"
#include	"Structs.h"
#include	"MidiPlay.h"

#include	"DebugsOff.h"
#define		KPRINTF
//#include	"DebugsOn.h"
//#define	PRINTF	DBugMsg

#define		RC_BAD_MIDI	RC_BAD_SMUS

int cm_FreeMem( CACHE_NODE * unused, unsigned char * ptr, int size );
int cm_free_node( CACHE_NODE * node );
int cm_alloc_node( char * name, int type, int size, CACHE_NODE ** nodeptr );
int LoadTrack( BPTR fh, IFF_MIDI * node, MIDITRACK * Track );
VOID * cm_AllocMem( CACHE_NODE * unused, int size, int flags );

VOID
MidiUnLoad( IFF_MIDI * node )
{
    int i;

    D(PRINTF("MidiUnLoad() dumping node 0x%lx\n", node );)
    if ( node ) {
	for ( i = 0; i < node->iffm_TrackCount; i++ ) {
	    D(PRINTF("MidiUnLoad() Track= %ld\n",i);)
	    if ( node->iffm_Tracks[i].TrackData ) {
		cm_FreeMem( (CACHE_NODE *)node, node->iffm_Tracks[i].TrackData,
	 	node->iffm_Tracks[i].TrackSize );
	    }
	}
	cm_free_node( (CACHE_NODE *)node );
    }

}	/* MidiUnLoad() */


int
MidiLoad( UBYTE * path, UBYTE * file, IFF_MIDI ** nodeptr )
{
    IFF_MIDI		* node = NULL;
    BPTR		  fh;
    MIDIFILEHEADER	  filehdr;
    int			  ret,i;

    *nodeptr = NULL;

    if ( !(fh = (BPTR)Open( path, MODE_OLDFILE)) ) {
	D(RINTF("MidiLoad can't Open '%ls'\n",PFSTR(path));)
	return( RC_CANT_FIND );
    }

	/* Read in file's header */
    if ( sizeof( MIDIFILEHEADER ) == Read( fh, &filehdr, sizeof( MIDIFILEHEADER )) )
    {
	if ( (filehdr.mfh_ckID == ID_MTHD) ) {
	    D(PRINTF("\nFileFormat= %ld, TrackCount= %ld\n",
		filehdr.mfh_FileFormat,filehdr.mfh_TrackCount);)

	    if ( filehdr.mfh_TrackCount <= MAXTRACKS ) {
		if ( !(ret = cm_alloc_node( file, CACHE_MIDI,
		    sizeof(IFF_MIDI), (CACHE_NODE **)&node)) ) {

		    Seek( fh, 8+filehdr.mfh_HeaderLength, OFFSET_BEGINNING );
		    node->iffm_TrackCount = filehdr.mfh_TrackCount;
		    node->iffm_Division = filehdr.mfh_Division;

		    for ( i = 0; i < node->iffm_TrackCount; i++ ) {
			if ( ret = LoadTrack( fh, node, &node->iffm_Tracks[i] ) )
			    break;
		    }
		}
	    } else {
		ret = RC_TOO_MANY;
	    }
	} else {
	    ret = RC_BAD_MIDI;
	} 
    } else {
	ret = RC_BAD_MIDI;
    }

exit:

    Close( fh);

    if (ret) {
	MidiUnLoad( node );
    } else {
	*nodeptr = node;
    }

    D(PRINTF("MidiLoad() RETURNING with ret= %ld\n",ret);)

    return (ret);

}	/* MidiLoad() */


int
LoadTrack( BPTR fh, IFF_MIDI * node, MIDITRACK * Track )
{
    MIDITRACKHEADER	trackhdr;
    int			ret;

    D(PRINTF("\nLoadTrack ENTERED with Track= 0x%lx\n",Track);)

    if ( Read( fh, &trackhdr, sizeof( MIDITRACKHEADER )) != sizeof( MIDITRACKHEADER ) ) {
	return( RC_BAD_MIDI );
    }

    D(PRINTF("Track hdr len= %ld\n",trackhdr.mth_TrackLength);)
    if ( !(Track->TrackData = cm_AllocMem( (CACHE_NODE *)node, trackhdr.mth_TrackLength, MEMF_CLEAR)) ) {
	return( RC_NO_MEM );
    }

    ret = RC_OK;
    Track->TrackSize = trackhdr.mth_TrackLength;

    if ( Read( fh, Track->TrackData, Track->TrackSize) != Track->TrackSize ) {
	ret = RC_READ_ERROR;
	D(PRINTF("RC_READ_ERROR... Track->TrackSize= %ld\n",Track->TrackSize);)
    }

    D(PRINTF("LoadTrack RETURNING\n");)

    return( ret );

} // LoadTrack()


int
MidiExpand( CACHE_NODE * node, int flags )
{

    return( RC_OK );

} /* MidiExpand() */


int
MidiCompress( CACHE_NODE * node )
{

 return( RC_OK );

} /* MidiCompress() */

