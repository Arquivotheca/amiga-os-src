
/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/


/****************************************************************************/


#ifndef EDITHOOK_H
#define EDITHOOK_H


/*****************************************************************************/


#include <exec/types.h>
#include <dos.h>


/*****************************************************************************/


ULONG __asm HexHook(register __a0 struct Hook *hook,
                    register __a2 struct SGWork *sgw,
                    register __a1 ULONG *msg);


/*****************************************************************************/


#endif /* EDITHOOK_H */
