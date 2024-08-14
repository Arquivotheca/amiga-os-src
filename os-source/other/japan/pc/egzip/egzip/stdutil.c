/**********************************************************************/
/***																***/
/***	StdUtil.c : Convert Address string to Zip Code module		***/
/***																***/
/**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egzip.h"


/*******************************************/
/*** strcat() : String catenate function ***/
/*******************************************/
void a2z_strcat(dst, src)
	UBYTE*	dst;
	UBYTE*	src;
{
	while (*dst)
		++dst;
	while (*src)
		*dst++ = *src++;
	*dst = '\0';
} 

/**************************************************/
/*** strncmp() : Count string compaire function ***/
/**************************************************/
WORD a2z_strncmp(dst, src, n)
	UBYTE*	dst;
	UBYTE*	src;
	WORD		n;
{
	while (n--) {
		if (*dst != *src)
			return(*dst - *src);
		++dst;
		++src;
	}
	return(0);
}

/* page */
/*******************************************/
/*** strcmp() : String compaire function ***/
/*******************************************/
/*
WORD a2z_strcmp(dst, src)
	UBYTE*	dst;
	UBYTE*	src;
{
	while ((*src == *dst) && *src) {
		++dst;
		++src;
	}
	return(*dst - *src);
}
*/
/*******************************************/
/*** memcmp() : Memory compaire function ***/
/*******************************************/

WORD a2z_memcmp(dst, src, n)
	UBYTE*	dst;
	UBYTE*	src;
	WORD		n;
{
	register WORD	d;
	
	while (n--) {
		d = *dst++ - *src++;
		if (d)
			return(d);
	}
	return(d);
}



