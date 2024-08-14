#ifndef IEMESSAGES_H
#define IEMESSAGES_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <dos/dos.h>

#include "iemain.h"


/*****************************************************************************/


VOID ProcessIMessages(WindowInfoPtr wi);
VOID HandleCursor(struct IntuiMessage *msg);


/*****************************************************************************/


#endif /* IEMESSAGES_H */
