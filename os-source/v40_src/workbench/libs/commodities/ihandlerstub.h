#ifndef IHANDLERSTUB_H
#define IHANDLERSTUB_H


/*****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>


/****************************************************************************/


struct InputEvent * ASM IHandlerStub(REG(a0) struct InputEvent *ie,
                                     REG(a1) APTR handlerData);


/*****************************************************************************/


#endif /* IHANDLERSTUB_H */
