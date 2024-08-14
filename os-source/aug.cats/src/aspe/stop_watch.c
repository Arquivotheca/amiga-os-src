#include <exec/types.h>
#include <devices/timer.h>
#include <proto/all.h>
#include "aspe_protos.h"

struct Library *TimerBase=NULL;

/******************************
* Function: Get starting time *
* --------------------------- *
* Argument:                   *
******************************/
VOID Timer_Start(struct timerequest *Time_Req)
{
    Time_Req->tr_node.io_Command=TR_GETSYSTIME;
    Time_Req->tr_node.io_Flags=IOF_QUICK;
    DoIO((struct IORequest *)Time_Req);
}

/********************************************************
* Function: Get ending time and compute time difference *
* ----------------------------------------------------- *
* Arguments:                                            *
* Results: in global timerequest->tr_time               *
********************************************************/
VOID Timer_Stop(struct timerequest *Time_Req)
{
struct timeval StartTime;

    StartTime=Time_Req->tr_time;

    Time_Req->tr_node.io_Command=TR_GETSYSTIME;
    Time_Req->tr_node.io_Flags=IOF_QUICK;
    DoIO((struct IORequest *)Time_Req);

    SubTime(&(Time_Req->tr_time),&StartTime);
}

/*********************************
* Function: Initialize stopwatch *
* ------------------------------ *
* Results:                       *
*********************************/
struct timerequest *Init_Timer(VOID)
{
register struct timerequest *Time_Req=NULL;
register struct MsgPort *port=NULL;

   if (port=CreatePort(NULL,NULL))
      {
      if (Time_Req=(struct timerequest *)
         CreateExtIO(port, sizeof(struct timerequest)))
         {
         if (!OpenDevice(TIMERNAME,UNIT_VBLANK,
                         (struct IORequest *)Time_Req,NULL))
            {
            TimerBase=(struct Library *)Time_Req->tr_node.io_Device;
            }
         else
            {
            DeleteExtIO((struct IORequest *)Time_Req);
            Time_Req=NULL;
            }
         }
      if (!Time_Req)
         {
         DeletePort(port);
         port=NULL;
         }
      }
   return(Time_Req);
}

/**************************
* Function: Free up timer *
* ----------------------- *
* Arguments:              *
* Results: none           *
**************************/
VOID Free_Timer(struct timerequest *Time_Req)
{
   if (Time_Req)
      {
      CloseDevice((struct IORequest *)Time_Req);
      DeletePort(Time_Req->tr_node.io_Message.mn_ReplyPort);
      DeleteExtIO((struct IORequest *)Time_Req);
      }
}
/* eof */