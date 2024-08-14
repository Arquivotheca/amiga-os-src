/* dbwin */

#include <exec/types.h>
#include <exec/ports.h>
#include <libraries/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

void
main (argc,argv)
	int argc;
	char **argv;
{
	struct MsgPort *port;
	struct Message *msg;
	BPTR fh;

	port = CreatePort("dbwin.port",1);
	if (!port)
		exit(20);

	fh = Open("CON:320/0/320/200/dbwin debug window",MODE_NEWFILE);
	if (!fh)
	{
		DeletePort(port);
		exit(20);
	}

	while (1) {
		while ((msg = GetMsg(port)) == NULL)
			WaitPort(port);

		Write(fh,msg->mn_Node.ln_Name,strlen(msg->mn_Node.ln_Name));
		Write(fh,"\n",1L);
		ReplyMsg(msg);
	}

	/* not reached */
	exit(0);
}
