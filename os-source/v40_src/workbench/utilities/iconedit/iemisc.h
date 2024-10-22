#ifndef IEMISC_H
#define IEMISC_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>

#include "texttable.h"


/*****************************************************************************/


VOID SetBusyPointer(struct Window *window);
VOID SetHelpPointer(struct Window *window);
VOID SetFillPointer(struct Window *window);
VOID SetCrossPointer(struct Window *window);

SHORT EasyReq(struct Window *win, STRPTR title, STRPTR text, STRPTR gadgets, STRPTR args, ...);
VOID NotifyUser(AppStringsID msg, STRPTR string);


/*****************************************************************************/


#endif /* IEMISC_H */
