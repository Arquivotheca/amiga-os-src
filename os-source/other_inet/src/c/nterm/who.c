/*****************************************************************************
*Who requester
*****************************************************************************/

#include "st.h"
#include "colors.h"

#define WHOWIDTH 224
#define WHOHEIGHT 96

char WhoStr[] = "AmigaTerm", WhoNegStr[] = "Continue";
char WhoStr2[] = "Network", WhoStr3[] = "Version 1.1";
/* char WhoStr2[] = "by", WhoStr3[] = "David Berezowski"; */
char WhoStr4[] = "Copyright c 1990", WhoStr5[] = "Commodore-Amiga, Inc.";

struct IntuiText WhoText5 = { /* Commodore-Amiga, Inc. */
	COLOR0,COLOR1,JAM1,0,52,NULL,WhoStr5,NULL};

struct IntuiText WhoText4= { /* Copyright c 1986 */
	COLOR0,COLOR1,JAM1,0,40,NULL,WhoStr4,&WhoText5};

struct IntuiText WhoText3 = { /* David Berezowski */
	COLOR0,COLOR1,JAM1,0,28,NULL,WhoStr3,&WhoText4};

struct IntuiText WhoText2 = { /* by */
	COLOR0,COLOR1,JAM1,0,16,NULL,WhoStr2,&WhoText3};

struct IntuiText WhoText = { /* AmigaTerm c */
	COLOR0,COLOR1,JAM1,0,4,NULL,WhoStr,&WhoText2};

struct IntuiText WhoNegText = { /* text for positive response */
	COLOR0,COLOR1,JAM2,6,3,NULL,WhoNegStr,NULL};

WhoReq(w)
struct Window *w;
{
	WhoText.LeftEdge = (WHOWIDTH-strlen(WhoStr)*8)/2-8;
	WhoText2.LeftEdge = (WHOWIDTH-strlen(WhoStr2)*8)/2-8;
	WhoText3.LeftEdge = (WHOWIDTH-strlen(WhoStr3)*8)/2-8;
	WhoText4.LeftEdge = (WHOWIDTH-strlen(WhoStr4)*8)/2-8;
	WhoText5.LeftEdge = (WHOWIDTH-strlen(WhoStr5)*8)/2-8;
	WhoStr4[10] = 0xA9; /* copyright symbol */
	AutoRequest(w,&WhoText,NULL,&WhoNegText,NULL,NULL,WHOWIDTH,WHOHEIGHT);
}
