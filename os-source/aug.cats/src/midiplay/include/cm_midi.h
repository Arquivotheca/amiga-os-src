/***********
 cm_midi.h

************/

#ifndef	CM_MIDI_H	// [
#define	CM_MIDI_H

#include	"midi/mididefs.h"
#define	MIDINODE	struct MidiNode

typedef struct	MidiFileHeader {
    ULONG	mfh_ckID;
    ULONG	mfh_HeaderLength;
    UWORD	mfh_FileFormat;
    UWORD	mfh_TrackCount;
    UWORD	mfh_Division;
} MIDIFILEHEADER;


typedef	struct	MidiTrackHeader {
    ULONG	mth_ckID;
    ULONG	mth_TrackLength;
} MIDITRACKHEADER;

#define MAXTRACKS 64L

#define	TRACKDELTA_SET	0x1000
#define	TRACK_DONE		0x0100

typedef	struct MidiTrack {
    UBYTE	* TrackData;
    ULONG	  TrackSize;
    ULONG	  Offset;    // current 'playing' offset into TrackData
    ULONG	  Delta;
    ULONG	  Flags;
} MIDITRACK;


typedef struct cache_node
{
    struct cache_node	* next;
    char		* name;
    int			  type;
    int			  use_cnt;
    short		  flags;
    int			  size;
    int			  chips;
    int			  fast;
    long		  time;

} CACHE_NODE;


typedef	struct	IFF_Midi {
    CACHE_NODE	iffm_cache;
    ULONG	iffm_Flags;
    UWORD	iffm_TrackCount;
    UWORD	iffm_Division;
    MIDITRACK	iffm_Tracks[MAXTRACKS];

} IFF_MIDI;

#define	CACHE_MIDI	12

#define MAKE_ID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )

// Specific to MIDI data files
#define	ID_MTHD	MAKE_ID('M','T','h','d')
#define	ID_MTRK	MAKE_ID('M','T','r','k')


#endif			// ]

