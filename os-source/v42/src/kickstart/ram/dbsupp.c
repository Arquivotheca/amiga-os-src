/* dbsupp.c */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/alib_protos.h>

#include "dbwin.h"

char buffer[255];
struct MsgPort *port = NULL;
struct MsgPort *myport = NULL;
struct Message *msg  = NULL;

void
dbwrite (str)
	char *str;
{
	struct Message *tmsg;

	if (!port)
	{
		port = FindPort("dbwin.port");
		if (port)
		{
			msg = AllocMem(sizeof(struct Message),MEMF_PUBLIC);
			if (!msg)
				port = NULL;
		}
	}
	if (!myport)
	{
		myport = CreateMsgPort();
		if (myport) {
			myport->mp_Node.ln_Name = "myram";
			AddPort(myport);
		}
	}
	if (port && myport)
	{
		msg->mn_ReplyPort    = myport;
		msg->mn_Node.ln_Name = str;
		PutMsg(port,msg);
		while (1) {
			WaitPort(myport);
			while (tmsg = GetMsg(myport))
				if (msg == tmsg)
					goto done;
		}
done:		;
	}
}

void
dbwrite1 (str,p1)
	char *str;
	long p1;
{
	sprintf(buffer,str,(BYTE *) p1);
	dbwrite(buffer);
}

void
dbwrite2 (str,p1,p2)
	char *str;
	long p1;
	long p2;
{
	sprintf(buffer,str,(BYTE *) p1,p2);
	dbwrite(buffer);
}
