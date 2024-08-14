
#include "commoditiesbase.h"
#include "cxpools.h"
#include "pool.h"


/*****************************************************************************/


struct Message *GetEMsg(LONG size)
{
struct Message *msg;

    if (msg = AllocPoolMem(size))
    {
        msg->mn_ReplyPort = &CxBase->cx_EMsgReply;
        msg->mn_Length    = size;
    }

    return (msg);
}
