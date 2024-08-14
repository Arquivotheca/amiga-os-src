
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
#include "dn.h"



#include "netdnet.h"
#include "handler.h"

#define Timeout -101	


#ifdef CPR
char *dbgwind = "CON:0/0/640/160/NETTSS-HANDLER/a";
#endif


int open_lib (GLOBAL);
int TSS_connect (int, unsigned char *, unsigned char *) ;

/*******************************************************************/
/*                                                                 */
/* I had trouble linking in my routines that displayed the node    */
/* choice screen - these 3 items + the xcexit one below came from  */
/* the startup code - I put these in to fake it out.  Showing      */
/* my lack of familiarity with the Lattice compiler...             */
/*                                                                 */
/*******************************************************************/


ULONG _oserr ;
ULONG _ONBREAK ;
ULONG _OSERR ;



VOID SetTimer (unsigned long, unsigned long, struct IOStdReq *) ;
VOID cleanup () ;

#define NETSERVER       65    /* the Object Number I assigned to NET: */

BYTE *b ;
BYTE *c ;
int i,j;

struct MsgPort *TSSReplyPort ;
ULONG TSSMask ;
ULONG TSSbit ;

struct IOStdReq *timermsg ;
struct MsgPort *timerport ;
ULONG timerbit ;

ULONG wakeupbits ;



struct IOExtTSS *TSSreq = NULL;



int message_size ;

        unsigned char node_name [10],node_num [2] ;
        int  errorcode, flag ;


void XCEXIT (ab)
int ab ;
 {
  return;
 }






#if 1

int ReSync(global, ioptr)
GLOBAL global;
APTR ioptr;
{
   struct NetNode *tmpnode;

BUG(("ReSync: ioptr 0x%08lx\n", ioptr));

   InitRDevice(global);

BUG(("ReSync: Setting all ioptrs to 0x%08lx\n", TSSreq));

   for(tmpnode=global->netchain.next; tmpnode; tmpnode=tmpnode->next)
      tmpnode->ioptr = (APTR)TSSreq;

BUG(("ReSync: Exit\n"));

   return(TSSreq != NULL);
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
   unsigned char node_name[8] ;
   struct sockaddr_dn sa ;
   struct NetNode *tmpnode;

   BUG(("InitRDevice: Initializing for TSS operation on unit %d\n",
        global->unitnum));

   BUGP("InitRDevice: Entry")

   if(!global->RP.DataP) 
      SetPacketBuffer(&global->RP, NETBUFSIZE);

   if(!TSSreq)
   {
      errorcode = open_lib (global) ;

      if(errorcode )
      {
         BUG(("********ERROR: Can't open tss!!\n"));
         BUGR("Can't open channel!")
         return (0) ;
      }

/* process_node_def gets a node name to connect to using a screen */
/* with a list of the nodes in tss:tssnodes (all the nodes this   */
/* node knows about)                                              */


      errorcode = process_node_def (&node_name[0]) ; 

/* then get the node address for the node name returned by process_node_def */

      sa.sdn_add.a_addr[0] = 0 ;
      sa.sdn_add.a_addr[1] = 0 ;
      sa.sdn_objnamel= strlen (node_name) ;
      CopyMem (&node_name[0], sa.sdn_objname, (LONG)sa.sdn_objnamel) ;

      TSSreq->IOTSS.io_Command = TSCMD_FIND_NODE_NAME ;
      TSSreq->IOTSS.io_Data = (APTR)&sa ;
      DoIO (TSSreq) ;
       
      node_num[0] = sa.sdn_add.a_addr[0] ;
      node_num[1] = sa.sdn_add.a_addr[1]  ;
   
      errorcode=TSS_connect(NETSERVER, node_num, "TSSnet") ;
      if(errorcode)
      {
         BUG(("********ERROR: Can't connect to server machine!!\n"));
         BUGR("Can't connect!")
         return (0) ;
      }
   }

   BUG(("InitRDevice: Exit, tss Channel %lx open\n", TSSreq));

   BUGP("InitRDevice: Exit")

   return(TSSreq != NULL);
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
TSSMask = TSSbit;


timerport = (struct MsgPort *)CreatePort(0L, 0L) ;
timermsg = (struct IOStdReq *)CreateStdIO(timerport) ;
OpenDevice(TIMERNAME, (LONG)UNIT_VBLANK, timermsg, 0L) ;
timerbit = 1L << timerport->mp_SigBit ;

    
 return (errorcode) ;
}





int TSS_connect (objectnum,nodenum,ourname)

char objectnum, nodenum[2], ourname[13] ;

{
struct Message *msg ;
BOOL done ;
int errorcode, i ;

/** Define our connect request structure **/
struct tss_connect
		{
		char	node_number[2];
		char	fmt0;
		char	object_num;
		char	fmt2;
		char	must_be_zero;
		char	groupcode[2];
		char	usrcode[2];
		char	len_descrpt;
		char	descrpt [12];
		char	options;
		char	userdata[3];
	    } connect;
	
	
/** now we set up our connect request **/

	connect.node_number [0] = nodenum [0];
	connect.node_number [1] = nodenum [1];
	connect.fmt0	= 0;
	connect.object_num = objectnum;
	connect.fmt2	= 2;
	connect.must_be_zero = 0;
	connect.groupcode[0] = connect.groupcode[1] = 0;
	connect.usrcode[0] = connect.usrcode[1] = 0;
	
/** for good looks we tuck our own name into the connect request
	so that the remote system can display who is connected to it.
	Note that this connection does not pass a username or password
	for network connect; i.e., it uses the default DECnet account **/
	
	connect.len_descrpt	 = 12;
	
	for (i=0;i<6;i++)
           connect.descrpt[i]=ourname[i] ;
			
	for (i;i<12;connect.descrpt[i++]=' ')
			;
			
	
	connect.options = 2; /* userdata follows */
	
	connect.userdata[0] = 2;   /* two bytes of data follows */
	connect.userdata[1] = 0 ;
	connect.userdata[2] = 0 ;
	
/** everything is now formatted - let's issue the connect request! **/

        TSSreq->IOTSS.io_Command = TSCMD_CONNECT ;
        TSSreq->IOTSS.io_Data = (APTR)&connect ;
        TSSreq->IOTSS.io_Length = sizeof(connect) ;
        

	SendIO (TSSreq) ;

        SetTimer (30L, 0L, timermsg) ; /* time it out after 30 sec */

        done = FALSE ;
        errorcode = 0 ;
        while (!done)
          {
           wakeupbits = Wait (timerbit | TSSbit) ;
           msg = GetMsg (TSSReplyPort) ;
           if (msg != (struct Message *)0L)
             {
              done = TRUE ;
              errorcode = TSSreq->User.Status ;
BUG(("TSS_connect: Back from SendIO, status %d io_Error %d\n", 
      TSSreq->User.Status, TSSreq->User.Status, TSSreq->IOTSS.io_Error));
              AbortIO (timermsg) ;
              WaitIO (timermsg) ;
              GetMsg (timerport) ;
              SetSignal (0L, timerbit) ;
             }
           else
             {
BUG(("TSS_connect: Connection timed out\n"))
              msg = GetMsg (timerport) ;
              if (msg != (struct Message *)0L) 
                {
                 done = TRUE ;
                 errorcode = 1 ;
                 AbortIO (TSSreq) ;
                 WaitIO (TSSreq) ;
                 GetMsg (TSSReplyPort) ;
                 TSSreq->IOTSS.io_Command = TSCMD_EXP_PORT_DELETE ;
                 DoIO (TSSreq) ;
                }
             }

          }

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
   unsigned char Buffer[4] ;
   struct NetNode *netnode;
   int didit;

BUG(("TermRDevice: Status %d\n", status));

   for(netnode=global->netchain.next; netnode; netnode=netnode->next)
   {
      netnode->status = NODE_DEAD;
      netnode->incarnation++;
      netnode->RootLock.incarnation = netnode->incarnation;
      netnode->ioptr = NULL;
   }

   if(TSSreq == NULL) 
   {
BUG(("TermRDevice: Early exit\n"));
      return(0);
   }

   if(!status)
   {
      if (TSSreq->User.Status > -80) /* still alive - needs a disconnect */
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
   }
   CloseDevice (timermsg) ;
   DeleteStdIO (timermsg) ;
   DeletePort (timerport) ;  
   CloseDevice (TSSreq) ;
   FreeMem (TSSreq, (long)sizeof(*TSSreq)) ;
   DeletePort (TSSReplyPort) ;
   TSSreq = NULL;

BUG(("TermRDevice: Exit\n"));
   return(0);
}
