
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <utility/tagitem.h>
#include <graphics/text.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "aslutils.h"


/*****************************************************************************/


VOID AslCloseWindow(struct ASLLib *AslBase, struct Window *wp, BOOL others)
{
struct IntuiMessage *msg;
struct Node         *succ;

    ClearMenuStrip(wp);

    if (wp->UserPort)
    {
        Forbid();

        msg = (struct IntuiMessage *) wp->UserPort->mp_MsgList.lh_Head;
        while (succ = msg->ExecMessage.mn_Node.ln_Succ)
        {
            if (msg->IDCMPWindow == wp)
            {
                Remove(msg);
                ReplyMsg(msg);
            }
            msg = (struct IntuiMessage *) succ;
        }

        if (others)
            wp->UserPort = NULL;

        ModifyIDCMP(wp,NULL);
        Permit();
    }

    CloseWindow(wp);
}


/*****************************************************************************/


ULONG AslPackBoolTags(struct ASLLib *AslBase, ULONG flags, struct TagItem *tags, ULONG mappers, ...)
{
    return(PackBoolTags(flags,tags,(struct TagItem *)&mappers));
}


/*****************************************************************************/


VOID AslDrawBevelBox(struct ASLLib *AslBase, struct Window *wp, WORD x, WORD y, WORD w, WORD h, ULONG tags, ...)
{
    DrawBevelBoxA(wp->RPort,x+wp->BorderLeft,y+wp->BorderTop,
                            w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID StripExtension(struct ASLLib *AslBase, STRPTR name, STRPTR extension)
{
UWORD nlen,xlen;

    nlen = strlen(name);
    xlen = strlen(extension);
    if (nlen >= xlen)
        if (Stricmp(&name[nlen-xlen],extension) == 0)
            name[nlen-xlen] = 0;
}


/*****************************************************************************/


VOID BtoC(struct ASLLib *AslBase, APTR bstr, STRPTR string)
{
STRPTR ptr;
UBYTE  i;
UBYTE  len;

    ptr = bstr;
    len = ptr[0];
    i   = 0;
    while (i < len)
    {
        string[i] = ptr[++i];
    }
    string[i] = 0;
}
