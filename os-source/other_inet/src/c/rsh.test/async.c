/*
**	nonblocking, async I/O functions
**
**	Martin Hunt	Mar 1992
*/

#include "aio.h"
#include <stdio.h>
#include <proto/all.h>

BOOL set_screen(AFH *af, int mode)
{
		af->pk.sp_Msg.mn_Node.ln_Name = (char *)&(af->pk.sp_Pkt);
		af->pk.sp_Pkt.dp_Link = &(af->pk.sp_Msg);
		af->pk.sp_Pkt.dp_Port = af->rp;
		af->pk.sp_Pkt.dp_Type = ACTION_SCREEN_MODE;
		af->pk.sp_Pkt.dp_Arg1 = mode;

		PutMsg(af->fh->fh_Type,&af->pk.sp_Msg);
		WaitPort(af->rp);
		GetMsg(af->rp); 
		return((BOOL)ARes1(af));
}

AFH *AOpen(char *name, LONG mode)
{
	AFH *af;

	af = (AFH *)AllocMem(sizeof(AFH),MEMF_PUBLIC);
	if(af==NULL) 
		return NULL;

	if(( af->fh = (struct FileHandle *)Open(name,mode)) ==NULL) {
		FreeMem(af,sizeof(AFH));
		return NULL;
	}
	af->fh = BADDR(af->fh);	/* convert BPTR to pointer */

	af->rp = CreateMsgPort();	/* create our replyport */
	set_screen(af,-1);
	return(af);
}

void AClose(AFH *af)
{

	/* read all the outstanding messages */
	WaitPort(af->rp);
	while(GetMsg(af->rp)) ;

	set_screen(af,0);

	DeleteMsgPort(af->rp);

	Close(MKBADDR(af->fh));
	FreeMem(af,sizeof(AFH));
}


static void sendit(AFH *af,void *buf, LONG size, LONG action)
{
	af->pk.sp_Msg.mn_Node.ln_Name = (char *)&(af->pk.sp_Pkt);
	af->pk.sp_Pkt.dp_Link = &(af->pk.sp_Msg);
	af->pk.sp_Pkt.dp_Port = af->rp;
	af->pk.sp_Pkt.dp_Type = action;
	af->pk.sp_Pkt.dp_Arg1 = af->fh->fh_Arg1;
	af->pk.sp_Pkt.dp_Arg2 = (LONG)buf;
	af->pk.sp_Pkt.dp_Arg3 = size;
	PutMsg(af->fh->fh_Type,&(af->pk.sp_Msg));
	af->pending++;
}

void ARead(AFH *af,void *buf, LONG size)
{
	sendit(af,buf,size,ACTION_READ);
}


void AWrite(AFH *af, void *buf, LONG size)
{
	sendit(af,buf,size,ACTION_WRITE);
}


