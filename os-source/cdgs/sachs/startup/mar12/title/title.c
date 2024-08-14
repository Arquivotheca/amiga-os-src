/* :ts=4
*
*	title.c
*
*	William A. Ware						D303
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/


#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <proto/exec.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <graphics/sprite.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <utility/tagitem.h>

#include <cdtv/debox.h>
#include <cdtv/deboxproto.h>


#include "color.h"
#include "spriteanim.h"
#include "title.h"
#include "animation.h"

// SPOOF THE PRAGMAS
//
#define	GfxBase		(td->GfxBase)
#define DeBoxBase	(td->DeBoxBase)


#define CDY_START	210
#define CDY_END		430

#define AMIGA_SPRITEBANK		48
#define CD_SPRITEBANKODD		16
#define CD_SPRITEBANKEVEN		0



/*----------------------------------------------------------------------*/
#define	SData( name,num )  ((UWORD *)(&((UBYTE *)&SAnim)[ SAnim.name[ num ] ])))

void SetSpriteGroup(struct TData *td, struct SpriteInfo *si, WORD count,
					WORD xloc, WORD yloc );

/*----------------------------------------------------------------------*/



/*------------------------------------------------------------------------*/
BOOL SetFadeTo( register struct TData *td, LONG lv )
{
	td->FadeTo = lv;
	td->LastFadeTime = td->CountUp;

	return TRUE;
}

BOOL SetFadeStep( register struct TData *td, LONG sixtieth )
{
	td->FadeStep = 0x100*sixtieth*60/td->FPS;

	return TRUE;
}


Fade( register struct TData *td )
{
	LONG			ct,cu = td->CountUp;
	register LONG	fl = td->FadeLv;
	WORD			temp;


	ct = cu - td->LastFadeTime;
	td->LastFadeTime = cu;

	if (RunCNodes( td,&td->CnList,td->Table,ct*td->MPF))
		FON(td->Flags,TD_TCHANGED);
	
	
	for(;(td->FadeTo != fl) && (--ct >= 0);)
	{
		if (td->FadeTo > fl)
		{
			fl += td->FadeStep;
			if (td->FadeTo < fl) fl = td->FadeTo;
		}
		if (td->FadeTo < fl)
		{
			fl -= td->FadeStep;
			if (td->FadeTo > fl) fl = td->FadeTo;
		}
	}
	if (fl > 0xffff) fl = 0xffff;
	if (fl < 0) fl = 0;

	if (fl != td->FadeLv)
	{
		FON(td->Flags,TD_TCHANGED);
		td->FadeLv = fl;
	}


	return (((td->FadeLv != td->FadeTo) || td->CnList.mlh_Head->mln_Succ) ? 1:0);
}


void LoadTitleRGB32( register struct TData *td )
{
	if (TD_TCHANGED)
	{
		if (td->FadeLv)
		{
			FadeRGB32( td,td->Table, td->FadeTable, td->FadeLv );
			LoadRGB32( &td->Vp,td->FadeTable );
		}
		else
			LoadRGB32( &td->Vp, td->Table );

		FOFF( td->Flags, TD_TCHANGED );
	}
}




/*------------------------------------------------------------------------*/

InitBackground(register struct TData *td )
{
	struct BMInfo	*bmi;

	if (td->Flags & TD_SCREENUP) return 0;
	

	if (!(td->Bmi = DecompBMInfo( NULL,NULL,back ))) TERR( TERR_DATA );
	bmi = td->Bmi;

	if (!(td->PalBmi = DecompBMInfo( NULL,NULL,pal1))) TERR( TERR_DATA );


	if (!(td->Bm = AllocBitMap( bmi->bmi_Width,bmi->bmi_Height,bmi->bmi_Depth,
							BMF_DISPLAYABLE, NULL )))
		TERR( TERR_MEMORY );

	
	if (DecompBitMap( NULL, back, td->Bm, NULL )) TERR(TERR_DATA);

	if (!(td->Table = GetBMInfoRGB32( bmi,bmi->bmi_ColorCount, 0 )))
		TERR( TERR_MEMORY );
	if (!(td->FadeTable = GetBMInfoRGB32( NULL,bmi->bmi_ColorCount, 0 )))
		TERR( TERR_MEMORY );
	
	td->FadeLv = td->FadeTo = 0xffff;
	SetFadeStep( td,16 );

	//
	// Set up the VIEW
	//
	InitVPort( &td->Vp );
	td->Vp.DxOffset = -16;
	td->Vp.DyOffset = -62;
	td->Vp.DWidth	= 352;
	td->Vp.DHeight 	= (td->Pal ? 592:480);
	td->Vp.Modes	= bmi->bmi_Modes | SPRITES;
	td->Vp.RasInfo	= &td->Ri;
	
	td->Vp.ColorMap = GetColorMap( (1<<8) );		// Hardwire 8 bit planes.
	
	td->Ri.BitMap	= td->Bm;
	td->Ri.RxOffset = 0;
	td->Ri.RyOffset = (td->Pal ? 0:56);
	
	InitView( &td->View );	// Use defaults
	td->View.Modes	= bmi->bmi_Modes | LACE;
	td->View.ViewPort = &td->Vp;

	td->Sx = td->View.DxOffset + td->Vp.DxOffset - td->Ri.RxOffset;
	td->Sy = td->View.DyOffset + td->Vp.DyOffset + 4;


	VideoControlTags( td->Vp.ColorMap,
						VTAG_SPODD_BASE_SET, AMIGA_SPRITEBANK,
						VTAG_SPEVEN_BASE_SET, AMIGA_SPRITEBANK,
						VTAG_BORDERSPRITE_SET, 1L,
						VTAG_SPRITERESN_SET,SPRITERESN_70NS,
						VTAG_FULLPALETTE_SET,1L, 
						0L );

	td->UPtr = CINIT( &td->UCop, 50 );
	if (!td->UPtr) TERR(TERR_MEMORY);
	
	CWAIT( &td->UCop, CDY_START-2,0 );
	CMOVE( &td->UCop, custom.bplcon4,( (CD_SPRITEBANKODD>>4)+(CD_SPRITEBANKEVEN)) );
	CEND ( &td->UCop );
	td->Vp.UCopIns = &td->UCop;



	if (!CutCNode( td,&td->CNode[ CNODE_WAVE ],td->Bmi,td->Table,64,159 ))
		TERR( TERR_MEMORY );
	if (!CutCNode( td,&td->CNode[ CNODE_AMIGA],td->Bmi,td->Table,160,223 ))
		TERR( TERR_MEMORY );
	if (!CutCNode( td,&td->CNode[ CNODE_NUMBER],td->Bmi,td->Table,32,47 ))
		TERR( TERR_MEMORY );



	MakeVPort( &td->View,&td->Vp );
	MrgCop( &td->View );

	SetFadeTo( td, 0xffff );
	LoadRGB32( &td->Vp, td->Table );

	FON( td->Flags, TD_SCREENUP );

	return 0;
}

void FreeBackground( struct TData *td )
{
	if (td->OldView)
	{
		LoadView( td->OldView );
		WaitTOF();
	}
	if (td->UPtr)
	{
		td->Vp.UCopIns = NULL;
		FreeCopList( td->UCop.FirstCopList );
		MemSet( &td->UCop, 0,sizeof(struct UCopList));
	}
	if (td->Table) 		FreeBMInfoRGB32( td->Table );
	if (td->FadeTable) 	FreeBMInfoRGB32( td->FadeTable );
	if (td->Bm3)		FreeBitMap( td->Bm3 );
	if (td->Bm) 		FreeBitMap( td->Bm );
	if (td->Bmi) 		FreeBMInfo( td->Bmi );
	if (td->PalBmi)		FreeBMInfo( td->PalBmi );

	if (td->View.LOFCprList) FreeCprList( td->View.LOFCprList );
	if (td->View.SHFCprList) FreeCprList( td->View.SHFCprList );
	FreeVPortCopLists( &td->Vp );
	if (td->Vp.ColorMap) FreeColorMap( td->Vp.ColorMap );
	
	td->UPtr = NULL;
	td->FadeTable = td->Table = NULL;
	td->Bm = NULL;
	td->Bm3 = NULL;
	td->PalBmi = td->Bmi = NULL;
	
	InitView( &td->View );
	InitVPort( &td->Vp );

	FOFF( td->Flags, TD_SCREENUP );
}



/*------------------------------------------------------------------------*/

void SetSpriteGroup(struct TData *td, struct SpriteInfo *si, WORD count,
					WORD xloc, WORD yloc )
{
	register UBYTE *db,*db1;
	WORD			x,y,h;

	while( --count >= 0)
	{
		db  = &td->SData[0][ si->Offset ];
		db1 = &td->SData[1][ si->Offset ];
		x  = td->Sx + xloc + si->X;
		y  = td->Sy + yloc/2 + si->Y;
		h  = si->Height+y;

		db1[0] = db[0] = y;
		db1[1] = db[1] = x>>1;
		db1[8] = db1[2] = db[8] = db[2] = h;
		db1[9] = db1[3] = db[9] = db[3] =
			(si->Attached ? 0x80:0)+(y & 0x7f00 ? 4:0)+
			(h & 0x7f00 ? 2:0) + (x & 1);
		si++;
	}
}
/*------------------------------------------------------------------------*/
void SpriteDecomp( struct TData *td, UBYTE *data1,UBYTE *data2 )
{
	WORD temp;

	temp = td->DecompFrame ^ 1;
	WaitBOVP( &td->Vp );
	DecompDLTAX((temp ? data2:data1),0,&td->SpriteBm[temp],td->YTable );
	temp ^= 1;
	WaitBOVP( &td->Vp );
	DecompDLTAX((temp ? data2:data1),0,&td->SpriteBm[temp],td->YTable );
}
/*------------------------------------------------------------------------*/




void GetSpriteXY( UWORD *data, WORD *x, WORD *y )
{
	UBYTE *db = (UBYTE *)data;

	*y = db[0] + (db[3] & 4 ? 256:0);
	*x = (WORD)db[1] + (WORD)db[1] + (db[3] & 1);
}

InitSprites( register struct TData *td )
{
	struct SpriteInfo	*si;
	int i;

	if (td->Flags & TD_SPRITESUP) return 0;

	td->SData[0] = AllocMem( SAnim.SDataSize, (MEMF_CLEAR | MEMF_CHIP ));
	td->SData[1] = AllocMem( SAnim.SDataSize, (MEMF_CLEAR | MEMF_CHIP ));

	if (!td->SData[0] || !td->SData[1]) TERR( TERR_MEMORY );

	td->SpriteBm[0].BytesPerRow = td->SpriteBm[1].BytesPerRow	= 16;
	td->SpriteBm[0].Rows		= td->SpriteBm[1].Rows			= SAnim.SDataSize/16;
	td->SpriteBm[0].Depth		= td->SpriteBm[1].Depth		 	= 1;
	td->SpriteBm[0].Planes[0]	= (PLANEPTR)td->SData[0];
	td->SpriteBm[1].Planes[0]	= (PLANEPTR)td->SData[1];

	if (!(td->YTable = AllocYTable( 16,256 ))) TERR(TERR_MEMORY);


	for(i=0,si = SAnim.SData;i<8;i++,si++)
	{
		td->Sprite[i].es_SimpleSprite.posctldata =
			(UWORD *)( &td->SData[0][si->Offset] );

		td->Sprite[i].es_SimpleSprite.height = si->Height;
		td->Sprite[i].es_SimpleSprite.x	= si->X + 40;
		td->Sprite[i].es_SimpleSprite.y	= si->Y + 80;
		td->Sprite[i].es_wordwidth = 4;
		td->Sprite[i].es_flags = 0;

		td->SpriteData[i] = (UBYTE *)td->Sprite[i].es_SimpleSprite.posctldata;
		td->SpriteData[i+8] = &td->SData[1][si->Offset];


		if (i==0)
		{
			td->Sprite[i].es_SimpleSprite.num =
				GetExtSprite( &td->Sprite[i],GSTAG_SPRITE_NUM,1,0L);

			if (td->Sprite[i].es_SimpleSprite.num == 0xffff)
				TERR( TERR_SPRITE );

			FreeSprite(1);
			td->Sprite[i].es_SimpleSprite.num = 0;
		}
		else
		{
			td->Sprite[i].es_SimpleSprite.num =
				GetExtSprite( &td->Sprite[i],GSTAG_SPRITE_NUM,i,0L);
			if (td->Sprite[i].es_SimpleSprite.num == 0xffff)
				TERR( TERR_SPRITE );
		}
	}
	MakeVPort( &td->View,&td->Vp );
	MrgCop( &td->View );

	for(i=0;i<8;i++)
		MoveSprite( &td->Vp,&td->Sprite[i].es_SimpleSprite,0,0 );
		
	SetSpriteGroup( td, SAnim.Amiga,6,37,180 );
	SetSpriteGroup( td, SAnim.Number,2,245,305 );

//	SetSpriteGroup( td, SAnim.CD,CD_SECCOUNT,CDCoords[0]/2,
//					CDCoords[1]+td->Ri.RyOffset );

	td->CDSpeed = 1000000/20;

	FON( td->Flags,TD_SPRITESUP );
	
	return 0;
}


void FreeSprites( register struct TData *td )
{
	int i;

	FOFF( td->Flags,TD_SPRITESUP );

	for(i=0;i<8;i++)
	{
		if (td->Sprite[i].es_SimpleSprite.num != 0xffff)
			FreeSprite( td->Sprite[i].es_SimpleSprite.num );
		td->Sprite[i].es_SimpleSprite.num = 0xffff;
	}
	WaitTOF();
	MakeVPort( &td->View, &td->Vp );
	MrgCop( &td->View );

	if (td->YTable) FreeYTable( td->YTable,256 );
	if (td->SData[0]) FreeMem( td->SData[0],SAnim.SDataSize );
	if (td->SData[1]) FreeMem( td->SData[1],SAnim.SDataSize );
	td->YTable = NULL;
	td->SData[0] = td->SData[1] = NULL;
}

/*------------------------------------------------------------------------*/

static BOOL cmdFadeLv( register struct TData *td, LONG lv )
{
	td->FadeTo = td->FadeLv = lv;
	td->LastFadeTime = td->CountUp;
	FON(td->Flags,TD_TCHANGED);
	return TRUE;
}

static BOOL cmdWait( register struct TData *td, LONG micro )
{
	td->NextWait = micro;
	return TRUE;
}

static BOOL cmdEnd( register struct TData *td, LONG micro )
{
	td->PC = NULL;
	td->NextWait = 0;
	return TRUE;

}

static BOOL cmdStartCNode( register struct TData *td,
							LONG frmto, WORD type, UBYTE sn )
{
	WORD			*wptr = (WORD *)&frmto;
	struct CNode	*cn;

	cn = &td->CNode[sn];
	StartCNode( td,&td->CnList,cn,wptr[0],wptr[1],td->MPF,0,type );

	return TRUE;
}

static BOOL cmdLinkCNode( register struct TData *td,
							LONG linkto, WORD type,UBYTE sn )
{
	struct CNode	*fcn,*cn;

	cn = &td->CNode[sn];
	fcn= &td->CNode[linkto];

	StartCNode( td,(fcn->Node.mln_Succ ? NULL:&td->CnList),
				cn,fcn->From,fcn->To,td->MPF,0,type);

	while (fcn->NextCNode != NULL)
		fcn = fcn->NextCNode ;

	cn->NextCNode = NULL;
	fcn->NextCNode = cn;

	return TRUE;
}


static BOOL cmdSetCNode( register struct TData *td,
						 LONG frmto,WORD from,UBYTE sn )
{
	WORD			*wptr = (WORD *)&frmto;
	struct CNode	*cn;


	cn = &td->CNode[sn];

	if (!CutCNode( td,cn,td->PalBmi,NULL,wptr[0],wptr[1] ))
	{
		td->Error = TERR_MEMORY;
		return FALSE;
	}
	return TRUE;
}




static BOOL cmdAmigaOnOff( register struct TData *td, LONG pos )
{

	if ((pos && (td->Flags & TD_AMIGAON)) || (!pos && !(td->Flags & TD_AMIGAON)))
		return TRUE;

	if (pos) 	FON( td->Flags, TD_AMIGAON);
	else		FOFF(td->Flags, TD_AMIGAON);


	SpriteDecomp( td,&((UBYTE *)&SAnim)[SAnim.AmigaOnOff[0]],
					 &((UBYTE *)&SAnim)[SAnim.AmigaOnOff[1]] );
	return TRUE;
}



static BOOL cmdNumberOnOff( register struct TData *td, LONG pos )
{

	if ((pos && (td->Flags & TD_NUMBERON)) || (!pos && !(td->Flags & TD_NUMBERON)))
		return TRUE;

	if (pos) 	FON( td->Flags, TD_NUMBERON);
	else		FOFF(td->Flags, TD_NUMBERON);


//	SpriteDecomp( td,&((UBYTE *)&SAnim)[SAnim.NumberOnOff[0]],
//					 &((UBYTE *)&SAnim)[SAnim.NumberOnOff[1]] );
//
	return TRUE;
}




static BOOL cmdCNodeWait( register struct TData *td, LONG temp,WORD bottom,UBYTE sn )
{
	struct CNode	*cn;

	if (sn == CNODE_ALL)
	{
		return (BOOL)(td->CnList.mlh_Head->mln_Succ ? FALSE:TRUE);
	}
	
	cn = &td->CNode[sn];

	if ((cn->Flags & CNF_FINISHED) || (cn->Bottom >= bottom)) return TRUE;

	return FALSE;
}


static BOOL cmdSetCDState( register struct TData *td, LONG state )
{
	td->NextCDState = state;
	return TRUE;
}


static BOOL cmdWaitCDState( register struct TData *td, LONG state )
{
	if (td->CDState != td->NextCDState) return FALSE;
	return TRUE;
}



static BOOL cmdCNodeSwitch( register struct TData *td, 
							LONG temp,WORD bottom,UBYTE sn )
{
	struct CNode	*cn;

	cn = &td->CNode[sn];

	SwitchCNode( td,cn );

	return TRUE;
}

static BOOL cmdWaitFade( register struct TData *td, LONG state ) 
{
	if (td->FadeLv != td->FadeTo) return FALSE;
	return TRUE;
}

static BOOL cmd3PlaneOnOff( register struct TData *td, LONG state ) 
{
	struct BitMap	*numbm;
	struct BitMap	tbm;

	if (state)
	{
		if (BackDecompBitMap( td, back3plane,&td->Bm3 )) return TRUE;
		td->Ri.BitMap = td->Bm3;
	
		if (((td->Flags & TD_NUMBERON) && !(td->Flags & TD_3PL_NUMON)) ||
			(!(td->Flags & TD_NUMBERON) && (td->Flags & TD_3PL_NUMON)))
		{
			numbm = NULL;
			CopyMem( td->Bm3, &tbm, sizeof(struct BitMap));
			tbm.Depth = 1;
			tbm.Planes[0] = tbm.Planes[2];
		
			if (BackDecompBitMap( td, number,&numbm )) return TRUE; 
			BltBitMap( 	numbm, 0,0, &tbm, 246,305, 
						70,numbm->Rows,
						( td->Flags & TD_3PL_NUMON ? 0x00:0xc0),0xff,NULL );
			FreeBitMap( numbm );
			WaitBlit();
			td->Flags ^= TD_3PL_NUMON;
		}
	}
	else td->Ri.BitMap = td->Bm;
	
	
	MakeVPort( &td->View, &td->Vp );
	MrgCop( &td->View );
	return TRUE;
}


//=--------------------------------------
BOOL  (*cmdFuncs[])(...) =
{
	(BOOL (*)(...))cmdEnd,
	(BOOL (*)(...))cmdFadeLv,
	(BOOL (*)(...))SetFadeTo,
	(BOOL (*)(...))cmdWait,
	(BOOL (*)(...))cmdStartCNode,
	(BOOL (*)(...))cmdLinkCNode,
	(BOOL (*)(...))cmdSetCNode,
	(BOOL (*)(...))cmdAmigaOnOff,
	(BOOL (*)(...))cmdCNodeWait,
	(BOOL (*)(...))cmdSetCDState,
	(BOOL (*)(...))cmdWaitCDState,
	(BOOL (*)(...))cmdCNodeSwitch,
	(BOOL (*)(...))cmdWaitFade,
	(BOOL (*)(...))cmd3PlaneOnOff,
	(BOOL (*)(...))cmdNumberOnOff,
	NULL
};

ExecCommands( register struct TData *td )
{
	LONG				ct = td->CountUp;
	struct Command		*cm;

	if (!td->PC) return FALSE;
	if (td->NextWait > 0)
	{
		if (ct - td->LastUpdate > 0)
			td->NextWait -= (ct - td->LastUpdate) * td->MPF;
 	}
	while (td->NextWait <= 0)
	{
		cm = td->PC;
		if (!cmdFuncs[ cm->Command ]( td, cm->ArgL,cm->ArgW,cm->ArgB )) break;
		if (!td->PC) break;
		td->PC++;
	}
	td->LastUpdate = ct;
	return TRUE;
}

/*------------------------------------------------------------------------*/


struct Command	StartupCmd[] =
{
	{ CMD_FADELV,		0,0,0xffff },
	{ CMD_FADETO,		0,0,0x0000 },
	{ CMD_STARTCNODE,	CNODE_WAVE,CNTYPE_MOVEUP,(64<<16)+159 },
	{ CMD_CNODEWAIT,	CNODE_WAVE,16,0 },
	{ CMD_STARTCNODE,	CNODE_AMIGA,CNTYPE_MOVEUP,(160<<16)+223 },
	{ CMD_SETCNODE,		CNODE_TEMP1,0,(0<<16)+63 },
	{ CMD_LINKCNODE,	CNODE_TEMP1,CNTYPE_ONECOLOR, CNODE_AMIGA },

	
	{ CMD_CNODEWAIT,	CNODE_TEMP1,32,0 },
	{ CMD_SETCNODE,		CNODE_TEMP2,0,(64<<16)+95 },
	{ CMD_AMIGAONOFF,	0,0,1 },
	{ CMD_STARTCNODE,	CNODE_TEMP2,CNTYPE_SPRITECOLOR,(AMIGA_SPRITEBANK+2<<16)+0 },

	{ CMD_CNODEWAIT,	CNODE_ALL,0,0 },

	{ CMD_3PLANE_ONOFF,	0,0,1 },
	
	{ CMD_SETCDSTATE,   0,0,CDSTATE_START },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_WAIT },
	{ CMD_WAITCDSTATE,  0,0,0 },

	{ CMD_3PLANE_ONOFF,	0,0,0 },
	{ CMD_SETCNODE,		CNODE_TEMP3,0,(96<<16)+127 },
	{ CMD_STARTCNODE,	CNODE_TEMP3,CNTYPE_MOVEUP,(32<<16)+47 },
	{ CMD_SETCNODE,		CNODE_TEMP4,0,(128<<16)+159 },
	{ CMD_STARTCNODE,	CNODE_TEMP4,CNTYPE_ONECOLOR,(16<<16)+16 },
	
	{ CMD_CNODEWAIT,	CNODE_TEMP3,18,0 },
	{ CMD_CNODESWITCH,	CNODE_TEMP3,0,0 },
	{ CMD_NUMBER_ONOFF, 0,0,1 },
	
	{ CMD_CNODEWAIT,	CNODE_ALL,0,0 },

	{ CMD_END,0,0,0 }
};

void UpScreen( register struct TData *td )
{
	// Bring Up the Screen..
	//
	
	InitBackground(td);
	InitSprites( td );
	LoadView( &td->View );

	td->Bank = 0;

	td->PC = StartupCmd;
	td->LastUpdate = td->CountUp;

	PlaySmSound( td, td->Sample,1,0x3,63,0 );

	while( ExecCommands( td ) )
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
}


struct Command	FinalCmd[] =
{
	{ CMD_3PLANE_ONOFF,	0,0,1 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_LEAVE },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_STARTWAIT },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_FADETO,		0,0,0xffff },
	{ CMD_WAITFADE,		0,0,0 },

	{ CMD_CNODEWAIT,	CNODE_TEMP3,47,0 },
	{ CMD_CNODEWAIT,	CNODE_TEMP3,159,0 },


	{ CMD_END,0,0,0 }
};



void EndScreen( register struct TData *td )
{

	td->PC = FinalCmd;
	td->LastUpdate = td->CountUp;


	while( ExecCommands( td ) )
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
}


struct Command	SpinCDCmd[] =
{
	{ CMD_3PLANE_ONOFF,	0,0,1 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_ROTATE },
	{ CMD_END,0,0,0 }
};



void SpinCD( register struct TData *td )
{
	td->PC = SpinCDCmd;
	td->LastUpdate = td->CountUp;

	while( ExecCommands( td ) )
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
}


struct Command	StopCDCmd[] =
{
	{ CMD_3PLANE_ONOFF,	0,0,0 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_WAIT },
	{ CMD_END,0,0,0 }
};



void StopCD( register struct TData *td )
{
	td->PC = StopCDCmd;
	td->LastUpdate = td->CountUp;

	while( ExecCommands( td ) )
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
}




/*------------------------------------------------------------------------*/



void __saveds Title(void)
{
	int						i;
	register struct TData	*td;
	struct Message			*msg = NULL;
	
	td = AllocMem( sizeof(struct TData),(MEMF_REVERSE | MEMF_PUBLIC | MEMF_CLEAR));
	if (!td)
	{
		// If this cant allocate we are in serious trouble!!!
	 	RemTask(NULL);                                                                              /* Remove ourselves           */
		Wait(0);
	}

	// INIT the Basic TData.
#undef GfxBase
	if (!(td->GfxBase = (struct GfxBase *)OpenLibrary( "graphics.library",39L )))
		{ td->Error = TERR_GFXBASE; goto xit; }
#define	GfxBase		(td->GfxBase)

	if (!(DeBoxBase = OpenLibrary( "debox.library",39L )))
		{ td->Error = TERR_DEBOXBASE; goto xit; }

	td->Bank = -1;
	if (!(td->VBlank = AddVBlank(td))) 
		{ td->Error = TERR_MEMORY;goto xit; }


	// Last stop is to put up the message port.
	//
	if (!(td->MsgPort = CreateMsgPort()))
		{ td->Error = TERR_MEMORY; goto xit; }

	td->MsgPort->mp_Node.ln_Name = "Startup Animation";
	AddPort( td->MsgPort );

	// Now Initilized the Inital data in TData (No allocation in this section.
	//
	td->SAnim = &SAnim; 	// For Debugger Purposes...Do not use.

	for(i=0;i<8;i++) td->Sprite[i].es_SimpleSprite.num = 0xffff;

	td->FPS = ((struct ExecBase *)SysBase)->VBlankFrequency;
	td->MPF = 1000000/td->FPS;

	NewList( (struct List *)&td->CnList );

	td->OldView = GfxBase->ActiView;


	InitAudio(td);
	
	if (!(td->Sample = AllocMem( sample[0].ci_Size,(MEMF_REVERSE | MEMF_CHIP) )))
		{ td->Error = TERR_MEMORY; goto xit; }
	if (DecompData( sample,NULL,td->Sample) != sample[0].ci_Size)
		{ td->Error = TERR_MEMORY; goto xit; }
	
	td->Sample->Lock++;
	

	//************ td = Ready for action!!! ******************************
	UpScreen(td);	// Start the Screen.

#ifndef NONTASK 
	while( 1 )
	{
		while( msg = GetMsg(td->MsgPort ) )
		{
			switch (msg->mn_Length)
			{			
				case ANIMMSG_BOOTING:
					SpinCD(td);
					break;
				case ANIMMSG_FALSEALARM:
					StopCD(td);
					break;
				case ANIMMSG_SHUTDOWN: 
					EndScreen( td );
					goto xit;
				default:
					break;
			}
			if (msg) { ReplyMsg( msg );msg= NULL; }
		}
		WaitTOF();

		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
	}
#else
	for(i=0;i<60;i++) 
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
	SpinCD(td);
	for(i=0;i<60;i++)
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
	EndScreen( td );
#endif

xit:
	Forbid();

	FreeAudio(td);

	if (td->Sample) FreeMem( td->Sample,sample[0].ci_Size );

	// Free any unfreed messages
	// then free the port.
	//
	if (msg) ReplyMsg( msg );
	if (td->MsgPort)
	{
		while( msg = GetMsg( td->MsgPort ))
			ReplyMsg( msg );

        RemPort(td->MsgPort);                                                                       /* Remove and delete MsgPort  */
        DeleteMsgPort(td->MsgPort);
	}

	if (td->VBlank) FreeVBlank(td->VBlank);

	// Down the Sections of 'td'
	FreeSprites( td );
	FreeBackground( td );


	for(i=0;i<10;i++)
		FreeCNodeData( td,&td->CNode[i] );



	if (td->OldView) LoadView( td->OldView );
	if (DeBoxBase) CloseLibrary( DeBoxBase );
	if (GfxBase) CloseLibrary( GfxBase );

	FreeMem( td, sizeof(struct TData));

	Permit();                                                                                   /* Let 'er rip                */
#ifndef NONTASK
 	RemTask(NULL); /* Remove ourselves           */
	Wait(0);
#endif
}


/*------------------------------------------------------------------------*/
