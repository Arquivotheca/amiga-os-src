#ifndef TEXT_H
#define TEXT_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


VOID CreateText(struct MPEGPlayerLib *MPEGPlayerBase, STRPTR title, STRPTR text, ULONG textLen);
VOID DeleteText(struct MPEGPlayerLib *MPEGPlayerBase);
VOID ScrollText(struct MPEGPlayerLib *MPEGPlayerBase, LONG direction);


/*****************************************************************************/


#endif /* TEXT_H */
