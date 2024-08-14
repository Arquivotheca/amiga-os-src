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

extern LONG HdOpen();
extern void HdClose();
extern char *gets();
extern char yesreply();

typedef struct startup {	/* startup information		*/
    LONG st_unit;
    LONG st_devName;
    LONG st_disk_info;
} STARTUP;

typedef struct DevInfo {	/* Device information for AmigaDos */
    LONG d_Next;	/* Next node on list			*/
    LONG d_directory;	/* 1 means is direcory, 0 means device	*/
    LONG d_task;	/* Task pointer				*/
    LONG d_lock;
    LONG d_handler;
    LONG d_stacksize;
    LONG d_priority;
    LONG d_strtup;
    LONG d_seglist;
    LONG d_global;
    LONG d_name;
} DEVINFO;

struct Unit *o_unit;
struct Device *o_device;
struct FileHandle *OurHandle;
struct Message *OurMessage;	/* Pointer to our message structure	*/
struct DosPacket *OurPacket;	/* Pointer to DOS packet		*/
struct MsgPort	*PtMsgPort;	/* Pointer to Console device message port */
struct Process *MyProcce;	/* Pointer to my process		*/
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
LONG unit;			/* which physical drive */
short Enable_Abort;		/* Disable ^C			*/

main(argc, argv)
int argc;			/* Number of passed arguments	*/
char *argv[];			/* Pointers to arguments	*/
{
    char BufDevName[80]; 	/* Buffer for dev name if input from prompt */
    char *t_ptr;		/* Temp pointer to test "gets" status */
    int i;			/* Temp index/counter */

    printf("\nPREP Version 33.%ld\n\n", REVISION);
    *devName=0;

    Enable_Abort = 0;		/* Disable ^C */
    if ((IntuitionBase = (LONG)OpenLibrary("intuition.library",0)) < 1) {
	printf("\nUnable to open Intuition Library!\n");
	cleanup(20);
    }
    RememberKey = NULL;		/* Show no memory allocated yet		*/
		/* Allocate memory for one cylinder		*/
    format_data = (FMT_DATA *)AllocRemember(&RememberKey,(512L*17L*16L),
	(LONG)(MEMF_PUBLIC | MEMF_CLEAR));
    OurMessage = (struct Message *)AllocRemember(&RememberKey,
	(LONG)sizeof(*OurMessage), (LONG)(MEMF_PUBLIC | MEMF_CLEAR));
    OurPacket = (struct DosPacket *)AllocRemember(&RememberKey,
	(LONG)sizeof(*OurPacket), (LONG)(MEMF_PUBLIC | MEMF_CLEAR));
    if ((format_data == NULL) || (OurPacket == NULL) || (OurMessage == NULL)) {
	printf("\nError: not enough memory\n");
	cleanup(20);
    }
    if (argc == 2)		/* Single parameter supplied */
	devName = argv[1];	/* Assume is device name	*/

    else printf("Usage: PREP <device>\n");

	if (*devName== '?') { /*If ? ask for device */
	    devName = BufDevName;
	    printf("DEVICE/A: ");
	    t_ptr = (char *)gets(devName);
	    if (t_ptr == NULL)*devName = 0; /* If get failed, indicate it */
	}

	if (*devName != 0) {
	    for (i=0; devName[i] != 0; i++) /* Make sure str all upper case */
		if (isalpha(devName[i]))devName[i] = toupper(devName[i]);
	    if (devName[i-1] == ':')devName[--i] = 0;
		prep(devName, &success, i);	/* Prep hard disk */
	}
    if (!success)  {
	printf("\nDevice not PREPed\n");
	cleanup(20);
    }
    else printf(
	"PREP complete. Reboot the system before using this drive.\n");
    cleanup(0);		/* Cleanly leave this AMIGA environment */
}


/*************************************
 *    cleanup: close up open things  *
 *************************************/

cleanup(error)
int error;
{
    if (HdError == NULL) HdClose();
    if (RememberKey != NULL) FreeRemember(&RememberKey,(LONG)TRUE);
    if (DosPtr) CloseLibrary(DosPtr);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    exit(error);
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
    LONG cyl_size;	/* Number of bytes per cylinder	*/

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
	if ((LONG)DevInfoPtr != 0) {	/* If not end of list */
	    if (!DevInfoPtr->d_directory) {
	        /* If is a device, not dir then get pointer to dev */
		d_ptr = (char *)BADDR(DevInfoPtr->d_name);
		d_ptr++;	/* Skip length byte */
		match = TRUE;
		for (i = 0; i < d_len; i++)
		    match = (*d_ptr++ == devName[i]) && match;
	    }
	    if(!match)DevInfoPtr=(DEVINFO *)BADDR(DevInfoPtr->d_Next);
	}
    } while (((LONG)DevInfoPtr != 0) && !match);

    if (!match) printf("Device %s: unknown\n",devName);

    else {
	d_ptr=(char *)BADDR(DevInfoPtr->d_strtup);/* Point to STARTUP structure */

	unit = ((STARTUP *)d_ptr)->st_unit; /* Get unit number */

	/* Point to device driver name */	    
	d_ptr=(char *)(BADDR(((STARTUP *)d_ptr)->st_devName)+1);

	/* open the device */	    
	if (HdError = HdOpen(d_ptr, unit)) cleanup(20);
	else {
	    select_d_type(format_data);
	    printf("\nContinuing will destroy any information on the\n\
entire physical device, and the system will have to be rebooted.\n");
	    if (yesreply("Do you wish to proceed ?", "N")) {
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
	    }
	}
    }
}


LONG HdOpen(d_ptr, unit)
char *d_ptr;	/* Pointer to device driver name*/
LONG unit;	/* Unit number of desired drive	*/
{
    int error= -1;

    /* Open the disk device */
    if(*d_ptr) error = OpenDevice(d_ptr, unit, &hdrequest, 0);
    if(error) {
	printf("Error %ld on OpenDevice of %s\n",error,d_ptr);
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

