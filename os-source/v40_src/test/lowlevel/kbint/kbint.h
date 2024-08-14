/*
 * Constants
 */

#define PROGRAM_NAME "KBInt"            /* Program name */

#define KICKSTART_VERSION 40            /* Minimum Kickstart version */
#define WORKBENCH_VERSION 40            /* Minimum Workbench version */
#define KEYMAP_VERSION 37               /* Minimum keymap.library version */
                                        /* !!! kludge alert -- keymap.library
                                           version does not match Kickstart or
                                           Workbench version at time of writing !!! */

#define GUI_TYPEFACE "topaz.font"       /* Typeface for GUI */
#define GUI_TYPESIZE 8                  /* Typesize for GUI */

#define GUI_KEY_ID 1
#define GUI_CODE_ID 2
#define GUI_TRANSITION_ID 3
#define GUI_LSHIFT_ID 4
#define GUI_RSHIFT_ID 5
#define GUI_CAPSLOCK_ID 6
#define GUI_CONTROL_ID 7
#define GUI_LALT_ID 8
#define GUI_RALT_ID 9
#define GUI_LAMIGA_ID 10
#define GUI_RAMIGA_ID 11

#define KBINT_COOKIE 0xC0EDBABE         /* Cookie for interrupt data */

#define KB_MAP_LENGTH 4                 /* Length of buffer for mapped
                                           keyboard event */

/*
 * Prototypes
 */

void main(int argc,char *argv[]);
void goodbye(int returnCode);

BOOL guiOpen(void);
void guiClose(void);
void guiLoop(void);

void kbUpdate(void);
void __asm __interrupt __saveds kbInterrupt(register __d0 UBYTE rawKey,
    register __a1 APTR intData);

/*
 * Globals
 */

#ifdef MAIN

struct Library *IntuitionBase=NULL,             /* intuition.library base */
    *GadtoolsBase=NULL,                         /* gadtools.library base */
    *KeymapBase=NULL,                           /* keymap.library base */
    *LowLevelBase=NULL;                         /* lowlevel.library base */

struct Window *window=NULL;
void *visualInfo=NULL;
struct Gadget *gadgetList=NULL,
    *keyGadget, *codeGadget, *transitionGadget,
    *lShiftGadget, *rShiftGadget,
    *capsLockGadget, *controlGadget,
    *lAltGadget, *rAltGadget,
    *lAmigaGadget, *rAmigaGadget;

APTR kbIntHandle=NULL;                          /* Keyboard interrupt handler */
struct Task *kbIntTask;                         /* KBInt task */
BYTE kbSignal=-1;                               /* Keyboard signal */
ULONG kbIntBadData=0L;                          /* Counter for bad data passed to
                                                   keyboard interrupt handler */
ULONG kbCode;                                   /* Keyboard code */

#else

extern struct Library *IntuitionBase, *GadtoolsBase, *KeymapBase,
    *LowLevelBase;

extern struct Window *window;
extern void *visualInfo;
extern struct Gadget *gadgetList,
    *keyGadget, *codeGadget, *transitionGadget,
    *lShiftGadget, *rShiftGadget,
    *capsLockGadget, *controlGadget,
    *lAltGadget, *rAltGadget,
    *lAmigaGadget, *rAmigaGadget;

extern APTR kbIntHandle;
extern struct Task *kbIntTask;
extern BYTE kbSignal;
extern ULONG kbIntBadData;

extern ULONG kbCode;

#endif /* DEBUG */
