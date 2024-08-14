

#include <exec/types.h>


/*****************************************************************************/


struct FloatData
{
    LONG  fd_MinValue;
    LONG  fd_MaxValue;
    LONG  fd_NumDecimals;
    UBYTE fd_Buffer[32];
    UBYTE fd_Flags;
};

#define	FDF_EXIT	0x01


/*****************************************************************************/


#define ASM    __asm
#define REG(x) register __ ## x


ULONG ASM FloatHook(REG(a0) struct Hook *hook,
                    REG(a2) struct SGWork *sgw,
                    REG(a1) ULONG *msg);
