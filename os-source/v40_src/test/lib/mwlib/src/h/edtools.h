/****** mwhead/EdTools ********************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

    Structs for EdTools

    RastPort Line Editor

*****************************************************************************/


typedef enum EdError {
    ed_ok,          /* no error */
    ed_hidden,      /* cursor is hidden from view */
    ed_overflow,    /* buffer overflow (too many characters typed) */
    ed_nonsequiter, /* an input event was received that has no revelance here */
    ed_raw,         /* a raw code was received that needs ext. processing */
    ed_vanilla,     /* a VANILLAKEY code was rec'd that need ext. processing */
    ed_fatal        /* a fatal error was generated (out of memory, perhaps?) */
    } EdError;


typedef struct RPLineEd {
    struct Window *win;             /* window to process */
    short startx, starty;           /* where string will be printed */
    short chpos, xpos;              /* character & pixel position of cursor (xpos relative to beginning) */
    short curheight, curbase;       /* height & baseline of cursor */
    short spacing;                  /* on CR - extra pixel spacing to add */
    struct TextFont *font;          /* font to use, or NULL */

    ULONG chClass, chCode, chQualifier;
    short chX, chY;

    void (*exfunct)(), (*helpfunct)();  /* ex called after every event */

    EdError error;                  /* error code */

    USHORT fore, back, mode;        /* draw modes and colors */

    short topm, botm, leftm, rightm; /* margin settings */

    UBYTE buf[256];                 /* edit buffer */
    } RPLineEd;

