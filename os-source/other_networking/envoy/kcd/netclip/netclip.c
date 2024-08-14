/*
** $Id$
**
** NetClip - Synchronize the clipboards of two machines via NIPC.
**
** (C) 1992 Commodore-Amiga, Inc.
**
** Initial version by Ken Dyke
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <devices/clipboard.h>
#include <appn/nipc.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>

#define NC_START	0
#define NC_WRITE    1
#define NC_UPDATE   2

ULONG __saveds __asm HookFunc(register __a0 struct Hook *hook,
                              register __a2 APTR         object,
                              register __a1 APTR         message);

struct Library *NIPCBase;

int __saveds netclip(void)
{
    struct Library *DOSBase;
    struct RDArgs *ncargs;
    struct Entity *local_entity;
    struct Entity *dest_entity;
    struct Transaction *trans,*read_trans;
    struct IOClipReq *read_clipIO,*write_clipIO;
    struct MsgPort *clipPort;
    struct Hook ClipHook;
    ULONG nipc_signal,nipc_mask,fe_error;
    ULONG signals,mask;
    ULONG args[1];

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
        if(NIPCBase = OpenLibrary("nipc.library",0L))
        {
            args[0]=0;

            if(ncargs = ReadArgs("Hostname/A",args,NULL))
            {
                if(args[0])
                {
                    if(trans = AllocTransaction(TRN_AllocReqBuffer,256,TAG_DONE,0))
                    {
						if(clipPort=CreateMsgPort())
						{
							if(read_clipIO = (struct IOClipReq *) CreateIORequest(clipPort,sizeof(struct IOClipReq)))
							{
								if(write_clipIO	= (struct IOClipReq	*) CreateIORequest(clipPort,sizeof(struct IOClipReq)))
								{
									if(!OpenDevice("clipboard.device",0L,(struct IORequest *) read_clipIO,0))
									{
										/* Copy over required stuff for the IORequest. */
										write_clipIO->io_Device	= read_clipIO->io_Device;
										write_clipIO->io_Unit =	read_clipIO->io_Unit;

										/* Set up our custom Hook. */
										ClipHook.h_Entry =	HookFunc;
										ClipHook.h_SubEntry	= NULL;
										ClipHook.h_Data	= FindTask(NULL);

						                /* Install Hook */
										write_clipIO->io_Data	= (UBYTE *)	&ClipHook;
										write_clipIO->io_Length	=	1;
										write_clipIO->io_Command = CBD_CHANGEHOOK;

										write_clipIO->io_ClipID = 0;
										read_clipIO->io_ClipID = 0;
										if(!DoIO((struct IORequest *)write_clipIO))
										{
											if(local_entity	= CreateEntity(ENT_Name,"NetClip",
																		   ENT_Public, TRUE,
																		   ENT_AllocSignal,&nipc_signal,
																		   TAG_DONE))
											{
												nipc_mask =	(1L	<< nipc_signal);
												mask = (1L << clipPort->mp_SigBit) | nipc_mask | SIGBREAKF_CTRL_C |	SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_E;

												while(TRUE)
												{
													signals	= Wait(mask);

													if(signals & SIGBREAKF_CTRL_C)
														break;

													if(signals & SIGBREAKF_CTRL_F)
													{
														/* Since there isn't a separate client & server, we have to do this
														 * here, instead of up above.
														 */
														if(dest_entity = FindEntity((UBYTE *)args[0],"NetClip",local_entity,&fe_error))
														{
															/* Tell the other side we will be sending them some data. */
															trans->trans_ReqDataActual = 1;
															trans->trans_Command = NC_START;
															DoTransaction(dest_entity,local_entity,trans);

															/* Init the read IORequest. */
															read_clipIO->io_Command	= CMD_READ;
															read_clipIO->io_Data	= trans->trans_RequestData;
															read_clipIO->io_Length =	256;
															read_clipIO->io_Offset =	0;
															read_clipIO->io_ClipID = 0;

															/* Send the entire clip. */
                                                           	while(TRUE)
															{
																if(DoIO(read_clipIO))
																	break;

																if(read_clipIO->io_Actual)
																{
																	trans->trans_ReqDataActual = read_clipIO->io_Actual;
																	trans->trans_Command = NC_WRITE;

																	DoTransaction(dest_entity,local_entity,trans);
																}
																else
																	break;
															}
															trans->trans_Command = NC_UPDATE;
															trans->trans_ReqDataActual = 0;
															DoTransaction(dest_entity,local_entity,trans);

															LoseEntity(dest_entity);
														}
													}

													/* Service our local entity. */
													while(read_trans=GetTransaction(local_entity))
													{
														if(read_trans->trans_Command == NC_START)
														{
															write_clipIO->io_Offset = 0L;
															write_clipIO->io_ClipID = 0;
														}
														if(read_trans->trans_Command == NC_WRITE)
														{
															write_clipIO->io_Command = CMD_WRITE;
															write_clipIO->io_Data	= read_trans->trans_RequestData;
															write_clipIO->io_Length	= read_trans->trans_ReqDataActual;

															DoIO((struct IORequest *)write_clipIO);
														}
														/* Note.  We have to disable our hook, otherwise we would
														 * end up bouncing the clip right back to the source machine,
														 * who would in turn bounce it right back.
														 */
														if(read_trans->trans_Command == NC_UPDATE)
														{
															ClipHook.h_Data=NULL;

															write_clipIO->io_Command = CMD_UPDATE;
															DoIO((struct IORequest *)write_clipIO);

															/* Re-enable our hook. */
															ClipHook.h_Data	= FindTask(NULL);

														}
														ReplyTransaction(read_trans);
													}
												}
												DeleteEntity(local_entity);
											}
											/* Remove our hook for good this time. */
											write_clipIO->io_Data	= (UBYTE *)	&ClipHook;
											write_clipIO->io_Length	=	0;
											write_clipIO->io_Command = CBD_CHANGEHOOK;
											DoIO((struct IORequest *)write_clipIO);
										}
										CloseDevice((struct	IORequest *)read_clipIO);
									}
									DeleteIORequest((struct	IORequest *)write_clipIO);
								}
								DeleteIORequest((struct	IORequest *)read_clipIO);
							}
							DeleteMsgPort((struct MsgPort *)clipPort);
                        }
                        FreeTransaction(trans);
                    }
                }
                FreeArgs(ncargs);
            }
            else
                PutStr("No Host name given.\n");

            CloseLibrary(NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
    return(0L);
}

/* Our custom hook.  Doesn't have to do very much, except pay attention to
 * whether or not h_Data is meaningful or not.
 */
ULONG __saveds __asm HookFunc(register __a0 struct Hook *hook,
                              register __a2 APTR         object,
                              register __a1 APTR         message)
{
    if(hook->h_Data)
        Signal((struct Task *)hook->h_Data,SIGBREAKF_CTRL_F);
    return(0L);
}