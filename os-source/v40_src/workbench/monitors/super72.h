#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MONITOR_NUM   8
#define MONITOR_NAME  "Super72.monitor"
#define ICON_NAME "PROGDIR:Super72"
#define BEAMCON0 0x1b88
#define MSFLAGS (REQUEST_SPECIAL | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define TOTALCLKS	0x91
#define TOTALROWS	0x156
#define MINIMUMROW	0x1c
/* Bad AA definitions */
#define AATOTCLKS	(TOTALCLKS + 8)
#define AATOTROWS	0x147
#define AAMINROW	0x15

#define TOTROWS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTROWS : TOTALROWS)
#define TOTCLKS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTCLKS : TOTALCLKS)
#define MINROW  ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AAMINROW  : MINIMUMROW)

#define PREFERRED_MODEID (SUPER72_MONITOR_ID | SUPERLACE_KEY)
#define COMPAT MCOMPAT_MIXED

#define VRESNX 68
#define VRESNY 40

#define NOM_WIDTH 800
#define NOM_HEIGHT 300

#define PIX_RES_X 17
#define PIX_RES_Y 40
#define SPRITE_RES_X (PIX_RES_X << 1)
#define SPRITE_RES_Y PIX_RES_Y

#define MOUSETICKS_X 17
#define MOUSETICKS_Y 20

#define MIN_DIPF_FLAGS	(DIPF_IS_ECS | DIPF_IS_PROGBEAM | DIPF_IS_SPRITES | DIPF_IS_DRAGGABLE | DIPF_IS_BEAMSYNC | DIPF_IS_DBUFFER | DIPF_IS_PROGBEAM)
#define WB_35 TRUE
#define WB_70 TRUE
#define WB_140 FALSE
#define LOW_X_RESN RESN_140
#define HIGH_X_RESN (IsECS ? RESN_35 : RESN_70)
#define LOW_Y_RESN (IsAA ? SDBLED : NON_LACE)
