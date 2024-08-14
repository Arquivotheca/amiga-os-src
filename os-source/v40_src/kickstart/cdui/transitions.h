#ifndef TRANSITIONS_H
#define TRANSITIONS_H


/*****************************************************************************/


#include <exec/types.h>
#include "cduibase.h"


/*****************************************************************************/


void NothingToMain(struct CDUILib *CDUIBase);
void NothingToBooting(struct CDUILib *CDUIBase);
void DoorClosedToMain(struct CDUILib *CDUIBase);
void DoorClosedToBooting(struct CDUILib *CDUIBase);
void DoorClosedToClosing(struct CDUILib *CDUIBase);
void MainToDoorClosed(struct CDUILib *CDUIBase);
void MainToClosing(struct CDUILib *CDUIBase);
void BootingToClosing(struct CDUILib *CDUIBase);


/*****************************************************************************/


#endif  /* TRANSITIONS_H */
