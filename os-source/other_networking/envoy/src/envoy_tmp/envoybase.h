
#include <exec/types.h>
#include <exec/libraries.h>
#include <graphics/gfxbase.h>
#include <dos/dos.h>
#include <sana2.h>
#include <envoy/errors.h>
#include <devices/timer.h>
#include <exec/io.h>
#include <utility/tagitem.h>
#include <dos.h>

struct EnvoyBase
{
    struct Library       LibNode;
    struct Library      *eb_DOSBase;
    struct Library      *eb_UtilityBase;
    struct Library      *eb_SysBase;
    struct Library      *eb_NIPCBase;
    struct Library      *eb_GadToolsBase;
    struct Library      *eb_IntuitionBase;
    struct GfxBase      *eb_GfxBase;
    struct Library      *eb_LayersBase;
    ULONG                eb_SegList;
    struct TextFont     *eb_TopazFont;

};

#define ENVOYBASE ((struct EnvoyBase *)getreg(REG_A6))

#define eb        ((struct EnvoyBase *)getreg(REG_A6))
#define SysBase             (ENVOYBASE->eb_SysBase)
#define DOSBase             (ENVOYBASE->eb_DOSBase)
#define GfxBase             (ENVOYBASE->eb_GfxBase)
#define GadToolsBase        (ENVOYBASE->eb_GadToolsBase)
#define NIPCBase            (ENVOYBASE->eb_NIPCBase)
#define UtilityBase         (ENVOYBASE->eb_UtilityBase)
#define IntuitionBase       (ENVOYBASE->eb_IntuitionBase)
#define LayersBase	    (ENVOYBASE->eb_LayersBase)
