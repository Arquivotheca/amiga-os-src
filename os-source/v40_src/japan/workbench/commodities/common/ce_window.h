#ifndef CE_WINDOW_H
#define CE_WINDOW_H


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/commodities.h>


/*****************************************************************************/


VOID CreateWindow(VOID);
VOID DisposeWindow(VOID);
BOOL ProcessIntuiMsg(struct IntuiMessage *intuiMsg);


/*****************************************************************************/


#endif /* CE_WINDOW_H */
