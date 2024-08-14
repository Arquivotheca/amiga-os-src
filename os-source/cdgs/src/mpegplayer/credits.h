#ifndef CREDITS_H
#define CREDITS_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


VOID CreateCredits(struct MPEGPlayerLib *MPEGPlayerBase, struct CDSequence *track);
VOID DeleteCredits(struct MPEGPlayerLib *MPEGPlayerBase);
VOID ScrollCredits(struct MPEGPlayerLib *MPEGPlayerBase, LONG direction);
VOID DrawCreditTitle(struct MPEGPlayerLib *MPEGPlayerBase, struct CDSequence *track);


/*****************************************************************************/


#endif /* CREDITS_H */
