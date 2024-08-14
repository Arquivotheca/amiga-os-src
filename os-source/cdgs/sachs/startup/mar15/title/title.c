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

#define CD_ALTBANKEVEN			32

#define REGULAR_FADESTEP	16

#define PLACE_NUMBERX		234
#define PLACE_NUMBERY		302
#define NUMBER_WIDTH		80
#define	NUMBER_HEIGHT		52

#define STARS_WAVELV		32

/*----------------------------------------------------------------------*/
#define	SData( name,num )  ((UWORD *)(&((UBYTE *)&SAnim)[ SAnim.name[ num ] ])))

void SetSpriteGroup(struct TData *td, struct SpriteInfo *si, WORD count,
					WORD xloc, WORD yloc );

/*----------------------------------------------------------------------*/

extern struct CompHeader __far	sound1[];
extern struct CompHeader __far	sound2[];

struct CompHeader	*SndArr[] =
{
	sound1,sound2
};


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
			FadeRGB32( td,td->Table, td->FadeTable, td->FadeLv,
						td->FadeFromColor, td->FadeToColor);
			LoadRGB32( &td->Vp,td->FadeTable );
		}
		else
			LoadRGB32( &td->Vp, td->Table );

		FOFF( td->Flags, TD_TCHANGED );
	}
}




/*------------------------------------------------------------------------*/

void CopClear( struct TData *td )
{
	if (td->UPtr)
	{
		td->Vp.UCopIns = NULL;
		FreeCopList( td->UCop.FirstCopList );
		MemSet( &td->UCop, 0,sizeof(struct UCopList));
	}
}

SetCopList( struct TData *td, WORD odd, WORD even )
{
	CopClear( td );
	td->UPtr = CINIT( &td->UCop, 50 );
	if (!td->UPtr) TERR(TERR_MEMORY);
	
	CWAIT( &td->UCop, CDY_START-2,0 );
	CMOVE( &td->UCop, custom.bplcon4,( (odd>>4)+(even)) );
	CEND ( &td->UCop );
	td->Vp.UCopIns = &td->UCop;
}


InitBackground(register struct TData *td )
{
	struct BMInfo	*bmi;

	if (td->Flags & TD_SCREENUP) return 0;
	

	// BMINFOS 
	//
	td->PalBmi		= DecompBMInfo( NULL,NULL,pal1 );
	bmi = td->Bmi	= DecompBMInfo( NULL,NULL,back );
	if (!bmi || !td->PalBmi ) TERR(TERR_DATA);


	// BITMAPS
	td->Bm	= AllocBitMap( bmi->bmi_Width,bmi->bmi_Height,bmi->bmi_Depth,
							BMF_DISPLAYABLE,NULL );
	td->RBm	= AllocBitMap( bmi->bmi_Width,bmi->bmi_Height,bmi->bmi_Depth,
							BMF_DISPLAYABLE,NULL );
	td->Bm3	= AllocBitMap( bmi->bmi_Width,bmi->bmi_Height,3,BMF_DISPLAYABLE,NULL );
	td->RBm3= AllocBitMap( bmi->bmi_Width,bmi->bmi_Height,3,BMF_DISPLAYABLE,NULL );

	if (!td->Bm || !td->RBm || !td->Bm3 || !td->RBm3) TERR( TERR_MEMORY );

	// DECOMPRESS THE BIT MAPS
	//
//	if (DecompBitMap( NULL, back, td->Bm, NULL )) TERR(TERR_DATA);
//	BltBitMap( td->Bm, 0,0, td->RBm, 0,0, bmi->bmi_Width,bmi->bmi_Height,
//				0xc0,0xff,NULL );
	
	if (DecompBitMap( NULL, back3plane, td->Bm3, NULL )) TERR( TERR_DATA );
	BltBitMap( td->Bm3, 0,0, td->RBm3, 0,0, bmi->bmi_Width,bmi->bmi_Height,
				0xc0,0xff,NULL );

	td->Bm3->Planes[3] = td->RBm3->Planes[3] = NULL;

	// Color Tables
	//
	//
	td->Table = GetBMInfoRGB32( bmi,bmi->bmi_ColorCount, 0 );
	td->FadeTable = GetBMInfoRGB32( NULL, bmi->bmi_ColorCount, 0 );

	if (!td->Table || !td->FadeTable) TERR( TERR_MEMORY );

	td->FadeLv = td->FadeTo = 0xffff;
	SetFadeStep( td,REGULAR_FADESTEP );

	if (!(td->BYTable = AllocYTable( td->Bm->BytesPerRow,bmi->bmi_Height ))) 
		TERR(TERR_MEMORY);
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

	SetCopList( td, CD_SPRITEBANKODD, CD_SPRITEBANKEVEN );
	
	
	// Cut the already existing colors out of the color palette for later use.
	//
	if (!CutCNode( td,&td->CNode[ CNODE_WAVE ],td->Bmi,td->Table,72,159 ))
		TERR( TERR_MEMORY );
	if (!CutCNode( td,&td->CNode[ CNODE_AMIGA],td->Bmi,td->Table,160,196 ))
		TERR( TERR_MEMORY );
	if (!CutCNode( td,&td->CNode[ CNODE_NUMBER],td->Bmi,td->Table,32,47 ))
		TERR( TERR_MEMORY );
	if (!CutCNode( td,&td->CNode[ CNODE_LOWWAVE],td->Bmi,td->Table,197,255 ))
		TERR( TERR_MEMORY );

	MakeVPort( &td->View,&td->Vp );
	MrgCop( &td->View );
	
	SetFadeTo( td, 0xffff );
	LoadRGB32( &td->Vp, td->FadeTable );

	FON( td->Flags, TD_SCREENUP );

	return 0;
}

/*------------------------------------------------------------------------*/



void FreeBackground( struct TData *td )
{
	if (td->OldView)
	{
		LoadView( td->OldView );
		WaitTOF();
	}
	CopClear( td );
	if (td->BYTable)	FreeYTable( td->YTable,td->Bmi->bmi_Height );
	if (td->Table) 		FreeBMInfoRGB32( td->Table );
	if (td->FadeTable) 	FreeBMInfoRGB32( td->FadeTable );
	if (td->RBm3)		FreeBitMap( td->RBm3 );
	if (td->RBm) 		FreeBitMap( td->RBm );
	if (td->Bm3)		FreeBitMap( td->Bm3 );
	if (td->Bm) 		FreeBitMap( td->Bm );
	if (td->Bmi) 		FreeBMInfo( td->Bmi );
	if (td->PalBmi) 	FreeBMInfo( td->PalBmi );

	if (td->View.LOFCprList) FreeCprList( td->View.LOFCprList );
	if (td->View.SHFCprList) FreeCprList( td->View.SHFCprList );
	FreeVPortCopLists( &td->Vp );
	if (td->Vp.ColorMap) FreeColorMap( td->Vp.ColorMap );

	td->BYTable = NULL;
	td->UPtr = NULL;
	td->FadeTable = td->Table = NULL;
	td->Bm	= td->RBm = td->Bm3 = td->RBm3 = NULL;
	td->PalBmi = td->Bmi = NULL;

	InitView( &td->View );
	InitVPort( &td->Vp );
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
		
	SetSpriteGroup( td, SAnim.Amiga,6,38,180 );
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
	struct BitMap *bm;

	if ((pos && (td->Flags & TD_NUMBERON)) || (!pos && !(td->Flags & TD_NUMBERON)))
		return TRUE;

	if (pos) 	FON( td->Flags, TD_NUMBERON);
	else		FOFF(td->Flags, TD_NUMBERON);

//	SpriteDecomp( td,&((UBYTE *)&SAnim)[SAnim.NumberOnOff[0]],
//					 &((UBYTE *)&SAnim)[SAnim.NumberOnOff[1]] );

	if (!(td->Flags & TD_3PL_NUMON))
	{
		bm = NULL;
		if (BackDecompBitMap( td,number,&bm )) return TRUE;
		BltBitMap( bm,0,0,td->Bm3,PLACE_NUMBERX,PLACE_NUMBERY, 
					NUMBER_WIDTH,NUMBER_HEIGHT,0xc0,0xff,NULL );
		BltBitMap( bm,0,0,td->RBm3,PLACE_NUMBERX,PLACE_NUMBERY, 
					NUMBER_WIDTH,NUMBER_HEIGHT,0xc0,0xff,NULL );
		FON( td->Flags,TD_3PL_NUMON );
		WaitBlit();
		FreeBitMap( bm );
	}

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
	if ((state && (td->Flags & TD_3PLANEON)) ||
		(!state && !(td->Flags & TD_3PLANEON)))
		return TRUE;
	
	SendSMsg( td, ST_SET3PLANE, state );
	
	return TRUE;
}



static BOOL cmdMark( register struct TData *td )
{
	return TRUE;
}

static BOOL cmdPlaySnd( register struct TData *td, LONG number )
{
	LONG			size = SndArr[number]->ci_Size;
	struct SmSound *ss;

	if (!(ss = AllocMem( size,(MEMF_REVERSE | MEMF_CHIP) )))
		{ td->Error = TERR_MEMORY; return TRUE; }
	
	if (DecompData( SndArr[number],NULL,ss) != size) 
	{ 
		FreeMem( ss,size );
		td->Error = TERR_DATA; 
		return TRUE; 
	}
	ss->FreeSize = size;
	
	PlaySmSound( td,ss,1,0x3,63,0 );
}

static BOOL cmdDecomp( register struct TData *td, LONG number )
{
	struct BMInfo	*bmi;

	switch( number )
	{
		case DECOMP_MAINBM:
			bmi = td->Bmi;
			if (DecompBitMap( NULL, back, td->Bm, NULL )) 
				{ td->Error = TERR_DATA; return TRUE; }
			BltBitMap( td->Bm, 0,0, td->RBm, 0,0, bmi->bmi_Width,bmi->bmi_Height,
						0xc0,0xff,NULL );
			WaitBlit();
			break;
	}
	return TRUE;
}

static BOOL cmdSetColor( register struct TData *td, LONG cn, WORD number )
{
	UBYTE *c = ((UBYTE *)&cn)+1;
	UBYTE *cptr = (UBYTE *)(&td->Table[ 1 + number * 3 ]);
	int		i;
	
	for(i=0;i<3;i++,c++)
	{
		*cptr++ = *c;
	}
	FON( td->Flags, TD_TCHANGED );
	return TRUE;
}

static BOOL cmdLoadView( register struct TData *td, LONG number )
{
	LoadTitleRGB32(td);
	LoadView( &td->View );

	return TRUE;
}

static BOOL cmdSetStars( register struct TData *td, LONG number )
{
	SendSMsg( td, ST_SETSTARS, number );
	return TRUE;
}

static BOOL cmdStarsOnOff( register struct TData *td, LONG number )
{
	SendSMsg( td, ST_STARS_ONOFF, number );
	return TRUE;
}

static BOOL cmdClearStars( register struct TData *td, LONG number )
{
	if (td->DestStars != 0) SendSMsg( td,ST_SETSTARS, 0 );

	if (!SendSMsg( td, ST_STARCOUNT, number )) return FALSE;
	return TRUE;
}

static BOOL cmdSetCNodeCycle( register struct TData *td, 
							LONG cyclewait,WORD state,UBYTE sn )
{
	struct CNode	*cn;

	cn = &td->CNode[sn];
	cn->CycleWait = cyclewait;
	if (state) 	FON( cn->Flags, CNF_CYCLE );
	else		FOFF( cn->Flags, CNF_CYCLE );
	return TRUE;
}


static BOOL cmdSetTableFromTo( register struct TData *td, 
							LONG fadelv,WORD to,UBYTE from )
{
	td->FadeFromColor 	= from;
	td->FadeToColor		= to;
	
	td->FadeTo = td->FadeLv = fadelv;
	td->LastFadeTime = td->CountUp;
	FON(td->Flags,TD_TCHANGED);
	return TRUE;
}
//=--------------------------------------
BOOL  (*cmdFuncs[])(...) =
{
	(BOOL (*)(...))cmdEnd,
	(BOOL (*)(...))cmdMark,
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
	(BOOL (*)(...))cmdPlaySnd,
	(BOOL (*)(...))cmdDecomp,
	(BOOL (*)(...))cmdSetColor,
	(BOOL (*)(...))cmdLoadView,
	(BOOL (*)(...))cmdSetStars,
	(BOOL (*)(...))cmdStarsOnOff,
	(BOOL (*)(...))cmdClearStars,
	(BOOL (*)(...))cmdSetCNodeCycle,
	(BOOL (*)(...))cmdSetTableFromTo,
	
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


void ExecLoop( struct TData *td, struct Command *clist )
{
	td->PC = clist;
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


struct Command	StartupCmd[] =
{
	// Initilize
	{ CMD_FADELV,		0,0,0xffff },
	{ CMD_SETCOLOR,		0,4,0 },
	{ CMD_3PLANE_ONOFF,	0,0,1 },
	{ CMD_SETSTARS,		0,0,64 },
	
	// Fade in.
	{ CMD_STARS_ONOFF,	0,0,1 },
	{ CMD_LOADVIEW,		0,0,0 },
	{ CMD_FADETO,		0,0,0x0000 },
	{ CMD_WAITFADE,		0,0,0 },
	
	// Start Inital sound.
	{ CMD_PLAYSND,		0,0,SND_TITLE1 },
	
	// Decompress Stuff while stars are going.
	{ CMD_DECOMP,		0,0,DECOMP_MAINBM },
	{ CMD_PLAYSND,		0,0,SND_TITLE2 },
	{ CMD_SETSTARS,		0,0,0 },

	// Change to 8 PLANE mode.
	{ CMD_3PLANE_ONOFF,	0,0,0 },
	// Start the Wave.
	{ CMD_WAIT,			0,0,1 },
	{ CMD_STARTCNODE,	CNODE_WAVE,CNTYPE_MOVEUP,(64<<16)+159 },
	{ CMD_CNODEWAIT,	CNODE_WAVE,16,0 },
	{ CMD_SETCOLOR,		0,4, (239<<16)+0+0 },
	{ CMD_STARTCNODE,	CNODE_AMIGA,CNTYPE_MOVEUP,(160<<16)+196 },
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
	
	{ CMD_END,0,0,0 }
};

struct Command WaitSetup[] =
{
	{ CMD_3PLANE_ONOFF,	0,0,0 },
	{ CMD_SETSTARS,		0,0,STARS_WAVELV },
	{ CMD_STARS_ONOFF,	0,0,1 },
	
	{ CMD_CNODEWAIT,	CNODE_ALL,0,0 },

	{ CMD_STARTCNODE,	CNODE_WAVE,CNTYPE_MOVEUP,(64<<16)+159 },
	{ CMD_STARTCNODE,	CNODE_LOWWAVE,CNTYPE_MOVEUP,(197<<16)+255 },
	{ CMD_SETCNODECYCLE,CNODE_WAVE,1,1000000 },
	{ CMD_SETCNODECYCLE,CNODE_LOWWAVE,1,1000000 },

	{ CMD_END,0,0,0 }
};

void UpScreen( register struct TData *td )
{
	// Bring Up the Screen..
	//
	
	InitBackground(td);
	InitSprites( td );


	LoadView( &td->View );

	ExecLoop( td,StartupCmd );
	ExecLoop( td,WaitSetup );
}

//---------------------------------------------------------------------------
void AddMsgFront( struct MsgPort *mp, struct Message *m )
{
	Forbid();
	
	AddHead( &mp->mp_MsgList, m );
	
	Permit();
}

struct Command	ReadySpin[] =
{
	{ CMD_SETSTARS,		0,0,0 },
	{ CMD_SETCNODECYCLE,CNODE_WAVE,0,0 },
	{ CMD_SETCNODECYCLE,CNODE_LOWWAVE,0,0 },
	{ CMD_CNODEWAIT,	CNODE_ALL,0,0 },
	{ CMD_CLEARSTARS,	0,0,0 },
	{ CMD_WAITFADE,		0,0,0 },
	{ CMD_3PLANE_ONOFF,	0,0,1 },
	{ CMD_END,0,0,0 }
};


struct Command	SpinFade[] =
{

	{ CMD_SETCDSTATE,   0,0,CDSTATE_ROTATE },
	{ CMD_SET_FROMTO,	0,15,0x0000 },
	{ CMD_FADETO,		0,0,0xffff },
	{ CMD_WAITFADE,		0,0,0 },
	// AT THIS POINT WE ARE READY TO TAKE DOWN THE SCREEN
	{ CMD_END,0,0,0 }
	
};



void SpinCD( register struct TData *td, WORD speed )
{
	struct Message 	*msg;


	ExecLoop( td, ReadySpin );

	// One last change before the drastic mesures need to be taken!!!
	if (msg = GetMsg(td->MsgPort ))
	{
		// False alarm... Just do the resetup the screen.
		if (msg->mn_Length == ANIMMSG_FALSEALARM)
		{
			ReplyMsg( msg );
			ExecLoop( td,WaitSetup );
			return;
		}
		
		// Nop store it for future checking.
		AddMsgFront( td->MsgPort,msg );
	}
	
	// Now destroy everthing in sight!!!
	CopyMem( &td->Table[1], &td->Table[(1+32*3)],(16*12));
	LoadRGB32( &td->Vp, td->Table );
	
	SetCopList( td, CD_SPRITEBANKODD, CD_ALTBANKEVEN );
	MakeVPort( &td->View,&td->Vp );
	MrgCop( &td->View );
	WaitTOF();

	SetFadeStep( td, speed );
	ExecLoop( td, SpinFade );
	SetFadeStep( td,REGULAR_FADESTEP );
	
	MemSet( &td->Table[1],0,(16*12));

	FON( td->Flags,TD_SPINNING );
}




struct Command	FinalCmd[] =
{
	{ CMD_SET_FROMTO,	48,128,0x0000 },
	{ CMD_FADETO,		0,0,0xffff },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_LEAVE },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_SETCDSTATE,   0,0,CDSTATE_STARTWAIT },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_WAITFADE,		0,0,0 },
	{ CMD_END,0,0,0 }
};



void EndScreen( register struct TData *td )
{
	if (!(td->Flags & TD_SPINNING)) 
		SpinCD( td, 32 );	// Get that thing Spinning!!!!!!! 

	ExecLoop( td, FinalCmd );
}


struct Command	Stop1Cmd[] =
{
	{ CMD_SETCDSTATE,   0,0,CDSTATE_WAIT },
	{ CMD_SET_FROMTO,	0,16,0xffff },
	{ CMD_FADETO,		0,0,0x0000 },
	{ CMD_WAITFADE,		0,0,0 },
	{ CMD_WAITCDSTATE,  0,0,0 },
	{ CMD_SET_FROMTO,	0,0,0x0000 },
	{ CMD_END,0,0,0 }
};



void StopCD( register struct TData *td )
{
	if (!(td->Flags & TD_SPINNING)) return;

	// Renew the lower sixteen by copying back the upper colors. 
	CopyMem( &td->Table[(1+32*3)],&td->Table[1],(16*12));

	ExecLoop( td, Stop1Cmd );

	SetCopList( td, CD_SPRITEBANKODD, CD_SPRITEBANKEVEN );

	MemSet( &td->Table[(1+32*3)],0,(16*12) );
	LoadRGB32( &td->Vp, td->Table );

	ExecLoop( td,WaitSetup );	
	
	
	FOFF( td->Flags, TD_SPINNING);
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

	if (InitStarSystem( td )) goto xit;

	InitAudio(td);
	

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
					ReplyMsg( msg ); msg = NULL;
					SpinCD(td,4);
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
	for(i=0;i<120;i++) 
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
	SpinCD(td,4);
	for(i=0;i<120;i++)
	{
		AnimCD(td);
		Fade(td);
		LoadTitleRGB32(td);
		WaitTOF();
	}
	StopCD( td );
	for(i=0;i<120;i++)
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
	FreeStarSystem( td );

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
