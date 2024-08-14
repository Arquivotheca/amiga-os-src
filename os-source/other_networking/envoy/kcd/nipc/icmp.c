/**************************************************************************
**
** icmp.c       - Internet Control Message Protocol
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: icmp.c,v 1.11 92/06/08 10:09:10 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"


#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

/*------------------------------------------------------------------------*/

BOOL init_icmp(void)
{
    register struct NBase *nb = gb;
    gb->icmpproto.ipproto_ProtocolNumber = 1;
    gb->icmpproto.ipproto_Input = &icmp_input;
    gb->icmpproto.ipproto_Deinit = &icmp_deinit;
    gb->icmpproto.ipproto_Timeout = &icmp_timeout;
    AddHead((struct List *) &nb->ProtocolList, (struct Node *) &nb->icmpproto);       /* Tell IP about ICMP */
    return(TRUE);
}

/*------------------------------------------------------------------------*/

void icmp_timeout(void)
{
    return;
}

/*------------------------------------------------------------------------*/

void icmp_deinit(void)
{
    register struct NBase *nb = gb;
    Remove((struct Node *) & nb->icmpproto);
}

/*------------------------------------------------------------------------*/

void icmp_input(buff, length, dev)
struct Buffer *buff;
ULONG length;
struct sanadev *dev;
{
    struct BuffEntry *be;
    struct NIPCRoute *route;
    struct icmphdr *pkt;
    ULONG datalength;
    struct Buffer *obuff;
    struct BuffEntry *obe;
    struct icmphdrshort *ihdr;

    be = (struct BuffEntry *) (buff->buff_list.mlh_Head);
    if (be->be_length >= sizeof(struct icmphdr))
    {
        pkt = (struct icmphdr *) (be->be_data + be->be_offset);

        datalength = length - sizeof(struct icmphdr);

        switch (pkt->icmp_icmp.icmps_type)
        {
            case (ICMP_ECHO):
                                if(obuff = AllocBuffer(datalength))
                                {
                                    CopyToBuffer(obuff, (be->be_data) + (length - datalength), datalength);
                                    if(obe = AllocBuffEntry(sizeof(struct icmphdrshort)))
                                    {
                                        obe->be_length = (sizeof(struct icmphdrshort));
                                        AddHead((struct List *) & (obuff->buff_list), (struct Node *) obe);
                                        ihdr = (struct icmphdrshort *) obe->be_data;
                                        ihdr->icmps_type = ICMP_ECHOREPLY;
                                        ihdr->icmps_code = 0;
                                        ihdr->icmps_checksum = 0;
                                        ihdr->icmps_ident = pkt->icmp_icmp.icmps_ident;
                                        ihdr->icmps_seqnum = pkt->icmp_icmp.icmps_seqnum;
                                        ihdr->icmps_checksum = (0xFFFF - CalcChecksumBuffer(obuff));
                                        if (route = Route(pkt->icmp_iphdr.iph_SrcAddr, FALSE))
                                        {
                                            ip_out(obuff, pkt->icmp_iphdr.iph_DestAddr, pkt->icmp_iphdr.iph_SrcAddr, 1, route->nr_Device, route->nr_Gateway, 0, FALSE, 0, 30, FALSE, FALSE);
                                        }
                                    }
                                    else
                                        FreeBuffer(obuff);
                                }
                                break;
        }
    }
    FreeBuffer(buff);

}
/*------------------------------------------------------------------------*/
