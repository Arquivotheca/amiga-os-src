/****** mwhead/MenuTools ******************************************************

    Mitchell/Ware Systems, Inc          Version 2.00            19 Oct 1990

                Structs for MenuTools & HyperMenuTools

                * A Control Packet for menus.

*****************************************************************************/

typedef struct  MenuPac     {
    struct Menu menu;               // menu -- used to be a pointer-to, but did it this
                                    //         way to allow treating the MenuPac as an
                                    //          actual menu.

    struct MenuPac *next, *prev;    // next and prev one in chain
    struct Remember *key;           // key for this menu
    short length;                   // length of this name in pixels
    short sep;                      // offset from last entry in pixels

    /* The following are only valid for the FIRST MenuPac in chain
    */
    short width,            /* width of font to be used in menu */
          height;           /* height to make select area in menus and sub-menus */
    short FrontPen,
          BackPen;
    ULONG DrawMode;
    char *helpdir;          /* pointer to the help directory pathname */
    void (*helpfunct)();    /* pointer to function to be called for help */
    } MENUPAC, MenuPac;

typedef struct MenuItemExt {
    struct MenuItem mi;
    BOOL (*mfunct)();               /* function to call if this item was selected (if non-NULL) */
    char *help;                     /* name of help file for this function */
    void (*helpfunct)();            // Help function to call, if non-NULL
    ULONG itemdata;                 /* item data field */
    } MenuItemExt;

typedef struct HMTFunct     {   // Hyper-Menu Function array
    char *fname;                    // Function name as referenced via ascii file
                                    //  or NULL to signify end-of-array
    void *(*funct)();               // Function to call (or NULL if NIY)

    char *help;                     // Name of helpfile
    void (*helpfunct)();            // Help Function, or NULL if no help is available
    } HMTFunct;
