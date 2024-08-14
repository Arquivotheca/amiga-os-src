/************
   MidiPlay

    W.D.L
*************/

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

#define	STANDALONE

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>

#include	"Structs.h"

#include	"MidiPlay.h"

#include	"libraries/realtime.h"

#include	"midi/camd.h"        /* CMP_In(), CMP_Out() macros */
#include	"midi/camdbase.h"
#include	"midi/mididefs.h"

#include	"clib/camd_protos.h"
#include	"clib/realtime_protos.h"

#include	"pragmas/camd_pragmas.h"
#include	"pragmas/realtime_pragmas.h"



#include "stdio.h"
#include <stdlib.h>
#include <math.h>	// For min() & max()
#include <string.h>	// for setmem()


#include	"retcodes.h"
#include	"phonyseg.h"

#include	"MidiPlay_rev.h"

#include	"DebugsOff.h"
#define		KPRINTF
//#include	"DebugsOn.h"
//#define	PRINTF	DBugMsg

//#define	kprintf printf


#define	MIDIPLAY	VERSTAG " Wayne D. Lutz"

UBYTE	* Version = MIDIPLAY;


MIDICONTEXT	* MidiContext = NULL;

struct MidiLink		* OutLink;
struct Player	* PInfo;


#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif


// Systring #defines
//#define	SS_OUTLINKNAME		"AV_MIDI_OUT.0"
#define	SS_OUTLINKNAME			"out.0"
#define	SS_MLINK_NAME			"AV_MIDI_Link"
#define	SS_AV_MIDI				"AV_MIDI"
#define	SS_AV_MIDI_PLAYER		"AV_MIDI_PLAYER"
#define	SS_AV_MIDI_CONDUCTOR	"AV_MIDI_CONDUCTOR"
#define SS_CAMDNAME				"camd.library"
#define	SS_REALTIME_NAME		"realtime.library"


#define CamdVersion		37
#define RealTimeVersion	0



#define	SER_UNIT	0	/* Serial Unit */


#define SYSEXBUFSIZE (MAXTRACKS*1024)
#define INITTEMPO (500000L)
//#define TSHIFT2 (11)
#define TSHIFT2 (10)
#define tadjust(t) (t)

#define		RC_BAD_MIDI	RC_BAD_SMUS

ULONG	tfactor;

int		MidiGetChar();
VOID	CloseCamd();

STATIC	int	Running;
STATIC	int SySexContinue;

typedef int (*PFI)();

#define NULLFUNC 0
#ifndef NULL
#define NULL 0
#endif
#ifndef EOF
#define EOF (-1)
#endif



/* public stuff */

/* Functions to be called while processing the MIDI file. */

int (*Mf_trackstart)( VOID ) = (PFI)NULLFUNC;
int (*Mf_noteon)( int chan, int pitch, int vol ) = (PFI)NULLFUNC;
int (*Mf_noteoff)( int chan, int pitch, int vol ) = (PFI)NULLFUNC;
int (*Mf_tempo)( long tempo ) = (PFI)NULLFUNC;

#ifdef	INERT
int (*Mf_error)() = (PFI)NULLFUNC;
int (*Mf_header)() = (PFI)NULLFUNC;
int (*Mf_pressure)() = (PFI)NULLFUNC;
int (*Mf_parameter)() = (PFI)NULLFUNC;
int (*Mf_pitchbend)() = (PFI)NULLFUNC;
int (*Mf_program)() = (PFI)NULLFUNC;
int (*Mf_chanpressure)() = (PFI)NULLFUNC;
int (*Mf_sysex)() = (PFI)NULLFUNC;
int (*Mf_arbitrary)() = (PFI)NULLFUNC;
int (*Mf_metamisc)() = (PFI)NULLFUNC;
int (*Mf_seqnum)() = (PFI)NULLFUNC;
int (*Mf_eot)() = (PFI)NULLFUNC;
int (*Mf_smpte)() = (PFI)NULLFUNC;
int (*Mf_timesig)() = (PFI)NULLFUNC;
int (*Mf_keysig)() = (PFI)NULLFUNC;
int (*Mf_seqspecific)() = (PFI)NULLFUNC;
int (*Mf_text)() = (PFI)NULLFUNC;
#endif

int MidiGetChar( MIDITRACK * Track);
VOID cm_release( CACHE_NODE * cur );
void play_noteoff( int chan, int pitch, int vol );
int cm_obtain( char * name, int type, int flags, CACHE_NODE ** ptr );
int MidiLoad( UBYTE * path, UBYTE * file, IFF_MIDI ** nodeptr );
VOID MidiUnLoad( IFF_MIDI * node );


int Mf_nomerge = 0;		/* 1 => continue'ed system exclusives are */
				/* not collapsed. */
long Mf_currtime = 0L;		/* current time in delta-time units */

/* private stuff */

#ifndef	INERT
static long readvarinum( MIDITRACK * Track );
static long read32bit( MIDITRACK * Track );
static long to32bit(char,char,char,char);
static int read16bit( MIDITRACK * Track );
static int to16bit(int,int);
//static void readheader(void);
static void metaevent(int);
static void sysex(void);
static void chanmessage(int,int,int);
static char *msg(void);
static int msgleng(void);
static void msginit(void);
static msgadd(int);
static biggermsg(void);
#endif


struct CamdBase *CamdBase;
struct RealTimeBase	*RealTimeBase;

ULONG starttime;
ULONG playtempo;
STATIC int	MidiAbortMask;
STATIC int	MidiAlarmMask;



#ifdef	STANDALONE	// [
#define		SS_MIDIPROCESS	"MidiProcess"

BOOL	DumpMidi;
TASK * parent;
int par_sigBit = -1;
int par_sigMask;
int	anim_sig_mask = 0;
UBYTE	UNDOBUFFER[256];

#else	// ][

extern int	par_sigMask;
extern int	anim_sig_mask;
extern struct Task *parent;

#endif	// ]

STATIC	int	SubReturnCode;
STATIC	TASK * MidiTask = NULL;

#define	STACKSIZE_MIDIPLAYER	(1024 * 8)


#ifndef	OUTT	// [
	/* stub functions */

struct MidiNode *CreateMidi(Tag tag, ...)
{	return CreateMidiA((struct TagItem *)&tag );
}

BOOL SetMidiAttrs(struct MidiNode *mi, Tag tag, ...)
{	return SetMidiAttrsA(mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type, Tag tag, ...)
{	return AddMidiLinkA(mi, type, (struct TagItem *)&tag );
}

BOOL SetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...)
{	return SetMidiLinkAttrsA(mi, (struct TagItem *)&tag );
}

struct Player *CreatePlayer(Tag tag, ...)
{	return CreatePlayerA((struct TagItem *)&tag );
}

BOOL SetPlayerAttrs(struct Player *pi, Tag tag, ...)
{	return SetPlayerAttrsA(pi, (struct TagItem *)&tag );
}
#endif			// ]

//***************************************************************

/* The code below allows collection of a system exclusive message of */
/* arbitrary length.  The Msgbuff is expanded as necessary.  The only */
/* visible data/routines are msginit(), msgadd(), msg(), msgleng(). */

#define MSGINCREMENT 128;
static char *Msgbuff = NULL;	/* message buffer */
static int Msgsize = 0;		/* Size of currently allocated Msg */
static int Msgindex = 0;	/* index of next available location in Msg */

static
void msginit()
{
    Msgindex = 0;
}

static char *
msg()
{
    return(Msgbuff);
}

static int
msgleng()
{
    return(Msgindex);
}

static
biggermsg()
{
    char *newmess;
    char *oldmess = Msgbuff;
    int oldleng = Msgsize;

    Msgsize += MSGINCREMENT;
    if ( newmess = AllocMem( (unsigned)(sizeof(char)*Msgsize), MEMF_CLEAR ) ) {

	/* copy old message into larger new one */
	if ( oldmess != NULL ) {
	    register char *p = newmess;
	    register char *q = oldmess;
	    register char *endq = &oldmess[oldleng];

	    for ( ; q!=endq ; p++,q++ )
		*p = *q;
	    FreeMem( oldmess, oldleng );
	}
	Msgbuff = newmess;
	return( TRUE );
    } else {
	Msgsize = oldleng;
	return( FALSE );
    }
}


static
msgadd( int c )
{
    /* If necessary, allocate larger message buffer. */
    if ( Msgindex >= Msgsize ) {
	if ( !biggermsg() )
	    return( FALSE );
    }

    Msgbuff[Msgindex++] = c;
    return( TRUE );
}

//*****************************************************************

static void
metaevent( int type )
{
    char *m = msg();

    switch  ( type ) {
	case 0x51:	/* Set tempo */
	    if ( Mf_tempo )
		(*Mf_tempo)(to32bit( '\0', m[0], m[1], m[2] ));
	    break;

#ifdef	INERT
	case 0x00:
	    if ( Mf_seqnum )
		(*Mf_seqnum)(to16bit(m[0],m[1]));
	    break;
	case 0x01:	/* Text event */
	case 0x02:	/* Copyright notice */
	case 0x03:	/* Sequence/Track name */
	case 0x04:	/* Instrument name */
	case 0x05:	/* Lyric */
	case 0x06:	/* Marker */
	case 0x07:	/* Cue point */
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
	    /* These are all text events */
	    if ( Mf_text )
		(*Mf_text)(type,leng,m);
	    break;
	case 0x2f:	/* End of Track */
	    if ( Mf_eot )
		(*Mf_eot)();
	    break;
	case 0x54:
	    if ( Mf_smpte )
		(*Mf_smpte)(m[0],m[1],m[2],m[3],m[4]);
	    break;
	case 0x58:
	    if ( Mf_timesig )
		(*Mf_timesig)(m[0],m[1],m[2],m[3]);
	    break;
	case 0x59:
	    if ( Mf_keysig )
		(*Mf_keysig)(m[0],m[1]);
	    break;
	case 0x7f:
	    if ( Mf_seqspecific )
		(*Mf_seqspecific)(leng,m);
	    break;

	default:
	    if ( Mf_metamisc )
		(*Mf_metamisc)(type,leng,m);
#endif
    }

}	/* metaevent() */


static void
sysex()
{

#ifdef	INERT
    if ( Mf_sysex )
	(*Mf_sysex)( msgleng(), msg());
#endif

}	/* sysex() */


/*
 * readvarinum - read a varying-length number, and return the
 *	number of characters it took.
 */
static long
readvarinum( MIDITRACK * Track )
{
    long	value;
    int		c;

    c = MidiGetChar(Track);
    value = c;

    if ( c & 0x80 ) {
	value &= 0x7f;
	do {
	    c = MidiGetChar(Track);
	    if ( Track->Flags & TRACK_DONE )
		return( 0 );

	    value = (value << 7) + (c & 0x7f);
	} while ( c & 0x80 );
    }
    return( value );

}	/* readvarinum() */


static long
to32bit( char c1, char c2, char c3, char c4)
{
    long value = 0L;

    value = (c1 & 0xff);
    value = (value<<8) + (c2 & 0xff);
    value = (value<<8) + (c3 & 0xff);
    value = (value<<8) + (c4 & 0xff);
    return (value);

}	/* to32bit() */


static
to16bit( int c1, int c2 )
{

    return( ((c1 & 0xff ) << 8) + (c2 & 0xff) );

}	/* to16bit() */


static long
read32bit( MIDITRACK * Track )
{
    int c1, c2, c3, c4;

    c1 = MidiGetChar(Track);
    c2 = MidiGetChar(Track);
    c3 = MidiGetChar(Track);
    c4 = MidiGetChar(Track);
    if ( Track->Flags & TRACK_DONE )
	return(0);

    return( to32bit(c1,c2,c3,c4) );

}	/* read32bit() */



static
read16bit( MIDITRACK * Track )
{
    int c1, c2;

    c1 = MidiGetChar(Track);
    c2 = MidiGetChar(Track);
    if ( Track->Flags & TRACK_DONE )
	return( 0 );

    return( to16bit(c1,c2) );

}	/* read16bit() */


ULONG
waittime( VOID )
{
    ULONG etime;
    ULONG signals = NULL;

    etime = starttime + Mf_currtime;

    D(PRINTF("etime= %ld, starttime= %ld, currtime= %ld, tempo= %ld, Division= %ld, TickFreq= %ld\n",
	etime,starttime,Mf_currtime,playtempo,MidiContext->mc_Division,((struct RealTimeBase *)RealTimeBase)->rtb_Reserved1);)

    if ( RealTimeBase->rtb_Time < etime ) {

	if ( SetPlayerAttrs( PInfo,
	    PLAYER_AlarmTime, etime,
	    PLAYER_Ready, TRUE,
	    TAG_END) ) {


	    while (1) {

		signals = Wait( MidiAbortMask | MidiAlarmMask );

		if ( signals & MidiAlarmMask ) {
		    break;
		}

		if ( signals & MidiAbortMask ) {
		    D(PRINTF("waittime() GOT MidiAbortMask\n");)

		    if ( MidiContext->mc_Flags & MIDICONTEXT_ABORTED ) {
			MidiContext->mc_Flags |= MIDICONTEXT_FLAG_EOF;

			D(PRINTF("waittime() abort requested: set EOF flag\n");)
			break;
		    }
		    D(PRINTF("waittime() GOT AbortMask but NOT MIDICONTEXT_ABORTED signals= 0x%lx, MidiAbortMask= 0x%lx, MidiAlarmMask= 0x%lx\n",
			signals,MidiAbortMask,MidiAlarmMask);)
		}
	    }
	}
    }

    return(signals);

}	/* waittime() */


MIDICONTROL *
GetMidiControl( VOID )
{
    int	chan;

    for( chan = 0; chan < MAXTRACKS; chan++ ) {
	if ( !MidiContext->mc_MidiControl[chan].turnon ) {
	    D(PRINTF("ON %ld - 0x%lx\n",chan,&MidiContext->mc_MidiControl[chan]);)
	    return( &MidiContext->mc_MidiControl[chan] );
	}
    }

    return( &MidiContext->mc_MidiControl[0] );

} // GetMidiControl()


MIDICONTROL *
FindMidiControl( int midichan, int pitch )
{
    int	chan;

    D(PRINTF("FindMidiControl() ENTERED\n");)

    for( chan = 0; chan < MAXTRACKS; chan++ ) {
	if( MidiContext->mc_MidiControl[chan].turnon &&
	  ( MidiContext->mc_MidiControl[chan].MidiChan == midichan ) &&
	  ( MidiContext->mc_MidiControl[chan].pitch == pitch ) ) {

		return( &MidiContext->mc_MidiControl[chan] );
	}
    }

    return( NULL );

} // FindMidiControl()


void
play_noteon( int chan, int pitch, int vol)
{
    MidiMsg msg;
    ULONG	signals;

    D(PRINTF("play_noteon ENTERED with vol= %ld\n",vol);)

    if ( !(vol = (vol * MidiContext->mc_Volume) / 64) ) {
	D(PRINTF("play_noteon CALLING play_noteoff with vol= %ld\n",vol);)
	play_noteoff(chan,pitch,vol);
	return;
    }

    D(PRINTF("play_noteon...  chan= %ld, pitch= %ld, vol= %ld\n",chan,pitch,vol);)

    if ( !Mf_currtime && (starttime != RealTimeBase->rtb_Time) )
	starttime = RealTimeBase->rtb_Time;

    signals = waittime();

    if ( MidiContext->mc_Flags & MIDICONTEXT_MODEMIDI ) {
	MIDICONTROL * m;

	D(PRINTF("MODEMIDI ");)
	msg.mm_Status = MS_NoteOn | chan;
	msg.mm_Data1 = pitch;
	msg.mm_Data2 = vol;
	PutMidiMsg( OutLink, &msg );
	m = GetMidiControl();
	m->turnon = TRUE;
	m->MidiChan = chan;
	m->pitch = pitch;
    }

}	/* play_noteon() */


void
play_noteoff( int chan, int pitch, int vol )
{
    MidiMsg	msg;
    ULONG	signals;

    signals=waittime();

    D(PRINTF("play_noteoff...  chan= %ld, pitch= %ld, vol= %ld\n",chan,pitch,vol);)

    if ( MidiContext->mc_Flags & MIDICONTEXT_MODEMIDI ) {
	MIDICONTROL * m;

	D(PRINTF("noteoff !!!!!!!!MODEMIDI!!!!!!!!\n");)
	msg.mm_Status = MS_NoteOff | chan;
	msg.mm_Data1 = pitch;
	msg.mm_Data2 = vol;
	PutMidiMsg( OutLink, &msg );

	if ( m = FindMidiControl( chan, pitch ) ) {
		m->turnon = FALSE;
	}
    }

}	/* play_noteoff() */


void
play_allnotesoff( VOID )
{
    MidiMsg	msg;
    int		chan;


    D(PRINTF("play_allnotesoff() ENTERED\n");)

    if (MidiContext->mc_MidiNode) {

	D(PRINTF("play_allnotesoff() 1\n");)

	if ( MidiContext->mc_Flags & MIDICONTEXT_MODEMIDI ) {

	    D(PRINTF("!!!!!!!!MODEMIDI!!!!!!!!\n");)
	    for( chan=0; chan<MAXTRACKS; chan++ ) {
		D(PRINTF("play_allnotesoff() 3\n");)

		if ( MidiContext->mc_MidiControl[chan].turnon ) {
		    msg.mm_Status = MS_NoteOff | MidiContext->mc_MidiControl[chan].MidiChan;
    		    msg.mm_Data1 = MidiContext->mc_MidiControl[chan].pitch;
		    msg.mm_Data2 = 0;
		    PutMidiMsg( OutLink, &msg );
		    D(PRINTF("play_allnotesoff() turning off chan %ld\n",chan);)
		    MidiContext->mc_MidiControl[chan].turnon = FALSE;
		}
	    }
	}

	D(PRINTF("play_allnotesoff() 5\n");)
    }

    D(PRINTF("play_allnotesoff() RETURNING\n");)

}	/* play_allnotesoff() */


void
play_trackstart( VOID )
{
    starttime = RealTimeBase->rtb_Time;

}	/* play_trackstart() */


ULONG
changetempo(ULONG ctbpm)
{
    ULONG timefac,timerem,tickfreq;

//    tickfreq=(ULONG)((struct RealTimeBase *)RealTimeBase)->rtb_TickFreq;
//    tickfreq = TICK_FREQ;
    tickfreq=(ULONG)((struct RealTimeBase *)RealTimeBase)->rtb_Reserved1;

    if ( tickfreq != TICK_FREQ )
	tickfreq = 600;

    D(PRINTF("tickfreq= %ld\n",tickfreq);)

    // Note that older realtime.librarys used 600 as the TickFreq value.
    // This was stored in the TickFreq field. Now, since it has been
    // realized that this TickFreq value can not realistically be changed
    // without breaking existing programs, the recommended method is to
    // use the TICK_FREQ (1200) value. But, if you are running on a 1.3
    // system that has the older realtime.library on it, you need to
    // use the value stored in the TickFreq field. This field has been
    // renamed to rtb_Reserved1, but still holds the correct Tick_Freq
    // value. So, I read this value to get the correct Tick_Freq.

    /*
     *	One tick = quarter note/pSMFHeader->Division.
     *	Hence, microseconds per delta is given by ctbpm/division
     *	and BPM is given by (tickfreq*100000)/ctbpm.
     */

    timefac=(tickfreq * (ctbpm/MidiContext->mc_Division)) / 1000000;
    timerem=(tickfreq * (ctbpm/MidiContext->mc_Division)) % 1000000;

    if ( timerem >= 500000 )
	timefac++;

    D(PRINTF("changetempo ctbpm= %ld, timefac= %ld\n",ctbpm,timefac);)

    return( tfactor = timefac);

} // changetempo()


void
play_tempo( long tempo )
{
    ULONG signals;


    D(PRINTF("play_tempo NEW tempo= %ld, old= %ld, MidiContext->mc_Division= %ld\nTickFreq= %ld\n",
	tempo,playtempo,MidiContext->mc_Division,((struct RealTimeBase *)RealTimeBase)->rtb_Reserved1);)


    signals = waittime();
    playtempo = tempo;		  

    changetempo( tempo );

}	/* play_tempo() */




int
MidiGetChar( MIDITRACK * Track)
{
    int	ch;

    if ( Track->Offset >= Track->TrackSize ) {
	Track->Flags |= TRACK_DONE;
	ch = EOF;
	D(PRINTF("MidiGetChar SETTING EOF offset= %ld, Size= %ld\n",
	Track->Offset,Track->TrackSize);)
    } else {
	ch = *(Track->TrackData + Track->Offset++);
    }

    D(PRINTF("%lx ", ch );)
    return( ch );

}	/* MidiGetChar() */


static void
chanmessage(int status, int c1, int c2)
{
    int chan = status & 0xf;

    D(PRINTF("chanmessage ENTERED\n");)

    switch ( status & 0xf0 ) {
	case NOTEOFF:
	    if ( Mf_noteoff )
		(*Mf_noteoff)(chan,c1,c2);
	    break;

	case NOTEON:
	    if ( Mf_noteon )
		(*Mf_noteon)(chan,c1,c2);
	    break;

#ifdef	INERT	// [
	case PRESSURE:
	    if ( Mf_pressure )
		(*Mf_pressure)(chan,c1,c2);
	    break;

	case PARAMETER:
	    if ( Mf_parameter )
		(*Mf_parameter)(chan,c1,c2);
	    break;

	case PITCHBEND:
	    if ( Mf_pitchbend )
		(*Mf_pitchbend)(chan,c1,c2);
	    break;

	case PROGRAM:
	    if ( Mf_program )
		(*Mf_program)(chan,c1);
	    break;

	case CHANPRESSURE:
	    if ( Mf_chanpressure )
		(*Mf_chanpressure)(chan,c1);
	    break;
#endif		// ]
    }

}	/* chanmessage() */


MIDITRACK *
GetTrack( VOID )
{
    MIDITRACK * track,* ret_track = NULL;
    int		i;

    D(PRINTF("\n*********GetTrack()*********** TrackCount= %ld\n",
	MidiContext->mc_TrackCount);)

    for ( i = 0; i < MidiContext->mc_TrackCount; i++ ) {

	D(PRINTF("GetTrack() 1 i= %ld\n",i);)

	track = &MidiContext->mc_Tracks[i];

	D(PRINTF("GetTrack() 2, track= 0x%lx\n",track);)

	if ( track->Flags & TRACK_DONE ) {
	    D(PRINTF("TRACK_DONE!!!\n");)
	    continue;
	}

	D(PRINTF("GetTrack() 3\n");)

	if ( !(track->Flags & TRACKDELTA_SET) ) {
	    D(PRINTF("GetTrack() 4\n");)

	    track->Delta = readvarinum(track);
	    track->Flags |= TRACKDELTA_SET;
	    D(PRINTF("Setting Delta= %ld\n",track->Delta);)
	} else {
	    D(PRINTF("Delta= %ld\n",track->Delta);)
	}

	D(PRINTF("GetTrack() 5\n");)

	if ( !i || !ret_track ) {
	    ret_track = track;
	    D(PRINTF("GetTrack() 6 SETTING ret_track to 0x%lx\n",ret_track);)

	} else if ( ret_track && (track->Delta < ret_track->Delta) ) {
	    ret_track = track;

	    D(PRINTF("GetTrack() 7 RESETTING ret_track to 0x%lx\n",ret_track);)
	}

	D(PRINTF("GetTrack() 8\n");)
    }

    D(PRINTF("GetTrack() 9\n");)

    if ( ret_track ) {
	D(PRINTF("GetTrack() 10 ret_track= 0x%lx\n",ret_track);)

	ret_track->Flags &= ~TRACKDELTA_SET;

	D(PRINTF("********** RETURNING Track Delta= %ld\n",ret_track->Delta);)

	for ( i = 0; i < MidiContext->mc_TrackCount; i++ ) {
	    track = &MidiContext->mc_Tracks[i];
	    if ( track == ret_track )
		continue;

	    track->Delta -= ret_track->Delta;
	}
    }

    D(PRINTF("GetTrack() END ret_track= 0x%lx\n",ret_track);)

    return( ret_track );

} // GetTrack()


static VOID
readtrack( VOID )
{
    /* This array is indexed by the high half of a status byte.  It's */
    /* value is either the number of bytes needed (1 or 2) for a channel */
    /* message, or 0 (meaning it's not  a channel message). */
    static int chantype[] = {
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x00 through 0x70 */
	2, 2, 2, 2, 1, 1, 2, 0		/* 0x80 through 0xf0 */
    };
    long lng,len;
    int c, c1, c2, type;
    int sysexcontinue = 0;	/* 1 if last message was an unfinished sysex */
    int running = 0;	/* 1 when running status used */
    int status = 0;		/* (possibly running) status byte */
    int needed;
    MIDITRACK	* Track;

    D(PRINTF("readtrack() ENTERED\n");)

    Mf_currtime = 0;

    if ( Mf_trackstart )
	(*Mf_trackstart)();

    while (1) {
	if ( MidiContext->mc_Flags & MIDICONTEXT_FLAG_EOF ) {
	    D(PRINTF("readtrack() RETURNING 1\n");)
	    return;		// terminates playback
	}

	if ( !(Track = GetTrack()) ) {
	    D(PRINTF("readtrack() GetTrack RETURNED NULL;\n");)
	    MidiContext->mc_Flags |= MIDICONTEXT_FLAG_EOF;
	    return;
	}

	D(PRINTF("readtrack() GetTrack() RETURNED Track= 0x%lx\n",Track);)

	Mf_currtime += (Track->Delta * tfactor);

	c = MidiGetChar(Track);
	if ( Track->Flags & TRACK_DONE ) {
	    continue;
	}

	if ( sysexcontinue && c != 0xf7 ) {
	    D(PRINTF("readtrack() RETURNING 2\n");)
	    return;		// terminates playback
	}

	if ( (c & 0x80) == 0 ) {	 /* running status? */
	    if ( status == 0 ) {
		D(PRINTF("readtrack() RETURNING 3\n");)
		return;		// terminates playback
	    }
	    running = 1;
	} else {
	    status = c;
	    running = 0;
	}

	Running = running;

	needed = chantype[ (status>>4) & 0xf ];

	if ( needed ) {		/* ie. is it a channel message? */
	    c1 = ( running ) ? c : MidiGetChar(Track);
	    c2 = (needed > 1) ? MidiGetChar(Track) : 0;
	    if ( Track->Flags & TRACK_DONE ) {
		continue;
	    }
	    chanmessage( status, c1, c2 );
	    continue;
	}

	switch ( c ) {
	case 0xff:			/* meta event */
	    type = MidiGetChar(Track);
	    len = lng = readvarinum(Track);
	    if ( Track->Flags & TRACK_DONE ) {
		continue;
	    }
	    msginit();
	    while ( lng-- ) {
		if ( !msgadd( c = MidiGetChar(Track) ) ) {
		    D(PRINTF("readtrack() RETURNING 4\n");)
		    return;
		}
	    }

	    if ( Track->Flags & TRACK_DONE ) {
		continue;
	    }

	    metaevent(type);
	    break;

	case 0xf0:		/* start of system exclusive */
	    lng = readvarinum(Track);
	    msginit();
	    if ( !msgadd(0xf0) ) {
		D(PRINTF("readtrack() RETURNING 5\n");)
		return;
	    }
	    while ( lng-- ) {
		if ( !msgadd( c = MidiGetChar(Track) ) ) {
		    D(PRINTF("readtrack() RETURNING 6\n");)
		    return;
		}
	    }

	    if ( Track->Flags & TRACK_DONE ) {
		continue;
	    }

	    if ( (c == 0xf7) || (Mf_nomerge == 0) )
		sysex();
	    else
		SySexContinue = sysexcontinue = 1;  /* merge into next msg */
	    break;

	case 0xf7:	/* sysex continuation or arbitrary stuff */
	    lng = readvarinum(Track);
	    if ( !sysexcontinue )
		msginit();

	    while ( lng-- ) {
		if ( !msgadd( c = MidiGetChar(Track) ) ) {
		    D(PRINTF("readtrack() RETURNING 7\n");)
		    return;
		}
	    }

	    if ( Track->Flags & TRACK_DONE ) {
		continue;
	    }

	    if ( !sysexcontinue ) {
#ifdef	INERT
		if ( Mf_arbitrary )
		    (*Mf_arbitrary)(msgleng(),msg());
#endif
	    } else if ( c == 0xf7 ) {
		sysex();
		SySexContinue = sysexcontinue = 0;
	    }
	    break;
	default:
	    break;
	}
    }

    D(PRINTF("readtrack() RETURNING END\n");)
    return;

}	/* readtrack() */


VOID
CloseCamd( VOID )
{

    if ( CamdBase ) {
	D(PRINTF("\nClosing CamdBase!!\n");)
	CloseLibrary( (LIBRARY *)CamdBase );
	CamdBase = NULL;
    }

    if ( RealTimeBase ) {
	D(PRINTF("\nClosing RealTimeBase!!\n");)
	CloseLibrary( (LIBRARY *)RealTimeBase );
	RealTimeBase = NULL;
    }

}	/* CloseCamd() */


STATIC VOID
MidiCleanup( VOID )
{

    D(PRINTF("MidiCleanup ENTERED\n");)

    if ( MidiContext->mc_MidiNode ) {
	play_allnotesoff();
	D(PRINTF("MidiCleanup 1\n");)
	Delay( 10 );

	D(PRINTF("MidiCleanup A\n");)

	if ( PInfo ) {
	    DeletePlayer( PInfo );
	    PInfo = NULL;
	}

	D(PRINTF("MidiCleanup B\n");)

	if ( OutLink ) {
	    RemoveMidiLink( OutLink );
	    OutLink= NULL;
	}

	DeleteMidi( MidiContext->mc_MidiNode );
    }

    D(PRINTF("MidiCleanup 2\n");)
    CloseCamd();
    MidiTask = NULL;

    D(PRINTF("MidiPlayProc(abort) MidiContext = 0x%lx\n\t CNode 0x%lx\n",
	MidiContext, MidiContext->mc_CacheNode);)

    cm_release( (CACHE_NODE *)MidiContext->mc_CacheNode );

    D(PRINTF("MidiCleanup 3\n");)

    FreeMem( MidiContext, sizeof( MIDICONTEXT ) );

    MidiContext = NULL;

    D(PRINTF("MidiCleanup 4\n");)

    if ( Msgbuff ) {
	FreeMem( Msgbuff, Msgsize );
	Msgbuff = NULL;
	Msgsize = NULL;
    }

    D(PRINTF("returned RETURNING\n");)

}	/* MidiCleanup() */


STATIC int
GetMidiError( VOID )
{
    int	rc;

    switch ( rc = IoErr() ) {
	case CME_NoMem:
	case CME_NoSignals:
	case CME_NoTimer:
	    rc = RC_NO_MEM;
	    break;

	case CME_NoUnit( 0 ):
	    rc = RC_SERIAL_BUSY;
	    break;

	default:
	    rc = RC_FAILED;
	    break;
    }

    return( rc );

} // GetMidiError()



STATIC VOID __saveds
MidiPlayProc( VOID )
{
    int 	i;
    int		MidiAbortBit = -1,MidiAlarmBit = -1;



    D(PRINTF("MidiPlayProc() ENTERED\n");)

    if ( (MidiAbortBit = AllocSignal(-1)) == -1 )
	goto MPP_Abort;

    D(PRINTF("MidiPlayProc() 1\n");)

    MidiAbortMask = 1 << MidiAbortBit;

    MidiTask = FindTask( NULL );

    D(PRINTF("Calling CreateMidi\n");)

    if ( MidiContext->mc_MidiNode = (MIDINODE *)CreateMidi(
	MIDI_Name,		SS_AV_MIDI,
	MIDI_MsgQueue,		255,
	MIDI_ErrFilter,		CMEF_All,
	MIDI_SysExSize,		SYSEXBUFSIZE,
	TAG_END) )
    {

	D(PRINTF("Got MidiNode \n");)

	if ( !(OutLink = AddMidiLink(MidiContext->mc_MidiNode,MLTYPE_Sender,
	    MLINK_Name,		SS_MLINK_NAME,
	    // Only give it an outlink name if MODEMIDI
	    MidiContext->mc_Flags & MIDICONTEXT_MODEMIDI ? 
	    MLINK_Location : TAG_IGNORE,	SS_OUTLINKNAME,
	    TAG_END ) ) )
	{
	    SubReturnCode = GetMidiError();
	    D(PRINTF("Could NOT AddMidiLink!!! SubReturnCode= %ld\n",SubReturnCode);)
	    goto MPP_Abort;
	}

	    D(PRINTF("Got Outlink\n");)

	    if ( (MidiAlarmBit = AllocSignal(-1)) == -1 ) {
		SubReturnCode = RC_NO_MEM;
		goto MPP_Abort;
	    }

	    MidiAlarmMask = 1 << MidiAlarmBit;

	    D(PRINTF("Got MidiAlarmMask \n");)

	    if ( !(PInfo = CreatePlayer(
		PLAYER_Name,		SS_AV_MIDI_PLAYER,
		PLAYER_Conductor,	SS_AV_MIDI_CONDUCTOR,
		PLAYER_AlarmSigBit, 	MidiAlarmBit,
		PLAYER_AlarmSigTask,	MidiTask,
		TAG_DONE )) )
	    {

		SubReturnCode = GetMidiError();

		D(PRINTF("Could NOT CreatePlayer!!! SubReturnCode= %ld\n",SubReturnCode);)
		goto MPP_Abort;
	    }

	    D(PRINTF("Gpt PInfo \n");)

	    SetConductorState( PInfo, CONDSTATE_RUNNING, 0 );

	    D(PRINTF("SetConductorState \n");)

	    SubReturnCode = RC_OK;
	    Signal( parent, par_sigMask );

	    Mf_trackstart = (PFI)play_trackstart;
   	    Mf_noteon =  (PFI)play_noteon;
	    Mf_noteoff = (PFI)play_noteoff;
	    Mf_tempo = (PFI)play_tempo;

	    playtempo = tadjust(INITTEMPO);
	    changetempo( playtempo );


	    D(PRINTF("repeatcnt: %ld\n", MidiContext->mc_RepeatCnt ))

	    while (1) {
		readtrack();

		if ( MidiContext->mc_Flags & MIDICONTEXT_ABORTED )
		    break;

		if ( (MidiContext->mc_RepeatCnt == -1) ||
		  --MidiContext->mc_RepeatCnt )
		{
		    D(PRINTF("repeatcnt: %ld\n", MidiContext->mc_RepeatCnt ))

		    MidiContext->mc_Flags &= ~MIDICONTEXT_FLAG_EOF;

		    for ( i = 0; i < MidiContext->mc_TrackCount; i++ ) {
			MidiContext->mc_Tracks[i].Offset =
			MidiContext->mc_Tracks[i].Delta =
			MidiContext->mc_Tracks[i].Flags = NULL;
		    }

		} else {
		    PF("exiting repeat loop")
		    break;
		}
	    }

    } else {
	PF("JDGJr 2.5")
	SubReturnCode = GetMidiError();
	PF("Error: could not initialize midi environment")

	Signal( parent, par_sigMask );

    }
    PF("JDGJr 2.6")


MPP_Abort:

    D(PRINTF("Calling MidiCleanup()\n");)
    MidiCleanup();

    D(PRINTF("MPP_Abort: 1 \n");)

    D(PRINTF("MPP_Abort: 2\n");)

    if ( MidiAlarmBit != -1 )
	FreeSignal( MidiAlarmBit );

	D(PRINTF("MPP_Abort: 3\n");)

    if ( MidiAbortBit != -1 )
	FreeSignal( MidiAbortBit );

    D(PRINTF("MPP_Abort: 4\n");)

    Forbid();
    MidiTask = NULL;
    Signal( parent, par_sigMask );

    D(PRINTF("MidiPlayProc() last message before returning!!!\n");)

}	/* MidiPlayProc() */


STATIC PHONYSEGLIST __aligned MidiFakeSegment= { 0, 0x4ef9, (LONG(* )())MidiPlayProc, 0 };


MidiPlay( UBYTE * filename, UBYTE * ipath, int vol, int mode, ULONG repcnt )
{
    PROCESS	* newproc;
    TASK	* newtask;
    int		  retcode;

    D(PRINTF("MidiPlay ENTERED\n");)

    if ( MidiTask )
	return( RC_SMUS_ACTIVE );

    PF("\n\n\n\n-------------------------------JDGJr 1.1--------------------------------")
    D(PRINTF("MidiPlay ENTERED with vol= %ld, filename= '%ls', repcnt= %ld, mode= 0x%lx, ipath= '%ls'\n",
	vol,PFSTR(filename),repcnt,mode,PFSTR(ipath) );)

    if ( !(CamdBase = (struct CamdBase *)OpenLibrary(SS_CAMDNAME, CamdVersion)) ) {
	printf("Error: could not open camd.library\n");
	return( RC_NO_AVMIDI );
    }

    if ( !(RealTimeBase = (struct RealTimeBase *)OpenLibrary(SS_REALTIME_NAME, RealTimeVersion)) ) {
	printf("Error: could not open realtime.library\n");
	return( RC_NO_REALTIME );
    }

    if ( !(MidiContext = AllocMem( sizeof( MIDICONTEXT ), MEMF_CLEAR )) ) {
	PF("Could not alloc MidiContext!")
	retcode = RC_NO_MEM;
	goto MidiPlayFail;
    }

    MidiContext->mc_RepeatCnt = (repcnt) ? repcnt : -1;
    MidiContext->mc_Flags = mode + 1;		// convert to bit pattern

    D(PRINTF("MidiPlay() 1. MidiContext = 0x%lx\n", MidiContext );)

    PF("JDGJr 1.2")
    if ( !(retcode = cm_obtain( filename, CACHE_MIDI,
		NULL, (CACHE_NODE **)&MidiContext->mc_CacheNode )) )
    {
	MidiContext->mc_Tracks = MidiContext->mc_CacheNode->iffm_Tracks;
	MidiContext->mc_TrackCount = MidiContext->mc_CacheNode->iffm_TrackCount;
	MidiContext->mc_Division = MidiContext->mc_CacheNode->iffm_Division;
	MidiContext->mc_Volume = vol;

	D(PRINTF("MidiPlay() 2. MidiContext = 0x%lx\n\t CNode 0x%lx\n,mc_Tracks= 0x%lx, TrackCount= %ld\n",
	    MidiContext, MidiContext->mc_CacheNode,MidiContext->mc_Tracks,MidiContext->mc_TrackCount);)

	D(PRINTF("/* Launch the child, and wait for it to get started */\n");)

	Forbid();
	SubReturnCode = -1;

	if ( newproc = (PROCESS *)CreateProc( SS_MIDIPROCESS, 35L,
	  (LONG)&MidiFakeSegment >> 2, STACKSIZE_MIDIPLAYER) ) 
	{
	    newtask = (TASK *)((UBYTE *)newproc - sizeof(TASK));
	    Permit();
	    D(PRINTF("/* Wait for the child to startup */\n");)

	    while (SubReturnCode == -1)
		Wait( par_sigMask );

	    /*
	     * Normal return: we've initiated the playback; return
	     *	signaling everything's OK. Do not free anything!
	     */
	    PF("JDGJr 1.5")
	    SetSignal(0L, par_sigMask);

	    if ( (retcode = SubReturnCode) != RC_OK )
		goto MidiPlayFail;

	    retcode = RC_OK;

	} else {
	    Permit();
	    PF("JDGJr 1.6")
	}
	PF("MidiPlay() last message before returning SUCCESS")
	return( retcode );

    } else {
	D(PRINTF("%ls error #%ld.\n\n", filename, retcode );)
    }

MidiPlayFail:
    D(PRINTF("MidiPlay() last message before returning FAILURE %ld\n", retcode );)

    return( retcode );

}	/* MidiPlay() */


/*
 * Call this routine to poll if MIDI is operational.
 *	NULL: no MIDI operational
 *	otherwise: MIDI operational
 */
int
MidiCheck( VOID )
{

    return( (int)MidiTask );

}	/* MidiCheck() */


VOID
MidiAbort( VOID )
{
    int mask;

    D(PRINTF("MidiAbort() entered... 0x%lx\n", MidiTask );)

    if ( MidiTask) {
	D(PRINTF("Send the kill signal %lx\n", MidiAbortMask );)

	MidiContext->mc_Flags |= MIDICONTEXT_ABORTED;

	/* Tell the child to terminate */
	Signal( MidiTask, MidiAbortMask);

	mask = par_sigMask | anim_sig_mask;

	while ( MidiTask ) {
	    D(PRINTF("MidiAbort() waiting for MidiTask == NULL\n");)
	}
	D(PRINTF("MidiAbort() we're outa here!!\n");)
    }

}	/* MidiAbort() */


//*********************************************************************

#ifdef	STANDALONE /* [ */

#define	MAX_ERRORS	36	
#define	UNKNOWN_ERROR	29

STATIC	UBYTE * ErrorStr[MAX_ERRORS+1] = {
	" An operation failed ",
	" Done with an operation ",
	" No error ",
	" File not found ",
	" Bad ILBM file ",
	" Read error ",
	" Can't find instrument ",
	" Bad compression type ",
	" No memory ",
	" Bad ANIM file ",
	" Confused programmer ",
	" Off the screen ",
	" Syntax error ",
	" No translator library ",
	" Unable to open the narratore device ",
	" SMUS is currently active (can not speak) ",
	" Bad 8SVX file ",
	" Too many MIDI tracks ",
	" No audio device ",
	" Bad SMUS ",
	" Too Many Instruments ",
	" No Cache Manager ",
  	" No screen, not a good situation ",
  	" DOB list already in the active list ",
	" Voice active ",
 	" Serial Device is BUSY ",
  	" Audio Device BUSY ",
  	" CIA Device BUSY ",
	" No Instruments ",
	" Unknown Error ",
	" Missing File ",
	" they can't nest! ",					
	" No CD Audio ",
	" CD Audio Bad Track ",
	" CD Audio Bad Range ",
	" Could NOT open camd.library ",
	" Could NOT open realtime.library ",
};


int
cm_FreeMem( CACHE_NODE * unused, unsigned char * ptr, int size )
{

    D(PRINTF("cm_AllocMem() entered 0x%lx size = %ld\n", ptr, size ))
    FreeMem( ptr, size );

    return( 0 );

}	/* cm_FreeMem() */


VOID *
cm_AllocMem( CACHE_NODE * unused, int size, int flags )
{
    VOID * ptr;

    ptr = AllocMem( size, flags );

    D(PRINTF("cm_AllocMem() 0x%lx\n", ptr ))

    return( ptr );

}	/* cm_AllocMem() */

STATIC CACHE_NODE * CachedNode;

cm_alloc_node( char * name, int type, int size, CACHE_NODE ** nodeptr )
{
    CACHE_NODE	* node;

    *nodeptr = CachedNode = NULL;

    PF("cm_alloc_node() entered")
     if ( node = (CACHE_NODE *)AllocMem( size, MEMF_CLEAR )) {
	node->size = size;
	node->type = type;
	*nodeptr = CachedNode = node;
	D(PRINTF("... node = 0x%lx\n", node ))
    } else {
	D(PRINTF("cm_alloc_node Alloc FAILED size= %ld\n",size);)
    }

    return( 0 );

}	/* cm_alloc_node() */


int
cm_free_node( CACHE_NODE * node )
{
    D(PRINTF("cm_free_node() entered 0x%lx size = %ld\n", node, node->size))
    FreeMem( node, node->size );

    return( 0 );

}	/* cm_free_node() */


int
cm_obtain( char * name, int type, int flags, CACHE_NODE ** ptr )
{

    switch ( type ) {
	case	CACHE_MIDI:
	    D(PRINTF("cm_obtain()... MIDI name= '%ls'\n",PFSTR(name));)
	    return( MidiLoad( name, name, (IFF_MIDI **)ptr ) );

	default:
	    D(PRINTF("cm_obtain()... default... %ld name= '%ls'\n",type,PFSTR(name));)
	    break;

    }

}	/* cm_obtain() */


VOID
cm_release( CACHE_NODE * cur )
{

    switch (cur->type) {
	case CACHE_MIDI:
	    D(PRINTF("cm_release() MIDI\n");)
	    MidiUnLoad( (IFF_MIDI *)cur );
	    break;

	default:
	    D(PRINTF("cm_release() UNKNOWN type 0x%lx !!!\n",cur->type);)
	    break;
    }

}	/* cm_release() */


VOID
PrintTemplate( VOID )
{
    printf("\nMidiPlay W.D.L 1992\nUsage:\n  MidiPlay FileName\n\n");

    exit(0);

} /* PrintTemplate() */


UBYTE *
ParseCmdLine( int argc, char * argv[] )
{
    int	i, j;

    if ( argc < 2 )
	PrintTemplate();

    for ( i=1; i<argc; i++ ) {
	j = ( argv[i][0] == '-' ) ? 1: 0;

	if( i == (argc-1) ) {
	    if( argv[i][j] == '?' )
		PrintTemplate();

	    return( &argv[i][j] );
	}

	switch ( argv[i][j] ) {
	    // DumpMidi is NOT supported in this version. W.D.L 931101
	    case 'D':
	    case 'd':
		DumpMidi = TRUE;
		break;

	    default:
		printf("Unrecognized paramater %c\n",argv[i][j]);
		break;

	} /* switch */
    }

    return( NULL );

}	/* ParseCmdLine() */


int
main( int argc, char ** argv )
{
    int		  retcode;
    ULONG	  signals,mask;
    UBYTE	* filename;

    if( !(filename = ParseCmdLine( argc, argv )) )
	exit(0);

    parent = FindTask( NULL );

    /* Allocate a signal for the child to ping us with */
    if ((par_sigBit = AllocSignal(-1)) == -1) {
	printf("main Could not AllocSignal!!!\n");
	goto exit;
    }

    par_sigMask = 1 << par_sigBit;

    if ( RC_OK == (retcode = MidiPlay( filename, NULL, 64, 2, 1)) ) {

	D(PRINTF("main() waiting for MidiPlay() to complete ...\n");)
	mask = par_sigMask|SIGBREAKF_CTRL_C;

	while ( MidiCheck() ) {
	    printf("CTRL_C to break...\n");

	    if( (signals = Wait( mask )) & SIGBREAKF_CTRL_C ) {
		MidiAbort();
	    }
	}
	PF("MidiCheck() says we're finished!")

    } else {
	printf("MidiPlay() returned error %ld -  ", retcode );
	if ( (retcode += 2) > MAX_ERRORS )
	    retcode = UNKNOWN_ERROR;
	printf("'%ls'\n",ErrorStr[retcode]);
    }


exit:

    FreeSignal(par_sigBit);

    D(PRINTF("returning gracefully ...\n");)

    SetTaskPri(FindTask(NULL),0L);
    exit( 0 );

}	/* main() */


#endif	/* ] */

