/****** CALib.headers/WBCliArgs ***********************************************

    NAME
        WBCLIArgs -- Header for the WBCLIArgs() routine.

    VERSION
        1.00    04 Feb 1992 - Inception
        1.01    10 Mar 1992 - Added WC_MToolTypes

    AUTHOR
        Fred Mitchell, Products Assurance, Commodore-Amiga, Inc.


    SEE ALSO
        CALib.library/WBCLIArgs

******************************************************************************/

#include "LinkTools.h"

// Tags
#define WC_Tags             0x31000
#define WC_Startup          (WC_Tags+1)
#define WC_DiskObject       (WC_Tags+2)
#define WC_DiskObjectName   (WC_Tags+3)
#define WC_TrustStartup     (WC_Tags+4)
#define WC_Template         (WC_Tags+5)
#define WC_Help             (WC_Tags+6)
#define WC_Args             (WC_Tags+7)
#define WC_MToolTypes       (WC_Tags+8)
#define WC_Project          (WC_Tags+9)
#define WC_GUI              (WC_Tags+10)

// Internal Gadget/Field-type Flags

// Field type
#define WCG_FieldTypes  0x00FFF
#define WCG_String      0x00001
#define WCG_Fixed       0x00002
#define WCG_Boolean     0x00004
#define WCG_Toggle      0x00008
#define WCG_Numeric     0x00010
#define WCG_Hex         0x00020
#define WCG_Float       0x00040  // IEEE double

// Modifiers
#define WCG_Modifiers   0xFF000
#define WCG_Multiple    0x01000
#define WCG_Keyword     0x02000
#define WCG_Required    0x04000

#define WCG_maok        0x80000000   // state flag

// Private structures

struct WBCGui {
    struct Link list;
    ULONG flags;
    ULONG *parr;    // pointer to field in array
    ULONG index;    // index into array (a bit redundant - may be eliminated)
    char *name;     // name of template
    char *alt_name; // alternate name of template
    };

struct WBCli {
    struct RDArgs *rda;
    struct Remember *key;

    struct Library *Intuition, *Utility, *Icon, *GadTools;

    // dynamic GUI creation
    struct Link gui;    // GUI list
    struct Window *gwin;
    };
