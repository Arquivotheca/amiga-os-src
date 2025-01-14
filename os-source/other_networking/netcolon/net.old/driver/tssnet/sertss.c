
/*******************************************************************/
/*                                                                 */
/*    nettss.c - TSSnet (TM) net access routines                   */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>
#include <graphics/regions.h>
#include <graphics/gels.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>


#include "TSSnetUInc.h"




#include "netdnet.h"
#include "server.h"

#define Timeout -101	


#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETDNET-SERVER/a";
#endif


int open_lib (GLOBAL);


VOID SetTimer (unsigned long, unsigned long, struct IOStdReq *) ;
VOID cleanup () ;

#define NETSERVER       65

BYTE *b ;
BYTE *c ;
int i,j;

struct MsgPort *TSSReplyPort ;
ULONG TSSbit ;
ULONG TSSMask;

struct IOStdReq *timermsg ;
struct MsgPort *timerport ;
ULONG timerbit ;

ULONG wakeupbits ;



struct IOExtTSS *TSSreq ;

int message_size ;

        unsigned char node_name [10],node_num [2] ;
        int  errorcode, flag ;




#if 1

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   return(InitRDevice(global));
}

#else

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   return(1);
}

#endif

int InitRDevice(global)

GLOBAL global;

{
   struct NetNode *tmpnode;
   struct RPacket RP;

   unsigned char Buffer [5] ;

   BUGP("InitRDevice: Entry")



errorcode = open_lib (global) ;

   if(errorcode )
   {
      BUG(("********ERROR: Can't open tss!!\n"));
      BUGR("Can't open channel!")
   }



         Buffer[0] = NETSERVER ;
         TSSreq->IOTSS.io_Command = TSCMD_DCL_OBJECT ;
         TSSreq->IOTSS.io_Data = (APTR)&Buffer[0] ;
         TSSreq->IOTSS.io_Length = 1 ;
         DoIO (TSSreq) ;  /* return from here should have a valid connect */

         if (TSSreq->IOTSS.io_Error != 0)
           {
            BUG(("********ERROR: Can't get connect from handler machine!!\n"));
            BUGR("error on connection!")
            return (1) ;
           }

         TSSreq->IOTSS.io_Command = TSCMD_UCON_CONFIRM ;
         TSSreq->User.UserDataSize = 2 ;
         TSSreq->User.UserData[0] = 0 ;
         TSSreq->User.UserData[1] = 4 ;
         DoIO (TSSreq) ;

         global->n.devptr = (APTR)TSSreq ;


   BUGP("InitRDevice: Exit")

   return(0);
}




int open_lib (global)

GLOBAL global ;

{
 int errorcode ;
	

 TSSReplyPort = CreatePort (0L,0L) ;
 if (TSSReplyPort == (LONG)NULL)
   {
      BUG(("********ERROR: Can't Create TSS Reply Port!!\n"));
      BUGR("Can't Create TSS Reply Port!")
   }

 TSSreq = (struct IOExtTSS *)
    AllocMem ((long)sizeof(*TSSreq), MEMF_PUBLIC | MEMF_CLEAR) ;
 if (TSSreq == 0L)
   {
      BUG(("********ERROR: Can't get mem for tss req!!\n"));
      BUGR("Can't get mem for tss req!")
   }
 
 TSSreq->IOTSS.io_Message.mn_ReplyPort = TSSReplyPort ;

 if (errorcode=OpenDevice ("TSSnet.device", (LONG)NULL, TSSreq, (LONG)NULL))
   {
      BUG(("********ERROR: Can't open tss!!\n"));
      BUGR("Can't open tss!")
   }

 TSSbit = 1L << TSSReplyPort->mp_SigBit ;
 TSSMask = TSSbit | SIGBREAKF_CTRL_C;

timerport = (struct MsgPort *)CreatePort(0L, 0L) ;
timermsg = (struct IOStdReq *)CreateStdIO(timerport) ;
OpenDevice(TIMERNAME, (LONG)UNIT_VBLANK, timermsg, 0L) ;
timerbit = 1L << timerport->mp_SigBit ;

    
 return (errorcode) ;
}





VOID SetTimer(sec, micro, timermsg)
ULONG sec, micro;
struct IOStdReq *timermsg;
/* This routine simply sets the timer to interrupt us after secs.micros */
{
    timermsg->io_Command = TR_ADDREQUEST;    /* add a new timer request */
    timermsg->io_Actual = sec;    /* seconds */
    timermsg->io_Length = micro;    /* microseconds */
    SendIO(timermsg);	/* post a request to the timer */
}


int TermRDevice(global, status)
GLOBAL global;
int status;
{
   struct IOExtTSS *TSSreq ;
   unsigned char Buffer[4] ;
   struct NetNode *netnode;

         TSSreq = (struct IOExtTSS *)global->n.devptr ;
         if (TSSreq->User.Status > -80)
           {
            TSSreq->IOTSS.io_Command = TSCMD_DISCONNECT ;
            TSSreq->IOTSS.io_Length = 2 ;
            Buffer[0] = 0 ;
            Buffer[1] = 0 ;
            TSSreq->User.UserDataSize = 2 ;
            TSSreq->User.UserData[0] = 0 ;
            TSSreq->User.UserData[1] = 0 ;
            DoIO (TSSreq) ;
           }

         CloseDevice (timermsg) ;
         DeleteStdIO (timermsg) ;
         DeletePort (timerport) ;  
         CloseDevice (TSSreq) ;
         FreeMem (TSSreq, (long)sizeof(*TSSreq)) ;
         DeletePort (TSSReplyPort) ;

         DeletePort(global->n.devport);

   return(0);
}

void ActNetHello(global, pkt)
GLOBAL global;
struct DosPacket *pkt;
{
}
