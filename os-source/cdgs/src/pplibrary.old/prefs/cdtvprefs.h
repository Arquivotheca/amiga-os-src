#ifndef CDTVPREFS_H
#define CDTVPREFS_H
/*
** CDTV Preferences Include File
**
**	Copyright (c) 1991 Commodore Electronics Ltd.
**	All rights reserved. Confidential and Proprietary.
**	CDTV is a trademark of Commodore Electronics Ltd.
**
**	Written by: Carl Sassenrath
**	            Sassenrath Research, Ukiah, CA
**
**	Version of: March 10, 1991
**	Updated by: William Ware, Silent Software, Inc.
**	            Added new pref flags and Korean language.
*/

/*
** CDTV Preferences Bookmark ID
**
**	Used to access CDTV preferences via "bookmark.device".
*/
#define	BID_CDTVPREFS	0x00010001

/*
** CDTV Preferences Structure
**
**	This is the primary CDTV preferences structure.
**	It is found in the bookmark identified above and
**	is only accessed through the bookmark interface.
**
**	NOTE:
**	Should other preferences be added to the system in the
**	future, they will be created by adding new bookmark IDs
**	(so they will not affect the size of this structure).
*/
struct CDTVPrefs
{
	WORD 	DisplayX;	/* Default display View offset	*/
	WORD	DisplayY;	/* Default display View offset	*/
	UWORD	Language;	/* Human interface language	*/
	UWORD	AudioVol;	/* Default audio volume		*/
	UWORD	Flags;		/* Preference flags		*/
	UBYTE	SaverTime;	/* In Minuites 			*/
	UBYTE	Reserved;	/* Future function		*/
};

/*
** CDTV Preference Flags
*/
#define	CDTVPB_AUDIOVOL	0	/* Audio volume control	enabled	*/
#define CDTVPB_AMPM	1	/* Clock AM/PM option		*/
#define CDTVPB_KEYCLICK	2	/* 'Click' when key is pressed  */
#define CDTVPB_LACE	3	/* Screen is always in Interlace*/

#define	CDTVPF_AUDIOVOL	0x01
#define CDTVPF_AMPM	0x02
#define CDTVPF_KEYCLICK	0x04
#define CDTVPF_LACE	0x08

/*
** CDTV Human Language Defines
*/
#define	CDTVLANG_UNKNOWN	0
#define	CDTVLANG_AMERICAN	1	/* American English	*/
#define	CDTVLANG_ENGLISH	2	/* British English	*/
#define	CDTVLANG_GERMAN		3
#define	CDTVLANG_FRENCH		4
#define	CDTVLANG_SPANISH	5
#define	CDTVLANG_ITALIAN	6
#define	CDTVLANG_PORTUGUESE	7
#define	CDTVLANG_DANISH		8
#define	CDTVLANG_DUTCH		9
#define	CDTVLANG_NORWEGIAN	10
#define	CDTVLANG_FINNISH	11
#define	CDTVLANG_SWEDISH	12
#define	CDTVLANG_JAPANESE	13
#define	CDTVLANG_CHINESE	14
#define	CDTVLANG_ARABIC		15
#define	CDTVLANG_GREEK		16
#define	CDTVLANG_HEBREW		17
#define	CDTVLANG_KOREAN		18


#endif	/* CDTVPREFS_H */
