/*
 * Hard Disk Utility for the Commodore-Amiga Hard Disk Controller
 * 
 * Uses Commodore's SASI-like command block structure.
 * March 13, 1986 (lce)
 *
 */

#include "exec/types.h"
#include "exec/memory.h"
#include "exec/exec.h"
#include "exec/io.h"
#include "devices/trackdisk.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "prep.h"
#include "prep_rev.h"

#define OUTOFMEMORY	52
#define SetRawMode	994L

extern long HdOpen();
extern void HdClose();
extern char *gets();
extern char yesreply();

typedef struct startup {	/* startup information		*/
    long st_unit;
    long st_devName;
    long st_disk_info;
} STARTUP;

typedef struct DevInfo {	/* Device information for AmigaDos */
    long d_Next;	/* Next node on list			*/
    long d_directory;	/* 1 means is direcory, 0 means device	*/
    long d_task;	/* Task pointer				*/
    long d_lock;
    long d_handler;
    long d_stacksize;
    long d_priority;
    long d_strtup;
    long d_seglist;
    long d_global;
    long d_name;
} DEVINFO;

struct Unit *o_unit;
struct Device *o_device;
struct FileHandle *OurHandle;
struct Message *OurMessage;	/* Pointer to our message structure	*/
struct DosPacket *OurPacket;	/* Pointer to DOS packet		*/
struct MsgPort	*PtMsgPort;	/* Pointer to Console device message port */
struct Process *MyProc;		/* Pointer to my process		*/
struct Process	*CONTask;	/* Pointer to console device process	*/
LONG IntuitionBase;		/* Pointer to Intuition library	*/
struct DosLibrary *DosPtr;	/* Pointer to DosLibrary structure	*/
FMT_DATA *format_data;		/* data passed to driver when format. trk 0*/
struct Remember *RememberKey;	/* Remember what memory we've allocated	*/
struct RootNode *RootPtr;	/* Pointer to RootNode structure	*/
struct DosInfo *DosInfoPtr;	/* Pointer to DosInfo structure		*/
struct IOStdReq hdrequest;
struct MsgPort HdMsgPort = {0};
char *p_dev_name;		/* Pointer to the device name	*/
DEVINFO *p_DevInfo;		/* Pointer to Device Info	*/
STARTUP *p_startup;		/* Pointer to startup info	*/
DEVINFO *DevInfoPtr;		/* Pointer to DevInfo structure */
LONG HdError = !NULL;
char success = FALSE;		/* Set TRUE when PREP succeeds	*/
char *devName;			/* Pointer to string containing device name */
int typeno = 0;			/* drive type selected from table */
LONG unit;			/* which physical drive ? */
short Enable_Abort;		/* Disable ^C			*/
#ifndef NOTEST
struct SCSICMD {
	unsigned char *buffer;
	unsigned long data_length;
	unsigned char *command;
	unsigned short cmd_length;
	unsigned char direction;
	unsigned char scsi_status;
	unsigned char ctlr_status;
} scsicmd;

unsigned char scsi_buffer[512];
unsigned char scsi_command[6];

#endif	

main(argc, argv)
int argc;			/* Number of passed arguments	*/
char *argv[];			/* Pointers to arguments	*/
{
    char BufDevName[80]; 	/* Buffer for dev name if input from prompt */
    char *t_ptr;		/* Temp pointer to test "gets" status */
    int i;			/* Temp index/counter */

    printf("\n PREP Version 33.%ld\n\n", REVISION);

    Enable_Abort = 0;		/* Disable ^C */
    if ((IntuitionBase = (long)OpenLibrary("intuition.library",0L)) < 1L)
    {
	printf("\nUnable to open Intuition Library!\n");
	cleanup();
    }
    RememberKey = NULL;		/* Show no memory allocated yet		*/
		/* Allocate memory for one cylinder		*/
    format_data = (FMT_DATA *)AllocRemember(&RememberKey,(512L*17L*16L),
	(long)(MEMF_PUBLIC | MEMF_CLEAR));
    OurMessage = (struct Message *)AllocRemember(&RememberKey,
	(long)sizeof(*OurMessage), (long)(MEMF_PUBLIC | MEMF_CLEAR));
    OurPacket = (struct DosPacket *)AllocRemember(&RememberKey,
	(long)sizeof(*OurPacket), (long)(MEMF_PUBLIC | MEMF_CLEAR));
    if ((format_data == NULL) || (OurPacket == NULL) || (OurMessage == NULL))
    {
	printf("\nInsufficient RAM\n");
	cleanup();
    }
    if (argc == 2)		/* Single parameter supplied */
	devName = argv[1];	/* Assume is device name	*/
    if ((argc > 2) ||		/* Illegal call		*/
	(*devName == '?'))	/* 	or "?"		*/
	printf("Usage: PREP <device>\n");
    else
    {
	if (argc <= 1)	/* If no parameters, ask for device */
	{
	    devName = BufDevName;
	    printf("Enter device name: ");
	    t_ptr = (char *)gets(devName);
	    if (t_ptr == NULL)*devName = 0; /* If get failed, indicate it */
	}
	if (*devName != 0)
	{
	    for (i=0; devName[i] != 0; i++) /* Make sure str all upper case */
		if (isalpha(devName[i]))devName[i] = toupper(devName[i]);
	    if (devName[i-1] == ':')devName[--i] = 0;
		prep(devName, &success, i);	/* Prep hard disk */
	}
    }
    if (!success) printf("PREP failed\n");
    else printf(
	"PREP complete. Reboot the system before accessing this drive.\n");
    cleanup();		/* Cleanly leave this AMIGA environment */
}


/*************************************
 *    cleanup: close up open things  *
 *************************************/

cleanup()
{
    if (HdError == NULL) HdClose();
    if (RememberKey != NULL) FreeRemember(&RememberKey,(long)TRUE);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (DosPtr) CloseLibrary(DosPtr);
    exit(0);
}


/*
 * Routine to check for any hard disks present, and tell AmigaDos about them
 * if found.
 */

prep(devName, success, d_len)
char*devName;		/* Pointer to device name	*/
char *success;		/* PASS/FAIL flag		*/
int d_len;		/* Length of devName string	*/
{
    char *d_ptr;	/* Pointer to device name	*/
    char match;		/* Flag for character compare	*/
    int i;		/* Index/counter		*/
    long cyl_size;	/* Number of bytes per cylinder	*/

    /* Locate driver name for this device	*/

    /* Find RootNode, start of all Dos structures */
    d_len = strlen(devName);	/* Determine length of name */
    DosPtr = (struct DosLibrary *)OpenLibrary("dos.library",0L);
    RootPtr = (struct RootNode *)DosPtr->dl_Root;
    /* Now find DosInfo from root node BPTR */
    DosInfoPtr = (struct DosInfo *)BADDR(RootPtr->rn_Info);
    /* Now find start of DevInfo list from BPTR */
    DevInfoPtr = (DEVINFO *)BADDR(DosInfoPtr->di_DevInfo);

    /* Find entry in this list that matches the passed device name */
    match = FALSE;
    do {
	if ((long)DevInfoPtr != 0L)	/* If not end of list */
	{
	    if (!DevInfoPtr->d_directory)
	    { /* If is a device, not dir then get pointer to dev */
		d_ptr = (char *)BADDR(DevInfoPtr->d_name);
		d_ptr++;	/* Skip length byte */
		match = TRUE;
		for (i = 0; i < d_len; i++)
		    match = (*d_ptr++ == devName[i]) && match;
	    }
	    if(!match)DevInfoPtr=(DEVINFO *)BADDR(DevInfoPtr->d_Next);
	}
    } while (((long)DevInfoPtr != 0L) && !match);

    if (!match)
	printf("Device %s: unknown\n",devName);
    else
    {
	d_ptr =		/* Point to STARTUP structure */
	    (char *)BADDR(DevInfoPtr->d_strtup);
	unit =		/* Get unit number */
	    ((STARTUP *)d_ptr)->st_unit;
	d_ptr =		/* Point to device driver name */
	    (char *)(BADDR(((STARTUP *)d_ptr)->st_devName)+1);
	if ((HdError = HdOpen(d_ptr, unit)) != NULL)	/* open the device */
	{
	    printf("Disk open failed\n");
	    cleanup();
	}
	else
	{
#ifdef NOTEST
	    select_d_type(format_data);
	    printf("Proceeding further will destroy any information on \
the\n entire physical device, and the system will have to be\n rebooted.  ");
	    if (yesreply("Do you wish to proceed?", "N"))
#endif
	    {
#ifdef NOTEST
		cyl_size = format_data->bootinfo.b_disk_info.ds_heads *
		    format_data->bootinfo.b_disk_info.ds_sectors * 512L;
		*success = TRUE;
		for (i = 0; (i < 2) && *success; i++)
		{
		    SetIO(&hdrequest, (UWORD)TD_FORMAT, format_data,
			cyl_size, cyl_size * i, o_unit, o_device);
		    SendIO(&hdrequest);		/* Startup IO */	
		    WaitPort(&HdMsgPort);
		    while (GetMsg(&HdMsgPort) != NULL);
		    *success = *success && (hdrequest.io_Error == 0);
		}
#else
	*success = TRUE;
	scsicmd.buffer = scsi_buffer;
	scsicmd.data_length = 36;
	scsicmd.command = scsi_command;
	scsicmd.cmd_length = 6;
	scsicmd.direction = 0;	/* IN */
	scsi_command[0] = 0x12;	/* Inquiry command */
	scsi_command[1] = 0;	/* LUN 0	*/
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = scsicmd.data_length; /* Allocation length */
	scsi_command[5] = 0;

		    SetIO(&hdrequest,(UWORD)0x12, &scsicmd, 0L,0L,
			o_unit, o_device);
		    SendIO(&hdrequest);		/* Startup IO */	
		    WaitPort(&HdMsgPort);
		    while (GetMsg(&HdMsgPort) != NULL);
		    *success = *success && (hdrequest.io_Error == 0);
	for (i=0;i<10;i++)
	    printf(" %lx",scsi_buffer[i]);
	printf("\n");
	printf("%s\n",&scsi_buffer[8]);
	printf("scsi_status = %lx, ctlr_status = %lx, io_Error = %lx\n",
		scsicmd.scsi_status,scsicmd.ctlr_status,hdrequest.io_Error);
#endif
	    }
	}
    }
}


long HdOpen(d_ptr, unit)
char *d_ptr;	/* Pointer to device driver name*/
long unit;	/* Unit number of desired drive	*/
{
    int error;

    /* Open the disk device */
    if ((error = OpenDevice(d_ptr, unit, &hdrequest, 0L)) != 0)
    {
	printf("OpenDevice Error: %ld.\n", error);
	return(error);
    }

    o_unit = hdrequest.io_Unit;
    o_device = hdrequest.io_Device;

    /* Set up the message port in the IO request */
    HdMsgPort.mp_Node.ln_Type = NT_MSGPORT;
    HdMsgPort.mp_Node.ln_Name = "PREP";
    HdMsgPort.mp_Flags = 0;
    HdMsgPort.mp_SigBit = AllocSignal(-1L);
    HdMsgPort.mp_SigTask = (struct Task *)FindTask((char *)NULL);
    AddPort(&HdMsgPort);
    hdrequest.io_Message.mn_ReplyPort = &HdMsgPort;
    return(0);
}


void HdClose()		/* Quit using hddisk device */
{
    while (GetMsg(&HdMsgPort) != NULL);
    CloseDevice(&hdrequest);
    while (GetMsg(&HdMsgPort) != NULL);
    FreeSignal(HdMsgPort.mp_SigBit);
    RemPort(&HdMsgPort);
}

