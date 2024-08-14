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

#define MAX_STARS	256


#define SysBase *((LONG *)4)

#define FON(x,flag)		x |= (flag)
#define FOFF(x,flag)	x &= ~(flag)



#define		TERR(err)	return (td->Error = err );


#define 	TERR_MEMORY		-1
#define		TERR_DATA		-2
#define		TERR_SPRITE		-3
#define		TERR_GFXBASE	-4
#define		TERR_DEBOXBASE	-5


//------------------------------------------------------------
struct StarMsg
{
	struct Message		Msg;
	struct TData		*td;
	LONG				arg1;
	LONG				*arglist;
};

struct Star
{
	struct MinNode	Node;
	
	WORD			X,Y,Z;
	WORD			Offset;
};

struct SoundInfo
{
	UBYTE			Volume,DestVolume;
	UWORD			Period;
	
	LONG			RampTime,RampTimer,LastTime;
	ULONG			Flags;
};

#define SIF_CYCLE	(1<<0)

enum
{
	INSTR_VIOLS,
	
	INSTR_MAX
};


struct MemHandData
{
	struct Interrupt	MemIntr;
	struct Task			*Task;
	APTR				Data;
	LONG				DataSize;
};

//-----------------------------------------------------------
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
	struct BMInfo		*PalBmi;
	struct BMInfo		*Bmi;
	struct BitMap		*Bm;
	struct BitMap		*Bm3;
	
	struct BitMap		*RBm,*RBm3;	// Restore Bitmaps!
	UWORD				*BYTable;

	WORD				FadeFromColor,FadeToColor;
	ULONG				*Table,*FadeTable;
	UBYTE				*FadeMask;
	LONG				FadeLv,FadeTo,FadeStep;
	LONG				LastFadeTime;


	struct ViewPort		*CurVp;
	
	struct View			View;
	struct ViewPort		Vp;
	struct RasInfo		Ri;

	struct UCopList		UCop;
	struct CopList		*UPtr;

	struct cprList		*LCop,*SCop;
	
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

	LONG				CDTimer,LastCDTime,CDSpeed;
	WORD				CDFrame;
	UBYTE				CDState,NextCDState;


	// SOUND
	// 
	struct MsgPort		*AudioReply;
	struct List			inusecmd; 
	struct List			freecmd;  
	struct List			readycmd;  
	struct SoundCommand keysc;  
	LONG	 			SQue;

	struct SmSound		*Instr[INSTR_MAX];

	UBYTE				oldciabits;

	WORD				*SoundPC;
	LONG				NextCmd;
	struct SoundInfo	Sound[2];

	// Star STuff.
	struct StarMsg		SMsg;
	struct MsgPort		*SPort;
	struct MsgPort		*SReply;
	
	struct List			freeList,activeList;
	struct Star			starArr[MAX_STARS];
	LONG				lastStarTime;
	LONG				starTimer;
	LONG				starSeed;
	LONG				Warp;
	WORD				MaxStars,starCount;
	WORD				DestStars;

	struct MemHandData	MHand;
};
#define	TD_TCHANGED		(1<<1)
#define TD_AMIGAON		(1<<2)
#define TD_CDLOOP		(1<<3)

#define	TD_SCREENUP		(1<<4)
#define	TD_SPRITESUP	(1<<5)

#define TD_NUMBERON		(1<<6)
#define TD_3PL_NUMON	(1<<7)

#define TD_STARSON		(1<<8)
#define TD_3PLANEON		(1<<9)

#define TD_SPINNING		(1<<10)


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
	CNODE_LOWWAVE,
	CNODE_CD,
	CNODE_TEMP1,
	CNODE_TEMP2,
	CNODE_TEMP3,
	CNODE_TEMP4,
	CNODE_TEMP5
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
	CMD_MARK,
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
	CMD_PLAYSND,
	CMD_DECOMP,
	CMD_SETCOLOR,
	CMD_LOADVIEW,
	CMD_SETSTARS,
	CMD_STARS_ONOFF,
	CMD_CLEARSTARS,
	CMD_SETCNODECYCLE,
	CMD_SET_FROMTO,
	CMD_SETCNODESPEED,
	CMD_SETDESTVOL,

	CMD_MAX
};

//-----------------------------------------------------------
enum
{
	SND_TITLE1,
	SND_TITLE2
};

enum
{
	DECOMP_MAINBM
};

//-----------------------------------------------------------


//--------------------------------------------
// STAR SYSTEM Commands and Prototyes..
//-------------------------------------------

#define SMSG_WAIT				0x80


#define		ST_SHUTDOWN		(SMSG_WAIT+0)
#define		ST_STARTUP		(SMSG_WAIT+1)
#define		ST_SETSTARS		2
#define		ST_STARS_ONOFF	3
#define		ST_SET3PLANE	(SMSG_WAIT+4)
#define		ST_STARCOUNT	(SMSG_WAIT+5)
#define 	ST_ONESOUND		(SMSG_WAIT+6)
#define 	ST_SETDESTVOL	(SMSG_WAIT+7)


LONG SendSMsg			( struct TData *td, LONG msg, LONG arg1, ... );
InitStarSystem			( struct TData *td );
void FreeStarSystem		( struct TData *td );

//--------------------------------------------------------------------------
/* PROTO TYPES FOR OTHER MODULES */

LONG DecompDLTAX(	UBYTE *data, WORD bpr, struct BitMap *bm, UWORD *ytable );
UWORD *AllocYTable( WORD bpr, WORD rows );
void FreeYTable( UWORD *tbl,WORD rows );

struct Interrupt * __asm	AddVBlank( register __a0 void * );
void __asm					FreeVBlank( register __a0 struct Interrupt * );

__asm Rnd( register __a0 LONG *seed, register __d0 WORD range );


void __saveds Title(void);

StartDecompTask( register struct TData *td );
WaitDecompMsg( register struct TData *td, BOOL wait );
void EndDecompTask( register struct TData *td );
void AnimCD( register struct TData *td );

BackDecompBitMap( struct TData *td, UBYTE *data, struct BitMap **bm );

void SetSpriteGroup(struct TData *td, struct SpriteInfo *si, WORD count,
					WORD xloc, WORD yloc );


LONG MicrosSince( struct TData *td, LONG *lasttime );

extern struct Custom __far	custom;

extern UBYTE __far	back[];
extern UBYTE __far	pal1[];
extern struct SpriteAnim __far SAnim;

extern UBYTE __far  back3plane[];
extern UBYTE __far  number[];

#endif // TITLE_H