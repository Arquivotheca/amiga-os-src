#ifndef LOCALEBASE_H
#define LOCALEBASE_H


/*****************************************************************************/


#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <prefs/locale.h>
#include <utility/utility.h>
#include <dos.h>
#include "locale.h"


/*****************************************************************************/


struct ExtLocale
{
    struct Locale      el_Locale;
    UWORD              el_UsageCnt;
    UWORD              el_MaxDateLen;   /* used by date patches */

    struct Library    *el_Language;
    APTR               el_ConvToLower;
    APTR               el_ConvToUpper;
    APTR               el_GetCodeSet;
    APTR	       el_GetDriverInfo;
    APTR               el_GetLocaleStr;
    APTR               el_IsAlNum;
    APTR               el_IsAlpha;
    APTR               el_IsCntrl;
    APTR               el_IsDigit;
    APTR               el_IsGraph;
    APTR               el_IsLower;
    APTR               el_IsPrint;
    APTR               el_IsPunct;
    APTR               el_IsSpace;
    APTR               el_IsUpper;
    APTR               el_IsXDigit;
    APTR               el_StrConvert;
    APTR               el_StrnCmp;

    char	       el_LocaleName[32];
    struct LocalePrefs el_Prefs;

    UBYTE	       el_NumFormat[16];
};


/*****************************************************************************/


struct LocaleLib
{
    struct Library	   lb_Lib;
    BOOL                   lb_PatchedOS;
    struct SignalSemaphore lb_LibLock;   /* Lock before modifying library */
    UWORD		   lb_UsageCnt;
    struct Library        *lb_SysLib;
    struct Library        *lb_DosLib;
    struct UtilityBase    *lb_UtilityLib;
    struct Library        *lb_IntuitionLib;
    ULONG		   lb_SegList;

    APTR		   lb_SMult32;
    APTR		   lb_UMult32;
    APTR		   lb_SDivMod32;
    APTR		   lb_UDivMod32;

    /* stuff used by the OS patches */
    struct Catalog        *lb_DOSCatalog;
    ULONG		   lb_OldGetDOSString;

    struct MinList	   lb_Catalogs;  /* see semaphore above */

    struct ExtLocale      *lb_DefaultLocale;
    struct ExtLocale       lb_PrefLocale;

    STRPTR                 lb_OldReqTitle;
    STRPTR                 lb_OldWBTitle;
};


/*****************************************************************************/


#define ASM           __stdargs __asm
#define REG(x)	      register __ ## x

#define LocaleBase    ((struct LocaleLib *)getreg(REG_A6))
#define SysBase       LocaleBase->lb_SysLib
#define DOSBase       LocaleBase->lb_DosLib
#define IntuitionBase LocaleBase->lb_IntuitionLib
#define UtilityBase   LocaleBase->lb_UtilityLib

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);
kprintf(STRPTR,...);


/*****************************************************************************/


#define BOOLEAN UBYTE    /* byte boolean */


/*****************************************************************************/


struct ExtLocale * ASM OpenLocaleLVO(REG(a0) STRPTR name);

VOID ASM CloseLocaleLVO(REG(a0) struct ExtLocale *locale);

struct ExtLocale * ASM SetDefaultLocaleLVO(REG(a0) struct ExtLocale *locale);

VOID ASM LockLib(VOID);
VOID ASM UnlockLib(VOID);
VOID ASM FlushLib(VOID);

APTR AllocVecFlush(ULONG memSize, ULONG memReqs);


/*****************************************************************************/


#endif /* LOCALEBASE_H */
