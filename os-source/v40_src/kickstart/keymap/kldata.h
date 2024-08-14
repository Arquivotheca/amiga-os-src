#include	"exec/libraries.h"

#include	"keymap.h"

struct KeyMapLibrary {
    struct Library kl_L;
    ULONG  kl_SegList;
    struct KeyMap *kl_KMDefault;
    struct KeyMapResource kl_R;
    struct KeyMapNode kl_USA;
    struct KeyMapNode kl_USA1;
};
