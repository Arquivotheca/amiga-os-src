#ifndef RESETWB_H
#define RESETWB_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


BOOL ResetWB(VOID);
BOOL CouldCloseWB(struct Window *refwindow);
struct Screen *LockWB(VOID);


/*****************************************************************************/


#endif /* RESETWB_H */
