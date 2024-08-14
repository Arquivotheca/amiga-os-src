/*
**
**	Envoy Talk -- real-time interactive messaging (ala BSD 'talk').
**
**  This is an example of a program which does not follow the client-server
**  model.  Both sides use the same binary.  Both initiate and respond to 
**  Transactions.
**
**  From the programmer perspective, the ability to initiate
**  network transactions from both sides of a connection requires 4 Entities --
**  two on each side.  It also shows, that from the application point-of-view,
**  an Entity is an in-Entity or an out-Entity.  In-Entities are always created
**  with CreateEntity() and freed with DeleteEntity().  Out-Entities are always
**  created with FindEntity() and freed with LoseEntity().  In reality, there
**  is only one Entity for each pair of in/out, but each side of the application
**  "sees" two.
**
**  This talk includes no way to notify the remote party that you wish to talk.
**  That would require a server of some sort.
**
**  It takes 5 or 6 seconds from the time of calling FindEntity() to printing
**  "Waiting for host to talk" if the remote machine isn't already running
**  nipc.library.  This is necessary because NIPC can't tell if the machine
**  isn't responding or if it is just taking a little while for it to answer.
**  Note that talk will wait for even non-existent hosts to talk.
**
**  Potential NIPC problems:
**
**  If one side exits, the other side will get a timeout when it trys to DoTransaction().
**  It shouldn't have to wait 10 seconds and it's error code should be tha the Entity
**  has been deleted, not that a timeout occured.
*/
#include <exec/types.h>
extern struct Library *SysBase;
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
extern struct Library *DOSBase;
#include <dos/dos.h>
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <envoy/nipc.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>

#include <stdlib.h>
#include <stdio.h>

#define EVER ;;

struct Library *NIPCBase = NULL;
struct Entity *MyEnt, *HisEnt = NULL;
struct Transaction *MyTrans;
ULONG EntMask;	/*  Wait() mask for MyEnt  */
BPTR Fh;	/*  Input filehandle  */
#define LEN (81)
UBYTE Buf[LEN];

VOID init(int argc, char *argv[]);
BOOL connect(char *argv[]);
BOOL talk(BOOL);
VOID cleanup(LONG);
BOOL receive(BOOL outstanding);

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */

main(int argc, char *argv[])
{
	init(argc, argv);
	cleanup( talk( connect(argv) ) );
}


BOOL connect(char *argv[])
{
struct Transaction *hisTrans;
ULONG error, 
      tweak;	/*  return value from Wait() -- tweaked sig bits  */
/*
**  If we find the entity on the other side, send to it that we have.
**  If we haven't, wait until we get a ctrl-c or we get a message from
**  the other side saying that they have found us and then find them.
**  Unfortunately, someone finding you does *not* automatically make it
**  so that you have found them.
*/
	printf("Contacting %s.\n",argv[1]);
	if(HisEnt = FindEntity(argv[1], "etalk", MyEnt, &error))
	{
		printf("Connected to %s.\n", argv[1]);
		sprintf(Buf, "Connect.\n");  /*"from %s.\n", localhost);*/
		return TRUE;
	}else
	{
		printf("Waiting for %s to talk...\n", argv[1]);
		tweak = Wait(EntMask | SIGBREAKF_CTRL_C);
		if(tweak & SIGBREAKF_CTRL_C)
		{
			printf("^C\n");
			cleanup(RETURN_OK);
		}
//		hisTrans = GetTransaction(MyEnt);
//		GetHost(hisTrans);
//		if(strcmp );
		if(HisEnt = FindEntity(argv[1], "etalk", MyEnt, &error))
		{
			return FALSE;
		}
/*
**  This is possible.  It means that they send us something then promptly
**  died (or the network did).  We need to reply the transaction and exit.
**  Since we haven't actually done a GetTransaction(), we can just cleanup,
**  because DeleteEntity() will do the replies for us.
*/
		printf("Connection lost.\n"); 
		cleanup(RETURN_ERROR);
	}
}


BOOL talk(BOOL gotconnect)
{
ULONG tweak;	/*  return value from Wait() -- tweaked sig bits  */
LONG err, 	/*  IoErr() value */
     status, 	/*  WaitForChar() status */
     actual;	/*  Read() return value */
int n=0;	/*  Number of chars in Buf  */
BOOL outstanding = FALSE;	/*  Is MyTrans pending?  */
/*
**  Now that we have my Entity and his Entity, we loop forever, printing
**  lines from the other side, sending lines from our side, and aborting when
**  we get a ctrl-c or a Transaction error.  In order to simplify input 
**  processing, we use WaitForChar() rather than opening a window and 
**  console.device or using DOS Packets.  This means that we can't Wait()
**  on our Entity or for ctrl-c.  Instead, we check our signals with SetSignal()
**  every time WaitForChar() returns (about twice a second with the arguments
**  we use).
*/

	if(gotconnect)
	{
		receive(outstanding);
	}
	else
	{
		BeginTransaction(HisEnt, MyEnt, MyTrans);
		outstanding = TRUE;
	}
	for(EVER)
	{
		if(outstanding)
		{
			tweak = Wait(EntMask | SIGBREAKF_CTRL_C);
		}else
		{
			tweak = SetSignal(0,EntMask);  /*  clear it when we read it  */
		}
		if(tweak & EntMask)
		{
			outstanding = receive(outstanding);
		}
		if(tweak & SIGBREAKF_CTRL_C)
		{
			printf("^C\n");
			if(outstanding)
			{
				AbortTransaction(MyTrans);
				WaitTransaction(MyTrans);
			}
			return RETURN_OK;
		}
		if(outstanding)
		{
			continue;
		}
		status = WaitForChar(Fh, 500000);  /* wait half a second */
		if(status == FALSE)
		{
			continue;
		}
		actual = Read(Fh, Buf+n, 1);
		if(!actual)
		{
			err = IoErr();
			if(err == 0)
			{
				printf("Local input EOF.\n");
			}else
			{
				printf("IoErr = %ld on FGets()!\n", err);
			}
			if(outstanding)
			{
				AbortTransaction(MyTrans);
				WaitTransaction(MyTrans);
			}
			return RETURN_ERROR;
		}
		if(n+2 > LEN)
		{
			Buf[++n] = '\n';
		}
		if(Buf[n++] == '\n')
		{
			Buf[n] = '\0';
			MyTrans->trans_ReqDataActual = n+1;
			BeginTransaction(HisEnt, MyEnt, MyTrans);
			outstanding = TRUE;
			n = 0;
		}
	}
}


BOOL receive(BOOL outstanding)
{
struct Transaction *hisTrans;

	while(hisTrans = GetTransaction(MyEnt))
	{
		if(hisTrans->trans_Error)
		{
			printf("Transaction error code %ld.\n", hisTrans->trans_Error);
/*
**  It could be that this *is* a replied transaction so we don't have to abort,
**  but the abort doesn't hurt and saves us an extra conditional.
*/
			if(outstanding)
			{
				AbortTransaction(MyTrans);
				WaitTransaction(MyTrans);
			}
			cleanup(RETURN_ERROR);
		}
//		printf("type = %ld\n", hisTrans->trans_Type);
//		if(hisTrans->trans_Type == TYPE_RESPONSE)
		if(hisTrans == MyTrans)
		{
			outstanding = FALSE;
		}else
		{
			printf(">%s", hisTrans->trans_RequestData);
			ReplyTransaction(hisTrans);
		}
	}
	return outstanding;
}


VOID init(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s hostname.\n", argv[0]);
		cleanup(RETURN_ERROR);
	}
	if( !(NIPCBase = OpenLibrary("nipc.library", 0L)) )
	{
		printf("Can't open nipc.library.\n");
		cleanup(RETURN_FAIL);
	}
	Fh = Open("CONSOLE:", MODE_OLDFILE);
	if(!Fh)
	{
		printf("Can't open CONSOLE:.\n");
		cleanup(RETURN_FAIL);
	}
	MyEnt = CreateEntity(ENT_Name, "etalk",
			  ENT_AllocSignal, &EntMask,
			  ENT_Public, 0L,
			  TAG_DONE);
	EntMask = 1 << EntMask;
	if(!MyEnt)
	{
		printf("Can't CreateEntity.\n");
		cleanup(RETURN_FAIL);
	}
	if( !(MyTrans = AllocTransaction(TAG_DONE)) )
	{
		printf("Can't AllocTransaction.\n");
		cleanup(RETURN_FAIL);
	}
	MyTrans->trans_ResponseData = NULL;
	MyTrans->trans_RespDataLength = 0;
	MyTrans->trans_RespDataActual = 0;
	MyTrans->trans_RequestData = Buf;
	MyTrans->trans_ReqDataLength = LEN;
	MyTrans->trans_ReqDataActual = LEN;
	MyTrans->trans_Timeout = 1;  /* Max one second to process. */
}


VOID cleanup(LONG rc)
{
	if(NIPCBase)
	{
		LoseEntity(HisEnt); 
		DeleteEntity(MyEnt);
		Close(Fh);
		CloseLibrary(NIPCBase);
	}
printf("I'm outta here.\n");
	exit(rc);
}

