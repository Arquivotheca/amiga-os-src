/**************************************************************************
**
** s2io.c       - SANA2 IO Routines
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: s2io.c,v 1.43 93/07/30 18:39:34 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

/* The following line forces NIPC to switch packets to AS225 based on
 * IP address;  not defining this allows it to switch them on packet type
 * instead.
 */
//#define SWITCHONADDRESS         1

#include "nipcinternal.h"

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <devices/sana2.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>

extern ULONG far FastRand(ULONG seed);

#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include "externs.h"

extern APTR far ACTB;
extern APTR far ACFB;

static BOOL SwitchPacket(struct IOSana2ReqSS *ioreq);
static void ProcessReq(struct IOSana2ReqSS *ioreq);

/*------------------------------------------------------------------------*/

/* worthwhile constants */
#define TOTALQUEUE 25           /* Number of IP read requests that I keep
                                 * running at any given time */

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define MAX(x, y) ((x) > (y) ? (x):(y))

/*------------------------------------------------------------------------*/

BOOL __stdargs(*ExCTB) (UBYTE * from, ULONG length, APTR to);


/* superstructure */
struct IOSana2ReqSS
{
    struct IOSana2Req ss;
    struct sanadev *ios2_DevPtr;
};

/*------------------------------------------------------------------------*/

/* Get things rolling */

BOOL InitSana2()
{
    register struct NBase *nb = gb;

    struct sanadev *s2dev;
    struct CollectionItem *ci;
    struct NIPCDevicePrefs *ndp;

    BOOL override;
    ULONG DevIP, IPMask;

    struct IOSana2Req ioreq;
    struct Sana2DeviceQuery s2dq;

    /* Init some globals */
    gb->notags[0]=TAG_DONE;
    gb->copytags[0]=S2_CopyToBuff;
    gb->copytags[1]=(LONG) &ACTB;
    gb->copytags[2]=S2_CopyFromBuff;
    gb->copytags[3]=(LONG) &ACFB;
    gb->copytags[4]=TAG_DONE;

    gb->Switching = FALSE;

    /* make sure our device list is usable */
    NewList(&gb->DeviceList);
    NewList((struct List *) & gb->ExchangeList);

    /* Create the reply port for all Sana II device requests */
    if(nb->replyport = (struct MsgPort *) CreateMsgPort())
    {
        if(nb->localport = (struct MsgPort *) CreateMsgPort())
        {
            if(nb->CMDPort = (struct MsgPort *) CreateMsgPort())
            {
                if(nb->DataPort = (struct MsgPort *) CreateMsgPort())
                {
                    gb->s2sigmask = (1 << gb->replyport->mp_SigBit) | (1 << gb->localport->mp_SigBit) |
                                    (1 << (gb->CMDPort->mp_SigBit)) | (1 << (gb->DataPort->mp_SigBit));

                    if(s2dev = (struct sanadev *) AllocMem(sizeof(struct sanadev), MEMF_PUBLIC | MEMF_CLEAR))
                    {
                        s2dev->sanadev_IPAddress = 0x7f000001;
                        s2dev->sanadev_IPNetwork = 0x7f000000;
                        s2dev->sanadev_IPMask = 0xff000000;
                        s2dev->sanadev_MTU = 0xffffffff;
                        s2dev->sanadev_BPS = 0xffffffff;
                        NewList((struct List *) & (s2dev->sanadev_Fragments));
                        AddTail((struct List *) & nb->DeviceList, (struct Node *) s2dev);
                    }
                    ci = FindCollection(nb->iff,ID_PREF,ID_NDEV);

                    while (ci)
                    {
                        ndp = (struct NIPCDevicePrefs *)ci->ci_Data;

                        if(ndp->ndp_Flags & NDPFLAGF_ONLINE)
                        {
                            override = (ndp->ndp_Flags & NDPFLAGF_HARDADDR);

                            ioreq.ios2_BufferManagement = &gb->copytags;
                            ioreq.ios2_Req.io_Message.mn_ReplyPort = gb->replyport;
                            if (!OpenDevice(ndp->ndp_DevPathName, ndp->ndp_Unit, (struct IORequest *) & ioreq, 0L))
                            {
                                s2dev = (struct sanadev *) AllocMem(sizeof(struct sanadev), MEMF_PUBLIC + MEMF_CLEAR);
                                if (!s2dev)
                                    continue;
                                ioreq.ios2_Req.io_Command = S2_DEVICEQUERY;
                                ioreq.ios2_StatData = (void *) &s2dq;
                                s2dq.SizeAvailable = sizeof(struct Sana2DeviceQuery);
                                DoIO((struct IORequest *) & ioreq);
                                s2dev->sanadev_IPType = ndp->ndp_IPType;
                                s2dev->sanadev_ARPType = ndp->ndp_ARPType;
                                s2dev->sanadev_MTU = s2dq.MTU;
                                s2dev->sanadev_BPS = s2dq.BPS;
                                s2dev->sanadev_HardwareType = s2dq.HardwareType;
                                if (s2dq.HardwareType == 1)  /* If ethernet */
                                    s2dev->sanadev_Flags |= S2DF_USEARP;
                                s2dev->sanadev_AddressSize = s2dq.AddrFieldSize;
                                s2dev->sanadev_Device = ioreq.ios2_Req.io_Device;
                                s2dev->sanadev_Unit = ioreq.ios2_Req.io_Unit;
                                s2dev->sanadev_BuffMan = ioreq.ios2_BufferManagement;
                                s2dev->sanadev_IPAddress = DevIP = ndp->ndp_IPAddress;
                                s2dev->sanadev_IPNetwork = NetNum(s2dev->sanadev_IPAddress);
                                s2dev->sanadev_MaxIPReq = TOTALQUEUE;
                                s2dev->sanadev_MaxARPReq = 5;
                                s2dev->sanadev_NumIPReq = 0;
                                s2dev->sanadev_NumARPReq = 0;

                                if (IP_CLASSA(DevIP))
                                    IPMask = 0xff000000;
                                if (IP_CLASSB(DevIP))
                                    IPMask = 0xffff0000;
                                if (IP_CLASSC(DevIP))
                                    IPMask = 0xffffff00;
                                if (IP_CLASSD(DevIP))
                                    IPMask = 0xffffffff;
                                if (IP_CLASSE(DevIP))
                                    IPMask = 0xffffffff;

                                s2dev->sanadev_IPMask = IPMask;

                                if (ndp->ndp_Flags & NDPFLAGF_SUBNET)
                                {
                                    s2dev->sanadev_SubnetMask = ndp->ndp_IPSubnet;
                                    s2dev->sanadev_IPSubnet = s2dev->sanadev_SubnetMask & s2dev->sanadev_IPAddress;
                                    s2dev->sanadev_SValid = TRUE;
                                }
                                else
                                    s2dev->sanadev_SValid = FALSE;

                                if (!override)
                                {
                                    ioreq.ios2_Req.io_Command = S2_GETSTATIONADDRESS;
                                    DoIO((struct IORequest *) & ioreq);
                                    memcpy(&(ioreq.ios2_SrcAddr), &(ioreq.ios2_DstAddr), 16);
                                }
                                else
                                {
                                    memcpy(&(ioreq.ios2_SrcAddr), &ndp->ndp_HardAddress, (s2dev->sanadev_AddressSize) >> 3);
                                }

                                ioreq.ios2_Req.io_Command = S2_CONFIGINTERFACE;
                                DoIO((struct IORequest *) & ioreq);

                                if (ioreq.ios2_Req.io_Error)
                                {
                                    ioreq.ios2_Req.io_Command = S2_GETSTATIONADDRESS;
                                    DoIO((struct IORequest *) & ioreq);
                                }

                                NewList((struct List *) & (s2dev->sanadev_Fragments));

                                memcpy(&(s2dev->sanadev_HardAddress), &(ioreq.ios2_SrcAddr), (s2dev->sanadev_AddressSize) >> 3);

                                AddTail((struct List *) & nb->DeviceList, (struct Node *) s2dev);

                                QueueRead(s2dev, PACKET_IP);

                                if (s2dev->sanadev_Flags & S2DF_USEARP)
                                    QueueRead(s2dev, PACKET_ARP);

                            }
                            else
                            {
                                struct Library *IntuitionBase;
                                struct EasyStruct es;
                                APTR ERArgs;
                                if(IntuitionBase = OpenLibrary("intuition.library",37L))
                                {
                                    ERArgs=(APTR)ndp->ndp_DevPathName;
                                    es.es_StructSize = sizeof(struct EasyStruct);
                                    es.es_Flags = NULL;
                                    es.es_Title = "NIPC";
                                    es.es_TextFormat = "Could not open %s.";
                                    es.es_GadgetFormat = "Ok";
                                    EasyRequestArgs(NULL,&es,NULL,&ERArgs);
                                    CloseLibrary(IntuitionBase);
                                }
                            }
                        }
                        ci = ci->ci_Next;
                    }
                    nb->CMDPort->mp_Node.ln_Name = "NIPC Exchange";
                    AddPort(nb->CMDPort);
                    CheckForInet();
                    return(TRUE);
                }
                DeleteMsgPort(nb->CMDPort);
            }
            DeleteMsgPort(nb->localport);
        }
        DeleteMsgPort(nb->replyport);
    }
    return(FALSE);
}

/*------------------------------------------------------------------------*/

void DeinitSana2()
{
    register struct NBase *nb = gb;
    struct sanadev *deldev;
    struct IOSana2Req *ioretreq;
    struct IOSana2Req ioreq;
    while (deldev = (struct sanadev *) RemHead(&nb->DeviceList))
    {
        if(deldev->sanadev_Device)
        {
            deldev->sanadev_Flags |= S2DF_OFFLINE;  /* prevent any further
                                                     * CMD_WRITE's */
            ioreq.ios2_Req.io_Unit = deldev->sanadev_Unit;
            ioreq.ios2_Req.io_Device = deldev->sanadev_Device;
            ioreq.ios2_BufferManagement = deldev->sanadev_BuffMan;
            ioreq.ios2_Req.io_Message.mn_ReplyPort = gb->replyport;
            ioreq.ios2_Req.io_Command = CMD_FLUSH;
            DoIO((struct IORequest *) & ioreq);
            while (ioretreq = (struct IOSana2Req *) GetMsg(nb->replyport))
            {
                if (ioretreq->ios2_Req.io_Command == CMD_READ)
                    FreeBuffer(ioretreq->ios2_Data);
                EDeleteIORequest( (struct IORequest *) ioretreq);
            }
            CloseDevice((struct IORequest *) & ioreq);
        }
        FreeMem(deldev, sizeof(struct sanadev));
    }

    DeleteMsgPort(nb->replyport);
    DeleteMsgPort(nb->localport);
    /* Must notify callers that this port is no longer "usable" */
    NotifyInet();
    /* FIXME */ /* What?  FIXME what??  I -hate- it when I do this! */
    DeleteMsgPort(nb->DataPort);
    RemPort(nb->CMDPort);
    DeleteMsgPort(nb->CMDPort);

}

/*------------------------------------------------------------------------*/

void CheckCmdPort()
{
    struct xstart *rd;
    register struct NBase *nb = gb;

    while (rd = (struct xstart *) GetMsg(nb->CMDPort))
    {
        if (rd->xs_Command == XCMD_START)
        {
            struct sanadev *td;

            /* Make sure that we're using that device & Unit */
            td = (struct sanadev *) gb->DeviceList.lh_Head;
            while (td->sanadev_Node.mln_Succ)
            {
                if ((td->sanadev_Device == rd->xs_Device) && (td->sanadev_Unit == rd->xs_Unit))
                {
                    gb->Switching = TRUE;
                    ExCTB = rd->xs_CTB;
                    break;
                }
                td = (struct sanadev *) td->sanadev_Node.mln_Succ;
            }
            if (td->sanadev_Node.mln_Succ)  /* If a match was found */
            {
                rd->xs_Link = gb->DataPort;
            }
            else
            {
                rd->xs_Link = 0L;
            }
        }
        if (rd->xs_Command == XCMD_END)
        {
            struct IORequest *xm;
            gb->Switching = FALSE;
            while (xm = (struct IORequest *) RemHead((struct List *) & nb->ExchangeList))
            {
                xm->io_Error = S2ERR_OUTOFSERVICE;
                ReplyMsg(&xm->io_Message);
            }
        }
        ReplyMsg(&rd->xs_Msg);
    }
}

/*------------------------------------------------------------------------*/

void CheckDataPort()
{
    struct IORequest *ior;
    register struct NBase *nb = gb;

    while (ior = (struct IORequest *) GetMsg(nb->DataPort))
    {
        Forbid();
        AddTail((struct List *) & nb->ExchangeList, (struct Node *) ior);
        Permit();
    }

}

/*------------------------------------------------------------------------*/

static BOOL SwitchPacket(ioreq)
struct IOSana2ReqSS *ioreq;
{
    struct Buffer *sb;
    struct iphdr *iph;
    struct BuffEntry *be;
#ifdef SWITCHONADDRESS
    struct sanadev *dv;
#endif
    int x;
    struct IOSana2ReqSS *xtreq;

    if (!gb->Switching)
    {
        return (FALSE);         /* If we're not switching, don't BOTHER .. */
    }

    /*
     * Okay, the crap begins here.  We need to actually find our way into the
     * buffer for this thing, find the IP header, and find the destination
     * address.  Sheesh!
     */

    sb = (struct Buffer *) ioreq->ss.ios2_Data;
    iph = (struct iphdr *) BuffPointer(sb, 0, &be);


#ifdef SWITCHONADDRESS
    /* Okay, check the IP address vs. those that we have in our devices ... */
    if (ioreq->ss.ios2_PacketType == ioreq->ios2_DevPtr->sanadev_IPType)
    {
        if (IsListEmpty((struct List *) & gb->DeviceList))
            return (FALSE);

        dv = (struct sanadev *) gb->DeviceList.lh_Head;
        while (dv->sanadev_Node.mln_Succ)
        {
            if (dv->sanadev_IPAddress == iph->iph_DestAddr)     /* This packet is for us */
                return (FALSE);                                 /* return (FALSE)        */
                                                                /* ..                    */
            dv = (struct sanadev *) dv->sanadev_Node.mln_Succ;
        }
    }
#else
    if (iph->iph_Protocol == 26) /* RDP is NIPC's */
        return(FALSE);
    if (iph->iph_Protocol == 17) /* UDP could be NIPC's .. Let's check the port */
    {
        struct udphdr *uh;
        uh = (struct udphdr *) ( ((ULONG) iph) + (iph->iph_IHL<<2) );

        if ( (uh->udp_DestPort == NIP_UDP_PORT) && (!iph->iph_FragmentOffset) )
        {
            return(FALSE);
        }
    }
#endif

    /* Not for us ... */
    /* Find an ioreq for this device and unit */
    Forbid();
    if (!IsListEmpty((struct List *) & gb->ExchangeList))
    {
        xtreq = (struct IOSana2ReqSS *) gb->ExchangeList.mlh_Head;
        while (xtreq->ss.ios2_Req.io_Message.mn_Node.ln_Succ)
        {
            if ((xtreq->ss.ios2_Req.io_Device == ioreq->ss.ios2_Req.io_Device) &&
                (xtreq->ss.ios2_Req.io_Unit == ioreq->ss.ios2_Req.io_Unit) &&
                (xtreq->ss.ios2_PacketType == ioreq->ss.ios2_PacketType))
                break;
            xtreq = (struct IOSana2ReqSS *) xtreq->ss.ios2_Req.io_Message.mn_Node.ln_Succ;
        }
        Permit();
        if (xtreq->ss.ios2_Req.io_Message.mn_Node.ln_Succ)
        {
            Remove((struct Node *) xtreq);
            xtreq->ss.ios2_DataLength = ioreq->ss.ios2_DataLength;
            xtreq->ss.ios2_Req.io_Error = ioreq->ss.ios2_Req.io_Error;
            xtreq->ss.ios2_WireError = ioreq->ss.ios2_WireError;
            xtreq->ss.ios2_PacketType = ioreq->ss.ios2_PacketType;
            (*ExCTB) ((UBYTE *) iph, (ULONG) ioreq->ss.ios2_DataLength, (void *) xtreq->ss.ios2_Data);   /* copy the data over */

            /* Copy the addresses over .. */
            for (x = 0; x < 16; x++)
            {
                xtreq->ss.ios2_SrcAddr[x] = ioreq->ss.ios2_SrcAddr[x];
                xtreq->ss.ios2_DstAddr[x] = ioreq->ss.ios2_DstAddr[x];
            }

            ReplyMsg(&xtreq->ss.ios2_Req.io_Message);
        }
    }
    else
        Permit();

    /* Clean up after ourselves */
    if (ioreq->ss.ios2_PacketType == ioreq->ios2_DevPtr->sanadev_IPType)
    {
        if(!IsBroadcast(iph->iph_DestAddr))     /* We need IP broadcast packets too! */
        {
            FreeBuffer(sb);
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    else
        return (FALSE);         /* We need a copy of the arp packets too! */
}

/*------------------------------------------------------------------------*/

void PacketWrite(dev, gateway, buffer, destination, packettype, brc, ether)
struct sanadev *dev;
ULONG gateway;
struct Buffer *buffer;
ULONG destination;
UWORD packettype;
BOOL brc;
BOOL ether;
{
    register struct NBase *nb = gb;
    ULONG dest;
    struct IOSana2ReqSS *wioreq;
    struct LocalIOReq *localreq;
    UBYTE eaddr[16];
//    register ULONG seed;

/* Randomly toss packets */
//    seed = nb->RandomSeed;
//    seed = FastRand(seed);
//    nb->RandomSeed = seed;
//    if ((seed & 0xFF) < 32)
//    {
//        kprintf("Dumping buffer\n");
//        buffer->buff_refcnt++;
//        FreeBuffer(buffer);
//    }


//    kprintf("WRITTEN PACKET\n");

    gb->outbuffs++;

    if(dev == (struct sanadev *)gb->DeviceList.lh_Head)
    {
        if(localreq = (struct LocalIOReq *) AllocMem(sizeof(struct LocalIOReq),MEMF_PUBLIC))
        {
            localreq->lreq_Buffer = buffer;
            localreq->lreq_Device = dev;
            buffer->buff_refcnt++;
            PutMsg(nb->localport,(struct Message *)localreq);
        }
        else
            FreeBuffer(buffer);

        return;
    }
    if (ether)
    {
        char *ex = (char *) destination;
        CopyMem(ex,&eaddr[0],6);
    }

    if ((!brc) && (!ether))
    {
        if (gateway == dev->sanadev_IPAddress)
            dest = destination;
        else
            dest = gateway;

        if(dev->sanadev_Flags & S2DF_USEARP)
        {
            if (!FindArpEntry(dev, dest, (UBYTE *) & eaddr))
            {
                AddBuffList(buffer, destination, dev, gateway, packettype);
                return;
            }
        }
        else
        {
            ULONG lmask,portion;
            int n;
            n = dev->sanadev_AddressSize;
            lmask = (1 << n) - 1;
            portion = dest & lmask;
            portion = (portion << (32-n));
            memcpy(&eaddr,&portion,4);
        }
    }

    if(wioreq = (struct IOSana2ReqSS *) ECreateIORequest(nb->replyport, sizeof(struct IOSana2ReqSS)))
    {
        wioreq->ss.ios2_Req.io_Device = dev->sanadev_Device;
        wioreq->ss.ios2_Req.io_Unit = dev->sanadev_Unit;
        wioreq->ss.ios2_BufferManagement = dev->sanadev_BuffMan;
        wioreq->ss.ios2_Data = (void *) buffer;
        wioreq->ss.ios2_DataLength = DataLength(buffer);
        wioreq->ios2_DevPtr = dev;
        if(!brc)
            wioreq->ss.ios2_Req.io_Command = CMD_WRITE;
        else
            wioreq->ss.ios2_Req.io_Command = S2_BROADCAST;

        switch (packettype)
        {
            case PACKET_IP:
                wioreq->ss.ios2_PacketType = dev->sanadev_IPType;
                break;
            case PACKET_ARP:
                wioreq->ss.ios2_PacketType = dev->sanadev_ARPType;
                break;
            default:
                break;
        }

        /* Our version of ARP is hardcoded for ETHERNET -- thus the "6" */
        if(!brc)
            memcpy(&(wioreq->ss.ios2_DstAddr), &eaddr, 6);

        buffer->buff_refcnt++;

        SendIO((struct IORequest *) wioreq);
    }
    else
        FreeBuffer(buffer);
}

/*------------------------------------------------------------------------*/

#ifdef PACKET_BRC

void PacketBroadcast(dev, buffer, packettype)
struct sanadev *dev;
struct Buffer *buffer;
UWORD packettype;
{
    register struct NBase *nb = gb;
    struct IOSana2ReqSS *wioreq;

//    kprintf("BROADCAST PACKET\n");

    if(wioreq = (struct IOSana2ReqSS *) ECreateIORequest(nb->replyport, sizeof(struct IOSana2ReqSS)))
    {
        wioreq->ss.ios2_Req.io_Device = dev->sanadev_Device;
        wioreq->ss.ios2_Req.io_Unit = dev->sanadev_Unit;
        wioreq->ss.ios2_BufferManagement = dev->sanadev_BuffMan;

        wioreq->ss.ios2_Data = (void *) buffer;
        wioreq->ss.ios2_DataLength = DataLength(buffer);
        wioreq->ios2_DevPtr = dev;
        wioreq->ss.ios2_Req.io_Command = S2_BROADCAST;
        switch (packettype)
        {
            case PACKET_IP:
                wioreq->ss.ios2_PacketType = dev->sanadev_IPType;
                break;
            case PACKET_ARP:
                wioreq->ss.ios2_PacketType = dev->sanadev_ARPType;
                break;
            default:
                break;
        }

        buffer->buff_refcnt++;

        SendIO((struct IORequest *) wioreq);
    }

}

#endif
/*------------------------------------------------------------------------*/

void PollPort()
{
    struct IOSana2ReqSS *ioreq;
    register struct NBase *nb = gb;
    struct LocalIOReq *localreq;
    CheckCmdPort();
    CheckDataPort();
    while (ioreq = (struct IOSana2ReqSS *) GetMsg(nb->replyport))
    {
        ProcessReq(ioreq);
    }
    while (localreq = (struct LocalIOReq *) GetMsg(nb->localport))
    {
    localreq->lreq_Buffer->buff_refcnt--;
    ip_in(localreq->lreq_Buffer, localreq->lreq_Device);

    FreeMem(localreq,sizeof(struct LocalIOReq));
    }
}

/*------------------------------------------------------------------------*/

void QueueRead(dev, pt)
struct sanadev *dev;
UWORD pt;
{
    register struct NBase *nb = gb;
    struct IOSana2ReqSS *ioreq;
    struct Buffer *tmpbuff;
    UWORD queue;
    ONTIMER(12);
    if(pt == PACKET_IP)
    {
        queue = dev->sanadev_MaxIPReq - dev->sanadev_NumIPReq;
    }
    else
    {
        queue = dev->sanadev_MaxARPReq - dev->sanadev_NumARPReq;
    }

    while(queue--)
    {
        tmpbuff = AllocBuffer(dev->sanadev_MTU);
        if(!tmpbuff)
            return;

        ioreq = (struct IOSana2ReqSS *) ECreateIORequest(nb->replyport, sizeof(struct IOSana2ReqSS));
        if (!ioreq)
        {
            OFFTIMER(12);
            return;
        }

        ioreq->ss.ios2_Req.io_Device = dev->sanadev_Device;
        ioreq->ss.ios2_Req.io_Unit = dev->sanadev_Unit;
        ioreq->ss.ios2_BufferManagement = dev->sanadev_BuffMan;
        ioreq->ss.ios2_Req.io_Command = CMD_READ;
        ioreq->ss.ios2_Data = (void *) tmpbuff;

//        kprintf("READ PACKET\n");

        switch (pt)
        {
            case PACKET_IP:
                dev->sanadev_NumIPReq++;
                ioreq->ss.ios2_PacketType = dev->sanadev_IPType;
                break;
            case PACKET_ARP:
                dev->sanadev_NumARPReq++;
                ioreq->ss.ios2_PacketType = dev->sanadev_ARPType;
                break;
            default:
                break;
        }

        ioreq->ios2_DevPtr = dev;
        SendIO((struct IORequest *) ioreq);
    }
    OFFTIMER(12);
}

/*------------------------------------------------------------------------*/

static void ProcessReq(ioreq)
struct IOSana2ReqSS *ioreq;
{

    UWORD packt;
    struct sanadev *queuedev=NULL;
    struct Buffer *buff;
    struct BuffEntry *be;
    struct iphdr *hdr;
    ONTIMER(8);
    if (ioreq->ss.ios2_Req.io_Command == CMD_READ)
    {
        if (!ioreq->ss.ios2_Req.io_Error)       /* If a read w/ no error,
                                                 * feed the packet to the
                                                 * next level up */
        {
/*            if (ioreq->ios2_DevPtr->sanadev_Flags & S2DF_USEARP)
            {
                buff = (struct Buffer *) ioreq->ss.ios2_Data;
                be = (struct BuffEntry *) buff->buff_list.mlh_Head;
                hdr = (struct iphdr *) be->be_data;
                if (ioreq->ss.ios2_PacketType == ioreq->ios2_DevPtr->sanadev_ARPType)
                {
                    struct arppkt *a=(struct arppkt *) hdr;
                    kprintf("Arp op=%ld",a->arp_Operation);
                    if (a->arp_Operation == ARP_REPLY)
                        AddARPEntry(hdr->iph_SrcAddr, (UBYTE *) & (ioreq->ss.ios2_SrcAddr));
                }
            }
*/
            packt = PACKET_IP;
            if (ioreq->ss.ios2_PacketType != ioreq->ios2_DevPtr->sanadev_IPType)
            {
                ioreq->ios2_DevPtr->sanadev_NumARPReq--;
                packt = PACKET_ARP;
            }
            else
            {
                ioreq->ios2_DevPtr->sanadev_NumIPReq--;
            }
            if (!SwitchPacket(ioreq))
            {
                ProtocolInput(ioreq->ss.ios2_Data, ioreq->ss.ios2_DataLength,
                              packt, ioreq->ios2_DevPtr);
            }
        }
        else
        {
            if (ioreq->ss.ios2_PacketType != ioreq->ios2_DevPtr->sanadev_IPType)
            {
                ioreq->ios2_DevPtr->sanadev_NumARPReq--;
                packt=PACKET_ARP;
            }
            else
            {
                ioreq->ios2_DevPtr->sanadev_NumIPReq--;
                packt=PACKET_IP;
            }
            FreeBuffer((struct Buffer *) ioreq->ss.ios2_Data);
            if (ioreq->ss.ios2_Req.io_Error == S2ERR_OUTOFSERVICE)      /* If we're told "out of
                                                                         * service", queue a */
            {                   /* S2_ONEVENT, an wait for it to go into
                                 * service. */
                if (!(ioreq->ios2_DevPtr->sanadev_Flags & S2DF_WAITING))
                {
                    ioreq->ios2_DevPtr->sanadev_Flags |= (S2DF_WAITING | S2DF_OFFLINE);
                    ioreq->ss.ios2_Req.io_Command = S2_ONEVENT;
                    ioreq->ss.ios2_WireError = S2EVENT_ONLINE;
                    SendIO((struct IORequest *) ioreq);
                }
            }
        }
        if (ioreq->ss.ios2_Req.io_Command == CMD_READ)  /* Don't do this for the
                                                         * one that changed to
                                                         * an ONEVENT */
        {
            packt = PACKET_IP;
            if (ioreq->ss.ios2_PacketType != ioreq->ios2_DevPtr->sanadev_IPType)
                packt = PACKET_ARP;
            if (!(ioreq->ios2_DevPtr->sanadev_Flags & S2DF_OFFLINE))
            {
                queuedev=ioreq->ios2_DevPtr;
//                QueueRead(ioreq->ios2_DevPtr, packt);
            }
            EDeleteIORequest((struct IORequest *) ioreq);
        }
    }

    else if (ioreq->ss.ios2_Req.io_Command == S2_ONEVENT)
    {
        ioreq->ios2_DevPtr->sanadev_Flags &= ~(S2DF_WAITING | S2DF_OFFLINE);
        QueueRead(ioreq->ios2_DevPtr, PACKET_IP);
        if (ioreq->ios2_DevPtr->sanadev_Flags & S2DF_USEARP)
            queuedev=ioreq->ios2_DevPtr;
//            QueueRead(ioreq->ios2_DevPtr, PACKET_ARP);
        EDeleteIORequest((struct IORequest *) ioreq);
    }


    else if ((ioreq->ss.ios2_Req.io_Command == CMD_WRITE) || (ioreq->ss.ios2_Req.io_Command == S2_BROADCAST))
    {
        gb->outbuffs--;
        ((struct Buffer *)ioreq->ss.ios2_Data)->buff_refcnt--;   /* The device no longer needs this buffer. */
        FreeBuffer(ioreq->ss.ios2_Data);                         /* Free it if nobody wants it anymore. */
        EDeleteIORequest((struct IORequest *) ioreq);
    }
    if(queuedev)
        QueueRead(queuedev, packt);

    OFFTIMER(8);

}

/*------------------------------------------------------------------------*/

/* The two pieces of callback material */
BOOL __stdargs CTB(from, n, to)
struct Buffer *to;
UBYTE *from;
ULONG n;
{
    return (CopyToBuffer(to, from, n));
}

/*------------------------------------------------------------------------*/

BOOL __stdargs CFB(to, n, from)
struct Buffer *from;
ULONG n;
UBYTE *to;
{
    CopyFromBuffer(to, from, n);
    return TRUE;
}

/*------------------------------------------------------------------------*/

void CheckForInet()
{
    struct MsgPort *inetport;
    struct MsgPort *tmpreply;
    struct xstart xs;

    /* Try to send an xstart for each device we have */

    if(tmpreply = CreateMsgPort())
    {
        if (!IsListEmpty((struct List *) & gb->DeviceList))
        {
            struct sanadev *sd;
            sd = (struct sanadev *) gb->DeviceList.lh_Head;
            while (sd->sanadev_Node.mln_Succ)
            {
                xs.xs_Command = XCMD_START;
                xs.xs_Device = sd->sanadev_Device;
                xs.xs_Unit = sd->sanadev_Unit;
                xs.xs_Link = gb->DataPort;
                xs.xs_Msg.mn_ReplyPort = tmpreply;
                Forbid();
                inetport = FindPort("INET Exchange");
                if (inetport)
                {
                    PutMsg(inetport, (struct Message *) & xs);
                }
                Permit();
                if (inetport)
                {
                    WaitPort(tmpreply);
                    GetMsg(tmpreply);
                    if (xs.xs_Link == 0L)
                    {
                        ExCTB = xs.xs_CTB;
                        gb->Switching = TRUE;
                    }
                }
                sd = (struct sanadev *) sd->sanadev_Node.mln_Succ;
            }
        }
        DeleteMsgPort(tmpreply);
    }
}

/*------------------------------------------------------------------------*/

void NotifyInet()
{
    register struct NBase *nb = gb;
    struct MsgPort *tmpreply;
    struct MsgPort *inetport;
    struct xstart xs;
    struct IORequest *ior;

    /* First, flush their read requests */
    Forbid();
    while (ior = (struct IORequest *) RemHead((struct List *) & nb->ExchangeList))
    {
        ior->io_Error = -128;
        ReplyMsg(&ior->io_Message);
    }
    Permit();


    if(tmpreply = CreateMsgPort())
    {
        xs.xs_Command = XCMD_END;
        xs.xs_Msg.mn_ReplyPort = tmpreply;
        Forbid();
        inetport = FindPort("INET Exchange");
        if (inetport)
            PutMsg(inetport, &xs.xs_Msg);
        Permit();
        if (inetport)
        {
            WaitPort(tmpreply);
            GetMsg(tmpreply);
        }

        DeleteMsgPort(tmpreply);
    }

}

/*------------------------------------------------------------------------*/

void DevReqTimeout()
{
    struct sanadev *dev;

    dev = (struct sanadev *) gb->DeviceList.lh_Head;
    while(dev->sanadev_Node.mln_Succ)
    {
        QueueRead(dev,PACKET_IP);
        dev = (struct sanadev *) dev->sanadev_Node.mln_Succ;
    }
}

/*------------------------------------------------------------------------*/

struct IORequest *ECreateIORequest(struct MsgPort *rp,ULONG size)
{
    register struct NBase *nb=gb;
    struct IORequest *i;
    i = (struct IORequest *) EAllocPooled(nb->MemoryPool,size);
    if (i)
    {
        i->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
        i->io_Message.mn_ReplyPort = rp;
        i->io_Message.mn_Length = size;
        return(i);
    }
    return(0L);
}

VOID EDeleteIORequest(struct IORequest *i)
{
    register struct NBase *nb=gb;

    EFreePooled(nb->MemoryPool,i,i->io_Message.mn_Length);

}

