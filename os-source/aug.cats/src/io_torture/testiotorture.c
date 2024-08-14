;/* testiotorture.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 testiotorture.c
Blink FROM LIB:c.o,testiotorture.o TO testiotorture LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <devices/console.h>
#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }		/* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }	/* really */
#endif

/* Note - using two character <CSI> ESC[.  Hex 9B could be used instead */ 
#define RESETCON  "\033c"
#define CURSOFF   "\033[0 p"
#define CURSON    "\033[ p"
#define DELCHAR   "\033[P"

/* SGR (set graphic rendition) */
#define COLOR02   "\033[32m"
#define COLOR03   "\033[33m"
#define ITALICS   "\033[3m"
#define BOLD      "\033[1m"
#define UNDERLINE "\033[4m"
#define NORMAL    "\033[0m"

/* our functions */
void cleanexit(UBYTE *,LONG);
void cleanup(void);
BYTE OpenConsole(struct IOStdReq *,struct IOStdReq *, struct Window *);
void CloseConsole(struct IOStdReq *);
void QueueRead(struct IOStdReq *, UBYTE *);
UBYTE ConGetChar(struct MsgPort *, UBYTE *);
LONG ConMayGetChar(struct MsgPort *, UBYTE *);
void ConPuts(struct IOStdReq *, UBYTE *);
void ConWrite(struct IOStdReq *, UBYTE *, LONG);
void ConPutChar(struct IOStdReq *, UBYTE);

struct NewWindow nw = {
        10, 10,                           /* starting position (left,top) */
        620,180,                          /* width, height */
        -1,-1,                            /* detailpen, blockpen */
        CLOSEWINDOW,                      /* flags for idcmp */
        WINDOWDEPTH|WINDOWSIZING|
        WINDOWDRAG|WINDOWCLOSE|
        SMART_REFRESH|ACTIVATE,           /* window flags */
        NULL,                             /* no user gadgets */
        NULL,                             /* no user checkmark */
        "Console Test",                   /* title */
        NULL,                             /* pointer to window screen */
        NULL,                             /* pointer to super bitmap */
        100,45,                           /* min width, height */
        640,200,                          /* max width, height */
        WBENCHSCREEN                      /* open on workbench screen */
        };
 

/* Opens/allocations we'll need to clean up */
struct Library  *IntuitionBase = NULL;
struct Window   *win = NULL;
struct IOStdReq *writeReq = NULL;    /* I/O request block pointer */
struct MsgPort  *writePort = NULL;   /* replyport for writes      */   
struct IOStdReq *readReq = NULL;     /* I/O request block pointer */
struct MsgPort  *readPort = NULL;    /* replyport for reads       */   
BOOL OpenedConsole = FALSE;

BOOL FromWb;

void main(int argc, char **argv)
    {
    struct IntuiMessage *winmsg;
    ULONG signals, conreadsig, windowsig;
    LONG lch;
    SHORT InControl = 0;
    BOOL Done = FALSE;
    UBYTE ch, ibuf;
    UBYTE obuf[200];
    BYTE error;

    FromWb = (argc==0L) ? TRUE : FALSE;

    if(!(IntuitionBase=OpenLibrary("intuition.library",0)))
         cleanexit("Can't open intuition\n",RETURN_FAIL);

    /* Create reply port and io block for writing to console */
    if(!(writePort = CreatePort("RKM.console.write",0)))
         cleanexit("Can't create write port\n",RETURN_FAIL);

    if(!(writeReq = (struct IOStdReq *)
	CreateExtIO(writePort,(LONG)sizeof(struct IOStdReq))))
         cleanexit("Can't create write request\n",RETURN_FAIL);

    /* Create reply port and io block for reading from console */
    if(!(readPort = CreatePort("RKM.console.read",0)))
         cleanexit("Can't create read port\n",RETURN_FAIL);

    if(!(readReq = (struct IOStdReq *)
	CreateExtIO(readPort,(LONG)sizeof(struct IOStdReq))))
         cleanexit("Can't create read request\n",RETURN_FAIL);

    /* Open a window */
    if(!(win = OpenWindow(&nw)))
         cleanexit("Can't open window\n",RETURN_FAIL);

    /* Now, attach a console to the window */
    if(error = OpenConsole(writeReq,readReq,win))
         cleanexit("Can't open console.device\n",RETURN_FAIL);
    else OpenedConsole = TRUE;


    QueueRead(readReq,&ibuf); /* send the first console read request */
    QueueRead(readReq,&ibuf); /* reuse request with SendIO while it's in use */
    ConPutChar(readReq,'x');  /* reuse request with DoIO while it's in use */

    /* We always have an outstanding queued read request
     * so we must abort it if it hasn't completed,
     * and we must remove it.
     */
    if(!(CheckIO(readReq)))  AbortIO(readReq);
    WaitIO(readReq);     /* clear it from our replyport */

    cleanup();
    exit(RETURN_OK);
    }

void cleanexit(UBYTE *s,LONG n)
    {
    if(*s & (!FromWb)) printf(s);
    cleanup();
    exit(n);
    }

void cleanup()
    {
    if(OpenedConsole) CloseConsole(writeReq);
    if(readReq)       DeleteExtIO(readReq);
    if(readPort)      DeletePort(readPort);
    if(writeReq)      DeleteExtIO(writeReq);
    if(writePort)     DeletePort(writePort);
    if(win)           CloseWindow(win);
    if(IntuitionBase) CloseLibrary(IntuitionBase);
    }


/* Attach console device to an open Intuition window.
 * This function returns a value of 0 if the console 
 * device opened correctly and a nonzero value (the error
 * returned from OpenDevice) if there was an error.
 */
BYTE OpenConsole(writereq, readreq, window)
struct IOStdReq *writereq;
struct IOStdReq *readreq;
struct Window *window;
    {
    BYTE error;
   
    writereq->io_Data = (APTR) window;
    writereq->io_Length = sizeof(struct Window);
    error = OpenDevice("console.device", 0, writereq, 0);
    readreq->io_Device = writereq->io_Device; /* clone required parts */
    readreq->io_Unit   = writereq->io_Unit;
    return(error);   
    }

void CloseConsole(struct IOStdReq *writereq)
    {
    CloseDevice(writereq);
    }

/* Output a single character to a specified console 
 */ 
void ConPutChar(struct IOStdReq *writereq, UBYTE character)
    {
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR)&character;
    writereq->io_Length = 1;
    DoIO(writereq);
    /* command works because DoIO blocks until command is done
     * (otherwise ptr to the character could become invalid)
     */
    }


/* Output a stream of known length to a console 
 */ 
void ConWrite(struct IOStdReq *writereq, UBYTE *string, LONG length)
    {
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR)string;
    writereq->io_Length = length; 
    DoIO(writereq);
    /* command works because DoIO blocks until command is done
     * (otherwise ptr to string could become invalid in the meantime)
     */
   }


/* Output a NULL-terminated string of characters to a console 
 */ 
void ConPuts(struct IOStdReq *writereq,UBYTE *string)
    {
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR)string;
    writereq->io_Length = -1;  /* means print till terminating null */
    DoIO(writereq);
    }

/* Queue up a read request to console, passing it pointer
 * to a buffer into which it can read the character
 */
void QueueRead(struct IOStdReq *readreq, UBYTE *whereto)
   {
   readreq->io_Command = CMD_READ;
   readreq->io_Data = (APTR)whereto;
   readreq->io_Length = 1;
   SendIO(readreq);
   }

/* Check if a character has been received.
 * If none, return -1 
 */
LONG ConMayGetChar(struct MsgPort *msgport, UBYTE *whereto)
    {
    register temp;
    struct IOStdReq *readreq;
 
    if (!(readreq = (struct IOStdReq *)GetMsg(msgport))) return(-1);
    temp = *whereto;                /* get the character */
    QueueRead(readreq,whereto);     /* then re-use the request block */
    return(temp);
    }

/* Wait for a character
 */
UBYTE ConGetChar(struct MsgPort *msgport, UBYTE *whereto)
    {
    register temp;
    struct IOStdReq *readreq;

    WaitPort(msgport);
    readreq = (struct IOStdReq *)GetMsg(msgport);
    temp = *whereto;               /* get the character */
    QueueRead(readreq,whereto);    /* then re-use the request block*/
    return((UBYTE)temp);
    }
