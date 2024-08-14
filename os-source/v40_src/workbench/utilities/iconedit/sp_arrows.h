#ifndef SP_ARROWS_H
#define SP_ARROWS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>

#include "dynamicimages.h"
#include "sketchpad.h"


/*****************************************************************************/


VOID MoveImage(USHORT id, DynamicImagePtr di);
VOID AddMoveBox(SketchPadPtr sp, SHORT LeftEdge, SHORT TopEdge);
VOID SPScroll(SketchPadPtr sp, struct IntuiMessage *msg);


/*****************************************************************************/


#endif /* SP_ARROWS_H */
