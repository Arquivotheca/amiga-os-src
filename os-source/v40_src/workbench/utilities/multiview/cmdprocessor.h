/* cmdprocessor.h
 *
 */

#ifndef	EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#ifndef	INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif

/*****************************************************************************/

#define ASM				__asm
#define REG(x)				register __ ## x

/*****************************************************************************/

struct Cmd
{
    STRPTR		 c_Cmd;			/* String command */
    ULONG		(* ASM c_Func)(REG (a0) struct Hook *, REG (a2) struct Cmd *, REG (a1) Msg msg);
    ULONG		 c_CmdID;		/* Numeric command ID */
    STRPTR		 c_Template;		/* ReadArgs command template */
    ULONG		 c_NumOpts;		/* Number of arguments */
    ULONG		*c_Options;		/* System allocated */
};
