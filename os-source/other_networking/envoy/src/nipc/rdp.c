/**************************************************************************
**
** rdp.c        - RDP Protocol routines for nipc.library
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: rdp.c,v 1.46 94/02/07 19:53:40 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

//#define NOBUFFSHARING           1

#define max(a,b) ((a > b) ? a : b)

#include "nipcinternal.h"


#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "externs.h"
#include <pragmas/timer_pragmas.h>
#include <clib/timer_protos.h>


#define MAXSEGS 1
#define MAX_ACK_DIFF 3		/* Max number of non-acked packets to allow before
				   forcing an ACK packet */
#define MAXSIZE 8192

#define MIN(x, y) ((x) < (y) ? (x):(y))

/* #define DEBUGMSGS */
#define RETRIES  8
#define TIMEOUT  2             /* Temporary until I can work in an adaptive (2*10) timeout */
#define CONNTIME 1             /* 10ths of a second between connection state
                                 * timeouts */
#define TOTALALLOWED 10         /* # of times we'll send connection
                                 * retranmissions before giving up */

/*------------------------------------------------------------------------*/
/*
 * RDP
 *
 * - Piggybacked/delayed ACKS would be nice to save on net bandwidth.  Do this
 * if we get approval for ANMP/RDP/IP/SANAII set
 *
 * - The multiple spawning mechanism for the well-known entity resolver should
 * also time out dead connections.
 *
 */

/* Watch out for sequence # wraparound!! */

/*------------------------------------------------------------------------*/

BOOL rdp_init()
{
    register struct NBase *nb = gb;
//    ULONG eclkdata[2];

//    ReadEClock( (struct EClockVal *) &eclkdata);

//    gb->RDPPorts = (UWORD) ((UWORD *)&eclkdata[1])[1];
    gb->RDPPorts = 1024L;

    InitSemaphore(&nb->RDPCLSemaphore);     /* Init the semaphore */
    NewList((struct List *) & gb->RDPConnectionList);       /* Init the list */

    gb->RDPProto.ipproto_ProtocolNumber = 26;       /* Set up the ipproto
                                                 * structure */
    gb->RDPProto.ipproto_Input = &rdp_input;
    gb->RDPProto.ipproto_Deinit = &rdp_deinit;
    gb->RDPProto.ipproto_Timeout = &rdp_timeout;
    AddHead((struct List *) & nb->ProtocolList, (struct Node *) & nb->RDPProto);        /* Announce us to IP */

    return(TRUE);

}

/*------------------------------------------------------------------------*/

void rdp_deinit()
{
        register struct NBase *nb = gb;

    Remove((struct Node *) & nb->RDPProto);
    ObtainSemaphore(&nb->RDPCLSemaphore);
    while (!(IsListEmpty( (struct List *) &nb->RDPConnectionList)))
    {
        struct RDPConnection *rc;
        rc = (struct RDPConnection *) gb->RDPConnectionList.mlh_Head;
        rc->conn_State = STATE_CLOSE;  /* keep it from trying to send RST's */
        CloseConnection(rc);
    }
    ReleaseSemaphore(&nb->RDPCLSemaphore);
}


/*------------------------------------------------------------------------*/

//
// From our table, given the speed of the device, make a guess as to the correct
// window size.  (We need a SANA-2 field for devices that can describe themselves
// in a level of "lossy-ness" sooooo badly!  ARCNet almost never loses a packet;
// AmokNet loses about 1/4th of them.
//
int RDPFindWindow(struct sanadev *sd)
{
    struct initdataform
    {
        LONG BPSRate;
        LONG WindowSize;
    };

    int x;
    struct initdataform initdata[]={9600,3,30000,3,60000,3,100000,4,500000,5,2000000,6,10000000,8,-1,16};

    x = 0;
    while (initdata[x].BPSRate > 0)
    {
        if (sd->sanadev_BPS <= initdata[x].BPSRate)
            return(initdata[x].WindowSize);
        x++;
    }
    return(32); // Max.  For VERY high speed thingys.

}

/*------------------------------------------------------------------------*/



void rdp_ack_timeout()
{
    register struct NBase *nb = gb;

    struct RDPConnection *conn;

    ObtainSemaphore(&nb->RDPCLSemaphore);
    conn = (struct RDPConnection *) gb->RDPConnectionList.mlh_Head;
    while (conn->conn_link.mln_Succ)
    {
        if (conn->conn_Flags & CONN_UNSENTACK)
        {
            Forbid();
            conn->conn_Flags &= ~CONN_UNSENTACK;
            Permit();
            SendFlags(conn,RDPF_ACK);
        }

        conn = (struct RDPConnection *) conn->conn_link.mln_Succ;
    }
    ReleaseSemaphore(&nb->RDPCLSemaphore);
}


/*------------------------------------------------------------------------*/

void rdp_timeout()
{
        register struct NBase *nb = gb;

    struct RDPConnection *conn;
    struct retransmit *rexmit;
    BOOL rig;


    /*
     * Run through all of the open connections, and decrement the counts for
     * timeouts
     */
    ObtainSemaphore(&nb->RDPCLSemaphore);
    conn = (struct RDPConnection *) gb->RDPConnectionList.mlh_Head;
    while (conn->conn_link.mln_Succ)
    {
        rig = FALSE;
        /* Do any connection timeouts here */
        if ((conn->conn_State != STATE_OPEN) && (conn->conn_State != STATE_CLOSE) &&
            (conn->conn_State != STATE_LISTEN))
        {
            conn->conn_LastConnTime--;      /* dec countdown */
            if (!conn->conn_LastConnTime)   /* when it reaches 0 */
            {
                conn->conn_ConnRetransmits++;
                if ((conn->conn_ConnRetransmits == TOTALALLOWED) || (conn->conn_State == STATE_TIMEOUT))
                {
                    SendFlags(conn, RDPF_RST);
                    conn->conn_State = STATE_CLOSE;
                    conn = (struct RDPConnection *) conn->conn_link.mln_Succ;
                    ObtainSemaphore(&nb->RDPCLSemaphore);
                    /* Don'tcha just LOVE C when the code gets like the next line? */
                    if ((*((struct RDPConnection *)conn->conn_link.mln_Pred)->conn_Status) ((struct RDPConnection *)conn->conn_link.mln_Pred))
                    /* In case I forget ... the above line is basically just (*conn->conn_Status)(conn); except dealing with the conn one back on the list from conn */
                    {
                        ReleaseSemaphore(&nb->RDPCLSemaphore);
                        continue;
                    }
                    ReleaseSemaphore(&nb->RDPCLSemaphore);
                }
                else
                    SendFlags(conn, conn->conn_LastFlags);
            }
        }
        if (conn->conn_link.mln_Succ)
        {
            /* Retransmission timeouts */
            ObtainSemaphore(&(conn->conn_WriteListSemaphore));
            rexmit = (struct retransmit *) conn->conn_DataListW.mlh_Head;
            while (rexmit->xmit_link.mln_Succ)
            {
                /* Decrement the countdown */
                rexmit->xmit_countdown--;
                /* If we reach zero,  consider retransmitting */
                if (!rexmit->xmit_countdown)
                {
                    /*
                     * decrement the retry count.  If 0, free this entry,
                     * and give up on it
                     */
                    rexmit->xmit_retrycount--;
                    if ((!rexmit->xmit_retrycount) || (rexmit->xmit_seqnum < conn->conn_SendOldest))
                    {
                        struct retransmit *temp;
                        BOOL closeit = FALSE;
                        temp = rexmit;
                        if (!rexmit->xmit_retrycount)
                        {
                          /*  struct rdphdr *rr; */
                          /*  struct BuffEntry *xbe; */
                            closeit = TRUE;
                          /*  rr = (struct rdphdr *) BuffPointer(temp->xmit_data,0L,&xbe); */
                        }
                        rexmit = (struct retransmit *) rexmit->xmit_link.mln_Pred;
                        Remove((struct Node *) temp);
                        temp->xmit_data->buff_refcnt--;
#if WHAT_THE_FUCK // ????
                        if (TRUE)
                        {
                            struct rdphdr *rr;
                            struct BuffEntry *xbe;
                            rr = (struct rdphdr *) BuffPointer(temp->xmit_data,sizeof(struct iphdr),&xbe);
                        }
#endif
                        FreeBuffer(temp->xmit_data);
                        FreeMem(temp, sizeof(struct retransmit));
                        /*
                         * This has gotten out of hand - force the
                         * connection closed
                         */
                        if (closeit)
                        {
                            SendFlags(conn, RDPF_RST);
                            conn->conn_State = STATE_CLOSE;
                            conn = (struct RDPConnection *) conn->conn_link.mln_Succ;
                            ObtainSemaphore(&nb->RDPCLSemaphore);                       /* Isn't this redundant?  Don't we already have it locked? */
                            rig = (*((struct RDPConnection *)conn->conn_link.mln_Pred)->conn_Status) ((struct RDPConnection *)conn->conn_link.mln_Pred);
                            ReleaseSemaphore(&nb->RDPCLSemaphore);                      /* See above */
                            if (!rig)
                                conn = (struct RDPConnection *) conn->conn_link.mln_Pred; /* If not deleted, step back ... jeeze, how fucking ugly.  I need to recode this whole damned thing. */
                            break;
                        }
                    }
                    else
                    {
                        if (conn->conn_State == STATE_OPEN)
                        {
                            /* We have to yank the IP header off the front of this thing, since one will have
                               already been tacked on by now. */

#ifndef NOBUFFSHARING
                            if(rexmit->xmit_data->buff_refcnt == 1) /* Make sure it's not in ARP's queue... */
                            {

                                struct BuffEntry *bufe;

				bufe = (struct BuffEntry *)rexmit->xmit_data->buff_list.mlh_Head;
				if(bufe->be_link.mln_Succ && (bufe->be_length != sizeof(struct rdphdr)))
                                {
                                    Remove((struct Node *)bufe);
                                    FreeBuffEntry(bufe);
                                }
                                ip_sendsrc(rexmit->xmit_data /* rcpy */, conn->conn_OurAddress, rexmit->xmit_ToAddress, 26);
                            }

#else
                            if (TRUE)
                            {
                                struct Buffer *cb;
                                cb = CloneBuffer(rexmit->xmit_data);
                                ip_sendsrc(cb, conn->conn_OurAddress, rexmit->xmit_ToAddress, 26);
                            }
#endif
                        }
                        rexmit->xmit_countdown = TIMEOUT + conn->conn_MTUDelay;
                    }
                }
                rexmit = (struct retransmit *) rexmit->xmit_link.mln_Succ;
            }
            if (!rig)   /* As long as the connection wasn't deleted, release the semaphore, and skip to the next connection */
            {
                ReleaseSemaphore(&(conn->conn_WriteListSemaphore));
                conn = (struct RDPConnection *) conn->conn_link.mln_Succ;
            }
        }
    }
    ReleaseSemaphore(&nb->RDPCLSemaphore);

    return;
}

/*------------------------------------------------------------------------*/

void rdp_input(buff, length, dev)
struct Buffer *buff;
ULONG length;
struct sanadev *dev;
{
    register struct NBase *nb = gb;

    struct BuffEntry *be;
    struct rdphdr *r;
    struct iphdr *ip;
    ULONG source_address, destination_address;
    struct RDPConnection *conn;
    UWORD oldchecksum;

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    ip = (struct iphdr *) ((ULONG) be->be_data + (ULONG) be->be_offset);
    r = (struct rdphdr *) ((ULONG) ip + (ULONG) sizeof(struct iphdr));

    source_address = ip->iph_SrcAddr;
    destination_address = ip->iph_DestAddr;

    /* If this isn't a packet of the version we know about, forget it */
    if (r->rdp_Version != RDP_VERSION)
    {
        FreeBuffer(buff);
        return;
    }


    /* Check to see if the checksum is valid */
    /* FILL THIS CODE IN */
    oldchecksum = r->rdp_Checksum;
    r->rdp_Checksum = 0;
    if (oldchecksum != (UWORD) CalcChecksum((UBYTE *) r, (ip->iph_Length - sizeof(struct iphdr)), 0))
    {
        FreeBuffer(buff);       /* Bad checksum -- drop the packet */
        return;
    }

    /* Check to see if there's a connection in our list that this matches */
    ObtainSemaphore(&nb->RDPCLSemaphore);
    conn = (struct RDPConnection *) gb->RDPConnectionList.mlh_Head;
    while (conn->conn_link.mln_Succ)
    {
        struct RDPConnection *nxt;
        nxt = (struct RDPConnection *) conn->conn_link.mln_Succ;
        if ((conn->conn_OurPort == r->rdp_DestPort) &&
            (conn->conn_TheirPort == r->rdp_SrcPort) &&
            (conn->conn_TheirAddress == source_address))
        {
            ULONG size;
            if (StateMachine(conn, r->rdp_Flags, r, destination_address, source_address))
            {
                size = ip->iph_Length - sizeof(struct iphdr) - r->rdp_HeaderLength;
                if (r->rdp_Flags & RDPF_SYN)
                    size -= 6;
                if (size)
                    PacketInput(conn, buff);
                else
                    FreeBuffer(buff);
            }
            else
            {
                conn = (struct RDPConnection *) nxt->conn_link.mln_Pred;
//                continue;
                FreeBuffer(buff);
            }
            break;
        }
        conn = nxt;
    }
    if (!conn->conn_link.mln_Succ)      /* If no connection exists */
    {
        ReleaseSemaphore(&nb->RDPCLSemaphore);
        if (r->rdp_Flags & RDPF_SYN)    /* They're trying to create a
                                         * connection */
        {
            struct RDPConnection *rconn;
            /*
             * Check to see if there's someone on that port - who isn't
             * connected
             */
            ObtainSemaphore(&nb->RDPCLSemaphore);
            rconn = (struct RDPConnection *) gb->RDPConnectionList.mlh_Head;
            while (rconn->conn_link.mln_Succ)
            {
                struct RDPConnection *nxt;
                nxt = (struct RDPConnection *) rconn->conn_link.mln_Succ;
                if ((rconn->conn_State == STATE_LISTEN) &&
                    (rconn->conn_OurPort == r->rdp_DestPort))
                {
                    if (rconn->conn_Flags & CONN_MULTIPLE)
                    {
                        rconn = OpenPassive(rconn->conn_OurPort, rconn->conn_DataIn, rconn->conn_Status);
                        if (!rconn)
                        {
                            FreeBuffer(buff);
                            break;
                        }
                    }
                    rconn->conn_TheirAddress = source_address;  /* Keep track of who
                                                                 * this is! */
                    rconn->conn_TheirPort = r->rdp_SrcPort;     /* ditto */


                    if (!StateMachine(rconn, r->rdp_Flags, r, destination_address,source_address))
                    {
//                        rconn = nxt;
//                        continue;
                    }
                    FreeBuffer(buff);
                    break;
                }
                rconn = nxt;
            }
            ReleaseSemaphore(&nb->RDPCLSemaphore);
            if (!rconn->conn_link.mln_Succ)
            {
                struct RDPConnection tc;                       /* Reset them, so that they     */
                if (!(r->rdp_Flags & RDPF_RST))
                {
                    tc.conn_TheirPort = r->rdp_SrcPort;             /* stop trying immediately.     */
                    tc.conn_OurPort = r->rdp_DestPort;
                    tc.conn_TheirAddress = source_address;
                    tc.conn_OurAddress = destination_address;
                    tc.conn_SendNxt = r->rdp_AckNum+1;
                    SendFlags(&tc,RDPF_RST);
                }
                FreeBuffer(buff);
            }
        }
        else
        {
            struct RDPConnection tc;                       /* Reset them, so that they     */
            if (!(r->rdp_Flags & RDPF_RST))
            {
                tc.conn_TheirPort = r->rdp_SrcPort;             /* stop trying immediately.     */
                tc.conn_OurPort = r->rdp_DestPort;
                tc.conn_TheirAddress = source_address;
                tc.conn_OurAddress = destination_address;
                SendFlags(&tc,RDPF_RST);
            }
            FreeBuffer(buff);
        }
    }
    else
        ReleaseSemaphore(&nb->RDPCLSemaphore);

}

/*------------------------------------------------------------------------*/

/*
 * The RDP Connection state machine
 */

BOOL StateMachine(rc, flags, packet, ipdest,ipsrc)
struct RDPConnection *rc;       /* The connection in question */
UBYTE flags;                    /* The flag bits that describe the current
                                 * action */
struct rdphdr *packet;
ULONG ipdest, ipsrc;
{
    register struct NBase *nb = gb;

    UBYTE oldstate;



    if (flags & RDPF_SYN)
    {
        rc->conn_OurAddress = ipdest;
    }

    oldstate = rc->conn_State;  /* So we can see if the state changed */

    if (flags & RDPF_RST)
    {
        if (packet->rdp_SeqNum != (rc->conn_Current+1))
            return(TRUE);
        rc->conn_State = STATE_CLOSE;
    }

    switch (rc->conn_State)
    {
        case STATE_CLOSED:
            if (flags)
            {
                SendFlags(rc, RDPF_RST);        /* If we get ANYTHING while
                                                 * in this state, RESET
                                                 * ourselves! */
            }
            break;
        case STATE_LISTEN:
            if (flags & RDPF_SYN)
            {
                struct NIPCRoute *route;
                struct sanadev *sd;
                route = Route(ipsrc,TRUE);
                if (route)
                {
                    sd = route->nr_Device;  /* Find device it goes out on  ... */

                    rc->conn_RecvMax = RDPFindWindow(sd);

                    // stick with single-packet sizes for slow devices.
                    if (sd->sanadev_BPS < 1000000)
                       rc->conn_RecvMaxSize = ((sd->sanadev_MTU)-(sizeof(struct iphdr)+sizeof(struct rdphdr)));
                    else
                       rc->conn_RecvMaxSize = ((sd->sanadev_MTU)-(sizeof(struct iphdr)+sizeof(struct rdphdr)))*3;

                    rc->conn_InitialBPS = DivSASSucks(sd->sanadev_BPS,10);

                    if (sd->sanadev_BPS < 655360) /* If greater, time is trivial */
                        if (sd->sanadev_BPS > 65535) /* handle special cased, so we generate DIVS/DIVU not SAS divide code */
                        {                            /* If divisor > 16 bits, div by 16 first */
                            UWORD bps;
                            bps = (UWORD) ((ULONG) sd->sanadev_BPS >> 4);
                            if (bps)
                                rc->conn_TimeoutDelay = (UWORD) ( (ULONG)  40960 / bps ) < 1;
                        }
                        else
                            if (sd->sanadev_BPS)
                                rc->conn_TimeoutDelay = DivSASSucks(655360,sd->sanadev_BPS) < 1;
                    if (sd->sanadev_MTU < 65536)
                        rc->conn_MTUDelay = (rc->conn_TimeoutDelay >> 3); // div 3 because AMP dumps 8K chunks to RDP; time for 64K >> 3 = time for 8K
                }

                SendFlags(rc, RDPF_SYN | RDPF_ACK);
                rc->conn_State = STATE_RCVD;
                ReadEClock( (struct EClockVal *)&rc->conn_TimeStamp);

            }
            break;
        case STATE_SENT:
            if ((flags & (RDPF_SYN | RDPF_ACK)) == (RDPF_SYN | RDPF_ACK))
            {
                ULONG tmpts[2], freq;
                SendFlags(rc, RDPF_ACK);
                rc->conn_State = STATE_OPEN;
                freq = ReadEClock( (struct EClockVal *) &tmpts);
                if (tmpts[1] < rc->conn_TimeStamp[1])
                     tmpts[0]--;

                tmpts[1] = tmpts[1] - rc->conn_TimeStamp[1];
                tmpts[0] = tmpts[0] - rc->conn_TimeStamp[0];

                rc->conn_TimeStamp[0] = Div64by32(tmpts[0],tmpts[1],freq/1024);  /* Seconds*1024 */

            }
            else if (flags & RDPF_SYN)
            {
                SendFlags(rc, RDPF_SYN);
                rc->conn_State = STATE_RCVD;
                ReadEClock( (struct EClockVal *)&rc->conn_TimeStamp);

            }
            break;
        case STATE_RCVD:
            if (flags & RDPF_ACK)
            {
                ULONG tmpts[2], freq;
                rc->conn_State = STATE_OPEN;

                freq = ReadEClock( (struct EClockVal *) &tmpts);
                if (tmpts[1] < rc->conn_TimeStamp[1])
                    tmpts[0]--;

                tmpts[1] = tmpts[1] - rc->conn_TimeStamp[1];
                tmpts[0] = tmpts[0] - rc->conn_TimeStamp[0];

                rc->conn_TimeStamp[0] = Div64by32(tmpts[0],tmpts[1],freq/1024);  /* Seconds*1024 */
            }
            break;
        case STATE_OPEN:
            if (flags & RDPF_RST)
            {
                rc->conn_State = STATE_CLOSE;
            }
            break;

    }


    /* If they give us an ACK, log the ack # */
    if (flags & RDPF_ACK)
    {
        if ((packet->rdp_AckNum + 1) > rc->conn_SendOldest)
        {
            rc->conn_SendOldest = (packet->rdp_AckNum + 1);
            Forbid();
            if (rc->conn_Flags & CONN_WINDOWSIGNAL)
                Signal(rc->conn_WakeTask,(1 << rc->conn_WakeBit));
            Permit();
            /*
             * Now, filter out all retransmission saves that fall below
             * SendOldest
             */
            /* fill me in */
        }
    }

    /* If we have a SYN, get the SYN data too */

    if (flags & RDPF_SYN)
    {
        struct rdphdrSYN *rs;
        rs = (struct rdphdrSYN *) packet;
        rc->conn_SendMax = MIN(rs->rsyn_MaxOutstanding,rc->conn_RecvMax);
        rc->conn_SendMaxSize = MIN(rc->conn_RecvMaxSize,rs->rsyn_MaxSegment);
        rc->conn_InitRecv = packet->rdp_SeqNum + 1;
        rc->conn_SeqDelivered = packet->rdp_SeqNum;
        rc->conn_OurAddress = ipdest;
    }

    if (rc->conn_State != oldstate)     /* Let them know that this connection
                                         * status has changed */
        if (rc->conn_Status)
        {
            BOOL wasdead;
//            if (rc->conn_State == STATE_CLOSE)
//                wasdead = TRUE;
            ObtainSemaphore(&nb->RDPCLSemaphore);   /* This ought to be redundant - anyone who calls StateMachine() should already have the list locked */
            wasdead = (*rc->conn_Status)(rc);
            ReleaseSemaphore(&nb->RDPCLSemaphore);  /* Ditto */
//            if (wasdead)
//                return(FALSE);
            return((BOOL)!wasdead);
        }

    /* If we enter close-wait, perhaps a timer should start? */
    return(TRUE);

}

/*------------------------------------------------------------------------*/

/*
 * SendFlags(connection,flags); struct RDPConnection *connection; UBYTE
 * flags;
 *
 * This function is designed _entirely_ to provide for creating plain (dataless)
 * packets with certain flags set - like ACK, SYN, EAK, RST, NUL. It gains
 * all important knowledge of values for ACK fields and the like from the the
 * RDPConnection structure passed with the flag bitmap.
 */

void SendFlags(c, flags)
struct RDPConnection *c;
UBYTE flags;
{
    struct Buffer *fb;
    struct BuffEntry *fbe;
    struct rdphdr *r;
    ULONG headersize;

    headersize = sizeof(struct rdphdr);
    if (flags & RDPF_SYN)
        headersize += 6;
    if (flags & RDPF_EAK)
        headersize += 0;        /* This is dynamic FIXME later */
    fb = AllocBuffer(headersize);
    if (!fb)
        return;
    fbe = (struct BuffEntry *) fb->buff_list.mlh_Head;
    fbe->be_length = headersize;
    r = (struct rdphdr *) fbe->be_data;
    r->rdp_Flags = flags;
    r->rdp_Version = 2;
    r->rdp_HeaderLength = sizeof(struct rdphdr);
    r->rdp_SrcPort = c->conn_OurPort;
    r->rdp_DestPort = c->conn_TheirPort;
    Forbid();
    if (flags & RDPF_SYN)
    {
        c->conn_SendNxt = 1;
        r->rdp_SeqNum = 0;
    }
    else
        r->rdp_SeqNum = c->conn_SendNxt;
    if (flags & !(RDPF_SYN | RDPF_NUL))
        r->rdp_SeqNum--;
    Permit();
    r->rdp_AckNum = c->conn_Current;
    c->conn_LastAcked = c->conn_Current;
    if (flags & RDPF_SYN)
    {
        struct rdphdrSYN *rs;

        rs = (struct rdphdrSYN *) r;
        rs->rsyn_MaxOutstanding = c->conn_RecvMax;
        rs->rsyn_MaxSegment = c->conn_RecvMaxSize;  /* Figure this dynamically, based on the interface */
        rs->rsyn_Options = 0;
    }

    /*
     * if (flags & RDPF_EAK) { // build the eack list }
     *
     */
    r->rdp_Checksum = 0;
    r->rdp_Checksum = CalcChecksumBuffer(fb);

    c->conn_LastFlags = flags;
    c->conn_LastConnTime = CONNTIME;

    ip_sendsrc(fb, c->conn_OurAddress, c->conn_TheirAddress, 26);

}

/*------------------------------------------------------------------------*/

/*
 * PacketInput(connection,buffer) struct RDPConnection *connection; struct
 * Buffer *buffer;
 *
 * Queues up appropriate input on the appropriate connection lists
 */

void PacketInput(c, b)
struct RDPConnection *c;
struct Buffer *b;
{
        register struct NBase *nb = gb;

    UWORD pack;

    ObtainSemaphore(&(c->conn_ReadListSemaphore));
    AddTail((struct List *) & (c->conn_DataListR), (struct Node *) & (b->buff_link));
    c->conn_Current = FindSequential(c);

    Forbid();
    if((c->conn_Current - c->conn_LastAcked) >= max(1,(c->conn_RecvMax >> 1)))
        SendFlags(c, RDPF_ACK);
    else
        c->conn_Flags |= CONN_UNSENTACK;
    Permit();


    /*
     * Go through the list, and return all packets that are now between the
     * range of c->conn_SeqDelivered and c->conn_Current
     */

    for (pack = (c->conn_SeqDelivered + 1); pack <= c->conn_Current; pack++)
    {
        struct Buffer *bb;
        struct Buffer *tmpb;
        bb = (struct Buffer *) c->conn_DataListR.mlh_Head;
        while (bb->buff_link.mln_Succ)
        {
            struct rdphdr *rr;
            struct iphdr *ip;
            struct BuffEntry *bex;
            bex = (struct BuffEntry *) bb->buff_list.mlh_Head;
            ip = (struct iphdr *) ((ULONG) bex->be_data + (ULONG) bex->be_offset);
            rr = (struct rdphdr *) ((ULONG) ip + (ULONG) sizeof(struct iphdr));
            if (rr->rdp_SeqNum < pack)  /* Free any that're redundant */
            {
                tmpb = (struct Buffer *) bb->buff_link.mln_Pred;
                Remove((struct Node *) &(bb->buff_link));
                FreeBuffer(bb);
                bb = tmpb;
            }
            else
            {
                if (rr->rdp_SeqNum == pack) /* Return the proper packet
                                             * to the caller */
                {
                    tmpb = (struct Buffer *) bb->buff_link.mln_Pred;        /* Pull it safely from
                                                                             * the list */
                    Remove((struct Node *) & (bb->buff_link));
                    c->conn_SeqDelivered = pack;    /* Update the largest
                                                     * delivered # */
                    bex->be_offset = (sizeof(struct rdphdr) + sizeof(struct iphdr));
                    bex->be_length -= bex->be_offset;
                    if (c->conn_DataIn)
                    {
                        ObtainSemaphore(&nb->RDPCLSemaphore);
                        (*c->conn_DataIn) (c, bb);  /* Send it back to the
                                                     * opener */
                        ReleaseSemaphore(&nb->RDPCLSemaphore);
                        break; /* NEW */
                    }
                    bb = tmpb;
                }
            }
            bb = (struct Buffer *) bb->buff_link.mln_Succ;
        }
    }
    ReleaseSemaphore(&(c->conn_ReadListSemaphore));
}

/*------------------------------------------------------------------------*/

/*
 * UWORD FindSequential(connection)
 *
 * Finds the top sequence # of packets we have (in sequence) in the list
 *
 */

UWORD FindSequential(c)
struct RDPConnection *c;
{

    UWORD curseq;
    struct Buffer *b;

    curseq = c->conn_SeqDelivered + 1;

    /* Search the list for a packet with this sequence # */
    if (!c->conn_DataListR.mlh_Head->mln_Succ)
    {
        return (curseq);
    }

    while (TRUE)
    {
        b = (struct Buffer *) c->conn_DataListR.mlh_Head;
        while (b->buff_link.mln_Succ)
        {
            struct BuffEntry *be;
            struct iphdr *ip;
            struct rdphdr *r;
            be = (struct BuffEntry *) b->buff_list.mlh_Head;
            ip = (struct iphdr *) ((ULONG) be->be_data + (ULONG) be->be_offset);
            r = (struct rdphdr *) ((ULONG) ip + (ULONG) sizeof(struct iphdr));
            if (r->rdp_SeqNum == curseq)
            {
                curseq++;
                break;
            }
            b = (struct Buffer *) b->buff_link.mln_Succ;
        }
        if (!b->buff_link.mln_Succ)
            break;
    }
    curseq--;
    return (curseq);
}

/*------------------------------------------------------------------------*/

/*
 * connection = OpenActive(IP Address, port #, DataIn, Status, signal);
 *
 * Open an active RDP connection
 *
 */

struct RDPConnection *OpenActive(address, port, DataIn, Status)
ULONG address;
ULONG port;
RDP_DATA DataIn;
RDP_STATUS Status;
{
    register struct NBase *nb = gb;
    struct sanadev *sd;
    struct RDPConnection *c;
    struct NIPCRoute *route;

    c = (struct RDPConnection *) AllocMem(sizeof(struct RDPConnection), MEMF_CLEAR | MEMF_PUBLIC);
    if (!c)
        return ((struct RDPConnection *) 0L);

    c->conn_WakeBit = -1;
    c->conn_DataIn = DataIn;
    c->conn_Status = Status;
    /*
     * c->conn_WakeBit = signalbit; c->conn_WakeTask = FindTask(0L);
     */
    c->conn_State = STATE_SENT;

    c->conn_TheirAddress = address;
    c->conn_TheirPort = port;
    c->conn_LastConnTime = CONNTIME;
    route = Route(address, TRUE);

    if(!route)
    {
        FreeMem(c,sizeof(struct RDPConnection));
        return((struct RDPConnection *)NULL);
    }

    c->conn_OurAddress = route->nr_Device->sanadev_IPAddress;
    c->conn_RecvMax = RDPFindWindow(route->nr_Device);

    if (route->nr_Device->sanadev_BPS < 1000000)
        c->conn_RecvMaxSize = ((route->nr_Device->sanadev_MTU)-(sizeof(struct iphdr)+sizeof(struct rdphdr)));
    else
        c->conn_RecvMaxSize = ((route->nr_Device->sanadev_MTU)-(sizeof(struct iphdr)+sizeof(struct rdphdr)))*3;

    Forbid();
    c->conn_OurPort = gb->RDPPorts++;
    if (gb->RDPPorts == 0)
        gb->RDPPorts = 1024;
    Permit();

    InitSemaphore(&c->conn_ReadListSemaphore);
    InitSemaphore(&c->conn_WriteListSemaphore);
    InitSemaphore(&c->conn_InSema);
    NewList((struct List *) & c->conn_DataListR);
    NewList((struct List *) & c->conn_DataListW);
    NewList((struct List *) & c->conn_InList);  /* for amp */


    ObtainSemaphore(&nb->RDPCLSemaphore);
    AddTail((struct List *) & nb->RDPConnectionList, (struct Node *) & (c->conn_link));
    ReleaseSemaphore(&nb->RDPCLSemaphore);

    if (route)
    {
        sd = route->nr_Device;  /* Find device it goes out on  ... */

        c->conn_InitialBPS = DivSASSucks(sd->sanadev_BPS,10);

        if (sd->sanadev_BPS < 655360) /* If greater, time is trivial */
            if (sd->sanadev_BPS > 65535) /* handle special cased, so we generate DIVS/DIVU not SAS divide code */
            {                            /* If divisor > 16 bits, div by 16 first */
                UWORD bps;
                bps = (UWORD) ((ULONG) sd->sanadev_BPS >> 4);
                if (bps)
                    c->conn_TimeoutDelay = (UWORD) ( (ULONG)  40960 / bps ) < 1;
            }
            else
                if (sd->sanadev_BPS)
                    c->conn_TimeoutDelay = DivSASSucks(655360,sd->sanadev_BPS) < 1;
        if (sd->sanadev_MTU < 65536)
            c->conn_MTUDelay = (c->conn_TimeoutDelay >> 3); // div 3 because AMP dumps 8K chunks to RDP; time for 64K >> 3 = time for 8K
    }

    SendFlags(c, RDPF_SYN);
    ReadEClock( (struct EClockVal *)&c->conn_TimeStamp);


    return (c);
}


/*------------------------------------------------------------------------*/

/*
 * connection = OpenPassive(port);
 *
 * struct RDPConnection *OpenPassive(UWORD);
 *
 * port - either the port number you require, or -1 to allow one to be picked
 * for you.
 */

struct RDPConnection *OpenPassive(reqport, DataIn, Status)
ULONG reqport;
RDP_DATA DataIn;
RDP_STATUS Status;
{
        register struct NBase *nb = gb;
    struct RDPConnection *c;
    c = (struct RDPConnection *) AllocMem(sizeof(struct RDPConnection), MEMF_CLEAR | MEMF_PUBLIC);
    if (!c)
        return ((struct RDPConnection *) 0L);

    if (reqport == (ULONG) - 1)
    {
        Forbid();
        reqport = gb->RDPPorts++;
        if (gb->RDPPorts == 0)
            gb->RDPPorts = 1024;
        Permit();
    }

    c->conn_WakeBit = -1;
    c->conn_DataIn = DataIn;
    c->conn_Status = Status;
    c->conn_State = STATE_LISTEN;
    c->conn_RecvMax = MAXSEGS;
    c->conn_RecvMaxSize = MAXSIZE;
    c->conn_OurAddress = ((struct sanadev *) gb->DeviceList.lh_Head)->sanadev_IPAddress;   /* Our address is the
                                                                                         * 1st of the bunch */
    c->conn_OurPort = reqport;
    /*
     * c->conn_WakeBit = signalbit; c->conn_WakeTask = FindTask(0L);
     */
    c->conn_LastConnTime = CONNTIME;

    InitSemaphore(&c->conn_ReadListSemaphore);
    InitSemaphore(&c->conn_InSema);
    InitSemaphore(&(c->conn_WriteListSemaphore));
    NewList((struct List *) & c->conn_DataListR);
    NewList((struct List *) & c->conn_DataListW);
    NewList((struct List *) & c->conn_InList);  /* for amp */

    ObtainSemaphore(&nb->RDPCLSemaphore);
    AddTail((struct List *) & nb->RDPConnectionList, (struct Node *) & (c->conn_link));
    ReleaseSemaphore(&nb->RDPCLSemaphore);

    return (c);

}

/*------------------------------------------------------------------------*/

BOOL rdp_output_lame(conn, data, len)
struct RDPConnection *conn;
UBYTE *data;
ULONG len;
{
    struct Buffer *b;

    if(b = AllocBuffer(len))
    {
        CopyToBuffer(b,data,len);
        if(rdp_output(conn, b))
	{
	    return TRUE;
	}
	else
	{
	    FreeBuffer(b);
	    return FALSE;
	}
    }
    else
        return(FALSE);
}

BOOL rdp_output(conn, b)
struct RDPConnection *conn;
struct Buffer *b;              /* pointer to the data to packetize */
{

    register struct NBase *nb = gb;
    struct BuffEntry *be;
    struct rdphdr *r;
    UBYTE *data;
    UWORD thisseq;
    ULONG length;
    struct retransmit *rexmit;

    if (conn->conn_State != STATE_OPEN)
        return (FALSE);

    length = DataLength(b);

    be = AllocBuffEntry(sizeof(struct rdphdr));
    if(!be)
    {
        return(FALSE);
    }
    be->be_length = sizeof(struct rdphdr);

    r = (struct rdphdr *) be->be_data;
    AddHead((struct List *)&b->buff_list,(struct Node *)be);

    r->rdp_Flags = RDPF_ACK;
    r->rdp_Version = 2;
    r->rdp_HeaderLength = sizeof(struct rdphdr);
    r->rdp_SrcPort = conn->conn_OurPort;
    r->rdp_DestPort = conn->conn_TheirPort;

    Forbid();
    thisseq = r->rdp_SeqNum = conn->conn_SendNxt++;
    conn->conn_Flags &= ~CONN_UNSENTACK;
    Permit();


    r->rdp_AckNum = conn->conn_Current;
    conn->conn_LastAcked = conn->conn_Current;
    r->rdp_DataLength = length;
    r->rdp_Checksum = 0;
    r->rdp_Checksum = CalcChecksumBuffer(b);

    b->buff_refcnt++;   /* Make sure no one frees this buffer on us...*/

    rexmit = (struct retransmit *) AllocMem(sizeof(struct retransmit), MEMF_PUBLIC | MEMF_CLEAR);

    if (!rexmit)
    {
        b->buff_refcnt--;
        conn->conn_State = STATE_TIMEOUT; /* Close the connection; the connection timeout routine will do the dirty work. */
        return FALSE;
    }

#ifndef NOBUFFSHARING
    rexmit->xmit_data = b /* clone */ ;
#else
    rexmit->xmit_data = CloneBuffer(b);
#endif
    rexmit->xmit_countdown = TIMEOUT + conn->conn_MTUDelay;
    rexmit->xmit_retrycount = RETRIES;
    rexmit->xmit_ToAddress = conn->conn_TheirAddress;
    rexmit->xmit_seqnum = thisseq;

    ip_sendsrc(b, conn->conn_OurAddress, conn->conn_TheirAddress, 26);    /* Send the packet off to IP */

    ObtainSemaphore(&nb->RDPCLSemaphore);
    ObtainSemaphore(&conn->conn_WriteListSemaphore);
    AddTail((struct List *) & conn->conn_DataListW, (struct Node *) rexmit);
    ReleaseSemaphore(&conn->conn_WriteListSemaphore);
    ReleaseSemaphore(&nb->RDPCLSemaphore);

    return (TRUE);
}

/*------------------------------------------------------------------------*/

/*
 * CloseConnection(connection)
 *
 * Begins the process of closing a connection.
 *
 */

void CloseConnection(c)
struct RDPConnection *c;
{
    register struct NBase *nb = gb;
    struct Buffer *bt;

    ObtainSemaphore(&nb->RDPCLSemaphore);

    if (c->conn_State != STATE_CLOSE)
    {
        SendFlags(c, RDPF_RST);
        c->conn_State = STATE_CLOSE;
    }

    Remove((struct Node *) c);

    /* Free any queued reads */
    ObtainSemaphore(&(c->conn_ReadListSemaphore));
    while (bt = (struct Buffer *) RemHead((struct List *) & (c->conn_DataListR)))
    {
        FreeBuffer(bt);
    }
    ReleaseSemaphore(&(c->conn_ReadListSemaphore));

    ObtainSemaphore(&(c->conn_InSema));
    while (bt = (struct Buffer *) RemHead((struct List *) &(c->conn_InList)))
    {
        FreeBuffer(bt);
    }
    ReleaseSemaphore(&(c->conn_InSema));

    /* Free any retransmit queued writes */
    ObtainSemaphore(&(c->conn_WriteListSemaphore));
    while (!IsListEmpty((struct List *) & c->conn_DataListW))
    {
        struct retransmit *rt;
        struct Buffer *bt;
        rt = (struct retransmit *) RemHead((struct List *) & (c->conn_DataListW));
        bt = rt->xmit_data;
        bt->buff_refcnt--;
        FreeBuffer(bt);
        FreeMem(rt, sizeof(struct retransmit));
    }
    ReleaseSemaphore(&(c->conn_WriteListSemaphore));

    FreeMem(c, sizeof(struct RDPConnection));

    ReleaseSemaphore(&nb->RDPCLSemaphore);

}
/*------------------------------------------------------------------------*/


