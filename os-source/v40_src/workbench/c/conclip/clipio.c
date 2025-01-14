
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "conclip.h"
#include "clipio.h"
#include "stripansi.h"


/*****************************************************************************/


#define ID_FTXT MAKE_ID('F','T','X','T')
#define ID_CHRS MAKE_ID('C','H','R','S')


/*****************************************************************************/


extern struct Library  *SysBase;
extern struct Device   *ConsoleBase;
extern ULONG            clipUnit;

struct IFFHandle       *iffHandle;
struct ClipboardHandle *clipHandle;
struct Library         *IFFParseBase;


/*****************************************************************************/


static VOID NewList(struct List *list)
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)list;
}


/*****************************************************************************/


static LONG PrepareClipIO(VOID)
{
LONG error;

    error = ERR_NOIFFPARSE;
    if (IFFParseBase = OpenLibrary("iffparse.library",39))
    {
        error = ERR_NOMEMORY;
        if (iffHandle = AllocIFF())
	{
            InitIFFasClip(iffHandle);

            error = ERR_NOCLIPBOARD;
	    if (clipHandle = OpenClipboard(clipUnit))
	    {
                iffHandle->iff_Stream = (ULONG)clipHandle;

                return(ERR_NOERROR);
            }
            FreeIFF(iffHandle);
        }
        CloseLibrary(IFFParseBase);
    }

    return(error);
}


/*****************************************************************************/


static VOID CleanupClipIO(VOID)
{
    CloseClipboard(clipHandle);
    FreeIFF(iffHandle);
    CloseLibrary(IFFParseBase);
}


/*****************************************************************************/


LONG GetChars(struct CmdMsg *cm)
{
struct CHRSChunk *chrsCurr;
LONG              error;
LONG              size;

    NewList((struct List *)&cm->cm_Chunks);	/* empty list to start */

    if (cm->cm_Error = PrepareClipIO())
	return(cm->cm_Error);

    cm->cm_Error = ERR_BADIFF;
    if (!OpenIFF(iffHandle, IFFF_READ))
    {
        if (!StopChunk(iffHandle, ID_FTXT, ID_CHRS))
        {
            do
	    {
		if (!(error = ParseIFF(iffHandle, IFFPARSE_SCAN)))
		{
		    size = CurrentChunk(iffHandle)->cn_Size;

		    chrsCurr = (struct CHRSChunk *)
		        AllocVec(size + 1 + sizeof(struct CHRSChunk),
			    MEMF_PUBLIC|MEMF_CLEAR);

		    if (chrsCurr)
		    {
			if (size == ReadChunkBytes(iffHandle,chrsCurr->cc_Data,size))
			{
			    StripANSI(&chrsCurr->cc_Data[0],size);

			    /* if after stripping we get a 0 size, throw away */

			    if (chrsCurr->cc_Size = strlen(&chrsCurr->cc_Data[0]))
			    {
			        chrsCurr->cc_Text       = &chrsCurr->cc_Data[0];
			        chrsCurr->cc_FreeHandle = 0;

			        AddTail((struct List *)&cm->cm_Chunks,
					(struct Node *)&chrsCurr->cc_Link);

			    }
			    else
			    {
                                FreeVec(chrsCurr);
			    }
			}
			else
			{
			    FreeVec(chrsCurr);
			    cm->cm_Error = ERR_BADIFF;
			    error        = 1;
			}
		    }
		    else
		    {
	    		cm->cm_Error = ERR_NOMEMORY;
	    		error        = 1;
		    }
		}
	    }
            while (!error);

	    if (error == IFFERR_EOF)
    	    {
		cm->cm_Error = ERR_NOERROR;
	    }
	    else
	    {
		while (chrsCurr = (struct CHRSChunk *)RemHead((struct List *)&cm->cm_Chunks))
		    FreeVec(chrsCurr);
	    }
	}
        CloseIFF(iffHandle);
    }

    CleanupClipIO();

    return (cm->cm_Error);
}


/*****************************************************************************/


LONG PutChars(VOID)
{
char *chrsData;
LONG  chrsLen;
LONG  error;

    if (error = PrepareClipIO())
	return(error);

    error = ERR_NOMEMORY;

    if (chrsData = (char *)GetConSnip())
    {
	chrsLen = strlen(chrsData);

	error = ERR_BADIFF;

        if (!OpenIFF(iffHandle, IFFF_WRITE))
        {
	    if (!PushChunk(iffHandle, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))
	    {
		if (!PushChunk(iffHandle, 0, ID_CHRS, IFFSIZE_UNKNOWN))
		{
		    if (WriteChunkBytes(iffHandle, chrsData, chrsLen) == chrsLen)
		    {
                        if (!PopChunk(iffHandle))
                        {
                            if (!PopChunk(iffHandle))
                            {
                                error = ERR_NOERROR;
                            }
                        }
                    }
                }
	    }
            CloseIFF(iffHandle);
        }
        FreeVec(chrsData);
    }

    CleanupClipIO();

    return (error);
}
