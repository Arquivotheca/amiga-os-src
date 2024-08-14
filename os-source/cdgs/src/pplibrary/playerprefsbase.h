/* :ts=4
*
*	playerprefsbase.h
*
*	William A. Ware							AC27
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright (C) 1990, Silent Software Incorporated.						*
*   All Rights Reserved.													*
****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

struct PlayerPrefsBase { 
	struct Library			ppb_Lib; 
	APTR	     			ppb_DataArea; 
	ULONG        			ppb_SegList; 
	ULONG        			ppb_Flags; 
	struct Library      	*ppb_ExecBase;        	/* pointer to exec base */ 
	struct Library			*ppb_DeBoxBase;
	struct GfxBase			*ppb_GfxBase;
	struct Library                  *ppb_IntuitionBase;
	struct Library                  *ppb_LowLevelBase;
	
	BYTE					ppb_Blanked;		/* !0 if screen is blanked */
	BYTE					ppb_TitleStatus;		/* status of title prog */
	UBYTE					ppb_TitleSigNo;
	UBYTE					ppb_TitleAns;
	struct TMData			*ppb_TMData;
	struct Task				*ppb_TitleSigTask;
	struct MsgPort			*ppb_TitlePort;
	UBYTE					ppb_CDTVVersionStr[8];	/* up to 7 char vers str */
	struct SignalSemaphore	ppb_LibLock;			/* For modification */

	/* CDTVRotate */
	struct SignalSemaphore	ppb_CRLock;
	struct BitMap			*ppb_CRBm;
	LONG					ppb_CROpenCnt;
};

#define TITLESTATUS_ENDED		0xff
#define TITLESTATUS_OFF			0
#define TITLESTATUS_LEAVE		1	
#define TITLESTATUS_NORMAL		2
#define TITLESTATUS_ERROR		3
#define TITLESTATUS_CDTM		4	/* For later */

#define TITLESTATUS_MAX			4

#define TITLESTATUS_BLANK		5



/*
 * This is the Default for the CDTVPrefs structure.  The only value that will
 * change is the default language - Both are defined below -- These should
 * probley go into CDTVPrefs.h.
 */
#define CDTVLANG_DEFAULT_NTSC 		CDTVLANG_AMERICAN
#define CDTVLANG_DEFAULT_PAL		CDTVLANG_ENGLISH

