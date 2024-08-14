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
**      $Revision: 1.26 $
**      $Date: 91/03/09 21:02:17 $
**      $Log:	main.c,v $
 * Revision 1.26  91/03/09  21:02:17  J_Toebes
 * Correct parameters for setfiledate
 * 
 * Revision 1.25  91/01/22  00:01:32  Doug
 * *** empty log message ***
 * 
 * Revision 1.24  91/01/15  02:56:05  Doug
 * Kill outstanding HPKTs when connection terminates
 * 
 * Revision 1.23  91/01/11  10:38:33  Doug
 * Add Network password capabilities
 * 
 * Revision 1.22  91/01/10  23:15:45  Doug
 * *** empty log message ***
 * 
 * Revision 1.21  91/01/06  20:55:55  Doug
 * Revise debugging messages
 * 
 * Revision 1.20  90/12/31  15:33:28  Doug
 * change packet array
 * 
 * Revision 1.19  90/12/30  15:45:48  Doug
 * Add ActNetInit, renumber several actions
 * Implement MEMCHK debugging
 * 
**
**/

#include "handler.h"

#if DEBUG
char *dbgwind = "CON:0/0/639/199/NET handler debug window/a";
int memcheck = 0;   /* check memory every packet */
#endif

/******************************************************************************/
/******************************************************************************/
/********************* Dispatch table to handle all packets *******************/
/******************************************************************************/
/******************************************************************************/
#define BP1 1
#define BP2 2
#define BP3 4
#define BP4 8

#if PARANOID
#undef BUGP
#define BUGP(sss) request(&global, REQ_GENERAL, sss);
#endif

struct LookupTable {
    short     num;
    short     flags;
    ifuncp  subr;
};

struct LookupTable worktab[] = {
   /*    5 */ { ACTION_DIE,            0  | 0  | 0  | 0  , ActDie           }, /* */
   /*    6****{ ACTION_EVENT,          0  | 0  | 0  | 0  , NULL             },*****/
   /*    7 */ { ACTION_CURRENT_VOLUME, BP1| 0  | 0  | 0  , ActCurentVol     }, /* */
   /*    8 */ { ACTION_LOCATE_OBJECT,  BP1| BP2| 0  | 0  , ActLock          }, /* */
   /*    9 */ { ACTION_RENAME_DISK,    BP1| BP2| 0  | 0  , ActRenameDisk    }, /* */
   /*   15 */ { ACTION_FREE_LOCK,      BP1| 0  | 0  | 0  , ActUnLock        }, /* */
   /*   16 */ { ACTION_DELETE_OBJECT,  BP1| BP2| 0  | 0  , ActDelete        }, /* */
   /*   17 */ { ACTION_RENAME_OBJECT,  BP1| BP2| BP3| BP4, ActRename        }, /* */
   /*   18****{ ACTION_MORE_CACHE,     0  | 0  | 0  | 0  , NULL             },*****/
   /*   19 */ { ACTION_COPY_DIR,       BP1| 0  | 0  | 0  , ActDupLock       }, /* */
   /*   20****{ ACTION_WAIT_CHAR,      0  | 0  | 0  | 0  , NULL             },*****/
   /*   21 */ { ACTION_SET_PROTECT,    0  | BP2| BP3| 0  , ActSetProtection }, /* */
   /*   22 */ { ACTION_CREATE_DIR,     BP1| BP2| 0  | 0  , ActCreateDir     }, /* */
   /*   23 */ { ACTION_EXAMINE_OBJECT, BP1| BP2| 0  | 0  , ActExamine       }, /* */
   /*   24 */ { ACTION_EXAMINE_NEXT,   BP1| BP2| 0  | 0  , ActExNext        }, /* */
   /*   25 */ { ACTION_DISK_INFO,      BP1| 0  | 0  | 0  , ActDiskInfo      }, /* */
   /*   26 */ { ACTION_INFO,           BP1| BP2| 0  | 0  , ActInfo          }, /* */
   /*   27 */ { ACTION_FLUSH,          0  | 0  | 0  | 0  , ActFlush         }, /* */
   /*   28 */ { ACTION_SET_COMMENT,    0  | BP2| BP3| BP4, ActSetComment    }, /* */
   /*   29 */ { ACTION_PARENT,         BP1| 0  | 0  | 0  , ActParent        }, /* */
   /*   30 */ { ACTION_TIMER,          0  | 0  | 0  | 0  , ActTimer         }, /* */
   /*   31 */ { ACTION_INHIBIT,        0  | 0  | 0  | 0  , ActInhibit       }, /* */
   /*   34 */ { ACTION_SET_FILE_DATE,  0  | BP2| BP3| 0  , ActSetFileDate   }, /* */
   /*   40 */ { ACTION_SAME_LOCK,      0  | 0  | 0  | 0  , ActSameLock      }, /* */
   /*   82 */ { ACTION_READ,           0  | 0  | 0  | 0  , ActRead          }, /* */
   /*   87 */ { ACTION_WRITE,          0  | 0  | 0  | 0  , ActWrite         }, /* */
   /* 1001****{ ACTION_READ_RETURN,    0  | 0  | 0  | 0  , NULL             },*****/
   /* 1002****{ ACTION_WRITE_RETURN,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 1004 */ { ACTION_FIND_WRITE,     BP1| BP2| BP3| 0  , ActFindwrite     }, /* */
   /* 1005 */ { ACTION_FIND_INPUT,     BP1| BP2| BP3| 0  , ActFindwrite     }, /* */
   /* 1006 */ { ACTION_FIND_OUTPUT,    BP1| BP2| BP3| 0  , ActFindwrite     }, /* */
   /* 1007 */ { ACTION_END,            0  | 0  | 0  | 0  , ActEnd           }, /* */
   /* 1008 */ { ACTION_SEEK,           0  | 0  | 0  | 0  , ActSeek          }, /* */
   /* 1020 */ { ACTION_FORMAT,         0  | 0  | 0  | 0  , ActFormat        }, /* */
   /* 1021 */ { ACTION_MAKE_LINK,      BP1| BP2| BP3| 0  , ActMakeLink      }, /* */
   /* 1022****{ ACTION_SET_FILE_SIZE,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1023****{ ACTION_WRITE_PROTECT,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 1024****{ ACTION_READ_LINK,      0  | 0  | 0  | 0  , NULL             },*****/
   /* 1026 */ { ACTION_FH_FROM_LOCK,   0  | 0  | 0  | 0  , ActFHFromLock    }, /* */
   /* 1027 */ { ACTION_IS_FILESYSTEM,  0  | 0  | 0  | 0  , ActIsFS          }, /* */
   /* 1028 */ { ACTION_CHANGE_MODE,    0  | 0  | 0  | 0  , ActChangeMode    }, /* */
   /* 1030 */ { ACTION_COPY_DIR_FH,    0  | 0  | 0  | 0  , ActCopyDirFH     }, /* */
   /* 1031 */ { ACTION_PARENT_FH,      0  | 0  | 0  | 0  , ActParentFH      }, /* */
   /* 1033 */ { ACTION_EXAMINE_ALL,    0  | 0  | 0  | 0  , ActExamineAll    }, /* */
   /* 1034 */ { ACTION_EXAMINE_FH,     0  | 0  | 0  | 0  , ActExamineFH     }, /* */
   /* 2010 */ { ACTION_HANDLER_DEBUG,  0  | 0  | 0  | 0  , ActSetDebug      }, /* */
   /* 2011****{ ACTION_SET_TRANS_TYPE, BP1| 0  | 0  | 0  , NULL             },*****/
   /* 2012 */ { ACTION_NETWORK_HELLO,  BP1| BP2| BP3| 0  , ActNetHello      }, /* */
   /* 4097****{ ACTION_ADD_NOTIFY,     0  | 0  | 0  | 0  , NULL             },*****/
   /* 4098****{ ACTION_REMOVE_NOTIFY,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 5554****{ ACTION_NETWORK_START,  0  | 0  | 0  | 0  , NULL             },*****/
   /* 5555****{ ACTION_NETWORK_INIT,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 5556****{ ACTION_NETWORK_TERM,   0  | 0  | 0  | 0  , NULL             },*****/
   /* 5557 */ { ACTION_NETWORK_PASSWD, BP1| BP2| 0  | 0  , ActNetPwd        }, /* */
  };
#define WORKSIZE (sizeof(worktab)/sizeof(struct LookupTable))


extern struct Library *SysBase;
struct DosLibrary *DOSBase;

void _main(char *x);

void
_main(char *x)
{
   HPACKET          *hpkt;       /* The RPacket for the current action*/
   HPACKET          *thp, *lhp;
   struct DosPacket *dpkt, *pkt;
   int         action;
   int         low, high, this;
   ULONG           sigmask;
   ifuncp              subr;
   int         flags;
   struct global      global;
   DRIVER           *driver;
   struct Library *DriverBase;
   struct RPacket   *RP;
   struct Message *msg;
   struct MsgPort *replyport;

   MWInit(NULL, MWF_NOCHECK, "CON:0/0/639/199/NET handler memory debug");

   /* Initialize our global data structure */
   memset((char *)&global, 0, sizeof(struct global));

   global.n.self   = (struct Process *) FindTask(0L);  /* find myself      */
   global.n.run    = 1;
   global.n.port   = CreatePort("NET Access Port", 0L);
   global.flags    = NGF_RELOCK;
   /* install our taskid ...   */


   /* Initialize the intuitext structures for the requesters we might have   */
   /* to display                                                              */
   /* Because we have no scruples we can cheat and do this with a couple of  */
   /* long word assignments.  We leave the acual C code commented out here   */
   /* so that if this structure ever changed we will still be able to work   */
#if 0
   global.n.line1.FrontPen = global.n.line1.BackPen = -1;
   global.n.line1.DrawMode = JAM1;
   global.n.line1.LeftEdge = global.n.line1.TopEdge = 4;
   global.n.line2 = global.n.line1;
   global.n.line3 = global.n.line1;
   global.n.retrytxt = global.n.line1;
   global.n.canceltxt = global.n.line1;
#else
   *(long *)&global.n.line1.FrontPen     = 0x00010000L | (JAM1<<8);
   *(long *)&global.n.line1.LeftEdge     = 0x00040004L;  /* 4,4  */
   *(long *)&global.n.line2.FrontPen     = 0x00010000L | (JAM1<<8);
   *(long *)&global.n.line2.LeftEdge     = 0x0004000EL;  /* 4,14 */
   *(long *)&global.n.line3.FrontPen     = 0x00010000L | (JAM1<<8);
   *(long *)&global.n.line3.LeftEdge     = 0x00040018L;  /* 4,24 */
   *(long *)&global.n.retrytxt.FrontPen  = 0x00010000L | (JAM1<<8);
   *(long *)&global.n.retrytxt.LeftEdge  = 0x00040004L;
   *(long *)&global.n.canceltxt.FrontPen = 0x00010000L | (JAM1<<8);
   *(long *)&global.n.canceltxt.LeftEdge = 0x00040004L;
#endif
   global.n.retrytxt.IText = "Retry";
   global.n.canceltxt.IText = "Cancel";

   BUGP("Net Handler Starting")

   InitDevice(&global);

   BUGP("Net Handler Initialized");

   Mount(&global, NULL);

   BUGP("Net Handler Main");

   OpenTimer(&global, global.n.port);

   PostTimerReq(&global, HPTICK);

   global.waitbits |= (1 << global.n.port->mp_SigBit);
   
   driver = NULL;
   while(global.n.run)    /* start of the real work */
   {
      do
      {
         hpkt = NULL;

#if DEBUG
         if(memcheck) MWCheck();
#endif

         /* Clear up any packets waiting for our action */
         while(global.donepkts) 
         {
            thp = global.donepkts;
            global.donepkts = global.donepkts->hp_WNext;
            retpkt(&global, thp);
         }

         if(driver == NULL)
         {
            /* Check for new messages from the application */
            if(msg = GetMsg(global.n.port))
            {
               /* This is a NEW packet from an application */
               pkt = (struct DosPacket *)msg->mn_Node.ln_Name;

               BUG(("NEW 0x%08lx type %d args (%08lx,%08lx,%08lx,%08lx)\n",
                  pkt, pkt->dp_Type, pkt->dp_Arg1, pkt->dp_Arg2, 
                                     pkt->dp_Arg3, pkt->dp_Arg4))

               if(!(hpkt = GetHPacket(&global)))
               {
                  pkt->dp_Res1 = DOS_FALSE;
                  pkt->dp_Res2 = ERROR_NO_FREE_STORE;
                  replyport = pkt->dp_Port;
                  msg = pkt->dp_Link;
                  pkt->dp_Port = global.n.port;
                  PutMsg(replyport, msg);
               }
               else
               {
                  /* We have a new hpkt structure - initialize it */
                  hpkt->hp_Pkt = pkt;
                  hpkt->hp_NNode = NULL;
                  hpkt->hp_RP = NULL;
                  hpkt->hp_Driver = NULL;
                  hpkt->hp_Func = NULL;
                  hpkt->hp_NPFlags = 0;
                  hpkt->hp_PktFlags = 0;
               }
               break;
            }

            sigmask = Wait(global.waitbits);
            driver = global.drivers;
         }

         while(driver != NULL)
         {
            if(!(driver->sigmask & sigmask))
            {
               /* This one can't be it, the bits don't match */
               driver = driver->next;
            }
            else
            {
               /* Looks like an eligible driver, ask if he has anything */
               DriverBase = driver->libbase;
               switch(SDNReceive(driver->drglobal, sigmask, &RP))
               {
                  case SDN_OK: 
                     /* We have a valid RPacket back from the device */
                     hpkt = (HPACKET *)RP->handle;

#if DEBUG
                     if(!hpkt) 
                     {
                        request((GLOBAL)&global, REQ_GENERAL, "RCV NO HANDLE!");
                        BUG(("RCV NO HANDLE!!!\n"))
                        break;
                     }
                     if(hpkt->hp_Pkt)
                        BUG(("RCV 0x%08lx type %d args (%08lx,%08lx,%08lx,%08lx)\n",
                           hpkt->hp_Pkt, hpkt->hp_Pkt->dp_Type, 
                           hpkt->hp_Pkt->dp_Arg1, hpkt->hp_Pkt->dp_Arg2, 
                           hpkt->hp_Pkt->dp_Arg3, hpkt->hp_Pkt->dp_Arg4))
                     else
                        BUG(("RCV NULL PKT\n"))
#endif
                     for(lhp=NULL, thp=global.qpkts; 
                         thp && thp != hpkt; 
                         lhp=thp, thp=thp->hp_WNext);

                     if(thp)
                     {
                        if(lhp) lhp->hp_WNext = hpkt->hp_WNext;
                        else    global.qpkts = hpkt->hp_WNext;
                        hpkt->hp_RP = RP;
                        retpkt(&global, hpkt);
                     }
#if DEBUG
                     else
                        request((GLOBAL)&global, REQ_GENERAL,
                                "Bad hpkt from SDNReceive!");
#endif
                     hpkt = NULL;
                     break;

                  case SDN_ERR:
#if DEBUG
                     request((GLOBAL)&global, REQ_GENERAL,
                             "SDN_ERR from SDNReceive");
#endif
                     break;

                  case SDN_NONE:
                     driver = driver->next; 
                     break;
               }
            }
         }

      }
      while(hpkt == NULL);

      dpkt = hpkt->hp_Pkt;

      action = dpkt->dp_Type;

      low = 0;
      high = WORKSIZE;
      subr = NULL;
      while (low <= high)
      {
         this = (high-low)/2 + low;
         if (worktab[this].num == action)
         {
            subr = worktab[this].subr;
            flags = worktab[this].flags;
            break;
         }
         if (worktab[this].num < action)
            low = this + 1;
         else
            high = this - 1;
      }

      dpkt->dp_Res1 = DOS_FALSE;
      dpkt->dp_Res2 = ERROR_ACTION_NOT_KNOWN;

      if (subr != NULL) 
      {
         global.n.reply = 1;
         if (flags & BP1) dpkt->dp_Arg1 <<= 2;
         if (flags & BP2) dpkt->dp_Arg2 <<= 2;
         if (flags & BP3) dpkt->dp_Arg3 <<= 2;
         if (flags & BP4) dpkt->dp_Arg4 <<= 2;
         hpkt->hp_PktFlags = flags;

         (*subr)(&global, hpkt);

      }
      else 
      {
         BUG(("***Unknown packet type %ld\n",dpkt->dp_Type));
         retpkt(&global, hpkt);
      }
   }

    /* do our final cleanup */
    global.node->dn_Task = FALSE; /* zero the taskid field of device node */
    global.node->dn_SegList = 0;  /* make us be gone */

    DisMount(&global);
    TermDevice(&global);

#if TIMEDEBUG
    CloseTimer(&global);
#endif

    MWTerm();

    BUGTERM()
}

void KillHPackets(GLOBAL global, struct NetNode *nnode, int errmsg)
{
   HPACKET *thp, *lhp, *nhp;
   thp = global->qpkts;
   lhp = NULL;
   while(thp)
   {
      if(thp->hp_NNode == nnode)
      {
         if(lhp) lhp->hp_WNext = nhp = thp->hp_WNext;
         else    global->qpkts = nhp = thp->hp_WNext;
         thp->hp_State = HPS_ERR;
         /* Best guess at a reasonable error message */
         if(thp->hp_Pkt)
         {
            thp->hp_Pkt->dp_Res1 = DOS_FALSE;
            thp->hp_Pkt->dp_Res2 = errmsg;
         }
         HPQ(global, thp);
         thp = nhp;
         BUG(("***KILLING 0x%08lx\n", thp->hp_Pkt))
      }
      else
      {
         lhp = thp;
         thp=thp->hp_WNext;
      }
   }
}

ACTFN(retpkt)
{
   struct Message *mess;
   struct MsgPort *replyport;
   struct DosPacket *pkt;
   struct Library *DriverBase;
   ifuncp func;


   if(hpkt->hp_State == HPS_RETRY)
   {
      hpkt->hp_State = HPS_NEW;
      RemotePacket(global, hpkt);
   }
   else
   {
      if(func=hpkt->hp_Func)
      {
         /* The secondary function must be called.  If the packet is to be */
         /* returned after this call, the secondary function will be NULL  */
         /* after it is called.  If there is more to do, the secondary     */
         /* function itself will replace the function pointer and requeue  */
         /* the packet when appropriate.                                   */

         hpkt->hp_Func = NULL;
         (*func)(global, hpkt);
      }

      if(!hpkt->hp_Func)
      {
         /* No more functions, might as well get rid of it now */
         if(pkt = hpkt->hp_Pkt)
         {
            if (hpkt->hp_PktFlags & BP1) pkt->dp_Arg1 >>= 2;
            if (hpkt->hp_PktFlags & BP2) pkt->dp_Arg2 >>= 2;
            if (hpkt->hp_PktFlags & BP3) pkt->dp_Arg3 >>= 2;
            if (hpkt->hp_PktFlags & BP4) pkt->dp_Arg4 >>= 2;

            replyport        = pkt->dp_Port;
            mess             = pkt->dp_Link;
            pkt->dp_Port     = global->n.port;

            if(hpkt->hp_RP)
            {
               DriverBase = hpkt->hp_Driver->libbase;
               SDNFreeRPacket(hpkt->hp_Driver->drglobal, hpkt->hp_RP);
            }

            BUG(("RET 0x%08lx type %d res %d(%08lx) %d(%08lx)\n",
               pkt, pkt->dp_Type, pkt->dp_Res1, pkt->dp_Res1, 
                                  pkt->dp_Res2, pkt->dp_Res2))

            PutMsg(replyport, mess);
         }
#if DEBUG
         else
         {
            BUG(("RET no dos pkt\n"))
         }
#endif

         FreeHPacket(global, hpkt);
      }
   }

}

#if 0
ACTFN(forwardpkt)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   BUG(("forwardpkt: Entry, hpkt 0x%08lx\n", hpkt))

   if (hpkt->hp_PktFlags & BP1) pkt->dp_Arg1 >>= 2;
   if (hpkt->hp_PktFlags & BP2) pkt->dp_Arg2 >>= 2;
   if (hpkt->hp_PktFlags & BP3) pkt->dp_Arg3 >>= 2;
   if (hpkt->hp_PktFlags & BP4) pkt->dp_Arg4 >>= 2;

   PutMsg(global->devslock->fl_Task, (struct Message *)pkt->dp_Link);

   FreeHPacket(global, hpkt);
}
#endif

/* Arg1 - BSTR Name of node */
/* Arg2 - BSTR userid */
/* Arg3 - BPTR challenge buffer */
/* Arg4 - LONG length of challenge */
ACTFN(ActNetHello)
{
   struct RPacket *RP = hpkt->hp_RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   NETPTR nlock;

   if(ParseName(global, (char *)pkt->dp_Arg1, &nlock, global->work) || !nlock)
   {
      pkt->dp_Res1 = DOS_FALSE;  /* Root of NET: */
      pkt->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
      HPQ(global, hpkt);
   }
   else if(!(RP = AllocRPacket(nlock->NetNode, 0)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
   }
   else
   {
      RP->Type = ACTION_NETWORK_HELLO;
      RP->DLen = BSTRLEN(pkt->dp_Arg2);
      RP->DataP = (char *)pkt->dp_Arg2;

      RP->handle = hpkt;

      hpkt->hp_RP = RP;
      hpkt->hp_NPFlags = 0;
      hpkt->hp_NNode = nlock->NetNode;
      hpkt->hp_Driver = nlock->NetNode->driver;
      hpkt->hp_Func = ActNetHello2;

      RemotePacket(global, hpkt);
   }
}

ACTFN(ActNetHello2)
{
   struct DosPacket *pkt = hpkt->hp_Pkt;
   struct RPacket *RP = hpkt->hp_RP;

   if(hpkt->hp_State != HPS_ERR)
   {
      if(RP->Arg1 == DOS_FALSE)
      {
         pkt->dp_Res1 = DOS_FALSE;
      }
      else
      {
         if(pkt->dp_Arg4 < RP->DLen)
         {
            pkt->dp_Res1 = DOS_FALSE;
            pkt->dp_Res2 = ERROR_OBJECT_TOO_LARGE;
         }
         else
         {
            pkt->dp_Res1 = RP->Arg1;
            memcpy((char *)pkt->dp_Arg3, RP->DataP, RP->DLen);
         }
      }
      pkt->dp_Res2 = RP->Arg2;
   }
}

/* Arg1 - BSTR Name of node */
/* Arg2 - BSTR response buffer */
ACTFN(ActNetPwd)
{
   struct RPacket *RP = hpkt->hp_RP;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   NETPTR nlock;


   if(ParseName(global, (char *)pkt->dp_Arg1, &nlock, global->work) || !nlock)
   {
      pkt->dp_Res1 = DOS_FALSE;  /* Root of NET: */
      pkt->dp_Res2 = ERROR_OBJECT_WRONG_TYPE;
      HPQ(global, hpkt);
   }
   else if(!(RP = AllocRPacket(nlock->NetNode, 0)))
   {
      pkt->dp_Res1 = DOS_FALSE;
      pkt->dp_Res2 = ERROR_NO_FREE_STORE;
      HPQ(global, hpkt);
   }
   else
   {
      RP->Type = ACTION_NETWORK_PASSWD;
      RP->DLen = BSTRLEN(pkt->dp_Arg2);
      RP->DataP = (char *)pkt->dp_Arg2;

      RP->handle = hpkt;

      hpkt->hp_RP = RP;
      hpkt->hp_NPFlags = 0;
      hpkt->hp_NNode = nlock->NetNode;
      hpkt->hp_Driver = nlock->NetNode->driver;
      hpkt->hp_Func = NULL;

      RemotePacket(global, hpkt);
   }
}

                        /* DP_Arg1 - LONG type/flags 0=nodebug          */
                        /* DP_Arg2 - BPTR FileHandle for debug or NULL  */
                        /* DP_Res1 - BPTR old filehandle                */
ACTFN(ActSetDebug)
{
#if DEBUG
    /********************************************************************/
    /*                                                                  */
    /* Several possibilities:                                           */
    /* 1. Arg1 is 0:                                                    */
    /*       All debugging is turned off.  Arg2 is not looked at.       */
    /*       Res1 is DOS_TRUE.  Old log file is returned in dp_Res2.    */
    /*                                                                  */
    /* 2. Arg1 is a special handler-defined code:                       */
    /*       If the second bit from the top is ON, the code is a special*/
    /*       debugging command to the handler.  If the handler knows the*/
    /*       command, it returns DOS_TRUE in Res1.  If it doesn't, it   */
    /*       returns DOS_FALSE in Res1 and Res2.                        */
    /*                                                                  */
    /* 3. Neither of the above:                                         */
    /*       Arg2 contains a BPTR to a FileHandle to send debugging to. */
    /*       Note that if this is NULL, debugging will be turned off.   */
    /*       If Arg1 > 1, Arg3 contains a BSTR filename of a file on    */
    /*       network node the user wants to debug.  The debug request   */
    /*       should be passed along to that node.                       */
    /*       Res1 contains DOS_TRUE.  Old log file returned in dp_Res2. */
    /*                                                                  */
    /********************************************************************/

#define DEBUG_SPECIAL 0x40000000   /* Mask for handler-defined dbg type*/
#define DEBUG_WAIT    0x40000001   /* Wait for debugger to catch us    */
#define DEBUG_INFO    0x40000002   /* Send transmit info to msgport in */
                                   /* dp_Arg2                          */
#define DEBUG_OPKTS   0x40000003   /* Send info on outstanding hpkts   */
#define DEBUG_TERM    0x40000004   /* Terminate specified connection   */
#define DEBUG_MEMCHK  0x40000005   /* Change memory check frequency    */

   extern BPTR debuglog;
   struct DosPacket *pkt = hpkt->hp_Pkt;
   NETPTR nlock;

BUGP("ActSetDebug Entry")

   pkt->dp_Res1 = DOS_TRUE;
   pkt->dp_Res2 = 0;

   if(pkt->dp_Arg1 & DEBUG_SPECIAL) 
   {
BUGP("DEBUG_SPECIAL")
      if (pkt->dp_Arg1 == DEBUG_WAIT) 
      {
         pkt->dp_Res2 = NULL;
      } 
      else if(pkt->dp_Arg1 == DEBUG_INFO) 
      {
BUGP("DEBUG_INFO")
         global->n.infoport = (struct MsgPort *)BADDR(pkt->dp_Arg2);
         global->n.ntirec.m.mn_Node.ln_Type =
         global->n.ntitrans.m.mn_Node.ln_Type = NT_MESSAGE;
         if (!global->n.ntirec.m.mn_ReplyPort) 
         {
            global->n.ntirec.m.mn_ReplyPort =
            global->n.ntitrans.m.mn_ReplyPort = CreatePort(NULL,0);
         }
         global->n.inf_rec = global->n.inf_trans = 0L;
      }
      else if(pkt->dp_Arg1 == DEBUG_OPKTS)
      {
         HPACKET *thp;
         for(thp = global->qpkts; 
             thp && thp != (HPACKET *)pkt->dp_Arg2;
             thp = thp->hp_WNext);
         pkt->dp_Res1 = DOS_TRUE;
         pkt->dp_Res2 = (LONG)(thp ? thp->hp_WNext : global->qpkts);
      } 
      else if(pkt->dp_Arg1 == DEBUG_TERM)
      {
         if(ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work) || !nlock)
         {
            /* NET: root - what does this mean? */
            pkt->dp_Res1 = DOS_FALSE;
            pkt->dp_Res2 = ERROR_WRITE_PROTECTED;
         }
         else
         {
            struct Library *DriverBase = nlock->NetNode->driver->libbase;
            SDNTermNode(nlock->NetNode->driver->drglobal, nlock->NetNode->ioptr);
            nlock->NetNode->status = NODE_CRASHED;
            pkt->dp_Res1 = DOS_TRUE;
            pkt->dp_Res2 = 0;
            KillHPackets(global, nlock->NetNode, ERROR_OBJECT_NOT_FOUND);
         }
      }
      else if(pkt->dp_Arg1 = DEBUG_MEMCHK)
      {
         MWCheck();
         memcheck = pkt->dp_Arg2;
      }
      else 
      {
         BUGP("***Unknown debug type")
         pkt->dp_Res1 =
         pkt->dp_Res2 = DOS_FALSE;
      }
      HPQ(global, hpkt);
   }
   else
   {
      /* New handd puts '2' in arg1;  if we get '1', it's old-style and */
      /* there is no name in the dp_Arg3 field.                         */

      if(pkt->dp_Arg1 == 1 ||
         ParseName(global, (char *)pkt->dp_Arg3, &nlock, global->work))
      {
         /* It's the NET: root;  that means debug the handler */
         if(pkt->dp_Arg1) 
         {
            BUGP("Calling initdebug")
            pkt->dp_Res2 = initdebug((BPTR)pkt->dp_Arg2);
         } 
         else 
         {
            BUGP("Terminating debug")
            pkt->dp_Res2 = debuglog;
            debuglog = NULL;
         }
         HPQ(global, hpkt);
      }
      else if(!nlock)
      {
         /* No such node */
         pkt->dp_Res1 = DOS_FALSE;
         pkt->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
         HPQ(global, hpkt);
      }
      else
      {
         /* It's a valid remote node */
         struct RPacket *RP;
         if(!(RP = AllocRPacket(nlock->NetNode, 0)))
         {
            pkt->dp_Res1 = DOS_FALSE;
            pkt->dp_Res2 = ERROR_NO_FREE_STORE;
            HPQ(global, hpkt);
            return;
         }
         RP->Type = pkt->dp_Type;
         RP->Arg1 = pkt->dp_Arg1;
         RP->Arg2 = 0;
         RP->DLen = 0;

         RP->handle = hpkt;

         hpkt->hp_RP = RP;
         hpkt->hp_NPFlags = 0;
         hpkt->hp_NNode = nlock->NetNode;
         hpkt->hp_Driver = nlock->NetNode->driver;

         RemotePacket(global, hpkt);
      }
   }
#endif
BUGP("Exiting")
}


void MemCleanup(void);
void MemCleanup(void)
{
}

#if MWDEBUG == 0
char *
DosAllocMem(global, len)
GLOBAL global;
long len;
{
    long *p;

    if ((p = (long *)AllocMem(len+4, MEMF_PUBLIC | MEMF_CLEAR)) == NULL) 
    {
	    /* Gee.  Out of memory AND there is nobody to tell about it ...  */
	    /* Only choice is to GURU.	Maybe we could do something clever   */
	    /* but I doubt it...					     */
	    BUG(("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!               !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!! OUT OF MEMORY !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!               !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	}
   else
   {
	   *p++ = len;
   }
   return((char *)p);
}

void
DosFreeMem(p)
char *p;
{
    long *lp;
    long len;

    lp = (long *)p;
    len = *--lp;
    FreeMem((char *)lp, len+4);
}

#endif
