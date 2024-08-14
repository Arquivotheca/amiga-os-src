#ifndef SP_TOOLS_H
#define SP_TOOLS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>

#include "sketchpad.h"


/*****************************************************************************/


VOID AddToolBox(SketchPadPtr sp, WORD LeftEdge, WORD TopEdge);
SHORT SPSelectTool(SketchPadPtr sp, struct IntuiMessage *msg);
VOID SPSelectToolGad(SketchPadPtr sp, UWORD num);


/*****************************************************************************/


#endif /* SP_TOOLS_H */
