/*
** $Id: docmd.c,v 1.2 93/02/16 16:14:49 kcd Exp $
**
** Module that contains the routine to send a command transaction off to
** the accounts server.
**
*/

/*--------------------------------------------------------------------------*/

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#include "accountsbase.h"
#include "/accounts.h"
#include "/transactions.h"

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

/*--------------------------------------------------------------------------*/

BOOL DoCommand(struct Transaction *trans)
{
    register struct AccountsLib *alib = AccountsBase;
    struct MsgPort *replyport;
    BOOL result = FALSE;

    if(replyport = CreateMsgPort())
    {
    	trans->trans_Msg.mn_ReplyPort = replyport;
    	trans->trans_ResponseData = trans->trans_RequestData;
    	trans->trans_RespDataLength = trans->trans_ReqDataLength;
    	trans->trans_Error = 0;
    	trans->trans_Timeout = 5;

    	BeginTransaction(alib->ACNTS_AccEntity,alib->ACNTS_Entity,trans);

    	WaitPort(replyport);
    	GetMsg(replyport);
	DeleteMsgPort(replyport);

    	result = TRUE;
    }
    return(result);
}
