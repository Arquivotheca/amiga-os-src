/* This program was written to show the use of gadgets in a
 * window. Thus one menu, one auto requester, but lots of
 * gadget types.
 * For the sake of example, two mutual exclude gadgets
 * (female/male) are shown that perform a function that might
 * be better implemented as a toggle image, in a manner similar
 * to that shown in the inflection Mode gadget.
 * Again for the sake of example, the proportional gadgets
 * are generated dynamicly (that is, copied from a template and
 * filled in), whereas most gadgets are declared staticly in
 * the struct declaration, which saves the initialization code
 * space.
 * Lastly, for the sake of example, this program is extremely
 * verbose in the way of comments. Hope you don't mind.
 */
/* Written by David M Lucas. */
/* If you find somthing you don't like, fix it! Have fun! */
/* Thanks Amiga. */

#include "exec/types.h"
#include "exec/exec.h"
#include "intuition/intuition.h"
#include "intuition/intuitionbase.h"
#include "graphics/regions.h"
#include "graphics/copper.h"
#include "graphics/gels.h"
#include "graphics/gfxbase.h"
#include "devices/keymap.h"
#include "hardware/blit.h"
#include "devices/narrator.h"
#include "libraries/translator.h"

/* #define DEBUG */

/* Revision number is used on calls to OpenLibrary
 * to let the system know what revision level the
 * program was written for. Usually refers to the
 * ROM release number or the release number of a
 * disk based library.
 */
#define REVISION 1

#define CONTWINDW 321 /* Overall Control Window Width/Height */
#define CONTWINDH 123
#define FACEWINDW 32  /* Overall Face Window Width / Height */
#define FACEWINDH 44  /* this includes borders, incedently */

/* Pen numbers to draw gadget borders/images/text with */
#define REDP   3        /* color in register 3 once was red */
#define BLKP   2        /* color in register 2 was black */
#define WHTP   1        /* color in register 1 was white */
#define BLUP   0        /* color in register 0 was blue */
/* the length of the English and phonetic buffers */
#define ESTRINGSIZE 512
#define PSTRINGSIZE 768 /* phonemes are longer than english */

#define NUMPROPS 4      /* number of proportional gadgets */

/* Ranges of proportional data */
#define RNGFREQ  (MAXFREQ  - MINFREQ)  +1
#define RNGRATE  (MAXRATE  - MINRATE)  +1
#define RNGPITCH (MAXPITCH - MINPITCH) +1
#define RNGVOL   (MAXVOL   - MINVOL)   +1

struct TextAttr TestFont = /* Needed for opening screen */
   {
   (STRPTR)"topaz.font",
   TOPAZ_EIGHTY, 0, 0
   };

/* Which audio channels to use */
BYTE audio_chan[] = {3, 5, 10, 12};
/* Pointer to translator library vectors */
struct Library *TranslatorBase = 0;
struct MsgPort talk_port; /* Message port for the say's I/O  */
struct MsgPort read_port; /* Message port for the say's I/O  */
struct mouth_rb mouth_io;  /* IO Request block, mouth flavor */
/* IO Request block, narrator flavor */
struct narrator_rb voice_io;
/* indicative of the Open return */
UBYTE NarratorOpenError = 0;
/* indicative of a Translations success */
UBYTE TranslatorError = 0;
USHORT i;

/* These are used to draw the eyes and mouth size relative */
USHORT MouthWMult;
USHORT EyesLeft;
USHORT MouthHMult;
USHORT EyesTop;
USHORT EyesBottom;
USHORT YMouthCenter;  /* Pixels from top edge */
USHORT XMouthCenter;  /* Pixels from left edge */
USHORT yaw;
USHORT LipWidth, LipHeight;

/* String Gadgets *********************************************
 * First the string gadgets.
 * 1) because the Phonetic string is refreshed programaticly
 * (that is, deleted and added again) quite often, and doing
 * this requires the use of RefreshGadgets(), and this causes
 * gadgets that are closer to the beginning of the list than
 * the gadget given to RefreshGadgets() to flicker.
 * 2) because they don't flicker when OTHER gadgets
 * (ie female/male, coming up) are deleted and added.
 */
/* These'll be used to put a nice double line border around
 * each of the two string gadgets.
 */
/* y,x pairs drawn as a connected line. Be sure to have an even
 * number of arguments (ie complete pairs).
 */

USHORT StrVectors[] = {
   0, 0,   297, 0,   297, 14,   0, 14,
   0, 1,   296, 1,   296, 13,   1, 13,
   1, 1
};
struct Border StrBorder = {
   -4, -3,           /* initial offsets, gadget relative */
   WHTP, BLUP, JAM1, /* pens (fore, back) and drawmode */
   9,                /* number of vectors */
   StrVectors,     /* pointer to the actual array of vectors */
   NULL       /* no next Border, can point to another border */
};

/* The same undo buffer is used for both string gadgets,
 * this is sized to largest so that largest fits.
 */
UBYTE UndoBuffer[PSTRINGSIZE];

/* English String Gadget is where the user types in English */
/* default text */
UBYTE EnglBuffer[ESTRINGSIZE] = "This is amiga speaking.";
struct StringInfo EnglInfo = {
   EnglBuffer,    /* pointer to I/O buffer */
   UndoBuffer,    /* pointer to undo buffer */
   0,             /* buffer position */
   ESTRINGSIZE,   /* max number of chars, including NULL */
   0, 0,          /* first char in display, undo positions */
   24,          /* number of chars (currently) in the buffer */
   0, 0, 0,    /* position variables calculated by Intuition */
   NULL,          /* no pointer to RastPort */
   0,             /* not a LongInt string gadget */
   NULL           /* no pointer to alternate keymap */
};
struct IntuiText EnglText = {
   WHTP, BLUP,    /* FrontPen, BackPen */
   JAM1,          /* DrawMode */
   0, -13,        /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,     /* pointer to TextFont */
   "English:",    /* pointer to Text */
   NULL           /* no pointer to NextText */
};
struct Gadget EnglStrGadget = {
   NULL,             /* pointer to Next Gadget */
   11, 63, 290, 10,  /* (Left Top Width Height) Hit Box */
   GADGHCOMP,        /* Flags */
   RELVERIFY,        /* Activation flags */
   STRGADGET,        /* Type */
   (APTR)&StrBorder, /* pointer to Border Image */
   NULL,             /* no pointer to SelectRender */
   &EnglText,        /* pointer to GadgetText */
   0,                /* no MutualExclude */
   (APTR)&EnglInfo,  /* pointer to SpecialInfo */
   0,                /* no ID */
   NULL              /* no pointer to special data */
};

/* Phonetic string gadget is where the program puts the
 * translated string, necessating a call to RefreshGadgets(),
 * and is where the user can type in Phonemes.
 */
UBYTE PhonBuffer[PSTRINGSIZE] =
  "DHIHS IHZ AHMIY3GAH SPIY4KIHNX.";
struct StringInfo PhonInfo = {
   PhonBuffer,    /* pointer to input buffer */
   UndoBuffer,    /* pointer to undo buffer */
   0,             /* initial buffer position */
   PSTRINGSIZE,   /* max number of chars, including NULL */
   0, 0,          /* display, undo positions */
   32,          /* number of chars (currently) in the buffer */
   0, 0, 0,    /* position variables calculated by Intuition */
   NULL,          /* no pointer to RastPort */
   NULL,          /* not a LongInt string gadget */
   NULL           /* no pointer to alternate keymap */
};
struct IntuiText PhonText = {
   WHTP, BLUP,    /* FrontPen, BackPen */
   JAM1,          /* DrawMode */
   0, -13,        /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,     /* pointer to TextFont */
   "Phonetics:",  /* pointer to Text */
   NULL           /* no pointer to NextText */
};
struct Gadget PhonStrGadget = {
   &EnglStrGadget,   /* pointer to Next Gadget */
   11, 94, 290, 10,  /* (Left Top Width Height) Hit Box */
   GADGHCOMP,        /* Flags */
   RELVERIFY,        /* Activation flags */
   STRGADGET,        /* Type */
   (APTR)&StrBorder, /* pointer to Border Image */
   NULL,             /* no pointer to SelectRender */
   &PhonText,        /* pointer to GadgetText */
   0,                /* no MutualExclude */
   (APTR)&PhonInfo,  /* pointer to SpecialInfo */
   0,                /* no ID */
   NULL              /* no pointer to special data */
};

/* Now come the Boolean Gadgets.
 * The female/male pair shows the simplest implementation I
 * could think of to show how you can do mutual-exclude type
 * things yourself. They are two toggle gadgets that use
 * highlight image. The program starts with one selected, and
 * then if either of them are hit, both toggle. Until either
 * of them is hit again. Gadgets must be deleted and added
 * whenever you want to change structure member values that
 * intuition expects to be coming from the user, not a program
 * (like the SELECTED bit in flags). Note that certain
 * structure values CAN be changed programaticly without all
 * this broohaha. Haha. Consult the intuition manual.
 */
/* Female Toggle (Highlight Image)
   (Quasi mutual exclude with Male Toggle) */
struct Image FemaleImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   20, 10, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Gadget FemaleGadget = {
   &PhonStrGadget,      /* pointer to Next Gadget */
   134, 34, 20, 10,     /* (Left Top Width Height) Hit Box */
   GADGIMAGE | GADGHCOMP,  /* Flags */
   /* Activation flags */
   RELVERIFY | GADGIMMEDIATE | TOGGLESELECT,
   BOOLGADGET,          /* Type */
   (APTR)&FemaleImage,  /* pointer to GadgetRender */
   NULL,                /* no pointer to SelectRender */
   NULL,                /* no pointer to GadgetText */
   0,                   /* no MutualExclude */
   NULL,                /* no pointer to SpecialInfo */
   0,                   /* no ID */
   NULL                 /* no pointer to special data */
};

/* Male Toggle (Highlight Image)
   (Quasi mutual Exclude with above) */
struct Image MaleImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   20, 10, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Gadget MaleGadget = {
   &FemaleGadget,    /* pointer to Next Gadget */
   154, 34, 20, 10,  /* (Left Top Width Height) Hit Box */
   GADGIMAGE | GADGHCOMP | SELECTED,   /* Flags */
   /* Activation flags */
   RELVERIFY | GADGIMMEDIATE | TOGGLESELECT | ACTIVATE,
   BOOLGADGET,       /* Type */
   (APTR)&MaleImage, /* pointer to GadgetRender */
   NULL,             /* no pointer to SelectRender */
   NULL,             /* no pointer to GadgetText */
   0,                /* no MutualExclude */
   NULL,             /* no pointer to SpecialInfo */
   0,                /* no ID */
   NULL              /* no pointer to special data */
};

/* This boolean toggle gadget has an
 * alternate image that indicates
 * selection. The image stays flipped
 * until it gets another hit. (it toggles)
 */
/* Inflection Mode Toggle (AltImage) *************************/
struct Image HumanImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   40, 20, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Image RobotImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   40, 20, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Gadget ModeGadget = {
   &MaleGadget,            /* pointer to Next Gadget */
   134, 2, 40, 20,        /* (Left Top Width Height) Hit Box */
   GADGIMAGE | GADGHIMAGE, /* Flags */
   /* Activation flags */
   RELVERIFY | GADGIMMEDIATE | TOGGLESELECT,
   BOOLGADGET,             /* Type */
   (APTR)&HumanImage,      /* pointer to GadgetRender */
   (APTR)&RobotImage,      /* pointer to SelectRender */
   NULL,                   /* no pointer to GadgetText */
   0,                      /* no MutualExclude */
   NULL,                   /* no pointer to SpecialInfo */
   0,                      /* no ID */
   NULL                    /* no pointer to special data */
};

/* Face Toggle (image and text) ******************************/
struct IntuiText FaceIText = {
   WHTP, BLUP, /* FrontPen, BackPen */
   JAM2,       /* DrawMode */
   4, 1,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,  /* pointer to TextFont */
   "Face",/* pointer to Text */
   NULL        /* no pointer to NextText */
};
struct Image FaceImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   40, 10, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Gadget FaceGadget = {
   &ModeGadget,            /* pointer to Next Gadget */
   134, 23, 40, 10,       /* (Left Top Width Height) Hit Box */
   GADGIMAGE | GADGHCOMP,  /* Flags */
   /* Activation flags */
   RELVERIFY | GADGIMMEDIATE | TOGGLESELECT,
   BOOLGADGET,             /* Type */
   (APTR) &FaceImage,      /* pointer to GadgetRender */
   NULL,                   /* no pointer to SelectRender */
   &FaceIText,             /* pointer to GadgetText */
   0,                      /* no MutualExclude */
   NULL,                   /* no pointer to SpecialInfo */
   0,                      /* no ID */
   NULL                    /* no pointer to special data */
};

/* Stop Hit (image and text) ******************************/
struct IntuiText StopIText = {
   WHTP, BLUP, /* FrontPen, BackPen */
   JAM2,       /* DrawMode */
   4, 1,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,  /* pointer to TextFont */
   "Stop",/* pointer to Text */
   NULL        /* no pointer to NextText */
};
struct Image StopImage = {
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   40, 10, 1,  /* Width, Height, Depth */
   NULL,       /* pointer to ImageData */
   0x1, 0x0    /* PlanePick, PlaneOnOff */
};
struct Gadget StopGadget = {
   &FaceGadget,            /* pointer to Next Gadget */
   134, 45, 40, 10,        /* Left Top Width Height Hit Box */
   GADGIMAGE | GADGHCOMP,  /* Flags */
   RELVERIFY | GADGIMMEDIATE, /* Activation flags */
   BOOLGADGET,             /* Type */
   (APTR) &StopImage,      /* pointer to GadgetRender */
   NULL,                   /* no pointer to SelectRender */
   &StopIText,             /* pointer to GadgetText */
   0,                      /* no MutualExclude */
   NULL,                   /* no pointer to SpecialInfo */
   0,                      /* no ID */
   NULL                    /* no pointer to special data */
};

/* This is a hit (as opposed to toggle)
   gadget that starts the translation.*/
/* Translate Hit (Highlight image) ***************************/
USHORT TransVectors[] = {
   0, 0,    79, 0,    79, 13,   0, 13,
   0, 1,    78, 1,    78, 12,   1, 12,
   1, 1
};
struct Border TransBorder = {
   -4, -3,           /* initial offsets, gadget relative */
   WHTP, BLUP, JAM1, /* pens (fore, back) and drawmode */
   9,                /* number of vectors */
   TransVectors,     /* pointer to the array of vectors */
   NULL              /* no next Border, can point to another */
};
struct IntuiText TranslateIText = {
   WHTP, BLUP, /* FrontPen, BackPen */
   JAM2,       /* DrawMode */
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,  /* pointer to TextFont */
   "Translate",/* pointer to Text */
   NULL        /* no pointer to NextText */
};
struct Gadget TranslateGadget = {
   &StopGadget,      /* pointer to Next Gadget */
   229, 48, 71, 8,   /* (Left Top Width Height) Hit Box */
   GADGHCOMP,        /* Flags */
   RELVERIFY | GADGIMMEDIATE, /* Activation flags */
   BOOLGADGET,       /* Type */
   (APTR)&TransBorder,  /* no pointer to GadgetRender */
   NULL,             /* no pointer to SelectRender */
   &TranslateIText,  /* pointer to GadgetText */
   0,                /* no MutualExclude */
   NULL,             /* no pointer to SpecialInfo */
   0,                /* no ID */
   NULL              /* no pointer to special data */
};

/* This is a hit (as opposed to toggle) Starts the narration */
/* Speak Hit (Highlight Image) *******************************/
USHORT SpeakVectors[] = {
   0, 0,    47, 0,    47, 13,   0, 13,
   0, 1,    46, 1,    46, 12,   1, 12,
   1, 1
};
struct Border SpeakBorder = {
   -4, -3,           /* initial offsets, gadget relative */
   WHTP, BLUP, JAM1, /* pens (fore, back) and drawmode */
   9,                /* number of vectors */
   SpeakVectors,   /* pointer to the actual array of vectors */
   NULL       /* no next Border, can point to another border */
};
struct IntuiText SpeakIText = {
   WHTP, BLUP, /* FrontPen, BackPen */
   JAM2,       /* DrawMode */
   0, 0,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,  /* pointer to TextFont */
   "Speak",    /* pointer to Text */
   NULL        /* no pointer to NextText */
};
struct Gadget SpeakGadget = {
   &TranslateGadget, /* pointer to Next Gadget */
   261, 79, 40, 8,   /* (Left Top Width Height) Hit Box */
   GADGHCOMP,        /* Flags */
   RELVERIFY | GADGIMMEDIATE, /* Activation flags */
   BOOLGADGET,       /* Type */
   (APTR)&SpeakBorder,  /* pointer to GadgetRender */
   NULL,             /* no pointer to SelectRender */
   &SpeakIText,      /* pointer to GadgetText */
   0,                /* no MutualExclude */
   NULL,             /* no pointer to SpecialInfo */
   0,                /* no ID */
   NULL              /* no pointer to special data */
};

/* Now the proportional gadgets. */
/* Proportional Gadgets **************************************/
/* The following variables are used to create proportional
 * Gadgets. These variables will be filled in with copies of
 * the generic Gadgetry below.
 */
SHORT PropCount = 0;       /* index to next available Gadget */
struct IntuiText PTexts[NUMPROPS];/* get copies of TPropText */
/* dummy AUTOKNOB Images are required */
struct Image PImages[NUMPROPS];
/* These get copies of TPropInfo */
struct PropInfo PInfos[NUMPROPS];
/* These get copies of TPropGadget */
struct Gadget Props[NUMPROPS];
struct IntuiText TPropText = {
   WHTP, BLUP,   /* FrontPen, BackPen */
   JAM1,         /* DrawMode */
   0, -10,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,    /* pointer to TextFont */
   NULL,         /* pointer to Text is filled at runtime */
   NULL          /* no pointer to NextText */
};   
struct PropInfo TPropInfo = {
   AUTOKNOB | FREEHORIZ,  /* Flags */
   0, 0,            /* Pots:  Horiz, Vert: both start at 0 */
   0x1FFF, 0x1FFF,  /* Bodies: Horiz is 1/8, Vert is 1/8 */
   0, 0, 0, 0, 0, 0 /* System usage stuff */
};
/* this is the template for the Gadget of a horizontal */
/* Proportional Gadget */
struct Gadget TPropGadget = {
   &SpeakGadget,              /* pointer to NextGadget */
   7, 12, 115, 10,            /* Select Box L T W H */
   GADGHCOMP | GADGIMAGE,     /* Flags */
   GADGIMMEDIATE | RELVERIFY, /* Activation flags */
   PROPGADGET,                /* Type */
   NULL,       /* pointer to Image filled in later */
   NULL,       /* no pointer to SelectRender */
   NULL,       /* no pointer to GadgetText */
   0,          /* no MutualExclude */
   NULL,       /* SpecialInfo proportional data filled later */
   0,          /* no ID */
   NULL        /* no pointer to special data */
};

struct IntuitionBase *IntuitionBase = 0;
struct GfxBase *GfxBase = 0;

/* Only one menu. */

ULONG MenuNumber;
ULONG TheMenu;
ULONG TheItem;

struct IntuiText MenuItemText = {
   BLUP,         /* Front Pen */
   WHTP,         /* Back pen */
   JAM2,         /* Draw Mode */
   0,            /* Left Edge */
   0,            /* Top */
   &TestFont,    /* pointer to TextFont */
   "About SpeechToy...", /* text */
   NULL          /* next */
};
struct MenuItem MyMenuItem = {
   NULL,                /* pointer to next item */
   0,                   /* left */
   0,                   /* top */
   150,                 /* width */
   8,                   /* height */
   ITEMTEXT | ITEMENABLED | HIGHCOMP,  /* flags */
   0,                   /* no mutual exclude */
   (APTR)&MenuItemText, /* Render */
   NULL,                /* pointer to alternate image */
   NULL,                /* Command "amiga" char */
   NULL,                /* Sub Item */
   MENUNULL             /* nextselect */
};

struct Menu MyMenu = {
   NULL,          /* pointer to next menu */
   0,0,150,0,     /* left,0,Width,0 */
   MENUENABLED,   /* flags */
   "SpeachToy Menu",   /* menu name */
   &MyMenuItem    /* First Item in list */
};

struct IntuiText ReqText1 = {
   BLUP,         /* Front Pen */
   WHTP,         /* Back pen */
   JAM2,         /* Draw Mode */
   5,            /* Left Edge */
   23,           /* Top */
   &TestFont,    /* pointer to TextFont */
   "Version 1.1   8 Dec, 1985",  /* text */
   NULL          /* next */
};
struct IntuiText ReqText2 = {
   BLUP,         /* Front Pen */
   WHTP,         /* Back pen */
   JAM2,         /* Draw Mode */
   5,            /* Left Edge */
   13,           /* Top */
   &TestFont,    /* pointer to TextFont */
   "Freeware - Public Domain ", /* text */
   &ReqText1     /* next */
};
struct IntuiText ReqText3 = {
   BLUP,         /* Front Pen */
   WHTP,         /* Back pen */
   JAM2,         /* Draw Mode */
   5,            /* Left Edge */
   3,            /* Top */
   &TestFont,    /* pointer to TextFont */
   "Written by David M Lucas ", /* text */
   &ReqText2     /* next */
};

struct IntuiText OKIText = {
   BLUP, WHTP, /* FrontPen, BackPen */
   JAM2,       /* DrawMode */
   6, 3,       /* LeftEdge, TopEdge (relative to gadget) */
   &TestFont,  /* pointer to TextFont */
   "OK",       /* pointer to Text */
   NULL        /* no pointer to NextText */
};

struct Requester *AboutRequester;

USHORT autoret;
struct Window *ControlWindow = NULL;
struct Window *FaceWindow = NULL;
struct IntuiMessage *MyIntuiMessage;
struct NewWindow NewControlWindow = {
   00, 11,                    /* start LeftEdge, TopEdge */
   CONTWINDW, CONTWINDH,      /* start Width, Height */
   -1, -1,                    /* DetailPen, BlockPen */
   GADGETUP | CLOSEWINDOW | MENUPICK,  /* IDCMP FLAGS */
   WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE
   | GIMMEZEROZERO | ACTIVATE,/* Flags (can be NULL) */
   &Props[NUMPROPS-1],        /* Pointer to FirstGadget */
   NULL,                    /* no pointer to first CheckMark */
   "SpeechToy",               /* Title (can be NULL) */
   NULL,                      /* no Pointer to Screen */
   NULL,                      /* no Pointer to BitMap */
   20, 20,                    /* Min/max  Sizable to (w/h) */
   CONTWINDW, CONTWINDH,      /* These aint used, can't size */
   WBENCHSCREEN         /* Type of screen window appears in */
};

struct NewWindow NewFaceWindow = {
   CONTWINDW, 11,             /* start LeftEdge, TopEdge */
   FACEWINDW * 2, FACEWINDH,  /* start Width, Height */
   -1, -1,                    /* DetailPen, BlockPen */
   SIZEVERIFY | NEWSIZE,      /* IDCMP FLAGS */
   /* Flags (can be NULL) */
   WINDOWDRAG | WINDOWDEPTH | WINDOWSIZING
    | SIZEBBOTTOM | GIMMEZEROZERO | ACTIVATE,
   NULL,                /* no Pointer to FirstGadget */
   NULL,                /* no Pointer to first CheckMark */
   "Face",              /* Title */
   NULL,                /* no Pointer to Screen */
   NULL,                /* no Pointer to BitMap */
   FACEWINDW, FACEWINDH,/* Minimum sizeable to */
   640, 200,            /* Maximum sizeable to */
   WBENCHSCREEN         /* Type of screen window appears in */
};

USHORT FemaleIData[] = {
/*   ----    -  These nibbles matter to image. */
   0x0000, 0x0000,
   0x00F0, 0x0000,
   0x0198, 0x0000,
   0x030C, 0x0000,
   0x0198, 0x0000,
   0x00F0, 0x0000,
   0x0060, 0x0000,
   0x01F8, 0x0000,
   0x0060, 0x0000,
   0x0000, 0x0000
};

USHORT MaleIData[] = {
/*   ----    -   These nibbles matter to image. */
   0x0000, 0x0000,
   0x003E, 0x0000,
   0x000E, 0x0000,
   0x0036, 0x0000,
   0x01E0, 0x0000,
   0x0330, 0x0000,
   0x0618, 0x0000,
   0x0330, 0x0000,
   0x01E0, 0x0000,
   0x0000, 0x0000
};
USHORT HumanIData[] = {
/*   ----   ----   --   These nibbles matter to image. */
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0007,0x9E00,0x0000,
   0x0001,0x8600,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x2000,0x0000,
   0x0000,0x1000,0x0000,
   0x0000,0x0800,0x0000,
   0x0000,0x7C00,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x7800,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000
};
USHORT RobotIData[] = {
/*   ----   ----   --   These nibbles matter to image. */
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0007,0x9E00,0x0000,
   0x0004,0x9200,0x0000,
   0x0007,0x9E00,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0001,0xF800,0x0000,
   0x0001,0x0800,0x0000,
   0x0001,0xF800,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000
};
USHORT FaceIData[] = {
/*   ----   ----   --   These nibbles matter to image. */
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000
};
USHORT StopIData[] = {
/*   ----   ----   --   These nibbles matter to image. */
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000,
   0x0000,0x0000,0x0000
};
USHORT *FemaleIData_chip = 0;
USHORT *MaleIData_chip = 0;
USHORT *HumanIData_chip = 0;
USHORT *RobotIData_chip = 0;
USHORT *FaceIData_chip = 0;
USHORT *StopIData_chip = 0;

/** start of code ***************************/
main()
{
   ULONG Signals;        /* Wait() tells me which to look at */
   ULONG MIClass;        /* Save quickly, ReplyMsg() asap */
   USHORT MICode;        /* These hold my needed information */
   APTR MIAddress;

   /* let MyCleanup know these signals not allocated yet */
   talk_port.mp_SigBit = -1;
   read_port.mp_SigBit = -1;

   /* Open those libraries that the program uses directly */
   if ((IntuitionBase = (struct IntuitionBase *)
    OpenLibrary("intuition.library", REVISION)) == 0) {
#ifdef DEBUG
      printf("Can't open the intuition library\n");
#endif
      MyCleanup();
      exit(FALSE);
   }

   if ((GfxBase = (struct GfxBase *)
    OpenLibrary("graphics.library", REVISION)) == 0) {
#ifdef DEBUG
      printf("Can't open the graphics library\n");
#endif
      MyCleanup();
      exit(FALSE);
   }

   if ((TranslatorBase = (struct Library *)
    OpenLibrary("translator.library", REVISION)) == 0) {
#ifdef DEBUG
      printf("Can't open the translator library\n");
#endif
      MyCleanup();
      exit(FALSE);
   }

   /* Open the device */
   if ((NarratorOpenError = OpenDevice("narrator.device", 0,
    &voice_io, 0)) != 0) {
#ifdef DEBUG
      printf("Can't open the narrator device\n");
#endif
      MyCleanup();
      exit(FALSE);
   }
   /* This is where the proportional gadgets are set up, using
    * the templates that were declared staticly.
    */
   for(PropCount = 0; PropCount < NUMPROPS; PropCount++) {
      PTexts[PropCount] = TPropText;
      Props[PropCount] = TPropGadget;
      PInfos[PropCount] = TPropInfo;
      Props[PropCount].GadgetText = (struct IntuiText *)
       &PTexts[PropCount];
      Props[PropCount].GadgetRender = (APTR)
       &PImages[PropCount];
      Props[PropCount].SpecialInfo = (APTR)&PInfos[PropCount];
      switch (PropCount) {
      case 0:
         PTexts[PropCount].IText = "Sample Freq:";
         if (DEFFREQ == MAXFREQ)
            PInfos[PropCount].HorizPot = 65535;
         else
            PInfos[PropCount].HorizPot = ((DEFFREQ - MINFREQ)
             << 16) / (MAXFREQ - MINFREQ);
         break;
      case 1:
         PTexts[PropCount].IText = "Rate:";
         Props[PropCount].TopEdge += 22;
         Props[PropCount].NextGadget = &Props[PropCount-1];
         if (DEFRATE == MAXRATE)
            PInfos[PropCount].HorizPot = 65535;
         else
            PInfos[PropCount].HorizPot = ((DEFRATE - MINRATE)
             << 16) / (MAXRATE - MINRATE);
         break;
      case 2:
         PTexts[PropCount].IText = "Pitch:";
         Props[PropCount].LeftEdge += 183;
         Props[PropCount].NextGadget = &Props[PropCount-1];
         if (DEFPITCH == MAXPITCH)
            PInfos[PropCount].HorizPot = 65535;
         else
            PInfos[PropCount].HorizPot = ((DEFPITCH-MINPITCH)
             << 16) / (MAXPITCH - MINPITCH);
         break;
      case 3:
         PTexts[PropCount].IText = "Volume:";
         Props[PropCount].TopEdge += 22;
         Props[PropCount].LeftEdge += 183;
         Props[PropCount].NextGadget = &Props[PropCount-1];
         if (DEFVOL == MAXVOL)
            PInfos[PropCount].HorizPot = 65535;
         else
            PInfos[PropCount].HorizPot = ((DEFVOL - MINVOL)
             << 16) / (MAXVOL - MINVOL);
         break;
/* the following for lattice bug */
/* it likes 1,2,3,5.. but not 4 in a switch */
      case 4: break;
      }
   }

   /* Now allocate memory accessable by the chips for images */
   if (InitImages() != TRUE) {
#ifdef DEBUG
      printf("Couldn't Allocate Images in chip memory.\n");
#endif
      MyCleanup();
      exit(FALSE);
   }

   /* Set up the write port, allocate the signal, */
   /* and the message */
   talk_port.mp_Node.ln_Type = NT_MSGPORT;
   talk_port.mp_Flags = 0;
   if ((talk_port.mp_SigBit = AllocSignal(-1)) == -1) {
#ifdef DEBUG
      printf("Couldn't Allocate talk Signal bit\n");
#endif
      MyCleanup();
      exit(FALSE);
   }
   talk_port.mp_SigTask = (struct Task *) FindTask((char *)
    NULL);
   NewList(&talk_port.mp_MsgList);

   /* Set up the read port, allocate the signal, */
   /*  and the message */
   read_port.mp_Node.ln_Type = NT_MSGPORT;
   read_port.mp_Flags = 0;
   if ((read_port.mp_SigBit = AllocSignal(-1)) == -1) {
#ifdef DEBUG
      printf("Couldn't Allocate read Signal bit\n");
#endif
      MyCleanup();
      exit(FALSE);
   }
   read_port.mp_SigTask = (struct Task *)
    FindTask((char *) NULL);
   NewList(&read_port.mp_MsgList);

   /* Set up the write channel information */
   voice_io.ch_masks = (audio_chan);
   voice_io.nm_masks = sizeof(audio_chan);
   voice_io.mouths = 0;
   voice_io.message.io_Message.mn_ReplyPort = &talk_port;
   voice_io.message.io_Command = CMD_WRITE;
   voice_io.message.io_Offset = 0;
   voice_io.message.io_Data = (APTR)PhonBuffer;
   voice_io.message.io_Message.mn_Length = sizeof(voice_io);

  /* Set up the read channel information */
   mouth_io.voice = voice_io;
   mouth_io.width = 0;
   mouth_io.height = 0;
   mouth_io.voice.message.io_Message.mn_ReplyPort = &read_port;
   mouth_io.voice.message.io_Command = CMD_READ;
   mouth_io.voice.message.io_Error = 0;
   if (FaceWindow == NULL) {
      if ((ControlWindow = (struct Window *)OpenWindow
       (&NewControlWindow)) == NULL) {
#ifdef DEBUG
         printf("Couldn't open the control window.\n");
#endif
         MyCleanup();
         exit(FALSE);
      }
   }

   /* fill background of window */
   SetAPen(ControlWindow->RPort, BLKP);
   RectFill(ControlWindow->RPort,0,0,
    ControlWindow->GZZWidth, ControlWindow->GZZHeight);
   RefreshGadgets(&Props[NUMPROPS-1],ControlWindow,NULL);

   SetMenuStrip(ControlWindow, &MyMenu);

/* !!! Ah, But what if FaceWindow's not been opened? */
   for (;;) {  /* ever wait for a signal and process it */
      /* wait lets the rest of the system run, */
      /* this program sleeps */
      Signals = Wait((1 << ControlWindow->UserPort->mp_SigBit)
| (1 << FaceWindow->UserPort->mp_SigBit)
| (1 << voice_io.message.io_Message.mn_ReplyPort->mp_SigBit)
| (1 <<
   mouth_io.voice.message.io_Message.mn_ReplyPort->mp_SigBit));
      /* now check to see to what we owe the intrusion */

      if (Signals & (1<< ControlWindow->UserPort->mp_SigBit)) {
         /* Process the Intuition message */
         while (MyIntuiMessage=(struct IntuiMessage *)
                      GetMsg(ControlWindow->UserPort)) {
            /* Get all the needed info and give message back */
            MIClass = MyIntuiMessage->Class;
            MICode = MyIntuiMessage->Code;
            MIAddress = MyIntuiMessage->IAddress;
            ReplyMsg(MyIntuiMessage);
            /* Now, what was it you wanted? */
            switch (MIClass) {
               case MENUPICK:
                  menumessage(MICode, ControlWindow);
                  break;
               case GADGETUP:         /* reply, then process */
                  gadgetmessage(MIAddress, ControlWindow);
                  break;
               case CLOSEWINDOW:       /* bye! */
                  while (MyIntuiMessage = (struct
                   IntuiMessage *) GetMsg(
                   ControlWindow->UserPort))
                    ReplyMsg(MyIntuiMessage);
                  MyCleanup();
                  exit(TRUE);
                  break;
               default:
#ifdef DEBUG
                  printf("Unhandled Message Received.\n");
#endif
                  break;
            }  /* switch */
         } /* while */
      } /* if */


      /* Woken by intuition for FaceWindow*/
      if (Signals & (1<< FaceWindow->UserPort->mp_SigBit)) {
         /* Process the Intuition message */
         while (MyIntuiMessage=(struct IntuiMessage *)
          GetMsg(FaceWindow->UserPort)) {
            switch (MyIntuiMessage->Class) {
               case SIZEVERIFY:
                  ReplyMsg(MyIntuiMessage);
                  break;
               case NEWSIZE:  /* Don't reply until processed */
                  DrawFace();
                  ReplyMsg(MyIntuiMessage);
                  break;
               default:
#ifdef DEBUG
                  printf("Unhandled Message Received.\n");
#endif
                  ReplyMsg(MyIntuiMessage);
                  break;
            }  /* switch */
         } /* while */
      } /* if */

      /* A voice SendIO (Write) has completed */
      if (Signals & (1 <<
       voice_io.message.io_Message.mn_ReplyPort->mp_SigBit)) {
         /* Was it Sucessful? filter out the abort error */
         if (voice_io.message.io_Error == -2)
            voice_io.message.io_Error = 0;
         if (voice_io.message.io_Error != 0) {
#ifdef DEBUG
            printf("Narrator won't. (%ld)\n",
             voice_io.message.io_Error);
#endif
            /* flash this screen */
            DisplayBeep(ControlWindow->WScreen);
            /* let user see where phoneme string was bad. */
            i = RemoveGadget(ControlWindow, &PhonStrGadget);
            /* move the cursor to the error char */
            PhonInfo.BufferPos = voice_io.message.io_Actual -1;
            /* assure cursor (error point) is shown in gad. */
            /* within 29 (number of chars shown) of front */
            if (voice_io.message.io_Actual < 29)
               PhonInfo.DispPos = 0;
            /* within 29 of end */
            else if ((voice_io.message.io_Length -
                      voice_io.message.io_Actual) < 29)
               PhonInfo.DispPos = voice_io.message.io_Length
                                  - 29;
            else
               PhonInfo.DispPos = voice_io.message.io_Actual
                                  - 15;
            AddGadget(ControlWindow, &PhonStrGadget, i);
            RefreshGadgets(&PhonStrGadget, ControlWindow,
             NULL);
            voice_io.message.io_Error = 0;
         }
         SpeakGadget.Flags ^= GADGDISABLED;
         FaceGadget.Flags ^= GADGDISABLED;
      }

      /* A mouth DoIO (Read) has completed */
      if (Signals & (1 <<
  mouth_io.voice.message.io_Message.mn_ReplyPort->mp_SigBit)) {
         WaitBOVP(&FaceWindow->WScreen->ViewPort);
         SetAPen(FaceWindow->RPort, WHTP);
         RectFill(FaceWindow->RPort, 0, EyesBottom,
          FaceWindow->GZZWidth, FaceWindow->GZZHeight);
         if (MouthWMult == 0)
            LipWidth = mouth_io.width >> 1;
         else
            LipWidth = mouth_io.width * MouthWMult;
         if (MouthHMult == 0)
            LipHeight = mouth_io.height >> 1;
         else
            LipHeight = mouth_io.height * (MouthHMult);
         SetAPen(FaceWindow->RPort, REDP);
         Move(FaceWindow->RPort,
          XMouthCenter - LipWidth, YMouthCenter);
         Draw(FaceWindow->RPort,
          XMouthCenter, YMouthCenter - LipHeight);
         Draw(FaceWindow->RPort,
          XMouthCenter + LipWidth, YMouthCenter);
         Draw(FaceWindow->RPort,
          XMouthCenter, YMouthCenter + LipHeight);
         Draw(FaceWindow->RPort,
          XMouthCenter - LipWidth, YMouthCenter);
         /* the narrator will give an error when the */
         /* write has completed and I've tried to read */
         /* so I stop trying when that happens */
         if (mouth_io.voice.message.io_Error == 0)
            SendIO(&mouth_io);
      }
   }  /* for */
}  /* main */

/* a MENUPICK has been received, this
 * routine takes the appropriate action
 */
menumessage(code, w)
USHORT code;
struct Window *w;
{
   switch (MENUNUM(code)) {
      case 0:
         switch (ITEMNUM(code)) {
            case 0:
               AutoRequest(w, &ReqText3, NULL,
                           &OKIText, 0, 0, 280, 47);
               break;
         }
      break;
   }
}

/* a GADGETUP has been received, this
 * routine takes the appropriate action
 */
gadgetmessage(address, w)
APTR address;
struct Window *w;
{
   USHORT i;
   long PropRange;
   if (address == (APTR)&ModeGadget) {
      if (ModeGadget.Flags & SELECTED)
         voice_io.mode = ROBOTICF0;
      else
         voice_io.mode = NATURALF0;
   }
   else if (address == (APTR)&FaceGadget) {
      /* tell the write that reads will be forthcomming */
      if (FaceGadget.Flags & SELECTED) {
         voice_io.mouths = 1;
         if ((FaceWindow = (struct Window *)
          OpenWindow(&NewFaceWindow)) == NULL) {
#ifdef DEBUG
            printf("Couldn't open the face window.\n");
#endif
            MyCleanup();
            exit(FALSE);
         }
         SetMenuStrip(FaceWindow, &MyMenu);
         DrawFace();
      }
      else { /* FaceGadget de-SELECTed */
         voice_io.mouths = 0;
         NewFaceWindow.LeftEdge = FaceWindow->LeftEdge;
         NewFaceWindow.TopEdge = FaceWindow->TopEdge;
         NewFaceWindow.Width = FaceWindow->Width;
         NewFaceWindow.Height = FaceWindow->Height;
         CloseWindow(FaceWindow);
         FaceWindow = NULL;
      }
   }
   else if (address == (APTR)&StopGadget) {
      AbortIO(&voice_io);
      voice_io.message.io_Error = 0;
      mouth_io.voice.message.io_Error = 0;
   }
   /* Since this program changes a flag that intuition expects
    * only the user to change (SELECTED bit), this program has
    * to remove, then change, then add this gadget. Then by
    * passing the address of this gadget to RefreshGadgets(),
    * only the gadgets from here to the start of the list will
    * be refreshed, which minimizes the visible flash that
    * RefreshGadgets() can introduce.
    * If one of the two gadgets (female/male) is hit, toggle
    * the selection of the other gadget (since the gadget hit
    * was toggled by intuition when it was hit).
    */
   else if (address == (APTR)&FemaleGadget) {
      if (FemaleGadget.Flags & SELECTED)
      voice_io.sex = FEMALE;
      else
      voice_io.sex = MALE;
      i = RemoveGadget(ControlWindow, &MaleGadget);
      MaleGadget.Flags ^= SELECTED;
      AddGadget(ControlWindow, &MaleGadget, i);
      RefreshGadgets(&MaleGadget, ControlWindow, NULL);
   }
   else if (address == (APTR)&MaleGadget) {
      if (MaleGadget.Flags & SELECTED)
      voice_io.sex = MALE;
      else
      voice_io.sex = FEMALE;
      i = RemoveGadget(ControlWindow, &FemaleGadget);
      FemaleGadget.Flags ^= SELECTED;
      AddGadget(ControlWindow, &FemaleGadget, i);
      RefreshGadgets(&MaleGadget, ControlWindow, NULL);
   }
   /* Since the program changes the contents of the string
    * gadgets' buffer and it's size, which is something else
    * intuition doesn't expect a program (as opposed to the
    * user) to do. The program must remove, then change, then
    * add this gadget, and then by passing the address of this
    * gadget to RefreshGadgets(), only the gadgets from here
    * to the start of the list will be refreshed, which
    * minimizes the visible flash that RefreshGadgets() can
    * introduce.
    */
   else if (address == (APTR)&TranslateGadget) {
      i = RemoveGadget(ControlWindow, &PhonStrGadget);
      if ((TranslatorError = Translate((APTR)EnglBuffer,
       EnglInfo.NumChars, (APTR)PhonBuffer, PhonInfo.MaxChars))
       != 0) {
#ifdef DEBUG
         printf("Translator won't. (%lx)\n",TranslatorError);
#endif
         /* flash this screen */
         DisplayBeep(ControlWindow->WScreen);
      }
      /* Hey! NumChars includes the terminating NULL. */
      /* This must be done. */
      PhonInfo.NumChars = voice_io.message.io_Length + 1;
      if (PhonInfo.DispPos > voice_io.message.io_Length)
          PhonInfo.DispPos = voice_io.message.io_Length;
      AddGadget(ControlWindow, &PhonStrGadget, i);
      RefreshGadgets(&PhonStrGadget, ControlWindow, NULL);
   }
   else if (address == (APTR)&SpeakGadget) {
      SpeakGadget.Flags ^= GADGDISABLED;
      FaceGadget.Flags ^= GADGDISABLED;
      voice_io.message.io_Length = strlen(PhonBuffer);
      SendIO(&voice_io);
      if (voice_io.mouths == 1) {
         mouth_io.voice.message.io_Error = 0;
         SendIO(&mouth_io);
      }
   }
   else if (address == (APTR)&EnglStrGadget);  /* do nothing */
   else if (address == (APTR)&PhonStrGadget);  /* do nothing */
   else if (address == (APTR)&Props[0]) {
      PropRange = RNGFREQ;
      voice_io.sampfreq = (( (PInfos[0].HorizPot >> 1)
       * PropRange) >> 15) + MINFREQ;
#ifdef DEBUG
      printf("Freq. = %ld\n", voice_io.sampfreq);
#endif
   }
   else if (address == (APTR)&Props[1]) {
      PropRange = RNGRATE;
      voice_io.rate = (((PInfos[1].HorizPot >> 1)
       * PropRange) >> 15) + MINRATE;
#ifdef DEBUG
      printf("Rate  = %ld\n", voice_io.rate);
#endif
   }
   else if (address == (APTR)&Props[2]) {
      PropRange = RNGPITCH;
      voice_io.pitch = (((PInfos[2].HorizPot >> 1)
       * PropRange) >> 15) + MINPITCH;
#ifdef DEBUG
      printf("Pitch = %ld\n", voice_io.pitch);
#endif
   }
   else if (address == (APTR)&Props[3]) {
      PropRange = RNGVOL;
      voice_io.volume = (((PInfos[3].HorizPot >> 1)
       * PropRange) >> 15) + MINVOL;
#ifdef DEBUG
      printf("Vol.  = %ld\n", voice_io.volume);
#endif
   }
#ifdef DEBUG
   else printf("Unhandled gadget up received!\n");
#endif
}

/* This calculates variables used to draw the mouth
 * and eyes, as well as redrawing the face.
 * Proportionality makes it very wierd, but it's
 * wierder if you don't use a GimmeZeroZero window
 * and GZZWidth/GZZHeight.
 */
DrawFace() {
   XMouthCenter =  FaceWindow->GZZWidth >> 1;
   /* set left edge of left eye */
   EyesLeft = FaceWindow->GZZWidth >> 2;
   /* multiplier for mouth width */
   MouthWMult = FaceWindow->GZZWidth >> 6;

   EyesTop = (FaceWindow->GZZHeight >> 2)
    - (FaceWindow->GZZHeight >> 4);
   EyesBottom = EyesTop + (FaceWindow->GZZHeight >> 3) + 1;
   yaw = FaceWindow->GZZHeight - EyesBottom;
   YMouthCenter = ((yaw >> 1) + EyesBottom);
   MouthHMult = yaw >> 5;

   /* Set pen to White */
   SetAPen(FaceWindow->RPort, WHTP);
   RectFill(FaceWindow->RPort, 0, 0,
    FaceWindow->GZZWidth, FaceWindow->GZZHeight);

   /* Set pen to Blue */
   SetAPen(FaceWindow->RPort, BLUP);
   RectFill(FaceWindow->RPort,
    EyesLeft, EyesTop,
    EyesLeft + (FaceWindow->GZZWidth >> 3),
    EyesTop + (FaceWindow->GZZHeight >> 3));
   RectFill(FaceWindow->RPort,
    (FaceWindow->GZZWidth >> 1)
    + (FaceWindow->GZZWidth >> 3),
    EyesTop,
    (FaceWindow->GZZWidth >> 1)
     + (FaceWindow->GZZWidth >> 3) /* two >> 3, not one >> 2 */
     + (FaceWindow->GZZWidth >> 3),/* so eyes are same width */
    EyesTop + (FaceWindow->GZZHeight >> 3));

    SetAPen(FaceWindow->RPort, REDP);  /* Set pen to Red */
    Move(FaceWindow->RPort,
     XMouthCenter - (FaceWindow->GZZWidth >> 3), YMouthCenter);
    Draw(FaceWindow->RPort,
     XMouthCenter + (FaceWindow->GZZWidth >> 3), YMouthCenter);

}

/* Deallocate any memory, and close all of the
 * windows/screens/devices/libraries in reverse order to
 * make things work smoothly. And be sure to check
 * that the open/allocation was successful before
 * closing/deallocating.
 */
MyCleanup()
{
   if (read_port.mp_SigBit != -1)
      FreeSignal(read_port.mp_SigBit);
   if (talk_port.mp_SigBit != -1)
      FreeSignal(talk_port.mp_SigBit);
   if (FaceWindow != NULL)
      CloseWindow(FaceWindow);
   if (ControlWindow != NULL)
      CloseWindow(ControlWindow);
   /* freeimages makes sure image allocation was successful */
   freeimages();
   if (NarratorOpenError == 0)
      CloseDevice(&voice_io);
   if (TranslatorBase != 0)
      CloseLibrary(TranslatorBase);
   if (GfxBase != 0)
      CloseLibrary(GfxBase);
   if (IntuitionBase != 0)
      CloseLibrary(IntuitionBase);
   return(0);
}

/* Allocate chip memory for gadget images, and set the
 * pointers in the corresponding image structures to point
 * to these images. This must be done because the program
 * could be loaded into expansion memory (off the side of
 * the box), which the custom chips cannot access.
 * And images must be in chip ram (that's memory that the
 * custom chips can access, the internal 512K).
 */
InitImages()
{
   /* the images were staticly initialized above main */
   extern USHORT *FemaleIData_chip;
   extern USHORT *MaleIData_chip;
   extern USHORT *HumanIData_chip;
   extern USHORT *RobotIData_chip;
   extern USHORT *FaceIData_chip;
   extern USHORT *StopIData_chip;
   int i;

   /* Allocate them all, stop and return false on failure */
   if ((FemaleIData_chip = (USHORT *)
    AllocMem(sizeof(FemaleIData),MEMF_CHIP)) == 0)
      return(FALSE);
   if ((MaleIData_chip = (USHORT *)
    AllocMem(sizeof(MaleIData),MEMF_CHIP)) == 0)
      return(FALSE);
   if ((HumanIData_chip = (USHORT *)
    AllocMem(sizeof(HumanIData),MEMF_CHIP)) == 0)
      return(FALSE);
   if ((RobotIData_chip = (USHORT *)
    AllocMem(sizeof(RobotIData),MEMF_CHIP)) == 0)
      return(FALSE);
   if ((FaceIData_chip = (USHORT *)
    AllocMem(sizeof(FaceIData),MEMF_CHIP)) == 0)
      return(FALSE);
   if ((StopIData_chip = (USHORT *)
    AllocMem(sizeof(StopIData),MEMF_CHIP)) == 0)
      return(FALSE);

   for (i=0; i<20; i++)
      FemaleIData_chip[i] = FemaleIData[i];
   for (i=0; i<20; i++)
      MaleIData_chip[i] = MaleIData[i];
   for (i=0; i<60; i++)
      HumanIData_chip[i] = HumanIData[i];
   for (i=0; i<60; i++)
      RobotIData_chip[i] = RobotIData[i];
   for (i=0; i<30; i++)
      FaceIData_chip[i] = FaceIData[i];
   for (i=0; i<30; i++)
      StopIData_chip[i] = StopIData[i];

   FemaleImage.ImageData = FemaleIData_chip;
   MaleImage.ImageData = MaleIData_chip;
   HumanImage.ImageData = HumanIData_chip;
   RobotImage.ImageData = RobotIData_chip;
   FaceImage.ImageData = FaceIData_chip;
   StopImage.ImageData = StopIData_chip;

   return(TRUE);
}

/* Deallocate the memory that was used for images,
 * See initimages for more details.
 */
freeimages()
{
   /* the images were staticly initialized above main */
   extern USHORT *FemaleIData_chip;
   extern USHORT *MaleIData_chip;
   extern USHORT *HumanIData_chip;
   extern USHORT *RobotIData_chip;
   extern USHORT *FaceIData_chip;
   extern USHORT *StopIData_chip;

   /* Deallocate only if the pointer is really there. */
   if (RobotIData_chip != 0)
      FreeMem(RobotIData_chip, (sizeof(RobotIData),MEMF_CHIP));
   if (HumanIData_chip != 0)
      FreeMem(HumanIData_chip, (sizeof(HumanIData),MEMF_CHIP));
   if (MaleIData_chip != 0)
      FreeMem(MaleIData_chip, (sizeof(MaleIData),MEMF_CHIP));
   if (FemaleIData_chip != 0)
      FreeMem(FemaleIData_chip, (sizeof(FemaleIData),
       MEMF_CHIP));
   if (FaceIData_chip != 0)
      FreeMem(FaceIData_chip, (sizeof(FaceIData),MEMF_CHIP));
   if (StopIData_chip != 0)
      FreeMem(StopIData_chip, (sizeof(StopIData),MEMF_CHIP));
}
