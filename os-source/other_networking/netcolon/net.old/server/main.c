/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||   John Toebes     Doug Walker    Dave Baker                   */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
**      $Filename: main.c $ 
**      $Author: J_Toebes $
**      $Revision: 1.20 $
**      $Date: 91/03/11 11:18:39 $
**      $Log:	main.c,v $
 * Revision 1.20  91/03/11  11:18:39  J_Toebes
 * Add code to limit the number of active open file handles (really fix it)
 * 
 * Revision 1.19  91/03/11  11:16:55  J_Toebes
 * Add code to limit the number of active open file handles
 * 
 * Revision 1.18  91/01/15  01:49:41  J_Toebes
 * Added version information
 * 
 * Revision 1.17  91/01/11  00:00:29  J_Toebes
 * Added code to compress the FIB portion of the return on an ACT_EXAMINE/EXNEXT
 * 
 * Revision 1.16  90/12/30  15:38:40  J_Toebes
 * Add support for Network Init and Network Login
 * 
 * Revision 1.15  90/12/29  13:09:36  J_Toebes
 * Add memory Debugging code.
 * 
 * Revision 1.14  90/11/29  02:00:20  J_Toebes
 * General cleanup to eliminate debugging, reduce size.
 * 
 * Revision 1.13  90/11/28  01:53:21  J_Toebes
 * Added new actions, lock on config directory.
 * 
 * Revision 1.12  90/11/23  15:07:07  J_Toebes
 * Added session support back in.
 * 
 * Revision 1.11  90/11/18  22:54:50  J_Toebes
 * Added RmtNameUnlock routine for namenode state machine.
 * 
 * Revision 1.10  90/11/17  21:50:50  J_Toebes
 * Corrected handling of SDN_ERR for accept calls
 * 
 * Revision 1.9  90/11/15  08:17:10  J_Toebes
 * Correct multiple bugs associated with new version.  Sessions still are not supported.
 * 
 * Revision 1.8  90/11/05  06:56:24  J_Toebes
 * Major rewrite to make server multi-threaded with support for multiple
 * devices.  At this point in time it is not debugged and the concept of
 * sessions is missing.
 * 
**
**/

#define _main xxxumain
#define MemCleanup xxxmemcleanup
#include "server.h"
#undef _main
#undef MemCleanup

/******************************************************************************/
/******************************************************************************/
/********************* Dispatch table to handle all packets *******************/
/******************************************************************************/
/******************************************************************************/
#define BP1 1
#define BP2 2
#define BP3 4
#define BP4 8
#define BP5 16
typedef void (*ifuncp)(GLOBAL, SPACKET *);

#if DEBUG
char *dbgwind = "CON:0/0/640/160/NET-SERVER/a";
#endif /* DEBUG */

struct LookupTable {
    short     num;
    short     flags;
    ifuncp  subr;
};

struct LookupTable worktab[] = {
   /*    5 */ { ACTION_DIE,            0  | 0  | 0  | 0  , RmtDie           }, /* */
   /*    6****{ ACTION_EVENT,          0  | 0  | 0  | 0  , NULL             },*****/
   /*    7****{ ACTION_CURRENT_VOLUME, BP1| 0  | 0  | 0  , NULL             },*****/
   /*    8 */ { ACTION_LOCATE_OBJECT,  BP1| BP2| 0  | 0  , RmtLock          }, /* */
   /*    9****{ ACTION_RENAME_DISK,    BP1| BP2| 0  | 0  , NULL             },*****/
   /*   15 */ { ACTION_FREE_LOCK,      BP1| 0  | 0  | 0  , RmtUnLock        }, /* */
   /*   16 */ { ACTION_DELETE_OBJECT,  BP1| BP2| 0  | 0  , RmtDelete        }, /* */
   /*   17 */ { ACTION_RENAME_OBJECT,  BP1| BP2| BP3| BP4, RmtRename        }, /* */
   /*   18****{ ACTION_MORE_CACHE,     0  | 0  | 0  | 0  , NULL             },*****/
   /*   19 */ { ACTION_COPY_DIR,       BP1| 0  | 0  | 0  , RmtDupLock       }, /* */
   /*   20****{ ACTION_WAIT_CHAR,      0  | 0  | 0  | 0  , NULL             },*****/
   /*   21 */ { ACTION_SET_PROTECT,    0  | BP2| BP3| 0  , RmtSetProtection }, /* */
   /*   22 */ { ACTION_CREATE_DIR,     BP1| BP2| 0  | 0  , RmtCreateDir     }, /* */
   /*   23 */ { ACTION_EXAMINE_OBJECT, BP1| BP2| 0  | 0  , RmtExamine       }, /* */
   /*   24 */ { ACTION_EXAMINE_NEXT,   BP1| BP2| 0  | 0  , RmtExNext        }, /* */
   /*   25****{ ACTION_DISK_INFO,      BP1| 0  | 0  | 0  , NULL             },*****/
   /*   26 */ { ACTION_INFO,           BP1| BP2| 0  | 0  , RmtInfo          }, /* */
   /*   27****{ ACTION_FLUSH,          0  | 0  | 0  | 0  , NULL             },*****/
   /*   28 */ { ACTION_SET_COMMENT,    0  | BP2| BP3| BP4, RmtSetComment    }, /* */
   /*   29 */ { ACTION_PARENT,         BP1| 0  | 0  | 0  , RmtParent        }, /* */
   /*   30****{ ACTION_TIMER,          BP1| 0  | 0  | 0  , NULL             },*****/
   /*   31****{ ACTION_INHIBIT,        0  | 0  | 0  | 0  , NULL             },*****/
   /*   34 */ { ACTION_SET_FILE_DATE,  0  | 0  | 0  | 0  , RmtSetFileDate   }, /* */
   /*   40****{ ACTION_SAME_LOCK,      0  | 0  | 0  | 0  , NULL             },*****/
   /*   52 */ { ACTION_READ,           0  | 0  | 0  | 0  , RmtRW            }, /* */
   /*   53 */ { ACTION_READ_MORE,      0  | 0  | 0  | 0  , RmtRWMore        }, /* */
   /*   57 */ { ACTION_WRITE,          0  | 0  | 0  | 0  , RmtRW            }, /* */
   /*   58 */ { ACTION_WRITE_MORE,     0  | 0  | 0  | 0  , RmtRWMore        }, /* */
   /* 1001****{ ACTION_READ_RETURN,    0  | 0  | 0  | 0  , NULL             },*****/
   /* 1002****{ ACTION_WRITE_RETURN,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 1004 */ { ACTION_FIND_WRITE,     BP1| BP2| BP3| 0  , RmtOpen          }, /* */
   /* 1005 */ { ACTION_FIND_INPUT,     BP1| BP2| BP3| 0  , RmtOpen          }, /* */
   /* 1006 */ { ACTION_FIND_OUTPUT,    BP1| BP2| BP3| 0  , RmtOpen          }, /* */
   /* 1007 */ { ACTION_END,            0  | 0  | 0  | 0  , RmtEnd           }, /* */
   /* 1008 */ { ACTION_SEEK,           0  | 0  | 0  | 0  , RmtSeek          }, /* */
   /* 1009****{ ACTION_FINDREADONLY,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 1010****{ ACTION_FINDONEWRITER,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1020****{ ACTION_FORMAT,         0  | 0  | 0  | 0  , NULL             },*****/
   /* 1021****{ ACTION_MAKE_LINK,      0  | 0  | 0  | 0  , NULL             },*****/
   /* 1022****{ ACTION_SET_FILE_SIZE,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1023****{ ACTION_WRITE_PROTECT,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1024****{ ACTION_READ_LINK,      0  | 0  | 0  | 0  , NULL             },*****/
   /* 1026****{ ACTION_FH_FROM_LOCK,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 1027****{ ACTION_IS_FILESYSTEM,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1028****{ ACTION_CHANGE_MODE,    0  | 0  | 0  | 0  , NULL             },*****/
   /* 1030****{ ACTION_COPY_DIR_FH,    0  | 0  | 0  | 0  , NULL             },*****/
   /* 1031****{ ACTION_PARENT_FH,      0  | 0  | 0  | 0  , NULL             },*****/
   /* 1033****{ ACTION_EXAMINE_ALL,    0  | 0  | 0  | 0  , NULL             },*****/
   /* 1034****{ ACTION_EXAMINE_FH,     0  | 0  | 0  | 0  , NULL             },*****/
#if DEBUG
   /* 2010 */ { ACTION_HANDLER_DEBUG,  0  | 0  | 0  | 0  , RmtSetDebug      }, /* */
#endif
   /* 2011****{ ACTION_SET_TRANS_TYPE, BP1| 0  | 0  | 0  , NULL             },*****/
   /* 2012 */ { ACTION_NETWORK_HELLO,  0  | 0  | 0  | 0  , RmtNetLogin      }, /* */
   /* 4097****{ ACTION_ADD_NOTIFY,     0  | 0  | 0  | 0  , NULL             },*****/
   /* 4098****{ ACTION_REMOVE_NOTIFY,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 5554 */ { ACTION_NETWORK_START,  0  | 0  | 0  | 0  , RmtNetStart      }, /* */
   /* 5555 */ { ACTION_NETWORK_INIT,   0  | 0  | 0  | 0  , RmtNetInit       }, /* */
   /* 5556 */ { ACTION_NETWORK_TERM,   0  | 0  | 0  | 0  , RmtNetStop       }, /* */
   /* 5557****{ ACTION_NETWORK_KLUDGE, 0  | 0  | 0  | 0  , RmtNetLogin      },*****/
   /* 5558 */ { ACTION_RETURN,         0  | 0  | 0  | 0  , RmtReturn        }, /* */
   /* 5559 */ { ACTION_RETRW,          0  | 0  | 0  | 0  , RmtRWRet         }, /* */
   /* 5560 */ { ACTION_RWMORE1,        0  | 0  | 0  | 0  , RmtRWMore1       }, /* */
   /* 5561 */ { ACTION_RWMORE2,        0  | 0  | 0  | 0  , RmtRWMore2       }, /* */
   /* 5562 */ { ACTION_RWMORE3,        0  | 0  | 0  | 0  , RmtRWMore3       }, /* */
   /* 5563 */ { ACTION_RETOPEN,        0  | 0  | 0  | 0  , RmtOpenRet       }, /* */
   /* 5564 */ { ACTION_RETEND,         0  | 0  | 0  | 0  , RmtEndRet        }, /* */
   /* 5565 */ { ACTION_RETLOCKNAME,    0  | 0  | 0  | 0  , RmtNameLockRet   }, /* */
   /* 5566 */ { ACTION_RETPARENT,      0  | 0  | 0  | 0  , RmtParentRet     }, /* */
   /* 5567 */ { ACTION_RETNAMEEX,      0  | 0  | 0  | 0  , RmtNameExamine   }, /* */
   /* 5568 */ { ACTION_RETNAMELOCK,    0  | 0  | 0  | 0  , RmtNameLock      }, /* */
   /* 5569 */ { ACTION_RETNAMEUNLOCK,  0  | 0  | 0  | 0  , RmtNameUnlock    }, /* */
   /* 5570 */ { ACTION_FINDINFO,       0  | 0  | 0  | 0  , RmtFindInfo      }, /* */
   /* 5571 */ { ACTION_RETINFO,        0  | 0  | 0  | 0  , RmtDoneInfo      }, /* */
   /* 5572 */ { ACTION_RETLOCK,        0  | 0  | 0  | 0  , RmtRetLock       }, /* */
   /* 5573 */ { ACTION_RETFIB,         0  | 0  | 0  | 0  , RmtRetFIB        }
  };
#define WORKSIZE (sizeof(worktab)/sizeof(struct LookupTable))

extern struct Library *SysBase;
struct DosLibrary *DOSBase;

void __stdargs _main(char *p);

void __stdargs _main(p)
char *p;
{
   ifuncp             subr;
   int                action;
   struct global      main_global;
   int                low, high, this;
   struct RPacket    *rp;
   long               sigmask;
   SPACKET           *spkt;
   DRIVER            *driver;
   struct Library    *DriverBase;
#define global (&main_global)  /* This allows us to use global in this main code */

   MWInit(NULL, MWF_NOCHECK,"CON:0/0/639/199/Memory debug");

   SysBase = (*((struct Library **) 4));
   DOSBase = (struct DosLibrary *)OpenLibrary(DOSNAME,0);

   /*===========================================================================*/
   /*                  Initialize our global data structure                     */
   /*===========================================================================*/
   memset((char *)&main_global, 0, sizeof(main_global));

   global->n.self   = (struct Process *) FindTask(0L);  /* find myself          */
   global->n.self->pr_WindowPtr = (APTR)-1;
   global->n.run    = 2;
   global->n.port   = CreatePort(NULL, 0);
   global->n.NetBufSize = 8192;  /* We need to get this from the connection */
   global->replymask = 1l << global->n.port->mp_SigBit;
   global->waitbits |= global->replymask;
   global->opencount = MAXOPENS;
#if DEBUG
   initreq(global);
#endif

   global->configlock = Lock("devs:networks" VERSTRING, SHARED_LOCK);
   /*===========================================================================*/
   /*                  Parse the user configuration file                        */
   /*===========================================================================*/
   ParseConfig(global);

   /*===========================================================================*/
   /*   Here we loop basically forever waiting for events to happen             */
   /*===========================================================================*/
   driver = NULL;
   while(global->n.run)
   {
      MWCheck();
      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
      /* One of two things will happen now:                                     */
      /*    1) A message will come in from some handler.                        */
      /*       a) Connection has been established                               */
      /*       b) Connection has been terminated                                */
      /*       c) Request for some file system service                          */
      /*    2) A response to a previous packet has been returned                */
      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
      do
      {
         if (global->openpend && global->opencount)
         {
            spkt = global->openpend;
            global->openpend = (SPACKET *)spkt->sp_Pkt.dp_Arg5;
            goto havemsg;
         }

         if (driver == NULL)
         {
            sigmask = Wait(global->waitbits);
            driver = global->drivers;
         }

         spkt = NULL;
         if (sigmask & global->replymask)
         {
            /* It is one of the original packets returning to us.               */
            spkt = (SPACKET *)GetMsg(global->n.port);
            if (spkt != NULL)
            {
               rp = spkt->sp_RP;
               break;
            }
            /* Note that we could turn off the signal bit here once we call     */
            /* getmsg and fail to find a message for us.  If however we have a  */
            /* driver that is sharing that same signal bit then this probably   */
            /* would cause us a lot of trouble.  For now we will take the extra */
            /* overhead of the message check but could consider doing the reset */
            /* later if we determine that this is an acceptable restriction     */
         }
         
         while(driver != NULL)
         {
            if (driver->sigmask & sigmask)
            {
               /* Looks like an eligible driver, ask him if he is.      */
               DriverBase = driver->libbase;
               switch(SDNAccept(driver->drglobal, &rp))
               {
                  case SDN_OK:
                     /* We got an incoming request.  Get us some memory to deal */
                     /* with it.                                                */
                     spkt = GetSPacket(global);
                     if (spkt == NULL)
                     {
                        /* OOps, not enough memory to even process the request  */
                        /* just respond to them with a 'sorry charlie'          */
                        /* We will have to worry about connect requests later   */
                        /* but if we fail on a connect request we really want to*/
                        /* force the request to be delayed and try it again when*/
                        /* we get some memory back                              */
                        rp->Arg1 = DOSFALSE;
                        rp->Arg2 = ERROR_NO_FREE_STORE;
                        SDNReply(driver->drglobal, rp);
                        break;
                     }

                     spkt->sp_RP = rp;
                     spkt->sp_Action = rp->Type;
                     spkt->sp_Driver = driver;
                     
                     /* For convenience, copy over all the arguments            */
                     spkt->sp_Pkt.dp_Type = rp->Type;
                     spkt->sp_Pkt.dp_Arg1 = rp->Arg1;
                     spkt->sp_Pkt.dp_Arg2 = rp->Arg2;
                     spkt->sp_Pkt.dp_Arg3 = rp->Arg3;
                     spkt->sp_Pkt.dp_Arg4 = rp->Arg4;
                     spkt->sp_Ses = SDNGetConData(driver->drglobal, rp);
                     goto havemsg;

                  case SDN_ERR:
                  case SDN_NONE:
                     driver = driver->next;
                     break;
               }
            }
            else
            {
               driver = driver->next;
            }
         }
      } while(spkt == NULL);


havemsg:
      /* We now have a message.  Actually we don't really care where it came    */
      /* from because it simply passes through our state machine to be          */
      /* processed.  Eventually it will hit a state where it gets returned      */
      /* to the sender.                                                         */

      BUG(("Execute: action #%ld arg1 %lx\n", rp->Type,rp->Arg1));

      action = spkt->sp_Action;
      low = 0;
      high = WORKSIZE;
      subr = NULL;
      while (low <= high)
      {
         this = (high-low)/2 + low;
         if (worktab[this].num == action)
         {
            subr = worktab[this].subr;
            break;
         }
         if (worktab[this].num < action)
            low = this + 1;
         else
            high = this - 1;
      }

      global->n.reply = 1;
      if (subr != NULL) 
      {
         (*subr)(global, spkt);
      } 
      else 
      {
         BUG(("Unknown packet type %ld\n",rp->Type));
         rp->Arg1 = DOS_FALSE;
         rp->Arg2 = ERROR_ACTION_NOT_KNOWN;
      }

      if (global->n.reply)
      {
         DriverBase = spkt->sp_Driver->libbase;
         
         /* For fake packets we will want to reclaim the storage without        */
         /* calling to reply to the packet.  We could protect against this in   */
         /* the driver, and might consider doing so if we believe that there    */
         /* is a driver that is actually sending a message for the network      */
         /* initialization and termination.                                     */
         if (global->n.reply == -1)
         {
            SDNFreeRPacket(spkt->sp_Driver->drglobal, rp);
         }
         else
         {
            SDNReply(spkt->sp_Driver->drglobal, rp);
         }
         FreeSPacket(global, spkt);
      }

      BUG(("-----\n"));
   }

fail: ;
    MWTerm();

   BUGTERM()
}
#undef global

#if DEBUG
ACTFN(RmtSetDebug)
{
    BUG(("RmtSetDebug: Entry, arg1 %lx\n", spkt->sp_RP->Arg1))
    if(spkt->sp_RP->Arg1)
        BUGINIT()
    else
        BUGTERM()

    spkt->sp_RP->Arg1 = DOS_TRUE;
}
#endif

void __stdargs MemCleanup(void);
void __stdargs MemCleanup() {}
