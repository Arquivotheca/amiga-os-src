/* :ts=4
*
*	/include/sound.h
*
*	William A. Ware						CC31
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1992, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/

#ifndef SOUND_H
#define SOUND_H


#ifndef DEVICES_AUDIO_H
#include <devices/audio.h>
#endif

struct SmSound
{
	LONG		Size;
	LONG		Pad;
	WORD		Speed;
	WORD		Volume;
	LONG		RepeatOffset;
	LONG		Lock;
	LONG		Extra[9];		/* For my use */
};


InitAudio(struct TData *td);
void FreeAudio(struct TData *td);

PlaySmSound( struct TData *td,
			struct SmSound *ss, int times, UBYTE ch, int volume,
			 int period);


struct SoundCommand
{
	struct IOAudio		ac;
	struct MinNode		n;
	struct SmSound		*s;
};

#ifndef TITLE_H
#include "title.h"
#endif

#endif