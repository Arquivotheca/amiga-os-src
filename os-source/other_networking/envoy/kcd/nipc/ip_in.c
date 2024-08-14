/**************************************************************************
**
** ip_in.c      - Deal with incoming IP packets
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: ip_in.c,v 1.26 93/08/10 16:15:36 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <clib/exec_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>

#define FRAGTIMEOUT 3          /* 3 seconds timeout for fragments */

/*------------------------------------------------------------------------*/

void ip_in(ibuff, dev)
struct sanadev *dev;            /* ... and the device you rode in on! */
struct Buffer *ibuff;           /* The packet to sift */
{

    struct ipproto *protoptr;

    struct iphdr *ipkt;
    struct BuffEntry *be;
    ULONG packet_length,datalen;
    UWORD our_checksum;
    ONTIMER(2);
    if (ibuff->buff_list.mlh_Head->mln_Succ)
    {
        be = (struct BuffEntry *) ibuff->buff_list.mlh_Head;

        if (be->be_length >= sizeof(struct iphdr))
        {
            ipkt = (struct iphdr *) (be->be_data + be->be_offset);

            if (ipkt->iph_Version == 4)
            {
                packet_length = (ipkt->iph_IHL << 2);

                datalen = ipkt->iph_Length;

//                our_checksum = CalcChecksum((UBYTE *) ipkt, packet_length, 0L);
//                if (our_checksum == 0xFFFF)
//                {

                    /* checksum already verified in multiplex_in() */

                    /*
                     * Check for some level of fragmentation.  If none, completely skip the
                     * frag code.  If some, add it to the frag list, with the proper id
                     * (source+dest+protocol+identification).  Then, attempt to put all of
                     * the pieces together. Note that duplicate packets are held onto - and
                     * even if they're not used, eventually should time out.  However, we
                     * can't protect upper levels from duplicate packets.  If all of the
                     * pieces are duplicated (or even one packet, for one with no
                     * fragmentation), they WILL get passed on. This isn't IP's problem.
                     */

                    if ((ipkt->iph_FragmentOffset != 0) || (ipkt->iph_Flags & IPFLAG_MOREFRAGMENTS))
                    {

                        struct Buffer *os;
                        struct Buffer *assemble[150];
                        struct iphdr *iptr;
                        int acount;
                        BOOL completed = FALSE;
                        ibuff->buff_timeout = FRAGTIMEOUT;
                        AddTail((struct List *) & (dev->sanadev_Fragments), (struct Node *) & (ibuff->buff_link));

                        /*
                         * Parse the list.  Each time we find the beginning of a fragment,
                         * parse the list for the next entry, and so on.
                         */

                        os = (struct Buffer *) dev->sanadev_Fragments.mlh_Head;
                        while (os->buff_link.mln_Succ)
                        {
                            struct BuffEntry *hbe;
                            iptr = (struct iphdr *) BuffPointer(os, 0L, &hbe);
                            if (iptr->iph_FragmentOffset == 0)  /* Found the start of one */
                            {
                                struct Buffer *is;
                                struct iphdr *iniptr;
                                BOOL cantfindit;
                                UWORD nxtoffset;
                                nxtoffset = (iptr->iph_Length - (iptr->iph_IHL << 2));

                                cantfindit = FALSE;
                                acount = 0;

                                assemble[acount] = os;
                                acount++;

                                while (!cantfindit)
                                {
                                    completed = FALSE;
                                    /* Look for that specific piece */
                                    is = (struct Buffer *) dev->sanadev_Fragments.mlh_Head;
                                    while (is->buff_link.mln_Succ)
                                    {
                                        iniptr = (struct iphdr *) ((struct BuffEntry *) is->buff_list.mlh_Head)->be_data;
                                        if ((iniptr->iph_FragmentOffset << 3) == nxtoffset)
                                        {
                                            if ((iniptr->iph_Ident == iptr->iph_Ident) &&
                                             (iniptr->iph_Protocol == iptr->iph_Protocol) &&
                                                (iniptr->iph_SrcAddr == iptr->iph_SrcAddr) &&
                                                (iniptr->iph_DestAddr == iptr->iph_DestAddr))
                                            {
                                                /*
                                                 * found the next piece.  If all done, drop
                                                 * out.  else loop for the next piece
                                                 */
                                                nxtoffset += (iniptr->iph_Length - (iniptr->iph_IHL << 2));
                                                if (!(iniptr->iph_Flags & IPFLAG_MOREFRAGMENTS))
                                                    completed = TRUE;
                                                /* regardless, get it on the list */
                                                assemble[acount] = is;
                                                acount++;
                                                break;
                                            }
                                        }
                                        is = (struct Buffer *) is->buff_link.mln_Succ;
                                    }
                                    if (!is->buff_link.mln_Succ)
                                    {
                                        cantfindit = TRUE;      /* if at end of list */
                                    }
                                    if (completed)
                                        break;
                                }

                                if (completed)
                                {
                                    struct Buffer *headbuffer;
                                    struct iphdr *headip;
                                    struct Buffer *piece;
                                    struct iphdr *iptmp;
                                    int rc;
                                    struct BuffEntry *hbe;
                                    rc = 0;
                                    headbuffer = assemble[rc];
                                    rc++;
                                    headip = (struct iphdr *) BuffPointer(headbuffer, 0L, &hbe);
                                    Remove((struct Node *) headbuffer);
                                    while (rc != acount)
                                    {
                                        piece = assemble[rc];
                                        rc++;
                                        Remove((struct Node *) piece);
                                        iptmp = (struct iphdr *) BuffPointer(piece, 0L, &hbe);
                                        ((struct BuffEntry *) piece->buff_list.mlh_Head)->be_offset += (iptmp->iph_IHL << 2);
                                        ((struct BuffEntry *) piece->buff_list.mlh_Head)->be_length -= (iptmp->iph_IHL << 2);

                                        headip->iph_Length += DataLength(piece);
                                        MergeBuffer(headbuffer, piece);
                                        ibuff = headbuffer;
                                        ipkt = (struct iphdr *) BuffPointer(ibuff, 0L, &hbe);
                                    }           /* endwhile */
                                    break;
                                }               /* endif */
                            }                   /* endif */
                            os = (struct Buffer *) os->buff_link.mln_Succ;
                        }                       /* endwhile */
                        if (!completed)
                        {
                            OFFTIMER(2);
                            return;
                        }
                    }                           /* endif */

                    /* Cool.  Let's ship the packet along up to the next protocol */

                    /*
                     * Unfortunately, this list could be changing underneath us. It's
                     * probably wise to rid the code of the Forbid/Permit pairs and institure
                     * a signal semaphore for the list.
                     */

//                    if(!datalen)
//                    {
//                        kprintf("Alert! Alert! Recevied zero length packet from %lx!\n",ipkt->iph_SrcAddr);
//                    }

                    Forbid();

                    protoptr = (struct ipproto *) gb->ProtocolList.mlh_Head;
                    while (protoptr->ipproto_link.mln_Succ)
                    {

                        if (protoptr->ipproto_ProtocolNumber == ipkt->iph_Protocol)
                        {
                            Permit();

                            if (CountEntries(ibuff) > 1)
                            {
                                struct Buffer *i2buff;
                                i2buff = (struct Buffer *) CloneBuffer(ibuff);
                                FreeBuffer(ibuff);
                                ibuff = i2buff;
                            }
                            OFFTIMER(2);
                            if(ibuff)
                                (*protoptr->ipproto_Input) (ibuff, datalen, dev);
                            return;
                        }
                        protoptr = (struct ipproto *) protoptr->ipproto_link.mln_Succ;
                    }
                    Permit();                   /* Couldn't find a destination protocol */
//                }
            }
        }
    }
    FreeBuffer(ibuff);

    OFFTIMER(2);
}

/*------------------------------------------------------------------------*/

/* Routine to timeout IP fragments when we perceive them to be worthless */

void FragTimeoutDev(dev)
struct sanadev *dev;
{
    struct Buffer *tob;
    tob = (struct Buffer *) dev->sanadev_Fragments.mlh_Head;

    while (tob->buff_link.mln_Succ)
    {
        tob->buff_timeout--;
        if (!(tob->buff_timeout))
        {
            struct Buffer *rem;
            rem = tob;
            tob = (struct Buffer *) tob->buff_link.mln_Pred;
            Remove((struct Node *) rem);
            FreeBuffer(rem);
        }
        tob = (struct Buffer *) tob->buff_link.mln_Succ;
    }
}

/*------------------------------------------------------------------------*/

void FragmentTimeout()
{

    struct sanadev *sd;

    sd = (struct sanadev *) gb->DeviceList.lh_Head;
    while (sd->sanadev_Node.mln_Succ)
    {
        FragTimeoutDev(sd);
        sd = (struct sanadev *) sd->sanadev_Node.mln_Succ;
    }
}

/*------------------------------------------------------------------------*/
