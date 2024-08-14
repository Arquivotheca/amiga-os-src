
/* includes */
#include <exec/types.h>
#include <utility/tagitem.h>
#include <dos/dos.h>

#include <clib/dos_protos.h>

#include <pragmas/dos_pragmas.h>

/* application includes */
#include "asl.h"
#include "aslbase.h"
#include "requtils.h"
#include "filereq.h"
#include "fontreq.h"
#include "screenmodereq.h"


/*****************************************************************************/


APTR ASM AllocAslRequest(REG(d0) ULONG reqType,
                         REG(a0) struct TagItem *tagList,
                         REG(a6) struct ASLLib *AslBase)
{
struct ReqInfo *req;

    switch (reqType)
    {
        case ASL_FileRequest      : req = (APTR)AllocFileRequest(AslBase,tagList);
                                    break;

        case ASL_FontRequest      : req = (APTR)AllocFontRequest(AslBase,tagList);
                                    break;

        case ASL_ScreenModeRequest: req = (APTR)AllocSMRequest(AslBase,tagList);
                                    break;

        default                   : return(NULL);
    }

    if (req)
    {
        req->ri_ReqType = reqType;
        return((APTR)((ULONG)req+sizeof(struct ReqInfo)));
    }

    return(NULL);
}


/*****************************************************************************/


VOID ASM FreeAslRequest(REG(a0) APTR requester,
                        REG(a6) struct ASLLib *AslBase)
{
struct ReqInfo *req;

    if (requester)
    {
        req = (struct ReqInfo *)((ULONG)requester - sizeof(struct ReqInfo));

        switch (req->ri_ReqType)
        {
            case ASL_FileRequest      : FreeFileRequest(AslBase,(struct ExtFileReq *)req);
                                        break;

            case ASL_FontRequest      : FreeFontRequest(AslBase,(struct ExtFontReq *)req);
                                        break;

            case ASL_ScreenModeRequest: FreeSMRequest(AslBase,(struct ExtSMReq *)req);
                                        break;
        }
    }
}


/*****************************************************************************/


APTR ASM AslRequest(REG(a0) APTR requester,
		    REG(a1) struct TagItem *tagList,
                    REG(a6) struct ASLLib *AslBase)
{
struct ReqInfo *req;

    if (requester)
    {
        req = (struct ReqInfo *)((ULONG)requester - sizeof(struct ReqInfo));

        req->ri_A4 = getreg(REG_A4);  /* needed for callback hooks */

        switch (req->ri_ReqType)
        {
            case ASL_FileRequest      : return(FileRequest(AslBase,(struct ExtFileReq *)req,tagList));
            case ASL_FontRequest      : return(FontRequest(AslBase,(struct ExtFontReq *)req,tagList));
            case ASL_ScreenModeRequest: return(SMRequest(AslBase,(struct ExtSMReq *)req,tagList));
        }
    }
    else
    {
        SetIoErr(ERROR_NO_FREE_STORE);
    }

    return(NULL);
}


/*****************************************************************************/


/* WARNING: This routine is called from the library's expunge vector and so
 *          runs under Forbid().
 */
VOID ASM FlushLib(REG(a6) struct ASLLib *AslBase)
{
    FlushFontCaches(AslBase);
}
