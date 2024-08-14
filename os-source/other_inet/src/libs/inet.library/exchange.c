
#include <exec/types.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <utility/tagitem.h> /* 2.0 only ? */

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>


#include <sana2.h>
#include <sana2specialstats.h>


#include "exchange.h"
#include "proto.h"

#define NUMREADS 16                             /* Number of CMD_READs kept queued at all times */


 struct s2_inddev
    {
    struct Node   s2_next;
    struct arpcom s2_ac;            /* fodder for ARP */
    struct Device *s2_DevBase;      /* Saved for Sana2 device */
    struct Unit   *s2_UnitNum;      /* Saved for Sana2 device */
    void          *s2_BuffMan;      /* ptr to BufMan stuff */
    UWORD         s2_Flags;         /* See below for flag defs */
    ULONG         s2_MTU;           /* Max. Transfer Unit size.  "biggest packet allowed" */
    long          s2_HardwareType;  /* Defined interface type (see RFC 1060) */
    UWORD         s2_AddressSize;   /* Size of the address field for this device (in bits) */
    long          s2_BPS;           /* Number of bits per second this ndevices allows */
    BOOL          s2_UseArp;        /* TRUE if ARP is to be used for this address */
    BOOL          s2_NIPC;          /* TRUE if cmd_reads come from NIPC, rather than the device itself */
    ULONG         s2_IPType;        /* types for these kind of packets */
    ULONG         s2_ARPType;       /* ditto */
    ULONG         s2_TempAddress;   /* Temp IP address held here */
    ULONG         s2_BitMask;
    };



extern BOOL CTB();
extern struct MinList interface_list;
extern BOOL queueread();
struct MsgPort    *NIPCDataPort;
struct MsgPort    *INETPort;
extern ULONG s2event;

BOOL AttemptExchange(devbase,unit)
struct Device *devbase;
struct Unit   *unit;
{

 struct MsgPort *NIPCPort;
 struct xstart smsg;
 struct MsgPort *reply;

 Forbid();
 if (!(NIPCPort = (struct MsgPort *) FindPort("NIPC Exchange")))         /* Can't find nipc ... */
   {
   Permit();
   return (FALSE);
   }

 smsg.xs_Device = devbase;
 smsg.xs_Unit = unit;
 smsg.xs_CTB = &CTB;
 smsg.xs_Command = XCMD_START;

 reply = (struct MsgPort *) CreateMsgPort();
 smsg.xs_Msg.mn_ReplyPort = reply;
 PutMsg(NIPCPort,(struct Message *) &smsg);
 Permit();
 while (!GetMsg(reply))
    WaitPort(reply);

 DeleteMsgPort(reply);

 if (!smsg.xs_Link)                       /* If NULL, NIPC knows nothing of that device. */
   return(FALSE);

 NIPCDataPort = smsg.xs_Link;
 return(TRUE);

}

void EndExchange(devbase,unit)
struct Device *devbase;
struct Unit   *unit;
{

struct xstart smsg;
struct MsgPort *NIPCPort;
struct MsgPort *reply;

 smsg.xs_Device = devbase;
 smsg.xs_Unit   = unit;
 smsg.xs_Command = XCMD_END;

 reply = (struct MsgPort *) CreateMsgPort();
 smsg.xs_Msg.mn_ReplyPort = reply;

 Forbid();
 if (NIPCPort = (struct MsgPort *) FindPort("NIPC Exchange"))
   {
   PutMsg(NIPCPort,(struct Message *) &smsg);
   Permit();
   WaitPort(reply);
   GetMsg(reply);
   }
 else
   Permit();

 DeleteMsgPort(reply);

}

void ImitationSendIO(ioreq)
struct Message *ioreq;
{
// kprintf("Dport is %lx\n",NIPCDataPort);
 PutMsg(NIPCDataPort,ioreq);
}

void InitExchange()
{

 INETPort = (struct MsgPort *) CreateMsgPort();
 if (!INETPort)
   return;

 INETPort->mp_Node.ln_Name = "INET Exchange";
 AddPort(INETPort);
 s2event |= (1 << (INETPort->mp_SigBit));

}

void DeinitExchange()
{

 if (!INETPort) return;

 RemPort(INETPort);
 DeleteMsgPort(INETPort);

}

void PollExchange()
{
 struct xstart *mxs;

 while (mxs = (struct xstart *) GetMsg(INETPort))
   {
//   kprintf("Inet got a command on it's port.  Cmd = %lx\n",mxs->xs_Command);
   if (mxs->xs_Command == XCMD_START)
      {
      /* NIPC is telling us that it has ahold of the device xs_Device, and unit xs_Unit */
      /* Check our device list, and see if that matches any of our own stuff */
      if (!IsListEmpty( (struct List *) &interface_list ))
         {
         struct s2_inddev *s;

         NIPCDataPort = mxs->xs_Link;     /* So we know where to send requests */
         s = (struct s2_inddev *) interface_list.mlh_Head;
         while(s->s2_next.ln_Succ)
            {
//            kprintf("inet is looking for matches .. \n");
            if ((mxs->xs_Device == s->s2_DevBase) &&
                (mxs->xs_Unit == s->s2_UnitNum))
               {
               /* Abort all current read requests via a CMD_FLUSH */
               struct IOSana2Req ior;
               int x;
//               kprintf("found ourself a match!  trying to CMD_FLUSH .. \n");
               ior.ios2_Req.io_Device = s->s2_DevBase;
               ior.ios2_Req.io_Unit = s->s2_UnitNum;
               ior.ios2_Req.io_Command = CMD_FLUSH;
               ior.ios2_Req.io_Message.mn_ReplyPort = INETPort;   /* Use INETPort as a reply port :') */
               DoIO((struct IORequest *) &ior);
//               kprintf("flushed\n");
               /* Mark this entry as communicating through NIPC */
               s->s2_NIPC = TRUE;
               mxs->xs_CTB = &CTB;
               mxs->xs_Link = 0L;
//               kprintf("queueing up new ones \n");
               }
//            kprintf("checking still ..\n");
            s = (struct s2_inddev *) s->s2_next.ln_Succ;
            }

         }

      }

   if (mxs->xs_Command == XCMD_END)
      {
      if (!IsListEmpty( (struct List *) &interface_list ))
         {
         struct s2_inddev *s;
         s = (struct s2_inddev *) interface_list.mlh_Head;
         while (s->s2_next.ln_Succ)
            {
            if (s->s2_NIPC == TRUE)
               {
               int x;
               s->s2_NIPC = FALSE;
               /* Old ones are automatically flushed by nipc - with a noted error code, so
                * that inet doesn't requeue them. */

               /* Queue up new ones */
               for (x=0;x<NUMREADS;x++)
                  queueread(s,AF_INET);
               if (s->s2_UseArp) queueread(s,AF_UNSPEC);
               /* ^^ issue an arp read req if needed ^^ */
               }
            s = (struct s2_inddev *) s->s2_next.ln_Succ;
            }
         }
      }

   ReplyMsg( (struct Message *) mxs);
//   kprintf("replied to it\n");
   }

}

