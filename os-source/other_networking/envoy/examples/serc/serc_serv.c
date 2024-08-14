/*
**  serc_serv.c  -- Simple Envoy Remote Command example program, server.  
**	      Requires NIPC.
**
**  Copyright 1992, Commodore-Amiga, Inc.
**  Permission to use granted this notice remains intact.
**
*/
#include <exec/exec.h>
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

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */

#define EVER ;;
void docmd(struct Transaction *trans);

struct Library *NIPCBase;

main(int argc, char *argv[])
{
struct Entity *entity;
struct Transaction *trans;
ULONG entmask, tweak;

	if( !(NIPCBase = OpenLibrary("nipc.library", 0)) )
	{
		printf("Can't open NIPC.\n");
		exit(RETURN_FAIL);
	}
	entity = CreateEntity(ENT_Name, "serc_server",
			  ENT_AllocSignal, &entmask,
			  ENT_Public, 0L,
			  TAG_DONE);
	if(!entity)
	{
		printf("Can't create entity.\n");
		CloseLibrary(NIPCBase);
		exit(RETURN_FAIL);
	}
	entmask = 1 << entmask;
	for(EVER)
	{
		tweak = Wait(entmask | SIGBREAKF_CTRL_C);
		if(tweak & SIGBREAKF_CTRL_C)
		{
			break;
		}
		if(tweak & entmask)
		{
			/* trans should always be valid in this particular case, but... */
			if( trans = GetTransaction(entity) )
			{
				docmd(trans);
				ReplyTransaction(trans);
			}
		}
	}
	DeleteEntity(entity);
	CloseLibrary(NIPCBase);
	exit(RETURN_OK);
}


void docmd(struct Transaction *trans)
{
LONG oldpos;
BPTR outputfh;

	if( !(outputfh = Open("t:serc_command_output", MODE_NEWFILE)) )
	{
		printf("Can't open temp file.\n");
	}
	trans->trans_Error = SystemTags(trans->trans_RequestData,
	         			SYS_Input, NULL,
		 	   		SYS_Output, outputfh,
			   		NP_Name, "ERC Command",
			   		NP_Priority, -1,
			   		TAG_DONE);
/* A fancier implementation might also want to deal with NP_CurrentDir,
 * and NP_StackSize */
	oldpos = Seek(outputfh,0,OFFSET_BEGINNING);
	if(oldpos == -1)
	{
		printf("Seek error.\n");
	}
	if(oldpos+1 > trans->trans_RespDataLength)
	{
		oldpos = trans->trans_RespDataLength-1;
	}
	trans->trans_RespDataActual = Read(outputfh, trans->trans_ResponseData, 
					   oldpos);
	((char *)trans->trans_ResponseData)[oldpos] = '\0';
	trans->trans_RespDataActual++;
	Close(outputfh);
	DeleteFile("t:serc_command_output");
}
