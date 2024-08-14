/**************************************************************************
**
** arp.c        - ARP Protocol routines for nipc.library
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: arp.c,v 1.18 93/07/30 18:41:41 gregm Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

/*------------------------------------------------------------------------*/

BOOL InitARP()
{
    register struct NBase *nb = gb;
    NewList((struct List *) & gb->ARPList);
    NewList((struct List *) & gb->ARPBuffer);
    InitSemaphore(&nb->ARPBufferSemaphore);
    return(TRUE);
}

/*------------------------------------------------------------------------*/

void DeinitARP()
{
    register struct NBase *nb = gb;
    struct arpentry *ae;
    struct arpbuff *ab;

    while(ae = (struct arpentry *)RemHead((struct List *) &nb->ARPList))
    {
        FreeMem(ae, sizeof(struct arpentry));
    }

//    ObtainSemaphore(&nb->ARPBufferSemaphore);
    while (ab = (struct arpbuff *) RemHead((struct List *) & nb->ARPBuffer))
    {
        FreeBuffer(ab->ab_Buffer);
        FreeMem(ab, sizeof(struct arpbuff));
    }
//    ReleaseSemaphore(&nb->ARPBufferSemaphore);
}

/*------------------------------------------------------------------------*/

void SendARPRequest(dev, ip_addr)
ULONG ip_addr;
struct sanadev *dev;
{

    struct arppkt *tmparp;
    struct Buffer *bp;
    struct BuffEntry *be;

    if(bp = AllocBuffer(sizeof(struct arppkt)))
    {
        be = (struct BuffEntry *) (bp->buff_list.mlh_Head);
        be->be_length = sizeof(struct arppkt);
        tmparp = (struct arppkt *) (be->be_data);

        tmparp->arp_HardwareType = dev->sanadev_HardwareType;
        tmparp->arp_ProtocolType = 0x800;
        tmparp->arp_HardAddrLen = 6;
        tmparp->arp_ProtoAddrLen = 4;
        memcpy(&(tmparp->arp_SenderEtherAddress), &(dev->sanadev_HardAddress), 6);
        memcpy(&(tmparp->arp_SenderIPAddress), &(dev->sanadev_IPAddress), 4);
        memcpy(&(tmparp->arp_TargetEtherAddress), "\x00\x00\x00\x00\x00\x00", 6);
        memcpy(&(tmparp->arp_TargetIPAddress), &ip_addr, 4);

        tmparp->arp_Operation = ARP_REQUEST;
        PacketWrite(dev, 0L, bp, 0L, PACKET_ARP, TRUE, FALSE);
    }
}

/*------------------------------------------------------------------------*/

void SendARPReply(dev, ip_to, ether_to, ip_us, ether_us)
ULONG ip_to;                    /* who this is to */
ULONG ip_us;                    /* our IP address */
struct sanadev *dev;            /* which device */
UBYTE *ether_to;                /* their ethernet address */
UBYTE *ether_us;                /* source ethernet address */
{

    struct arppkt *tmparp;
    struct Buffer *bp;
    struct BuffEntry *be;

    if(bp = AllocBuffer(sizeof(struct arppkt)))
    {
        be = (struct BuffEntry *) (bp->buff_list.mlh_Head);
        be->be_length = sizeof(struct arppkt);
        tmparp = (struct arppkt *) (be->be_data);

        tmparp->arp_HardwareType = dev->sanadev_HardwareType;
        tmparp->arp_ProtocolType = 0x800;
        tmparp->arp_HardAddrLen = 6;
        tmparp->arp_ProtoAddrLen = 4;
        memcpy(&(tmparp->arp_SenderEtherAddress), ether_us, 6);
        tmparp->arp_SenderIPAddress = ip_us;
        memcpy(&(tmparp->arp_TargetEtherAddress), ether_to, 6);
        tmparp->arp_TargetIPAddress = ip_to;

        tmparp->arp_Operation = ARP_REPLY;

        PacketWrite(dev, 0L, bp, (ULONG) ether_to, PACKET_ARP, FALSE, TRUE);
    }
}

/*------------------------------------------------------------------------*/

void ARPInput(arpbuff, length, dev)
struct Buffer *arpbuff;
ULONG length;
struct sanadev *dev;
{

    struct arppkt pkt;
    struct sanadev *devptr;

    CopyFromBuffer((UBYTE *) & pkt, arpbuff, sizeof(struct arppkt));
    FreeBuffer(arpbuff);

    switch (pkt.arp_Operation)
    {
        case ARP_REQUEST:
        {
            devptr = (struct sanadev *) gb->DeviceList.lh_Head;
            while (devptr->sanadev_Node.mln_Succ)
            {
                if (devptr->sanadev_IPAddress == pkt.arp_TargetIPAddress)
                    SendARPReply(dev, pkt.arp_SenderIPAddress,
                           (UBYTE *) & (pkt.arp_SenderEtherAddress),
                                 pkt.arp_TargetIPAddress,
                         (UBYTE *) & (devptr->sanadev_HardAddress));
                devptr = (struct sanadev *) devptr->sanadev_Node.mln_Succ;
            }
            break;
        }

        case ARP_REPLY:
        {
            int x;
            char *a, *b;
            a = (char *) &dev->sanadev_HardAddress[0];
            b =  (char *) &(pkt.arp_TargetEtherAddress);
            for (x = 0; x < 6; x++)
            {
                if (a[x] != b[x])
                    break;
            }
            if (x == 6)
                AddARPEntry(pkt.arp_SenderIPAddress, (UBYTE *) & (pkt.arp_SenderEtherAddress));
            break;
        }
    }
}

/*------------------------------------------------------------------------*/

void AddARPEntry(ip, ether)
ULONG ip;
UBYTE *ether;
{
    register struct NBase *nb = gb;
    struct arpentry *ae;
    int x;

    Forbid();
    x = 0;
    ae = (struct arpentry *) gb->ARPList.mlh_Head;
    while (ae->arpentry_link.mln_Succ)
    {
        x++;
        if (ae->arpentry_IP == ip)  /* prevent duplicate entries */
        {
            Permit();
            return;
        }
        ae = (struct arpentry *) ae->arpentry_link.mln_Succ;
    }

    if (x == TABLE_SIZE)
        ae = (struct arpentry *) RemTail((struct List *) & nb->ARPList);
    else
        ae = (struct arpentry *) AllocMem(sizeof(struct arpentry), MEMF_CLEAR + MEMF_PUBLIC);

    if(ae)
    {
        ae->arpentry_IP = ip;
        ae->arpentry_Timeout = 1200;  /* Two minutes */
        memcpy(&(ae->arpentry_ether), ether, 6);
        AddHead((struct List *) & nb->ARPList, (struct Node *) ae);
    }
    Permit();

    DoARPSends(ip);

}

/*------------------------------------------------------------------------*/

BOOL FindArpEntry(dev, IP, etherbuff)
struct sanadev *dev;
ULONG IP;
UBYTE *etherbuff;
{
    register struct NBase *nb = gb;
    struct arpentry *ae;

    Forbid();

    ae = (struct arpentry *) gb->ARPList.mlh_Head;
    while (ae->arpentry_link.mln_Succ)
    {
        if (ae->arpentry_IP == IP)
        {
            memcpy(etherbuff, &(ae->arpentry_ether), 6);
            Remove((struct Node *) ae); /* every time this entry gets used, */
            ae->arpentry_Timeout = 1200;
            AddHead((struct List *) & nb->ARPList, (struct Node *) ae);     /* move it to the head
                                                                         * of the list */
            Permit();
            return (TRUE);
        }

        ae = (struct arpentry *) ae->arpentry_link.mln_Succ;
    }

    Permit();
    SendARPRequest(dev, IP);
    return (FALSE);

}

/*------------------------------------------------------------------------*/

void DoARPSends(ipnum)
ULONG ipnum;
{
    register struct NBase *nb = gb;
    struct arpbuff *ab;

    ObtainSemaphore(&nb->ARPBufferSemaphore);

    ab = (struct arpbuff *) gb->ARPBuffer.mlh_Head;
    while (ab->ab_link.mln_Succ)
    {
        if (ab->ab_IPAddress == ipnum || ab->ab_gateway == ipnum)
        {
            struct arpbuff *at;
            at = ab;
            ab = (struct arpbuff *) ab->ab_link.mln_Pred;
            Remove((struct Node *) at);
            PacketWrite(at->ab_dev, at->ab_gateway, at->ab_Buffer, at->ab_IPAddress, at->ab_PacketType, FALSE, FALSE);
            at->ab_Buffer->buff_refcnt--;
            FreeMem(at, sizeof(struct arpbuff));
        }
        ab = (struct arpbuff *) ab->ab_link.mln_Succ;
    }

    ReleaseSemaphore(&nb->ARPBufferSemaphore);
}

/*------------------------------------------------------------------------*/

void AddBuffList(b, address, dev, gateway, ptype)
struct Buffer *b;
ULONG address;
struct sanadev *dev;
ULONG gateway;
UWORD ptype;
{
    register struct NBase *nb = gb;
    struct arpbuff *ab;

    if(ab = (struct arpbuff *) AllocMem(sizeof(struct arpbuff), MEMF_CLEAR))
    {
        b->buff_refcnt++;
        ab->ab_Buffer = b;
        ab->ab_IPAddress = address;
        ab->ab_CountDown = BUFFERTIME;
        ab->ab_dev = dev;
        ab->ab_gateway = gateway;
        ab->ab_PacketType = ptype;

        ObtainSemaphore(&nb->ARPBufferSemaphore);
        AddTail((struct List *) & nb->ARPBuffer, (struct Node *) ab);
        ReleaseSemaphore(&nb->ARPBufferSemaphore);
        return;
    }
    FreeBuffer(b);
}

/*------------------------------------------------------------------------*/

/* arp_timeout() times out any old arp-buffered packets */
void arp_timeout()
{
    register struct NBase *nb = gb;
    struct arpbuff *ab;
    struct arpentry *ae;
    struct arpentry *tmp_ae;

    ObtainSemaphore(&nb->ARPBufferSemaphore);

    ab = (struct arpbuff *) gb->ARPBuffer.mlh_Head;
    while (ab->ab_link.mln_Succ)
    {
        ab->ab_CountDown--;
        if (!ab->ab_CountDown)
        {
            struct arpbuff *at;
            at = ab;
            ab = (struct arpbuff *) ab->ab_link.mln_Pred;
            Remove((struct Node *) at);
            at->ab_Buffer->buff_refcnt--;
            FreeBuffer(at->ab_Buffer);
            FreeMem(at, sizeof(struct arpbuff));
        }
        ab = (struct arpbuff *) ab->ab_link.mln_Succ;
    }
    ReleaseSemaphore(&nb->ARPBufferSemaphore);

    Forbid();
    ae = (struct arpentry *)gb->ARPList.mlh_Head;
    while(ae->arpentry_link.mln_Succ)
    {
        ae->arpentry_Timeout--;
        if(!ae->arpentry_Timeout)
        {
            tmp_ae = ae;
            ae = (struct arpentry *) ae->arpentry_link.mln_Pred;
            Remove((struct Node *)tmp_ae);
            FreeMem(tmp_ae, sizeof(struct arpentry));
        }
        ae = (struct arpentry *) ae->arpentry_link.mln_Succ;
    }
    Permit();
}
/*------------------------------------------------------------------------*/
