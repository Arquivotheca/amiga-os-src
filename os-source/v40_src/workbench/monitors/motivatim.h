/******************************************************************************
*
*	$Id: motivatim.h,v 40.0 93/02/09 18:27:55 spence Exp $
*
******************************************************************************/

#include <graphics/monitor.h>
#include <graphics/modeid.h>
#include "monitorstuff.h"

#define MOTIVATIM_MONITOR_ID	0xc1000
#define MOTIVATIM_KEY		0xc9020
#define MOTIVATIMHAM_KEY	0xc9820
#define MOTIVATIMEHB_KEY	0xc90a0
#define MOTIVATIMDPF_KEY	0xc9420
#define MOTIVATIMDPF2_KEY	0xc9460

#define DDFSTRT 0x20

#define MONITOR_NUM   0xc
#define MONITOR_NAME  "Motivatim.monitor"
#define ICON_NAME "PROGDIR:Motivatim"
#define BEAMCON0 0x1b88
#define MSFLAGS (REQUEST_SPECIAL | (GfxBase->ChipRevBits0 & GFXF_AA_LISA ? MSF_DOUBLE_SPRITES : 0) | ((GfxBase->DisplayFlags & PAL)? REQUEST_PAL : REQUEST_NTSC))
#define TOTALCLKS	0xc1
#define TOTALROWS	0x486
#define MINIMUMROW	0x11
/* Bad AA definitions */
#define AATOTCLKS	TOTALCLKS
#define AATOTROWS	TOTALROWS
#define AAMINROW	MINIMUMROW

#define TOTROWS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTROWS : TOTALROWS)
#define TOTCLKS ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AATOTCLKS : TOTALCLKS)
#define MINROW  ((GfxBase->Bugs & BUG_ALICE_LHS_PROG) ? AAMINROW  : MINIMUMROW)

#define PREFERRED_MODEID MOTIVATIM_KEY
#define COMPAT MCOMPAT_MIXED

#define VRESNX 67
#define VRESNY 94

#define NOM_WIDTH 1280
#define NOM_HEIGHT 1024

#define PIX_RES_X 67
#define PIX_RES_Y 94
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
