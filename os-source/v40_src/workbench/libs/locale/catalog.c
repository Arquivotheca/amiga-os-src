
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "localebase.h"
#include "catalog.h"


/*****************************************************************************/


#define ID_CTLG MAKE_ID('C','T','L','G')
#define ID_STRS MAKE_ID('S','T','R','S')
#define ID_CSET MAKE_ID('C','S','E','T')
#define ID_LANG MAKE_ID('L','A','N','G')
#define ID_FVER MAKE_ID('F','V','E','R')

#define IFFPrefChunkCnt 4
static LONG far IFFPrefChunks[] =
{
    ID_CTLG, ID_STRS,
    ID_CTLG, ID_CSET,
    ID_CTLG, ID_FVER,
    ID_CTLG, ID_LANG
};


/*****************************************************************************/


struct CodeSet
{
    ULONG cs_CodeSet;
    ULONG cs_Reserved[7];
};


/*****************************************************************************/


extern struct LocaleLib * far LocBase;


/*****************************************************************************/


static BOOL LoadCatalog(struct ExtCatalog *catalog, STRPTR name,
                        STRPTR language, ULONG reqVersion)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
BOOLEAN             dataFlag = FALSE;
BOOL		    result = FALSE;
LONG                error;
struct Library     *IFFParseBase;
LONG		    bufSize;
ULONG		   *ptr,*lastpoke;
ULONG		    len;
struct CodeSet      codeset;
char		    buffer[100];
ULONG		    version,revision;
LONG		    chars;
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
                    if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_CTLG)
                    {
                        if (!StopChunks(iff,IFFPrefChunks,IFFPrefChunkCnt))
                        {
                            while (TRUE)
                            {
                                if (error = ParseIFF(iff,IFFPARSE_SCAN))
                                {
                                    if (error == IFFERR_EOF)
                                        result = dataFlag;

                                    break;
                                }

                                cn = CurrentChunk(iff);

                                if (cn->cn_ID == ID_STRS && cn->cn_Type == ID_CTLG)
                                {
                                    if (bufSize = cn->cn_Size)
                                    {
                                        if (!(catalog->ec_Strings = AllocVecFlush(bufSize,MEMF_PUBLIC)))
                                            break;

                                        if (ReadChunkBytes(iff,catalog->ec_Strings,bufSize) != bufSize)
                                            break;
                                    }
                                    dataFlag = TRUE;
                                }
                                else if (cn->cn_ID == ID_LANG && cn->cn_Type == ID_CTLG)
                                {
                                    if (cn->cn_Size > 32)
                                        break;

                                    if (ReadChunkBytes(iff,catalog->ec_Language,cn->cn_Size) != cn->cn_Size)
                                        break;

                                    if (strcmp(language,catalog->ec_Language) != 0)
                                        break;
                                }
                                else if (cn->cn_ID == ID_FVER && cn->cn_Type == ID_CTLG)
                                {
                                    if (cn->cn_Size > 100)
                                        break;

                                    if (ReadChunkBytes(iff,buffer,cn->cn_Size) != cn->cn_Size)
                                        break;

                                    buffer[99] = 0;
                                    i = 0;
                                    while (buffer[i] && (buffer[i] != ' '))
                                        i++;
                                    while (buffer[i] && (buffer[i] == ' '))
                                        i++;
                                    while (buffer[i] && (buffer[i] != ' '))
                                        i++;

                                    if (buffer[i])
                                    {
                                        if ((chars = StrToLong(&buffer[i],&version)) < 0)
                                            break;

                                        i += chars;
                                        if (buffer[i++])
                                        {
                                            if (StrToLong(&buffer[i],&revision) < 0)
                                                break;
                                        }

                                        catalog->ec_Catalog.cat_Version  = version;
                                        catalog->ec_Catalog.cat_Revision = revision;

                                        if (reqVersion && (version != reqVersion))
                                            break;
                                    }
                                }
                                else if (cn->cn_ID == ID_CSET && cn->cn_Type == ID_CTLG)
                                {
                                    if (ReadChunkBytes(iff,&codeset,sizeof(struct CodeSet)) != sizeof(struct CodeSet))
                                        break;

                                    catalog->ec_Catalog.cat_CodeSet = codeset.cs_CodeSet;
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
        catalog->ec_Catalog.cat_Language = catalog->ec_Language;

        /* now zip through buffer, changing the length longs to be pointers
         * to the next string instead. This speeds up the string searching
         * code as it doesn't need to add and compensate for padding as we do
         * that here.
         */

        if (ptr = catalog->ec_Strings)
        {
            lastpoke = NULL;
            while (bufSize)
            {
                ptr++;                      // skip id longword
                len = *ptr;                 // get length
                len = ((len+3) / 4) * 4;    // round up to multiple of 4
                *ptr = (ULONG)ptr+len+4;    // change length to point to next string
                lastpoke = ptr;
                ptr = (ULONG *)*ptr;
                bufSize -= len+8;
            }

            if (lastpoke)
                *lastpoke = NULL;
        }
    }
    else
    {
        FreeVec(catalog->ec_Strings);
        catalog->ec_Strings = NULL;
    }

    return(result);
}


/*****************************************************************************/


struct ExtCatalog * ASM OpenCatalogALVO(REG(a0) struct ExtLocale *locale,
				        REG(a1) STRPTR name,
				        REG(a2) struct TagItem *tags)
{
struct LocaleLib  *lib = LocaleBase;
struct ExtCatalog *result = NULL;
struct Process    *process;
char		   path[100];
UWORD		   i,maxLoop;
STRPTR             lang;
STRPTR             builtInLanguage;
UWORD		   version;
BOOLEAN		   openedLoc = FALSE;
BPTR               lock;
BOOLEAN            doProgDir;
UBYTE              cnt;
APTR               oldWP;

    if (!locale)
    {
        locale    = (struct ExtLocale *)OpenLocale(NULL);
        openedLoc = TRUE;
    }

/*
    builtInCodeSet  = (ULONG) GetTagData(OC_BuiltInCodeSet,0,tags);
*/
    builtInLanguage = (STRPTR)GetTagData(OC_BuiltInLanguage,(ULONG)"english",tags);
    version         = (UWORD) GetTagData(OC_Version,0,tags);
    lang            = (STRPTR)GetTagData(OC_Language,0,tags);

    maxLoop = 10;
    if (lang)
        maxLoop++;

    LockLib();

    for (i=0; i<maxLoop; i++)
    {
        if (lang)
        {
            if (builtInLanguage && (strcmp(lang,builtInLanguage) == 0))
            {
                /* preferred language matches built-in language */
                break;
            }
            else
            {
                /* is the needed catalog already in memory? */
                result = (struct ExtCatalog *)LocaleBase->lb_Catalogs.mlh_Head;
                while (result->ec_Catalog.cat_Link.ln_Succ)
                {
                    if (Stricmp(name,result->ec_Catalog.cat_Link.ln_Name) == 0)
                    {
                        if (((version == 0) || (result->ec_Catalog.cat_Version == version))
                        && (Stricmp(lang,result->ec_Catalog.cat_Language) == 0))
                            break;
                    }

                    result = (struct ExtCatalog *)result->ec_Catalog.cat_Link.ln_Succ;
                }

                /* did we actually find it in memory? */
                if (result->ec_Catalog.cat_Link.ln_Succ)
                {
                    /* make it faster to find this node the next time around */
                    Remove(result); /* added at head of list on exit of the loop */
                    break;
                }

                if (!(result = AllocVecFlush(sizeof(struct ExtCatalog)+strlen(name)+1,MEMF_PUBLIC|MEMF_CLEAR)))
                    break;

                result->ec_Catalog.cat_Link.ln_Name = (STRPTR)((ULONG)result+sizeof(struct ExtCatalog));
                strcpy(result->ec_Catalog.cat_Link.ln_Name,name);

                process = (struct Process *)FindTask(NULL);
                oldWP = process->pr_WindowPtr;
                process->pr_WindowPtr = (APTR)-1;

                doProgDir = TRUE;
                if (!(lock = Lock("PROGDIR:",ACCESS_READ)))
                    if (lock = Lock("LOCALE:",ACCESS_READ))
                        doProgDir = FALSE;
                UnLock(lock);

                process->pr_WindowPtr = oldWP;

                cnt = 3;
                while (--cnt)
                {
                    if (doProgDir)
                    {
                        doProgDir = FALSE;
                        strcpy(path,"PROGDIR:Catalogs");
                        AddPart(path,lang,sizeof(path));
                        AddPart(path,name,sizeof(path));

                        if (GetProgramDir() && LoadCatalog(result,path,lang,version))
                            break;
                    }
                    else
                    {
                        doProgDir = TRUE;
                        strcpy(path,"LOCALE:Catalogs");
                        AddPart(path,lang,sizeof(path));
                        AddPart(path,name,sizeof(path));

                        if (LoadCatalog(result,path,lang,version))
                            break;
                    }
                }

                /* we found something! */
                if (cnt)
                    break;

                FreeVec(result);
                result = NULL;
            }
        }

        if (i < 10)
            lang = locale->el_Locale.loc_PrefLanguages[i];
    }

    if (result)
    {
        AddHead((struct List *)&lib->lb_Catalogs,result);
        result->ec_UsageCnt++;
    }

    UnlockLib();

    if (openedLoc)
        CloseLocale(locale);

    return(result);
}


/*****************************************************************************/


VOID ASM CloseCatalogLVO(REG(a0) struct ExtCatalog *catalog)
{
    if (catalog)
        catalog->ec_UsageCnt--;
}


/*****************************************************************************/


/* WARNING: This routine is called from the library's expunge vector and so
 *          runs under Forbid().
 *
 *	    It is also called, and must be called, with the library
 *	    semaphore locked.
 */
VOID ASM FlushCatalogs(VOID)
{
struct ExtCatalog *node;
struct ExtCatalog *next;

    node = (struct ExtCatalog *)LocaleBase->lb_Catalogs.mlh_Head;
    while (next = (struct ExtCatalog *)node->ec_Catalog.cat_Link.ln_Succ)
    {
        if (!node->ec_UsageCnt)
        {
            Remove(node);
            FreeVec(node->ec_Strings);
            FreeVec(node);
        }

        node = next;
    }
}
