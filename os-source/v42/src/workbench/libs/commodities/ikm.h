#ifndef IKM_H
#define IKM_H


/*****************************************************************************/


#include <exec/types.h>
#include <devices/inputevent.h>
#include <devices/keymap.h>


/****************************************************************************/


BOOL ASM InvertKeyMapLVO(REG(d0) ULONG ansiCode,
                         REG(a0) struct InputEvent *ie,
                         REG(a1) struct KeyMap *km,
                         REG(a6) struct CxLib *CxBase);


/*****************************************************************************/


#endif /* IKM_H */
