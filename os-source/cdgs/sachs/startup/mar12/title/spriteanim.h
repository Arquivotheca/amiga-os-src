/* :ts=4
*
*	SpriteAnim.h
*
*	William A. Ware						D305
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#ifndef	SPRITEANIM_H
#define SPRITEANIM_H

struct SpriteInfo
{
	WORD	Offset,X,Y;
	UBYTE	Height,Attached;
};

#define CD_SECCOUNT		8

struct SpriteAnim
{
	LONG	SDataSize;
	LONG	InterLaceOffset;

	LONG	AmigaOnOff[2];
	LONG	NumberOnOff[2];
	LONG	CDAnim[59][2];


	struct SpriteInfo	SData[8];
	struct SpriteInfo	Amiga[6];
	struct SpriteInfo	CD[CD_SECCOUNT];
	struct SpriteInfo	Number[2];
};

#endif