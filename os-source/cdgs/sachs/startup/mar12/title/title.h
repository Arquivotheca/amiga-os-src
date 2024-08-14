/* :ts=4
*
*	title.h
*
*	William A. Ware						D303
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#ifndef TITLE_H
#define TITLE_H

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

#ifndef GRAPHICS_SPRITES_H
#include <graphics/sprite.h>
#endif

#ifndef COLOR_H
#include "color.h"
#endif

#ifndef SPRITENANIM_H
#include "spriteanim.h"
#endif

#ifndef SOUND_H
#include "sound.h"
#endif

#define SysBase *((LONG *)4)

#define FON(x,flag)		x |= (flag)
#define FOFF(x,flag)	x &= ~(flag)


#define		TERR(err)	return (td->Error = err );


#define 	TERR_MEMORY		-1
#define		TERR_DATA		-2
#define		TERR_SPRITE		-3
#define		TERR_GFXBASE	-4
#define		TERR_DEBOXBASE	-5


struct DecompMsg
{
	struct Message		Msg;
	UBYTE				*Data[2];
	struct BitMap		*Bm;
	UWORD				*YTable;
	WORD				*Frame;
};

struct TData
{
	// VBLANK INTERRUPT DATA
	WORD				SpriteDirty[2];
	ULONG				SpriteMoveData[8];
	UBYTE				*SpriteData[16];
	LONG				CountUp;
	WORD				DecompFrame;
	BYTE				Bank;		// Bank of sprite to display or -1 for off
	//
	
	UBYTE				Pal;	// TRUE if the system is pal.

	//General Stuff.. To be Initilized
	//
	struct MsgPort		*MsgPort;
	struct GfxBase		*GfxBase;
	struct Library		*DeBoxBase;

	WORD				FPS;	// Frames Per Second.
	UWORD				MPF;	// Micros per Frame;
	struct MinList		CnList;
	struct View			*OldView;

	LONG				Flags;

	LONG				Error;

	struct SpriteAnim	*SAnim;

	// SCREEN STUFF
	//
	struct BMInfo		*Bmi;	
	struct BMInfo		*PalBmi;
	struct BitMap		*Bm;
	struct BitMap		*Bm3;


	ULONG				*Table,*FadeTable;
	UBYTE				*FadeMask;
	LONG				FadeLv,FadeTo,FadeStep;
	LONG				LastFadeTime;


	struct View			View;
	struct ViewPort		Vp;
	struct RasInfo		Ri;

	struct UCopList		UCop;
	struct CopList		*UPtr;
	WORD				Sx,Sy;


	struct ExtSprite	Sprite[8];
	UBYTE				*SData[2];
	struct BitMap		SpriteBm[2];
	UWORD				*YTable;

	struct Interrupt	*VBlank;

	// Command Engine
	struct Command		*PC;
	LONG				NextWait;	// Micro seconds to wait for next command.
	LONG				LastUpdate;
	
	struct CNode		CNode[10];

	// Decomp Task
	struct MsgPort		*DPort;
	struct MsgPort		*DMsgReply;
	struct DecompMsg	DMsg;

	LONG				CDTimer,LastCDTime,CDSpeed;
	WORD				CDFrame;
	UBYTE				CDState,NextCDState;


	// SOUND
	// 
	struct MsgPort		*AudioReply;
	struct List			inusecmd; 
	struct List			freecmd;  
	struct SoundCommand keysc;  
	LONG	 			SQue;

	struct SmSound		*Sample;

	UBYTE				oldciabits;
};
#define	TD_TCHANGED		(1<<1)
#define TD_AMIGAON		(1<<2)
#define TD_CDLOOP		(1<<3)

#define	TD_SCREENUP		(1<<4)
#define	TD_SPRITESUP	(1<<5)

#define TD_NUMBERON		(1<<6)
#define TD_3PL_NUMON	(1<<7)



#define CDS_START	0
#define CDS_END		1
#define CDS_HOLD	2
#define CDS_CYCLE	3



#define CNODE_ALL	0xff
enum
{
	CNODE_WAVE,
	CNODE_AMIGA,
	CNODE_NUMBER,
	CNODE_TEMP1,
	CNODE_TEMP2,
	CNODE_TEMP3,
	CNODE_TEMP4
};


enum 
{
	CDSTATE_STARTWAIT,
	CDSTATE_WAIT,
	CDSTATE_START,
	CDSTATE_ROTATE,
	CDSTATE_LEAVE
};


struct Command
{
	UBYTE		Command;
	UBYTE		ArgB;
	WORD		ArgW;
	LONG		ArgL;
};

enum
{
	CMD_END,
	CMD_FADELV,
	CMD_FADETO,
	CMD_WAIT,
	CMD_STARTCNODE,
	CMD_LINKCNODE,
	CMD_SETCNODE,
	CMD_AMIGAONOFF,
	CMD_CNODEWAIT,
	CMD_SETCDSTATE,
	CMD_WAITCDSTATE,
	CMD_CNODESWITCH,
	CMD_WAITFADE,
	CMD_3PLANE_ONOFF,
	CMD_NUMBER_ONOFF,

	CMD_MAX
};


/* PROTO TYPES FOR OTHER MODULES */

LONG DecompDLTAX(	UBYTE *data, WORD bpr, struct BitMap *bm, UWORD *ytable );
UWORD *AllocYTable( WORD bpr, WORD rows );
void FreeYTable( UWORD *tbl,WORD rows );

struct Interrupt * __asm	AddVBlank( register __a0 void * );
void __asm					FreeVBlank( register __a0 struct Interrupt * );

void __saveds Title(void);

StartDecompTask( register struct TData *td );
WaitDecompMsg( register struct TData *td, BOOL wait );
void EndDecompTask( register struct TData *td );
void AnimCD( register struct TData *td );

BackDecompBitMap( struct TData *td, UBYTE *data, struct BitMap **bm );

void SetSpriteGroup(struct TData *td, struct SpriteInfo *si, WORD count,
					WORD xloc, WORD yloc );




extern struct Custom __far	custom;

extern UBYTE __far	back[];
extern UBYTE __far	pal1[];
extern struct SpriteAnim __far SAnim;

extern struct CompHeader __far	sample[];
extern UBYTE __far  back3plane[];
extern UBYTE __far  number[];

#endif // TITLE_H