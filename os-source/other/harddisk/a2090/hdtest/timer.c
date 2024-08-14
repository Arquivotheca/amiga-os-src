/* TIMER.C
 *
 * Purpose: To access the timer device
 *
 */

#include <intuition/intuition.h>
struct timerequest TimerRequest;
struct MsgPort timerMsgPort = {0};
char	TimerActive = 0;	/* TRUE if the timer is already active */
char	SkipTimeOut = 0;	/* TRUE means skip next timeout */
int	TimeLeft = -1;		/* # of seconds left before time exceeded */
extern	ULONG    WakeUpBit;	/* Bit causing WAIT to be satisfied */

int TimerOpen()
{
   int error;

/* Open the timer device */
   if ((error = OpenDevice(TIMERNAME, UNIT_VBLANK,&TimerRequest,0)) != 0) {
      printf("\nTimer OpenDevice Error: %d.\n",error);
      return(error);
      }

/* Set up the message port in the IO request */
   timerMsgPort.mp_Node.ln_Type = NT_MSGPORT;
   timerMsgPort.mp_Node.ln_Name = "TIMER";
   timerMsgPort.mp_Flags = 0;
   timerMsgPort.mp_SigBit = AllocSignal(-1);
   timerMsgPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
   AddPort(&timerMsgPort);
   TimerRequest.tr_node.io_Message.mn_ReplyPort = &timerMsgPort;
   return(0);
}

/* Issue a request for a timer of "secs" seconds */

void StartTime(secs,micros)
int secs, micros;
{
    if (TimerActive == 0)
    {
	TimerActive = 1;
	TimerRequest.tr_node.io_Command = TR_ADDREQUEST; 
	TimerRequest.tr_time.tv_secs = secs;
	TimerRequest.tr_time.tv_micro = micros;
	SendIO(&TimerRequest);
    }
}

/* Main routine to do event timeout checking
 *
 * If tval = 0, then it returns the # of seconds remaining (0 means done!)
 * If tval = -1, then it cancels any pending timeout.
 * If tval > 0, the requested period (in seconds) is timed, with 5 second
 *		resolution.
 */

int TimeCheck(tval)
   int tval;
{

/*    if (TimerActive)	*//* If the Timer is active */
/*      if(WakeUpBit & (1 << timerMsgPort.mp_SigBit))*/ /* If Timer timed out */
	if (GetMsg(&timerMsgPort) != NULL)	/* If timer message queued */
	{
	    TimerActive = 0;	/* Show timer inactive */
	    while (GetMsg(&timerMsgPort) != NULL); /* Get all queued messages */
	    if (SkipTimeOut) /* If skipping 1st timeout */
	    {
		SkipTimeOut = 0;
			/* Start up next Timeout */
		if (tval == 0) StartTime(1,0);
	    } else		
		if (TimeLeft > 0)	/* Event being timed out */
		{
		    TimeLeft--;		/* Count off second */
			/* If still some time left */
		    if ((TimeLeft > 0) && (tval == 0))
		    {
			StartTime(1,0); /* 	Timeout another second */
		    }
		}
	}

    if (tval > 0)		/* Add timeout request */
    {
        if (TimerActive)	/* If the timer is already active */
	    SkipTimeOut++;	/*	ignore next timeout */
	else
	{
	    StartTime(1,0);	/* Start timeout of requested length */
	    SkipTimeOut = 0;	/* Clear Skip flag */
	}
	TimeLeft = tval;	/* Indicate time left till timeout */

    } else if (tval < 0)	/* Cancel pending timeout */
    {
        if (TimerActive)	/* If the timer is already active */
	    SkipTimeOut++;	/*	ignore next timeout */
	TimeLeft = -1;		/* Indicate no timeout pending */

    }
	return(TimeLeft);	/* Return time remaining */
}

void TimerClose()		/* Quit using timer device */
{
/*  TimerRequest.tr_node.io_Command = CMD_CLEAR; *//* Clear any pending time req */
/*   SendIO(&TimerRequest);	*//* due to bug in DoIO */
/*   WaitPort(&timerMsgPort);*/
/*   while (GetMsg(&timerMsgPort) != NULL);	*//* Get all queued messages */
   CloseDevice(&TimerRequest);
/*   while (GetMsg(&timerMsgPort) != NULL);	*//* Get all queued messages */
   FreeSignal(timerMsgPort.mp_SigBit);
   RemPort(&timerMsgPort);
}
