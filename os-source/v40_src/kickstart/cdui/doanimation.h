#ifndef DOANIMATION_H
#define DOANIMATION_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/ports.h>


/*****************************************************************************/


/* Possible returns from DoAnimation() */
#define CDUI_LANGUAGE_SELECTOR 0
#define CDUI_NVRAM_EDITOR      1
#define CDUI_EXIT              2


/*****************************************************************************/


ULONG DoAnimation(struct CDUILib *CDUIBase, struct MsgPort *port);


/*****************************************************************************/


#endif  /* DOANIMATION_H */
