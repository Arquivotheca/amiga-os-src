/**********************************************************************/
/***																***/
/***	EGZipAtoZ.c : Convert Address string to Zip Code module		***/
/***																***/
/**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egzip.h"

/***********************/
/*** TYPE DEFINITION ***/
/***********************/

typedef struct {		/* Address String Separate Structure */
	short		start;	/* Separate start position in Original address string*/
	short		len;	/* Separated word length (in byte order)*/
	short		rr;		/* Address suffix code*/
} A2Z_SEP;

typedef struct {		/* Address Suffix Structure */
	UBYTE	str[14];	/* Address suffix string */
	WORD	len;		/* Suffix word length */
} A2Z_RR;

typedef struct {		/* Input Address Record Structure */
	WORD	kind;		/* Address kind (rid) */
	UWORD	vid[3];		/* VID array (1..3) */
} A2Z_ZIP;

typedef	struct {
	short		ptn;
	short		flag;
}A2Z_PTN;


/************************/
/*** MACRO DEFINITION ***/
/************************/

/* Condition of my routine */
#define	CND_FIRST	0	/* No search mode (pre-call) */
#define	CND_NEXT	1	/* Now search */

#define false		0
#define true		1

/* Max separate count */
#define	SEP_MAX		10

/* page */
/***************************/
/*** VARIABLE DEFINITION ***/
/***************************/

short	a2z_cnd = CND_FIRST;	/* Routine condition */
short	a2z_sepcnt;				/* Separate count */
short	a2z_vlcnt;				/* VID List count */
short	a2z_zlcnt;				/* ZIP code List count */
UWORD	a2z_crrblk;				/* Current Read block */
UBYTE	a2z_orgaddr[256];		/* Original address string */
A2Z_SEP	a2z_seplist[SEP_MAX];	/* Separate list */
UWORD	a2z_vidlist[55];		/* Input VID list */
A2Z_ZIP	a2z_ziplist[176];		/* Input ZIP code list */
A2Z_ZIP	a2z_zipcode[ 200];		
A2Z_PTN	a2z_zipptn[ 176];
short	a2z_ptncnt;
short	a2z_zipcnt;
ULONG	a2z_zip[ 200];
short	a2z_makelist;

static A2Z_RR	a2z_rrlist[14] = {
					"ïÄäÅæöá",0x08,
					"ÄûûâZâôâ^ü[",0x0c,
					"ï",0x02,
					"ôs",0x02,
					"ô",0x02,
	                "ò{",0x02,
					"îº",0x02,
					"Äs",0x02,
					"îS",0x02,
					"ÄxÆí",0x04,
					"ïµ",0x02,
					"Æ¼",0x02,
					"æ",0x02,
					"ïµ",0x02,   };			/* Suffix word list */

/***************************************/
/*** FUNCTION PROTO-TYPE DECLARATION ***/
/***************************************/
#ifndef	UNIX
/* (StdUtil.c) */
VOID	a2z_strcat(UBYTE *, UBYTE *);
short	a2z_strncmp(UBYTE *, UBYTE *, short);
short	a2z_memcmp(UBYTE *, UBYTE *, short);

/*
VOID	a2z_readRR();
*/
/* Local */
VOID	a2z_strncpy(UBYTE *, UBYTE*, short);
short	a2z_memcmpin(short, UBYTE *, short, UBYTE *);
VOID	a2z_searchIndex(PZIPM);
short	a2z_separateString(UBYTE *);
VOID	a2z_makeVidList(PZIPM);
VOID	a2z_makeZipList(VOID);
VOID	a2z_makeVidStr(PZIPM, UWORD, UBYTE *);
VOID	a2z_makeAddrStr(PZIPM, UBYTE *, short);
short	a2z_getIdx(short, short);
UWORD	a2z_Str2Vid(PZIPM, UBYTE *);
VOID	a2z_Vid2Str(PZIPM, UWORD, UBYTE *);
short	a2z_makeziplistptn( A2Z_ZIP *, short);
VOID	a2z_remakeziplist( A2Z_ZIP *, short, short, short);
VOID	a2z_makezipcode( UBYTE *, short);
VOID	a2z_remakezipno(VOID);
VOID	a2z_remakezipcode( short, short);
short	a2z_repcheck( A2Z_ZIP *, short);

extern	UBYTE *getZlTable(PZIPM, UWORD);
extern	UBYTE *getVlTable(PZIPM, UWORD);
#else
/* (StdUtil.c) */
VOID	a2z_strcat();
short	a2z_strncmp();
short	a2z_memcmp();

/*
VOID	a2z_readRR();
*/
/* Local */
VOID	a2z_strncpy();
short	a2z_memcmpin();
VOID	a2z_searchIndex();
short	a2z_separateString();
VOID	a2z_makeVidList();
VOID	a2z_makeZipList();
VOID	a2z_makeVidStr();
VOID	a2z_makeAddrStr();
short	a2z_getIdx();
UWORD	a2z_Str2Vid();
VOID	a2z_Vid2Str();
short	a2z_makeziplistptn();
VOID	a2z_remakeziplist();
VOID	a2z_makezipcode();
VOID	a2z_remakezipno();
VOID	a2z_remakezipcode();
short	a2z_repcheck();

extern	UBYTE *getZlTable();
extern	UBYTE *getVlTable();
#endif
/* page */
/**********************************************/
/*** strncpy() : Count string copy function ***/
/**********************************************/
VOID a2z_strncpy(dst, src, n)
	UBYTE*	dst;
	UBYTE*	src;
	short	n;
{
	while (*src && (n--))
		*dst++ = *src++;
	*dst = '\0';
}

/************************************************/
/*** memcmpin() : Memory compaire function II ***/
/************************************************/
short a2z_memcmpin(n1, ptr1, n2, ptr2)
	short	n1;
	UBYTE*	ptr1;
	short	n2;
	UBYTE*	ptr2;
{
	register short		n, len, r, i;
	register UBYTE		*p, *p1, *p2;
	
	if( n1 == n2)
		return(a2z_memcmp(ptr1, ptr2, n1 * 2));
	else if( n1 > n2)
	{
		p1 = ptr1;
		p2 = ptr2;
		n = n1 - n2 + 1;
		len = n2;
	}else
		return(1);
	for(i = 0, p = p1; i < n; i++, p += 2){
		if(*p == *p2){
			if(!(r = a2z_memcmp(p, p2, len * 2)))
				return(r);
		}
	}
	return(1);
}

/* page */

/***********************************************************/
/*** separateString() : Separate address string function ***/
/***********************************************************/
short	a2z_separateString(ptr)
	UBYTE*	ptr;
{
	register short	i, len, st, idx, rr;
	UBYTE			find;

	for (st = len = idx = 0; *ptr;) {
		for (i = 0, find = false; (i < 14) && (find == false); ++i) {
			if (a2z_strncmp(ptr, a2z_rrlist[i].str, a2z_rrlist[i].len) == 0) {
				rr = i;
				find = true;
			}
		}
		if (find) {
			a2z_seplist[idx].start = st;
			a2z_seplist[idx].len = len;
			a2z_seplist[idx].rr = rr;
			ptr += a2z_rrlist[rr].len;
			st += (len + a2z_rrlist[rr].len);
			len = 0;
			++idx;
			if (idx == SEP_MAX)
				break;
		} else {
			++ptr;
			++len;
		}
	}
	return(idx);
}

/* page */
/***************************************************************/
/*** makeVidList() : Make VID List from input address string ***/
/***************************************************************/
VOID	a2z_makeVidList(pzipm)
	PZIPM	pzipm;
{
	UWORD			*vptr = a2z_vidlist, vid;
	register short	n, i, stpos, len;
	UBYTE			str[256];
	
	for (stpos = n = 0; n < a2z_sepcnt; ++n) {
		for (i = len = 0; i < (a2z_sepcnt - n); ++i) {
			len += a2z_seplist[n + i].len;
			a2z_strncpy(str, &a2z_orgaddr[stpos], len);
			len += a2z_rrlist[a2z_seplist[n + i].rr].len;
			if (vid = a2z_Str2Vid(pzipm, str))
				*vptr++ = ((vid << 4) + a2z_seplist[n + i].rr + 1);
			else
				*vptr++ = 0;
		}
		stpos += (a2z_seplist[n].len + a2z_rrlist[a2z_seplist[n].rr].len);
	}
	a2z_vlcnt = vptr - a2z_vidlist;
}

/* page */
/*****************************************/
/*** Str2Vid() : Convert string to VID ***/
/*****************************************/
UWORD	a2z_Str2Vid(pzipm, ptr)
	PZIPM	pzipm;
	UBYTE*	ptr;
{
	register short	cmp, len, i, sl;
	register UWORD	vid, blk;
	UBYTE			*idx, *tbl;

	if (!(sl = strlen(ptr)))
		return(0);
	idx = pzipm->vlIndex;
	for (blk = i = 0; i < 297;) {
		vid = (*idx++ << 4);
		vid += (*idx >> 4);
		if (!vid)
			break;
		len = (*idx++ & 0x0f) * 2;
		cmp = a2z_strncmp(ptr, idx, sl > len ? sl : len);
		if (cmp < 0)
			break;
		idx += len;
		blk = *idx++;
		i += (len + 3);
		if (cmp == 0)
			return(vid);
	}
	tbl = getVlTable(pzipm, blk);
	for (i = 0; i < 1022;) {
		vid = (*tbl++ << 4);
		vid += (*tbl >> 4);
		if (!vid)
			break;
		len = (*tbl++ & 0x0f) * 2;
		if (!a2z_strncmp(tbl, ptr, sl > len ? sl : len))
			return(vid);
		tbl += len;
		i += (len + 2);
	}
	return(0);
}

/* page */
/*****************************************/
/*** Vid2Str() : Convert VID to string ***/
/*****************************************/
VOID	a2z_Vid2Str(pzipm, tvid, str)
	PZIPM	pzipm;
	UWORD	tvid;
	UBYTE*	str;
{
	register short	len, i;
	register UWORD	vid, blk;
	UBYTE			*idx, *tbl;

	idx = pzipm->vlIndex;
	for (blk = i = 0; i < 297;) {
		vid = (*idx++ << 4);
		vid += (*idx >> 4);
		if (!vid)
			break;
		len = (*idx++ & 0x0f) * 2;
		if (tvid == vid) {
			a2z_strncpy(str, idx, len);
			return;
		} else if (tvid < vid)
			break;
		idx += len;
		blk = *idx++;
		i += (len + 3);
	}
	tbl = getVlTable(pzipm, blk);
	for (i = 0; i < 1022;) {
		vid = (*tbl++ << 4);
		vid += (*tbl >> 4);
		if (!vid)
			break;
		len = (*tbl++ & 0x0f) * 2;
		if (tvid == vid) {
			a2z_strncpy(str, tbl, len);
			return;
		}
		tbl += len;
		i += (len + 2);
	}
	*str = '\0';
}

/* page */
/***********************************************************************/
/*** getIdx() : Caliculate VidList index from start(n) and length(i) ***/
/***********************************************************************/
short a2z_getIdx(n, i)
	short	n;
	short	i;
{
	register short	p;

	if ((n >= a2z_sepcnt) || ((a2z_sepcnt - n) <= i))
		return(-1);
	for (p = 0; n; --n)
		p += (a2z_sepcnt - n + 1);
	return(p + i);
}

/* page */
/********************************************/
/*** makeZipList() : Make ZIP number list ***/
/********************************************/
VOID a2z_makeZipList()
{
	register short	i, i2, i3;
	short				n, idx;
	UWORD			vid1, vid2, vid3;
	A2Z_ZIP			ziplist[ 176];
	
	for (i = 0; i < 176; ++i) {
		ziplist[i].kind = 0;
		ziplist[i].vid[0] = 0x00;
		ziplist[i].vid[1] = 0x00;
		ziplist[i].vid[2] = 0x00;
	}
	for (idx = i = 0; i < a2z_sepcnt; ++i) {
		if (((n = a2z_getIdx(0, i)) >= 0) && (vid1 = a2z_vidlist[n])) {
			if (((vid1 & 0x0f) < 4) || ((vid1 & 0x0f) > 7)) {
				ziplist[idx].kind = 1;
				ziplist[idx].vid[0] = vid1;
				++idx;
			}
			if ((i + 1) < a2z_sepcnt) {
				for (i2 = 0; (i + i2 + 1) < a2z_sepcnt; ++i2) {
					if (((n = a2z_getIdx(i + 1, i2)) >= 0) &&
										(vid2 = a2z_vidlist[n])) {
						if ((vid1 & 0x0f) < (vid2 & 0x0f)) {
							ziplist[idx].kind = 2;
							ziplist[idx].vid[0] = vid1;
							ziplist[idx].vid[1] = vid2;
							++idx;
						}
						if ((i + i2 + 2) < a2z_sepcnt) {
							for (i3 = 0; (i + i2 + i3 + 2) < a2z_sepcnt; ++i3) {
								if (((n = a2z_getIdx(i + i2 + 2, i3)) >= 0) &&
										(vid3 = a2z_vidlist[n]) &&
										((vid1 & 0x0f) < (vid2 & 0x0f)) &&
											((vid2 & 0x0f) < (vid3 & 0x0f))) {
									ziplist[idx].kind = 3;
									ziplist[idx].vid[0] = vid1;
									ziplist[idx].vid[1] = vid2;
									ziplist[idx].vid[2] = vid3;
									++idx;
								}
							}
						}
					}
				}
			}
		}
	}
	a2z_zlcnt = a2z_makeziplistptn( ziplist, idx);
}

/* page */
/***************************************************/
/*** makeVidStr() : Make address string from VID ***/
/***************************************************/
VOID a2z_makeVidStr(pzipm, vid, str)
	PZIPM	pzipm;
	UWORD	vid;
	UBYTE*	str;
{
	*str = '\0';
	a2z_Vid2Str(pzipm, vid >> 4, str);
	a2z_strcat(str, a2z_rrlist[(vid & 0x0f) - 1].str);
}

/* page */
/*******************************************/
/*** searchIndex() : Search address code ***/
/*******************************************/
VOID a2z_searchIndex(pzipm)
	PZIPM	pzipm;
{
	UBYTE			*tbl;
	UWORD			idx[3];
	register short	pos, i, kind;
	
	for (; a2z_crrblk < pzipm->zlCount;++a2z_crrblk) {
		tbl = getZlTable(pzipm, a2z_crrblk);
		for ( pos = 0; pos < 1020;) {
			kind = tbl[pos] >> 4;
			for (i = 0; i < kind; ++i) {
				if ((tbl[pos + 4 + i * 2] & 0x0f) == 14)
					tbl[pos + 4 + i * 2] -= 3;	
			}

			for (i = 0; i < kind; ++i) {
				idx[i] = (UWORD)tbl[pos + 3  + i * 2];
				idx[i] = idx[i] << 8;
				idx[i] += (UWORD)tbl[pos + 4 + i * 2];
			}

			for (i = 0; i < a2z_zlcnt; ++i) {
				if (a2z_memcmpin(kind, (UBYTE *)&idx,
						a2z_ziplist[i].kind , (UBYTE *)(&a2z_ziplist[i].vid[0])) == 0)
				{	
					a2z_makezipcode( &tbl[ pos], i);
					break;
				}
			}
			pos += ( 3 + kind * 2);
		}
	}
}	

/* page */
/****************************************************************/
/*** makeAddrStr() : Make address string from Zip code record ***/
/****************************************************************/
VOID a2z_makeAddrStr(pzipm, str, i)
	PZIPM	pzipm;
	UBYTE*	str;
	short	i;
{
	short		rid, ridc;
	short		ckflag;
	UBYTE		s[256];

	ckflag = 1;
	if( i != 0)
	{
		if( a2z_zipcode[ i - 1].kind == a2z_zipcode[ i].kind)
			ckflag = a2z_repcheck( a2z_zipcode, i - 1);
	}
	if( ckflag == 1)
	{
		rid = a2z_zipcode[ i].kind;
		*str = '\0';
		/* sprintf(str, "%03ld-%02ld ", zip / 100L, zip % 100L); */
		for( ridc = 0; ridc < rid; ridc++) {
			a2z_makeVidStr(pzipm, a2z_zipcode[ i].vid[ ridc], s);
			a2z_strcat(str, s);
		}
	}
}

/* page */
/*******************************************************/
/*** cnvAddrToZip() : Address to Zip Code MAIN entry ***/
/*******************************************************/
short	cnvAddrToZip(pzipm, addr, zipstr, zip)
	PZIPM	pzipm;
	UBYTE*	addr;
	UBYTE*	zipstr;
	ULONG*	zip;
{
	if (a2z_cnd == CND_FIRST) {
		strcpy(a2z_orgaddr, addr);
		a2z_ptncnt = 0;
		a2z_sepcnt = a2z_separateString(addr);
		a2z_makeVidList(pzipm);
		a2z_makeZipList();
		if (a2z_zlcnt) {
			a2z_crrblk = 0;
			a2z_zipcnt = 0;
			a2z_makelist = 0;
			a2z_cnd = CND_NEXT;
		}
		else
			return(NOFIND);
		a2z_searchIndex(pzipm);
		a2z_remakezipno();
	}
	if( a2z_makelist < a2z_zipcnt)
	{
		*zip = a2z_zip[ a2z_makelist];
		a2z_makeAddrStr(pzipm, zipstr, a2z_makelist);
		a2z_makelist++;
		return(FINDNEXT);
	}
	a2z_cnd = CND_FIRST;
	return( NOFIND);
}

/* Page */
/************************************************/
/*** makeziplistptn() : Make Zip List pattern ***/
/************************************************/
short	a2z_makeziplistptn(ziplist, idx)
	A2Z_ZIP*ziplist;
	short	idx;
{
	short		i, total;
	short		rid, ridc;
	short		rr, ckflag;
	
	total = 0;
	for( i = 0; i < idx; i++)
	{
		ckflag = a2z_repcheck( ziplist, i);
		if( ckflag == 0)
			i++;
		rid = ziplist[ i].kind;
		rr = ( ziplist[ i].vid[ 0] & 0x0F);
		if(( rr > 3) && ( rr < 8))
			rid--;
		for( ridc = 0; ridc < rid; ridc++)
		{
			a2z_remakeziplist( ziplist, i, total, ziplist[ i].kind - ridc);
			total++;
		}
		a2z_ptncnt++;
	}
	return( total);
}

/* Page */
/*****************************************/
/*** remakeziplist() : Remake Zip list ***/
/*****************************************/

VOID a2z_remakeziplist(ziplist, idx, total, rid)
	A2Z_ZIP*	ziplist;
	short		idx;
	short		total;
	short		rid;
{
	short i;
	
	for( i = 0; i < rid; i++)
	{
		a2z_ziplist[ total].vid[ i] = ziplist[ idx]. vid[ i];
	}
	a2z_ziplist[ total].kind = rid;
	a2z_zipptn[ total].ptn = a2z_ptncnt;
	a2z_zipptn[ total].flag = 0;
}

/* page */
/*************************************/
/*** makezipcode() : Make ZIP code ***/
/*************************************/

VOID a2z_makezipcode(tbl, idx)
	UBYTE*	tbl;
	short	idx;
{
	short 	i;
	
	a2z_zipcode[ a2z_zipcnt].kind = *tbl >> 4;
	a2z_zip[ a2z_zipcnt] = *tbl++ & 0x0F;
	a2z_zip[ a2z_zipcnt] = *tbl++ + ( a2z_zip[ a2z_zipcnt] << 8);
	a2z_zip[ a2z_zipcnt] = *tbl++ + ( a2z_zip[ a2z_zipcnt] << 8);
	for( i = 0; i < a2z_zipcode[ a2z_zipcnt].kind; i++)
	{
		a2z_zipcode[ a2z_zipcnt].vid[ i] = *tbl++;
		a2z_zipcode[ a2z_zipcnt].vid[ i] = 
				( a2z_zipcode[ a2z_zipcnt].vid[ i] << 8) + *tbl++;
	}
	a2z_zipcnt++;
	a2z_zipptn[ idx].flag = 1;
}

/* page */
/*****************************************/
/*** remakezipno() : ReMake ZIP Number ***/
/*****************************************/
VOID a2z_remakezipno()
{
	short		i, ptn, rid1, rid2;
	short		ck, idx, total;
	short		ckflag;
	
	total =  0;
	for( i = 0; i < a2z_zipcnt; i++)
	{
		idx = 0;
		for( ptn = 0; ptn < a2z_ptncnt; ptn++)
		{
			ckflag = 0;
			while(( a2z_zipptn[ idx].ptn == ptn) && ( ckflag == 0))
			{
				if( a2z_zipptn[ idx].flag == 1)
				{
					rid1 = a2z_zipcode[ i].kind;
					rid2 = a2z_ziplist[ idx].kind;
					ck = a2z_memcmpin( rid1, ( UBYTE *)( &a2z_zipcode[ i].vid[ 0]), 
						rid2, ( UBYTE *)( &a2z_ziplist[ idx].vid[ 0]));
					if( ck == 0)
					{
						a2z_remakezipcode( i, total);
						total++;
						break;
					}
					ckflag = 1;
				}
				idx++;
			}
		}
	}
	a2z_zipcnt = total;
}

/* page */
/*****************************************/
/*** remakezipcode() : ReMake ZIP code ***/
/*****************************************/

VOID a2z_remakezipcode(i, total)
	short	i;
	short	total;
{
	short		rid, ridc;
	
	a2z_zip[ total] = a2z_zip[ i];
	rid = a2z_zipcode[ i].kind;
	for( ridc = 0; ridc < rid; ridc++)
	{
		a2z_zipcode[ total].vid[ ridc] = a2z_zipcode[ i].vid[ ridc];
	}
}

/* page */
/******************************************/
/*** repcheck() : Check ZIP code repeat ***/
/******************************************/

short	a2z_repcheck(ziplist, idx)
	A2Z_ZIP*	ziplist;
	short		idx;
{
	short		i;
	
	for( i = 0; i < ziplist[ idx].kind; i++)
	{
		if( ziplist[ idx].vid[ i] != ziplist[ idx + 1].vid[ i])
			return( 1);
	}
	return( 0);
}

