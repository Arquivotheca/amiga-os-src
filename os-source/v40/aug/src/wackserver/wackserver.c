/*
 * Amiga Grand Wack
 *
 * wackserver.c -- Envoy "server" for the remote machine.
 *
 * $Id: wackserver.c,v 39.1 92/11/02 17:20:13 peter Exp $
 *
 * This code is a "server" to support Wack accessing the remote machine
 * via Envoy.  It should be made into a true Envoy service that can
 * be fired up by Envoy.  For now, just "run" it on the remote system.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <envoy/nipc.h>
#include <envoy/services.h>
#include <dos/dos.h>
#include <exec/types.h>

#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

#include <string.h>

#include "/envoyaccess.h"
#include "/validmem.h"
#include "/validmem_proto.h"

extern struct ExecBase *SysBase;

struct TagItem cetags[4] =
{
    ENT_Name, (LONG) "RemoteWack",
    ENT_Public, TRUE,
    ENT_AllocSignal, 0,
    TAG_END, 0
};

void main(void)
{
    struct Library *NIPCBase;

    void *entity;
    struct Transaction *trans;
    ULONG entity_sigbit,mymask,signals;
    struct MinList *validlist;
    struct MemoryBlock *marray;

    cetags[2].ti_Data = (ULONG) &entity_sigbit;
    entity_sigbit = 0; /* So SAS doesn't complain below. */

    /* Let's build a list of legal memory in this system.
     * Then, we convert it into an array, so that when Wack
     * calls, we can pass the array back to it.
     */
    if ( validlist = buildValidMemList() )
    {
	struct MemoryBlock *mb;
	long count = 0;
	for ( mb = (struct MemoryBlock *)validlist->mlh_Head;
	    mb->mb_Node.mln_Succ;
	    mb = (struct MemoryBlock *)mb->mb_Node.mln_Succ )
	{
	    count++;
	}

	if ( marray = AllocVec( count * sizeof(struct MemoryBlock), MEMF_CLEAR ) )
	{
	    count = 0;
	    for ( mb = (struct MemoryBlock *)validlist->mlh_Head;
		mb->mb_Node.mln_Succ;
		mb = (struct MemoryBlock *)mb->mb_Node.mln_Succ )
	    {
		marray[ count++ ] = *mb;
	    }
	    /* Now we can dispose of the linked-list version */
	    freeValidMemList( validlist );
	    validlist = NULL;

	    if (NIPCBase = OpenLibrary("nipc.library", 0L))
	    {
		if (entity = CreateEntityA(cetags))
		{
		    while(TRUE)
		    {
			mymask = (1L << entity_sigbit) | SIGBREAKF_CTRL_C| SIGBREAKF_CTRL_D;
			signals = Wait(mymask);

			if(signals & SIGBREAKF_CTRL_C)
			    break;

			if(trans = GetTransaction(entity))
			{
			    ULONG len;
			    struct RemoteWackCommand *rwc = (struct RemoteWackCommand *)trans->trans_RequestData;
			    ULONG *response = trans->trans_ResponseData;
			    trans->trans_RespDataActual = 0;

			    switch ( rwc->rwc_Action )
			    {
			    case RWCA_FORBID:
				break;

			    case RWCA_PERMIT:
				break;

			    case RWCA_READBYTE:
				*response = *(( UBYTE * )rwc->rwc_Address );
				trans->trans_RespDataActual = 4;
				break;

			    case RWCA_READWORD:
				*response = *(( UWORD * )rwc->rwc_Address );
				trans->trans_RespDataActual = 4;
				break;

			    case RWCA_READLONG:
				*response = *(( ULONG * )rwc->rwc_Address );
				trans->trans_RespDataActual = 4;
				break;

			    case RWCA_READBLOCK:
				CopyMem( rwc->rwc_Address,
				    trans->trans_ResponseData, rwc->rwc_Data );
				trans->trans_RespDataActual = rwc->rwc_Data;
				break;

			    case RWCA_READSTRING:
				len = strlen( rwc->rwc_Address ) + 1;
				if ( len > rwc->rwc_Data )
				{
				    len = rwc->rwc_Data;
				}
				CopyMem( rwc->rwc_Address,
				    trans->trans_ResponseData, len );
				((char *)trans->trans_ResponseData)[ rwc->rwc_Data-1 ] = 0;
				trans->trans_RespDataActual = len;
				break;

			    case RWCA_WRITEBYTE:
				*(( UBYTE * )rwc->rwc_Address ) = (UBYTE) rwc->rwc_Data;
				break;

			    case RWCA_WRITEWORD:
				*(( UWORD * )rwc->rwc_Address ) = (UWORD) rwc->rwc_Data;
				break;

			    case RWCA_WRITELONG:
				*(( ULONG * )rwc->rwc_Address ) = (ULONG) rwc->rwc_Data;
				break;

			    case RWCA_GETVALIDMEMCOUNT:
				*response = count;
				trans->trans_RespDataActual = 4;
				break;

			    case RWCA_GETVALIDMEM:
				CopyMem( marray, trans->trans_ResponseData,
				    count * sizeof( struct MemoryBlock ) );
				trans->trans_RespDataActual = count *
				    sizeof( struct MemoryBlock );
				break;
			    }
			    ReplyTransaction(trans);
			}
		    }
		    DeleteEntity(entity);
		}
		CloseLibrary(NIPCBase);
	    }
	    FreeVec( marray );
	}
	if ( validlist )
	{
	    freeValidMemList( validlist );
	}
    }
}
