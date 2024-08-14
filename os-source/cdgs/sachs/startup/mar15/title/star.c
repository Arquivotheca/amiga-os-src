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

#include "title.h"

#define XRND_SIZE	307
#define YRND_SIZE	444

#define WIDTH		382
#define HEIGHT		592
#define HWIDTH		(WIDTH/2)
#define HHEIGHT		(HEIGHT/2)

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
char __far StarTitle[] = "StarTask";


void __saveds StarTask( void )
{
	int					i;
	struct TData		*td = NULL;
	struct MsgPort		*dport = NULL;
	struct StarMsg		*msg = NULL;
	WORD				starsOn = 0;
	WORD				starres=3,starcnt=0;
	WORD				cd = 60;

	if (!(dport = CreateMsgPort())) goto xit;

	dport->mp_Node.ln_Name = StarTitle;
	AddPort( dport );

	while( 1 )
	{
		while( msg = (struct StarMsg *)GetMsg( dport ) )
		{
			if (!td) 
			{
				// First time Initizize Varables.
				td = msg->td;
			
				NewList( &td->freeList );
				NewList( &td->activeList );
				
				for(i=0;i<MAX_STARS;i++) 
					AddHead( &td->freeList, (struct Node *)&td->starArr[i] );
			
				td->DestStars = td->MaxStars = 16;
				td->Warp = 4;
			}
			switch( msg->Msg.mn_Length)
			{
				case ST_SHUTDOWN: 
					goto xit;
				case ST_STARTUP: break; // Do nothing for now.
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
		starcnt--;
		if (starsOn && (starcnt <= 0))
		{
			if (td->DestStars > td->MaxStars) td->MaxStars += starres;
			if (td->MaxStars > td->DestStars) td->MaxStars = td->DestStars;
			starcnt = starres;
			doStars( td,1 );
		}
		WaitTOF();		
	}
xit:
	Forbid();
	
	if (td) td->SPort = NULL;
	
	if (msg) 
	{
		msg->Msg.mn_Length = 0;
		ReplyMsg( msg );
	}
	if (dport)
	{
		while( msg = (struct StarMsg *)GetMsg( dport ))
		{
			msg->Msg.mn_Length = 0;
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
		return smsg->Msg.mn_Length;	// Answer;
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

	if (!CreateTask( StarTitle,6,StarTask,4096 )) TERR( TERR_MEMORY );

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
