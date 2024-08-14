/* -----------------------------------------------------------------------
 * sys_amiga.c  telnet  bj
 *
 * Amiga specific routines
 *
 * $Locker:  $
 *
 * $Id: sys_amiga.c,v 1.1 91/01/15 17:59:21 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inetx/src/telnet/RCS/sys_amiga.c,v 1.1 91/01/15 17:59:21 bj Exp $
 *
 * $Log:	sys_amiga.c,v $
 * Revision 1.1  91/01/15  17:59:21  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#define DEBUG 0 

#if DEBUG != 0
#define DB(x,y)    printf("DEBUG: %s: %s\n",(x),(y)) ;
#else
#define DB(x,y)  ;
#endif


#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <socket.h>

#include "ring.h"
#include "defines.h"
#include "externs.h"
#include "types.h"

#include <libraries/dos.h>

	/* buffer that holds the control sequence from the console device */
static char conbuffer[128] ;  

static unsigned char lfonly[] = { '\x9b', '2','0', 'l' } ;

	/* pointer into the conbuffer[] array */
static char *cbptr = NULL ;
	
static fd_set ibits, obits, xbits;
static console_up;
int net;
void *IntuitionBase;

/* --------------------------------------- */
memset(register char *buf, register char val,register int len)
{
	while(len--)
		{
		*buf++ = val;
		}
}

/* --------------------------------------- */
memcpy(char *to, char *from, int len)
{
	bcopy(from, to, len);
}

/* --------------------------------------- */
TerminalSpecialChars(char c)
{
}

/* --------------------------------------- */
/*
 * netin	- if set, include net input in select
 * netout	- if set, include net output in select
 * netin	- if set, include net exceptions in select
 * ttyin	- if set, include tty input in event mask
 * ttyout	- if set, include tty output in event mask
 */

process_rings(int netin, int netout, int netex, int ttyin, int ttyout, int poll)
{
	static struct timeval TimeValue = { 0 };
	extern int cons_kybd_sigF;
	struct timeval *tvp;
	int returnValue = 0;
	register int c;
  	long event;

	event = 0L;
	if (netout) 
		{
		FD_SET(net, &obits);
		} 
	if (netin) 
		{
		FD_SET(net, &ibits);
		}
	if (netex) 
		{
		FD_SET(net, &xbits);
		}

	event |= cons_kybd_sigF;

	tvp = (poll == 0) ? 0 : &TimeValue;
	event |= SIGBREAKF_CTRL_C;
	c = selectwait(net+1, &ibits, &obits, &xbits, tvp, &event);

		/*
		 * Any urgent data?
		 */

	if (FD_ISSET(net, &xbits)) 
		{
		FD_CLR(net, &xbits);
		SYNCHing = 1;
		ttyflush(1);	/* flush already enqueued data */
		}

		/*
		 * Something to read from the network...
		 */

	if (FD_ISSET(net, &ibits)) 
		{
		int canread;

		FD_CLR(net, &ibits);
		canread = ring_empty_consecutive(&netiring);
		c = recv(net, netiring.supply, canread, 0);
		if (c < 0 && errno == EWOULDBLOCK) 
			{
			c = 0;
			} 
		else if (c <= 0) 
			{
			return -1;
			}
		if(netdata) 
			{
			Dump('<', netiring.supply, c);
			}
		if(c)
			{
			ring_supplied(&netiring, c);
			}

		returnValue = 1;
		}

	/* ==================================================================
	 * Something to read from the tty...
	 * =================================================================
	 */

	if(event & cons_kybd_sigF)
		{
		c = TerminalRead(ttyiring.supply, ring_empty_consecutive(&ttyiring));
#if DEBUG != 0	
		printf("c = %d\n",c) ;
#endif
		if (c < 0)
			{
			c = 0;
			} 
		else 
			{
			if((c == 0) && MODE_LOCAL_CHARS(globalmode))
				{
				*ttyiring.supply = termEofChar;
				c = 1;
				}
			if (c <= 0) 
				{
				return -1;
				}
			ring_supplied(&ttyiring, c);
			}
		returnValue = 1;		/* did something useful */
		}

	returnValue |= ttyflush(0);

	if (FD_ISSET(net, &obits)) 
		{
		FD_CLR(net, &obits);
		returnValue |= netflush();
		}

	return returnValue;
}

/* ============================================================
 */

void
NetNonblockingIO( int fd, int onoff)
{
	ioctl(fd, FIONBIO, (char *)&onoff);
}

void
sys_telnet_init( void )
{
	setconnmode();
	NetNonblockingIO(net, 1);
	if(SetSockOpt(net, SOL_SOCKET, SO_OOBINLINE, 1) == -1) 
		{
		perror("SetSockOpt");
		}
}

/* --------------------------------------- */
void
TerminalSaveState(void)
{
}

int
NetClose(int fd)
{
	return close(fd);
}

/* --------------------------------------- */

int
init_sys( void )
{
	if(!console_up)
		{
		console_init("telnet");
		console_up++;
		}

	console_write( (char *)lfonly, 4 ) ;

	FD_ZERO(&ibits);
	FD_ZERO(&obits);
	FD_ZERO(&xbits);
	errno = 0;
}

/* --------------------------------------- */
void
TerminalAutoFlush(void)
{
}

/* --------------------------------------- */
void
TerminalFlushOutput(void)
{
}

/* --------------------------------------- */
int
TerminalWrite(char *buf, int n)
{
	console_write(buf, n);
	return n;
}

/* ==================================================
 * Terminalread.
 *
 * 'maxnum' is the maximum number of bytes that can be
 * be stuffed into the ring buffer for the tty input.
 * This is calculated by the 'ring_empty_consecutive()'
 * call
 * ==================================================
 */
 
#define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 
 
 	/* amiga cursor control sequences */
 	/* (these are the escape sequences that will be fiund in the */
 	/* buffer after processing in this function !!! */
 	
unsigned char a_cr_left[]  = { '\x1b', '[', 'D' } ;
unsigned char a_cr_right[] = { '\x1b', '[', 'C' } ;
unsigned char a_cr_up[]    = { '\x1b', '[', 'A' } ;
unsigned char a_cr_down[]  = { '\x1b', '[', 'B' } ;
 
	/* vt100 cursor control sequences as defined in the */
unsigned char v_cr_left[]  = { '\x1b', 'O', 'D' } ;
unsigned char v_cr_right[] = { '\x1b', 'O', 'C' } ;
unsigned char v_cr_up[]    = { '\x1b', 'O', 'A' } ;
unsigned char v_cr_down[]  = { '\x1b', 'O', 'B' } ;

int
TerminalRead(char *buf, int maxnum )
{
	static char hbuf[128] ;
	static int bcnt = 0 ;
	static int incontrol = 0 ;
	int a ;
	unsigned char c ;
	int xx ;
	
#if DEBUG != 0
printf("terminalread/maxnum: %d\n", maxnum) ;
#endif

	c = (unsigned char)(console_getchar() & 0xff) ;

	if( incontrol )
		{
		hbuf[bcnt++] = c ;
		bcnt++ ;
				
			/* valid ESC sequence terminator? then end & send */
		if( ESC_SEQ_TERMINATOR(c) )
			{
			strncpy(buf, hbuf, bcnt ) ;
			a = bcnt - 1 ;
			if( strncmp(buf, a_cr_left, 3 ) == 0 )
				{
				strcpy( buf, v_cr_left ) ;
				a = 3 ;
				}
			else if( strncmp(buf, a_cr_right, 3 ) == 0 )
				{
				strcpy( buf, v_cr_right ) ;
				a = 3 ;
				}
			else if( strncmp(buf, a_cr_up, 3 ) == 0 )
				{
				strcpy( buf, v_cr_up ) ;
				a = 3 ;
				}
			else if( strncmp(buf, a_cr_down, 3 ) == 0 )
				{
				strcpy( buf, v_cr_down ) ;
				a = 3 ;
				}
			bcnt = 0 ;
			incontrol = 0 ;

#if DEBUG != 0
	printf("terminalread - return val = %d\n", a ) ;
#endif

			return( a ) ;
			}
		}
	else if( c == 0x9b )    /* start of a control sequence ? */
		{
		incontrol = 1 ;
		hbuf[bcnt] = 0x1b ;
		bcnt++ ;
		hbuf[bcnt] = '[' ;
		bcnt++ ;
#if DEBUG != 0
	printf("terminalread - return val = -1\n" ) ;
#endif
		return(-1) ;
		}
	else 
		{
		*buf = c ;
		bcnt = 0 ;
#if DEBUG != 0
	printf("terminalread - return val = 1\n" ) ;
#endif
		return( 1 ) ;
		}
}

/* ================================================== */


void
TerminalNewMode(int mode)
{
}

/* --------------------------------------- */

int
tprintf(char *fmt, unsigned a, unsigned b, unsigned c, unsigned d, 
                      unsigned e, unsigned f, unsigned g)
{
	static char buf[2048];
	int rtn;

	rtn = sprintf(buf, fmt, a, b, c, d, e, f, g);
	TerminalWrite(buf, strlen(buf));
	return rtn;
}

/* --------------------------------------- */
char *
gets(char *buf)
{
	char 	*p;

	for(p = buf; ; p++)
		{
		TerminalRead(p, 1);
		if(*p == '\r' || *p == '\n')
			{
			TerminalWrite("\r\n", 2);
			break;
			} 
		else 
			{
			TerminalWrite(p, 1);
			}
		}
	*p = 0;
	return buf;	
}

/* --------------------------------------- */
void
init_3270(void)
{
}

/* --------------------------------------- */
void
MyExit(int code)
{
	if(console_up)
		{
		console_up = 0;
		console_close();
		}
	exit(code);
}

/* --------------------------------------- */
void
SetIn3270(void)
{
}

/* --------------------------------------- */
main(int argc, char **argv)
{
	return tmain(argc, argv);
}

