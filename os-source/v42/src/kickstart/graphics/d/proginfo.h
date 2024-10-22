/******************************************************************************
*
*   $Id: proginfo.h,v 42.0 93/06/16 11:18:33 chrisg Exp $
*
*******************************************************************************/

#include <exec/types.h>
#include "/displayinfo.h"
#include "/vp_internal.h"

#define KILLEHB 0x200
#define PLFD_PRIO 0x24
#define BPLCON2 KILLEHB
#define VGAPLFD_PRIO 0x24
#define VGABPLCON2 KILLEHB

extern void InitMVP(struct View *c, struct ViewPort *vp, struct ViewPortExtra **vpe, struct ProgInfo *pinfo);
extern void CleanUpMVP(DRIVER_PARAMS);
extern void CalcDispWindow(DRIVER_PARAMS);
extern void CalcADispWindow(DRIVER_PARAMS);
extern void CalcDataFetch(DRIVER_PARAMS);
extern void CalcModulos(DRIVER_PARAMS);
extern void CalcScroll(DRIVER_PARAMS);
extern void CalcDPFScrollMods(DRIVER_PARAMS);
extern void MakeAABplcon(DRIVER_PARAMS);
extern void BuildAAColours(DRIVER_PARAMS);
extern void BuildNmlAACopList(DRIVER_PARAMS);
extern void MakeECSBplcon(DRIVER_PARAMS);
extern void BuildColours(DRIVER_PARAMS);
extern void BuildNmlACopList(DRIVER_PARAMS);
extern void Fudge(DRIVER_PARAMS);

extern void DefaultMoveSprite(void);
extern void ChangeSprite(void);
extern void scrollvport(void);
extern void GfxNewVPE(void);
extern void DefaultPokeColors(void);

#define BuildNmlECSCopList BuildNmlAACopList

/* Pointers to display drivers */

void (*MakeItAA[])() = 
{
	InitMVP,
	CalcDispWindow,
	CalcDataFetch,
	CalcScroll,
	Fudge,
	CalcModulos,
	MakeAABplcon,
	BuildAAColours,
	BuildNmlAACopList,
	CleanUpMVP,
	NULL,
};

void (*MakeItDPFAA[])() = 
{
	InitMVP,
	CalcDispWindow,
	CalcDataFetch,
	CalcDPFScrollMods,
	MakeAABplcon,
	BuildAAColours,
	BuildNmlAACopList,
	CleanUpMVP,
	NULL,
};

void (*MakeItECS[])() = 
{
	InitMVP,
	CalcDispWindow,
	CalcDataFetch,
	CalcScroll,
	Fudge,
	CalcModulos,
	MakeECSBplcon,
	BuildColours,
	BuildNmlECSCopList,
	CleanUpMVP,
	NULL,
};

void (*MakeItDPFECS[])() = 
{
	InitMVP,
	CalcDispWindow,
	CalcDataFetch,
	CalcDPFScrollMods,
	MakeECSBplcon,
	BuildColours,
	BuildNmlECSCopList,
	CleanUpMVP,
	NULL,
};

void (*MakeItA[])() = 
{
	InitMVP,
	CalcADispWindow,
	CalcDataFetch,
	CalcScroll,
	Fudge,
	CalcModulos,
	MakeECSBplcon,
	BuildColours,
	BuildNmlACopList,
	CleanUpMVP,
	NULL,
};

void (*MakeItDPFA[])() = 
{
	InitMVP,
	CalcADispWindow,
	CalcDataFetch,
	CalcDPFScrollMods,
	MakeECSBplcon,
	BuildColours,
	BuildNmlACopList,
	CleanUpMVP,
	NULL,
};

void (*VecAA[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItAA,
	DefaultPokeColors,
};

void (*VecDPFAA[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItDPFAA,
	DefaultPokeColors,
};

void (*VecECS[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItECS,
	DefaultPokeColors,
};

void (*VecDPFECS[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItDPFECS,
	DefaultPokeColors,
};

void (*VecA[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItA,
	DefaultPokeColors,
};

void (*VecDPFA[])() =
{
	DefaultMoveSprite,
	ChangeSprite,
	scrollvport,
	MakeItDPFA,
	DefaultPokeColors,
};


#define AAVECS 2
#define ECSVECS 2

void *VecLists[] =
{
    VecAA,
    VecDPFAA,
    VecECS,
    VecDPFECS,
    VecA,
    VecDPFA,
};

/* compressed database with data */

/* All SqueezedProgInfo assume AA machine - db_startup() can massage the entries
 * as necessary.
 */

struct ProgInfo ProgData[] =
{
    {	/* LORES ProgInfo */					/* 0 */
	0,			/* bplcon0 */
	BPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* HIRES ProgInfo */					/* 1 */
	0x8000,			/* bplcon0 */
	BPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* SHIRES ProgInfo */					/* 2 */
	0x0040,			/* bplcon0 */
	BPLCON2,
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* LORES HAM ProgInfo */				/* 3 */
	0x800,			/* bplcon0 */
	BPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_HAM,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* HIRES HAM ProgInfo */				/* 4 */
	0x8800,			/* bplcon0 */
	BPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5 | PROGINFO_HAM,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* SHIRES HAM ProgInfo */				/* 5 */
	0x0840,			/* bplcon0 */
	BPLCON2,
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_HAM |
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* LORES EHB ProgInfo */				/* 6 */
	0,			/* bplcon0 */
	PLFD_PRIO,		/* No KillEHB */
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* LORES DPF ProgInfo */				/* 7 */
	0x400,			/* bplcon0 */
	BPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	1,			/* MakeItType */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* HIRES DPF ProgInfo */				/* 8 */
	0x8400,			/* bplcon0 */
	BPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* SHIRES DPF ProgInfo */				/* 9 */
	0x0440,			/* bplcon0 */
	BPLCON2,
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* LORES DPF2 ProgInfo */				/* 10 */
	0x400,			/* bplcon0 */
	BPLCON2 | 0x40,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	1,			/* MakeItType */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* HIRES DPF2 ProgInfo */				/* 11 */
	0x8400,			/* bplcon0 */
	BPLCON2 | 0x40,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* SHIRES DPF2 ProgInfo */				/* 12 */
	0x0440,			/* bplcon0 */
	BPLCON2 | 0x40,
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* PRODUCTIVITY ProgInfo */				/* 13 */
	0x0041,			/* bplcon0 */
	VGABPLCON2,			/* bplcon2 */
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* PRODUCTIVITY HAM ProgInfo */				/* 14 */
	0x0841,			/* bplcon0 */
	VGABPLCON2,			/* bplcon2 */
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5 | PROGINFO_HAM,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* VGAEXTRALORES ProgInfo */				/* 15 */
	1,			/* bplcon0 */
	VGABPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* VGALORES ProgInfo */					/* 16 */
	0x8001,			/* bplcon0 */
	VGABPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT3 |
	PROGINFO_VARBEAM,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* VGAEXTRALORESHAM ProgInfo */				/* 17 */
	0x801,			/* bplcon0 */
	VGABPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM | PROGINFO_HAM,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* VGALORESHAM ProgInfo */				/* 18 */
	0x8801,			/* bplcon0 */
	VGABPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT3 |
	PROGINFO_VARBEAM | PROGINFO_HAM,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* VGAEXTRALORESEHB ProgInfo */				/* 19 */
	1,			/* bplcon0 */
	VGAPLFD_PRIO,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* VGAEXTRALORESDPF ProgInfo */				/* 20 */
	0x401,			/* bplcon0 */
	VGABPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM,	/* Flags - may be massaged */
	1,			/* MakeItType */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* VGALORESDPF ProgInfo */				/* 21 */
	0x8401,			/* bplcon0 */
	VGABPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT3 |
	PROGINFO_VARBEAM,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* PRODUCTIVITYDPF ProgInfo */				/* 22 */
	0x0441,			/* bplcon0 */
	VGABPLCON2,			/* bplcon2 */
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* VGAEXTRALORESDPF2 ProgInfo */			/* 23 */
	0x401,			/* bplcon0 */
	VGABPLCON2 | 0x40,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM,	/* Flags - may be massaged */
	1,			/* MakeItType */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* VGALORESDPF2 ProgInfo */				/* 24 */
	0x8401,			/* bplcon0 */
	VGABPLCON2 | 0x40,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT3 |
	PROGINFO_VARBEAM,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* PRODUCTIVITYDPF2 ProgInfo */				/* 25 */
	0x0441,			/* bplcon0 */
	VGABPLCON2 | 0x40,			/* bplcon2 */
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_VARBEAM | 
	PROGINFO_SHIFT3 | PROGINFO_SHIFT5,	/* Flags - may be massaged */
	1,			/* makeittype */
	4,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* HIRES EHB ProgInfo */				/* 26 */
	0x8000,			/* bplcon0 */
	PLFD_PRIO,		/* No KillEHB */
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* SHIRES EHB ProgInfo */				/* 27 */
	0x0040,			/* bplcon0 */
	PLFD_PRIO,		/* No KillEHB */
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* VGALORESEHB ProgInfo */				/* 28 */
	0x8001,			/* bplcon0 */
	VGAPLFD_PRIO,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT3| 
	PROGINFO_VARBEAM,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
    {	/* VGAEHB ProgInfo */					/* 29 */
	0x0041,			/* bplcon0 */
	VGAPLFD_PRIO,
	2,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5 | PROGINFO_SHIFT3 |
	PROGINFO_VARBEAM,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	0,			/* To DIWResn */
	0,
    },
    {	/* LORES SDblProgInfo */				/* 30 */
	0,			/* bplcon0 */
	BPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SCANDBL,	/* Flags - may be massaged */
	0,			/* MakeItType */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* LORES HAM SDbl ProgInfo */				/* 31 */
	0x800,			/* bplcon0 */
	BPLCON2,
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_HAM |
	PROGINFO_SCANDBL,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* LORES EHB ProgInfo */				/* 32 */
	0,			/* bplcon0 */
	PLFD_PRIO,		/* No KillEHB */
	0,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SCANDBL,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	2,			/* To DIWResn */
	2,
    },
    {	/* HIRES HAM SDbl ProgInfo */				/* 33 */
	0x8800,			/* bplcon0 */
	BPLCON2,
	1,			/* ToViewX */
	0,			/* pad */
	PROGINFO_NATIVE | PROGINFO_SHIFT5 | PROGINFO_HAM |
	PROGINFO_SCANDBL,	/* Flags - may be massaged */
	0,			/* makeittype */
	6,			/* ScrollVPCount */
	0xfff8,			/* DDFSTRTMask */
	0xfff8,			/* DDFSTOPMask */
	1,			/* To DIWResn */
	1,
    },
};
