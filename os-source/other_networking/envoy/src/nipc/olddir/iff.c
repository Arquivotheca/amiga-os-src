#include <exec/types.h>
#include <dos/dos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include "nipc.h"
#include "nipcinternal.h"
#include "nipcinternal_protos.h"
#include "externs.h"

BOOL OpenConfig(void)
{
    register struct NBase *nb = gb;
    struct ContextNode *top;
    ULONG error;
    BOOL cont = TRUE;

    LONG props[]={ID_PREF,ID_NDEV,
                         ID_PREF,ID_NRRM,
                         ID_PREF,ID_NLRM,
                         ID_PREF,ID_NIRT,
                         ID_PREF,ID_HOST};

    if(nb->nb_IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
        if(nb->IFFStream = PrefOpen())
        {
            if(nb->iff = AllocIFF())
            {
                InitIFFasDOS(nb->iff);
                if(!OpenIFF(nb->iff,IFFF_READ))
                {
                    nb->iff->iff_Stream = (ULONG) nb->IFFStream;
                    if(!CollectionChunks(nb->iff,props,5))
                    {
                        while(cont)
                        {
                            error = ParseIFF(nb->iff,IFFPARSE_STEP);
                            if(!error)
                                continue;
                            if(error != IFFERR_EOC)
                                break;

                            top = CurrentChunk(nb->iff);

                            if((top->cn_Type == ID_PREF) && (top->cn_ID == ID_FORM))
                                return TRUE;
                        }
                    }
                }
            }
        }
    }
    CloseConfig();
    return FALSE;
}

BPTR PrefOpen(void)
{
    BPTR stream;

    if(stream = Open("ENV:envoy/nipc.prefs",MODE_OLDFILE))
        return stream;
    else if(stream = Open("ENVARC:envoy/nipc.prefs",MODE_OLDFILE))
        return stream;
    else
        return (BPTR)0L;
}

void CloseConfig(void)
{
    register struct NBase *nb = gb;

    if(nb->iff)
    {
        CloseIFF(nb->iff);
        FreeIFF(nb->iff);
        if(nb->IFFStream)
        {
            Close(nb->IFFStream);
            if(nb->nb_IFFParseBase);
            {
                CloseLibrary(nb->nb_IFFParseBase);
                nb->nb_IFFParseBase = NULL;
            }
            nb->IFFStream = NULL;
        }
        nb->iff = NULL;
    }
}
