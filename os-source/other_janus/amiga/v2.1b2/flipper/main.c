#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

/* #include <local/sccs.h> */

#define MAIN 1

#include "share.h"
#include "string.h"

/* static char SccsId[] = SCCS_STRING; */
static char port_name[] = SHARENAME;

struct MsgPort *FlipperPort;
struct Message ToFlipper;

/*
   Quit is supposed to make sure all resources opened get closed.
*/

void Quit(error)
int error;
{
    int retvalue = 10;

    switch(error) {
        case NO_ERROR:
            retvalue = 0;
            break;
        case PARAMETER_ERROR:
            Debug0("Bad or unknown parameter specification\n");
            retvalue = 5;
            break;
        default:
            Debug1("Unkown Error %d\n", error);
            retvalue = 20;
    }
    DebugEnd();
    Exit(retvalue);
}

void Usage(string)
char *string;
{
    static char hints[] = "Flipper Amiga|PC|Auto|Quit \
[df0:|df1: [A:|B: window=xxx,yyy]]\n";
    Write(Output(), hints, strlen(hints));
    Write(Output(), string, strlen(string));
}

int CMDFromPort()
{
    int newmode;
    struct Message *flippermessage;
    extern LocalMode(int);

    Debug0("CMDFromPort\n");
    flippermessage = GetMsg(FlipperPort);
    if(flippermessage == NULL) {
        Debug0("CMDFromPort: Null message received???\n");
        return 0;
    }
    newmode = flippermessage->mn_Length;
    ReplyMsg(flippermessage);
    Debug1("Got newmode: %lx\n", newmode);
    if(newmode & MODE_QUIT) 
        going = 0;
    return newmode;
}

AmigaToPC()
{
    inhibit(amiga_name, 1);
/*    OffDisk(); */
    DiskToPC();
    while(DiskInserted())
        Wait(disk_mask);
    inhibit(amiga_name, 0);
}

AmigaStaticToPC()
{
    inhibit(amiga_name, 1);
/*     OffDisk(); */
    DiskStaticPC();
    while(DiskInserted())
        Wait(disk_mask);
    inhibit(amiga_name, 0);
}

PCToAmiga()
{
    DiskToAmiga();
    inhibit(amiga_name, 1);
    inhibit(amiga_name, 0);
}

void FlipperLoop()
{
    Debug0("Enter Flipper Auto Mode\n");

    if(!OpenDiskStuff()) {
        Debug0("Can't open the disk!\n");
        return;
    }
    Debug0("Open Disk OK\n");
    if(window_mode) {
        if(!OpenDisplay()) {
            Debug0("Can't open the display!\n");
            CloseDiskStuff();
            return;
        }
    }
    FlipperPort = CreatePort(SHARENAME, 0);
    port_mask = (1 << FlipperPort->mp_SigBit);
    going = 1;
    owner = SELECT_AMIGA;
    HandleDisk();
    while(going) {
        int gotsignal;
        int newmode;

        wait_mask = (user_mask | port_mask | disk_mask);
        if( currentmode & MODE_AUTO) {
            int inhibited;
/*            if(DiskInserted()) { */
                inhibit(amiga_name, 1);
                inhibited = 1;
                DiskToAmiga();
                HandleDisk();
/*
            }
            else {
                inhibited = 0;
                currentdisk = DISK_EMPTY;
            }
*/
            if(currentdisk == DISK_PC) {
                DiskToPC();
                while(DiskInserted())
                    Wait(disk_mask);
                owner = SELECT_PC;
            }
            if(currentdisk == DISK_AMIGA) {
                owner = SELECT_AMIGA;
            }
            if(currentdisk == DISK_UNKNOWN) {
                if(owner == SELECT_PC) {
                    DiskToPC();
                    while(DiskInserted())
                        Wait(disk_mask);
                }
#ifdef AMIGA_NOTIFY
                else
                    PCToAmiga();
#endif
            }
            if(currentdisk == DISK_EMPTY) {
                Debug0("Disk is empty\n");
                if(currentselect == SELECT_PC) {
                    Debug0("Disk to PC\n");
                    DiskToPC();
                    owner = SELECT_PC;
                }
            }
            if(inhibited)
                inhibit(amiga_name, 0);
        }
        else if(currentmode & MODE_MANUAL) {
            if(currentselect == SELECT_PC) {
                AmigaStaticToPC();
                owner = SELECT_PC;
            }
            if(currentselect == SELECT_AMIGA) {
#ifdef AMIGA_NOTIFY
                PCToAmiga();
#else
                DiskToAmiga();
#endif
                owner = SELECT_AMIGA;
            }
        }
        else {
            Debug0("Neither AUTO nor MANUAL????? Error in FlipperLoop\n");
            Debug1("Currentmode: %lx\n", currentmode);
        }
        if(window_mode)
            UpdateDisplay(currentmode, owner);
        gotsignal = Wait(wait_mask);
        Debug1("Got Signal %lx\n", gotsignal);
        newmode = 0;
        if( gotsignal & user_mask) {
            Debug0("User Signal!\n");
            newmode = HandleUser();
        }
        if( gotsignal & port_mask) {
            Debug0("Port Signal!\n");
            newmode = CMDFromPort();
        }
        if(newmode) {
            currentmode = (newmode & (MODE_AUTO | MODE_MANUAL));
            if(currentmode & MODE_MANUAL) {
                int newselect;
                if ((newselect = (newmode & (SELECT_AMIGA | SELECT_PC))) != 0)
                    currentselect = newselect;
            }
        }
    }
#ifdef AMIGA_NOTIFY
    PCToAmiga();
#else
    DiskToAmiga();
#endif
    DeletePort(FlipperPort);
    if(window_mode)
        CloseDisplay();
    CloseDiskStuff();
}

/*
   NewMode is mostly used to manipulate an already running flipper from
   the CLI. It sends a message to its messageport which induces the
   background process to change the hardware settings and display
   accordingly. If the named port cannot be found, only simple
   back and forth switches can be made; if the flipper process is found,
   it can be told to go into automatic mode too.
*/

void NewMode(newmode)
int newmode;
{
    struct MsgPort *hisport, *myport;

    Debug1("Trying to set new mode %lx\n", newmode);
    if((hisport = FindPort(SHARENAME)) != NULL) {
        Debug0("Flipper is already running\n");
        myport = CreatePort(0,0);
        if(myport == NULL) {
            Debug0("Can't create reply port.. exiting\n");
            return;
        }
        ToFlipper.mn_Length = newmode;
        ToFlipper.mn_Node.ln_Type = NT_MESSAGE;
        ToFlipper.mn_ReplyPort = myport;
        PutMsg(hisport, &ToFlipper);
        WaitPort(myport);
        DeletePort(myport);
        Debug0("Received reply to newmode message\n");
        return;
    }
    else {
        /*
           No port found, we don't create one either. We just write the
           register to make the drive accessible to whichever machine
           is supposed to get it. In the case of PC this means that
           diskchange info won't get through to the Amiga anymore.
         */
        Debug0("No Flipper running.. check for manual\n");
        if(newmode & MODE_AUTO) {
            /* Auto only makes sense to an already running flipper. */
            Usage("Flipper is not running, can't set Automatic mode\n");
            Quit(PARAMETER_ERROR);
        }
        if(newmode & SELECT_PC) {
            DiskStaticPC();
        }
        if(newmode & SELECT_AMIGA) {
            DiskToAmiga();
        }
        if(newmode & MODE_QUIT) {
            /* Quit only makes sense to an already running flipper. */
            Usage("Flipper is not running, not possible to quit it\n");
            Quit(PARAMETER_ERROR);
        }
    }
}

void main(argc, argv)
int argc;
char **argv;
{
    extern int window_position(char *);

    DebugInit();
/*
   Check for hardware first, not much sense in going through the whole
   business to find out they don't have a board!
 */
    if(!FindJanus()) {
        Quit(JANUS_ERROR);
    }
    switch(argc) {
        case 0:
            /* Called from Workbench, look at tooltypes. */
#ifdef WB_SUPPORT
            HandleWB((struct WBStartup *) argv);
#else
            Debug0("Tooltypes not supported....\n");
            Quit(PARAMETER_ERROR);
#endif
            if(currentmode & MODE_AUTO) {
                if(FindPort(SHARENAME)) {
                    NewMode(MODE_AUTO);
                }
                else {
                    FlipperLoop();
                }
            }
            else {
                NewMode(currentmode| currentselect);
            }

            break;
        case 5:
        /* 
            Command line has all possible arguments: E.g.
            flipper auto dfx: A: window=xxx,yyy 
            We are concerned with window and PC drive spec. here.
         */
        {
            int len;
            len = strlen(window_spec);
            Debug2("Length(%): %d\n", window_spec, len);
        }
        if(strnicmp(window_spec , argv[4], strlen(window_spec)) == 0) {
            if(!window_position(argv[4] + strlen(window_spec))) {
                Debug1("Bad window specification %s\n", argv[4]);
                Quit(PARAMETER_ERROR);
            }
            if( stricmp(argv[3], pc_a ) == 0) {
                Debug0("PC drive is A:\n");
                pc_number = PC_A;
                pc_name = pc_a;
            }
            else if( stricmp(argv[3], pc_b ) == 0) {
                Debug0("PC drive is B:\n");
                pc_number = PC_B;
                pc_name = pc_b;
            }
            else {
                Debug1("Bad drive %s\n", argv[5]);
                Quit(PARAMETER_ERROR);
            }
            window_mode = 1;
        }
        else {
            Debug1("Illegal window specification: %s\n", argv[4]);
            Quit(PARAMETER_ERROR);
        }
        case 3:
            if(stricmp(argv[2], amiga_df0) == 0) {
                amiga_name = amiga_df0;
                amiga_number = AMIGA_DF0;
            }
            else if(stricmp(argv[2], amiga_df1) == 0) {
                amiga_name = amiga_df1;
                amiga_number = AMIGA_DF1;
            }
            else {
                Debug1("Unkown Amiga Drive %s\n", argv[2]);
                Quit(PARAMETER_ERROR);
            }
        case 2:
            if(stricmp(argv[1], "amiga") == 0) {
                NewMode(MODE_MANUAL | SELECT_AMIGA);
            }
            else if(stricmp(argv[1], "pc") == 0) {
                NewMode(MODE_MANUAL | SELECT_PC);
            }
            else if(stricmp(argv[1], "auto") == 0) {
                currentmode = MODE_AUTO;
                if(FindPort(SHARENAME)) {
                    /* Flipper is already running, send newmode message */
                    NewMode(MODE_AUTO);
                }
                else {
                    /* Try to start the flipper loop */
                    if(argc > 2) {
                        FlipperLoop();
                    }
                    else {
                        Debug0("Auto flipper requested, no flipper running!\n");
                        Usage("Flipper not running\n");
                        Quit(PARAMETER_ERROR);
                    }
                }
            }
            else if(stricmp(argv[1], "quit") == 0) {
                NewMode(MODE_QUIT);
            }
            else {
                Debug1("Bad command %s\n", argv[1]);
                Quit(PARAMETER_ERROR);
            }
            break;
        case 1:
        case 4:
        default:
            Usage("Argument count\n");
            Quit(PARAMETER_ERROR);
    }
    DebugEnd();
}
