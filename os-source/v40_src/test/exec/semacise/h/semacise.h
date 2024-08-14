/******** checkpoint.test/Semacise ********************************************

    NAME
        Semacise.h -- headers for Semacise.c

    VERSION
        1.00    08 Mar 1991 - Inception
        1.01    01 Apr 1991 - Added Loopback
        2.00    24 Jun 1992 - Procure()/Vacate() support (jjszucs)

    AUTHOR
        Fred Mitchell, Product Assurance, Commodore-Amiga, Inc.
        John J. Szucs, Systems Assurance, Commodore International Services Company

******************************************************************************/

#ifndef _SEMACISE_
#define _SEMACISE_

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/text.h>
#include <graphics/rastport.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

// defines and parameters and such

#define HELP_FILE   "Semacise.doc"
#define HELP_OUTPUT "CON:0/0/640/200/Semacise_Help/AUTO/CLOSE/WAIT"

#define SEMA_FONT   "topaz.font"
#define SEMA_YSIZE  8

#define MASTER_PORT         "SemaMasterPort"
#define CHILD_PORT          "SemaChildPortNumber_"  // Append # to this
#define LOWEST_TASK_PRIORITY    -7
#define NUMBER_OF_TASKS         7
#define NUMBER_OF_SEMAPHORES    26
#define NUMBER_OF_BIDS          10

#define START_LEFT  16
#define START_TOP   16

#define BUSY_LEFT   (START_LEFT + 248)
#define BUSY_TOP    (START_TOP + 0)

#define TASK_LEFT   (START_LEFT + 192)
#define TASK_TOP    (START_TOP + 40)

#define STRING_LEFT (START_LEFT + 0)
#define STRING_TOP  (START_TOP + 40)

#define STRING_WIDTH    (TASK_LEFT - 20)
#define STRING_HEIGHT   (BUTTON_HEIGHT)
#define MAX_CHARS       80

#define X_SPACE     56
#define Y_SPACE     32

#define BUTTON_WIDTH    49
#define BUTTON_HEIGHT   25

#define STATUS_BOX_LEFT (START_LEFT + 248)
#define STATUS_BOX_TOP  (START_TOP + 40)
#define STATUS_BOX_WIDTH    328
#define STATUS_BOX_HEIGHT   216

#define ACTIVITY_BOX_LEFT (STATUS_BOX_LEFT + 336)
#define ACTIVITY_BOX_TOP  (STATUS_BOX_TOP)
#define ACTIVITY_BOX_WIDTH    X_SPACE
#define ACTIVITY_BOX_HEIGHT   STATUS_BOX_HEIGHT

// Child Rendering area
#define ACTIVITY_AREA_LEFT (ACTIVITY_BOX_LEFT + ACTIVITY_BOX_WIDTH / 4)
#define ACTIVITY_AREA_TOP (ACTIVITY_BOX_TOP + BUTTON_HEIGHT / 4) // offset by tasknum of child
#define ACTIVITY_AREA_WIDTH (ACTIVITY_BOX_WIDTH / 2)
#define ACTIVITY_AREA_HEIGHT (BUTTON_HEIGHT / 2)

// Structures and enums

enum Errors {
    err_ok,
    err_failed,             // general failure (unknown)
    err_out_of_memory,
    err_port_failed,        // failed to create port
    err_child_stillborn,    // unable to create child
    };

enum SemaGadgets {
    // Null state
    sg_Null,

    // General
    sg_Start, sg_Halt, sg_Step, sg_Reset, sg_Clear, sg_Help, sg_Exit,

    // Tasks
    sg_Task1, sg_Task2, sg_Task3, sg_Task4, sg_Task5, sg_Task6, sg_Task7,

    // Strings
    sg_Str1, sg_Str2, sg_Str3, sg_Str4, sg_Str5, sg_Str6, sg_Str7,

    // Control
    sg_Busy, sg_Wait, sg_SsW, sg_SsX, sg_EsY, sg_EsZ,

    sg_Count
    };

enum Command {
    // commands to the children
    sc_noop,

    sc_get_message,
    sc_wait,
    sc_busy,              // arg = address of longword to increment, or NULL
    sc_grab_shared,       // arg = semaphore, index = sem number
    sc_grab_exclusive,    // arg = semaphore, index = sem number
    sc_release,           // arg = semaphore, index = sem number
    sc_procure_exclusive, // arg = semaphore, index = bid number
    sc_procure_shared,    // arg = semaphore, index = bid number
    sc_hold,              // arg = semaphore, index = bid number
    sc_vacate,            // arg = semaphore, index = bid number
    sc_die,               // die, o my children
    sc_count,             // tally of sc_ commands

    // notices (cmmmands) to the parent
    sm_dead_abort,      // could not start for some reason (no reply)
    sm_dead,            // died in response to sc_die (no reply)
    sm_my_port,         // child register of port
    sm_what_mytasknum,  // child requesting task number
    sm_what_rport,      // child requesting rastport
    sm_count,           // tally of sm_ commannds
    };

char *cmd_string[] = {
    "sc_noop              (.)  ",
    "sc_get_message            ",
    "sc_wait              (W)  ",
    "sc_busy              (U)  ",
    "sc_grab_shared       (S*) ",
    "sc_grab_exclusive    (E*) ",
    "sc_release           (R*) ",
    "sc_procure_exclusive (P*) ",
    "sc_procure_shared    (p*) ",
    "sc_hold              (H*) ",
    "sc_vacate            (V*) ",
    "sc_die (mommy save us!)   ",
    "",
    "sm_dead_abort             ",
    "sm_dead                   ",
    "sm_my_port                ",
    "sm_what_mytasknum         ",
    "sm_what_rport             ",
    "",
    };

struct SemaMessage  {
    struct Message m;
    struct Task *task; // Task that created this message (owner)

    enum Command cmd;   // command to this task
    void *arg;          // argument passed
    long index;         // general index information
    };

struct SemaState {  // State-Machine
    void (*Method)(struct GlobalEnvironment *ge);   // Activity
    enum SemaGadgets NextState;     // next state transition
    };

struct GlobalEnvironment {
    struct Window *win;
    struct Screen *scr; // Obtained via LockPubScreen()
    struct Gadget *gadlist, *prev_gadget;
    APTR visual_info;
    struct NewGadget ng;    // for NewGadget()
    struct TextAttr ta;
    enum Errors error;  // general error  reg. Non-zero if error

    // Child Tasks
    struct Tasks *child_tasks[NUMBER_OF_TASKS];
    struct MsgPort *child_ports[NUMBER_OF_TASKS];
    struct MsgPort *master_port;

    // Gadgets and such

    struct Gadget *ch_strgads[NUMBER_OF_TASKS];
    char *ch_str[NUMBER_OF_TASKS];
    long ch_index[NUMBER_OF_TASKS]; // indexes for each string
    long ch_loopback[NUMBER_OF_TASKS]; // loopback index

    // State Machine
    struct SemaState *state;    // Pointer to current state

    // Semaphores
    struct SignalSemaphore ss[NUMBER_OF_SEMAPHORES];

    };

#endif
