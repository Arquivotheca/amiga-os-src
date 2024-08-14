#ifndef SPEECH_H
#define SPEECH_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include "say_utils.h"


/*****************************************************************************/


enum SayStatus OpenSpeech(VOID);
VOID CloseSpeech(VOID);

enum SayStatus Say(STRPTR string, UWORD sex, UWORD pitch, UWORD rate, UWORD mode);
VOID WaitSpeech(VOID);
VOID StopSpeech(VOID);


/*****************************************************************************/


#endif /* SPEECH_H */
