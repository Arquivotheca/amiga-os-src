/**************************************************************************
**
** ip_out.c     - IP Packet output and gateway functions
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: ip_out.c,v 1.41 93/08/10 16:16:50 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/
#include "nipcinternal.h"


#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include "externs.h"

/*------------------------------------------------------------------------*/

void ip_out(obuff, srcip, destip, protocol, iface, gateway, frags, morefrags, inident, ttl, dbc, fraglast)
struct Buffer *obuff;
UBYTE protocol;
struct sanadev *iface;
ULONG gateway;
ULONG srcip;
ULONG destip;
ULONG frags;
BOOL morefrags;
BOOL fraglast;
UWORD inident;
UBYTE ttl;
BOOL dbc;
{

    struct BuffEntry *be;
    struct iphdr *hdr;
    /* struct sanadev *dev; */
    ULONG len;

    ONTIMER(0);
    if(!ttl)
    {
        OFFTIMER(0);
        FreeBuffer(obuff);
        return;
    }

    len = DataLength(obuff);

    if ((len + sizeof(struct iphdr)) > iface->sanadev_MTU)
    {
        /*
         * Icky.  We have to fragment packets. Luckily, I can get away with
         * recursively calling ip_out. Fragmentation currently incurs data
         * copying.  This is unpleasant, but a necessity.  I may later change
         * the algorithm to be intelligent enough to avoid copying whenver
         * possible, and to build packets around already existant
         * BuffEntry's, whenever possible.
         */

        struct Buffer *fragbuff;
        ULONG x, y;

        y = ((iface->sanadev_MTU - sizeof(struct iphdr)) & 0xFFFFFFF8);
        for (x = 0; x < len; x += y)
        {
            if ((x + y) >= len)
                y = len - x;    /* for the last piece */
            if(fragbuff = (struct Buffer *) AllocBuffer(y))
            {
                CopyBuffer(obuff, x, y, fragbuff);
                ip_out(fragbuff, srcip, destip, protocol, iface, gateway, frags + (x / 8), ((x + y) < len), inident, ttl, dbc, fraglast);
                ONTIMER(2);
            }
        }
        FreeBuffer(obuff);
        OFFTIMER(2);
        return;
    }


    /*
     * First, allocate a BuffEntry with the necessary space in it to house
     * the IP header, and insert it at the Head of the Buffer.
     */

    if(be = AllocBuffEntry(sizeof(struct iphdr)))
    {
        AddHead((struct List *) & (obuff->buff_list), (struct Node *) be);
        hdr = (struct iphdr *) be->be_data;

        /* Now, start filling in the fields */
        hdr->iph_Version = 4;       /* IP revision #4 is the most recent */
        hdr->iph_IHL = 5;           /* I never send options, so the header is 20
                                     * bytes.  20/4 = 5 */
        be->be_length = 20;
        hdr->iph_TypeOfService = 0; /* Default service */
        hdr->iph_Length = DataLength(obuff);        /* Count up the size */

        /* Make sure that fragged packets all use the same identification # */
        Forbid();
        if ((frags == 0) && (!morefrags))
            hdr->iph_Ident = gb->IPIdentNumber++;
        else
            hdr->iph_Ident = inident;
        Permit();

        hdr->iph_Flags = 0;

        if (morefrags || fraglast)
            hdr->iph_Flags |= IPFLAG_MOREFRAGMENTS;
        hdr->iph_FragmentOffset = frags;

        hdr->iph_TimeToLive = ttl;
        hdr->iph_Protocol = protocol;
        hdr->iph_Checksum = 0;

        if (srcip)
            hdr->iph_SrcAddr = srcip;
        else
            hdr->iph_SrcAddr = iface->sanadev_IPAddress;

        hdr->iph_DestAddr = destip;

        /*
         * Calculate the checksum Note that the IP checksum covers ONLY the IP
         * header, and NOT the rest of the datagram!
         */

        hdr->iph_Checksum = (0xffffffff - CalcChecksum((UBYTE *) hdr, 20, 0));

        if (dbc)
        {
            PacketWrite(iface, 0L, obuff, 0L, PACKET_IP, TRUE, FALSE);
        }
        else
        {
            ONTIMER(7);
            PacketWrite(iface, gateway, obuff, destip, PACKET_IP, FALSE, FALSE);
            OFFTIMER(7);
        }
    }
    OFFTIMER(0);
}

/*------------------------------------------------------------------------*/
void ip_sendsrc(obuff, srcip, destip, protocol)
struct Buffer *obuff;
ULONG srcip;
ULONG destip;
UBYTE protocol;
{
    UWORD tmpid;
    Forbid();
    tmpid = gb->IPIdentNumber++;
    Permit();
    ONTIMER(11);
    multiplex(obuff, 0L, NULL, srcip, destip, protocol, 0, FALSE, tmpid, 30, FALSE);
    OFFTIMER(11);
}

void multiplex_in(ibuff, dev)
struct Buffer *ibuff;
struct sanadev *dev;
{
    struct BuffEntry *be;
    UWORD checksum;
    struct iphdr *ipkt;
    BOOL fraglast;
    ULONG headerlength;

    ONTIMER(10);

    be = (struct BuffEntry *) ibuff->buff_list.mlh_Head;
    ipkt = (struct iphdr *) (be->be_data + be->be_offset);
    fraglast = (ipkt->iph_Flags & IPFLAG_MOREFRAGMENTS);
    headerlength = ipkt->iph_IHL << 2;

    checksum = CalcChecksum((UBYTE *) ipkt, headerlength, 0L);
    if((ipkt->iph_Version == 4) && (checksum == 0xFFFF))
    {
        be->be_offset += headerlength;
        be->be_length = ipkt->iph_Length - headerlength;

        multiplex(ibuff, headerlength, dev, ipkt->iph_SrcAddr, ipkt->iph_DestAddr, ipkt->iph_Protocol,
                  ipkt->iph_FragmentOffset, fraglast, ipkt->iph_Ident, ipkt->iph_TimeToLive, fraglast);
    }
    else
    {
        FreeBuffer(ibuff);
    }
    OFFTIMER(10);
}

/*------------------------------------------------------------------------*/

void multiplex(ibuff, headerlength, dev, SrcIP, DestIP, protocol, frags, morefrags, ident, ttl, fraglast)
struct Buffer *ibuff;
struct sanadev *dev;
ULONG headerlength, SrcIP, DestIP, frags;
UBYTE protocol, ttl;
BOOL morefrags,fraglast;
UWORD ident;
{
    struct sanadev *iface;
    struct sanadev *propiface;
    struct NIPCRoute *route;
    struct Buffer *clonebuff;
    struct BuffEntry *be;
    BOOL LocalCopy, Sent, SendClone, ForwardPacket;

    ONTIMER(1);
    LocalCopy = Sent = ForwardPacket = SendClone = FALSE;

    if(IsLocalDevice(DestIP))
    {
        LocalCopy = TRUE;
    }
    else if((dev) && (IsLocalDevice(SrcIP)))
    {
        ;
    }
    else if(IsBroadcast(DestIP))
    {
        if(iface = LocalNet(DestIP))
        {
            if(iface->sanadev_SValid)
            {
                if(route = Route(SrcIP, FALSE))
                {
                    if ((DestIP & ~(iface->sanadev_SubnetMask)) == ~iface->sanadev_SubnetMask)  /* Host broadcast */
                    {
                        if((route->nr_Device == dev) || (!dev))
                        {
                            propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

                            while (propiface->sanadev_Node.mln_Succ)
                            {
                                if (propiface != dev)       /* Don't bounce packets */
                                {
                                    if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                                    {
                                        ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                                               propiface->sanadev_IPAddress, frags, morefrags,
                                               ident, ttl - 1, TRUE, fraglast);
                                    }
                                }
                                propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
                            }
                        }
                    }
                    else if(!dev)
                        SendClone = TRUE;

                }
            }
            else
            {
                propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

                while (propiface->sanadev_Node.mln_Succ)
                {
                    if(propiface != dev)
                    {
                        if((propiface->sanadev_IPNetwork | ~propiface->sanadev_IPMask) == DestIP)
                        {
                            if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                            {
                                ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                                       propiface->sanadev_IPAddress, frags, morefrags,
                                       ident, ttl - 1, TRUE, fraglast);
                            }
                        }
                    }
                    propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
                }
            }
            LocalCopy = TRUE;
        }
        else
        {
            if((DestIP == 0xffffffff) && (!dev))
            {
                propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

                while (propiface->sanadev_Node.mln_Succ)
                {
                    if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                    {
                        ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                               propiface->sanadev_IPAddress, frags, morefrags,
                               ident, ttl - 1, TRUE, fraglast);
                    }
                    propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
                }
                LocalCopy = TRUE;
            }
            else if ((DestIP == 0xffffffff) && (dev))
            {
                LocalCopy = TRUE;
            }
            else if (route = Route(DestIP, FALSE))
            {
                ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                       frags, morefrags, ident, ttl - 1, FALSE, fraglast);
                Sent = TRUE;
                LocalCopy = FALSE;
            }
        }
    }
    else
    {
        ForwardPacket = TRUE;
    }
    if(ForwardPacket)
    {
        if(route = Route(DestIP, FALSE))
        {
            ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                   frags, morefrags, ident, ttl - 1, FALSE, fraglast);
            Sent = TRUE;
        }
    }
    if(SendClone)
    {
        if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
            ip_out(clonebuff, SrcIP, DestIP, protocol, iface, iface->sanadev_IPAddress,
                   frags, morefrags, ident, ttl - 1, TRUE, fraglast);
    }

#ifdef FOO

    if((dev) && (IsLocalDevice(SrcIP)))
    {
        ;
    }
    else if(IsLocalDevice(DestIP))
        LocalCopy = TRUE;
    else if (iface = LocalNet(DestIP))
    {
        if (iface->sanadev_SValid)
        {
            DestSubnet = DestIP & iface->sanadev_SubnetMask;
            SubMask = ~(iface->sanadev_IPMask | (~iface->sanadev_SubnetMask));
            if ((SubMask & DestIP) == SubMask)  /* Subnet Broadcast */
            {
                route = Route(SrcIP, FALSE);
                if (dev == route->nr_Device)
                {
                    if ((DestIP & ~(iface->sanadev_SubnetMask)) == ~iface->sanadev_SubnetMask)  /* Host broadcast */
                    {
                        propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

                        while (propiface->sanadev_Node.mln_Succ)
                        {
                            if (propiface != dev)       /* Don't bounce packets */
                            {
                                if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                                {
                                    ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                                           propiface->sanadev_IPAddress, frags, morefrags,
                                           ident, ttl - 1, TRUE, fraglast);
                                }
                            }

                            propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
                        }
                        LocalCopy = TRUE;
                    }
                }
            }
            else /* Not a Subnet Broadcast */
            {
                if (!dev)
                {
                    if ((DestIP & ~(iface->sanadev_SubnetMask)) == ~iface->sanadev_SubnetMask)  /* Host broadcast */
                    {
                            if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                                ip_out(clonebuff, SrcIP, DestIP, protocol, iface, iface->sanadev_IPAddress,
                                       frags, morefrags, ident, ttl - 1, TRUE, fraglast);
                            Sent = TRUE;
                            LocalCopy = TRUE;
                    }
                    else
                    if (route = Route(DestIP, FALSE))
                    {
                        ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                               frags, morefrags, ident, ttl - 1, FALSE, fraglast);
                        Sent = TRUE;
                        LocalCopy = FALSE;
                    }
                }
                else
                {
                    if (DestSubnet == dev->sanadev_IPSubnet)
                    {
                        if ((DestIP & ~(iface->sanadev_SubnetMask)) == ~iface->sanadev_SubnetMask)  /* Host broadcast */
                        {
                            LocalCopy = TRUE;
                        }
                    }
                    else
                    {
                        if (route = Route(DestIP, FALSE))
                        {
                            if(IsBroadcast(DestIP))
                            {
                                LocalCopy = TRUE;
                                if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                                    ip_out(clonebuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                                        frags, morefrags, ident, ttl - 1, TRUE, fraglast);
                            }
                            else
                            {
                                ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                                       frags, morefrags, ident, ttl - 1, FALSE, fraglast);
                                LocalCopy = FALSE;
                            }
                            Sent = TRUE;
                        }
                    }
                }
            }
        }
        else
        {
            if (IsBroadcast(DestIP))
            {
                propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

                while (propiface->sanadev_Node.mln_Succ)
                {
                    if(propiface != dev)
                    {
                        if((propiface->sanadev_IPNetwork | ~propiface->sanadev_IPMask) == DestIP)
                        {
                            if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                            {
                                ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                                       propiface->sanadev_IPAddress, frags, morefrags,
                                       ident, ttl - 1, TRUE, fraglast);
                            }
                        }
                    }
                    propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
                }
                LocalCopy = TRUE;
            }
            else if (route = Route(DestIP, FALSE))
            {
                ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                      frags, morefrags, ident, ttl - 1, FALSE, fraglast);

                Sent = TRUE;
                LocalCopy = FALSE;
            }
        }
    }
    else
    {
        if((DestIP == 0xffffffff) && (!dev))
        {
            propiface = (struct sanadev *) gb->DeviceList.lh_Head->ln_Succ;

            while (propiface->sanadev_Node.mln_Succ)
            {
                if(clonebuff = (struct Buffer *) CloneBuffer(ibuff))
                {
                    ip_out(clonebuff, SrcIP, DestIP, protocol, propiface,
                           propiface->sanadev_IPAddress, frags, morefrags,
                           ident, ttl - 1, TRUE, fraglast);
                }
                propiface = (struct sanadev *) propiface->sanadev_Node.mln_Succ;
            }
            LocalCopy = TRUE;
        }
        else if ((DestIP == 0xffffffff) && (dev))
        {
            LocalCopy = TRUE;
        }
        else if (route = Route(DestIP, FALSE))
        {
            ip_out(ibuff, SrcIP, DestIP, protocol, route->nr_Device, route->nr_Gateway,
                   frags, morefrags, ident, ttl - 1, FALSE, fraglast);
            Sent = TRUE;
            LocalCopy = FALSE;
        }
    }
#endif

    if(LocalCopy)
    {
        if(!dev)
            dev = (struct sanadev *) gb->DeviceList.lh_Head;
        if(!headerlength)
        {
            ip_out(ibuff, dev->sanadev_IPAddress, dev->sanadev_IPAddress, protocol, dev,
                        dev->sanadev_IPAddress, frags, morefrags, ident, ttl - 1, FALSE, fraglast);
        }
        else
        {
            be = (struct BuffEntry *) ibuff->buff_list.mlh_Head;
            be->be_offset -= headerlength;
            be->be_length += headerlength;
            ip_in(ibuff, dev);
        }
    }
    if (!Sent && !LocalCopy)
    {
        if(headerlength)
        {
            be = (struct BuffEntry *) ibuff->buff_list.mlh_Head;
            be->be_offset -= headerlength;
            be->be_length += headerlength;
        }
        FreeBuffer(ibuff);
    }
    OFFTIMER(1);
}
/*------------------------------------------------------------------------*/
