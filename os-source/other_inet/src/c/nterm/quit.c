/*****************************************************************************
*Quit requester
*****************************************************************************/

#include "st.h"
#include "colors.h"

#define QUITWIDTH 196
#define QUITHEIGHT 60

char QuitStr[] = "Quit: Are You Sure?", PosStr[] = "Yes", NegStr[] = "No";

struct IntuiText QuitText = { /* text for quit title */
	COLOR0,COLOR1,JAM1,0,12,NULL,QuitStr,NULL};

struct IntuiText QuitPosText = { /* text for positive response */
	COLOR0,COLOR1,JAM1,6,3,NULL,PosStr,NULL};

struct IntuiText QuitNegText = { /* text for negative response */
	COLOR0,COLOR1,JAM2,6,3,NULL,NegStr,NULL};

QuitReq(w)
struct Window *w;
{
	QuitText.LeftEdge = (QUITWIDTH-strlen(QuitStr)*8)/2-8;
	return(AutoRequest(w,&QuitText,&QuitPosText,&QuitNegText,NULL,NULL,QUITWIDTH,QUITHEIGHT));
}
