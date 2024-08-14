
#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/asl.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "texttable.h"
#include "requestfile_rev.h"


/*****************************************************************************/


#define TEMPLATE        "DRAWER,FILE/K,PATTERN/K,TITLE/K,POSITIVE/K,NEGATIVE/K,ACCEPTPATTERN/K,REJECTPATTERN/K,SAVEMODE/S,MULTISELECT/S,DRAWERSONLY/S,NOICONS/S,PUBSCREEN/K" VERSTAG
#define OPT_DRAWER      0
#define OPT_FILE        1
#define OPT_PATTERN     2
#define OPT_TITLE       3
#define OPT_POSITIVE    4
#define OPT_NEGATIVE    5
#define OPT_ACCEPTPAT   6
#define OPT_REJECTPAT   7
#define OPT_SAVEMODE    8
#define OPT_MULTISELECT 9
#define OPT_DRAWERSONLY 10
#define OPT_NOICONS     11
#define OPT_PUBSCREEN   12
#define OPT_COUNT       13


/*****************************************************************************/


#define LocaleBase li.li_LocaleBase
#define catalog    li.li_Catalog
BOOL GetFile(struct Library *AslBase, struct FileRequester *fr, ULONG tags, ...);


/*****************************************************************************/


LONG main(VOID)
{
struct Library       *SysBase = (*((struct Library **) 4));
struct Library       *DOSBase;
struct Library       *AslBase;
LONG                  opts[OPT_COUNT];
struct RdArgs        *rdargs;
LONG                  failureCode;
LONG                  failureLevel;
struct FileRequester *fr;
char                  result[300];
char                  parsepat[300];
struct LocaleInfo     li;
STRPTR                title;
STRPTR                positive;
STRPTR                negative;
STRPTR                file;
STRPTR                drawer;
STRPTR                pattern;
STRPTR                acceptpat;
STRPTR                rejectpat;
ULONG                 cnt;

    failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel = RETURN_FAIL;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (LocaleBase = OpenLibrary("locale.library",38))
            li.li_Catalog = OpenCatalogA(NULL,"sys/c.catalog",NULL);

        if (AslBase = OpenLibrary("asl.library",38))
        {
            if (fr = AllocAslRequest(ASL_FileRequest,NULL))
            {
                memset(opts,0,sizeof(opts));
                if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                {
                    title     = "Select File";
                    positive  = "Ok";
                    negative  = "Cancel";
                    file      = "";
                    drawer    = "";
                    pattern   = "#?";
                    acceptpat = "#?";
                    rejectpat = "~(#?)";

                    if (opts[OPT_TITLE])
                        title = (STRPTR)opts[OPT_TITLE];

                    if (opts[OPT_POSITIVE])
                        positive = (STRPTR)opts[OPT_POSITIVE];

                    if (opts[OPT_NEGATIVE])
                        negative = (STRPTR)opts[OPT_NEGATIVE];

                    if (opts[OPT_FILE])
                        file = (STRPTR)opts[OPT_FILE];

                    if (opts[OPT_DRAWER])
                        drawer = (STRPTR)opts[OPT_DRAWER];

                    if (opts[OPT_PATTERN])
                        pattern = (STRPTR)opts[OPT_PATTERN];

                    if (opts[OPT_ACCEPTPAT])
                        acceptpat = (STRPTR)opts[OPT_ACCEPTPAT];

                    if (opts[OPT_REJECTPAT])
                        rejectpat = (STRPTR)opts[OPT_REJECTPAT];

                    /* using "result" as a temp storage area for the parsed pattern */
                    ParsePatternNoCase(acceptpat,result,sizeof(result));
                    ParsePatternNoCase(rejectpat,parsepat,sizeof(parsepat));

                    if (GetFile(AslBase,fr,ASLFR_PubScreenName, opts[OPT_PUBSCREEN],
                                           ASLFR_AcceptPattern, result,
                                           ASLFR_RejectPattern, parsepat,
                                           ASLFR_TitleText,     title,
                                           ASLFR_PositiveText,  positive,
                                           ASLFR_NegativeText,  negative,
                                           ASLFR_InitialFile,   file,
                                           ASLFR_InitialDrawer, drawer,
                                           ASLFR_InitialPattern,pattern,
                                           ASLFR_DoMultiSelect, opts[OPT_MULTISELECT],
                                           ASLFR_DoSaveMode,    opts[OPT_SAVEMODE],
                                           ASLFR_DoPatterns,    opts[OPT_PATTERN] != NULL,
                                           ASLFR_DrawersOnly,   opts[OPT_DRAWERSONLY],
                                           ASLFR_RejectIcons,   opts[OPT_NOICONS],
                                           TAG_DONE))
                    {
                        if (opts[OPT_MULTISELECT])
                        {
                            cnt = 0;
                            while (cnt < fr->fr_NumArgs)
                            {
                                strncpy(result,fr->fr_Drawer,sizeof(result));
                                AddPart(result,fr->fr_ArgList[cnt].wa_Name,sizeof(result));
                                if (cnt > 0)
                                    PutStr(" \"");
                                else
                                    PutStr("\"");
                                PutStr(result);
                                PutStr("\"");
                                cnt++;
                            }

                            PutStr("\n");
                        }
                        else
                        {
                            strncpy(result,fr->fr_Drawer,sizeof(result));
                            AddPart(result,fr->fr_File,sizeof(result));
                            PutStr("\"");
                            PutStr(result);
                            PutStr("\"\n");
                        }

                        failureCode  = 0;
                        failureLevel = RETURN_OK;
                    }
                    else
                    {
                        if (failureCode = IoErr())
                        {
                            PrintFault(failureCode,NULL);
                        }
                        else
                        {
                            failureCode = ERROR_BREAK;
                            failureLevel = RETURN_WARN;
                        }
                    }
                    FreeArgs(rdargs);
                }
                else
                {
                    failureCode = IoErr();
                    PrintFault(failureCode,NULL);
                }
                FreeAslRequest(fr);
            }
            else
            {
                failureCode = ERROR_NO_FREE_STORE;
                PrintFault(failureCode,NULL);
            }

            CloseLibrary(AslBase);
        }
        else
        {
/*            PutStr(GetString(MSG_RF_NOASL_ERROR)); */
        }

        if (LocaleBase)
        {
            CloseCatalog(catalog);
            CloseLibrary(LocaleBase);
        }

        SetIoErr(failureCode);

        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


BOOL GetFile(struct Library *AslBase, struct FileRequester *fr, ULONG tags, ...)
{
    return(AslRequest(fr,(struct TagItem *)&tags));
}
