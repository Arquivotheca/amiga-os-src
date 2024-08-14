
/*
 *  CHANNEL.H
 *
 *  DNET (c)Copyright 1988, Matthew Dillon, All Rights Reserved.
 *
 *  Channel structures for SCMD_* channel commands.
 */

#ifndef DNET_H
typedef unsigned char ubyte;
typedef unsigned short uword;
typedef unsigned long ulong;
typedef struct MsgPort PORT;
typedef struct IOStdReq IOR;
#endif

#define CSWITCH struct _CSWITCH
#define COPEN	struct _COPEN
#define CCLOSE	struct _CCLOSE
#define CACKCMD struct _CACKCMD
#define CEOFCMD struct _CEOFCMD
#define CIOCTL	struct _CIOCTL

CSWITCH {		/*  SWITCH current data channel */
    ubyte   chanh;
    ubyte   chanl;
};

COPEN { 		/*  OPEN port on channel	*/
    ubyte   chanh;
    ubyte   chanl;
    ubyte   porth;
    ubyte   portl;
    ubyte   error;	/*  error return 0=ok		*/
    ubyte   pri;
};

CCLOSE {		/*  CLOSE a channel		*/
    ubyte   chanh;
    ubyte   chanl;
};

CACKCMD {		/*  Acknowledge an open/close	    */
    ubyte   chanh;
    ubyte   chanl;
    ubyte   error;	/*  ERETRY ENOPORT ECLOSE1 ECLOSE2  */
    ubyte   filler;
};

CEOFCMD {		/*  Send [R/W] EOF		*/
    ubyte   chanh;
    ubyte   chanl;
    ubyte   flags;
    ubyte   filler;
};

CIOCTL {
    ubyte   chanh;	/* channel			*/
    ubyte   chanl;
    ubyte   cmd;	/* ioctl command		*/
    ubyte   valh;	/* ioctl value			*/
    ubyte   vall;
    ubyte   valaux;	/* auxillary field		*/
};

#define CIO_SETROWS	1	/* PTY's only                   */
#define CIO_SETCOLS	2	/* PTY's only                   */
#define CIO_STOP	3	/* any channel, flow control	*/
#define CIO_START	4	/* any channel, flow control	*/
#define CIO_FLUSH	5

#define CHAN	struct _CHAN

CHAN {
    PORT    *port;
    IOR     *ior;
    ubyte   state;
    ubyte   flags;
    char    pri;	/*  transmit priority	*/
};

