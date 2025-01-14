#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include "devices/narrator.h"
#include "handler.h"
#include "speak-handler_rev.h"


ULONG TranslatorBase;
BYTE amaps[] = {3, 5, 10, 12};   /* audio channel allocation maps */

char copyright[] = 
  "Copyright 1987-1990 Commodore-Amiga, Inc. All Rights Reserved";
char vers[]=VERTAG;

main()
{
 /* AmigaDos/Handler  related data structures */
struct DosPacket      	*packet;      /* a pointer to the  dos packet sent */
struct Process        	*process;     /* pointer to our process */
UBYTE                 	*parmdevname; /* pointer to device name in parmpkt Arg1 */
ULONG                 	parmextra;    /* extra info passed in parmpkt      Arg2 */
struct DeviceNode     	*node;        /* our device node passed in parmpkt Arg3 */
struct FileHandle     	*fh;          /* a pointer to our file handle           */
ULONG			*point;
struct VARS 		*vars;	      /* pointer to our variable block	*/

packet = taskwait();	/* wait for parm. packet */

parmdevname = (UBYTE *) BADDR(packet->dp_Arg1); /* name for handler */
parmextra = (ULONG) packet->dp_Arg2;  		/* extra info */
node = (struct DeviceNode *) BADDR(packet->dp_Arg3);
process = (struct Process *) FindTask(0);


if(!(vars=(UBYTE *)AllocMem(sizeof(struct VARS),MEMF_PUBLIC|MEMF_CLEAR)))
	cleanup(packet,vars);

SetVars(packet,vars,parmdevname,process);

/* node->dn_Task = &process->pr_MsgPort;	/* only one gets fired up at a time */

returnpkt(packet,DOS_TRUE,packet->dp_Res2);    /* everything ok */

  while(TRUE) {
   packet = taskwait();                                /* wait for packet */
   switch(packet->dp_Type) {
     case ACTION_FIND_OUTPUT: 
          fh = (struct FileHandle *) BADDR(packet->dp_Arg1);  
          fh->fh_Port = (struct MsgPort *)DOS_TRUE;
          vars->OpenCount++;
          returnpkt(packet,DOS_TRUE,packet->dp_Res2);
          break;

     case ACTION_END:
          vars->OpenCount--;
          returnpkt(packet,DOS_TRUE,packet->dp_Res2);
          if((vars->OpenCount)<=0) {
	    clean(vars);
	    exit(0);
	  }
          break;

     case ACTION_WRITE:
	 Say((UBYTE *)packet->dp_Arg2,(packet->dp_Arg3)-1,vars);
         returnpkt(packet,packet->dp_Arg3,packet->dp_Res2);   
         break;

     case ACTION_DISK_INFO: {
	struct InfoData *v=(struct InfoData *)BADDR(packet->dp_Arg1);
        v->id_DiskType=  (LONG) IDS;
	returnpkt(packet,DOS_TRUE,0);
        break;
     }
     default:
	returnpkt(packet,FALSE,ERROR_ACTION_NOT_KNOWN);
/*        node->dn_Task = 0; */
	if(vars->OpenCount <= 0) {
	    clean(vars);
	    exit(0);
	}
   }
  }

}

clean(vars)
struct VARS *vars;
{
if(vars->TalkDevice)CloseDevice(vars->iow);
if(vars->iow)DeleteExtIO(vars->iow,(ULONG)sizeof(*(vars->iow)));
if(vars->devport)DeletePort(vars->devport);
if(TranslatorBase)CloseLibrary(TranslatorBase);
if(vars->buffer)FreeMem(vars->buffer,MAXSIZE+2);
if(vars->phoneme)FreeMem(vars->phoneme,MAXOSIZE+2);

if(vars)FreeMem(vars,sizeof(struct VARS));
return;
}

cleanup(packet,vars)
struct DosPacket *packet;
struct	 VARS *vars;
{
clean(vars);
returnpkt(packet,FALSE,RETURN_FAIL);
exit(0);
}

SetVars(packet,vars,parmdevname,process)
struct DosPacket *packet;
struct   VARS *vars;
UBYTE *parmdevname;
struct Process *process;
{
int i;


if(!((vars->buffer)=(UBYTE *)AllocMem(MAXSIZE+2,MEMF_PUBLIC|MEMF_CLEAR)))
	cleanup(packet,vars);

if(!((vars->phoneme)=(UBYTE *)AllocMem(MAXOSIZE+2,MEMF_PUBLIC|MEMF_CLEAR)))
	cleanup(packet,vars);

vars->clientport = packet->dp_Port;    /* get senders port (taskid) */

if(!(vars->devport=(struct MsgPort *)CreatePort("Chatter",0)))
	cleanup(packet,vars);

if(!(TranslatorBase=OpenLibrary("translator.library",0)))
	cleanup(packet,vars);

if(!(vars->iow=(struct narrator_rb *)
    CreateExtIO(vars->devport,(ULONG)sizeof(*(vars->iow))))) {
	cleanup(packet,vars);
}

vars->TalkDevice = (OpenDevice("narrator.device",0,vars->iow,0) == 0);
if(!(vars->TalkDevice))cleanup(packet,vars);

vars->iow->ch_masks= amaps;
vars->iow->nm_masks= sizeof(amaps);
vars->iow->mouths  = 0;   /* mouth OFF */

vars->iow->sex = DEFSEX;
vars->iow->mode = DEFMODE;
vars->iow->rate = DEFRATE;
vars->iow->pitch = DEFPITCH;

for (i = 0; ( (parmdevname[i]) && (parmdevname[i] != ':')) ; i++);
if (parmdevname[i] == ':')SetOpt(&parmdevname[i+1],TRUE,vars);

vars->iow->message.io_Message.mn_ReplyPort = vars->devport;
vars->iow->message.io_Command = CMD_WRITE;
vars->iow->message.io_Offset = 0;
vars->iow->message.io_Message.mn_Length = sizeof(*(vars->iow));
vars->iow->message.io_Message.mn_Node.ln_Type = 0;

vars->OpenCount=0;

return(0);
}
