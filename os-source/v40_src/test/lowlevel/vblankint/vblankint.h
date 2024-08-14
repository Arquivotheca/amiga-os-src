/*
 * Constants
 */

#define PROGRAM_NAME "VBlankInt"        /* Program name */

#define KICKSTART_VERSION 40            /* Minimum Kickstart version */
#define WORKBENCH_VERSION 40            /* Minimum Workbench version */

#define SCREEN_WIDTH 640                /* Screen width */
#define SCREEN_HEIGHT GfxBase->NormalDisplayRows /* Screen height */
#define SCREEN_DEPTH 2                  /* Screen depth */
#define SCREEN_DISPLAYID HIRES_KEY      /* Screen display ID */

#define VBLANKINT_COOKIE 0xC0EDBABE     /* Cookie for interrupt data */

#define VBEAMPOS_THRESHOLD_NTSC 20      /* Vertical beam position
                                           threshold for NTSC systems */
#define VBEAMPOS_THRESHOLD_PAL 20       /* Vertical beam position
                                           threshold for PAL systems */

/*
 * Prototypes
 */

/* from main.c */
void main(int argc,char *argv[]);
void goodbye(int returnCode);

/* from vblank.c */
void __asm __interrupt __saveds vblankInterrupt(register __a1 APTR intData);

/*
 * Globals
 */

#ifdef MAIN

struct GfxBase *GfxBase=NULL;                   /* graphics.library base */
struct Library *IntuitionBase=NULL,             /* intuition.library base */
    *LowLevelBase=NULL;                         /* lowlevel.library base */

struct Screen *screen=NULL;                     /* Screen */

struct MsgPort *timerPort=NULL;                 /* timer.device message port */
struct timerequest *timerRequest=NULL;          /* timer.device I/O request */
BOOL timerOpen=FALSE;                           /* timer.device open? */

WORD vbeamThreshold;                            /* Vertical beam position
                                                   threshold */

APTR vblankIntHandle=NULL;                      /* Vertical blank
                                                   interrupt handle */
ULONG vblankIntBadData=0UL;                     /* Counter for calls to
                                                   vertical blank interrupt
                                                   handler with bad data */
ULONG vblankIntThreshold=0UL;                   /* Counter for calls to
                                                   vertical blank interrupt
                                                   handler with vertical
                                                   beam position beyond
                                                   threshold */
ULONG vblankIntCall=0UL;                        /* Counter for total calls
                                                   to vertical blank interrupt
                                                   handler */

#else /* MAIN */

extern struct GfxBase *GfxBase;
extern struct Library *IntuitionBase, *LowLevelBase;

extern struct Screen *screen;

extern WORD vbeamThreshold;

extern APTR vblankIntHandle;
extern ULONG vblankIntBadData;
extern ULONG vblankIntBadVBlank;
extern ULONG vblankIntThreshold;
extern ULONG vblankIntCall;

#endif /* MAIN */
