#ifndef EDIT_H
#define EDIT_H


/*****************************************************************************/


#include <exec/types.h>
#include <graphics/gfx.h>
#include "pe_custom.h"


/*****************************************************************************/


VOID EditOverscan(EdDataPtr ed, BOOL text);
VOID ShiftRect(struct Rectangle *rect, WORD dx, WORD dy);
WORD Modulo(WORD num, WORD div);
WORD RectWidth(struct Rectangle *rect);
WORD RectHeight(struct Rectangle *rect);


/*****************************************************************************/


#endif /* EDIT_H */
