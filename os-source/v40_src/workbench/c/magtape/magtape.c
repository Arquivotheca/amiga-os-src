#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <devices/timer.h>
#include <libraries/dosextens.h>
#include "devices/scsidisk.h"


#include <string.h>

#include "internal/commands.h"

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "clib/intuition_protos.h"

#include "pragmas/dos_pragmas.h"
#include <pragmas/exec_pragmas.h>

#include "magtape_rev.h"

#define fsf 1

#define CMD_SCSIDIRECT  28              /* send direct scsi command */
#ifdef fsf
#define TEMPLATE    "DEVICE/K,UNIT/N/K,RET=RETENSION/S,REW=REWIND/S,SKIP/N/K" CMDREV
#else
#define TEMPLATE    "DEVICE/K,UNIT/N/K,RET=RETENSION/S,REW=REWIND/S" CMDREV
#endif
#define OPT_DEVICE	0
#define OPT_UNIT	1
#define OPT_RET		2
#define OPT_REW		3
#define OPT_FSF		4
#define OPT_COUNT   	5

void aprintf();

int cmd_mt(void)
{
        struct Library *DOSBase;
        struct Library *SysBase = (*((struct Library **) 4));
	long opts[OPT_COUNT];
        struct RDargs *rdargs;
	long rc=RETURN_FAIL;
	struct IOStdReq __aligned SCSIReq={0};
	struct SCSICmd __aligned Cmd={0};
	UBYTE  __aligned Sense[18];
	struct MsgPort __aligned Port={0};
	int i;
#ifdef fsf
        long nfile;
#endif
	UBYTE  TestUnitReady[] = { 0,0,0,0,0,0 };
	UBYTE  RetensionUnit[] =  { 0x1b,0,0,0,3,0 };
	UBYTE  RewindUnit[]    =  { 0x01,0,0,0,0,0 };
#ifdef fsf
	UBYTE  ForwardUnit[]   =  { 0x11,0x01,0,0,0,0 };
#endif
	UBYTE *device="scsi.device";
	int devok=0;
	int unit=4;

   if ((DOSBase = OpenLibrary(DOSLIB, DOSVER))) {

      memset((char *)opts, 0, sizeof(opts));
      if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL)))goto err;

    if(opts[OPT_DEVICE])device=(UBYTE *)opts[OPT_DEVICE];
    if(opts[OPT_UNIT])unit=*(long *)opts[OPT_UNIT];

    /* fire up a handler someone is passing us ? */
    for(i=0; i <strlen(device); i++)if(device[i]==':') {
	if(!IsFileSystem(device)) {
	    aprintf("Error: not a device.\n");
	    goto err;
	}
	SetIoErr(0);
    }
    if((strlen(device) > 7)&&!stricmp(&device[strlen(device)-7],".device")) {
        Port.mp_Node.ln_Pri = 0;
        Port.mp_SigBit      = AllocSignal(-1);
        Port.mp_SigTask     = (struct Task *)FindTask(0);
        NewList( &(Port.mp_MsgList) );
        SCSIReq.io_Message.mn_ReplyPort   = &Port;
        if(Port.mp_SigBit)devok=(!OpenDevice(device, unit, &SCSIReq, 0));
    }
    if (!devok) {
	aprintf("Error: device didn't open.\n");

	goto err;
    }
    SCSIReq.io_Length  = sizeof(struct SCSICmd);
    SCSIReq.io_Data    = (APTR)&Cmd;
    SCSIReq.io_Command = CMD_SCSIDIRECT;
    Cmd.scsi_Data = (APTR)0;
    Cmd.scsi_Length = 0;
    Cmd.scsi_CmdLength = 6;
    Cmd.scsi_Flags = SCSIF_AUTOSENSE;
    Cmd.scsi_SenseData = (APTR)Sense;
    Cmd.scsi_SenseLength = 18;
    Cmd.scsi_SenseActual = 0;

    Cmd.scsi_Command = (APTR)TestUnitReady;
    DoIO( &SCSIReq );
    if(Cmd.scsi_Status){
	aprintf("Unit not ready, scsi status is %ld\n",Cmd.scsi_Status);
	goto err;
    }

    if(opts[OPT_RET]) {
        Cmd.scsi_Command = (APTR)RetensionUnit;
	DoIO( &SCSIReq );
	if(Cmd.scsi_Status) {
	    aprintf("error %ld on retension\n",Cmd.scsi_Status);
	    goto err;
	}
    }
    if(opts[OPT_REW]) {
        Cmd.scsi_Command = (APTR)RewindUnit;
	DoIO( &SCSIReq );
	if(Cmd.scsi_Status) {
	    aprintf("error %ld on rewind\n",Cmd.scsi_Status);
	    goto err;
	}
    }
#ifdef fsf
    if(opts[OPT_FSF]) {
        Cmd.scsi_Command = (APTR)ForwardUnit;
	nfile= *(long *)opts[OPT_FSF];
	ForwardUnit[2]=(UBYTE)(nfile/4096)&255;
	ForwardUnit[3]=(UBYTE)(nfile/256)&255;
	ForwardUnit[4]=(UBYTE)(nfile&255);
	DoIO( &SCSIReq );
	if(Cmd.scsi_Status) {
	    aprintf("error %ld on fast forward\n",Cmd.scsi_Status);
	    goto err;
	}
    }
#endif
        rc=RETURN_OK;
err:;
	if (rdargs)FreeArgs(rdargs);
        if(Port.mp_SigBit)FreeSignal(Port.mp_SigBit);
        if(devok)CloseDevice( &SCSIReq );
        if(Port.mp_SigBit)FreeSignal(Port.mp_SigBit);
	if(rc)PrintFault(IoErr(),NULL);
        CloseLibrary((struct Library *)DOSBase);
   }
   else {OPENFAIL;}
   return(rc);
}

#if 0
void ShowSenseData()
{
int i;

    for(i=0; i<18; i++)
        aprintf("%x ",(int)Sense[i]);
    aprintf("\n");
}
#endif

void aprintf( fmt, args )
char *fmt, *args;
{
   struct Library *DOSBase;
   struct Library *SysBase = (*((struct Library **) 4));

   if(DOSBase=OpenLibrary(DOSLIB,DOSVER)) {
       VFPrintf(Output(), fmt, (LONG *)&args );
       CloseLibrary(DOSBase);
   }
}
