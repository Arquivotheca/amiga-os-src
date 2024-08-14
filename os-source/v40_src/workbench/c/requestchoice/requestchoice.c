
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <intuition/intuition.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "requestchoice_rev.h"


/*****************************************************************************/


#define TEMPLATE      "TITLE/A,BODY/A,GADGETS/M,PUBSCREEN/K" VERSTAG
#define OPT_TITLE     0
#define OPT_BODY      1
#define OPT_GADGETS   2
#define OPT_PUBSCREEN 3
#define OPT_COUNT     4


/*****************************************************************************/


struct Window *GetWindow(struct Library *IntuitionBase, ULONG tags, ...);


/*****************************************************************************/


LONG main(VOID)
{
struct Library    *SysBase = (*((struct Library **) 4));
struct Library    *DOSBase;
struct Library    *IntuitionBase;
LONG               opts[OPT_COUNT];
struct RdArgs     *rdargs;
LONG               failureCode;
LONG               failureLevel;
struct EasyStruct  es;
struct Window     *window;
STRPTR             body;
STRPTR            *gadgets;
STRPTR             gad;
UWORD              len;
UWORD              i;
UWORD              j;
STRPTR             newbody;
STRPTR             newgadgets;
char               buf[16];

    failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel = RETURN_FAIL;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (IntuitionBase = OpenLibrary("intuition.library",37))
        {
            memset(opts,0,sizeof(opts));
            if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
            {
                body    = (STRPTR)opts[OPT_BODY];
                gadgets = (STRPTR *)opts[OPT_GADGETS];

                if (gadgets && *gadgets)
                {
                    len = strlen(body);
                    while (gad = *gadgets)
                    {
                        len += strlen(gad) + 1;

                        i = 0;
                        while (gad[i])
                            if (gad[i++] == '%')
                                len++;

                        gadgets++;
                    }

                    i = 0;
                    while (body[i])
                        if (body[i++] == '%')
                            len++;

                    failureCode = ERROR_NO_FREE_STORE;
                    if (newbody = AllocVec(len+1,MEMF_ANY))
                    {
                        i = 0;
                        j = 0;
                        while (body[i])
                        {
                            newbody[j++] = body[i];
                            if (body[i++] == '%')
                                newbody[j++] = '%';
                        }
                        newbody[j++] = 0;
                        newgadgets = &newbody[j];

                        gadgets = (STRPTR *)opts[OPT_GADGETS];
                        while (gad = *gadgets)
                        {
                            i = 0;
                            while (gad[i])
                            {
                                newbody[j++] = gad[i];
                                if (gad[i++] == '%')
                                    newbody[j++] = '%';
                            }
                            gadgets++;

                            if (*gadgets)
                                newbody[j++] = '|';
                        }
                        newbody[j] = 0;

                        if (window = GetWindow(IntuitionBase,WA_PubScreenName, opts[OPT_PUBSCREEN],
                                                             WA_Left,          0,
                                                             WA_Top,           0,
                                                             WA_Width,         16,
                                                             WA_Height,        16,
                                                             WA_Backdrop,      TRUE,
                                                             WA_Borderless,    TRUE,
                                                             WA_NoCareRefresh, TRUE,
                                                             WA_RMBTrap,       TRUE,
                                                             TAG_DONE))
                        {
                            es.es_StructSize   = sizeof(struct EasyStruct);
                            es.es_Flags        = 0;
                            es.es_Title        = (STRPTR)opts[OPT_TITLE];
                            es.es_TextFormat   = newbody;
                            es.es_GadgetFormat = newgadgets;

                            failureCode  = EasyRequestArgs(window,&es,NULL,NULL);
                            failureLevel = RETURN_OK;

                            sprintf(buf,"%ld\n",failureCode);
                            PutStr(buf);

                            CloseWindow(window);
                        }
                    }
                }
                else
                {
                    failureCode = ERROR_REQUIRED_ARG_MISSING;
                }
            }
            else
            {
                failureCode = IoErr();
            }

            CloseLibrary(IntuitionBase);
        }

        SetIoErr(failureCode);
        if (failureLevel != RETURN_OK)
            PrintFault(failureCode,NULL);

        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


struct Window *GetWindow(struct Library *IntuitionBase, ULONG tags, ...)
{
    return(OpenWindowTagList(NULL,(struct TagItem *)&tags));
}
