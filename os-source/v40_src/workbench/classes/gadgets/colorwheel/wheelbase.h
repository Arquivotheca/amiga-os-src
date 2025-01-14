#ifndef	WHEELBASE_H
#define	WHEELBASE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/classes.h>
#include <dos.h>


/*****************************************************************************/


struct WheelLib
{
    struct Library wb_Library;
    UWORD          wb_Pad;
    APTR           wb_SysBase;
    APTR	   wb_UtilityBase;
    APTR	   wb_IntuitionBase;
    APTR	   wb_GfxBase;
    BPTR           wb_SegList;

    APTR           wb_SMult32;
    APTR           wb_UMult32;
    APTR           wb_SDivMod32;
    APTR           wb_UDivMod32;

    Class         *wb_Class;
};


#define ASM           __asm
#define REG(x)	      register __ ## x

#define WheelBase     ((struct WheelLib *)getreg(REG_A6))
#define SysBase       WheelBase->wb_SysBase
#define GfxBase       WheelBase->wb_GfxBase
#define IntuitionBase WheelBase->wb_IntuitionBase
#define UtilityBase   WheelBase->wb_UtilityBase

kprintf(STRPTR,...);
sprintf(STRPTR,...);


/*****************************************************************************/


BOOL CreateClass(VOID);
BOOL DestroyClass(VOID);


/*****************************************************************************/


#endif /* WHEELBASE_H */
