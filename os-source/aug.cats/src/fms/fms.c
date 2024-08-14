
/*
 *  FMS.C
 *
 *  File Disk Device (fmsdisk.device)
 *
 *  Simulates a trackdisk by using a large file to hold the 'blocks'.
 *  Mod - CAS - 7/93: look for ENV:FMSUnit<n> containing path to file,
 *        then fall back to old FMS:Unit<n> as filename if ENV not found
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

extern struct Library *SysBase;
extern struct DosLibrary *DosBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>


/*#define DEBUG*/

#define CMD_OPENUNIT	(0x7FF0 & ~TDF_EXTCOM)
#define CMD_CLOSEUNIT	(0x7FF1 & ~TDF_EXTCOM)
#define CMD_KILLPROC	(0x7FF2 & ~TDF_EXTCOM)

typedef struct Library	LIB;
typedef struct Device	DEV;
typedef struct Process	PROC;
typedef struct MsgPort	PORT;
typedef struct Message	MSG;
typedef struct List	LIST;
typedef struct Node	NODE;
typedef long (*func_ptr)();

typedef struct {
    struct  Unit    U;
    UWORD   OpenCnt;
    long    Fh; 	/*  file handle     */
    long    Size;	/*  current size    */
    char    NotSyn;
    char    Reserved;
} NDUnit;

typedef struct {
    LIB     Lib;
    NDUnit  Unit[32];
} NDev;

typedef struct {
    struct  Message io_Message;
    struct  Device  *io_Device;     /* device node pointer  */
    struct  Unit    *io_Unit;	    /* unit (driver private)*/
    UWORD   io_Command; 	    /* device command */
    UBYTE   io_Flags;
    BYTE    io_Error;		    /* error or warning num */
    ULONG   io_Actual;		    /* actual number of bytes transferred */
    ULONG   io_Length;		    /* requested number bytes transferred*/
    APTR    io_Data;		    /* points to data area */
    ULONG   io_Offset;		    /* offset for block structured devices */

    long    iotd_Count; 	    /*	(extension)     */
    long    iotd_SecLabel;	    /*	(extension)     */
} IOB;


void SynchroMsg(UWORD cmd, struct Unit *unit);
void ExtendSize(NDUnit *unit, long offset);
void GetUnitName(int unitnum, char *buf);

extern char DeviceName[];
extern char IdString[];
extern void DUMmySeg();

typedef NDev	*NDevP;
typedef IOB	*IOBP;

struct Library *SysBase	= NULL;
struct DosLibrary *DOSBase	= NULL;

NDev *DevBase	= NULL;
APTR DevSegment = NULL;
PORT *FProc	= NULL;
PORT FPort;

extern func_ptr DevVectors[];

__saveds NDevP	__asm
Init(register __a0 APTR seg)
{
    NDev *db;

    SysBase = (struct Library *)(*(ULONG *)4);
    DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0);

    DevBase = db = (NDev *)MakeLibrary((long **)DevVectors,NULL,NULL,sizeof(NDev),NULL);
    db->Lib.lib_Node.ln_Type = NT_DEVICE;
    db->Lib.lib_Node.ln_Name = DeviceName;
    db->Lib.lib_Flags = LIBF_CHANGED|LIBF_SUMUSED;
    db->Lib.lib_Version = 1;
    db->Lib.lib_IdString= (APTR)IdString;

    DevSegment = seg;
    AddDevice((DEV *)db);
    return(db);
}

__saveds NDevP __asm
DevOpen(register __d0 long unitnum,
	register __a1 IOBP iob,
	register __d1 long flags
       )
{
    NDev *nd = DevBase;
    NDUnit *unit = &nd->Unit[unitnum];

    if (++nd->Lib.lib_OpenCnt == 1) {
	FProc = CreateProc("FMS-Dummy", 0, (long)DUMmySeg >> 2, 4096);
	FPort.mp_SigBit = SIGBREAKB_CTRL_D;	/*  port init */
	FPort.mp_SigTask= FProc->mp_SigTask;
	FPort.mp_Flags = PA_SIGNAL;
	NewList(&FPort.mp_MsgList);
    }

    if (++unit->OpenCnt == 1)
	SynchroMsg(CMD_OPENUNIT, unit);

    nd->Lib.lib_Flags &= ~LIBF_DELEXP;
    iob->io_Unit = (struct Unit *)unit;
    iob->io_Error = 0;
    return(nd);
}


__saveds APTR
DevExpunge(void)
{
    NDev *nd = DevBase;

    if (DevSegment == NULL)
	Alert(24);
    if (nd->Lib.lib_OpenCnt) {
	nd->Lib.lib_Flags |= LIBF_DELEXP;
	return(NULL);
    }
    Remove((NODE *)nd);
    FreeMem((char *)nd - nd->Lib.lib_NegSize, nd->Lib.lib_NegSize + nd->Lib.lib_PosSize);
    DevSegment = NULL;
    return(DevSegment);
}

__saveds APTR __asm
DevClose(register __a1 IOBP iob)
{
    NDev *nd = DevBase;

    {
	NDUnit *unit = (NDUnit *)iob->io_Unit;
	if (unit->OpenCnt && --unit->OpenCnt == 0)
	    SynchroMsg(CMD_CLOSEUNIT, unit);
    }

    if (nd->Lib.lib_OpenCnt && --nd->Lib.lib_OpenCnt)
	return(NULL);
    if (FProc) {
	SynchroMsg(CMD_KILLPROC, NULL);
	FProc = NULL;
    }
    if (nd->Lib.lib_Flags & LIBF_DELEXP)
	return(DevExpunge());
    /*
     *	close down resources
     */
    return(NULL);
}

__saveds void __asm
DevBeginIO(register __a1 IOBP iob)
{
    /*NDev *nd = DevBase;*/

    iob->io_Error = 0;
    iob->io_Actual = 0;

    switch(iob->io_Command & ~TDF_EXTCOM) {
    case CMD_INVALID:
	iob->io_Error = IOERR_NOCMD;
	break;
    case CMD_RESET:
	break;
    case CMD_READ:
	PutMsg(&FPort, &iob->io_Message);
	iob->io_Flags &= ~IOF_QUICK;	/*  not quick */
	iob = NULL;
	break;
    case CMD_WRITE:
	PutMsg(&FPort, &iob->io_Message);
	iob->io_Flags &= ~IOF_QUICK;	/*  not quick */
	iob = NULL;
	break;
    case CMD_UPDATE:
	break;
    case CMD_CLEAR:
	break;
    case CMD_STOP:
	break;
    case CMD_START:
	break;
    case CMD_FLUSH:
	break;
    case TD_MOTOR:	    /*	motor,	no action   */
    case TD_SEEK:	    /*	seek,	no action   */
	break;
    case TD_FORMAT:	    /*	format		    */
	PutMsg(&FPort, &iob->io_Message);
	iob->io_Flags &= ~IOF_QUICK;	/*  not quick */
	iob = NULL;
	break;
    case TD_REMOVE:	    /*	not supported	    */
	iob->io_Error = IOERR_NOCMD;
	break;
    case TD_CHANGENUM:	    /*	change count never changes  */
	iob->io_Actual = 1;
	break;
    case TD_CHANGESTATE:    /*	0=disk in drive     */
	iob->io_Actual = 0;
	break;
    case TD_PROTSTATUS:     /*	io_Actual -> 0 (rw) */
	iob->io_Actual = 0;
	break;
    case TD_RAWREAD:	    /*	not supported	    */
    case TD_RAWWRITE:
	iob->io_Error = IOERR_NOCMD;
	break;
    case TD_GETDRIVETYPE:   /*	drive type?	    */
	iob->io_Actual = 0;
	break;
    case TD_GETNUMTRACKS:
	iob->io_Actual = 0; /*	# of tracks?	    */
	break;
    case TD_ADDCHANGEINT:   /*	action never taken  */
    case TD_REMCHANGEINT:
	break;
    default:
	iob->io_Error = IOERR_NOCMD;
	break;
    }
    if (iob) {
	if ((iob->io_Flags & IOF_QUICK) == 0)
	    ReplyMsg((MSG *)iob);
    }
}

__saveds void __asm
DevAbortIO(register __a1 IOBP iob)
{
    /*NDev *nd = DevBase;*/
}

func_ptr DevVectors[] = {
    (func_ptr)DevOpen,
    (func_ptr)DevClose,
    (func_ptr)DevExpunge,
    NULL,
    (func_ptr)DevBeginIO,
    (func_ptr)DevAbortIO,
    (func_ptr)-1
};

/*
 *  Server communications
 */

void
SynchroMsg(UWORD cmd, struct Unit *unit)
{
    IOB Iob;

    Iob.io_Message.mn_ReplyPort = CreatePort(NULL, 0);
    Iob.io_Command = cmd;
    Iob.io_Unit = unit;

    PutMsg(&FPort, &Iob.io_Message);
    WaitPort(Iob.io_Message.mn_ReplyPort);
    DeletePort(Iob.io_Message.mn_ReplyPort);
}

/*
 *	SERVER SIDE (IS A PROCESS)
 *
 *	File name is:
 */

__saveds void
CoProc(void)
{
    IOBP iob;
    NDUnit *unit;
    char buf[128];
    char notdone = 1;
#ifdef DEBUG
    long fh = Open("con:0/0/320/100/Debug", 1006);
#endif

    Wait(SIGBREAKF_CTRL_D);     /*  wait for port init  */

    while (notdone) {
	WaitPort(&FPort);
	while (iob = (IOBP)GetMsg(&FPort)) {
	    unit = (NDUnit *)iob->io_Unit;

#ifdef DEBUG
	    sprintf(buf, "Cmd %08lx/%04x @ %08lx Buf %08lx %04x\n",
		unit, iob->io_Command, iob->io_Offset, iob->io_Data, iob->io_Length
	    );
	    Write(fh, buf, strlen(buf));
#endif
	    switch(iob->io_Command & ~TDF_EXTCOM) {
	    case CMD_OPENUNIT:
		GetUnitName(unit - &DevBase->Unit[0], buf);
		unit->Fh = Open(buf, 1005);
		if (unit->Fh == NULL) {
		    unit->Fh = Open(buf, 1006);
		    unit->NotSyn = 1;
		}
		if (unit->Fh) {
		    Seek(unit->Fh, 0L, 1);
		    unit->Size = Seek(unit->Fh, 0L, -1);
		}
		break;
	    case CMD_CLOSEUNIT:
		if (unit->Fh) {
		    Close(unit->Fh);
		    unit->Fh = NULL;
		}
		break;
	    case CMD_KILLPROC:
		notdone = 0;
		break;
	    case CMD_READ:
		if (unit->Fh == NULL)
		    break;
		if (iob->io_Offset + iob->io_Length > unit->Size)
		    ExtendSize(unit, iob->io_Offset + iob->io_Length);
		Seek(unit->Fh, iob->io_Offset, -1);
		iob->io_Actual = Read(unit->Fh, (char *)iob->io_Data, iob->io_Length);
		break;
	    case CMD_WRITE:
		/*
		 *  This causes file to be closed/reopened after
		 *  formatting.
		 */
		if (unit->NotSyn && unit->Fh) {
		    Close(unit->Fh);
		    GetUnitName(unit - &DevBase->Unit[0], buf);
		    unit->Fh = Open(buf, 1005);
		    unit->NotSyn = 0;
		}
		/* fall through */
	    case TD_FORMAT:
		if (unit->Fh == NULL)
		    break;

		if (iob->io_Offset > unit->Size)
		    ExtendSize(unit, iob->io_Offset);
		Seek(unit->Fh, iob->io_Offset, -1);
		iob->io_Actual = Write(unit->Fh, (char *)iob->io_Data, iob->io_Length);
		break;
	    }

	    if (notdone == 0)       /*  forbid before falling through */
		Forbid();           /*  and esp before replying       */
	    ReplyMsg(&iob->io_Message);
	}
    }
    /* fall through to exit */
}

void
GetUnitName(int unitnum, char *buf)
{
BPTR envfile;
LONG envlen;
BOOL gotit = FALSE;

/* New - see if ENVIRONMENT variable FMSUnit<n> exists (ex. FMSUnit2)
 * containing full path to file.
 */
sprintf(buf,"ENV:FMSUnit%d",unitnum);
if(envfile = Open(buf,MODE_OLDFILE))
    {
    envlen = Read(envfile,buf,128-2);
    if(envlen >= 0)
	{
	gotit = TRUE;
	buf[envlen] = '\0';
    	if(buf[envlen-1]=='\n') buf[envlen-1] = '\n';
	}
    Close(envfile);
    }
/* Fallback to old FMS:Unit<n> name */
if(!gotit)
    {
    sprintf(buf, "FMS:Unit%d", unitnum);
    }
}

void
ExtendSize(NDUnit *unit, long offset)
{
    long pos;
    char *buf = AllocMem(512, MEMF_CLEAR|MEMF_PUBLIC);

    if (unit->NotSyn == 0)
	unit->NotSyn = 1;
    Seek(unit->Fh, 0L, 1);
    pos = Seek(unit->Fh, 0L, 0);
    while (pos < offset) {
	if (Write(unit->Fh, buf, 512) != 512)
	    break;
	pos += 512;
    }
    FreeMem(buf, 512);
}

