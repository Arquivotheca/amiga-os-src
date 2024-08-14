/* catalog.c - for using a catalog under V37 without locale.library
 * Also requires getcatalogstr.asm
 *
 * Requires these externals from the application it is linked with:
 *
 *  extern struct Library *LocaleBase  (NULL under <V38)
 *  extern struct Library *UtilityBase (required)
 *  for XGetString function:
 *  extern struct Catalog *catalog
 *  extern struct CatCompArrayType *StringArray;
 *  extern ULONG  StringCount;
 *
 */

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
#include <clib/locale_protos.h>
#include <clib/utility_protos.h>

/* direct ROM interface */
extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/utility_pragmas.h>

/* application includes */
#include "catalog.h"

/* defined in application */
extern struct Library *LocaleBase;
extern struct Library *UtilityBase;
extern struct Catalog *catalog;
extern struct CatCompArrayType *StringArray;
extern ULONG  StringCount;

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



/*****************************************************************************/


static BOOL LoadCatalog(struct XCatalog *catalog, STRPTR name,
                        STRPTR language, ULONG reqVersion)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
BOOL             dataFlag = FALSE;
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

    if (!(IFFParseBase = OpenLibrary("iffparse.library",0)))
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
                                        if (!(catalog->ec_Strings = AllocVec(bufSize,MEMF_PUBLIC)))
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

                                    if (ReadChunkBytes(iff,catalog->ec_LanguageStr,cn->cn_Size) != cn->cn_Size)
                                        break;

                                    if (strcmp(language,catalog->ec_LanguageStr) != 0)
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

                                        catalog->ec_Version  = version;
                                        catalog->ec_Revision = revision;

                                        if (reqVersion && (version != reqVersion))
                                            break;
                                    }
                                }
                                else if (cn->cn_ID == ID_CSET && cn->cn_Type == ID_CTLG)
                                {
                                    if (ReadChunkBytes(iff,&codeset,sizeof(struct CodeSet)) != sizeof(struct CodeSet))
                                        break;

                                    catalog->ec_CodeSet = codeset.cs_CodeSet;
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
        catalog->ec_Language = catalog->ec_LanguageStr;

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


/* tags on stack interface XOpenCatalog */

struct Catalog *XOpenCatalog(struct Locale *locale, STRPTR name, ULONG tags, ...)
    {
    return(XOpenCatalogA(locale, name, (struct TagItem *)&tags));
    }

/* pointer to tag list interface XOpenCatalogA */
struct Catalog *XOpenCatalogA(struct Locale *locale,
				 STRPTR name,
				 struct TagItem *tags)
{
struct XCatalog *result = NULL;
struct Process    *process;
char		   path[100];
STRPTR             lang;
STRPTR             builtInLanguage;
UWORD		   version;
BPTR               lock;
BOOL               doProgDir;
UBYTE              cnt;
APTR               oldWP;

    if(DOSBase->lib_Version < 36)	return(NULL);

    if(LocaleBase)	return(OpenCatalogA(locale,name,tags));

    builtInLanguage = (STRPTR)GetTagData(OC_BuiltInLanguage,(ULONG)"english",tags);
    version         = (UWORD) GetTagData(OC_Version,0,tags);
    lang            = (STRPTR)GetTagData(OC_Language,0,tags);


    /* No LocaleBase, no user prefs. Can't load without explicit Language tag */
    if (!lang)		return(NULL);

    /* Preferred language matches built-in language */
    if (builtInLanguage && (strcmp(lang,builtInLanguage) == 0)) return(NULL);

    /* Try explicit load */

    if (!(result = AllocVec(sizeof(struct XCatalog)+strlen(name)+1,MEMF_PUBLIC|MEMF_CLEAR)))
                return(NULL);

    result->ec_Link.ln_Name = (STRPTR)((ULONG)result+sizeof(struct XCatalog));
    strcpy(result->ec_Link.ln_Name,name);

    process = (struct Process *)FindTask(NULL);
    oldWP = process->pr_WindowPtr;
    process->pr_WindowPtr = (APTR)-1;

    /* To match the documented search path for catalogs */
    doProgDir = TRUE;
    if (!(lock = Lock("PROGDIR:",ACCESS_READ)))
          if (lock = Lock("LOCALE:",ACCESS_READ))
                doProgDir = FALSE;
    if(lock) UnLock(lock);

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

    if(!cnt)
	{
    	FreeVec(result);
    	result = NULL;
    	}

    return((struct Catalog *)result);
}


/*****************************************************************************/

VOID XCloseCatalog(struct Catalog *catalog)
{
    if (catalog)
    {
    if(LocaleBase)	CloseCatalog(catalog);
    else
	{
        FreeVec(((struct XCatalog *)catalog)->ec_Strings); /* can accept NULL */
        FreeVec(catalog);
	}
    }
}

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static UBYTE *NULLSTRING = "";

/* references external StringArray (pointer to CatCompArray)
 *                     StringCount
 *                     catalog
 */
STRPTR XGetString(LONG id)
    {
    struct CatCompArrayType *cca;
    UBYTE *s = NULLSTRING;
    ULONG k;

    cca = StringArray;

    for(k=0; k<StringCount; k++, cca++)
	{
	if(cca->cca_ID == id)
	    {
	    s = cca->cca_Str;
	    break;
	    }
	}

    if((catalog)&&(*s))
	{
	s = XGetCatalogStr(catalog,id,s);
	}

    return(s);
    }

