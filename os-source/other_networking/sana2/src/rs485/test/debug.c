/*
 * debug message catcher
 */

#include "debug.h"

#include <libraries/dos.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char **argv);

int main(argc, argv)
	int argc;
	char **argv;
{
	struct DebugMessage *dbmsg;
	struct MsgPort *dbport;
	long event;

	Forbid();
	if(FindPort(DEBUGPORT)){
		printf("debug already running!!\n");
		exit(RETURN_WARN);
	}

	dbport = CreatePort(DEBUGPORT, 0);
	if(!dbport){
		puts("Could not create port\n");
		exit(RETURN_FAIL);
	}

	do {
		event = Wait(SIGBREAKF_CTRL_C | (1L << dbport->mp_SigBit));

		while(dbmsg = (struct DebugMessage *)GetMsg(dbport)){
			printf(dbmsg->buf); fflush(stdout);
			FreeMem(dbmsg, sizeof(*dbmsg));
		}

	} while(!(event & SIGBREAKF_CTRL_C));
	
	DeletePort(dbport);
	exit(RETURN_OK);
}
