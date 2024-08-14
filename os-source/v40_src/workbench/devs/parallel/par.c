/****************************************************************************/
/* Compile using -b0 -v; Link with grabresources.a, server.a and amiga.lib; */
/* Blink WITH parlink.lnk                                                   */
/****************************************************************************/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/errors.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <devices/timer.h>
#include <hardware/cia.h>
#include "parallel.h"

/* uncomment these two lines for debugging */
/*
extern struct Library *SysBase;
*/

/* comment these two lines for debugging */

#define SysBase pd->md_SysLib

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

extern struct CIA ciaa, ciab;		/* The absolute base of the hardware regs */

struct ParDev
{
    struct Library Lib;

	UBYTE	md_Flags;
	UBYTE	pad1;
	ULONG	md_SysLib;
	ULONG	md_SegList;

    struct MinList PAR_queue;		/* The queue of all pending I/O requests */
    APTR Misc;				/* Base of misc.resource allocated */
    struct Interrupt *Intr;		/* Interrupt created by MakeIntr() */
    struct Interrupt *SoftIntr;		/* Interrupt for sleeping */
    APTR Cia;				/* = NULL; */
    struct IOPArray DefaultArray;
    struct IOExtPar *PAR_request;	/* The current active IO request */

	struct MsgPort *Timer_Port;	/* = NULL; */
	struct timerequest  *Timer_Req;	/* = NULL; */
	APTR ResourceOK;		/* = NULL; ?????? */

    BOOL PAR_shared;		/* Is the device opened in shared mode? */
    BOOL PAR_stopped;		/* Is the device stopped with CMD_STOP? */
    BOOL ACKwasON;		/* Was the ACK enabled previously? */
    BOOL TimerUsed;		/* Has the timer.device been used yet?  */

	UBYTE	ICRmask;
	UBYTE	OpenTimerError;
	UBYTE	DefaultServer;
	UBYTE	GotCIAFlag;	/* TRUE if AddICRVector() succeeded */

};

#define PAR_queue	pd->PAR_queue
#define Misc		pd->Misc
#define Intr		pd->Intr
#define SoftIntr	pd->SoftIntr
#define Cia		pd->Cia
#define PAR_signal	pd->PAR_signal
#define DefaultArray	pd->DefaultArray
#define PAR_request	pd->PAR_request
#define Timer_Port	pd->Timer_Port
#define Timer_Req	pd->Timer_Req
#define SleeperBits	pd->SleeperBits
#define ResourceOK	pd->ResourceOK
#define PAR_shared	pd->PAR_shared
#define PAR_stopped	pd->PAR_stopped
#define ACKwasON	pd->ACKwasON
#define TimerUsed	pd->TimerUsed
#define OpenTimerError	pd->OpenTimerError
#define DefaultServer	pd->DefaultServer
#define GotCIAFlag	pd->GotCIAFlag

#define PAR_length	PAR_request->IOPar.io_Length	/* The number of characters remaining to be transfered */
#define PAR_command	PAR_request->IOPar.io_Command	/* The current operation that is being performed */
#define PAR_flags	PAR_request->IOPar.io_Flags	/* The current status of this IO request */
#define PAR_error	PAR_request->IOPar.io_Error	/* The error encountered by this request */
#define PAR_data	PAR_request->IOPar.io_Data	/* Address of data to be spit out (or in) */
#define PAR_actual	PAR_request->IOPar.io_Actual	/* Number of character transferred to date */
#define PAR_status	PAR_request->IOPar.io_Status	/* The status of the port */

#define DisableACK()	AbleICR(Cia,CIAICRF_FLG)	/* Disable the *ACK interrupt */
#define EnableACK()	AbleICR(Cia,(CIAICRF_FLG|0x80))	/* Enable the *ACK interrupt */
#define SetACK()	SetICR(Cia,(CIAICRF_FLG|0x80))	/* Create an *ACK interrupt */
#define ClearACK()	SetICR(Cia,CIAICRF_FLG)		/* Clear the *ACK interrupt */

#define DataInput()	ciaa.ciaddrb = 0x00	/* Set data pins as inputs */
#define DataOutput()	ciaa.ciaddrb = 0xFF	/* Set data pins as outputs */
#define ControlInput()	ciab.ciaddra &= 0xF8	/* Set control pins as inputs */
#define ControlOutput() ciab.ciaddra |= 0x07	/* Set control pins as outputs */

#define Empty(x)	(x.mlh_TailPred == (struct MinNode *)&x)
#define ClearQuick()	request->IOPar.io_Flags &= 0xFE

/*************************************/
/* Here come the function prototypes */
/*************************************/

/* void InterruptServer(); Not needed any more */
void BoogieServer();
void SlowPokeServer();

extern void SoftwareInterrupt();
void __asm MD_BeginIO(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd);
BYTE __asm MD_AbortIO(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd);
BYTE __asm MD_Open(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd);
BYTE __asm MD_Close(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd);
void FreeAll();
void FreeInterrupt();
void DeleteMyPort();
struct Interrupt *MakeInterrupt();

/*************************************************************************/
/* When a CMD_WRITE command is issued, the IO request is either put in a */
/* queue or made the current active PAR_request. If it is the current    */
/* active PAR-request, then an artificial *ACK interrupt is created.     */
/* When an *ACK is received, the interrupt server is activated and spits */
/* out the remaining characters one by one.                              */
/*                                                                       */
/* If the command is a CMD_READ, then the IO request is merely put in    */
/* the queue or made active immediately, depending on whether there are  */
/* any other requests pending.                                           */
/*                                                                       */
/* If when an *ACK interrupt is detected the PAR_length is zero, then an */
/* an IO request has just been completed.                                */
/*************************************************************************/

void NextRequest(struct ParDev *pd)	/* Activate the next IO request, if any pending */
{
	/* KPrintF("parallel/NextRequest: started  \n"); */

	PAR_error = NULL;
	PAR_flags &= 0x0F;			 /* Clear upper four flags */
	ReplyMsg((struct Message *)PAR_request); /* Reply to the current one */
	PAR_request = NULL;			 /* There is no active request */

	/* KPrintF("parallel/NextRequest: ReplyMsg()ed  \n"); */

	/* Put a Disable() here? No, because we are running in supervisor mode. */

	if (!Empty(PAR_queue))
	    {
		/* KPrintF("parallel/Not Empty PAR_queue  \n"); */

		PAR_request = (struct IOExtPar *) RemTail((struct List *)&PAR_queue);
		PAR_flags &= 0x0F;
		PAR_flags |= IOPARF_ACTIVE;
		PAR_actual = 0;
		switch (PAR_command)
		    {
			case CMD_WRITE:
			    DataOutput();	/* Set the data lines as outputs */
			    ControlInput();	/* Set SEL, BUSY & POUT as input */
			    /* SetACK(); */
			    break;
			case CMD_READ:
			    DataInput();	/* Set the data lines as inputs */
			    ControlOutput();    /* Set the control lines as outputs */
			    ciab.ciapra|=0x04;	/* SEL=1 */
			    ciab.ciapra&=0xFC;	/* POUT=0, BUSY=0 */
			    break;
		    }
		/* KPrintF("parallel/PAR_request = %lx  \n",PAR_request); */

	    }
	    else
	    {
		/* KPrintF("parallel/Empty PAR_queue   \n"); */

		DisableACK();		/* ...since there are no other requests */
		DataInput();		/* Set data lines as input */
		ControlInput();		/* Set control lines as input */
	    }

	/* KPrintF("parallel/NextRequest: completed\n"); */
}

/****** parallel.device/CMD_WRITE *********************************************
*
*   NAME
*	Write -- send output to parallel port
*
*   FUNCTION
*	This command causes a stream of characters to be written to the
*	parallel output register. The number of characters is specified in
*	io_Length, unless -1 is used, in which case output is sent until
*	a zero byte occurs in the data. This is independent of, and may be
*	used simultaneously with setting the EOFMODE in io_ParFlags and using
*	the PTermArray to terminate the read or write.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_WRITE (03)
*	io_Flags        If IOF_QUICK is set, driver will attempt Quick IO
*	io_Length       number of characters to transmit, or if set
*	                to -1 send until zero byte encountered
*	io_Data         pointer to block of data to transmit
*
*   RESULTS
*	io_Error -- If the Write succeded, then io_Error will be null.
*	     If the Write failed, then io_Error will contain an error code.
*
*   SEE ALSO
*	parallel.device/PDCMD_SETPARAMS
*
***************************************************************/

/****** parallel.device/CMD_READ *******************************************
*
*   NAME
*	Read -- read input from parallel port
*
*   FUNCTION
*	This command causes a stream of characters to be read from the
*	parallel I/O register. The number of characters is specified in
*	io_Length. The EOF and EOL modes are supported, but be warned that
*	using these modes can result in a buffer overflow if the proper
*	EOL or EOF character is not received in time. These modes should
*	be used only when the sender and receiver have been designed to
*	cooperate. A safety guard can be implemented to EOF by setting
*	io_Length to a maximum allowed value. That cannot be done with EOL
*	since the EOL mode is identified by io_Length=-1.
*
*	The parallel.device has no internal buffer; if no read request has
*	been made, pending input (i.e. handshake request) is not
*	acknowledged.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_READ (02)
*	io_Flags        If IOF_QUICK is set, driver will attempt Quick IO
*	io_Length       number of characters to receive.
*	io_Data         pointer where to put the data.
*
*   RESULTS
*	io_Error -- if the Read succeded, then io_Error will be null.
*	    If the Read failed, then io_Error will contain an error code.
*
*   SEE ALSO
*	parallel.device/PDCMD_SETPARAMS
*
************************************************************/

/****** parallel.device/CMD_CLEAR ********************************************
*
*   NAME
*	Clear -- clear the parallel port buffer
*
*   FUNCTION
*	This command just RTS's (no buffer to clear)
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_CLEAR (05)
*
************************************************************/

/****** parallel.device/CMD_STOP ******************************************
*
*   NAME
*	Stop -- pause current activity on the parallel device
*
*   FUNCTION
*	This command halts the current I/O activity on the parallel
*	device by discontinuing the handshaking sequence. The stop and
*	start commands may not be nested.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_STOP (06)
*
*   SEE ALSO
*	parallel.device/CMD_START
*
************************************************************************/

/****** parallel.device/CMD_START *****************************************
*
*   NAME
*	Start -- restart paused I/O over the parallel port
*
*   FUNCTION
*	This command restarts the current I/O activity on the parallel
*	port by reactivating the handshaking sequence.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_START (07)
*
*   SEE ALSO
*	parallel.device/CMD_STOP
*
************************************************************************/

/****** parallel.device/CMD_RESET ******************************************
*
*   NAME
*	Reset -- reinitializes the parallel device
*
*   FUNCTION
*	This command resets the parallel device to its freshly initialized
*	condition. It aborts all I/O requests both queued and current and
*	sets the devices's flags and parameters to their boot-up time 
*	default values. At boot-up time the PTermArray is random, and it
*	will be so also here.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_RESET (01)
*
*   RESULTS
*	Error -- if the Reset succeded, then io_Error will be null.
*	         if the Reset failed, then the io_Error will be non-zero.
*
***************************************************************************/

/****** parallel.device/CMD_FLUSH *******************************************
*
*   NAME
*	Flush -- clear all queued I/O requests for the parallel port
*
*   FUNCTION
*	This command purges the read and write request queues for the
*	parallel device. The currently active request is not purged.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      CMD_FLUSH (08)
*
*************************************************************************/

/****** parallel.device/PDCMD_QUERY ******************************************
*
*   NAME
*	Query -- query parallel port/line status
*
*   FUNCTION
*	This command return the status of the parallel port lines and
*	registers. 
*
*   IO REQUEST
*	io_Message      must have mn_ReplyPort initialized
*	io_Device       set by OpenDevice
*	io_Unit         set by OpenDevice
*	io_Command      PDCMD_QUERY (09)
*
*   RESULTS
*	io_Status        BIT  ACTIVE  FUNCTION
*
*	                 0     high   printer busy toggle (offline)
*	                 1     high   paper out
*	                 2     high   printer selected on the A1000
*	                              printer selected & serial "Ring
*	                              Indicator" on the A500/A2000
*	                              Use care when making cables.
*	                 3      -     read=0,write=1
*	               4-7            reserved
*
*   BUGS
*	In a earlier version of this AutoDoc, BUSY and PSEL were reversed.
*	The function has always been correct.
*
***************************************************************************/

/****** parallel.device/PDCMD_SETPARAMS **************************************
*
*   NAME
*	SetParams -- change parameters for the parallel device
*
*   FUNCTION
*	This command allows the caller to change the EOFMODE parameter for
*	the parallel port device. It will disallow changes if any reads or
*	writes are active or queued.  
*
*	The PARB_EOFMODE bit of io_ParFlags controlls whether the
*	io_PTermArray is to be used as an additional termination criteria
*	for reads and writes.  It may be set directly without a call to
*	SetParams, setting it here performs the additional service of
*	copying the PTermArray into the device default array which is used
*	as the initial array for subsequent device opens. The Shared bit
*	can be changed here, and overrides the current device access mode
*	set at OpenDevice time.
*
*   IO REQUEST
*	io_Message      mn_ReplyPort initialized
*	io_Device       preset by OpenDevice
*	io_Unit         preset by OpenDevice
*	io_Command      PDCMD_SETPARAMS (0A)
*  			NOTE that the following fields of your IORequest
*	                are filled by Open to reflect the parallel device's
*	                current configuration.
*	io_PExtFlags    must be set to zero, unless used
*	io_ParFlags     see definition in parallel.i or parallel.h
*	                NOTE that x00 yields exclusive access, PTermArray
*	                inactive.
*	io_PTermArray   ASCII descending-ordered 8-byte array of
*	                termination characters. If less than 8 chars
*	                used, fill out array w/lowest valid value.
*	                Terminators are used only if EOFMODE bit of
*	                io_Parflags is set. (e.g. x512F040303030303 )
*	                This field is filled on OpenDevice only if the
*	                EOFMODE bit is set.
*
*   RESULTS
*	io_Error -- if the SetParams succeded, then io_Error will be null.
*	            if the SetParams failed, then io_Error will be non-zero.
*
************************************************************************/

void __asm MD_BeginIO(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd)
{
	struct IOExtPar *temp;
	BOOL dont_reply = FALSE;	/* shall we reply in the DoIO? */

	request->IOPar.io_Error = NULL;
	
	switch (request->IOPar.io_Command)
	{
	    case CMD_WRITE:
		request->IOPar.io_Actual = 0;	/* No characters sent yet */
		request->IOPar.io_Flags&=0x0F;	/* Clear upper four flags */

		/* KPrintF("parallel/Write: request= %lx  \n",request); */

		if (request->IOPar.io_Length != 0)
		    {
			ClearQuick();			/* Clear the IO Quick bit */
			dont_reply = TRUE;		/* Don't ReplyMsg in DoIO */
			Disable();
			if (PAR_request == NULL)
			    {
				ControlInput();
				DataOutput();
				PAR_request = request;
				PAR_flags |= IOPARF_ACTIVE;				
				SetACK();		/* Tell server to start */
				if (!PAR_stopped)
				    EnableACK();
				    else
				    ACKwasON=TRUE;	
			    }
			    else
			    {
				request->IOPar.io_Flags |= IOPARF_QUEUED;
				AddHead((struct List *)&PAR_queue, (struct Node *)request);
			    }
			Enable();
		    }
		break;

	    case CMD_READ:
		request->IOPar.io_Actual = 0;	/* No characters sent yet */
		request->IOPar.io_Flags&=0x0F;	/* Clear upper four flags */

		if (request->IOPar.io_Length != 0)
		    {
			ClearQuick();
			dont_reply = TRUE;
			Disable();
			if (PAR_request == NULL)
			    {
				ControlOutput();
				DataInput();
				ciab.ciapra|=0x04;	/* SEL=1 */
				ciab.ciapra&=0xFC;	/* POUT=0, BUSY=0 */
				PAR_request = request;
				PAR_flags |= IOPARF_ACTIVE;
				if (!PAR_stopped)
				    EnableACK();
				    else
				    ACKwasON=TRUE;
			    }
			    else
			    {
				request->IOPar.io_Flags |= IOPARF_QUEUED;
				AddHead((struct List *)&PAR_queue, (struct Node *)request);
			    }
			Enable();
		    }
		break;

	    case CMD_CLEAR:
	    case CMD_UPDATE:
		break;

	    case CMD_STOP:
		Disable();		/* Stop other tasks & interrupts */
		ACKwasON |= CIAICRF_FLG&AbleICR(Cia,0L);
		DisableACK();		/* Disable the *ACK interrupt */
					/* Assert busy line if currently reading */
		PAR_stopped = TRUE;
		Enable();		/* Continue multitasking */
		break;

	    case CMD_START:
		Disable();
					/* Negate busy flag if currently reading */
		PAR_stopped = FALSE;
		if (ACKwasON)
		    EnableACK();	/* Enable the *ACK interrupt */
		Enable();
		break;

	    case CMD_RESET:
	    case CMD_FLUSH:
		Disable();
		while (!Empty(PAR_queue))
		    {
			temp = (struct IOExtPar *) RemTail((struct List *)&PAR_queue);
			temp->IOPar.io_Flags &= 0x0F;
			temp->IOPar.io_Flags |= IOPARF_ABORT;
			temp->IOPar.io_Error = IOERR_ABORTED;
			ReplyMsg((struct Message *)temp);
		    }
		if (request->IOPar.io_Command == CMD_RESET)
		    {
			if (PAR_request)
			    {
				PAR_flags &= 0x0F;
				PAR_flags |= IOPARF_ABORT;
				PAR_error = IOERR_ABORTED;
				ReplyMsg((struct Message *)PAR_request);
				PAR_request = NULL;
				if (TimerUsed==TRUE)
				    {
					Timer_Port->mp_Flags = PA_IGNORE;
					AbortIO((struct IORequest *)Timer_Req);
					Timer_Port->mp_Flags = PA_SOFTINT;
				    }
			    }
			ControlInput();
			DataInput();
			ACKwasON = FALSE;
			PAR_stopped = FALSE;
		    }
		Enable();
		break;

	    case PDCMD_QUERY:
		request->io_Status = ciab.ciapra&0x07;
		if (PAR_request && PAR_command == CMD_WRITE)
			request->io_Status |= 0x08;
		break;

	    case PDCMD_SETPARAMS:
		DefaultArray.PTermArray0 = request->io_PTermArray.PTermArray0;
		DefaultArray.PTermArray1 = request->io_PTermArray.PTermArray1;
		DefaultServer = request->io_ParFlags&((1<<2)|(1<<3)|(1<<4));
		break;

	    default:
		request->IOPar.io_Error = IOERR_NOCMD;
		break;
	}
	if ((!(request->IOPar.io_Flags&IOF_QUICK)) && (dont_reply==FALSE))
		ReplyMsg((struct Message *)request);
}

BYTE __asm MD_AbortIO(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd)
{
	int i;
			/* What if the IO request has already completed? */
			/* The AbortIO() performs no action. -Andy */

/*	KPrintF("parallel/AbortIO called\n"); */


    Disable();
    switch ((request->IOPar.io_Flags) & 0x70)	/* Use only meaninful bits */
    {
	case IOPARF_QUEUED:
	    Remove((struct Node *)request);
	    request->IOPar.io_Flags &= 0x0F;
	    request->IOPar.io_Flags |= IOPARF_ABORT;
	    request->IOPar.io_Error = IOERR_ABORTED;
	    ReplyMsg((struct Messsage *)request);
	    Enable();
	    return(NULL);
	    break;				/* Useless instruction */

	case IOPARF_ACTIVE:
	    PAR_error = IOERR_ABORTED;
	    PAR_flags &= 0x0F;
	    PAR_flags |= IOPARF_ABORT;
	    ReplyMsg((struct Message *)PAR_request);
	    PAR_request = NULL;
	    if (!Empty(PAR_queue))
		{
		    PAR_request = (struct IOExtPar *) RemTail((struct List *)&PAR_queue);
		    PAR_flags &= 0x0F;
		    PAR_flags |= IOPARF_ACTIVE;
		    PAR_actual = 0;
		    switch (PAR_command)
			{
			    case CMD_WRITE:
				DataOutput();
				ControlInput();
				break;
			    case CMD_READ:
				DataInput();
				ControlOutput();
				ciab.ciapra|=0x04;
				ciab.ciapra&=0xFC;
				break;
			}
		}
		else
		{
		    ACKwasON = FALSE;
		    DisableACK();
		    DataInput();
		    ControlInput();
		    if (TimerUsed==TRUE)
			{
			    Timer_Port->mp_Flags = PA_IGNORE;
			    AbortIO((struct IORequest *)Timer_Req);
			    Timer_Port->mp_Flags = PA_SOFTINT;
			}
		}
	    Enable();
	    return(NULL);
	    break;

	default:
	    Enable();
	    return(IOERR_NOCMD);
	    break;
    }
}

/****** parallel.device/OpenDevice ******************************************
*
*   NAME
*	Open -- a request to open the parallel port
*
*   SYNOPSIS
*	error = OpenDevice("parallel.device", unit, ioExtPar, flags)
*	D0                  A0                D0    A1        D1
*
*   FUNCTION
*	This function allows the requestor software access to the parallel
*	device.  Unless the shared-access bit (bit 5 of io_ParFlags) is
*	set, exclusive use is granted and no other access is allowed
*	until the owner closes the device.
*
*	A FAST_MODE, can be specified (bit 3 of io_Parflags) to speed up
*	transfers to high-speed printers. Rather than waiting for the printer
*	to acknowledge a character using the *ACK interrupt, this mode will
*	send out data as long as the BUSY signal is low. The printer must be
*	able to raise the BUSY signal within 3 micro-seconds on A2630s,
*	otherwise data will be lost. Should be used only in an exclusive-
*	access Open().
*
*	A SLOWMODE mode can be specified (bit 4 of io_ParFlags) when very
*	slow printers are used. If the printer acknowledges data at less
*	than 5000 bytes per second, then this mode will actually save CPU
*	time, although it consumes much more with high-speed printers.
*
*	The PTermArray of the ioExtPar is initialized only if the EOFMODE
*	bit (bit 1 of io_ParFlags) is set. The PTermArray can be further
*	modified using the PDCMD_SETPARAMS command.
*
*   INPUTS
*	"parallel.device" - a pointer to literal string "parallel.device"
*	unit - Must be zero for future compatibility
*	ioExtPar - pointer to an IO Request block of structure IOExtPar to
*		   be initialized by the Open() function. The io_ParFlags
*		   field must be set as desired.
*	flags - Must be zero for future compatibility
*
*   RESULTS
*	d0 -- same as io_Error
*	io_Error -- if the Open succeded, then io_Error will be null.
*		    If the Open failed, then io_Error will be non-zero.
*
*   SEE ALSO
*	exec/CloseDevice
*
****************************************************************************/

#define PAR_opencount	pd->Lib.lib_OpenCnt
#define PAR_expungeflag	pd->Lib.lib_Flags

BYTE __asm MD_Open(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd)
{
    if (PAR_opencount > 1)
	{
     	if ( (request->io_ParFlags&PARF_SHARED) && (PAR_shared == TRUE))
	    {
		/* KPrintF("Open: subsequent shared access\n"); */

		PAR_opencount++;
		request->io_PTermArray.PTermArray0 = DefaultArray.PTermArray0;
		request->io_PTermArray.PTermArray1 = DefaultArray.PTermArray1;
		request->IOPar.io_Error = NULL;
		return(NULL);
	    }
	    else
	    {
		/* KPrintF("Open: Sorry, ParErr_DevBusy\n"); */

		request->IOPar.io_Error = ParErr_DevBusy;  /* = 1 */
		return(ParErr_DevBusy);
	    }
	}
	else
	{
	    OpenTimerError = 1;		/* Initialise for FreeAll() */
	    ResourceOK = (APTR)1;
	    Timer_Req = NULL;
	    Timer_Port = NULL;
	    TimerUsed = NULL;
	    Intr = NULL;
	    SoftIntr = NULL;
	    GotCIAFlag = 0;

	    PAR_request = NULL;

	    PAR_queue.mlh_Head = (struct MinNode *)&PAR_queue.mlh_Tail;
	    PAR_queue.mlh_TailPred = (struct MinNode *)&PAR_queue.mlh_Head;
	    PAR_queue.mlh_Tail = NULL;
	    
	    request->io_PTermArray.PTermArray0 = DefaultArray.PTermArray0;
	    request->io_PTermArray.PTermArray1 = DefaultArray.PTermArray1;
	    request->IOPar.io_Flags = 0x0F;	/* Clear upper bits */

	    PAR_expungeflag = NULL;
	    PAR_stopped = FALSE;

	    if ((request->io_ParFlags&PARF_SHARED) == PARF_SHARED)
	        {
			PAR_shared = TRUE;
			/* KPrintF("Open: shared access\n"); */
		}
		else
		{
			PAR_shared = FALSE;
			/* KPrintF("Open: exclusive access\n"); */
		}
	    ControlInput();
	    DataInput();	    /* Set all pins as inputs */

	    Cia = (APTR) OpenResource("ciaa.resource");
	    	if (Cia == NULL) return(IOERR_OPENFAIL);

/* 	KPrintF("parallel/Open: Cia OK "); */

	    switch(request->io_ParFlags&((1<<2)|(1<<3)|(1<<4)))
	    {
		case PARF_RAD_BOOGIE:
		    Intr = (struct Interrupt *) MakeInterrupt("parallel.device",
				0,&BoogieServer,pd);
/*	KPrintF("parallel/Open: RAD_BOOGIE server established\n"); */
		    break;

		case PARF_SLOWMODE:
		    Intr = (struct Interrupt *) MakeInterrupt("parallel.device",
				0,&SlowPokeServer,pd);
/*	KPrintF("parallel/Open: SLOWMODE server established\n"); */
		    break;

		case PARF_ACKMODE:
		    Intr = (struct Interrupt *) MakeInterrupt("parallel.device",
				0,&SlowPokeServer,pd);
/*	KPrintF("parallel/Open: ACKMODE server established\n"); */
		    break;

		default:	/* Use the preset interrupt server */
/*	KPrintF("parallel/Open: choosing default server...\n"); */
		    switch(DefaultServer)
		    {
			case PARF_RAD_BOOGIE:
			    Intr = (struct Interrupt *) MakeInterrupt("parallel.device",
					0,&BoogieServer,pd);
/*	KPrintF("parallel/Open: RAD_BOOGIE server established\n"); */
			    break;

			case PARF_SLOWMODE:
			case PARF_ACKMODE:
			default:
			    Intr = (struct Interrupt *) MakeInterrupt("parallel.device",
					0,&SlowPokeServer,pd);
/*	KPrintF("parallel/Open: SLOWMODE server established\n"); */
			    break;
		    }
		    break;
	    }
	    if (Intr == NULL) return(IOERR_OPENFAIL);

	    SoftIntr = (struct Interrupt *) MakeInterrupt("parallel.sleeper",0,&SoftwareInterrupt,(APTR)pd);
		if (SoftIntr == NULL)
		    {
			FreeAll(pd);
			return(IOERR_OPENFAIL);
		    }

/*	KPrintF("parallel/Open: Intr OK "); */

	    if (!(Timer_Port = (struct MsgPort *)AllocMem(sizeof(struct MsgPort),
			MEMF_PUBLIC|MEMF_CLEAR)))
		{
		    FreeAll(pd);
		    return(IOERR_OPENFAIL);
		}

	    Timer_Port->mp_Node.ln_Type = NT_MSGPORT;
	    Timer_Port->mp_Node.ln_Name = "sleeper.port";
	    Timer_Port->mp_Flags = PA_SOFTINT;
	    Timer_Port->mp_SigTask = (struct Task *)SoftIntr;
	    AddPort(Timer_Port);

	    if (!(Timer_Req = (struct timerequest *)CreateExtIO(Timer_Port, (ULONG) sizeof(struct timerequest))))
		{
		    FreeAll(pd);
		    return(IOERR_OPENFAIL);
		}

	    if (OpenTimerError = OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequet *)Timer_Req, 0))
		{
		    FreeAll(pd);
		    return(IOERR_OPENFAIL);
		}

/*	KPrintF("parallel/Open: Timer stuff OK "); */

	    if (ResourceOK = (APTR)ObtainParallelResource())	/* Written in assembler */
		{
		    FreeAll(pd);
		    return(IOERR_OPENFAIL);
		}

/*	KPrintF("parallel/Open: resource OK "); */

	    Disable();

	    if (AddICRVector(Cia,CIAICRB_FLG,Intr))	/* Add *ACK interrupt server to the list */
		{
		    Enable();
		    FreeAll(pd);
		    return(IOERR_OPENFAIL);
		}

	    ClearACK();		/* clear pending if any */

/*	KPrintF("parallel/Open: AddICRVector OK "); */

	    GotCIAFlag = 1;
	    PAR_opencount++;
	    DisableACK();	/* Don't allow any *ACK precessing until a CMD_READ or WRITE */
	    Enable();
	    return(NULL);
	}
}

BYTE __asm MD_Close(register __a1 struct IOExtPar *request,
			register __a6 struct ParDev *pd)
{
	UWORD	temp;

/*	KPrintF("\nparallel/Close: called\n"); */
/*	if (PAR_request) KPrintF("parallel/Close: PAR_request not NULL\n"); */

    Forbid();
    PAR_opencount--;
    if (PAR_opencount == 0)
	{
	    if ((PAR_request) && (!PAR_shared))
		{
		    temp = request->IOPar.io_Command;
		    request->IOPar.io_Command = CMD_RESET;
		    MD_BeginIO(request,pd);
		    request->IOPar.io_Command = temp;
		}
	    FreeAll(pd);
	}
    Permit();
    return(NULL);
}

void FreeAll(struct ParDev *pd)
{

/*	KPrintF("\nparallel/FreeAll: started"); */

	if (OpenTimerError==NULL)
		CloseDevice((struct IORequest *)Timer_Req);

/*	KPrintF("\nparallel/FreeAll: closed timer device"); */

	if (Timer_Req)
		DeleteExtIO((struct IORequest *)Timer_Req);

/*	KPrintF("\nparallel/FreeAll: deleted timer request"); */

	if (Timer_Port)
		DeleteMyPort(Timer_Port,pd);

/*	KPrintF("\nparallel/FreeAll: deleted timer port"); */

	if (!ResourceOK)
		ReleaseParallelResource();

/*	KPrintF("\nparallel/FreeAll: released reource"); */

	if (Intr)
	{
		if(GotCIAFlag)
		{
			RemICRVector(Cia, CIAICRB_FLG, Intr);
		}
		FreeInterrupt(Intr);
	}

/*	KPrintF("\nparallel/FreeAll: freed interrupt"); */

	if (SoftIntr)
		FreeInterrupt(SoftIntr);
}

struct Interrupt *MakeInterrupt(char *name, int pri, VOID (*code)(), struct ParDev *pd)
{
    register struct Interrupt *intr = NULL;
        
    intr = (struct Interrupt *)AllocMem(sizeof(*intr), MEMF_PUBLIC);
	if (intr == NULL) return(NULL);
    
    intr->is_Node.ln_Pri = pri;
    intr->is_Node.ln_Type = NT_INTERRUPT;
    intr->is_Node.ln_Name = name;
    intr->is_Data = (APTR)pd;
    intr->is_Code = code;
    
    return(intr);
}

void FreeInterrupt(register struct Interrupt *intr)
{
    struct ParDev *pd;

    pd = (struct ParDev *)intr->is_Data;

    if (intr == NULL) return;
    if (intr->is_Node.ln_Type != NT_INTERRUPT) return;
    
    intr->is_Node.ln_Type = 0;
    FreeMem(intr, sizeof(*intr));
}

void DeleteMyPort(register struct MsgPort *port, struct ParDev *pd)
{
    if (port->mp_Node.ln_Name)
	RemPort(port);
    port->mp_SigTask = (struct Task *)-1;
    port->mp_MsgList.lh_Head = (struct Node *)-1;
    FreeMem(port,(ULONG)sizeof(struct MsgPort));
}


/*************************************************************************/
/* For testing puposes, test code is put right here in main(). This way  */
/* I can step through the source code with CPR.                          */
/*************************************************************************/

/*

main()
{
	BYTE			error;
	long			*parbuffer = NULL;
	struct	Port		*parport = NULL;
	struct	IOExtPar	*parreq = NULL;
	struct	ParDev		Dev;

	if ( !( parbuffer = AllocMem(512,MEMF_CLEAR | MEMF_CHIP) ) )
		goto CleanUp;
	if ( !( parport = CreatePort(0L,0L) ) )
		goto CleanUp;
	if ( !( parreq = (struct IOExtPar *)CreateExtIO(parport, sizeof(struct IOExtPar)) ) )
		goto CleanUp;

	parreq->io_ParFlags = NULL;
	parreq->IOPar.io_Device = &Dev;

	parreq->IOPar.io_Device->dd_Library.lib_OpenCnt = NULL;
	parreq->IOPar.io_Device->dd_Library.lib_Flags = NULL;


	if ( error = MD_Open(parreq,&Dev) )
		goto CleanUp;

	while(TRUE)
	{
		parreq->IOPar.io_Length = sizeof("Hello, world.");
		parreq->IOPar.io_Data = (APTR)"Hello, world.";
		parreq->IOPar.io_Command = CMD_WRITE;
		parreq->io_ParFlags = NULL;
		MD_BeginIO(parreq,&Dev);


		if (parreq->IOPar.io_Error)
		{
			printf("Error trying to write");
		}
	}

CleanUp:
	if (parreq)
		{ DeleteExtIO((struct IORequest *)parreq); }
	if (parport)
		DeletePort(parport);
	if (parbuffer) 
		FreeMem(parbuffer,512);
	return(NULL);
}

*/
