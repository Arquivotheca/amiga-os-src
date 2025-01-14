#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MONITOR_NUM   7
#define MONITOR_NAME  "Euro36.monitor"
#define ICON_NAME "PROGDIR:Euro36"
#define BEAMCON0 ((GfxBase->Bugs & BUG_ALICE_LHS) ? 0x1b80 : 0x1b88)
#define MSFLAGS (REQUEST_SPECIAL | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define TOTALCLKS	STANDARD_COLORCLOCKS
#define TOTALROWS	0xd8
#define MINIMUMROW	0x10

#define TOTROWS TOTALROWS
#define TOTCLKS TOTALCLKS
#define MINROW  MINIMUMROW

#define PREFERRED_MODEID (EURO36_MONITOR_ID | HIRES_KEY)
#define COMPAT MCOMPAT_MIXED

#define VRESNX 44
#define VRESNY 44

#define NOM_WIDTH 1280
#define NOM_HEIGHT 200

#define PIX_RES_X 11
#define PIX_RES_Y 44
#define SPRITE_RES_X 44
#define SPRITE_RES_Y 44

#define MOUSETICKS_X 22
#define MOUSETICKS_Y 22

#define MIN_DIPF_FLAGS	(DIPF_IS_ECS | DIPF_IS_PROGBEAM | DIPF_IS_SPRITES | DIPF_IS_DRAGGABLE | DIPF_IS_BEAMSYNC | DIPF_IS_DBUFFER | DIPF_IS_PROGBEAM)
#define WB_35 TRUE
#define WB_70 TRUE
#define WB_140 TRUE
#define LOW_X_RESN RESN_140
#define HIGH_X_RESN (IsECS ? RESN_35 : RESN_70)
#define LOW_Y_RESN NON_LACE

/* Euro36 is kind of an oddball, because we have to make it programmed only for
 * vertical resolutions.
 *
 * This means that the horizontal sizes need to be the same as NTSC/PAL.
 */

#undef FETCH1
#undef MIN_DENISE
#undef HBSTRT
#undef MAX_WIDTH
#undef VIDEO_WIDTH
#undef VPMINX

#define FETCH1 0x1b
#define MIN_DENISE STANDARD_DENISE_MIN
#define HBSTRT 0x8
#define MAX_WIDTH (IsAA ? 1448 : 1440)
#define VIDEO_WIDTH (IsAA ? 1472 : 1440)
#define VPMINX MIN_DENISE
