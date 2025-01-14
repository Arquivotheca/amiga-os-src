#include "prepcard.h"

#ifndef GTNN_NewLookMenus
#define GTMN_NewLookMenus    GT_TagBase+67
#endif

static struct NewMenu menus[MSG_PREP_QUIT_MENU - MSG_PREP_TITLE_MENU + 2] = {
	{ NM_TITLE,	(STRPTR)MSG_PREP_TITLE_MENU,	NULL,				0,	0L,	(APTR)CM_DEFAULT },
	{ NM_ITEM,	(STRPTR)MSG_PREP_ADV_MENU,	NULL,				0,	0L,	(APTR)CM_ADV },
	{ NM_ITEM,	(STRPTR)MSG_PREP_QUIT_MENU,	(STRPTR)MSG_PREP_QUIT_COM,	0,	0L,	(APTR)CM_QUIT },
	{ NM_END,	NULL,				NULL,				0,	0L,	NULL },
};

struct Menu *CreateAMenuStrip( struct cmdVars *cv, struct NewMenu *nm, struct TagList *tags )
{
	return(CreateMenusA(nm, tags));
}

BOOL DoLayoutMenus( struct cmdVars *cv, struct Menu *menu, ULONG tags, ... )
{
	return(LayoutMenusA(menu,cv->cv_VI,(struct TagList *)&tags));
}

BOOL MakeBegMenu( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct NewMenu *md,*ms;
int loop;
struct	NewMenu	TempMenu[MSG_PREP_QUIT_MENU - MSG_PREP_TITLE_MENU + 2];


	cv = cmv;

	for(loop = MSG_PREP_TITLE_MENU-MSG_PREP_TITLE_MENU;
		loop < (MSG_PREP_QUIT_MENU-MSG_PREP_TITLE_MENU + 2);
		loop++)
	{
		ms = &menus[loop];
		md = &TempMenu[loop];

		md->nm_Type = ms->nm_Type;

		md->nm_Label = ms->nm_Label;

		if(ms->nm_Label)
		{
			md->nm_Label = GetString(cv,(ULONG)ms->nm_Label);
		}
	

		md->nm_CommKey = ms->nm_CommKey;

		if(ms->nm_CommKey)
		{
			md->nm_CommKey = GetString(cv,(ULONG)ms->nm_CommKey);
		}
	
		md->nm_Flags = ms->nm_Flags;
		md->nm_MutualExclude = ms->nm_MutualExclude;
		md->nm_UserData = ms->nm_UserData;

	}

	if(cv->cv_begmenu = CreateAMenuStrip(cv,&TempMenu[0],TAG_DONE))
	{
		if(DoLayoutMenus(cv,cv->cv_begmenu,GTMN_NewLookMenus,(BOOL)TRUE,TAG_DONE))
		{
			SetMenuStrip(cv->cv_win,cv->cv_begmenu);
			return(TRUE);
		}
		FreeMenus( cv->cv_begmenu );
	}
	return(FALSE);
}
