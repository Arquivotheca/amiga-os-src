
/* includes */
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <prefs/prefhdr.h>

/* prototypes */
#include <clib/iffparse_protos.h>
#include <clib/dos_protos.h>

/* direct ROM interface */
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* application includes */
#include "pe_iff.h"
#include "pe_custom.h"
#include "pe_utils.h"


/*****************************************************************************/


struct IFFHandle *GetIFF(EdDataPtr ed, STRPTR name, LONG mode, EdStatus *result)
{
struct IFFHandle *iff;
LONG              error;

    *result = ES_NO_MEMORY;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = (ULONG)Open(name,mode))
        {
            InitIFFasDOS(iff);

            if (!(error = OpenIFF(iff, (mode == MODE_OLDFILE)? IFFF_READ : IFFF_WRITE)))
	        return(iff);

            ed->ed_SecondaryResult = error;

            Close((BPTR)iff->iff_Stream);
        }
        else
        {
            *result = ES_DOSERROR;
            ed->ed_SecondaryResult = IoErr();
        }
        FreeIFF(iff);
    }
    return(NULL);
}


/*****************************************************************************/


VOID ReturnIFF(EdDataPtr ed, struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}


/*****************************************************************************/


EdStatus ReadIFF(EdDataPtr ed, STRPTR name, ULONG *stopChunks, ULONG chunkCnt,
                 IFFFunc readFunc)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
EdStatus            result = ES_IFFERROR;
BOOL                headerFlag = FALSE;
struct PrefHeader   phead;
LONG                error;

    ed->ed_ErrorFileName = name;

    if (iff = GetIFF(ed,name,MODE_OLDFILE,&result))
    {
        result = ES_IFFERROR;
        if (!ParseIFF(iff,IFFPARSE_STEP))
        {
            cn = CurrentChunk(iff);
            if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_PREF)
            {
                if (!StopChunks(iff,stopChunks,chunkCnt))
                {
                    while (TRUE)
                    {
                        if (error = ParseIFF(iff,IFFPARSE_SCAN))
                        {
                            if (error == IFFERR_EOF)
                            {
                                if (headerFlag)
                                    result = ES_NORMAL;
                            }
                            break;
                        }

                        cn = CurrentChunk(iff);

                        if (cn->cn_ID == ID_PRHD && cn->cn_Type == ID_PREF)
                        {
                            if (ReadChunkBytes(iff,&phead,sizeof(struct PrefHeader)) != sizeof(struct PrefHeader))
                            {
                                break;
                            }

                            headerFlag = TRUE;
                        }
                        else
                        {
                            result = readFunc(ed,iff,cn);
                            if ((result != ES_NORMAL) && (result != ES_IFF_UNKNOWNCHUNK))
                                break;
                        }
                    }
                }
                else
                {
                    result = ES_IFFERROR;
                }
            }
            else
            {
                result = ES_IFF_UNKNOWNCHUNK;
            }
        }
        ReturnIFF(ed,iff);
    }

    return(result);
}


/*****************************************************************************/


EdStatus WriteIFF(EdDataPtr ed, STRPTR name, APTR hdr, IFFFunc writeFunc)
{
struct IFFHandle *iff;
EdStatus          result;

    ed->ed_ErrorFileName = name;

    result = ES_IFFERROR;
    if (iff = GetIFF(ed,name,MODE_NEWFILE,&result))
    {
        if (!PushChunk(iff,ID_PREF,ID_FORM,IFFSIZE_UNKNOWN))
        {
            if (!PushChunk(iff,0,ID_PRHD,sizeof(struct PrefHeader)))
            {
                if (WriteChunkBytes(iff,hdr,sizeof(struct PrefHeader)) == sizeof(struct PrefHeader))
                {
                    if (!PopChunk(iff))
                    {
                        if ((result = writeFunc(ed,iff,NULL)) == ES_NORMAL)
                        {
		            if (PopChunk(iff))
		            {
		                result = ES_IFFERROR;
		            }
                        }
                    }
                }
            }
        }
        ReturnIFF(ed,iff);
        SetProtection(name,FIBF_EXECUTE);
    }

    return(result);
}
