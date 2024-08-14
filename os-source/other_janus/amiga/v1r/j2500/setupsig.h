#ifndef JANUS_SETUPSIG_H
#define JANUS_SETUPSIG_H
/*************************************************************************
*
* setupsig.i -- data structure for SetupJanusSig() routine
*
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
*
*************************************************************************/


#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif

#ifndef EXEC_INTERRUPTS_H
#include "exec/interrupts.h"
#endif



struct SetupSig {
    struct Interrupt ss_Interrupt;
    APTR	ss_TaskPtr;
    ULONG	ss_SigMask;
    APTR	ss_ParamPtr;
    ULONG	ss_ParamSize;
    UWORD	ss_JanusIntNum;
};

#endif
