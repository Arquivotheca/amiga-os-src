#include "prepcard.h"

#ifndef WA_NewLookMenus
#define WA_NewLookMenus         (WA_Dummy + 0x30)
#endif

struct Window *OpenAWindow( struct cmdVars *cv, ULONG tags, ... )
{
	return(OpenWindowTagList(NULL,(struct TagItem *)&tags));
}
 
BOOL MakeWindow( struct cmdVars *cmv )
{
register struct cmdVars *cv;


	cv = cmv;


	if(cv->cv_win = OpenAWindow(cv,
		WA_Title,	GetString(cv,MSG_PREP_TITLE),
		WA_InnerWidth,	620L,
		WA_InnerHeight,	150L,
		WA_Flags,	WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_DRAGBAR|WFLG_DEPTHGADGET,
		WA_IDCMP,	SLIDERIDCMP|IDCMP_GADGETUP|IDCMP_MENUPICK,
		WA_AutoAdjust,	(BOOL)TRUE,
		WA_PubScreen,	cv->cv_sp,
		WA_NewLookMenus, (BOOL)TRUE,
		TAG_DONE))
	{

		SetUpWindow( cv, cv->cv_win, cv->cv_gadgets );

		return(TRUE);
	}
	return(FALSE);
}

VOID CloseAdvWindow( struct cmdVars *cv )
{

	if(cv->cv_IsAdvanced)
	{

		CloseWindow(cv->cv_awin);
		FreeGadgets(cv->cv_advgadgets);
		cv->cv_IsAdvanced = FALSE;
	}

}

VOID SetUpWindow( struct cmdVars *cv, struct Window *win, struct Gadget *gad )
{

register struct RastPort *rp;

		rp = win->RPort;

		SetFont(rp,cv->cv_font);
		SetAPen(rp,TEXTPEN);
		SetBPen(rp,DETAILPEN);
		SetDrMd(rp,JAM2);

		AddGList(win,gad,-1,-1,NULL);
		RefreshGList(gad,win,NULL,-1);
		GT_RefreshWindow(win,NULL);


}
BOOL MakeAdvWindow( struct cmdVars *cmv )
{
register struct cmdVars *cv;

	cv = cmv;


	if(cv->cv_awin = OpenAWindow(cv,
		WA_Title,	GetString(cv,MSG_PREP_ADV_TITLE),
		WA_InnerWidth,	620L,
		WA_InnerHeight,	150L,
		WA_Flags,	WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_DRAGBAR|WFLG_DEPTHGADGET,
		WA_IDCMP,	CYCLEIDCMP|SLIDERIDCMP,
		WA_AutoAdjust,	(BOOL)TRUE,
		WA_PubScreen,	cv->cv_sp,
		TAG_DONE))
	{

		SetUpWindow( cv, cv->cv_awin, cv->cv_advgadgets );

		return(TRUE);
	}
	return(FALSE);
}
