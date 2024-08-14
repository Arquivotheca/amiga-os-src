#ifndef	CDTV_CDGPREFS_H
#define	CDTV_CDGPREFS_H	1

/*
**
**	$Id: cdgprefs.h,v 1.9 92/04/20 15:39:26 darren Exp $
**
**	CDTV CD+G -- cdgprefs.i (CD+G preferences structure)
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**
*/

#include "exec/types.h"
#include "exec/tasks.h"

#define CDGNAME	"cdg.library"

struct	CDGPrefs
{
	/* see cdgprefs.i for comments */

	UWORD		*cdgp_ChannelSprites[64];
	UWORD		*cdgp_PAUSESprite[4];
	UWORD		*cdgp_NTrackSprite[4];
	UWORD		*cdgp_PTrackSprite[4];
	UWORD		*cdgp_FFSprite[4];
	UWORD		*cdgp_RWSprite[4];
	UWORD		*cdgp_STOPSprite[4];
	UWORD		*cdgp_PLAYSprite[4];
	UWORD		cdgp_SpriteHeight;
	UWORD		cdgp_SpriteColors[8];
	WORD		cdgp_DisplayX;
	WORD		cdgp_DisplayY;
	UWORD		cdgp_Control;
	UWORD		cdgp_Reserved0;
	UWORD		cdgp_Reserved1;
	ULONG		cdgp_SigMask;
	struct	Task	*cdgp_SigTask;
};

/* Flag bits for cdgp_Control */

#define	CDGB_NOECC	0
#define	CDGF_NOECC	(1<<CDGB_NOECC)

#define	CDGB_NOMIDI	1
#define	CDGF_NOMIDI	(1<<CDGB_NOMIDI)

#define CDGB_ALTSPRITES	2
#define CDGF_ALTSPRITES (1<<CDGB_ALTSPRITES)

/* return values, and argument bits for CDGDraw() */

#define	CDGB_GRAPHICS	0
#define CDGF_GRAPHICS	(1<<CDGB_GRAPHICS)

#define	CDGB_MIDI	1
#define CDGF_MIDI	(1<<CDGB_MIDI)

#endif	/* CDTV_CDGPREFS_H */
