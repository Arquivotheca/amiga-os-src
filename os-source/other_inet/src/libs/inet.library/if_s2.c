
#include <exec/types.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <utility/tagitem.h> /* 2.0 only */

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
#include <string.h>
#include "proto.h"


#define NUMREADS 16                             /* Number of CMD_READs kept queued at all times */
#define mbufsize 512-16                         /* how big each mbuf is.  (I SHOULD attempt to get the data from mbuf.c/h) */

long s2event;
extern long event;
int netisr;

ULONG badqueues=0L;

extern BOOL ACTB(void);
extern BOOL ACFB(void);
extern long AP;

static long s2tags[5] = {S2_CopyToBuff, &ACTB,S2_CopyFromBuff, &ACFB, TAG_DONE};
static long notags[2] = {TAG_DONE, 0};


/* Individual interface structure - through this, we, (and through a substructure), ARP
 * keep track of each individual sana2 device.
 */

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


/* The following structure is a superset of an IOSana2Req - with one added field -
 * a pointer to an mbuf.  This allows me to continually associate an ioreq's NetBuffSegs
 * with a list of mbufs.
 */
 struct IOSana2ReqSuper
    {
    struct IOSana2Req ss;
    struct s2_inddev *S2io_dev;
    };


#define IFS2_OFFLINE 1
#define IFS2_BOOTED  2

 struct MinList interface_list;
 struct MsgPort *returnport;

char nibvert(ichar)
char ichar;
{
 if (ichar > 96) return( (char) (ichar-87));
 if ((ichar >= '0') && (ichar <= '9')) return((char) (ichar-'0'));
 else return((char) (ichar-55));
}

/* s2attach
 * Open up all appropiate sana2 devices.
 *
 */
void s2attach()
{

 struct s2_inddev *curdev;
 struct IOSana2ReqSuper ioreq;
 struct Sana2DeviceQuery s2dq;
 long table[5], *hardind;
 long fhandle;
 char linebuff[256], tbuff[80], *hardstring;
 struct RDArgs *myrdargs;
 int unitno, sanaunit,parselength, x;
 ulong copylength, bitmask, *tmpptr;
// ulong *tmpptr2;
 ulong  tmpaddr, copysize;
 struct ifnet *ifptr;
 ulong IPType, ARPType;
 UBYTE hardaddr[16];
 BOOL override;
 UBYTE *srcdata,*destdata;


 NewList((struct List *) &interface_list);
 bzero(&s2dq,sizeof(struct Sana2DeviceQuery));
 returnport = (struct MsgPort *) CreatePort("Sana II ReplyPort",0L);
 sanaunit = 0;

 fhandle = Open("inet:s/SANA2_devs",MODE_OLDFILE);
 if (!fhandle)
   {
   return;
   }
 myrdargs = (struct RDArgs *) AllocDosObject(DOS_RDARGS,(struct TagItem *) &notags);
 myrdargs->RDA_Buffer = (UBYTE *) &tbuff;
 myrdargs->RDA_BufSiz = 80;

 while (FGets(fhandle,(UBYTE *) &linebuff,256))
   {
   myrdargs->RDA_DAList = NULL;
   myrdargs->RDA_Flags = 0;
   if (linebuff[0] == '\n') break;
   if ((linebuff[0] == '#') || (linebuff[0] == ';')) continue;
   myrdargs->RDA_Source.CS_Buffer = (UBYTE *) &linebuff;
   myrdargs->RDA_Source.CS_Length = 256;
   myrdargs->RDA_Source.CS_CurChr = 0;
   ReadArgs("devname/A,unit/A,ip/A,arp/A,hardwareaddress/M",(ULONG *) &table,myrdargs);
   if (!((table[0]) && (table[1]))) continue;

   override = FALSE;
   hardind = (long *) table[4];
   if (hardind != 0)
      {
      hardstring = (char *) hardind[0];
      if (hardstring)
         {
         if (!strncmp("0x",hardstring,2))
            {
            override = TRUE;
            parselength = strlen(hardstring);
            if ((parselength & 1L) == 1) strcpy(hardstring+1,hardstring+2);
            else strcpy(hardstring, hardstring+2);
            for (x=0; x< strlen(hardstring); x+=2)
               {
               hardaddr[x/2] = ((nibvert(hardstring[x]) << 4) | (nibvert(hardstring[x+1])));
               }
            }
         }
      }


   ioreq.ss.ios2_Req.io_Message.mn_ReplyPort = returnport;

   curdev = (struct s2_inddev *) AllocMem(sizeof(struct s2_inddev),MEMF_CLEAR);

   ioreq.ss.ios2_BufferManagement = (void *) &s2tags;
   stcd_i((char *) table[1],&unitno);
   stcd_i((char *) table[2],&IPType);
   stcd_i((char *) table[3],&ARPType);
   if (!OpenDevice((char *) table[0], unitno, (struct IORequest *) &ioreq, 0))
      {
      if (AttemptExchange(ioreq.ss.ios2_Req.io_Device,ioreq.ss.ios2_Req.io_Unit))
         curdev->s2_NIPC = TRUE;
      else
         curdev->s2_NIPC = FALSE;

      AddTail((struct List *)&interface_list,(struct Node *)curdev);
      ioreq.ss.ios2_Req.io_Command = S2_DEVICEQUERY;
      ioreq.ss.ios2_StatData = (void *) &s2dq;
      s2dq.SizeAvailable = sizeof(struct Sana2DeviceQuery);
      DoIO((struct IORequest *) &ioreq);
      curdev->s2_IPType = IPType;
      curdev->s2_ARPType = ARPType;
      curdev->s2_MTU = s2dq.MTU;
      curdev->s2_BPS = s2dq.BPS;
      curdev->s2_HardwareType = s2dq.HardwareType;
      curdev->s2_UseArp = FALSE;
      if (s2dq.HardwareType == 1) curdev->s2_UseArp = TRUE;
      curdev->s2_AddressSize = s2dq.AddrFieldSize;
      curdev->s2_DevBase = ioreq.ss.ios2_Req.io_Device;
      curdev->s2_ac.ac_if.if_name = "s";
      curdev->s2_ac.ac_if.if_unit = sanaunit;
      curdev->s2_ac.ac_if.if_mtu = s2dq.MTU;
      curdev->s2_ac.ac_if.if_flags = IFF_BROADCAST;
      curdev->s2_ac.ac_if.if_ioctl = s2ioctl;
      curdev->s2_ac.ac_if.if_output = s2output;
      curdev->s2_ac.ac_if.if_init = s2init;

      if (!override)
         {
         ioreq.ss.ios2_Req.io_Command = S2_GETSTATIONADDRESS; /* returns default address in S2io_DstAddr */
         DoIO((struct IORequest *) &ioreq);
         memcpy(&(ioreq.ss.ios2_SrcAddr),&(ioreq.ss.ios2_DstAddr),16);
         }
      else
         {
         memcpy(&(ioreq.ss.ios2_SrcAddr),&hardaddr,(curdev->s2_AddressSize)/8);
         }

      ioreq.ss.ios2_Req.io_Command = S2_CONFIGINTERFACE;   /* and then sets the current address from what's in S2io_SrcAddr */
      DoIO((struct IORequest *) &ioreq);
      if (ioreq.ss.ios2_Req.io_Error)
      {
         ioreq.ss.ios2_Req.io_Command = S2_GETSTATIONADDRESS; /* returns default address in S2io_DstAddr */
         DoIO((struct IORequest *) &ioreq);
      }

      copylength = curdev->s2_AddressSize;
      copysize = ((copylength-1)/8)+1; /* # of bytes of data, rounded up */

      if (curdev->s2_AddressSize == 48) /* ethernet specific */
         memcpy(&(curdev->s2_ac.ac_enaddr),&(ioreq.ss.ios2_SrcAddr),6);
      else
         {
         /* If not an ethernet, things get VERY ugly.  We need to replace
          * the bottom N bits of the already set ethernet address with the
          * address returned by CONFIGINTERFACE, without damaging the
          * other (upper) bits.  So, I create a bitmask to do the job.
          * Some other ugliness occurs because the data here applies
          * to the IP address, beginning on the right, but is presented
          * here as left-justified (bitwise as WELL as bytewise).
          *
          */
         if (copylength > 32)
            {
            copylength = 32;
            copysize = 4;
            }
         if (copylength == 32) bitmask = 0xFFFFFFFF;
         else bitmask = (1 << copylength)-1;
         tmpptr = (ulong *) &(ioreq.ss.ios2_SrcAddr);
         srcdata = (UBYTE *) (tmpptr);
         destdata = (UBYTE *) (((ULONG)&tmpaddr) + (ULONG) (4 - copysize));
         tmpaddr = 0L;
         memcpy(destdata,srcdata,copysize);

         tmpaddr = (tmpaddr >> (8*copysize-copylength));
         ifptr = &(curdev->s2_ac.ac_if);
//         tmpptr2 = (ulong *) &(ifptr->if_addrlist->ifa_dstaddr.sa_data);
//         *tmpptr2 = ((*tmpptr2 & !bitmask) | (tmpaddr & bitmask));
         curdev->s2_TempAddress = tmpaddr;
         curdev->s2_BitMask = bitmask;
         }

      if_attach(&curdev->s2_ac.ac_if);                      /* give everyone some stats on us */
      curdev->s2_UnitNum = ioreq.ss.ios2_Req.io_Unit;
      curdev->s2_BuffMan = ioreq.ss.ios2_BufferManagement;
      sanaunit++;
      }
   else
      {
      FreeMem(curdev,sizeof(struct s2_inddev));
      }
   FreeArgs(myrdargs);
   }
 Close(fhandle);
 FreeDosObject(DOS_RDARGS,myrdargs);

 DeletePort(returnport);
 returnport = (struct MsgPort *) 0;
}

void s2init(int x)
{

 returnport = CreatePort("SanaII ReplyPort",0L);
 s2event = (1 << (returnport->mp_SigBit));

 InitExchange();

}



/* queueread
 *
 * Queues up a single CMD_READ request on the given sana ii device
 *
 * Arguments:
 *      dev - a pointer to a d2_inddev structure (which defines an individual interface)
 *
 * Results:
 *      none.
 */
BOOL queueread(dev, type)
struct s2_inddev *dev;
int type;
{

 struct mbuf *m;
 int mcntr, mneeded;
#ifndef DYNAMIC_ALLOC
 struct mbuf *n;
#endif
 struct IOSana2ReqSuper *rdreq;

 /* First, allocate three mbuf's */

#ifndef DYNAMIC_ALLOC

 if (((ULONG)dev->s2_MTU) < ((ULONG) mbufsize))
   mneeded = 1;
 else
   mneeded = (((ULONG)dev->s2_MTU)/( (ULONG) mbufsize));
//   if (dev->s2_MTU % mbufsize)
 mneeded ++;

 m = 0;
 for (mcntr=0;mcntr < mneeded;mcntr++)
   {
   n = m;
   MGET(m, M_DONTWAIT,MT_DATA);
   if (!m)
   {
        Forbid();
        badqueues++;
        Permit();
        m_freem(n);
        return(FALSE);
   }
   m->m_next = n;
   }
#endif

 rdreq = (struct IOSana2ReqSuper *) CreateIORequest(returnport,sizeof(struct IOSana2ReqSuper));
 if (!rdreq)
 {
    m_freem(m);
    return(FALSE);
 }

 rdreq->ss.ios2_Data = m;
 rdreq->ss.ios2_Req.io_Device = dev->s2_DevBase;
 rdreq->ss.ios2_Req.io_Unit = dev->s2_UnitNum;
 rdreq->ss.ios2_BufferManagement = dev->s2_BuffMan;
 rdreq->ss.ios2_Req.io_Command = CMD_READ;

 if (type == AF_INET)
    rdreq->ss.ios2_PacketType = dev->s2_IPType;
 else
    rdreq->ss.ios2_PacketType = dev->s2_ARPType;

 rdreq->S2io_dev = dev;
 rdreq->ss.ios2_Req.io_Message.mn_ReplyPort = returnport;

 if (dev->s2_NIPC)
   ImitationSendIO((struct Message *) rdreq);
 else
   SendIO((struct IORequest *) rdreq);

 return(TRUE);

}

void s2poll()
{

 struct IOSana2ReqSuper *oldreq;
 struct mbuf *m, *n;
 struct s2_inddev *dev;
 int x;

 PollExchange();        /* Call exchange code */

 while (oldreq = (struct IOSana2ReqSuper *) GetMsg(returnport))
   {
   if (oldreq->ss.ios2_Req.io_Command == CMD_READ)                  /* Oughta be a bunch of CASE statements.  Fix me later. */
      {
      if (!oldreq->ss.ios2_Req.io_Error)
         {
         m = (struct mbuf *) oldreq->ss.ios2_Data;

         dev = oldreq->S2io_dev;
         dev->s2_ac.ac_if.if_ipackets++;
         if (oldreq->ss.ios2_PacketType == dev->s2_IPType)
            {

            DeleteIORequest((struct IORequest *) oldreq);
            n = m_get(M_DONTWAIT,MT_DATA);
            n->m_next = m;

            n->m_len = (sizeof(struct ifnet *));
            *(mtod(n,struct ifnet **)) = &(dev->s2_ac.ac_if);
            schednetisr(NETISR_IP);
            if (IF_QFULL(&ipintrq))
               {
               IF_DROP(&ipintrq);
               m_freem(n);
               }
            else
               {
               IF_ENQUEUE(&ipintrq, n);
               }
            queueread(dev,AF_INET);
            }
         else /* must be an ARP packet */
            {
            DeleteIORequest((struct IORequest *) oldreq);
            n = m_get(M_DONTWAIT,MT_DATA);
            n->m_next = m;
            n->m_len = (sizeof(struct ifnet *));
            *(mtod(n,struct ifnet **)) = &(dev->s2_ac.ac_if);
            arpinput(&(dev->s2_ac),n);
            queueread(dev,AF_UNSPEC);
            }
         }
      else
         {
         /* Whoops!  An error! */
         m = (struct mbuf *) oldreq->ss.ios2_Data;
         m_freem(m);

//         /* If some self-important dipshit takes the unit offline ... */
         if (oldreq->ss.ios2_Req.io_Error == S2ERR_OUTOFSERVICE)
            {
            if (oldreq->S2io_dev->s2_Flags & IFS2_OFFLINE)
            /* We already know about the situation, and it's being taken care of. */
               DeleteIORequest((struct IORequest *) oldreq);
            else
               {
               oldreq->ss.ios2_Req.io_Command = S2_ONEVENT;
               oldreq->ss.ios2_WireError = S2EVENT_ONLINE;
               oldreq->S2io_dev->s2_Flags |= IFS2_OFFLINE;
               SendIO((struct IORequest *) oldreq);
               }
            }
         else
            {
            if ((UBYTE)oldreq->ss.ios2_Req.io_Error != (UBYTE) 255)        /* If not an exchange problem, requeue it, and delete the old one */
               {
               if (oldreq->ss.ios2_PacketType == oldreq->S2io_dev->s2_IPType)
                  queueread(oldreq->S2io_dev,AF_INET);          /* queue the same time as before */
               else
                  queueread(oldreq->S2io_dev,AF_UNSPEC);
               }
            DeleteIORequest((struct IORequest *) oldreq);
            }
         }
      }


   else if ((oldreq->ss.ios2_Req.io_Command == CMD_WRITE) || (oldreq->ss.ios2_Req.io_Command == S2_BROADCAST))
      {
      m = (struct mbuf *) oldreq->ss.ios2_Data;
      m_freem(m);
      DeleteIORequest((struct IORequest *) oldreq);
      }

   else if (oldreq->ss.ios2_Req.io_Command == S2_ONEVENT)
      {
      if (oldreq->ss.ios2_Req.io_Error)            /* If an error, queue it up again ... */
         {
         oldreq->ss.ios2_WireError = S2EVENT_ONLINE;
         SendIO((struct IORequest *) oldreq);
         }
      else
         {                                         /* If we're online again, clear the flag bit, and queue up the read requests */
         oldreq->S2io_dev->s2_Flags &= !IFS2_OFFLINE;
         for (x=0;x<NUMREADS;x++)
            queueread(oldreq->S2io_dev,AF_INET);

         if (oldreq->S2io_dev->s2_UseArp) queueread(oldreq->S2io_dev,AF_UNSPEC);    /* issue an arp read req if needed */
         DeleteIORequest((struct IORequest *) oldreq);
         }
      }

    /* uh-oh.  If it isn't cmd_read and it isn't cmd_write, we're in deep trouble */
   else DeleteIORequest((struct IORequest *) oldreq);

   }


}

/* write a packet (defined by the mbuf passed) */
int s2output(ifp,m,dst)
struct ifnet *ifp;
struct mbuf *m;
struct sockaddr *dst;
{
 struct s2_inddev *sp;
 struct in_addr idst;
 ULONG PType;
 int usetrailers, totallength;
 UWORD x, bitsize;
 u_char edst[6];  /* whoa!  ethernet dependancies.  This'll have to be fixed. */
 char *y, *z;
 ulong w;
 struct ether_header *eh;
 struct IOSana2ReqSuper *outreq;
 struct mbuf *m2;

/* find the appropriate s2_inddev structure */
 if IsListEmpty((struct List *)(&interface_list)) return(0);
 sp = (struct s2_inddev *) interface_list.mlh_Head;
 while ((sp->s2_next.ln_Succ) && (ifp != &(sp->s2_ac.ac_if)))
    sp= (struct s2_inddev *) sp->s2_next.ln_Succ;

 if (ifp != &(sp->s2_ac.ac_if)) return(0);

 if (sp->s2_Flags & IFS2_OFFLINE) return(0);

 outreq = (struct IOSana2ReqSuper *) CreateIORequest(returnport,sizeof(struct IOSana2ReqSuper));
 outreq->ss.ios2_Req.io_Command = CMD_WRITE;

 z = (char *) (&(outreq->ss.ios2_DstAddr));

/* Things get a little ugly here.  Here's the problem:
 * exactly what kind of address each sana 2 device wants is a mysterious question -
 * for ethernet, we need to do ARP - and "convert" the 32 bit internet address to a
 * 48 bit ethernet address.  For our RS-485, each device gets the bottom 8 bits of
 * the IP address.  Who knows what's expected of the next machine down-the-line?
 *
 * Thus, if you're not using ARP, for the time being what you'll get is the bottom n
 * bits in the IP address.  (padded with zeros)
 *
 * To make matters even worse - we need to take care of the situation when this packet
 * isn't an IP packet at all - but rather, an ARP request - with a specifically
 * defined address length.
 *
 */
 if (dst->sa_family == AF_INET)
    {
    PType = sp->s2_IPType;
    idst = ((struct sockaddr_in *) dst)->sin_addr;
    if (sp->s2_UseArp)
        {
        m2 = m;
        if (!arpresolve(&(sp->s2_ac), m2, &idst, edst, &usetrailers))
            {
            DeleteIORequest( (struct IORequest *) outreq);
            return(0);     /* arp can't resolve this */
            }
        else
            {
            memcpy(z,&edst,6);
            }
        }

    else
       {
       /* fixme - I'm not stripping excess bits off of IP addresses.  You'll get multiples of 8 right now. */
       bitsize = x = sp->s2_AddressSize;
       if (x > 32)
         bitsize = x = 32;
       if (x) x = ((x-1)/8)+1;
       y = (char *) ( ((ULONG)&idst) + (4 - x));

       memcpy(&w,y,x);
       w = (w << (x*8-bitsize));
       memcpy(z,&w,x);
       }
    }
 if (dst->sa_family == AF_UNSPEC)
    {
    eh = (struct ether_header *) dst->sa_data;
    memcpy(z,&(eh->ether_dhost),6);
    if (!bcmp(z,&(eh->ether_dhost),6))
      outreq->ss.ios2_Req.io_Command = S2_BROADCAST;
    PType = sp->s2_ARPType;
    }

 totallength = 0;
 m2 = m;
 while (m2)
    {
    totallength += m2->m_len;
    m2 = m2->m_next;
    }

 outreq->ss.ios2_DataLength = totallength;
 outreq->ss.ios2_Data = m;
 sp->s2_ac.ac_if.if_opackets++;
 outreq->ss.ios2_Req.io_Device = sp->s2_DevBase;
 outreq->ss.ios2_Req.io_Unit = sp->s2_UnitNum;
 outreq->ss.ios2_BufferManagement = sp->s2_BuffMan;
 outreq->ss.ios2_PacketType = PType;
 outreq->S2io_dev = sp;
 outreq->ss.ios2_Req.io_Message.mn_ReplyPort = returnport;
 SendIO((struct IORequest *) outreq);
 return(0);
}

int s2ioctl(ifp,cmd,data)
struct ifnet *ifp;
int cmd;
caddr_t data;
{
register struct ifaddr *ifa = (struct ifaddr *) data;
struct s2_inddev *sp;
//ULONG *tmpptr2;
int x;


 switch(cmd)
    {

    case SIOCSIFADDR:
        {
        ifp->if_flags |= IFF_UP;

        /* find the appropriate s2_inddev structure */
        if IsListEmpty((struct List *)(&interface_list)) return(0);
        sp = (struct s2_inddev *) interface_list.mlh_Head;
        while ((sp->s2_next.ln_Succ) && (ifp != &(sp->s2_ac.ac_if)))
           sp= (struct s2_inddev *) sp->s2_next.ln_Succ;
        if (ifp != &(sp->s2_ac.ac_if)) return(0);        /* If we couldn't find it, never mind. */

        if (sp->s2_TempAddress)
            {
            *((ULONG *) &ifa->ifa_addr.sa_data[2]) =
              ((sp->s2_TempAddress & sp->s2_BitMask) |
               (*((ULONG *) &ifa->ifa_addr.sa_data[2]) & (ULONG) (-(sp->s2_BitMask+1))));

            ((struct arpcom *)ifp)->ac_ipaddr.s_addr = *((ULONG *) &ifa->ifa_addr.sa_data[2]);
            }
        else
            ((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;

        if (!(sp->s2_Flags & IFS2_BOOTED))
           {
           for (x=0;x<NUMREADS;x++)
              queueread(sp,AF_INET);

           if (sp->s2_UseArp) queueread(sp,AF_UNSPEC);    /* issue an arp read req if needed */
           sp->s2_Flags |= IFS2_BOOTED;
           }


        break;
        }
    case SIOCSIFFLAGS:
        {
//      ifp->if_flags &= ~IFF_RUNNING;
        break;
        }
    }

 return(0);
}

/* These callbacks are set up with no geta4() or __saveds because inet.library is being
 * built by SAS's "voodoo libraries" - which requires you to use SAS's -l option, which
 * in turn references __LinkerDB - the base for the global data area - off of the
 * device base, which is supposed to always be in A6.
 *
 * In this case, since the following code is being made from a device driver, the 'correct'
 * value of A6 may or may not be present.  Therefore, __saveds is a bad thing because it
 * tries to reference something off of a garbaged register.  The bottom line?
 * No references to globals are allowed in the two callback routines.
 */

#undef SysBase
#define SysBase (*(void **)4L)

BOOL CTB(add,n,to)
char *add;
ULONG n;
struct mbuf *to;
{
int x;
struct mbuf *m;
char *ptr;

/* First, find out if the supplied mbuf is large enough to hold n bytes - and
 * if not, either enlarge the mbuf by adding more segments, or return, with
 * a failed returncode.
 */


 x = 0;
 m = to;
 while (m)
    {
    x += mbufsize;
    m = m->m_next;
    }

 if (x < n) return (FALSE);

/* copy the data over */
 m = to;
 while (n)
    {
    m->m_len = MIN(mbufsize,n);
    ptr = mtod(m,char *);
    CopyMem(add,ptr,m->m_len);
    n -= m->m_len;
    add = (char *) (add + m->m_len);
    m = m->m_next;
    }

 return(TRUE);
}

BOOL CFB(addr,n,from)
char *addr;
ULONG n;
struct mbuf *from;
{

 struct mbuf *m;
 char *ptr;

 m = from;

 while (m)
    {
    ptr = mtod(m,char *);
    CopyMem(ptr,addr,m->m_len);
    addr = (char *) (addr + m->m_len);
    m = m->m_next;
    }

 return(TRUE);
}


#undef SysBase

