#ifndef	GRADIENTBASE_H
#define	GRADIENTBASE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/libraries.h>
#include <intuition/classes.h>
#include <dos.h>


/*****************************************************************************/


struct GradientLib
{
    struct Library gb_Library;
    UWORD          gb_Pad;
    APTR           gb_SysBase;
    APTR	   gb_UtilityBase;
    APTR	   gb_IntuitionBase;
    APTR	   gb_GfxBase;
    BPTR           gb_SegList;

    APTR           gb_SMult32;
    APTR           gb_UMult32;
    APTR           gb_SDivMod32;
    APTR           gb_UDivMod32;

    Class         *gb_Class;
};


#define ASM           __asm
#define REG(x)	      register __ ## x

#define GradientBase  ((struct GradientLib *)getreg(REG_A6))
#define SysBase       GradientBase->gb_SysBase
#define GfxBase       GradientBase->gb_GfxBase
#define IntuitionBase GradientBase->gb_IntuitionBase
#define UtilityBase   GradientBase->gb_UtilityBase

kprintf(STRPTR,...);
sprintf(STRPTR,...);


/*****************************************************************************/


BOOL CreateClass(VOID);
BOOL DestroyClass(VOID);


/*****************************************************************************/


#endif /* GRADIENTBASE_H */
