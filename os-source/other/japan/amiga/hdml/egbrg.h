/*
 * $Header: egbrg.h,v 1.6 90/01/16 19:28:02 yamaguti Exp $
 */

/********************************************************************/
/*                                                                  */
/*	EGBrg.h        library for EGBridge V2.10                   */
/*                                                                  */
/*	                Nakamura  1987.1.21  	                    */
/*                                                                  */
/*     (C) Copyright 1985,1986 Ergo Soft Corp.                      */
/*     All Rights Reserved					    */
/*								    */
/*                    --- NOTE ---				    */
/*								    */
/*  THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*  TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*  WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*								    */
/********************************************************************/

#ifndef	Egbrg_Inc_Defined

#include	"edef.h"
#include	"egckcnv.h"
#include	"hdmlcvt.h"
#include	"egbctype.h"

/*----- max definition -----*/

#define	  MAXINPUT	1024

/*----- element buffer (ebp) definition -----*/

#define	  EBPINC	3	/* ebp increment	*/
#define	  EBPHENKAN	0	/* ebp henkan		*/
#define	  EBPGO		1	/* ebp length of after	*/
#define	  EBPMAE	2	/* ebp length of before	*/

#define	  EUC		1	/* EUC code input mode		*/
#define	  JIS		2	/* JIS code input mode		*/
#define	  SJIS		3	/* SJIS code input mode		*/
#define	  KUTEN		4	/* KUTEN code input mode	*/

#define   EGB_EV_END		0x100	/* end(^X)		*/
#define   EGB_EV_RETURN		0x101	/* return(^M)		*/
#define   EGB_EV_BACKSPACE	0x102	/* backspace(^H)	*/
#define   EGB_EV_DELETE		0x103	/* delete(^D)		*/
#define   EGB_EV_MOVERIGHT	0x104	/* move right(^F)	*/
#define	  EGB_EV_MOVELEFT	0x105	/* move left (^B)	*/
#define   EGB_EV_MOVEUP		0x106	/* move up(^A)		*/
#define   EGB_EV_MOVEDOWN	0x107	/* move down(^E)	*/
#define   EGB_EV_STRETCH	0x108 	/* shift-right(^O)	*/
#define   EGB_EV_SHORTEN	0x109 	/* shift-left(^I)	*/
#define   EGB_EV_PREVIOUS	0x10a 	/* shift-up(^P)		*/
#define   EGB_EV_NEXT		0x10b 	/* shift-down(^N)	*/
#define   EGB_EV_CHANGEMODEE	0x10c 	/* change mode English(^Y)*/
#define   EGB_EV_CHANGEMODEJ	0x10d 	/* change mode Japanese(^U)*/
#define   EGB_EV_MENU		0x10e 	/* menu mode(^T)	*/
#define   EGB_EV_CODESW		0x10f 	/* code sw(^R)		*/
#define   EGB_EV_CONVERT	0x110	/* henkan(^W)		*/
#define   EGB_EV_ALLCLEAR	0x111 	/* all clear(^K)	*/
#define   EGB_EV_CODECONVERT	0x112 	/* code convert(^C)	*/
#define   EGB_EV_REDRAW		0x113 	/* redraw FEP_line(^L)	*/
#define   EGB_EV_RESIZE		0x114 	/* Resize.		*/
#define   EGB_EV_ESC		0x1b 	/* esc			*/

/*----- on - off of switch -----*/

#define	  ON		1	/* on	*/
#define   OFF		0	/* off	*/
#define	  HALT		-1	/* halt	*/

#define	  CDMAX		7	/* max size of code */

#define	  JHANKAKU	0	/* code convert hankaku		 */
#define	  JHIRAGANA	1	/* code convert zenkaku.hiragana */
#define	  JKATAKANA	2	/* code convert zenkaku.katakana */

/*----- EUC size -----*/

#define	EUC0SIZ	2
#define	EUC1SIZ	4
#define	EUC2SIZ	4
#define	EUC3SIZ	6
#define JISSIZE	4

/*----- codeshu -----*/

#define	EUC0	0
#define	EUC1	1
#define	EUC2	2
#define	EUC3	3
#define	EUC4	4

/*----- Group of error(message) code. -----*/

#define EGC_ERR	1
#define EGB_ERR	2

/*----- Message code. -----*/

#define EGB_M01	1
#define EGB_M02	2
#define EGB_M03	3

/*----- structure of EGBridge status -----*/

typedef struct {
	int	innerst;	/* inner status			*/
	int	codesw;		/* code switch (ON,OFF)		*/
	int	kanasw;		/* kana switch (ON,OFF)		*/
	int	eijimod;	/* eiji mode			*/
	int	kanamod;	/* kana mode			*/
	int	codemod;	/* code mode			*/
	int	menupag;	/* menu page (1,2)		*/
	int	jiyuusw;	/* jiyuu-ikkatu sw (ON,OFF)	*/
	int	recvtsw;	/* re-convert sw (ON,OFF)	*/
	int	agesw;		/* age sw (ON,OFF)		*/
	int	commasw;	/* Comma  & period switch (ON,OFF). */
	int	errflag;	/* Error flag (OFF,EGCERR,EGBERR). */
	int	errcode;	/* Error code. */
}   EGB_ST;

/*----- inner status -----*/

#define	ST_INPUT	1	/* input mode		*/
#define	ST_HOMMO	2	/* hommony select mode	*/
#define	ST_MENUE	3	/* menu mode		*/
#define	ST_ADDWD	4	/* add word mode	*/
#define	ST_CODE		5	/* code mode		*/
#define	ST_PRE		6	/* pre-addword mode	*/
#define	ST_DELWD	7	/* delete word mode	*/
#define	ST_SDIC		8	/* Special dic. mode	*/

/*----- eiji,kana,code mode -----*/

#define	E_ROMAJ	1	/* romaji (zenkaku hiragana)	*/
#define	E_ZENKA	2	/* zenkaku eisuu_kigou		*/
#define	E_HANKA	3	/* hankaku eisuu_kigou		*/
#define	K_HIRA	4	/* zenkaku hiragana		*/
#define	K_ZENKA	5	/* zenkaku katakana		*/
#define	K_HANKA	6	/* hankaku katakana		*/
#define	C_JIS	7	/* JIS code			*/
#define	C_SJIS	8	/* SJIS code			*/
#define	C_KUTEN	9	/* KUTEN code			*/
#define	C_EUC	10	/* EUC code			*/

/*----- terminal & application code (pty/defs.h) -----*/

#define	TA_EUC	4	/* == T_EUC */

/*----- length -----*/

#define	M_YOMI	12	/* max length of yomi (hankaku) */
#define	M_YOMI2	24	/* max length of yomi (zenkaku) */
#define	M_KANJ	8	/* max length of kanji (characters) */
#define	M_KANJ2	16	/* max length of kanji (bytes) */
#define	M_WORDS	100	/* max words of same reading */
#define M_CONVL	40	/* Max non_converted characters length. */

/*----- For EGB_CONTEXT in "egb.h". -----*/

/*
data area for mainly fep control
*/

typedef
struct {
	/*
	fepmain inner variables
	*/
	char	sbinf[MAXCHN+1];	/* screen buffer information */
	int	start;	/* sb start for addword		*/
	int	end;	/* sb end for addword		*/
	UBYTE	jis[CHARLEN+1];			/* */
	UBYTE	coderaw[CHARS+1][CHARLEN];	/* */
	int	codecnt;	/* number of code input */
	int	coder[CDMAX];	/* raw of code input */
	char	incode[CDMAX];	/* Raw of input_code. */
	char	outcode[CDMAX*2];	/* output code for code input */
	int	codeshu;	/* kind of code(EUC0,1,2,3,other) */
	/*
	bridge globals
	*/
	int	jishu;		/* kind of chara in code-convert */
	char	npflag;		/* Has nextprev called ?	*/
	/* Code select mode. */
	UBYTE	jisrang[2];	/* Jis 2nd byte range to display. */
				/* 0 --- start			  */
				/* 1 --- end			  */
	/* Word_delete command. */
	char	d_status;
	UBYTE	d_yomi[M_YOMI2+1];
	UBYTE	d_words[M_WORDS][M_KANJ2+1];
	int	d_nwords;
	int	d_cwords;
	/* Menu. */
#define NUM_PAGE	2
#define NUM_MENU1	5
#define NUM_MENU2	3
	char	menu[NUM_PAGE][NUM_MENU1];
	/* Expert dictionaries select mode */
	char	expdic[3];	/* Information of expert dictionary. */
				/* 0 --- total number		     */
				/* 1 --- start no. to display	     */
				/* 2 --- end no. to display	     */
} EGB_BRIDGE;

/*
	add word to dictionay
*/
typedef
struct {
	/* touroku executing status */
	int	executing_status;
	/* string buffer for touroku operation */
#define	TOUROKU_MAXINPUT	16
	BYTE	ystr[TOUROKU_MAXINPUT*2];	/* yomi */
	BYTE	kstr[TOUROKU_MAXINPUT+1];	/* kanji */
	UBYTE	src[TOUROKU_MAXINPUT];	/* source */
	WORD	lystr, lkstr;	/* length of string */
	WORD	kstart,kend;	/* start,end position of kstr */
	int	hlength;	/* length of hiragana */
	UWORD	length;		/* length of kanji */
	/* kind of word */
#define	TOUROKU_KINDS_NUM	5	/* how many kinds. */
	WORD	kinds[TOUROKU_KINDS_NUM];	/* hinshi */
	char	hinsi_d[TOUROKU_KINDS_NUM];	/* Hinshi to display. */
} EGB_ADDWD;

/*----- Code select mode -----*/

#define MV_NEXT	1
#define MV_PREV	-1
#define STT_JIS	egb->bridge.jisrang[0]	/* Start JIS code. */
#define END_JIS	egb->bridge.jisrang[1]	/* End JIS code. */

#define MX_IN_P	9	/* Max number of selected item in a page. */

#define	Egbrg_Inc_Defined
#endif	Egbrg_Inc_Defined

