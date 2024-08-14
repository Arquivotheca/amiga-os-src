/* :ts=4
*
*	cdgsupp.h
*
*	William A. Ware			B305
*
*****************************************************************************
*	This information is CONFIDENTIAL and PROPRIETARY						*
*	Copyright (C) 1990, Silent Software Incorporated.						*
*	All Rights Reserved.													*
****************************************************************************/

/* SUPPLIMENTAL DATA TO PASS TO DoCDG() */

struct CDGSupp 
{
	struct PlayerPrefsBase	*ppb;
	struct BitMap			*bm;
	struct BMInfo			*bmi;
	
	LONG					sprsize;
	UWORD					*sprdata;
	UWORD					sprcolors[8];
	UWORD					*sprloc[64];
};