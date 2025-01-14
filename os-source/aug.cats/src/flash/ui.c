#include "flash.h"

#define STATLEFT	210
#define STATTOP		112
#define STATWIDTH	380
#define STATHEIGHT	40

static STRPTR Sources[] = {
	"Trackdisk Unit 0",
	"Carddisk  Unit 0",
	"Ramdrive  Unit 0",
	"Binary File",
	NULL
};

static STRPTR Manufacturer[] = {
	"Intel FlashROM",
	NULL
};

ULONG far totalsizes[] = {
	64*1024,
	128*1024,
	256*1024,
	512*1024,
	1*1024*1024,
	2*1024*1024,
	4*1024*1024
};

static STRPTR TotalSize[] = {
	"64K",
	"128K",
	"256K",
	"512K",
	"1 MB",
	"2 MB",
	"4 MB",
	NULL
};

ULONG far zonesizes[] = {
	8*1024,
	16*1024,
	32*1024,
	64*1024,
	128*1024,
	256*1024,
	512*1024,
	1*1024*1024
};

static STRPTR ZoneSize[] = {
	"8K",
	"16K",
	"32K",
	"64K",
	"128K",
	"256K",
	"512K",
	"1 MB",
	NULL
};

ULONG far speeds[] = {
	100,
	150,
	200,
	250
};

static STRPTR Speeds[] = {
	"100ns",
	"150ns",
	"200ns",
	"250ns",
	NULL
};

static struct TextAttr far Topaz80 = {
	"topaz.font",8,FS_NORMAL,FPF_ROMFONT
};

static struct GadGadget far GG[] = {
	{CYCLE_KIND,    010, 14,   180, 12, CM_GADGET_SOURCE,	"Source ",	NG_HIGHLABEL|PLACETEXT_ABOVE},
	{BUTTON_KIND,   010, 112,  180, 12, CM_GADGET_ERASE,	"Full Erase",	0},
	{BUTTON_KIND,   010, 100,   180, 12, CM_GADGET_CHECK,	"Check Blank",	0},
	{BUTTON_KIND,   010, 124,  180, 12, CM_GADGET_DUP,	"Program",	0},
	{CHECKBOX_KIND, 010, 40,   32, 12,  CM_GADGET_ERASEON,	"Erase Enable",	PLACETEXT_RIGHT},
	{CHECKBOX_KIND, 010, 52,   32, 12,  CM_GADGET_WVERIFY,	"Full Verify",	PLACETEXT_RIGHT},
	{CHECKBOX_KIND, 010, 64,   32, 12,  CM_GADGET_INSTALL,	"Install",	PLACETEXT_RIGHT},
	{BUTTON_KIND,   410, 146,  180, 12, CM_GADGET_ABORT,	"Abort",	0},
	{BUTTON_KIND,   010, 146,  180, 12, CM_GADGET_CONFIRM,	"Confirm",	0},
	{CYCLE_KIND,    200, 14,   180, 12, CM_GADGET_MANUFACTURER,	"Type",	NG_HIGHLABEL|PLACETEXT_ABOVE},
	{CYCLE_KIND,    200, 50,   180, 12, CM_GADGET_TOTALSIZE,	"Total Size",	NG_HIGHLABEL|PLACETEXT_ABOVE},
	{CYCLE_KIND,    400, 50,   180, 12, CM_GADGET_ZONESIZE,	"Zone Size",	NG_HIGHLABEL|PLACETEXT_ABOVE},
	{CYCLE_KIND,    400, 14,   180, 12, CM_GADGET_SPEED,	"Speed",	NG_HIGHLABEL|PLACETEXT_ABOVE},
};
	
struct Gadget *CreateAGadget( struct cmdVars *cmv, struct Gadget **GADS, struct GadGadget *gg, ULONG tags, ... )
{
register struct cmdVars *cv;
struct NewGadget ng;

	cv = cmv;

	ng.ng_LeftEdge   = gg->gg_LeftEdge + cv->cv_sp->WBorLeft;
	ng.ng_TopEdge    = gg->gg_TopEdge + cv->cv_sp->WBorTop + cv->cv_sp->Font->ta_YSize + 1;
	ng.ng_Width	 = gg->gg_Width;
	ng.ng_Height	 = gg->gg_Height;
	ng.ng_GadgetText = gg->gg_Label;
	ng.ng_TextAttr   = &Topaz80;
	ng.ng_GadgetID   = 0;
	ng.ng_Flags      = gg->gg_GadgetFlags;
	ng.ng_VisualInfo = cv->cv_VI;
	ng.ng_UserData   = (APTR)gg->gg_Command;

	return(*GADS = CreateGadgetA(gg->gg_GadgetKind,*GADS,
				&ng,(struct TagItem *)&tags));

}

struct Window *OpenAWindow( struct cmdVars *cv, ULONG tags, ... )
{
	return(OpenWindowTagList(NULL,(struct TagItem *)&tags));
}
 
/**
 ** Draw Bevel Box
 **/

VOID BevelBox( struct cmdVars *cv, struct RastPort *rp, WORD l, WORD t, WORD w, WORD h, ULONG tag1, ...)
{
	DrawBevelBoxA(rp,(ULONG)l,(ULONG)t,(LONG)w,(ULONG)h,(struct TagItem *)&tag1);
}

VOID ChangeAttrs(struct cmdVars *cv, struct Gadget *gad, ULONG tags, ... )
{
	GT_SetGadgetAttrsA(gad, cv->cv_win , NULL, (struct TagList *)&tags );

}

BOOL MakeWindow( struct cmdVars *cmv )
{
register struct cmdVars *cv;


	cv = cmv;


	if(cv->cv_win = OpenAWindow(cv,
		WA_Title,	FLASH_TITLE,
		WA_InnerWidth,	600L,
		WA_InnerHeight,	160L,
		WA_Flags,	WFLG_SMART_REFRESH|WFLG_ACTIVATE|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET,
		WA_IDCMP,	IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|IDCMP_INTUITICKS,
		WA_AutoAdjust,	(BOOL)TRUE,
		WA_PubScreen,	cv->cv_sp,
		TAG_DONE))
	{

		SetUpWindow( cv, cv->cv_win, cv->cv_gadgets );

		return(TRUE);
	}
	return(FALSE);
}

VOID SetUpWindow( struct cmdVars *cv, struct Window *win, struct Gadget *gad )
{

register struct RastPort *rp;
static char status[] = "Status";

		rp = win->RPort;

		SetFont(rp,cv->cv_font);
		SetAPen(rp,(ULONG)cv->cv_di->dri_Pens[HIGHLIGHTTEXTPEN]);
		SetBPen(rp,(ULONG)cv->cv_di->dri_Pens[BACKGROUNDPEN]);
		SetDrMd(rp,JAM2);

		BevelBox(cv,rp,STATLEFT,STATTOP,STATWIDTH,STATHEIGHT,
			GTBB_Recessed, (BOOL)TRUE,
			GT_VisualInfo, cv->cv_VI,
			TAG_DONE);

		Move(rp,384,94 + cv->cv_sp->WBorTop + cv->cv_sp->Font->ta_YSize + 1);
		Text(rp,status,strlen(status));

		AddGList(win,gad,-1,-1,NULL);
		RefreshGList(gad,win,NULL,-1);
		GT_RefreshWindow(win,NULL);

		SetAPen(rp,(ULONG)cv->cv_di->dri_Pens[SHINEPEN]);

		Move(rp,(ULONG)win->BorderLeft,92);
		Draw(rp,(ULONG)(win->Width+1 - (win->BorderRight + win->BorderLeft)),92);

		SetAPen(rp,(ULONG)cv->cv_di->dri_Pens[SHADOWPEN]);

		Move(rp,(ULONG)win->BorderLeft,90);
		Draw(rp,(ULONG)(win->Width+1 - (win->BorderRight + win->BorderLeft)),90);

}

BOOL MakeGadgets( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct Gadget **GADS;


	cv=cmv;


	GADS = &cv->cv_GADS;


	cv->cv_gadgets = CreateContext(GADS);


	cv->cv_Source = CreateAGadget(cv,GADS,&GG[0],
		GTCY_Labels,	(STRPTR *)&Sources[0],
		GTCY_Active,	(UWORD)cv->cv_SourceIndex,
		TAG_DONE);

	cv->cv_EraseOnly = CreateAGadget(cv,GADS,&GG[1],
		TAG_DONE);

	cv->cv_CheckBlank = CreateAGadget(cv,GADS,&GG[2],
		TAG_DONE);

	cv->cv_Duplicate = CreateAGadget(cv,GADS,&GG[3],
		TAG_DONE);

	cv->cv_EraseFirst = CreateAGadget(cv,GADS,&GG[4],
		GTCB_Checked, (BOOL)cv->cv_EraseOn,
		TAG_DONE);

	cv->cv_VerifyWrite = CreateAGadget(cv,GADS,&GG[5],
		GTCB_Checked, (BOOL)cv->cv_VerifyOn,
		TAG_DONE);

	cv->cv_Bootable = CreateAGadget(cv,GADS,&GG[6],
		GTCB_Checked, (BOOL)cv->cv_BootOn,
		GA_Disabled,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_Abort = CreateAGadget(cv,GADS,&GG[7],
		GA_Disabled,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_Confirm = CreateAGadget(cv,GADS,&GG[8],
		GA_Disabled,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_Manufacturer = CreateAGadget(cv,GADS,&GG[9],
		GTCY_Labels,	(STRPTR *)&Manufacturer[0],
		GTCY_Active,	(UWORD)cv->cv_ManufIndex,
		GA_Disabled,	(BOOL)TRUE,
		TAG_DONE);

	cv->cv_TotalSize = CreateAGadget(cv,GADS,&GG[10],
		GTCY_Labels,	(STRPTR *)&TotalSize[0],
		GTCY_Active,	(UWORD)cv->cv_TotalIndex,
		TAG_DONE);

	cv->cv_ZoneSize = CreateAGadget(cv,GADS,&GG[11],
		GTCY_Labels,	(STRPTR *)&ZoneSize[0],
		GTCY_Active,	(UWORD)cv->cv_ZoneIndex,
		TAG_DONE);

	cv->cv_Speeds = CreateAGadget(cv,GADS,&GG[12],
		GTCY_Labels,	(STRPTR *)&Speeds[0],
		GTCY_Active,	(UWORD)cv->cv_SpeedIndex,
		TAG_DONE);

	if(cv->cv_Speeds)
	{
		return(TRUE);
	}

	FreeGadgets(cv->cv_gadgets);
	return(FALSE);

}

/*
 * Disable/Enable gadgets
 */
void GadgetsOnOff( struct cmdVars *cv, BOOL disable )
{
	ChangeAttrs(cv,cv->cv_Source,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_EraseOnly,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_CheckBlank,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_Duplicate,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_EraseFirst,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_VerifyWrite,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	if(disable)
	{
		ChangeAttrs(cv,cv->cv_Bootable,
			GA_Disabled,	(BOOL)disable,
			TAG_DONE);
	}
	else
	{
		RadBootBlock(cv);
	}

/*
	ChangeAttrs(cv,cv->cv_Manufacturer,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);
*/

	ChangeAttrs(cv,cv->cv_TotalSize,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_ZoneSize,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

	ChangeAttrs(cv,cv->cv_Speeds,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);


}


/*
 * Confirm On/Off
 */
void ConfirmOnOff( struct cmdVars *cv, BOOL disable )
{
	ChangeAttrs(cv,cv->cv_Confirm,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

}


/*
 * Abort On/Off
 */
void AbortOnOff( struct cmdVars *cv, BOOL disable )
{
	ChangeAttrs(cv,cv->cv_Abort,
		GA_Disabled,	(BOOL)disable,
		TAG_DONE);

}


/*
 * Start user interface
 *
 */

LONG StartGUI( struct cmdVars *cmv )
{
int error;
register struct cmdVars *cv;

	cv = cmv;
	error = RETURN_FAIL;

	if(cv->cv_font = OpenFont(&Topaz80))
	{
		if(cv->cv_sp = LockPubScreen(NULL))
		{
			if(cv->cv_VI = GetVisualInfoA(cv->cv_sp,NULL))
			{
				if(MakeGadgets( cv ))
				{
					if(cv->cv_di = GetScreenDrawInfo(cv->cv_sp))
					{

						if(MakeWindow( cv ))
						{
							error = RETURN_OK;
							cv->cv_FromCLI = FALSE;

							HandleEvents( cv );

							CloseWindow( cv->cv_win );
						}
						FreeScreenDrawInfo(cv->cv_sp,cv->cv_di);
					}
					FreeGadgets( cv->cv_gadgets );
				}
				FreeVisualInfo( cv->cv_VI );
			}
			UnlockPubScreen( NULL, cv->cv_sp );
		}
		CloseFont( cv->cv_font );
	}

	return( error );

}

VOID DisplayError( struct cmdVars *cv, STRPTR str )
{

		DoEasyRequest(cv, str, "Ok");

}

BOOL DisplayQuery( struct cmdVars *cv, STRPTR str )
{

		return( (BOOL)DoEasyRequest(cv, str, "Ok|Cancel"));
}

LONG DoEasyRequest( struct cmdVars *cv, STRPTR body, STRPTR gadgets )
{

struct EasyStruct es;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = FLASH_TITLE;
	es.es_TextFormat = body;
	es.es_GadgetFormat = gadgets;
	
	return( (LONG)EasyRequestArgs(NULL,&es,NULL,NULL) );

}

/* Draw text in Status/Type box */

VOID StatusBox( struct cmdVars *cmv, UWORD col, UWORD row, BOOL centered, UWORD boxid, STRPTR line )
{
register struct cmdVars *cv;
register ULONG x,y;
ULONG	coffset;
struct RastPort *rp;

	cv = cmv;

	if(boxid)
	{
		x=(STATLEFT+4L);
		y=(STATTOP+13L);
	}

	x += col*8;
	y += row*9;

	rp = cv->cv_win->RPort;

	SetAPen(rp,(ULONG)cv->cv_di->dri_Pens[TEXTPEN]);

	coffset = 0;

	if(centered)
	{
		if(boxid)
		{
			coffset = (((STATWIDTH-4L) - ((strlen(line))*8L))/2);
		}
	}

	Move(rp,(ULONG)x+coffset,(ULONG)y);
	Text(rp,line,strlen(line));

}

VOID ClearBox(struct cmdVars *cmv, UWORD boxid)
{
register struct cmdVars *cv;
struct RastPort *rp;

	cv = cmv;

	rp = cv->cv_win->RPort;

	SetAPen(rp,(ULONG)cv->cv_di->dri_Pens[BACKGROUNDPEN]);

	if(boxid)
	{
		RectFill(rp,	STATLEFT+2L,
				STATTOP+2L,
				STATLEFT+STATWIDTH-4L,
				STATTOP+STATHEIGHT-2L);

	}
}



/* getfilename()
 * 
 * Inputs: 	UBYTE *title (or NULL)
 *		UBYTE *namebuffer (required to receive filename)
 *		ULONG size of name buffer (required)
 *		struct Window *window (required to come up on custom screen)
 *		BOOL IsSave (pass non-zero to get a save requester);
 *
 * Returns: pointer to filename, or NULL for CANCEL or failure
 *
 * NOTE - In init, application must OpenLibrary("asl.library",37);
 * NOTE - In cleanup, application must do:

	if(AslBase)
	    {
	    if(freq) FreeAslRequest(freq);
	    CloseLibrary(AslBase);
	    }
 */



BOOL MyAslRequestTags(struct cmdVars *cv, APTR requester, ULONG tags, ...);

BOOL MyAslRequestTags(struct cmdVars *cv, APTR requester, ULONG tags, ...)
    {
    return (AslRequest(requester, (struct TagItem *)&tags));
    }


UBYTE *getfilename(struct cmdVars *cmv, UBYTE *title,UBYTE *fnamebuf,ULONG bufsz,
			struct Window *win, BOOL IsSave)
{
register struct cmdVars *cv;
ULONG funcflags;

cv = cmv;

/* If have asl.library and buffer, try requester else return failure (0) */
if((AslBase)&&(fnamebuf))
    {
    /* if we don't have one, alloc it */
    if(!freq) freq = AllocAslRequest(ASL_FileRequest, NULL);

    if(freq)
	{
	funcflags = 0;
	if(IsSave) funcflags |= FILF_SAVE;

	if(MyAslRequestTags(cv, (APTR)(freq),
				ASL_Hail,title,
				ASL_Window, win,
				ASL_FuncFlags,funcflags,
				TAG_DONE ))
	    {
	    strcpy(fnamebuf,freq->rf_Dir);
	    if(AddPart(fnamebuf,freq->rf_File,bufsz)) return(fnamebuf);
	    }
	else return(NULL);
	}
    }
    return(NULL);
}


