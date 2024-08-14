/* :ts=4
*
*	qanim.c
*
*	William A. Ware							B716
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright (C) 1991, Silent Software Incorporated.						*
*   All Rights Reserved.													*
****************************************************************************/


#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>

#include <hardware/custom.h>

#include <devices/audio.h>

#include <proto/graphics.h>

#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>

#include "sound.h"

extern struct Custom __far custom;

#define	DeBoxBase	(td->DeBoxBase)


void flushaudio(struct TData *td )
{
	register struct SoundCommand *sc;
	struct SmSound	*ss;
	UBYTE			*data;
	
	while( sc = (struct SoundCommand *)GetMsg( td->AudioReply ))
	{
		/* kprintf("0x%08lx Returned\n",sc ); */
		if (ss = sc->s) 
		{
			ss->Lock--;
		}
		sc->s = NULL;
		Remove( (struct Node *)&sc->n );
		if (sc != &td->keysc)
			AddHead( &td->freecmd, (struct Node *)&sc->n );
		if (ss && (ss->Lock <= 0)) 
			if (ss->FreeSize) FreeMem(ss,ss->FreeSize); 
		td->SQue--;
	}
}


struct SoundCommand *AllocSC(struct TData *td)
{
	register struct SoundCommand *sc;
	UBYTE *tc;
	
//	if (tc = (UBYTE *)RemHead( &td->freecmd )) 
//	{
//		sc = (struct SoundCommand *)&tc[-sizeof(struct IOAudio)];
//		return(sc);
//	}
	flushaudio(td);
	if (tc = (UBYTE *)RemHead( &td->freecmd )) 
	{
		sc = (struct SoundCommand *)&tc[-sizeof(struct IOAudio)];
		return(sc);
	}

	if (sc = AllocMem( sizeof(struct SoundCommand),MEMF_PUBLIC))
	{
		CopyMem( &td->keysc, sc, sizeof( struct SoundCommand ) );
		return(sc);
	}
	return(NULL);
}


VOID sendaudio( struct TData *td, register struct SoundCommand *sc )
{
	if (sc->s) sc->s->Lock++;

	/* kprintf("0x%08lx Sent\n",sc ); */
	AddHead( &td->inusecmd, (struct Node *)&sc->n );
	BeginIO( sc );
	td->SQue++;
}


FlushSounds( struct TData *td,UBYTE ch )
{
	register struct SoundCommand *sc = NULL;

	if (!(sc = AllocSC(td))) return(0);  
	sc->ac.ioa_Request.io_Command = CMD_FLUSH;
	sc->ac.ioa_Request.io_Unit  = (struct Unit *)ch;
	sc->ac.ioa_Request.io_Flags = 0;
	sendaudio( td,sc );
	return(1);
}

VOID FreeSound(struct TData *td, register struct SmSound *ss)
{
	flushaudio(td);
	while (ss->Lock) 
	{
		flushaudio(td);
		WaitTOF();
	}
	FreeMem(ss,ss->Size + sizeof(struct SmSound));
}


static UBYTE channeldata[] = { 0x0f };

extern struct CIA __far ciaa;

void restoreFilter( struct TData *td )
{
	Disable();
	if (!(td->oldciabits & CIAF_LED))
	{
		ciaa.ciapra &= ~(CIAF_LED);
	}
	Enable();
}


void FreeAudio(struct TData *td)
{
	register UBYTE *u;	

	if (td->AudioReply)
	{
		FlushSounds( td,0xf );
		
		
		WaitTOF();WaitTOF();
		while( td->inusecmd.lh_Head->ln_Succ )
			{ flushaudio(td); }
		while( u = (UBYTE *)RemHead( &td->freecmd ) )
			FreeMem( &u[-sizeof(struct IOAudio)], sizeof(struct SoundCommand) );
		CloseDevice( (struct IORequest *)&td->keysc );
		DeleteMsgPort( td->AudioReply );
		td->AudioReply = NULL;
		
		restoreFilter( td );
	}
}

InitAudio(struct TData *td)
{
	struct SoundCommand *sc;
	int i;

	if (td->AudioReply) return 0;
	
	if (!(td->AudioReply = CreateMsgPort())) TERR(TERR_MEMORY);
	
	Disable();
	td->oldciabits = ciaa.ciapra;
	ciaa.ciapra |= CIAF_LED;
	Enable();
	
	
	NewList( &td->freecmd );
	NewList( &td->inusecmd );
	MemSet( &td->keysc, 0, sizeof(struct SoundCommand));

	td->keysc.ac.ioa_Request.io_Message.mn_ReplyPort   = td->AudioReply;
	td->keysc.ac.ioa_Data     = channeldata;
	td->keysc.ac.ioa_Length   = 1;
	td->keysc.ac.ioa_Request.io_Flags					= IOF_QUICK;
	td->keysc.ac.ioa_Request.io_Command					= ADCMD_ALLOCATE;
	td->keysc.ac.ioa_AllocKey = 0; 
	if (OpenDevice( "audio.device",0L,(struct IORequest *)&td->keysc.ac,0L )) goto err;
	
	for(i=0;i<10;i++) 
	{
		if (!(sc = AllocSC(td))) goto err;
		AddHead( &td->freecmd, (struct Node *)&sc->n );
	}
	
	

	return 0;
err:
	DeleteMsgPort( td->AudioReply );
	td->AudioReply = NULL;
	restoreFilter( td );

	TERR(TERR_MEMORY)
}


PlaySmSound(struct TData *td,
			struct SmSound *ss, int times, UBYTE ch, int volume,
			 int period)
{
	register struct SoundCommand *sc = NULL;
	register int i;
	long			size;
	UBYTE			*data;
	
	if (volume > 63) volume = 63;
	if ((ss->Volume > 63) || !ss->Volume) ss->Volume = 63;
	
	if ((size = ss->Size - sizeof(struct SmSound)) > 131072) times = 1;
	data = (UBYTE *)(&ss[1]);
	
	while( 1 ) 
	{
		for(i=1;i<0x10;i += i )
		{
			if (ch & i) 
			{
				if (!(sc = AllocSC(td))) return(0);  
				sc->s = ss;
				sc->ac.ioa_Request.io_Command = CMD_WRITE;
				sc->ac.ioa_Request.io_Unit  = (struct Unit *)i;
				sc->ac.ioa_Request.io_Flags = ADIOF_PERVOL; 
				sc->ac.ioa_Data				= (APTR)data; 
				sc->ac.ioa_Length			= (size > 131072 ? 131072:size);
				sc->ac.ioa_Period			= (period ? period:ss->Speed); 
				sc->ac.ioa_Volume			= (volume ? volume:ss->Volume); 
				sc->ac.ioa_Cycles			= 1;
				sendaudio(td,sc);
				if ((times > 1) || !times)
				{
					if (!(sc = AllocSC(td))) return(0);  
					sc->s = ss;
					sc->ac.ioa_Request.io_Command = CMD_WRITE;
					sc->ac.ioa_Request.io_Unit  = (struct Unit *)i;
					sc->ac.ioa_Request.io_Flags = ADIOF_PERVOL; 
					sc->ac.ioa_Data	= (APTR)&((UBYTE *)ss)[ss->RepeatOffset]; 
					sc->ac.ioa_Length			= ss->Size - ss->RepeatOffset;
					sc->ac.ioa_Period			= (period ? period:ss->Speed); 
					sc->ac.ioa_Volume			= (volume ? volume:ss->Volume); 
					sc->ac.ioa_Cycles			= times-1;
					sendaudio(td,sc);
				}
			}
		}
		if (size > 131072) { size -= 131072;data += 131072; } 
		else break;
	}
	return(1);
}


VOID SetVolume( struct TData *td,UBYTE ch, UWORD volume)
{
	if (volume > 63) volume = 63;

	flushaudio(td);
	Forbid();
	custom.aud[0].ac_vol = custom.aud[1].ac_vol = 
		custom.aud[2].ac_vol =  custom.aud[3].ac_vol = volume;
	/*
	for(	sa = (struct SoundCommand *)td->inusecmd.lh_Head;
			sa->n.mln_Succ; sa = (struct SoundCommand *)sa->n.mln_Succ )
	{
		if ((long)sa->ac.ioa_Request.io_Unit & ch) 
			sa->ac.ioa_Volume = volume;
	}
	*/
	Permit();
}


NextSound( struct TData *td,UBYTE ch, int next )
{
	register struct SoundCommand *sc = NULL;

	flushaudio(td);
	if (!(sc = AllocSC(td))) return(0);  
	sc->ac.ioa_Request.io_Command = ADCMD_FINISH;
	sc->ac.ioa_Request.io_Unit  = (struct Unit *)ch;
	sc->ac.ioa_Request.io_Flags = (next ? ADIOF_SYNCCYCLE:0);
	sendaudio(td,sc);
	return(1);
}

