/******************************************************************************
*
*	$Id: motivator.h,v 40.0 93/02/09 18:27:45 spence Exp $
*
******************************************************************************/

#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MOTIVATOR_MONITOR_ID	0xb1000
#define MOTIVATOR_KEY		0xb9020
#define MOTIVATORHAM_KEY	0xb9820
#define MOTIVATOREHB_KEY	0xb90a0
#define MOTIVATORDPF_KEY	0xb9420
#define MOTIVATORDPF2_KEY	0xb9460

#define DDFSTRT 0x20

#define MONITOR_NUM   0xb
#define MONITOR_NAME  "Motivator.monitor"
#define ICON_NAME "PROGDIR:Motivator"
#define BEAMCON0 0x1b88
#define MSFLAGS (REQUEST_SPECIAL | (GfxBase->ChipRevBits0 & GFXF_AA_LISA ? MSF_DOUBLE_SPRITES : 0) | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define TOTALCLKS	0xa1
#define TOTALROWS	0x313
#define MINIMUMROW	0x13
/* Bad AA definitions */
#define AATOTCLKS	TOTALCLKS
#define AATOTROWS	TOTALROWS
#define AAMINROW	0x13

#define TOTROWS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTROWS : TOTALROWS)
#define TOTCLKS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTCLKS : TOTALCLKS)
#define MINROW  ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AAMINROW  : MINIMUMROW)

#define PREFERRED_MODEID MOTIVATOR_KEY
#define COMPAT MCOMPAT_MIXED

#define VRESNX 11
#define VRESNY 11

#define NOM_WIDTH 1024
#define NOM_HEIGHT 768

#define PIX_RES_X 11
#define PIX_RES_Y 11
#define SPRITE_RES_X (PIX_RES_X << 1)
#define SPRITE_RES_Y PIX_RES_Y

#define MIN_DIPF_FLAGS	(DIPF_IS_ECS | DIPF_IS_PROGBEAM | DIPF_IS_SPRITES | DIPF_IS_DRAGGABLE | DIPF_IS_BEAMSYNC | DIPF_IS_DBUFFER | DIPF_IS_PROGBEAM)
#define WB_35 TRUE
#define WB_70 FALSE
#define WB_140 FALSE
#define LOW_X_RESN RESN_35
#define HIGH_X_RESN RESN_35
#define LOW_Y_RESN NON_LACE

#ifdef FETCH1
#undef FETCH1
#endif
#define FETCH1 0x28
