/***********************************************************************
 *                                     *
 *                COPYRIGHTS                   *
 *                                     *
 *   Copyright (c) 1991  Commodore-Amiga, Inc.  All Rights Reserved.   *
 *                                     *
 ***********************************************************************
 *
 * $Id: RemoteControl.c,v 1.1 91/08/18 14:12:18 ewout Exp Locker: ewout $
 *
 * RemoteControl.c - INet Remote Control connectionless client for InfraNet.
 * Saturday, 17-Aug-91 15:03:43, Ewout
 *
 * Compiled with SAS/C 5.10a:
 * lc -cfis -v -d0 -b0 RemoteControl.c
 * blink from lib:c.o RemoteControl.o to RemoteControl lib lib:amiga.lib lib:lc.lib \
 * DEFINE __main=__tinymain
 *
 */
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <dos/dosasl.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <appn/nipc.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/commodities_protos.h>
#include <clib/alib_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/commodities_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>

/* extern struct DosLibrary *DOSBase; */
struct Library *NIPCBase;
struct Library *IconBase;
struct Library *CxBase;

static UBYTE *VersTag = "\0$VER: RemoteControl 37.2 (5.9.91) Copyright © 1991 Ewout Walraven";
#define CXNAME  "RemoteControl"
#define CXDESCR "InfraNet RemoteControl Client"

#define LOCALHOST  -1

LONG main(LONG argc, UBYTE ** argv);

LONG main(LONG argc, UBYTE ** argv)
{
    struct Entity *entity;
    struct Entity *hp[4];
    struct Entity *server;
    struct Transaction *mouse_trans,*m_trans;
    struct Transaction *key_trans;
    UBYTE **tooltypes;
    UBYTE *host[4];
    UBYTE *hostkey[4], *localkey;
    struct NewBroker *newbroker;
    CxObj *ibroker, *filter, *keyfilter, *mousefilter, *keysender, *mousesender, *keytranslator,
    *mousetranslator;
    CxMsg *cxmsg;
    ULONG isignal, cxsignal, cxmsg_id, cxmsg_type;
    LONG tracker = LOCALHOST;
    LONG rval, rc = RETURN_FAIL;
    BOOL mouse_free = TRUE;
    BOOL valid_mouse = FALSE;
    struct InputEvent mouse_event,*ievent;
    BOOL ABORT = FALSE;
    ULONG mysig,trsignal;
    IX MouseIX =
    {IX_VERSION, IECLASS_RAWMOUSE, 0, 0, 0, 0, 0};
    IX KeyIX =
    {IX_VERSION, IECLASS_RAWKEY, 0, 0, 0, 0, 0};

    /*
     * If this is not > 36, the error console will be opened immediately. no
     * sweat, since we'll output a kick2.0 required string almost
     * immediately.
     */
	if (CxBase = OpenLibrary("commodities.library",	36))
	{
		if (IconBase = OpenLibrary("icon.library", 36))
		{
			if (NIPCBase = OpenLibrary("nipc.library", 0L))
			{
				mysig=0;
				if (entity = CreateEntity(ENT_AllocSignal,(ULONG) &mysig, TAG_DONE,	0))
				{
					if (mouse_trans	= AllocTransaction(TAG_DONE))
					{
						if(key_trans = AllocTransaction(TAG_DONE))
						{
							/*
							 * Time	to get the server names. Supported
							 * keywords: HOST1,	HOST2, HOST3, HOST4	HOST1KEY,
							 * HOST2KEY, HOST3KEY, HOST4KEY, LOCALKEY
							 * <hotkey>	CX_PRIORITY	<priority>
							 */
							tooltypes =	ArgArrayInit(argc, argv);
							host[0]	= ArgString(tooltypes, "HOST1",	NULL);
							host[1]	= ArgString(tooltypes, "HOST2",	NULL);
							host[2]	= ArgString(tooltypes, "HOST3",	NULL);
							host[3]	= ArgString(tooltypes, "HOST4",	NULL);
							hostkey[0] = ArgString(tooltypes, "HOST1KEY", "numericpad (");
							hostkey[1] = ArgString(tooltypes, "HOST2KEY", "numericpad )");
							hostkey[2] = ArgString(tooltypes, "HOST3KEY", "numericpad /");
							hostkey[3] = ArgString(tooltypes, "HOST4KEY", "numericpad *");
							localkey = ArgString(tooltypes,	"LOCALKEY",	"lalt esc");

							/* We do have a	server,	don't we? */
							if (host[0]	== NULL	&& host[1] == NULL && host[2] == NULL && host[3] ==	NULL)
								ABORT =	TRUE;
							else
							{
								/* make	sure servers are valid */
								for	(rval =	0; rval	< 4; rval++)
								{
									hp[rval]=NULL;
									if (host[rval])
									{
										hp[rval] = FindEntity(host[rval],"RemoteControl", entity,NULL);
										if (hp[rval] ==	NULL)
										{
											ABORT =	TRUE;
										}
									}
									if (ABORT == TRUE)
										break;
								}
								if (ABORT == FALSE)
								{
									/* initialize broker */
									if (newbroker =	AllocMem(sizeof(struct NewBroker), MEMF_CLEAR))
									{
										if (newbroker->nb_Port = CreateMsgPort())
										{
											newbroker->nb_Pri =	(BYTE) ArgInt(tooltypes, "CX_PRIORITY",	0);
											newbroker->nb_Version =	NB_VERSION;
											newbroker->nb_Name = CXNAME;
											newbroker->nb_Title	= CXNAME;
											newbroker->nb_Descr	= CXDESCR;
											newbroker->nb_Unique = NBU_UNIQUE |	NBU_NOTIFY;

											/* Create the broker */
											if (ibroker	= CxBroker(newbroker, NULL))
											{
												filter = HotKey(localkey, newbroker->nb_Port, 4);
												if (!(CxObjError(filter)))
													AttachCxObj(ibroker, filter);
												else
													ABORT =	TRUE;

												/*
												 * Set up the hotkey filters
												 * to switch servers
												 */
												for	(rval =	0; rval	< 4; rval++)
												{
													if (host[rval])
													{
														filter = HotKey(hostkey[rval], newbroker->nb_Port, rval);
														if (!(CxObjError(filter)))
														{
															AttachCxObj(ibroker, filter);
														}
														else
															ABORT =	TRUE;
													}

													if (ABORT == TRUE)
														break;
												}
												ABORT =	TRUE;
												/*
												 * Set up mouse	and	key
												 * filters,	but	don't attach
												 * them
												 */
												if (keysender =	CxSender(newbroker->nb_Port, 127))
												{
													if (mousesender	= CxSender(newbroker->nb_Port, 128))
													{
														if (keyfilter =	CxFilter(NULL))
														{
															if (mousefilter	= CxFilter(NULL))
															{
																if (keytranslator =	CxTranslate(NULL))
																{
																	if (mousetranslator	= CxTranslate(NULL))
																	{

																		SetFilterIX(keyfilter, &KeyIX);
																		SetFilterIX(mousefilter, &MouseIX);

																		AttachCxObj(keyfilter, keysender);
																		AttachCxObj(mousefilter, mousesender);

																		AttachCxObj(keyfilter, keytranslator);
																		AttachCxObj(mousefilter, mousetranslator);

																		if (!(CxObjError(keyfilter)))
																			if (!(CxObjError(mousefilter)))
																				ABORT =	FALSE;
																	}
																}
															}
														}
													}
												}

												if (ABORT == FALSE)
												{
													cxsignal = 1L << (newbroker->nb_Port->mp_SigBit);
													trsignal = 1L << (mysig);
													ActivateCxObj(ibroker, 1L);
													rc = RETURN_OK;

													do
													{
														isignal	= Wait(cxsignal	| trsignal | SIGBREAKF_CTRL_C);
														if (isignal	& SIGBREAKF_CTRL_C)
															ABORT =	TRUE;

														while (cxmsg = (CxMsg *) GetMsg(newbroker->nb_Port))
														{
															cxmsg_id = CxMsgID((CxMsg *) cxmsg);
															cxmsg_type = CxMsgType((CxMsg *) cxmsg);

															switch (cxmsg_type)
															{
																case CXM_IEVENT:
																	switch (cxmsg_id)
																	{
																		case 0:
																		case 1:
																		case 2:
																		case 3:
																			if (tracker	!= cxmsg_id)
																			{
																				ActivateCxObj(ibroker, 0);
																				if (tracker	== LOCALHOST)
																				{
																					AttachCxObj(ibroker, keyfilter);
																					AttachCxObj(ibroker, mousefilter);
																				}
																				server = hp[cxmsg_id];
																				tracker	= cxmsg_id;
																				ActivateCxObj(ibroker, 1L);
																			}
																			break;
																		case 4:
																			if (tracker	!= LOCALHOST)
																			{
																				ActivateCxObj(ibroker, 0);
																				RemoveCxObj(keyfilter);
																				RemoveCxObj(mousefilter);
																				tracker	= LOCALHOST;
																				ActivateCxObj(ibroker, 1L);
																			}
																			break;
																		case 127:
																			key_trans->trans_RequestData = CxMsgData(cxmsg);
																			key_trans->trans_ReqDataActual = sizeof(struct InputEvent);
																			DoTransaction(server, entity, key_trans);
																			break;
																		case 128:
																			if(mouse_free)
																			{
																				mouse_trans->trans_RequestData = CxMsgData(cxmsg);
																				mouse_trans->trans_ReqDataActual = sizeof(struct InputEvent);
																				BeginTransaction(server, entity, mouse_trans);
																				mouse_free = valid_mouse = FALSE;
																			}
																			else
																			{
																				ievent = (struct InputEvent *) CxMsgData(cxmsg);
																				if ((ievent->ie_Code      == mouse_event.ie_Code) &&
																				    (ievent->ie_Qualifier == mouse_event.ie_Qualifier))
																			    {
																					CopyMem(CxMsgData(cxmsg),&mouse_event,sizeof(struct InputEvent));
																					valid_mouse = TRUE;
																				}
																				else
																				{
																					WaitTransaction(mouse_trans);
																					CopyMem(CxMsgData(cxmsg),&mouse_event,sizeof(struct InputEvent));
																					mouse_trans->trans_RequestData = &mouse_event;
																					mouse_trans->trans_ReqDataActual = sizeof(struct InputEvent);
																					BeginTransaction(server, entity, mouse_trans);
																					mouse_free = FALSE;
																				}
																			}
																			break;
																		default:
																			break;
																	}
																case CXM_COMMAND:
																	switch (cxmsg_id)
																	{
																		case CXCMD_DISABLE:
																			ActivateCxObj(ibroker, 0);
																			break;
																		case CXCMD_ENABLE:
																			ActivateCxObj(ibroker, 1);
																			break;
																		case CXCMD_KILL:
																		case CXCMD_UNIQUE:
																			ABORT =	TRUE;
																			break;
																	}
																	break;
															}
															ReplyMsg((struct Message *)	cxmsg);
														}
														if(m_trans = GetTransaction(entity))
														{
															if(valid_mouse && (!ABORT))
															{
																m_trans->trans_RequestData = &mouse_event;
																m_trans->trans_ReqDataActual = sizeof(struct InputEvent);
																valid_mouse = mouse_free = FALSE;
																BeginTransaction(server,entity,m_trans);
															}
															else
																mouse_free = TRUE;
														}
													} while	(ABORT == FALSE);
													ActivateCxObj(ibroker, 0L);
												}
												DeleteCxObjAll(ibroker);
												if(tracker == LOCALHOST)
												{
													DeleteCxObjAll(keyfilter);
													DeleteCxObjAll(mousefilter);
												}
											}

											DeleteMsgPort(newbroker->nb_Port);
										}
										FreeMem(newbroker, sizeof(struct NewBroker));
									}
									for	(rval =	0; rval	< 4; rval++)
										if (hp[rval])
										{
											kprintf("Losing entity %ld\n",(ULONG)rval);
											LoseEntity(hp[rval]);
										}
								}
							}
							FreeTransaction(key_trans);
						}
						FreeTransaction(mouse_trans);
						ArgArrayDone();
					}
					DeleteEntity(entity);
				}
				CloseLibrary(NIPCBase);
			}
			CloseLibrary(IconBase);
		}
		CloseLibrary(CxBase);
    }
    return (rc);
}
