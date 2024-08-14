#ifndef	CLASSBASE_H
#define	CLASSBASE_H

/*****************************************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/classes.h>
#include <dos.h>

/*****************************************************************************/

#define ASM           __asm
#define REG(x)	      register __ ## x

/*****************************************************************************/

struct ClassLib
{
    struct Library	 cb_Library;
    UWORD		 cb_Pad;
    APTR		 cb_SysBase;
    APTR		 cb_UtilityBase;
    APTR		 cb_IntuitionBase;
    APTR		 cb_GfxBase;
    BPTR		 cb_SegList;

    APTR		 cb_SMult32;
    APTR		 cb_UMult32;
    APTR		 cb_SDivMod32;
    APTR		 cb_UDivMod32;

    Class		*cb_Class;
};

/*****************************************************************************/

#define ClassBase	((struct ClassLib *)getreg(REG_A6))
#define SysBase		ClassBase->cb_SysBase
#define GfxBase		ClassBase->cb_GfxBase
#define IntuitionBase	ClassBase->cb_IntuitionBase
#define UtilityBase	ClassBase->cb_UtilityBase

/*****************************************************************************/

kprintf(STRPTR,...);
sprintf(STRPTR,...);

/*****************************************************************************/

BOOL CreateClass(VOID);
BOOL DestroyClass(VOID);

/*****************************************************************************/

#endif /* CLASSBASE_H */
