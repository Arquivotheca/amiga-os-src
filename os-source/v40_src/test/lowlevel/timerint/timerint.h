/*
 * Constants
 */

#define PROGRAM_NAME "TimerInt"         /* Program name */

#define KICKSTART_VERSION 40            /* Minimum Kickstart version */
#define WORKBENCH_VERSION 40            /* Minimum Workbench version */

#define GUI_TYPEFACE "topaz.font"       /* Typeface for GUI */
#define GUI_TYPESIZE 8                  /* Typesize for GUI */

#define GUI_INTERVAL_ID 1
#define GUI_DEVIATION_ID 2
#define GUI_MODE_ID 3
#define GUI_START_ID 4
#define GUI_STOP_ID 5

#define GUI_MODE_ONESHOT_CODE 0         /* Code for one-shot mode */
#define GUI_MODE_CONTINUOUS_CODE 1      /* Code for continuous mode */

#define TIMERINT_COOKIE 0xC0EDBABE      /* Cookie for interrupt data */

/*
 * Prototypes
 */

/* from main.c */
void main(int argc,char *argv[]);
void goodbye(int returnCode);

/* from gui.c */
BOOL guiOpen(void);
void guiClose(void);
void guiLoop(void);

/* from timer.c */
void timerStart(void);
void timerStop(void);
void __asm __interrupt __saveds timerInterrupt(register __a1 APTR intData);

/*
 * Globals
 */

#ifdef MAIN

struct Library *IntuitionBase=NULL,             /* intuition.library base */
    *GadtoolsBase=NULL,                         /* gadtools.library base */
    *TimerBase=NULL,                            /* timer.device base */
    *LowLevelBase=NULL;                         /* lowlevel.library base */

struct MsgPort *timerPort=NULL;                 /* timer.device reply port */
struct timerequest *timerRequest=NULL;          /* timer.device I/O request */

struct Window *window=NULL;
void *visualInfo=NULL;
struct Gadget *gadgetList=NULL,
    *intervalGadget, *deviationGadget, *modeGadget,
    *startGadget, *stopGadget;

APTR timerIntHandle=NULL;                       /* Timer interrupt handle */
LONG timerInterval;                             /* Timer interval (ms) */
BOOL timerContinuous=FALSE;                     /* Continuous timer? */
ULONG timerIntBadData=0L;                       /* Counter for calls to
                                                   timer interrupt handler
                                                   with bad data */
BYTE timerSignal=-1;                            /* Timer interrupt signal
                                                   bit */
struct Task *mainTask;                          /* Main task */
struct timeval startTime, stopTime;             /* Start and stop time as
                                                   measured by timer.device */
LONG deviation=0L;                              /* Deviation */

#else /* MAIN */

extern struct Library *IntuitionBase, *GadtoolsBase, *TimerBase, *LowLevelBase;

extern struct MsgPort *timerPort;
extern struct timerequest *timerRequest;

extern struct Window *window;
extern void *visualInfo;
extern struct Gadget *gadgetList,
    *intervalGadget, *deviationGadget, *modeGadget,
    *startGadget, *stopGadget;

extern APTR timerIntHandle;
extern LONG timerInterval;
extern BOOL timerContinuous;
extern ULONG timerIntBadData;
extern BYTE timerSignal;
extern struct Task *mainTask;
extern struct timeval startTime, stopTime;
extern ULONG deviation;

#endif /* MAIN */
