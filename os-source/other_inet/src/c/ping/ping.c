/* ---------------------------------------------------------------------------------
 * PING.C
 *
 * $Locker: bj $
 *
 * $Id: ping.c,v 1.3 93/01/15 13:53:01 bj Exp Locker: bj $
 *
 * $Revision: 1.3 $
 *
 * $Header: AS225:src/c/ping/RCS/ping.c,v 1.3 93/01/15 13:53:01 bj Exp Locker: bj $
 *
 * $Log:	ping.c,v $
 * Revision 1.3  93/01/15  13:53:01  bj
 * Binary 3.4
 * 
 * Fixes bug where 'ping -l' calculation of average packet trip time
 * could generate a negative number.
 * 
 * Fixes bug where hitting control-C during certain code areas could cause massive
 * losses of memory and an eventual crash.
 * 
 * Fixes bug where user could specify a packet size that was too large. This would
 * crash the machine. User defined packet size limited to 4096 (larger than both
 * BSD and AT&T ping maximums.)
 * 
 *
 *-----------------------------------------------------------------------------------
 */

/* sample internet ping client */

/* This is an advanced sockets program that shows how to use */
/* raw sockets, async I/O, and timers.  This code will only  */
/* work with AmigaDOS 2.0.  To run under 1.3, simply replace */
/* ReadArgs() with your own parsing code and use another     */
/* timer in place of the ReadEClock() calls.                 */

/* to compile with SAS C , type
"lc -cfist -v -inetinc: -fi -Lm ping.c"                      */

/* Written by Martin Hunt, Commodore-Amiga, June 1991        */

/* Parts of this program were based on code by Mike Muuss    */

/* This code is placed in the public domain */

// #define DEBUG 1
#ifdef DEBUG
	#define D(x) printf(x)
#else
	#define D(x) ;
#endif

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <devices/timer.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/timer_protos.h>

/* shared library stuff */
#include <sys/types.h>
#include <ss/socket.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>


/* get version and revision information */
#include "ping_rev.h"

/* This is used with the templates to cause the version string to be */
/* stored in the command.  It depends upon the command having a #include */
/* of the appropriate version file to provide the value for this string */
#define CMDREV  "\0$VER: " VSTRING

/* template for ReadArgs() */
#define TEMPLATE    "-L/S,HOST/A,SIZE/N,COUNT/N" CMDREV
#define OPT_L       0
#define OPT_HOST    1
#define OPT_SIZE    2
#define OPT_COUNT   3
#define OPT_MAX     4

#define MAXPACKET   4096    /* max packet size */
struct Library *SockBase ;
struct Library *TimerBase ;
UBYTE buffer[MAXPACKET];
struct MsgPort *timerport1=NULL;
struct timerequest *timer1=NULL;
u_long ntransmitted = 0, nreceived = 0;
u_long tsum = 0;
u_long tmin = 999999;
u_long tmax = 0;
u_long e_freq;
UWORD ident;
long size,count;
BOOL loop = (BOOL)0;
long return_code = RETURN_ERROR;
int getout = 0 ;

/* prototypes */
UWORD in_cksum(UWORD *addr, int len);
ULONG create_timers(void);
void send_icmp(int s, struct sockaddr_in *to);
char *pr_type( int t );
void read_icmp(int s );
void pr_pack( int cc, struct sockaddr_in *from );

char *inet_ntoa( u_long ) ;
void exit(int) ;

int CXBRK(void)
{
	getout = 1 ;
	return(0) ;
}

void main (void)
{
    char *host, *hostname;
    long timerevent, netevent, event;
    struct RDargs *rdargs;
    int s;
    struct sockaddr_in to;
    struct hostent *hp;
    struct protoent *proto;
    long true = 1;
    char *toaddr = NULL;
    long opts[OPT_MAX];
    UWORD abort = 0;
    u_long ial ;

    memset((char *)opts, 0, sizeof(opts));
    rdargs = ReadArgs(TEMPLATE, opts, NULL);
    if( rdargs == NULL)
    {
        exit(20) ;
    }

    if ((SockBase = OpenLibrary( "inet:libs/socket.library", 1L ))==NULL) {
        printf("Opening of socket library failed.\n");
        goto exit1;
    }

    /* initialize for 5 sockets */
    setup_sockets( 5, &errno );

    /* process arguments */
    if(opts[OPT_COUNT])
        count = *(long *)opts[OPT_COUNT];
    else
        count = 100;

    if(opts[OPT_L])
        loop = (BOOL)opts[OPT_L];
    else
        count = 1;

    if (!(host = (char *)opts[OPT_HOST]))
    	goto exit2;

    if(opts[OPT_SIZE])
    {
        size = max( *(long *)opts[OPT_SIZE], 64);
        if( size > MAXPACKET )
        	size = MAXPACKET ;
        	
        printf("size arg = %ld\n", size) ;
    }
    else
        size = 64;

    /* zero structure */
    memset( (char *)&to, '\0',sizeof(struct sockaddr_in) );
	if(getout)
		goto exit2 ;
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = inet_addr(host);
    if (to.sin_addr.s_addr == INADDR_NONE) {
        hp = gethostbyname(host);
        if (!hp && getout == 0) {
            printf("Error finding host %s\n",host);
            goto exit2;
        }
		if(getout)
			goto exit2 ;
        to.sin_family = hp->h_addrtype;
        memcpy((char *)&to.sin_addr, hp->h_addr, hp->h_length);
        hostname = hp->h_name;
        ial = (long)to.sin_addr.s_addr ;
        
        toaddr = inet_ntoa(ial);
    } else
        hostname = host;

	if(getout)
		goto exit2 ;

    /* ident is used in the ICMP packet so we can tell that its ours */
    ident = (UWORD)((long)FindTask(0L) & 0xFFFF);

    if ((proto = getprotobyname("icmp")) == NULL) {
        printf("icmp: unknown protocol\n");
        goto exit2;
    }
	if(getout)
		goto exit2 ;

    /* open an ICMP socket */
    if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
        printf("ping: socket: %s\n",strerror(errno));
        goto exit2;
    }
	if(getout)
		goto exit3 ;    /* switch to exit 3 as we now have a socket */

    /* set socket to ASYNC so we will be signaled when packets arrive */
    if(s_ioctl(s, FIOASYNC, (char *)&true) < 0){
        printf("ping: s_ioctl: %s\n",strerror(errno));
        goto exit3;
    }
    /* set socket to nonblocking */
    if(s_ioctl(s, FIONBIO, (char *)&true) < 0){
        printf("ping: s_ioctl: %s\n",strerror(errno));
        goto exit3;
    }

	if(getout)
		goto exit3 ;

    if(loop) {
        printf("PING %s", hostname);
        if (toaddr)
            printf(" (%s)", toaddr);
        printf(": %ld data bytes\n", size);
    }

    /* setup timer.  timer1 is a 1 second timer for sending packets */

    if(create_timers()) {
        printf("Error setting up timer\n");
        goto exit3;
    }

	if(getout)
		goto exit2 ;

    timerevent = 1L << timerport1->mp_SigBit;
    netevent = 1L << s_getsignal(SIGIO);

    do {
        /* send ICMP echo request */
        send_icmp(s,&to);
        for(;;) {
            /* wait for a response, timeout, or ^C */
            event = Wait(netevent | timerevent | SIGBREAKF_CTRL_C);
            if(event & netevent) {
                read_icmp(s);
                SetSignal(0L,netevent);
                if(!loop) {
                    AbortIO((struct IORequest *)timer1);
                    WaitIO((struct IORequest *)timer1);
                    abort++;
                    break;
                }

            }
            if(event & SIGBREAKF_CTRL_C) {
                AbortIO((struct IORequest *)timer1);
                WaitIO((struct IORequest *)timer1);
                abort++;
                break;
            }
            if(event & timerevent) {
                AbortIO((struct IORequest *)timer1);
                WaitIO((struct IORequest *)timer1);
                SetSignal(0L, timerevent);
                break;
            }
        }

    } while (!abort && (ntransmitted != count) );

    if(loop) {
        printf("\n----%s PING Statistics----\n", hostname );
        printf("%d packets transmitted, ", ntransmitted );
        printf("%d packets received, ", nreceived );
        if (ntransmitted)
            printf("%d%% packet loss",
            (int) (((ntransmitted-nreceived)*100) / ntransmitted ) );
        printf("\n");
        if (nreceived)
        {
            printf("round-trip (ms)  min/avg/max = %#2.2lf/%#2.2lf/%#2.2lf\n",
                (double)tmin/(double)e_freq*1000.0,
                (double)(tsum*1000.0) / (double)(nreceived*((double)e_freq)) ,
                (double)tmax/(double)e_freq*1000.0 );
        }
            
    } else
        if(nreceived)
            printf("%s is alive!\n",host);

exit3:
    fflush(stdout);
    if (s)
        s_close(s);

    if(timerport1)
        DeleteMsgPort(timerport1);
    if(timer1)
        DeleteIORequest(timer1);
exit2:
    cleanup_sockets();
    CloseLibrary(SockBase);
exit1:
    if(rdargs)
        FreeArgs(rdargs);

    exit(return_code);
}

/*
 *          I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
UWORD in_cksum(UWORD *addr, int len)
{
    register int nleft = len;
    register UWORD *w = addr;
    register UWORD answer;
    register int sum = 0;
    UWORD odd_byte = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        *(u_char *)(&odd_byte) = *(u_char *)w;
        sum += odd_byte;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}


ULONG create_timers(void)
{
    timerport1 = CreateMsgPort();
    if(timerport1 == (struct MsgPort *)0)
        return (1);

    timer1 = (struct timerequest *)CreateIORequest(timerport1,(long)sizeof(struct timerequest));
    if(timer1 == (struct MsgPort *)0)
        return(1);

    if(OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)timer1, 0L) != 0L)
        return(1);

    /* set up TimerBase so we can use ReadEClock() */
    TimerBase = (struct Library *)timer1->tr_node.io_Device;

    return(0);
}

/* send an ICMP echo request */
/* the timestamp is written in the first 8 bytes of data */

void send_icmp(int s, struct sockaddr_in *to)
{
    UBYTE *datap;
    struct icmp *icp = (struct icmp *) buffer;
    int i;

    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = ntransmitted++;
    icp->icmp_id = ident;

    /* start the 1 second timer */
    timer1->tr_node.io_Command = TR_ADDREQUEST;
    timer1->tr_time.tv_secs = 1L;
    timer1->tr_time.tv_micro = 0L;
    SendIO((struct IORequest *)timer1);


    /* get system time and put it in first 8 bytes of data field */
    /* note that we will be timing filling the packet with data  */
    /* and checksumming it.                                      */

    ReadEClock((struct EClockVal *)(buffer+8));

    datap = (UBYTE *)(buffer+16);   /* skip 8 for header and 8 for timestamp */

    /* fill packet with data */
    for( i=16; i<size; i++)
        *datap++ = i;

    /* Compute ICMP checksum */
    icp->icmp_cksum = in_cksum( (UWORD *)icp, size );

    /* send it */
    i = sendto( s, buffer, size, 0, (struct sockaddr *)to, sizeof(struct sockaddr) );

    if( i < 0 || i != size )  {
        if( i<0 )
            printf("sendto: %s\n",strerror(errno));
        printf("ping: wrote %ld chars, ret=%ld\n",size, i );
    }

}

void read_icmp(int s)
{
    struct sockaddr_in from;
    int fromlen = sizeof (from);
    int cc;

    if ( (cc=recvfrom(s, buffer, MAXPACKET, 0, (struct sockaddr *)&from, &fromlen)) < 0) {
        if( errno == EINTR || errno == EWOULDBLOCK){
            return ;
        }
        printf("ping: recvfrom: %s\n",strerror(errno));
        return ;
    }
    pr_pack( cc, &from );
}


/*
 *          P R _ P A C K
 *
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
void pr_pack( int cc, struct sockaddr_in *from )
{
    struct ip *ip;
    register struct icmp *icp;
    register long *lp = (long *) buffer;
    register int i;
    struct EClockVal intime, *outtime;
    int hlen;
    u_long triptime;


    e_freq = ReadEClock(&intime);
    from->sin_addr.s_addr = ntohl( from->sin_addr.s_addr );
    ip = (struct ip *)buffer;

    /* get the IP header length */
    hlen = ip->ip_hl << 2;

    if (cc < hlen + ICMP_MINLEN) {
        printf("packet too short (%d bytes) from %s\n", cc,
                inet_ntoa(ntohl(from->sin_addr.s_addr)));
        return;
    }
    cc -= hlen;

    /* the ICMP packet starts after the IP header */
    icp = (struct icmp *)(buffer + hlen);
    if( icp->icmp_type != ICMP_ECHOREPLY )  {
        printf("%d bytes from %s: ", cc,
            inet_ntoa(ntohl(from->sin_addr.s_addr)));
        printf("icmp_type=%d (%s)\n",
            icp->icmp_type, pr_type(icp->icmp_type) );
        for( i=0; i<12; i++)
            printf("x%2.2x: x%8.8x\n", i*sizeof(long), *lp++ );
        printf("icmp_code=%d\n", icp->icmp_code );
        return;
    }
    if( icp->icmp_id != ident )
        return;         /* 'Twas not our ECHO */

    return_code = RETURN_OK;

    outtime = (struct EClockVal *)(&icp->icmp_data[0]);
    triptime = intime.ev_lo-outtime->ev_lo;
//    printf("triptime = %ld\n", triptime) ;

    if(loop) {
        printf("%d bytes from %s: ", cc,inet_ntoa(ntohl(from->sin_addr.s_addr)));
        printf("icmp_seq=%d. ", icp->icmp_seq );
        printf("time=%#2.2lf ms\n", (double)triptime/(double)e_freq*1000.0 );
    }

    /* update total, max, and min times */
    tsum += triptime;
    tmax = max(tmax,triptime);
    tmin = min(tmin,triptime);

    nreceived++;
}

/*
 *          P R _ T Y P E
 *
 * Convert an ICMP "type" field to a printable string.
 */
char *pr_type( int t )
{
    static char *ttab[] = {
        "Echo Reply",
        "ICMP 1",
        "ICMP 2",
        "Dest Unreachable",
        "Source Quench",
        "Redirect",
        "ICMP 6",
        "ICMP 7",
        "Echo",
        "ICMP 9",
        "ICMP 10",
        "Time Exceeded",
        "Parameter Problem",
        "Timestamp",
        "Timestamp Reply",
        "Info Request",
        "Info Reply"
    };

    if( t < 0 || t > 16 )
        return("OUT-OF-RANGE");

    return(ttab[t]);
}
