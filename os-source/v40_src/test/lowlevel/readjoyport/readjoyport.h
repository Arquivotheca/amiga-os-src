/*
 * Constants
 */

#define PROGRAM_NAME "ReadJoyPort"              /* Program name */

#define KICKSTART_VERSION 39                    /* Minimum Kickstart version */
#define WORKBENCH_VERSION 39                    /* Minimum Workbench version */

#define GUI_TYPEFACE "topaz.font"               /* Typeface for GUI */
#define GUI_TYPESIZE 8                          /* Typesize for GUI */

#define GUI_PORT_ID 1
#define GUI_TYPE_ID 2
#define GUI_STICKY_ID 3
#define GUI_RESET_ID 4
#define GUI_MODE_ID 5
#define GUI_BTN1_ID 6
#define GUI_BTN2_ID 7
#define GUI_BTN3_ID 8
#define GUI_BTN4_ID 9
#define GUI_BTN5_ID 10
#define GUI_BTN6_ID 11
#define GUI_BTN7_ID 12
#define GUI_LEFT_ID 13
#define GUI_RIGHT_ID 14
#define GUI_UP_ID 15
#define GUI_DOWN_ID 16
#define GUI_VERTICAL_ID 17
#define GUI_HORIZONTAL_ID 18

#define GUI_MODE_GAME_CONTROLLER 0
#define GUI_MODE_MOUSE 1
#define GUI_MODE_JOYSTICK 2
#define GUI_MODE_AUTOSENSE 3

#define DEFAULT_PORT 0                          /* Default port number */

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

/* from joyport.c */
void pollPort(void);
BOOL gameCtlrCheck(ULONG gcValue);
BOOL joystickCheck(ULONG jsValue);

/*
 * Globals
 */

#ifdef MAIN

struct Library *GfxBase=NULL,                   /* graphics.library base */
    *IntuitionBase=NULL,                        /* intuition.library base */
    *GadtoolsBase=NULL,                         /* gadtools.library base */
    *LowLevelBase=NULL;                         /* lowlevel.library base */

ULONG portNumber=DEFAULT_PORT;                  /* Game controller port */
LONG sticky=FALSE;                              /* Sticky mode */
LONG mode=GUI_MODE_AUTOSENSE;
ULONG portState=~0UL;                           /* Game controller port state
                                                   at last poll */

BOOL debugMode=FALSE;                           /* Debug mode */

struct Window *window=NULL;
void *visualInfo=NULL;
struct Gadget *gadgetList=NULL,
    *portGadget, *typeGadget,
    *stickyGadget, *resetGadget, *modeGadget,
    *btn1Gadget, *btn2Gadget, *btn3Gadget, *btn4Gadget,
        *btn5Gadget, *btn6Gadget, *btn7Gadget,
    *leftGadget, *rightGadget, *upGadget, *downGadget,
    *verticalGadget, *horizontalGadget;

void *parseData=NULL;                           /* WBCliArgs control structure */

#else

extern struct Library *GfxBase, *IntuitionBase,
    *GadtoolsBase, *LowLevelBase;

extern ULONG portNumber;
extern LONG sticky;
extern LONG mode;
extern ULONG portState;

extern BOOL debugMode;

extern struct Window *window;
extern void *visualInfo;
extern struct Gadget *gadgetList,
    *portGadget, *typeGadget,
    *stickyGadget, *resetGadget, *modeGadget,
    *btn1Gadget, *btn2Gadget, *btn3Gadget, *btn4Gadget,
        *btn5Gadget, *btn6Gadget, *btn7Gadget,
    *leftGadget, *rightGadget, *upGadget, *downGadget,
    *verticalGadget, *horizontalGadget;

extern void *parseData;

#endif /* MAIN */
