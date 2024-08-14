#include <exec/types.h>
#include <libraries/dosextens.h>
#include <lattice/stdio.h>
#include <exec/devices.h>
#include <exec/io.h>
#include "ddefs.h"

extern	WDCMD	*CMDBLKPADDR;		/* Pointer to Command block */
extern	struct	IOStdReq hdrequest;
extern	struct	MsgPort HdMsgPort;
extern	char	server_active;
extern	int	abort;		/* Not 0 means exit program */
extern	int	timeouts;	/* Number of timeouts this pass */
extern	int	stop_test;	/* Not 0 means <ctrl-c> typed */
extern	int	errors;		/* Number of errors this pass */
	int	delay_val=30;	/* Time to wait for next command */
	char	rpending=0;	/* Non-zero means waiting for reply to I/O */

extern	ULONG    WakeUpBit, WaitMask;

VOID wdwait()
{
	char	ostr[100];	/* Workspace for sprintf */

	if (rpending)	/* If previous I/O not complete, wait for it */
	{
		rpending = 0;
		while (GetMsg(&HdMsgPort) == NULL);
	}

		/* Build request for hddisk driver */
	SetIO(&hdrequest, (UWORD)HD_SPECIAL, CMDBLKPADDR,
		(long)sizeof(WDCMD), 0L);
	SendIO(&hdrequest);		/* Startup IO */	

    do {
	TimeCheck(delay_val);		/* Startup timeout req. for command */
	do
	{
	    WakeUpBit = Wait(WaitMask);	/* Wait for char. typed,*/
					/*	disk I/O int	*/
					/*	or time out	*/
	}
	while (
		(TimeCheck(0) > 0) &&
	      (rpending=(GetMsg(&HdMsgPort) == NULL)));

	if (TimeCheck(0) < 1)
	{
		sprintf(ostr,
		  "\n\rDisk controller command not complete after %d secs!\n\r",
		   delay_val);
		CDPutStr(ostr);
		++timeouts; ++errors;
	} else
	    TimeCheck(-1);	/* Cancel timeout request */
    } while (rpending);
    check_stop_test();
    delay_val = 10;		/* Set default timeout time to 10 secs. */
}
