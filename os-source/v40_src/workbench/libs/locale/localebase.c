
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/var.h>
#include <intuition/intuition.h>
#include <libraries/iffparse.h>
#include <prefs/locale.h>
#include <prefs/prefhdr.h>
#include <utility/utility.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "localebase.h"
#include "locale.h"
#include "english.h"
#include "catalog.h"
#include "driverface.h"
#include "languagedrivers.h"


/*****************************************************************************/


/* needed by OS patches */
extern struct LocaleLib * far LocBase;
VOID ASM GetDOSStringPatch(VOID);
VOID ASM ToUpperPatch(VOID);
VOID ASM ToLowerPatch(VOID);
VOID ASM StricmpPatch(VOID);
VOID ASM StrnicmpPatch(VOID);
VOID ASM RawDoFmtPatch(VOID);
VOID ASM DateToStrStub(VOID);
VOID ASM StrToDateStub(VOID);


/*****************************************************************************/


static VOID PatchOS(VOID);


/*****************************************************************************/


static APTR GetFuncAddr(struct Library *lib, WORD offset)
{
APTR *addr;

    addr = (APTR)(((ULONG)lib) + ((ULONG)offset) + 2);
    return(*addr);
}


/*****************************************************************************/


static BOOL LoadLanguage(struct ExtLocale *locale, STRPTR name)
{
char            path[60];
struct Library *language;
ULONG           info;

    locale->el_Locale.loc_LanguageName = "english.language";
    locale->el_Language                = NULL;
    locale->el_ConvToLower             = (APTR)EnglishConvToLower;
    locale->el_ConvToUpper             = (APTR)EnglishConvToUpper;
    locale->el_GetCodeSet              = (APTR)EnglishGetCodeSet;
    locale->el_GetDriverInfo           = (APTR)EnglishGetDriverInfo;
    locale->el_GetLocaleStr            = (APTR)EnglishGetLocaleStr;
    locale->el_IsAlNum                 = (APTR)EnglishIsAlNum;
    locale->el_IsAlpha                 = (APTR)EnglishIsAlpha;
    locale->el_IsCntrl                 = (APTR)EnglishIsCntrl;
    locale->el_IsDigit                 = (APTR)EnglishIsDigit;
    locale->el_IsGraph                 = (APTR)EnglishIsGraph;
    locale->el_IsLower                 = (APTR)EnglishIsLower;
    locale->el_IsPrint                 = (APTR)EnglishIsPrint;
    locale->el_IsPunct                 = (APTR)EnglishIsPunct;
    locale->el_IsSpace                 = (APTR)EnglishIsSpace;
    locale->el_IsUpper                 = (APTR)EnglishIsUpper;
    locale->el_IsXDigit                = (APTR)EnglishIsXDigit;
    locale->el_StrConvert              = (APTR)EnglishStrConvert;
    locale->el_StrnCmp                 = (APTR)EnglishStrnCmp;

    if (name && (strcmp(name,"english") != 0))
    {
        strcpy(path,"LOCALE:Languages");
        AddPart(path,name,sizeof(path));
        strcat(path,".language");

	if (!(language = OpenLibrary(path,0)))
        {
            SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);
            return(FALSE);
        }

        locale->el_Locale.loc_LanguageName = language->lib_Node.ln_Name;
        locale->el_Language                = language;
        locale->el_GetDriverInfo           = GetFuncAddr(language,-30);
        info = GetDriverInfo(locale);

        if (info & GDIF_CONVTOLOWER)
            locale->el_ConvToLower         = GetFuncAddr(language,-36);

        if (info & GDIF_CONVTOUPPER)
            locale->el_ConvToUpper         = GetFuncAddr(language,-42);

        if (info & GDIF_GETCODESET)
            locale->el_GetCodeSet          = GetFuncAddr(language,-48);

        if (info & GDIF_GETLOCALESTR)
            locale->el_GetLocaleStr        = GetFuncAddr(language,-54);

        if (info & GDIF_ISALNUM)
            locale->el_IsAlNum             = GetFuncAddr(language,-60);

        if (info & GDIF_ISALPHA)
            locale->el_IsAlpha             = GetFuncAddr(language,-66);

        if (info & GDIF_ISCNTRL)
            locale->el_IsCntrl             = GetFuncAddr(language,-72);

        if (info & GDIF_ISDIGIT)
            locale->el_IsDigit             = GetFuncAddr(language,-78);

        if (info & GDIF_ISGRAPH)
            locale->el_IsGraph             = GetFuncAddr(language,-84);

        if (info & GDIF_ISLOWER)
            locale->el_IsLower             = GetFuncAddr(language,-90);

        if (info & GDIF_ISPRINT)
            locale->el_IsPrint             = GetFuncAddr(language,-96);

        if (info & GDIF_ISPUNCT)
            locale->el_IsPunct             = GetFuncAddr(language,-102);

        if (info & GDIF_ISSPACE)
            locale->el_IsSpace             = GetFuncAddr(language,-108);

        if (info & GDIF_ISUPPER)
            locale->el_IsUpper             = GetFuncAddr(language,-114);

        if (info & GDIF_ISXDIGIT)
            locale->el_IsXDigit            = GetFuncAddr(language,-120);

        if (info & GDIF_STRCONVERT)
            locale->el_StrConvert          = GetFuncAddr(language,-126);

        if (info & GDIF_STRNCMP)
            locale->el_StrnCmp             = GetFuncAddr(language,-132);
    }

    return(TRUE);
}


/*****************************************************************************/


static VOID InitNumFormat(struct ExtLocale *locale)
{
UBYTE *table;
UWORD  i,j;

    for (i = 0; i<16; i++)
        locale->el_NumFormat[i] = 0;

    if (table = locale->el_Locale.loc_Grouping)
    {
        i = 0;
        j = 0;
        while ((i < 10) && (table[i] != '\xff'))
        {
            if ((table[i] == '\x00') && (i > 0))
                i--;

            j += table[i];
            if (j > 16)
                break;

            locale->el_NumFormat[j] = 1;
            i++;
        }
    }
}


/*****************************************************************************/


VOID TestLen(struct ExtLocale *locale, ULONG str)
{
UWORD len;

    len = strlen(GetLocaleStr(locale,str));
    if (len > locale->el_MaxDateLen)
        locale->el_MaxDateLen = len;
}


/*****************************************************************************/

#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_LCLE,
};


static BOOL LoadLocale(struct ExtLocale *locale, STRPTR name)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
BOOLEAN             headerFlag = FALSE;
BOOLEAN             dataFlag = FALSE;
BOOLEAN		    result = FALSE;
struct PrefHeader   phead;
LONG                error;
struct Library     *IFFParseBase;
UWORD		    i;

    if (!(IFFParseBase = OpenLibrary("iffparse.library",37)))
    {
        SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);
        return(FALSE);
    }

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = (ULONG)Open(name,MODE_OLDFILE))
        {
            InitIFFasDOS(iff);

            if (!OpenIFF(iff,IFFF_READ))
            {
                if (!ParseIFF(iff,IFFPARSE_STEP))
                {
                    cn = CurrentChunk(iff);
                    if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_PREF)
                    {
                        if (!StopChunks(iff,IFFPrefChunks,IFFPrefChunkCnt))
                        {
                            while (TRUE)
                            {
                                if (error = ParseIFF(iff,IFFPARSE_SCAN))
                                {
                                    if (error == IFFERR_EOF)
                                        result = (headerFlag && dataFlag);

                                    break;
                                }

                                cn = CurrentChunk(iff);

                                if (cn->cn_ID == ID_PRHD && cn->cn_Type == ID_PREF)
                                {
                                    if (ReadChunkBytes(iff,&phead,sizeof(struct PrefHeader)) != sizeof(struct PrefHeader))
                                        break;

                                    if (phead.ph_Version != 0)
                                        break;

                                    headerFlag = TRUE;
                                }
                                else if (cn->cn_ID == ID_LCLE && cn->cn_Type == ID_PREF)
                                {
                                    if (ReadChunkBytes(iff,&locale->el_Prefs,sizeof(struct LocalePrefs)) != sizeof(struct LocalePrefs))
                                        break;

                                    dataFlag = TRUE;
                                }
                            }
                        }
                    }
                }
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    else
    {
        SetIoErr(ERROR_NO_FREE_STORE);
    }

    CloseLibrary(IFFParseBase);

    if (result)
    {
        locale->el_Locale.loc_LocaleName = locale->el_LocaleName;
        strcpy(locale->el_LocaleName,FilePart(name));

        for (i=0; i<10; i++)
            if (locale->el_Prefs.lp_PreferredLanguages[i][0])
                locale->el_Locale.loc_PrefLanguages[i] = locale->el_Prefs.lp_PreferredLanguages[i];

        /* if there are no preferred languages, assume English */
        if (!locale->el_Locale.loc_PrefLanguages[0])
            locale->el_Locale.loc_PrefLanguages[0] = "english";

        for (i=0; i<10; i++)
        {
            if (LoadLanguage(locale,locale->el_Locale.loc_PrefLanguages[i]))
            {
                locale->el_Locale.loc_CodeSet              = GetCodeSet(locale);
                locale->el_Locale.loc_CountryCode          = locale->el_Prefs.lp_CountryData.cp_CountryCode;
                locale->el_Locale.loc_TelephoneCode        = locale->el_Prefs.lp_CountryData.cp_TelephoneCode;
                locale->el_Locale.loc_MeasuringSystem      = locale->el_Prefs.lp_CountryData.cp_MeasuringSystem;
                locale->el_Locale.loc_CalendarType         = locale->el_Prefs.lp_CountryData.cp_CalendarType;

                locale->el_Locale.loc_DateTimeFormat       = locale->el_Prefs.lp_CountryData.cp_DateTimeFormat;
                locale->el_Locale.loc_DateFormat           = locale->el_Prefs.lp_CountryData.cp_DateFormat;
                locale->el_Locale.loc_TimeFormat           = locale->el_Prefs.lp_CountryData.cp_TimeFormat;

                locale->el_Locale.loc_ShortDateTimeFormat  = locale->el_Prefs.lp_CountryData.cp_ShortDateTimeFormat;
                locale->el_Locale.loc_ShortDateFormat      = locale->el_Prefs.lp_CountryData.cp_ShortDateFormat;
                locale->el_Locale.loc_ShortTimeFormat      = locale->el_Prefs.lp_CountryData.cp_ShortTimeFormat;

                locale->el_Locale.loc_DecimalPoint         = locale->el_Prefs.lp_CountryData.cp_DecimalPoint;
                locale->el_Locale.loc_GroupSeparator       = locale->el_Prefs.lp_CountryData.cp_GroupSeparator;
                locale->el_Locale.loc_FracGroupSeparator   = locale->el_Prefs.lp_CountryData.cp_FracGroupSeparator;
                locale->el_Locale.loc_Grouping             = locale->el_Prefs.lp_CountryData.cp_Grouping;
                locale->el_Locale.loc_FracGrouping         = locale->el_Prefs.lp_CountryData.cp_FracGrouping;

                locale->el_Locale.loc_MonDecimalPoint      = locale->el_Prefs.lp_CountryData.cp_MonDecimalPoint;
                locale->el_Locale.loc_MonGroupSeparator    = locale->el_Prefs.lp_CountryData.cp_MonGroupSeparator;
                locale->el_Locale.loc_MonFracGroupSeparator= locale->el_Prefs.lp_CountryData.cp_MonFracGroupSeparator;
                locale->el_Locale.loc_MonGrouping          = locale->el_Prefs.lp_CountryData.cp_MonGrouping;
                locale->el_Locale.loc_MonFracGrouping      = locale->el_Prefs.lp_CountryData.cp_MonFracGrouping;
                locale->el_Locale.loc_MonFracDigits        = locale->el_Prefs.lp_CountryData.cp_MonFracDigits;
                locale->el_Locale.loc_MonIntFracDigits     = locale->el_Prefs.lp_CountryData.cp_MonIntFracDigits;

                locale->el_Locale.loc_MonCS                = locale->el_Prefs.lp_CountryData.cp_MonCS;
                locale->el_Locale.loc_MonSmallCS           = locale->el_Prefs.lp_CountryData.cp_MonSmallCS;
                locale->el_Locale.loc_MonIntCS             = locale->el_Prefs.lp_CountryData.cp_MonIntCS;

                locale->el_Locale.loc_MonPositiveSign      = locale->el_Prefs.lp_CountryData.cp_MonPositiveSign;
                locale->el_Locale.loc_MonPositiveSpaceSep  = locale->el_Prefs.lp_CountryData.cp_MonPositiveSpaceSep;
                locale->el_Locale.loc_MonPositiveSignPos   = locale->el_Prefs.lp_CountryData.cp_MonPositiveSignPos;
                locale->el_Locale.loc_MonPositiveCSPos     = locale->el_Prefs.lp_CountryData.cp_MonPositiveCSPos;

                locale->el_Locale.loc_MonNegativeSign      = locale->el_Prefs.lp_CountryData.cp_MonNegativeSign;
                locale->el_Locale.loc_MonNegativeSpaceSep  = locale->el_Prefs.lp_CountryData.cp_MonNegativeSpaceSep;
                locale->el_Locale.loc_MonNegativeSignPos   = locale->el_Prefs.lp_CountryData.cp_MonNegativeSignPos;
                locale->el_Locale.loc_MonNegativeCSPos     = locale->el_Prefs.lp_CountryData.cp_MonNegativeCSPos;

                locale->el_Locale.loc_GMTOffset            = locale->el_Prefs.lp_GMTOffset;

                InitNumFormat(locale);

                for (i = ABMON_1; i <= ABMON_12; i++)
                    TestLen(locale,i);

                locale->el_MaxDateLen += 2+1+2;

                for (i = DAY_1; i <= DAY_7; i++)
                    TestLen(locale,i);

                TestLen(locale,YESTERDAYSTR);
                TestLen(locale,TODAYSTR);
                TestLen(locale,TOMORROWSTR);
                TestLen(locale,FUTURESTR);

                return(TRUE);
            }
        }

        FreeVec(locale);
    }

    return(FALSE);
}


/*****************************************************************************/


static const struct ExtLocale far DefaultLocale =
{
    "united_states.country",	/* loc_LocaleName	*/
    "",				/* loc_LanguageName	*/
    "english",    		/* loc_PrefLanguages	*/
    NULL,NULL,NULL,
    NULL,NULL,NULL,
    NULL,NULL,NULL,
    0,				/* loc_Flags		*/

    0,				/* loc_CodeSet		*/
    MAKE_ID('U','S','A','\0'),	/* loc_CountryCode	*/
    1,				/* loc_TelephoneCode	*/
    -300,			/* loc_GMTOffset	*/
    MS_AMERICAN,		/* loc_MeasuringSystem	*/
    CT_7SUN,                    /* loc_CalendarType     */
    0,0,			/* loc_Reserved0	*/

    "%A %B %e %Y %Q:%M %p", 	/* loc_DateTimeFormat	*/
    "%A %B %e %Y",	   	/* loc_DateFormat	*/
    "%Q:%M %p",            	/* loc_TimeFormat	*/

    "%m/%d/%y %Q:%M %p", 	/* loc_ShortDateTimeFormat	*/
    "%m/%d/%y",   	   	/* loc_ShortDateFormat	*/
    "%Q:%M %p",            	/* loc_ShortTimeFormat	*/

    ".",			/* loc_DecimalPoint	  */
    ",",			/* loc_GroupSeparator     */
    ",",			/* loc_FracGroupSeparator */
    "\3\0",			/* loc_Grouping		  */
    "\3\0",			/* loc_FracGrouping	  */

    ".",			/* loc_MonDecimalPoint	  */
    ",",			/* loc_MonGroupSeparator  */
    ",",			/* loc_MonFracGroupSeparator  */
    "\3\0",			/* loc_MonGrouping        */
    "\3\0",			/* loc_MonFracGrouping    */
    2,				/* loc_MonFracDigits	*/
    2,				/* loc_MonIntFracDigits	*/
    0,0,			/* loc_Reserved1	  */

    "$",			/* loc_MonCS		*/
    "¢",			/* loc_MonSmallCS	*/
    "USD",			/* loc_MonIntCS		*/

    "",				/* loc_MonPositiveSign	*/
    SS_NOSPACE,			/* loc_MonPositiveSpaceSep */
    SP_PREC_ALL,		/* loc_MonPositiveSignPos  */
    CSP_PRECEDES,		/* loc_MonPositiveCSPos	*/
    0,				/* loc_Reserved2	*/

    "-",			/* loc_MonNegativeSign	*/
    SS_NOSPACE,			/* loc_MonNegativeSpaceSep */
    SP_PREC_ALL,		/* loc_MonNegativeSignPos  */
    CSP_PRECEDES,		/* loc_MonNegativeCSPos	*/
    0,				/* loc_Reserved3	*/

    0,				/* el_UsageCnt		*/
};


/*****************************************************************************/


struct LanguageEntry
{
    STRPTR Name;
    UWORD  Number;
};


static const struct LanguageEntry far entries[]=
{
    {"english",1},
    {"english",2},
    {"deutsch",3},
    {"français",4},
    {"español",5},
    {"italiano",6},
    {"português",7},
    {"dansk",8},
    {"nederlands",9},
    {"norsk",10},
    {"suomi",11},
    {"svenska",12},
    {NULL,0}
};

/* Either IPrefs or an application calls OpenLocale(). On the first call
 * to OpenLocale():
 *
 *   - Initializes itself to English
 *
 *   - Reads UtilityBase->ub_Language
 *
 *   - Attempts to load a language driver matching the
 *     GS language number. If this fails, the internal default
 *     language remains English.
 *
 *   - Sets the environment variable $Language to the current
 *     internal language name.
 *
 * If IPrefs called OpenLocale(), it will then call SetDefaultLocale()
 * with the result of OpenLocale():
 *
 *   - Searches for the locale's language name in its
 *     table. If found, the GS language number for the
 *     current language is poked into
 *     UtilityBase->cb_Language.
 *
 *   - Sets the environment variable $Language to the
 *     locale's language name.
 *
 * It is possible for the UtilityBase->ub_Language field to contain
 * a language number which is unknown to locale.library. Nothing is
 * done to prevent this.
 */


/*****************************************************************************/


struct ExtLocale * ASM OpenLocaleLVO(REG(a0) STRPTR name)
{
struct ExtLocale *result;
STRPTR            language;
UWORD             i;
char              buffer[60];

    LockLib();

    if (!LocaleBase->lb_DefaultLocale)
    {
        LocaleBase->lb_PrefLocale    = DefaultLocale;
        LocaleBase->lb_DefaultLocale = &LocaleBase->lb_PrefLocale;
        InitNumFormat(LocaleBase->lb_DefaultLocale);

        language = "english";
        i = 0;
        while (entries[i].Name)
        {
            if (entries[i].Number == UtilityBase->ub_Language)
            {
                language = entries[i].Name;
                break;
            }
            i++;
        }

        LoadLanguage(LocaleBase->lb_DefaultLocale,language);

        strcpy(buffer,LocaleBase->lb_DefaultLocale->el_Locale.loc_LanguageName);
        buffer[strlen(buffer) - 9] = 0;   /* remove .language extension */

        /* let shell users know what language the system is in */
        SetVar("Language",buffer,-1,LV_VAR|GVF_GLOBAL_ONLY);
    }

    result = LocaleBase->lb_DefaultLocale;

    if (name)
    {
        if (result = AllocVecFlush(sizeof(struct ExtLocale),MEMF_PUBLIC|MEMF_CLEAR))
        {
            if (!LoadLocale(result,name))
            {
                FreeVec(result);
                result = NULL;
            }
        }
    }

    if (result)
        result->el_UsageCnt++;

    UnlockLib();

    return(result);
}


/*****************************************************************************/


VOID ASM CloseLocaleLVO(REG(a0) struct ExtLocale *locale)
{
    if (locale)
    {
        LockLib();

        if (--locale->el_UsageCnt == 0)
        {
            CloseLibrary(locale->el_Language);
            locale->el_Language = NULL;

            if (locale != &LocaleBase->lb_PrefLocale)
                FreeVec(locale);
        }

        UnlockLib();
    }
}


/*****************************************************************************/


struct ExtLocale * ASM SetDefaultLocaleLVO(REG(a0) struct ExtLocale *locale)
{
struct ExtLocale *old;
char              language[32];
UWORD             i;

    LockLib();

    old = LocaleBase->lb_DefaultLocale;
    old->el_UsageCnt--;
    locale->el_UsageCnt++;
    LocaleBase->lb_DefaultLocale = locale;
    PatchOS();

    strcpy(language,locale->el_Locale.loc_LanguageName);
    language[strlen(language) - 9] = 0;   /* remove .language extension */
    SetVar("Language",language,-1,LV_VAR|GVF_GLOBAL_ONLY);

    i = 0;
    while (entries[i].Name)
    {
        if (strcmp(language,entries[i].Name) == 0)
        {
            UtilityBase->ub_Language = entries[i].Number;
            break;
        }
        i++;
    }

    UnlockLib();

    return(old);
}


/*****************************************************************************/


#define IBASE_SYSREQTITLE(ibase)  (*((char **)( ((char *)ibase) +0x60 )))
#define IBASE_WBTITLE(ibase)	  (*((char **)( ((char *)ibase) +0x64 )))

/* lib semaphore should be locked when calling this routine */
static VOID PatchOS(VOID)
{
struct LocaleLib *lib = LocaleBase;
struct ExtLocale *locale;
struct Catalog   *catalog;

    locale = LocaleBase->lb_DefaultLocale;
    locale->el_UsageCnt++;      /* causes this locale to remain in memory for ever */
    LocaleBase->lb_UsageCnt++;  /* causes the lib to remain in memory for ever */
    LocBase = LocaleBase;

    /* this guy can never close :-( */
    catalog = OpenCatalogA(locale,"Sys/dos.catalog",NULL);
    LocaleBase->lb_DOSCatalog = catalog;

    if (!LocaleBase->lb_OldReqTitle)
        LocaleBase->lb_OldReqTitle = IBASE_SYSREQTITLE(IntuitionBase);

    if (!LocaleBase->lb_OldWBTitle)
        LocaleBase->lb_OldWBTitle = IBASE_WBTITLE(IntuitionBase);

    IBASE_SYSREQTITLE(IntuitionBase) = GetCatalogStr(catalog,1000000,lib->lb_OldReqTitle);
    IBASE_WBTITLE(IntuitionBase) = GetCatalogStr(catalog,1000001,lib->lb_OldWBTitle);

    if (!LocaleBase->lb_PatchedOS)
    {
        LocaleBase->lb_OldGetDOSString = (ULONG)SetFunction(DOSBase,-978,(VOID *)GetDOSStringPatch);
        SetFunction(DOSBase,    -750,(VOID *)StrToDateStub);
        SetFunction(DOSBase,    -744,(VOID *)DateToStrStub);
        SetFunction(UtilityBase,-174,(VOID *)ToUpperPatch);
        SetFunction(UtilityBase,-180,(VOID *)ToLowerPatch);
        SetFunction(UtilityBase,-168,(VOID *)StrnicmpPatch);
        SetFunction(UtilityBase,-162,(VOID *)StricmpPatch);
        SetFunction(SysBase,    -522,(VOID *)RawDoFmtPatch);

        LocaleBase->lb_PatchedOS = TRUE;
    }
}


/*****************************************************************************/


VOID ASM LockLib(VOID)
{
struct LocaleLib *lib = LocaleBase;

    ObtainSemaphore(&lib->lb_LibLock);
}


/*****************************************************************************/


VOID ASM UnlockLib(VOID)
{
struct LocaleLib *lib = LocaleBase;

    ReleaseSemaphore(&lib->lb_LibLock);
}


/*****************************************************************************/


/* WARNING: This routine is called from the library's expunge vector and so
 *          runs under Forbid().
 */
VOID ASM FlushLib(VOID)
{
struct LocaleLib *lib = LocaleBase;

    if (AttemptSemaphore(&lib->lb_LibLock))
    {
        if (!LocaleBase->lb_UsageCnt)
            CloseLocale(lib->lb_DefaultLocale);

        FlushCatalogs();

        UnlockLib();
    }
}


/*****************************************************************************/


/* This is needed since a memory panic caused by an allocation failure
 * will have not been able to flush the library for us, since the
 * expunge function of this library tries to get the library semaphore, which
 * it can't get since the caller of this function has it locked already.
 */

APTR AllocVecFlush(ULONG memSize, ULONG memReqs)
{
APTR result;

    if (!(result = AllocVec(memSize,memReqs)))
    {
        FlushLib();

        result = AllocVec(memSize,memReqs);
    }

    return(result);
}
