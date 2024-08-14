/******************************************************************************
*
*   $Id: init.h,v 42.0 93/06/16 11:18:27 chrisg Exp $
*
*   $Locker:  $
*
*******************************************************************************/

#include <exec/types.h>
#include "/view.h"
#include "/displayinfo.h"
#include "/displayinfo_internal.h"
#include "/vp_internal.h"


/* compressed database with data */

#define RAWVEC_SKIP 2
/* due to my magical squeezing, there is no data for the VecInfo stuff. */

#define RAWDIMS_SKIP 8

struct SqueezedDimensionInfo DimsData[] =
{
    {					/* NTSC Lores */
	5,	
	DIMS_RANGE_LORES,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x3807, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x390F, 0x2C42 },
    },
    {					/* NTSC Hires */
	4,	
	DIMS_RANGE_HIRES,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x3807, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x390F, 0x2C42 },
    },
    {					/* NTSC SHires */
	2,	
	DIMS_RANGE_SHIRES,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x37AF, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x37AF, 0x2C42 },
    },
    {					/* NTSC HAM */
	6,	
	DIMS_RANGE_LORES,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x3807, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x390F, 0x2C42 },
    },
    {					/* PAL Lores */
	5,	
	DIMS_RANGE_LORES,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x3807, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x390F, 0x2E0F },
    },
    {					/* PAL Hires */		/* 5 */
	4,	
	DIMS_RANGE_HIRES,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x3807, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x390F, 0x2E0F },
    },
    {					/* PAL SHires */
	2,	
	DIMS_RANGE_SHIRES,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x37AF, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x37AF, 0x2E0F },
    },
    {					/* PAL HAM */
	6,	
	DIMS_RANGE_LORES,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x3807, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x390F, 0x2E0F },
    },
    {					/* NTSC EHB */
	6,	
	DIMS_RANGE_LORES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x3807, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x390F, 0x2C42 },
    },
    {					/* PAL EHB */
	6,	
	DIMS_RANGE_LORES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x3807, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x390F, 0x2E0F },
    },
    {					/* NTSC HIRES EHB */	/* 10 */
	6,
	DIMS_RANGE_HIRES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x3807, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x390F, 0x2C42 },
    },
    {					/* NTSC SHIRES EHB */
	6,	
	DIMS_RANGE_SHIRES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x289F },
	{ 0xF9D0, 0xFB54 ,0x37AF, 0x2C42 },
	{ 0xF9D0, 0xFB54 ,0x37AF, 0x2C42 },
    },
    {					/* PAL HIRES EHB */
	6,	
	DIMS_RANGE_HIRES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x3807, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x390F, 0x2E0F },
    },
    {					/* PAL SHIRES EHB */
	6,	
	DIMS_RANGE_SHIRES | DIMS_DEPTH_HW6,
	{ 0x0000, 0x0000 ,0x36FF, 0x2BFF },
	{ 0xF9D0, 0xFD6C, 0x37AF, 0x2E0F },
	{ 0xF9D0, 0xFD6C, 0x37AF, 0x2E0F },
    },
};

#define RAWDISP_SKIP 4

struct SqueezedDisplayInfo DispData[] =
{
    {	/* ntsc.lores */				/* 0 */
	DIPF_IS_WB | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DBUFFER |
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 44, 52 },
	140,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | EXTENDED_MODE,
    },
    {	/* ntsc.hires */
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_WB | 
	DIPF_IS_DBUFFER |
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 22, 52 },
	70,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | HIRES | EXTENDED_MODE,
    },
    {	/* ntsc.super */
	DIPF_IS_ECS | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_WB | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 11, 52 },
	35,
	64,
	{ 22, 52 },
	DEFAULT_MONITOR_ID | SUPERHIRES | EXTENDED_MODE,
    },
    {	/* pal.lores */
	DIPF_IS_WB | 
	DIPF_IS_PAL | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DBUFFER |
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 44, 44 },
	140,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | EXTENDED_MODE,
    },
    {	/* pal.hires */
	DIPF_IS_PAL | 
	DIPF_IS_SPRITES | 
	DIPF_IS_DBUFFER |
	DIPF_IS_GENLOCK | 
	DIPF_IS_WB | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 22, 44 },
	70,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | HIRES | EXTENDED_MODE,
    },
    {	/* pal.super */					/* 5 */
	DIPF_IS_ECS | 
	DIPF_IS_PAL | 
	DIPF_IS_SPRITES | 
	DIPF_IS_DBUFFER |
	DIPF_IS_GENLOCK | 
	DIPF_IS_WB | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 11, 44 },
	35,
	64,
	{ 22, 44 },
	DEFAULT_MONITOR_ID | SUPERHIRES | EXTENDED_MODE,
    },
    {	/* ntsc.ham */
	DIPF_IS_HAM | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DBUFFER |
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 44, 52 },
	140,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | HAM | EXTENDED_MODE,
    },
    {	/* pal.ham */	
	DIPF_IS_PAL | 
	DIPF_IS_HAM | 
	DIPF_IS_SPRITES | 
	DIPF_IS_DBUFFER |
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 44, 44 },
	140,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | HAM | EXTENDED_MODE,
    },
    {	/* ntsc.extrahalfbrite */
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 44, 52 },
	140,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
    {	/* pal.extrahalfbrite */
	DIPF_IS_PAL | 
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 44, 44 },
	140,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
    {	/* ntsc.hires.ham */				/* 10 */
	DIPF_IS_AA |
	DIPF_IS_HAM |
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 22, 52 },
	70,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | HIRES | HAM | EXTENDED_MODE,
    },
    {	/* ntsc.super.ham */
	DIPF_IS_AA |
	DIPF_IS_HAM |
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DBUFFER |
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 11, 52 },
	35,
	4096,
	{ 22, 52 },
	DEFAULT_MONITOR_ID | SUPERHIRES | HAM | EXTENDED_MODE,
    },
    {	/* pal.hires.ham */
	DIPF_IS_AA |
	DIPF_IS_PAL | 
	DIPF_IS_DBUFFER |
	DIPF_IS_HAM |
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 22, 44 },
	70,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | HIRES | HAM | EXTENDED_MODE,
    },
    {	/* pal.super.ham */
	DIPF_IS_AA |
	DIPF_IS_HAM |
	DIPF_IS_PAL | 
	DIPF_IS_DBUFFER |
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_BEAMSYNC,
	{ 11, 44 },
	35,
	4096,
	{ 22, 44 },
	DEFAULT_MONITOR_ID | SUPERHIRES | HAM | EXTENDED_MODE,
    },
    {	/* ntsc.hires.extrahalfbrite */
	DIPF_IS_AA |
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 22, 52 },
	70,
	4096,
	{ 44, 52 },
	DEFAULT_MONITOR_ID | HIRES | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
    {	/* ntsc.shires.extrahalfbrite */		/* 15 */
	DIPF_IS_AA |
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 11, 52 },
	35,
	4096,
	{ 22, 52 },
	DEFAULT_MONITOR_ID | SUPERHIRES | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
    {	/* pal.hires.extrahalfbrite */
	DIPF_IS_AA |
	DIPF_IS_PAL |
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 22, 44 },
	70,
	4096,
	{ 44, 44 },
	DEFAULT_MONITOR_ID | HIRES | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
    {	/* pal.shires.extrahalfbrite */
	DIPF_IS_AA |
	DIPF_IS_PAL |
	DIPF_IS_EXTRAHALFBRITE | 
	DIPF_IS_SPRITES | 
	DIPF_IS_GENLOCK | 
	DIPF_IS_DRAGGABLE | 
	DIPF_IS_DBUFFER |
	DIPF_IS_BEAMSYNC,
	{ 11, 44 },
	35,
	4096,
	{ 22, 44 },
	DEFAULT_MONITOR_ID | SUPERHIRES | EXTRA_HALFBRITE | EXTENDED_MODE,
    },
};

struct RawMonitorInfo MntrData[] =
{
    {	/* ntsc.monitor */
	{ DTAG_MNTR, INVALID_ID, TAG_SKIP, 9 },
	  NULL,
	{ 129, 44 },
        {  44, 52 },
	{  93, 21, 136, 63 },
	  262,
	  226,
	  21,
	  MCOMPAT_MIXED,
	{ 0x00000000, 0x00000000 ,0x000036FF, 0x0000289F },
	{ 0x00000000, 0x00000000 ,0x000036FF, 0x0000289F },
	{ 22, 26 },
	{ 129, 44 },
	NTSC_MONITOR_ID | HIRES_KEY,
	{ NULL }
    },
    {	/* pal.monitor */
	{ DTAG_MNTR, INVALID_ID, TAG_SKIP, 9 },
	  NULL,
	{ 129, 44 },
        {  44, 44 },
	{  93, 29, 136, 57 },
	  312,
	  226,
	  29,
	  MCOMPAT_MIXED,
	{ 0x00000000, 0x00000000 ,0x000036FF, 0x00002BFF },
	{ 0x00000000, 0x00000000 ,0x000036FF, 0x00002BFF },
	{ 22, 22 },
	{ 129, 44 },
	PAL_MONITOR_ID | HIRES_KEY,
	{ NULL }
    },
};

struct TagItem NullData[] =
{
    { TAG_DONE, 0 }
};

UBYTE PalDisplayTags[] =
{
    0,    
    /* MntrData */ 1,	/* Default == PAL Monitor */
    /* MntrData */ 0,	/* NTSC Monitor */
    /* MntrData */ 1,   /* PAL Monitor */

    /* DimsData */ 4,	/* PAL Lores */
    /* DimsData */ 5,	/* PAL Hires */		/* 5 */
    /* DimsData */ 6,	/* PAL SHires */
    /* DimsData */ 7,	/* PAL HAM */
    /* DimsData */ 9,	/* PAL EHB */

    /* DimsData */ 0,	/* NTSC Lores */
    /* DimsData */ 1,	/* NTSC Hires */	/* 10 */
    /* DimsData */ 2,	/* NTSC SHires */
    /* DimsData */ 3,	/* NTSC HAM */
    /* DimsData */ 8,	/* NTSC EHB */

    /* DispData */ 3,	/* PAL Lores */
    /* DispData */ 4,	/* PAL Hires */		/* 15 */
    /* DispData */ 5,	/* PAL SHires */
    /* DispData */ 7,	/* PAL HAM */
    /* DispData */ 9,	/* PAL EHB */
    /* DispData */ 12,	/* PAL HiresHAM */
    /* DispData */ 13,	/* PAL SHiresHAM */	/* 20 */

    /* DispData */ 0,	/* NTSC Lores */
    /* DispData */ 1,	/* NTSC Hires */
    /* DispData */ 2,	/* NTSC SHires */
    /* DispData */ 6,	/* NTSC HAM */
    /* DispData */ 8,	/* NTSC EHB */		/* 25 */
    /* DispData */ 10,	/* NTSC HiresHAM */
    /* DispData */ 11,	/* NTSC SHiresHAM */

    /* DimsData */ 4,	/* PAL Lores */
    /* DimsData */ 5,	/* PAL Hires */
    /* DimsData */ 6,	/* PAL SHires */	/* 30 */
    /* DimsData */ 7,	/* PAL HAM */
    /* DimsData */ 9,	/* PAL EHB */

    /* DispData */ 3,	/* PAL Lores */
    /* DispData */ 4,	/* PAL Hires */
    /* DispData */ 5,	/* PAL SHires */	/* 35 */
    /* DispData */ 7,	/* PAL HAM */
    /* DispData */ 9,	/* PAL EHB */
    /* DispData */ 12,	/* PAL HiresHAM */
    /* DispData */ 13,	/* PAL SHiresHAM */

    /* ProgData */ 0,	/* Lores */		/* 40 */
    /* ProgData */ 1,	/* Hires */
    /* ProgData */ 2,	/* SHires */
    /* ProgData */ 3,	/* LoresHAM */
    /* ProgData */ 4,	/* HiresHAM */
    /* ProgData */ 5,	/* SHiresHAM */		/* 45 */
    /* ProgData */ 6,	/* LoresEHB */
    /* ProgData */ 7,	/* LoresDPF */
    /* ProgData */ 8,	/* HiresDPF */
    /* ProgData */ 9,	/* SHiresDPF */
    /* ProgData */ 10,	/* LoresDPF2 */		/* 50 */
    /* ProgData */ 11,	/* HiresDPF2 */
    /* ProgData */ 12,	/* SHiresDPF2 */
    /* ProgData */ 26,	/* HiresEHB */
    /* ProgData */ 27,	/* SHiresEHB */

    /* DimsData */ 12,	/* PAL HIRES EHB */	/* 55 */
    /* DimsData */ 13,	/* PAL SHIRES EHB  */
    /* DimsData */ 12,	/* PAL HIRES EHB */
    /* DimsData */ 13,	/* PAL SHIRES EHB  */
    /* DimsData */ 10,	/* NTSC HIRES EHB */
    /* DimsData */ 11,	/* NTSC SHIRES EHB  */	/* 60 */
    /* DispData */ 16,	/* PAL HIRES EHB */
    /* DispData */ 17,	/* PAL SHIRES EHB */
    /* DispData */ 16,	/* PAL HIRES EHB */
    /* DispData */ 17,	/* PAL SHIRES EHB */
    /* DispData */ 14,	/* NTSC HIRES EHB */	/* 65 */
    /* DispData */ 15,	/* NTSC SHIRES EHB */

    /* ProgData */ 30,	/* LoresSDbl */
    /* ProgData */ 31,	/* LoresHAMSDbl */
    /* ProgData */ 32,	/* LoresEHBSDbl */
    /* ProgData */ 33,	/* HiresHAMSDbl */	/* 70 */
};

UBYTE NtscDisplayTags[] =
{
    0,    
    /* MntrData */ 0,	/* Default == NTSC Monitor */
    /* MntrData */ 0,	/* NTSC Monitor */
    /* MntrData */ 1,	/* PAL Monitor */

    /* DimsData */ 4,	/* PAL Lores */
    /* DimsData */ 5,	/* PAL Hires */		/* 5 */
    /* DimsData */ 6,	/* PAL SHires */
    /* DimsData */ 7,	/* PAL HAM */
    /* DimsData */ 9,	/* PAL EHB */

    /* DimsData */ 0,	/* NTSC Lores */
    /* DimsData */ 1,	/* NTSC Hires */	/* 10 */
    /* DimsData */ 2,	/* NTSC SHires */
    /* DimsData */ 3,	/* NTSC HAM */
    /* DimsData */ 8,	/* NTSC EHB */

    /* DispData */ 3,	/* PAL Lores */
    /* DispData */ 4,	/* PAL Hires */		/* 15 */
    /* DispData */ 5,	/* PAL SHires */
    /* DispData */ 7,	/* PAL HAM */
    /* DispData */ 9,	/* PAL EHB */
    /* DispData */ 12,	/* PAL HiresHAM */
    /* DispData */ 13,	/* PAL SHiresHAM */	/* 20 */

    /* DispData */ 0,	/* NTSC Lores */
    /* DispData */ 1,	/* NTSC Hires */
    /* DispData */ 2,	/* NTSC SHires */
    /* DispData */ 6,	/* NTSC HAM */
    /* DispData */ 8,	/* NTSC EHB */		/* 25 */
    /* DispData */ 10,	/* NTSC HiresHAM */
    /* DispData */ 11,	/* NTSC SHiresHAM */

    /* DimsData */ 0,	/* NTSC Lores */
    /* DimsData */ 1,	/* NTSC Hires */
    /* DimsData */ 2,	/* NTSC SHires */	/* 30 */
    /* DimsData */ 3,	/* NTSC HAM */
    /* DimsData */ 8,	/* NTSC EHB */

    /* DispData */ 0,	/* NTSC Lores */
    /* DispData */ 1,	/* NTSC Hires */
    /* DispData */ 2,	/* NTSC SHires */	/* 35 */
    /* DispData */ 6,	/* NTSC HAM */
    /* DispData */ 8,	/* NTSC EHB */
    /* DispData */ 10,	/* NTSC HiresHAM */
    /* DispData */ 11,	/* NTSC SHiresHAM */

    /* ProgData */ 0,	/* Lores */		/* 40 */
    /* ProgData */ 1,	/* Hires */
    /* ProgData */ 2,	/* SHires */
    /* ProgData */ 3,	/* LoresHAM */
    /* ProgData */ 4,	/* HiresHAM */
    /* ProgData */ 5,	/* SHiresHAM */		/* 45 */
    /* ProgData */ 6,	/* LoresEHB */
    /* ProgData */ 7,	/* LoresDPF */
    /* ProgData */ 8,	/* HiresDPF */
    /* ProgData */ 9,	/* SHiresDPF */
    /* ProgData */ 10,	/* LoresDPF2 */		/* 50 */
    /* ProgData */ 11,	/* HiresDPF2 */
    /* ProgData */ 12,	/* SHiresDPF2 */
    /* ProgData */ 26,	/* HiresEHB */
    /* ProgData */ 27,	/* SHiresEHB */

    /* DimsData */ 10,	/* NTSC HIRES EHB */	/* 55 */
    /* DimsData */ 11,	/* NTSC SHIRES EHB  */
    /* DimsData */ 12,	/* PAL HIRES EHB */
    /* DimsData */ 13,	/* PAL SHIRES EHB  */
    /* DimsData */ 10,	/* NTSC HIRES EHB */
    /* DimsData */ 11,	/* NTSC SHIRES EHB  */	/* 60 */
    /* DispData */ 14,	/* NTSC HIRES EHB */
    /* DispData */ 15,	/* NTSC SHIRES EHB */
    /* DispData */ 16,	/* PAL HIRES EHB */
    /* DispData */ 17,	/* PAL SHIRES EHB */
    /* DispData */ 14,	/* NTSC HIRES EHB */	/* 65 */
    /* DispData */ 15,	/* NTSC SHIRES EHB */

    /* ProgData */ 30,	/* LoresSDbl */
    /* ProgData */ 31,	/* LoresHAMSDbl */
    /* ProgData */ 32,	/* LoresEHBSDbl */
    /* ProgData */ 33,	/* HiresHAMSDbl */	/* 70 */
};


/**************************************************************************/
/*                                                                        */
/* RomCompressedNodes must be stored in the following order:              */
/*                                                                        */
/* MntrBind[]                                                             */
/* DimsBind[]                                                             */
/* DispBindNorm[]                                                         */
/* DispBindLace[]                                                         */
/* DispBindDual[]                                                         */
/* DispBindLaceDual[]                                                     */
/* DispBindDual2[]                                                        */
/* DispBindLaceDual2[]                                                    */
/* DispBindHam[]                                                          */
/* DispBindHamLace[]                                                      */
/* DispBindEHB[]                                                          */
/* DispBindEHBLace[]                                                      */
/* ProgBindNorm[]                                                         */
/* ProgBindNormLace[]                                                     */
/*                                                                        */
/* This is so the startup code can fall from one array through to the     */
/* next, and so avoid 120-odd pointers                                    */
/*                                                                        */
/**************************************************************************/

/*************************** Default Stuff ********************************/

struct RomCompressedNode DefaultMntrBind[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* mntr */
    { 0, 1,DEFAULT_MONITOR_ID | LORES_KEY },    

};

struct RomCompressedNode DefaultDimsBind[] =
{
    {22, 0,DEFAULT_MONITOR_ID >>16 }, /* dims */

    { 0,28,DEFAULT_MONITOR_ID | LORES_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRES_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPER_KEY },    
    { 0,31,DEFAULT_MONITOR_ID | HAM_KEY },    
    { 0,32,DEFAULT_MONITOR_ID | EXTRAHALFBRITE_KEY },    

    { 0,28,DEFAULT_MONITOR_ID | LORESLACE_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESLACE_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERLACE_KEY },    
    { 0,31,DEFAULT_MONITOR_ID | HAMLACE_KEY },    
    { 0,32,DEFAULT_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    

    { 0,31,DEFAULT_MONITOR_ID | LORESDPF_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESDPF_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERDPF_KEY },    
    { 0,31,DEFAULT_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERLACEDPF_KEY },    

    { 0,31,DEFAULT_MONITOR_ID | LORESDPF2_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESDPF2_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERDPF2_KEY },    
    { 0,31,DEFAULT_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERLACEDPF2_KEY },    
};
struct RomCompressedNode DefaultAADimsBind[] =
{
    {12, 0,DEFAULT_MONITOR_ID >>16 }, /* dims */

    { 0,29,DEFAULT_MONITOR_ID | HIRESHAM_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERHAM_KEY },    
    { 0,29,DEFAULT_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,30,DEFAULT_MONITOR_ID | SUPERHAMLACE_KEY },    

    { 0,55,DEFAULT_MONITOR_ID | HIRESEHB_KEY },
    { 0,56,DEFAULT_MONITOR_ID | SUPEREHB_KEY },
    { 0,55,DEFAULT_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0,56,DEFAULT_MONITOR_ID | SUPEREHBLACE_KEY },

    { 0,28,DEFAULT_MONITOR_ID | LORESSDBL_KEY },
    { 0,31,DEFAULT_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,32,DEFAULT_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0,29,DEFAULT_MONITOR_ID | HIRESHAMSDBL_KEY },
};

struct RomCompressedNode DefaultDispBindNorm[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORES_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRES_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPER_KEY },    

};

struct RomCompressedNode DefaultDispBindLace[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORESLACE_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRESLACE_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPERLACE_KEY },    

};

struct RomCompressedNode DefaultDispBindDual[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORESDPF_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRESDPF_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPERDPF_KEY },    

};

struct RomCompressedNode DefaultDispBindLaceDual[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPERLACEDPF_KEY },    

};

struct RomCompressedNode DefaultDispBindDual2[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORESDPF2_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRESDPF2_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPERDPF2_KEY },    

};

struct RomCompressedNode DefaultDispBindLaceDual2[] =
{
    { 3, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */

    { 0, 33,DEFAULT_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0, 34,DEFAULT_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0, 35,DEFAULT_MONITOR_ID | SUPERLACEDPF2_KEY },    

};

struct RomCompressedNode DefaultDispBindHam[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,36,DEFAULT_MONITOR_ID | HAM_KEY },    
};
struct RomCompressedNode DefaultDispBindHamLace[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,36,DEFAULT_MONITOR_ID | HAMLACE_KEY },    
};

struct RomCompressedNode DefaultDispBindEHB[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,37,DEFAULT_MONITOR_ID | EXTRAHALFBRITE_KEY },    
};
struct RomCompressedNode DefaultDispBindEHBLace[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,37,DEFAULT_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
};

struct RomCompressedNode DefaultProgBind[] = 
{
    { 22, 0,  DEFAULT_MONITOR_ID >> 16 },
    { 0, 40, DEFAULT_MONITOR_ID | LORES_KEY },
    { 0, 41, DEFAULT_MONITOR_ID | HIRES_KEY },
    { 0, 42, DEFAULT_MONITOR_ID | SUPER_KEY },
    { 0, 43, DEFAULT_MONITOR_ID | HAM_KEY },
    { 0, 46, DEFAULT_MONITOR_ID | EXTRAHALFBRITE_KEY },
    { 0, 47, DEFAULT_MONITOR_ID | LORESDPF_KEY },
    { 0, 48, DEFAULT_MONITOR_ID | HIRESDPF_KEY },
    { 0, 49, DEFAULT_MONITOR_ID | SUPERDPF_KEY },
    { 0, 50, DEFAULT_MONITOR_ID | LORESDPF2_KEY },
    { 0, 51, DEFAULT_MONITOR_ID | HIRESDPF2_KEY },
    { 0, 52, DEFAULT_MONITOR_ID | SUPERDPF2_KEY },
    { 0, 40, DEFAULT_MONITOR_ID | LORESLACE_KEY },
    { 0, 41, DEFAULT_MONITOR_ID | HIRESLACE_KEY },
    { 0, 42, DEFAULT_MONITOR_ID | SUPERLACE_KEY },
    { 0, 43, DEFAULT_MONITOR_ID | HAMLACE_KEY },
    { 0, 46, DEFAULT_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
    { 0, 47, DEFAULT_MONITOR_ID | LORESLACEDPF_KEY },
    { 0, 48, DEFAULT_MONITOR_ID | HIRESLACEDPF_KEY },
    { 0, 49, DEFAULT_MONITOR_ID | SUPERLACEDPF_KEY },
    { 0, 50, DEFAULT_MONITOR_ID | LORESLACEDPF2_KEY },
    { 0, 51, DEFAULT_MONITOR_ID | HIRESLACEDPF2_KEY },
    { 0, 52, DEFAULT_MONITOR_ID | SUPERLACEDPF2_KEY },
};

struct RomCompressedNode DefaultAADispBindSDbl[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,33,DEFAULT_MONITOR_ID | LORESSDBL_KEY },
};

struct RomCompressedNode DefaultAADispBindHam[] =
{
    { 2, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,38,DEFAULT_MONITOR_ID | HIRESHAM_KEY },    
    { 0,39,DEFAULT_MONITOR_ID | SUPERHAM_KEY },    
};
struct RomCompressedNode DefaultAADispBindHamLace[] =
{
    { 2, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,38,DEFAULT_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,39,DEFAULT_MONITOR_ID | SUPERHAMLACE_KEY },    
};
struct RomCompressedNode DefaultAADispBindHAMSDbl[] =
{
    { 2, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,36,DEFAULT_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,38,DEFAULT_MONITOR_ID | HIRESHAMSDBL_KEY },
};

struct RomCompressedNode DefaultAADispBindEHB[] =
{
    { 2, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,61,DEFAULT_MONITOR_ID | HIRESEHB_KEY },    
    { 0,62,DEFAULT_MONITOR_ID | SUPEREHB_KEY },    
};
struct RomCompressedNode DefaultAADispBindEHBLace[] =
{
    { 2, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,61,DEFAULT_MONITOR_ID | HIRESEHBLACE_KEY },    
    { 0,62,DEFAULT_MONITOR_ID | SUPEREHBLACE_KEY },    
};
struct RomCompressedNode DefaultAADispBindEHBSDbl[] =
{
    { 1, 0,DEFAULT_MONITOR_ID >>16 }, /* disp */
    { 0,37,DEFAULT_MONITOR_ID | LORESEHBSDBL_KEY },
};

struct RomCompressedNode DefaultAAProgBind[] = 
{
    { 12, 0,  DEFAULT_MONITOR_ID >> 16 },
    { 0, 44, DEFAULT_MONITOR_ID | HIRESHAM_KEY },
    { 0, 45, DEFAULT_MONITOR_ID | SUPERHAM_KEY },
    { 0, 53, DEFAULT_MONITOR_ID | HIRESEHB_KEY },
    { 0, 54, DEFAULT_MONITOR_ID | SUPEREHB_KEY },
    { 0, 44, DEFAULT_MONITOR_ID | HIRESHAMLACE_KEY },
    { 0, 45, DEFAULT_MONITOR_ID | SUPERHAMLACE_KEY },
    { 0, 53, DEFAULT_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0, 54, DEFAULT_MONITOR_ID | SUPEREHBLACE_KEY },
    { 0, 67, DEFAULT_MONITOR_ID | LORESSDBL_KEY },
    { 0, 68, DEFAULT_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0, 69, DEFAULT_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0, 70, DEFAULT_MONITOR_ID | HIRESHAMSDBL_KEY },
};

/***************************** PAL Stuff **********************************/

struct RomCompressedNode PalMntrBind[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0, 3,PAL_MONITOR_ID | LORES_KEY },    
};

struct RomCompressedNode PalDimsBind[] =
{
    {22, 0,PAL_MONITOR_ID >>16 }, 

    { 0,4,PAL_MONITOR_ID | LORES_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRES_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPER_KEY },    
    { 0,7,PAL_MONITOR_ID | HAM_KEY },    
    { 0,8,PAL_MONITOR_ID | EXTRAHALFBRITE_KEY },    

    { 0,4,PAL_MONITOR_ID | LORESLACE_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESLACE_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERLACE_KEY },    
    { 0,7,PAL_MONITOR_ID | HAMLACE_KEY },    
    { 0,8,PAL_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    

    { 0,7,PAL_MONITOR_ID | LORESDPF_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESDPF_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERDPF_KEY },    
    { 0,7,PAL_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERLACEDPF_KEY },    

    { 0,7,PAL_MONITOR_ID | LORESDPF2_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESDPF2_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERDPF2_KEY },    
    { 0,7,PAL_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERLACEDPF2_KEY },    
};
struct RomCompressedNode PalAADimsBind[] =
{
    {12, 0,PAL_MONITOR_ID >>16 }, 

    { 0,5,PAL_MONITOR_ID | HIRESHAM_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERHAM_KEY },    
    { 0,5,PAL_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,6,PAL_MONITOR_ID | SUPERHAMLACE_KEY },    

    { 0,57,PAL_MONITOR_ID | HIRESEHB_KEY },
    { 0,58,PAL_MONITOR_ID | SUPEREHB_KEY },
    { 0,57,PAL_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0,58,PAL_MONITOR_ID | SUPEREHBLACE_KEY },

    { 0,4,PAL_MONITOR_ID | LORESSDBL_KEY },
    { 0,7,PAL_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,8,PAL_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0,5,PAL_MONITOR_ID | HIRESHAMSDBL_KEY },
};

struct RomCompressedNode PalDispBindNorm[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORES_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRES_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPER_KEY },    

};

struct RomCompressedNode PalDispBindLace[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORESLACE_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRESLACE_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPERLACE_KEY },    

};

struct RomCompressedNode PalDispBindDual[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORESDPF_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRESDPF_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPERDPF_KEY },    

};

struct RomCompressedNode PalDispBindLaceDual[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPERLACEDPF_KEY },    

};

struct RomCompressedNode PalDispBindDual2[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORESDPF2_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRESDPF2_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPERDPF2_KEY },    

};

struct RomCompressedNode PalDispBindLaceDual2[] =
{
    { 3, 0,PAL_MONITOR_ID >>16 }, 

    { 0,14,PAL_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0,15,PAL_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0,16,PAL_MONITOR_ID | SUPERLACEDPF2_KEY },    

};

struct RomCompressedNode PalDispBindHam[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0,17,PAL_MONITOR_ID | HAM_KEY },    
};
struct RomCompressedNode PalDispBindHamLace[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0,17,PAL_MONITOR_ID | HAMLACE_KEY },    
};

struct RomCompressedNode PalDispBindEHB[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0,18,PAL_MONITOR_ID | EXTRAHALFBRITE_KEY },    
};
struct RomCompressedNode PalDispBindEHBLace[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0,18,PAL_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
};

struct RomCompressedNode PalProgBind[] = 
{
    { 22, 0,  PAL_MONITOR_ID >> 16 },
    { 0, 40, PAL_MONITOR_ID | LORES_KEY },
    { 0, 41, PAL_MONITOR_ID | HIRES_KEY },
    { 0, 42, PAL_MONITOR_ID | SUPER_KEY },
    { 0, 43, PAL_MONITOR_ID | HAM_KEY },
    { 0, 46, PAL_MONITOR_ID | EXTRAHALFBRITE_KEY },    
    { 0, 47, PAL_MONITOR_ID | LORESDPF_KEY },
    { 0, 48, PAL_MONITOR_ID | HIRESDPF_KEY },
    { 0, 49, PAL_MONITOR_ID | SUPERDPF_KEY },
    { 0, 50, PAL_MONITOR_ID | LORESDPF2_KEY },
    { 0, 51, PAL_MONITOR_ID | HIRESDPF2_KEY },
    { 0, 52, PAL_MONITOR_ID | SUPERDPF2_KEY },
    { 0, 40, PAL_MONITOR_ID | LORESLACE_KEY },
    { 0, 41, PAL_MONITOR_ID | HIRESLACE_KEY },
    { 0, 42, PAL_MONITOR_ID | SUPERLACE_KEY },
    { 0, 43, PAL_MONITOR_ID | HAMLACE_KEY },
    { 0, 46, PAL_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
    { 0, 47, PAL_MONITOR_ID | LORESLACEDPF_KEY },
    { 0, 48, PAL_MONITOR_ID | HIRESLACEDPF_KEY },
    { 0, 49, PAL_MONITOR_ID | SUPERLACEDPF_KEY },
    { 0, 50, PAL_MONITOR_ID | LORESLACEDPF2_KEY },
    { 0, 51, PAL_MONITOR_ID | HIRESLACEDPF2_KEY },
    { 0, 52, PAL_MONITOR_ID | SUPERLACEDPF2_KEY },
};

struct RomCompressedNode PalAADispBindSDbl[] =
{
    { 1, 0,PAL_MONITOR_ID >> 16 },
    { 0, 14, PAL_MONITOR_ID | LORESSDBL_KEY },
};

struct RomCompressedNode PalAADispBindHam[] =
{
    { 2, 0,PAL_MONITOR_ID >>16 }, 
    { 0,19,PAL_MONITOR_ID | HIRESHAM_KEY },    
    { 0,20,PAL_MONITOR_ID | SUPERHAM_KEY },    
};
struct RomCompressedNode PalAADispBindHamLace[] =
{
    { 2, 0,PAL_MONITOR_ID >>16 }, 
    { 0,19,PAL_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,20,PAL_MONITOR_ID | SUPERHAMLACE_KEY },    
};
struct RomCompressedNode PalAADispBindHamSDbl[] =
{
    { 2, 0,PAL_MONITOR_ID >>16 }, 
    { 0,17,PAL_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,19,PAL_MONITOR_ID | HIRESHAMSDBL_KEY },    
};

struct RomCompressedNode PalAADispBindEHB[] =
{
    { 2, 0,PAL_MONITOR_ID >>16 }, 
    { 0,63,PAL_MONITOR_ID | HIRESEHB_KEY },    
    { 0,64,PAL_MONITOR_ID | SUPEREHB_KEY },    
};
struct RomCompressedNode PalAADispBindEHBLace[] =
{
    { 2, 0,PAL_MONITOR_ID >>16 }, 
    { 0,63,PAL_MONITOR_ID | HIRESEHBLACE_KEY },    
    { 0,64,PAL_MONITOR_ID | SUPEREHBLACE_KEY },    
};
struct RomCompressedNode PalAADispBindEHBSDbl[] =
{
    { 1, 0,PAL_MONITOR_ID >>16 }, 
    { 0,18,PAL_MONITOR_ID | LORESEHBSDBL_KEY },    
};

struct RomCompressedNode PalAAProgBind[] = 
{
    { 12, 0,  PAL_MONITOR_ID >> 16 },
    { 0, 44, PAL_MONITOR_ID | HIRESHAM_KEY },
    { 0, 45, PAL_MONITOR_ID | SUPERHAM_KEY },
    { 0, 53, PAL_MONITOR_ID | HIRESEHB_KEY },
    { 0, 54, PAL_MONITOR_ID | SUPEREHB_KEY },
    { 0, 44, PAL_MONITOR_ID | HIRESHAMLACE_KEY },
    { 0, 45, PAL_MONITOR_ID | SUPERHAMLACE_KEY },
    { 0, 53, PAL_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0, 54, PAL_MONITOR_ID | SUPEREHBLACE_KEY },
    { 0, 67, PAL_MONITOR_ID | LORESSDBL_KEY },
    { 0, 68, PAL_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0, 69, PAL_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0, 70, PAL_MONITOR_ID | HIRESHAMSDBL_KEY },
};

/***************************** NTSC Stuff **********************************/

struct RomCompressedNode NtscMntrBind[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 },
    { 0, 2,NTSC_MONITOR_ID | LORES_KEY },    

};

struct RomCompressedNode NtscDimsBind[] =
{
    {22, 0,NTSC_MONITOR_ID >>16 },

    { 0,9,NTSC_MONITOR_ID | LORES_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRES_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPER_KEY },    
    { 0,12,NTSC_MONITOR_ID | HAM_KEY },    
    { 0,13,NTSC_MONITOR_ID | EXTRAHALFBRITE_KEY },    

    { 0,9,NTSC_MONITOR_ID | LORESLACE_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESLACE_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERLACE_KEY },    
    { 0,12,NTSC_MONITOR_ID | HAMLACE_KEY },    
    { 0,13,NTSC_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    

    { 0,12,NTSC_MONITOR_ID | LORESDPF_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESDPF_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERDPF_KEY },    
    { 0,12,NTSC_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERLACEDPF_KEY },    

    { 0,12,NTSC_MONITOR_ID | LORESDPF2_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESDPF2_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERDPF2_KEY },    
    { 0,12,NTSC_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERLACEDPF2_KEY },    
};
struct RomCompressedNode NtscAADimsBind[] =
{
    {12, 0,NTSC_MONITOR_ID >>16 },

    { 0,10,NTSC_MONITOR_ID | HIRESHAM_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERHAM_KEY },    
    { 0,10,NTSC_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,11,NTSC_MONITOR_ID | SUPERHAMLACE_KEY },    

    { 0,59,NTSC_MONITOR_ID | HIRESEHB_KEY },
    { 0,60,NTSC_MONITOR_ID | SUPEREHB_KEY },
    { 0,59,NTSC_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0,60,NTSC_MONITOR_ID | SUPEREHBLACE_KEY },

    { 0,9,NTSC_MONITOR_ID | LORESSDBL_KEY },
    { 0,12,NTSC_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,13,NTSC_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0,10,NTSC_MONITOR_ID | HIRESHAMSDBL_KEY },
};

struct RomCompressedNode NtscDispBindNorm[] =
{
    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORES_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRES_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPER_KEY },    

};

struct RomCompressedNode NtscDispBindLace[] =
{
    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORESLACE_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRESLACE_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPERLACE_KEY },    

};

struct RomCompressedNode NtscDispBindDual[] =
{
    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORESDPF_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRESDPF_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPERDPF_KEY },    

};

struct RomCompressedNode NtscDispBindLaceDual[] =
{
    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORESLACEDPF_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRESLACEDPF_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPERLACEDPF_KEY },    

};

struct RomCompressedNode NtscDispBindDual2[] =
{
    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORESDPF2_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRESDPF2_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPERDPF2_KEY },    

};

struct RomCompressedNode NtscDispBindLaceDual2[] =
{

    { 3, 0,NTSC_MONITOR_ID >>16 },

    { 0,21,NTSC_MONITOR_ID | LORESLACEDPF2_KEY },    
    { 0,22,NTSC_MONITOR_ID | HIRESLACEDPF2_KEY },    
    { 0,23,NTSC_MONITOR_ID | SUPERLACEDPF2_KEY },    

};

struct RomCompressedNode NtscDispBindHam[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 },
    { 0,24,NTSC_MONITOR_ID | HAM_KEY },    
};
struct RomCompressedNode NtscDispBindHamLace[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 },
    { 0,24,NTSC_MONITOR_ID | HAMLACE_KEY },    
};

struct RomCompressedNode NtscDispBindEHB[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 },
    { 0,25,NTSC_MONITOR_ID | EXTRAHALFBRITE_KEY },    
};
struct RomCompressedNode NtscDispBindEHBLace[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 },
    { 0,25,NTSC_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
};

struct RomCompressedNode NtscProgBind[] = 
{
    { 22, 0,  NTSC_MONITOR_ID >> 16 },
    { 0, 40, NTSC_MONITOR_ID | LORES_KEY },
    { 0, 41, NTSC_MONITOR_ID | HIRES_KEY },
    { 0, 42, NTSC_MONITOR_ID | SUPER_KEY },
    { 0, 43, NTSC_MONITOR_ID | HAM_KEY },
    { 0, 46, NTSC_MONITOR_ID | EXTRAHALFBRITE_KEY },    
    { 0, 47, NTSC_MONITOR_ID | LORESDPF_KEY },
    { 0, 48, NTSC_MONITOR_ID | HIRESDPF_KEY },
    { 0, 49, NTSC_MONITOR_ID | SUPERDPF_KEY },
    { 0, 50, NTSC_MONITOR_ID | LORESDPF2_KEY },
    { 0, 51, NTSC_MONITOR_ID | HIRESDPF2_KEY },
    { 0, 52, NTSC_MONITOR_ID | SUPERDPF2_KEY },
    { 0, 40, NTSC_MONITOR_ID | LORESLACE_KEY },
    { 0, 41, NTSC_MONITOR_ID | HIRESLACE_KEY },
    { 0, 42, NTSC_MONITOR_ID | SUPERLACE_KEY },
    { 0, 43, NTSC_MONITOR_ID | HAMLACE_KEY },
    { 0, 46, NTSC_MONITOR_ID | EXTRAHALFBRITELACE_KEY },    
    { 0, 47, NTSC_MONITOR_ID | LORESLACEDPF_KEY },
    { 0, 48, NTSC_MONITOR_ID | HIRESLACEDPF_KEY },
    { 0, 49, NTSC_MONITOR_ID | SUPERLACEDPF_KEY },
    { 0, 50, NTSC_MONITOR_ID | LORESLACEDPF2_KEY },
    { 0, 51, NTSC_MONITOR_ID | HIRESLACEDPF2_KEY },
    { 0, 52, NTSC_MONITOR_ID | SUPERLACEDPF2_KEY },
};

struct RomCompressedNode NTSCAADispBindSDbl[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 }, /* disp */
    { 0,21,NTSC_MONITOR_ID | LORESSDBL_KEY },
};

struct RomCompressedNode NtscAADispBindHam[] =
{
    { 2, 0,NTSC_MONITOR_ID >>16 },
    { 0,26,NTSC_MONITOR_ID | HIRESHAM_KEY },    
    { 0,27,NTSC_MONITOR_ID | SUPERHAM_KEY },    
};
struct RomCompressedNode NtscAADispBindHamLace[] =
{
    { 2, 0,NTSC_MONITOR_ID >>16 },
    { 0,26,NTSC_MONITOR_ID | HIRESHAMLACE_KEY },    
    { 0,27,NTSC_MONITOR_ID | SUPERHAMLACE_KEY },    
};
struct RomCompressedNode NtscAADispBindHamSDbl[] =
{
    { 2, 0,NTSC_MONITOR_ID >>16 },
    { 0,24,NTSC_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0,26,NTSC_MONITOR_ID | HIRESHAMSDBL_KEY },    
};

struct RomCompressedNode NtscAADispBindEHB[] =
{
    { 2, 0,NTSC_MONITOR_ID >>16 },
    { 0,65,NTSC_MONITOR_ID | HIRESEHB_KEY },    
    { 0,66,NTSC_MONITOR_ID | SUPEREHB_KEY },    
};
struct RomCompressedNode NtscAADispBindEHBLace[] =
{
    { 2, 0,NTSC_MONITOR_ID >>16 },
    { 0,65,NTSC_MONITOR_ID | HIRESEHBLACE_KEY },    
    { 0,66,NTSC_MONITOR_ID | SUPEREHBLACE_KEY },    
};
struct RomCompressedNode NTSCAADispBindEHBSDbl[] =
{
    { 1, 0,NTSC_MONITOR_ID >>16 }, /* disp */
    { 0,25,NTSC_MONITOR_ID | LORESEHBSDBL_KEY },
};

struct RomCompressedNode NtscAAProgBind[] = 
{
    { 12, 0,  NTSC_MONITOR_ID >> 16 },
    { 0, 44, NTSC_MONITOR_ID | HIRESHAM_KEY },
    { 0, 45, NTSC_MONITOR_ID | SUPERHAM_KEY },
    { 0, 53, NTSC_MONITOR_ID | HIRESEHB_KEY },
    { 0, 54, NTSC_MONITOR_ID | SUPEREHB_KEY },
    { 0, 44, NTSC_MONITOR_ID | HIRESHAMLACE_KEY },
    { 0, 45, NTSC_MONITOR_ID | SUPERHAMLACE_KEY },
    { 0, 53, NTSC_MONITOR_ID | HIRESEHBLACE_KEY },
    { 0, 54, NTSC_MONITOR_ID | SUPEREHBLACE_KEY },
    { 0, 67, NTSC_MONITOR_ID | LORESSDBL_KEY },
    { 0, 68, NTSC_MONITOR_ID | LORESHAMSDBL_KEY },
    { 0, 69, NTSC_MONITOR_ID | LORESEHBSDBL_KEY },
    { 0, 70, NTSC_MONITOR_ID | HIRESHAMSDBL_KEY },
};

APTR DataBaseStuff[] =
{
	&DefaultMntrBind,
	&NtscMntrBind,
	&PalMntrBind,
};
