#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MONITOR_NUM   4
#define MONITOR_NAME  "a2024.monitor"
#define ICON_NAME "PROGDIR:A2024"
#define MSFLAGS (REQUEST_A2024 | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define BEAMCON0 ((GfxBase->DisplayFlags & PAL) ? STANDARD_PAL_BEAMCON : STANDARD_NTSC_BEAMCON)
#define TOTROWS ((GfxBase->DisplayFlags & PAL) ? STANDARD_PAL_ROWS : STANDARD_NTSC_ROWS)
#define TOTCLKS STANDARD_COLORCLOCKS
#define MINROW ((GfxBase->DisplayFlags & PAL) ? MIN_PAL_ROW : MIN_NTSC_ROW)

#define PREFERRED_MODEID (A2024_MONITOR_ID | LORES_KEY)
#define COMPAT MCOMPAT_NOBODY

#define VRESNX 14
#define VRESNY ((GfxBase->DisplayFlags & PAL) ? 11 : 13)

#define NOM_WIDTH (IsBigAgnus ? 1024 : 1008)
#define NOM_HEIGHT ((GfxBase->DisplayFlags & PAL) ? 1024 : 800)

#define PIX_RES_X VRESNX
#define PIX_RES_Y VRESNY
#define SPRITE_RES_X (VRESNX << 1)
#define SPRITE_RES_Y (VRESNY << 1)

#define MOUSETICKS_X 22
#define MOUSETICKS_Y ((GfxBase->DisplayFlags & PAL) ? 22 : 26)

#define MIN_DIPF_FLAGS (DIPF_IS_DRAGGABLE | DIPF_IS_SPRITES  | DIPF_IS_WB | DIPF_IS_PANELLED)
#define WB_35 FALSE
#define WB_70 TRUE
#define WB_140 TRUE
#define LOW_X_RESN RESN_140
#define HIGH_X_RESN RESN_70
#define LOW_Y_RESN NON_LACE

#undef FETCH1
#undef MIN_DENISE
#undef MAX_DENISE
#undef HBSTRT
#undef MAX_WIDTH
#undef MAX_HEIGHT
#undef VIDEO_WIDTH
#undef VIDEO_HEIGHT
#undef VPMINX
#undef VPMINY
#undef VPRANGEX
#undef VPRANGEY

#define FETCH1 0x1b
#define MIN_DENISE STANDARD_DENISE_MIN
#define MAX_DENISE 465
#define HBSTRT 0x8
#define MAX_WIDTH NOM_WIDTH
#define MAX_HEIGHT NOM_HEIGHT
#define VIDEO_WIDTH MAX_WIDTH
#define VIDEO_HEIGHT MAX_HEIGHT
#define VPMINX 0x81
#define VPMINY 0x2c
#define VPRANGEX 0
#define VPRANGEY 0

#undef LASTID
#define LASTID (IDNORM + 1)
