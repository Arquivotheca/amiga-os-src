#ifndef VERSIONBASE_H
#define VERSIONBASE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <dos/dos.h>
#include <dos.h>


/*****************************************************************************/


struct VersionLib
{
    struct Library     vl_LibNode;
    UWORD              vl_Pad;
    struct DosLibrary *vl_DOSBase;
    struct Library    *vl_UtilityBase;
    struct ExecBase   *vl_SysBase;
    BPTR               vl_SegList;
/*
    struct LocaleInfo  vl_LocaleInfo;
*/
    APTR	       vl_SMult32;
    APTR	       vl_UMult32;
    APTR	       vl_SDivMod32;
    APTR	       vl_UDivMod32;
};

#define ASM           __asm
#define REG(x)	      register __ ## x

#define AslBase       ((struct VersionLib *)getreg(REG_A6))
#define SysBase       VersionBase->vl_SysBase
#define DOSBase       VersionBase->vl_DOSBase
#define UtilityBase   VersionBase->vl_UtilityBase
#define LocaleBase    VersionBase->vl_LocaleInfo.li_LocaleBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);
kprintf(STRPTR,...);
sprintf(STRPTR,...);


/*****************************************************************************/


#define BOOLEAN UBYTE    /* byte boolean */


/*****************************************************************************/


struct Resident * ASM FindResidentNC(REG(a6) struct VersionLib *VersionBase,
                                     REG(a0) STRPTR name);

struct Node * ASM FindNameNC(REG(a6) struct VersionLib *VersionBase,
                             REG(a0) struct List *list,
                             REG(a1) STRPTR name);



/*****************************************************************************/


#endif /* VERSIONBASE_H */
