#include <exec/types.h>
#include <exec/exec.h>
#include <devices/trackdisk.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/all.h>
#include <resources/disk.h>

/* #include <local/sccs.h> */

#include "share.h"

/* static char SccsId[] = SCCS_STRING; */
static char td_name[] = TD_NAME;

extern struct MsgPort	*CreatePort();
extern struct IOStdReq	*CreateStdIO();
extern void ChangeServer();

struct DiscResource *DiskBase = NULL;

struct MsgPort  *port = NULL;
struct MsgPort  *dru_port = NULL;
struct IOStdReq *request = NULL;
struct MsgPort  *readport = NULL;
struct IOStdReq *readrequest = NULL;

struct Interrupt *changeserver = NULL;

ULONG changesig = 0;

struct IntData {
    struct Task  *mytask;
    ULONG sigmask;
} changedata;

UBYTE *readbuffer = NULL;

int GetReader()
{
    int error;
    if ((readport = CreatePort(0,0)) == NULL) {
        Debug0("GetReader: CreatePort failed!\n");
        return 0;
    }

    readbuffer = AllocMem(BUFFERSIZE, MEMF_CHIP | MEMF_CLEAR);

    if ((readrequest = CreateStdIO(readport)) == NULL) {
        Debug0("GetReader: CreateStdIO failed!\n");
        FreeMem(readbuffer, BUFFERSIZE);
        DeletePort(readport);
        return 0;
    }

    if (error = OpenDevice(td_name, amiga_number, \
        (struct IORequest*) readrequest, 0)) {
        Debug1("GetReader: OpenDevice %ld failed!\n", amiga_number);
        DeleteStdIO(readrequest);
        FreeMem(readbuffer, BUFFERSIZE);
        DeletePort(readport);
        return 0;
    }
    Debug1("IDString: %s\n", (ULONG) readrequest->io_Device->dd_Library.lib_IdString);
    Debug1("Vers: %ld\n", (ULONG) readrequest->io_Device->dd_Library.lib_Version);
    Debug1("Rev : %ld\n", (ULONG) readrequest->io_Device->dd_Library.lib_Revision);
    return 1;
}

void DeleteReader(void)
{
    Debug0("Deleting Reader structs...\n");
    CloseDevice(readrequest);
    DeleteStdIO(readrequest);
    FreeMem(readbuffer, BUFFERSIZE);
    DeletePort(readport);
}

int OpenDiskStuff(void)
{
    int error;

    if(GetReader() == 0) {
        Debug0("OpenDisk: GetReader failed!\n");
        return 0;
    }

    if(( changeserver = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|
        MEMF_CLEAR)) == NULL) {
        Debug0("OpenDisk: AllocMem for server failed!\n");
        DeleteReader();
        return 0;
    }

    changedata.mytask = FindTask(0);
    changesig = AllocSignal(-1);
    if(changesig < 0) {
        Debug0("OpenDisk: AllocSignal for server failed!\n");
        FreeMem(changeserver, sizeof(struct Interrupt));
        DeleteReader();
        return 0;
    }

    if ((dru_port = CreatePort(0,0)) == NULL) {
        Debug0("OpenDisk: CreatePort failed!\n");
        FreeSignal(changesig);
        FreeMem(changeserver, sizeof(struct Interrupt));
        DeleteReader();
        return 0;
    }

	if (!(DiskBase = (struct DiscResource *) OpenResource(DISKNAME))) {
	        Debug0("OpenDisk: OpenResource(disk.resource) failed!\n");
		DeletePort(dru_port);
	        FreeSignal(changesig);
	        FreeMem(changeserver, sizeof(struct Interrupt));
	        DeleteReader();
	        return 0;
	}

    if ((port = CreatePort(0,0)) == NULL) {
        Debug0("OpenDisk: CreatePort failed!\n");
	DeletePort(dru_port);
        FreeSignal(changesig);
        FreeMem(changeserver, sizeof(struct Interrupt));
        DeleteReader();
        return 0;
    }

    if ((request = CreateStdIO(port)) == NULL) {
        Debug0("OpenDisk: CreateStdIO failed!\n");
        DeletePort(port);
	DeletePort(dru_port);
        FreeSignal(changesig);
        FreeMem(changeserver, sizeof(struct Interrupt));
        DeleteReader();
        return 0;
    }

    if (error = OpenDevice(td_name, amiga_number, \
        (struct IORequest *) request, 0)) {
        Debug0("OpenDisk: OpenDevice failed!\n");
        DeleteStdIO(request);
        DeletePort(port);
	DeletePort(dru_port);
        FreeSignal(changesig);
        FreeMem(changeserver, sizeof(struct Interrupt));
        DeleteReader();
        return 0;        
    }

    changedata.sigmask = (1 << changesig);
    changeserver->is_Node.ln_Pred = NULL;
    changeserver->is_Node.ln_Succ = NULL;
    changeserver->is_Node.ln_Type = NT_INTERRUPT;
    changeserver->is_Node.ln_Pri = 0;
    changeserver->is_Node.ln_Name = "DiskChangeDaemon";
    changeserver->is_Data = &changedata;
    changeserver->is_Code = ChangeServer;
  

    request->io_Length  = sizeof(struct Interrupt);
    request->io_Data    = (APTR) changeserver;
    request->io_Command = TD_ADDCHANGEINT;
    request->io_Offset  = 0;
    request->io_Flags   = 0;
    SendIO((struct IORequest *) request);
    disk_mask = (1 << changesig);
    return 1;
}

void CloseDiskStuff(void)
{
    Debug0("CloseDisk:\n");
    Debug0("Removing ADDCHANGEINT request....\n");
    Remove(request);

    FreeMem(changeserver, sizeof(struct Interrupt));
    Debug0("Freeing changesignal\n");
    FreeSignal(changesig);

	DeletePort(dru_port);

    CloseDevice(request);
    DeleteStdIO(request);
    DeletePort(port);

    DeleteReader();
}

int DiskInserted()
{
    int error;
    Debug0("DiskInserted?\n");
    readrequest->io_Length = 0;
    readrequest->io_Offset = 0;
    readrequest->io_Command = TD_CHANGESTATE;
    readrequest->io_Flags = IOF_QUICK;
    error = DoIO((struct IORequest *) readrequest);
    if(error) {
        return 0;
    }
    return (readrequest->io_Actual ? 0 : 1);
}

void OffDisk(void)
{
    int error;

    Debug0("OffDisk:\n");
    readrequest->io_Length = 0;
    readrequest->io_Command = TD_MOTOR;
    readrequest->io_Flags = IOF_QUICK;
    error = DoIO(readrequest);
}

void InvalidDisk(void)
{
    int error;

    Debug0("InvalidDisk:\n");
    readrequest->io_Length = 0;
    readrequest->io_Command = CMD_CLEAR;
    readrequest->io_Flags = IOF_QUICK;
    error = DoIO(readrequest);
}

/*
    This tries to read track zero on the disk. This costs us some time
    but is the only place even weird formats use for data. Additionally
    the disk is moved to the outermost track, so even if either machine
    gets confused as to the position of the head, no harm is done.
    Retrycount set to as low as possible (empirically determined) to
    save some time.
*/
int Read0()
{
    int error;
    int retryvalue;
    struct TDU_PublicUnit * myunit;

    Debug0("ReadSector 0:\n");
    myunit = (struct TDU_PublicUnit *) readrequest->io_Unit;
    retryvalue = myunit->tdu_RetryCnt;
    myunit->tdu_RetryCnt = 1;

    readrequest->io_Offset = 0;
    readrequest->io_Length = 512;
    readrequest->io_Data = readbuffer;
    readrequest->io_Command = CMD_READ;
    readrequest->io_Flags = IOF_QUICK;
    DoIO((struct IORequest *) readrequest);
    error = readrequest->io_Error;
    myunit->tdu_RetryCnt = retryvalue;
    Debug1("ReadSector 0 Error %ld\n",readrequest->io_Error);

    OffDisk();

    return error;
}

/*
   Guess whether a disk is likely to be  PC disk.
   Once RAWREAD works, i could examine the raw data a la DOS2DOS.
   This works reasonably well though.
*/
void HandleDisk(void)
{
    int code;
    if((code = Read0()) != 0) {
        switch(code) {
#if 0
            case 21:
                currentdisk = DISK_UNKNOWN;
                break;
            case 26: /* for kick 1.4.... 1.4 will use rawread though... */
            case 27:
                currentdisk = DISK_PC;
                break;
#endif
            case 29:
                currentdisk = DISK_EMPTY;
                break;
            default:
#if 0
                currentdisk = DISK_AMIGA;
#else
		currentdisk = DISK_PC;
#endif
                break;
        }
    }
    else {
        currentdisk = DISK_AMIGA;
    }
    switch(currentdisk) {
        case DISK_PC:
            Debug0("currentdisk is DISK_PC\n");
            break;
        case DISK_AMIGA:
            Debug0("currentdisk is DISK_AMIGA\n");
            break;
        case DISK_EMPTY:
            Debug0("currentdisk is DISK_EMPTY\n");
            break;
    }
}

