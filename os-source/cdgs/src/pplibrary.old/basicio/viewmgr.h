/* :ts=4
*
*   cdtv.h -- scr/prefs
*
*   William A. Ware                         9007.30
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY                        *
*   Copyright (C) 1990, Silent Software Incorporated.                       *
*   All Rights Reserved.                                                    *
****************************************************************************/

#ifndef EXEC_INTERRUPTS_H
#include <exec/interrupts.h>
#endif

#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif

#ifndef CDTVPREFS_H
#include <cdtv/cdtvprefs.h>
#endif


/* COLOR CYCLING interrupt info */

struct CycleIntInfo
{
    struct Interrupt    cintd_intr;
    struct BMInfo       *cintd_BMInfo;          /* Disable */
    struct ViewPort     *cintd_View;	        /* Disable */
	UWORD				**cintd_CCList;			/* From load */
    ULONG               cintd_VCountDown;       /* use as you wish */
    ULONG               cintd_VCountUp;         /* use as you wish */
    UBYTE               cintd_Bump;             /* set to 1 to update screen */
    UBYTE               cintd_Flags;            /* see below */
			/* Fade */
	ULONG				cintd_FadeMask;
    		/* Fade 0 info */
	UBYTE               cintd_Fade0;            
    UBYTE               cintd_DestFade0;        
	LONG                cintd_MicroFadeDelay0;  
    		/* Fade 1 info:			AltCMap fade */
	UBYTE				cintd_Fade1;
	UBYTE				cintd_DestFade1;
	LONG                cintd_MicroFadeDelay1;  
	UWORD				*cintd_AltCMap;
			/* Load new view */
	UWORD				**cintd_LoadCCList;
    struct View         *cintd_LoadView;
        
		/* Private information */   
    LONG                cintd_vtime;           	/* Init on setup */
    LONG                cintd_fadetimer0;       /* P */
    LONG                cintd_fadetimer1;       /* P */
    struct PlayerPrefsBase	*cintd_PlayerPrefsBase;	/* P */
	struct GfxBase      	*cintd_GfxBase;         /* P */
    struct DeBoxBase    	*cintd_DeBoxBase;       /* P */
    struct View         *cintd_oldview;         /* P */
};



/*------------------------------------------------------------------*/

struct BuildBg_Info
{
	struct Header 	*BackHeader;
	UBYTE			*BackData;
	struct Header	*ForHeader;
	UBYTE			*ForData;
	WORD			Width,Height,Depth;
	WORD			ForXOff,ForYOff;
	LONG			flags;
};


/*------------------------------------------------------------------*/


#define CINTD_NOCYCLE   1
