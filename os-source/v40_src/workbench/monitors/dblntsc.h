/******************************************************************************
*
*	$Id: dblntsc.h,v 40.1 93/02/16 18:24:12 spence Exp $
*
******************************************************************************/

#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MONITOR_NUM   9
#define MONITOR_NAME  "DblNTSC.monitor"
#define ICON_NAME "PROGDIR:DblNTSC"
#define BEAMCON0 0x1b88
#define MSFLAGS (REQUEST_SPECIAL | (GfxBase->ChipRevBits0 & GFXF_AA_LISA ? MSF_DOUBLE_SPRITES : 0) | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define TOTALCLKS	0x79
#define TOTALROWS	0x1ec
#define MINIMUMROW	0x19
/* Bad AA definitions */
#define AATOTCLKS	(TOTALCLKS + 8)
#define AATOTROWS	0x1dd
#define AAMINROW	0x17

#define TOTROWS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTROWS : TOTALROWS)
#define TOTCLKS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTCLKS : TOTALCLKS)
#define MINROW  ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AAMINROW  : MINIMUMROW)

#define PREFERRED_MODEID DBLNTSCHIRESFF_KEY
#define COMPAT MCOMPAT_MIXED

#define VRESNX 88
#define VRESNY 26

#define NOM_WIDTH 640
#define NOM_HEIGHT 400

#define PIX_RES_X 22
#define PIX_RES_Y 26
#define SPRITE_RES_X (PIX_RES_X << 1)
#define SPRITE_RES_Y PIX_RES_Y

#define MOUSETICKS_X 22
#define MOUSETICKS_Y 26

#define MIN_DIPF_FLAGS	(DIPF_IS_ECS | DIPF_IS_PROGBEAM | DIPF_IS_SPRITES | DIPF_IS_DRAGGABLE | DIPF_IS_BEAMSYNC | DIPF_IS_DBUFFER | DIPF_IS_PROGBEAM)
#define WB_35 TRUE
#define WB_70 TRUE
#define WB_140 FALSE
#define LOW_X_RESN RESN_140
#define HIGH_X_RESN (IsECS ? RESN_35 : RESN_70)
#define LOW_Y_RESN (IsAA ? SDBLED : NON_LACE)
