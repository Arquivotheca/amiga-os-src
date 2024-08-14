#ifndef IHANDLER_H
#define IHANDLER_H


/*****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>


/****************************************************************************/


struct InputEvent * ASM CxHandler(REG(a0) struct InputEvent *ie,
                                  REG(a1) struct CxLib *CxBase);


/*****************************************************************************/


#endif /* IHANDLER_H */
