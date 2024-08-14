
#include <devices/inputevent.h>

#include <clib/keymap_protos.h>

#include <pragmas/keymap_pragmas.h>

#include "commoditiesbase.h"
#include "ikm.h"


/*****************************************************************************/


#define STIMSIZE 3


/*****************************************************************************/


BOOL ASM InvertKeyMapLVO(REG(d0) ULONG ansiCode,
                         REG(a0) struct InputEvent *ie,
                         REG(a1) struct KeyMap *km,
                         REG(a6) struct CxLib *CxBase)
{
unsigned char buffer[80];
unsigned char rBuffer[STIMSIZE*2];

    ie->ie_EventAddress       = 0;
    ie->ie_SubClass           = 0;
    ie->ie_TimeStamp.tv_secs  = 0;
    ie->ie_TimeStamp.tv_micro = 0;
    ie->ie_Class              = IECLASS_RAWKEY;
    buffer[0]                 = (unsigned char)ansiCode;

    switch (MapANSI(buffer,1,rBuffer,STIMSIZE,km))
    {
        case 2 : ie->ie_Prev1DownCode = rBuffer[0];
                 ie->ie_Prev1DownQual = rBuffer[1];
                 ie->ie_Code          = rBuffer[2];
                 ie->ie_Qualifier     = rBuffer[3];
                 break;

        case 1 : ie->ie_Prev1DownCode = 0;
                 ie->ie_Prev1DownQual = 0;
                 ie->ie_Code          = rBuffer[0];
                 ie->ie_Qualifier     = rBuffer[1];
                 break;

        default: return(FALSE);
    }

    return(TRUE);
}
