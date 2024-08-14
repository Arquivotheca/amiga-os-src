/* :ts=4
*
*	star.c
*
*	William A. Ware						D312
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>
#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>

#include "title.h"

#define XRND_SIZE	307
#define YRND_SIZE	444

#define WIDTH		352
#define HEIGHT		592
#define HWIDTH		(WIDTH/2-32)
#define HHEIGHT		(HEIGHT/2+10)

#define MAGIC		256

static UBYTE StarColors[ 8 ] =
{
	7,3,6,6,2,2,1,1
};

WORD __asm QWPixel(	register __a0 struct BitMap *,
					register __a1 UWORD *YTable,
					register __d0 WORD,register __d1 WORD,register __d2 WORD);

void __asm RestoreBB( 	register __a0 struct BitMap *,
						register __a1 struct BitMap *,
						register __d0 WORD );

#define	DeBoxBase	(td->DeBoxBase)
#define GfxBase 	(td->GfxBase)


extern __far struct CompHeader viols[];

static struct CompHeader *InstrArr[ INSTR_MAX ] =
{
	viols
};

enum
{
	PINS_END,
	PINS_PLAY,	// duration,SND#,times,CH,PERIOD
	PINS_VOLUME // duration,ch,speed,destvol
};

static WORD demoPlay[] =
{
	PINS_VOLUME,0,0,0,0,
	PINS_PLAY,0,INSTR_VIOLS,99,0,380,
	PINS_VOLUME,20,0,1000000/63,63,
	PINS_PLAY,10,INSTR_VIOLS,99,0,340,
	PINS_VOLUME,10,0,1000000/63,0,
	PINS_END
};

/*------------------------------------------------------------------------*/

LONG MicrosSince( struct TData *td, LONG *lasttime )
{
	LONG 	ct = td->CountUp;
	WORD	ans;
	
	ans = ct - *lasttime;
	*lasttime = ct;
	return ((WORD)ans*(WORD)td->MPF);
}

//------------------------------------------------------------------------------

void AddStars( struct TData *td )
{
	struct Star		*s;
	WORD			count;

	count = td->MaxStars - td->starCount;

	while( count-- > 0 )
	{
		if (!(s = (struct Star *)RemHead( &td->freeList ))) 
			break;
		
		td->starCount++;
		s->X = Rnd( &td->starSeed,XRND_SIZE ) - (XRND_SIZE/2);
		s->Y = Rnd( &td->starSeed,YRND_SIZE ) - (YRND_SIZE/2);
		s->Z = 255;
		
		AddTail( &td->activeList, (struct Node *)s );
	}
}


//------------------------------------------------------------------------------

void doStars( struct TData *td, WORD move )
{
	LONG			ct = td->CountUp;
	WORD			x,y,warp = 0;
	struct Star		*s,*nexts;
	struct BitMap	*bm,*rbm;
	
	if (td->Flags & TD_3PLANEON)
	{
		bm = td->Bm3;
		rbm= td->RBm3;
	}
	else
	{
		bm = td->Bm;
		rbm = td->RBm;
	}

	if (move)
	{
		warp = (ct-td->lastStarTime) * td->Warp;
		td->lastStarTime = ct;
	}
	else warp = 0;
	
	s = (struct Star *)td->activeList.lh_Head;
	while( nexts = (struct Star *)s->Node.mln_Succ )
	{
		if ((s->Z -= warp) < 0) goto endpt;

		x = (s->X << 8)/s->Z + HWIDTH;
		y = (s->Y << 8)/s->Z + HHEIGHT;

		RestoreBB( rbm,bm,s->Offset );

		if ((x < 0) || (x > WIDTH) || (y < 0) || (y > HEIGHT))
		{
endpt:
			Remove( (struct Node *)s );
			td->starCount--;
			AddTail( &td->freeList,(struct Node *)s );
			s = nexts;
			continue;
		}

		s->Offset = QWPixel( bm, td->BYTable, StarColors[s->Z>>5], x,y );
		s = nexts;
	}
	AddStars( td );
}




void starsOff( struct TData *td )
{
	struct Star		*s,*nexts;
	struct BitMap	*bm,*rbm;
	
	// Bitmap is oppisate since we want to turn of stars on the non displayed map.
	//
	if (td->Flags & TD_3PLANEON)
	{
		bm = td->Bm;
		rbm = td->RBm;
	}
	else
	{
		bm = td->Bm3;
		rbm= td->RBm3;
	}

	s = (struct Star *)td->activeList.lh_Head;
	while( nexts = (struct Star *)s->Node.mln_Succ )
	{
		RestoreBB( rbm,bm,s->Offset );
		s = nexts;
	}
}

//===========================================================================
static playSound( struct TData *td, struct SmSound *ss,
						LONG times, LONG ch,LONG volume,LONG period )
{
	struct SoundInfo	*si = &td->Sound[ch];
	LONG				ans;

	if (ch) ch = 0x0c;
	else	ch = 0x03;

	if (volume >= 0) 
		{ si->DestVolume = si->Volume = volume;si->RampTimer = 1; }
	
	ans = PlaySmSound( td,ss,times,ch, si->Volume, period ); 

	si->Period = period;
	
	return ans;
}

void StartMusic( struct TData *td, WORD *music )
{
	td->SoundPC = music;
	td->NextCmd = td->CountUp;
}



static void setDestVolume( struct TData *td,WORD ch, LONG time,LONG dest )
{
	struct SoundInfo	*si = &td->Sound[ch];

	si->DestVolume = dest;
	si->LastTime = td->CountUp;
	si->RampTimer = si->RampTime = time;
}

static void nextCmd( struct TData *td )
{
	WORD temp;

	if (temp = *td->SoundPC++)
		td->NextCmd = td->CountUp + temp * (td->FPS/10);
}


DoSound( struct TData *td )
{
	int					i,ans = 0;
	struct SoundInfo	*si;
	LONG				ms,ch;
	struct SmSound		*ss;

	flushaudio(td);

	
	while (td->SoundPC && (td->NextCmd <= td->CountUp))
	{
		ans = 1;
		switch (*td->SoundPC++)
		{
			case PINS_END: 
				FlushSounds( td,0xf );
				td->SoundPC = NULL; 
				break;
			case PINS_PLAY: 
				nextCmd( td );
				ss = td->Instr[ *td->SoundPC++ ];
				i = *td->SoundPC++;
				ch = *td->SoundPC++;
				playSound( td, ss, i,ch,-1,*td->SoundPC++ );
				break;
			case PINS_VOLUME:
				nextCmd( td );
				ch = *td->SoundPC++;
				i = *td->SoundPC++;
				setDestVolume( td, ch, i, *td->SoundPC++ );
				break;
		}
	}
	

	for(i=0,si = td->Sound;i<2;i++,si++)
	{
		if ( si->Volume != si->DestVolume )
		{
			ms = MicrosSince( td, &si->LastTime ); 
			ans = 1;
			si->RampTimer -= ms;
			while(si->RampTimer < 0)
			{
				if (si->Volume < si->DestVolume) si->Volume ++;
				if (si->Volume > si->DestVolume) si->Volume --;
				if (si->DestVolume == si->Volume)
					{ si->RampTimer = si->RampTime; break; }
				
				si->RampTimer += si->RampTime;
			}
			SetVolume( td, i, si->Volume );
		}
	}
	return ans;
}


//===========================================================================
char __far StarTitle[] = "StarTask";


void __saveds StarTask( void )
{
	int					i;
	struct TData		*td = NULL;
	struct MsgPort		*dport = NULL;
	struct StarMsg		*msg = NULL;
	WORD				starsOn = 0;
	WORD				starres=3,starcnt=0;
	WORD				nowait = 0;
	struct SmSound		**ss;

	if (!(dport = CreateMsgPort())) goto xit;

	dport->mp_Node.ln_Name = StarTitle;
	AddPort( dport );

	// Get the Startup Message.
	// 
	while (1 )
	{
		while (msg = (struct StarMsg *)GetMsg( dport ))
		{
			if (msg->Msg.mn_Length == ST_STARTUP)  goto startup;
			msg->arg1 = TERR_DATA;
			ReplyMsg( msg );
			msg = NULL;
		}
		Wait( 1 << dport->mp_SigBit );
	}	

startup:
	td = msg->td;
			
	NewList( &td->freeList );
	NewList( &td->activeList );
				
	for(i=0;i<MAX_STARS;i++) 
		AddHead( &td->freeList, (struct Node *)&td->starArr[i] );
			
	td->DestStars = td->MaxStars = 16;
	td->Warp = 4;

	// Initize audio
	if (msg->arg1 = InitAudio(td)) goto xit;

	
	// Decompress instruments!
	for(i=0,ss = td->Instr;i < INSTR_MAX;i++,ss++)
	{
		if (!(*ss = GetSmSound( td, InstrArr[i] )))
		{
			msg->arg1 = td->Error;
			goto xit;
		}
		(*ss)->Lock++;
	}

	// RETURN STARTUP MESSAGE!
	ReplyMsg( msg );
	msg = NULL;
	
	MemSet( td->Sound, 0, (sizeof(struct SoundInfo) * 2));
	td->Sound[0].DestVolume = td->Sound[1].DestVolume = 63;
	td->Sound[0].Volume = td->Sound[1].Volume = 63;
	td->Sound[0].RampTime = td->Sound[1].RampTime = 1000000/10;

	td->SoundPC = NULL;
	
	// Ready to start

	while( 1 )
	{
		while( msg = (struct StarMsg *)GetMsg( dport ) )
		{
			switch( msg->Msg.mn_Length)
			{
				case ST_SHUTDOWN: 
					goto xit;
				case ST_ONESOUND:// 0:ss 1:times 2:ch 3:volumn 4:period
					msg->arg1 = playSound( td, (struct SmSound *)msg->arglist[0],
								msg->arglist[1],msg->arglist[2],
								msg->arglist[3],msg->arglist[4] );
					break;
				case ST_SETDESTVOL:
//					setDestVolume( td, msg->arglist[0], msg->arglist[1] );
					break;
				case ST_STARCOUNT:
					msg->arg1 = (starsOn ? td->starCount:0);
					break;
				case ST_SETSTARS:
					td->DestStars = msg->arg1;
					break;
				case ST_STARS_ONOFF:
					starsOn = msg->arg1;
					td->lastStarTime = td->CountUp;
					td->starSeed = td->CountUp;
					break;
				case ST_SET3PLANE:
					if ((msg->arg1 && (td->Flags & TD_3PLANEON)) ||
						(!msg->arg1 && !(td->Flags & TD_3PLANEON)))
						break;
				
					if (msg->arg1)
					{
						FON( td->Flags, TD_3PLANEON );
						if (starsOn) doStars( td,0 );
						td->Ri.BitMap = td->Bm3;
					}
					else 
					{
						FOFF( td->Flags, TD_3PLANEON );
						if (starsOn) doStars( td,0 );
						td->Ri.BitMap = td->Bm;
					}
					MakeVPort( &td->View,&td->Vp );
					MrgCop( &td->View );

					ReplyMsg(msg);msg = NULL;
					if (starsOn) starsOff( td );
					break;
			}
			if (msg) {ReplyMsg(msg); msg = NULL; }
		}
		if (starsOn)
		{
			starcnt--;
			if (starcnt <= 0)
			{
				if (td->DestStars > td->MaxStars) td->MaxStars += starres;
				if (td->MaxStars > td->DestStars) td->MaxStars = td->DestStars;
				starcnt = starres;
				doStars( td,1 );
			}
			nowait = 1;
		}
		if (DoSound(td)) nowait = 1;
		
		
		if (nowait)
		{
			nowait = 0;
			WaitTOF();
		}
		else  
			Wait( 1 << dport->mp_SigBit );
	}
xit:
	FreeAudio(td);

	for(i=0,ss = td->Instr;i < INSTR_MAX;i++,ss++)
	{
		if (*ss) FreeMem( *ss, (*ss)->FreeSize );
		*ss = NULL;
	}

	Forbid();

	if (td) td->SPort = NULL;
	
	if (msg) 
	{
		msg->arg1 = 0;
		ReplyMsg( msg );
	}
	if (dport)
	{
		while( msg = (struct StarMsg *)GetMsg( dport ))
		{
			msg->arg1 = 0;
			ReplyMsg( msg );
		}

        RemPort(dport);                                                                       /* Remove and delete MsgPort  */
        DeleteMsgPort(dport);
	}
	Permit();                                                                                   /* Let 'er rip                */
	RemTask(NULL); /* Remove ourselves           */
	Wait(0);
}
//----------------------------------------------------------------------------

static void waitSMsg( register struct TData *td )
{
	while( td->SMsg.Msg.mn_Node.ln_Succ ) 
	{
		if (GetMsg( td->SReply )) 
		{
			td->SMsg.Msg.mn_Node.ln_Succ = NULL;
			break;
		}
		Wait( 1 << td->SReply->mp_SigBit );
	}
}

LONG SendSMsg( struct TData *td, LONG msg, LONG arg1, ... )
{
	register struct StarMsg *smsg = &td->SMsg;

	if (!td->SPort) 
		if (InitStarSystem( td )) return 0;

	waitSMsg( td );

	smsg->Msg.mn_Length = msg;
	smsg->arg1 = arg1;
	smsg->arglist = &arg1;	// For SMSG_WAIT types only!
	
	PutMsg( td->SPort,smsg );
	
	if (msg & SMSG_WAIT)
	{
		waitSMsg( td );
		return smsg->arg1;
	}
	return 0;
}
//--------------------------------------------------------------------------

void FreeStarSystem( register struct TData *td )
{
	if (td->SPort)
	{
		SendSMsg( td, ST_SHUTDOWN, 0 );
		// SPort is NULL by other program!
	}
	
	if (td->SReply) DeleteMsgPort( td->SReply );
	td->SReply = NULL;
}

InitStarSystem( register struct TData *td )
{
	int i;

	if (!(td->SReply = CreateMsgPort())) TERR(TERR_MEMORY);

	if (!CreateTask( StarTitle,1,StarTask,8192 )) TERR( TERR_MEMORY );

	td->SMsg.Msg.mn_Node.ln_Succ = NULL;
	td->SMsg.Msg.mn_ReplyPort = td->SReply;
	td->SMsg.td = td;		// Give other task access to our data.
	
	for(i=0;i<30;i++)
	{
		if (td->SPort = FindPort( StarTitle )) break;
	}
	if (!td->SPort) TERR(TERR_MEMORY);
	
	SendSMsg( td,ST_STARTUP,0 );
	if (!td->SPort) TERR(TERR_MEMORY);
	
	return 0;
}
