/*
 * MidiPlay.h
 */

#include	"cm_midi.h"

#ifndef	MIDIPLAY_H
#define	MIDIPLAY_H

#define	CHAN_COUNT		4

typedef	struct MidiControl
{
	LONG	MidiChan;
	LONG	pitch;
	BOOL	turnon;

} MIDICONTROL;

typedef	struct Note {
	LONG	MidiChan;
	LONG	AmigaChan;
	LONG	instrument;
	LONG	pitch;
	LONG	volume;
	BOOL	turnon;
} NOTE;


typedef struct MidiContext {
	IFF_MIDI	* mc_CacheNode;
	UWORD		  mc_Division;
	UWORD		  mc_TrackCount;

	MIDINODE	* mc_MidiNode;

	MIDITRACK	* mc_Tracks;

	ULONG		  mc_Flags;
	ULONG		  mc_RepeatCnt;


	MSGPORT		* mc_Ports[CHAN_COUNT];	// 4 ports for AIOBs
	IOAUDIO		* mc_AudioIOB;
	NOTE		  mc_Notes[CHAN_COUNT];
	SHORT		  mc_NoteCnt;				// mc_Notes[] count MAX - CHAN_COUNT
	LONG		  mc_InstrumentNo;
	LONG		  mc_SCount;
	LONG		  mc_Mode;
	ULONG		  mc_ChannelsAlloced;	// for AudioChannel#?() tracking

	BOOL		  mc_DeviceFlag[CHAN_COUNT];
	UBYTE		  mc_InReg[4];			// 4 intrument registers
	SHORT		  mc_Volume;
	MIDICONTROL	  mc_MidiControl[MAXTRACKS];

} MIDICONTEXT;


#define	MIDICONTEXT_FLAG_EOF	0x0010
#define	MIDICONTEXT_ABORTED		0x0020
#define	MIDICONTEXT_MODESMUS	0x0001
#define	MIDICONTEXT_MODEMIDI	0x0002

IMPORT	MIDICONTEXT	* MidiContext;


#define NOTEOFF 0x80
#define NOTEON 0x90
#define PRESSURE 0xa0
#define PARAMETER 0xb0
#define PITCHBEND 0xe0
#define PROGRAM 0xc0
#define CHANPRESSURE 0xd0


#endif

