/* ========================================================================= *
 * Window.c - initialization and window creation for installer utility       *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#ifndef EXEC_TYPES_H
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <exec/memory.h>

/* #define __NO_PRAGMAS */
#include "functions.h"
#endif

#ifdef ONLY2_0
#include <clib/locale_protos.h>
#include <locale.p>
#include <libraries/locale.h>
#include "installer_2_rev.h"
#else
#include "installer_rev.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "xfunctions.h"
#include "macros.h"

#include "wild.h"
#include "window.h"
#include "installer.h"
#include "text.h"

#define LEAVE			goto exitit;

/*====================== Intuition Structures ======================== */

struct NewWindow new_window =
{	100,60,300,120, -1,-1, STD_FLAGS,
	WINDOWDRAG | WINDOWDEPTH | NOCAREREFRESH | ACTIVATE | SMART_REFRESH,
	NULL,NULL,NULL,NULL,NULL,100,120,-1,-1,WBENCHSCREEN,
};

struct TextAttr Topaz8 = { (STRPTR)"topaz.font", 8,0,0 };
/* struct TextAttr Topaz9 = { (STRPTR)"topaz.font", 9,0,0 }; */

struct Library			*IntuitionBase=NULL,
						*LayersBase=NULL,
						*GfxBase=NULL;
extern struct Library	*IconBase;			/* in Manx startup code */
#ifdef ONLY2_0
struct Library			*LocaleBase,
						*GadToolsBase;
struct Catalog			*catalog;
APTR					visinfo;
struct TagItem			null_tag = { TAG_DONE, 0 };
#endif

char					*language = "english";

struct TextFont			*TFont = NULL,
						*UFont = NULL;

struct Screen			*theScreen;
struct Window 			*window = NULL;
struct RastPort			*rp;
struct Gadget			*first_gadget;

struct WBStartup		*wbmsg = NULL;				/* workbench message */

WORD					window_width,
						window_height,
						depth_width;

WORD					darkest_color,
						lightest_color,
						text_color,
						highlight_color;

WORD					left_edge,
						right_edge,
						top_edge,
						bottom_edge;

extern WORD				place_top,				/* where to place next upper control */
						place_bottom;			/* where to place next lowe control */

extern WORD				prop_flags,
						textzone_count,
						checkmark_count,
						button_count;

/* ========================= Prototypes ================================== */

int VSprintf(char *,char *,void *);

/* void render_text(struct RastPort *rp,char *text,WORD length); */
#define	render_text(a,b,c)	Text(a,b,c)

/* ==================== Parser stuff =============================== */

struct TreeNode 		*root;

extern UBYTE			ierror;
extern struct TreeNode	*ehead;
extern char				format_string[];

UWORD					pretend_flag = FALSE,
						user_level = 0,
						minuser_level = 0,
						transcript_mode = 2,
						no_pretend = FALSE;
char					*icon_name=NULL;
char					*templatefile;
char					app_name[40] = "Test App";

	/* 0 = no (welcome), 1 = has (welcome), 2 = called welcome_page() */
WORD					welcome_state = 0;

struct String			*extra_exit_text;

	/* fill in to do change-startup */
char					*textbuf;
int						textloc;

WORD					xsize;

void					**pretend_ptr,
						**ulevel_ptr,
						**language_ptr,*language_save,
						**app_ptr,*app_save,
						**icon_ptr,*icon_save,
						**defpart_ptr,*defpart_save;
BPTR					transcript_file = NULL;
BOOL					must_log = TRUE,
						no_print = FALSE,
						exit_quiet = FALSE;

extern BOOL				show_gauge;
extern UBYTE			local_delopts;

long					percent_complete = -1;

BOOL					did_final = FALSE;

/* ==================== Busy Pointer Variables ===================== */

USHORT					*chip_busy = NULL;
BYTE					inited_busy = 0,
						inited_fail = 0,
						is_busy = 0;

/* ==================== Text Strings =============================== */

extern char *gir_prompt, *gir_default, *gir_help, *gir_deftool;

extern char *transcript_header, *def_transcript_name, *usage_tx;

char	transcript_name[128],
		xtra_trans_name[128];
char 	*temp_transcript_name;
char	allpat_tx[] = "#?";

extern char	*minuser_tt, *defuser_tt, *pretend_tt, *script_tt,
	*language_tt,
	*appname_tt, *logfile_tt, *nopretend_tt, *nolog_tt, *log_tt,
	*noprint_tt, *print_tt,
	*false_tv, *novice_tv, *average_tv, *expert_tv;

extern char	*dot_info, *dot_font;

extern char *no_find_icon;

extern char *trans_made_drawer, *trans_copy_file, *trans_copy_drawer,
	*trans_ask_choice, *trans_no_options, *trans_ask_options,
	*trans_ask_string, *trans_ask_number, *trans_ask_yesno, *trans_dest_drawer,
	*trans_ask_drawer, *trans_ask_file;

extern char	*prob_with_source,
	*help_copyfiles, *help_with_choice, *help_with_options, *help_ask_string,
	*help_ask_number, *help_insert_disk,
	*help_ask_yesno,
	*help_select_drawer,
	*help_select_file, *inst_complete1, *inst_complete2, *inst_logfile_name,
	*help_with_confirm, *version_msg1, *version_msg2, *version_msg3,
	*set_inst_mode, *welcome_msg, *help_inst_mode,
	*welcome_msg2, *help_with_settings,
	*prob_with_file, *help_change_startseq, *error_msg, *dos_error_type,
	*sorry_msg, *please_confirm, *press_escape, *valid_range;

extern char *bad_params_copy, *invalid_drawer_name;

char	*user_level_names[3],
		*pretend_mode_names[2];

extern char VersionString[];		/* created by "bumprev" */

WORD	paramflags = 0;				/* CLI & WB parameters */
#define PARAM_SCRIPT	(1<<0)
#define PARAM_APPNAME	(1<<1)
#define PARAM_MINUSER	(1<<2)
#define PARAM_DEFUSER	(1<<3)
#define PARAM_LOGFILE	(1<<4)
#define PARAM_LANGUAGE	(1<<5)
#define PARAM_PRETEND	(1<<6)
#define PARAM_NOLOG		(1<<7)
#define PARAM_NOPRINT	(1<<8)

#define MAX_PARAMS		9

char					*params[MAX_PARAMS];

/* ==================== GUI & Main code ============================ */

	/* calculate the installer's window size based on system fonts */

void calc_window_size(struct TextFont *tf)
{	struct RastPort rp;

	InitRastPort(&rp);
	SetFont(&rp,tf);
	xsize = TextLength(&rp,"MEi",3) / 3;
	window_height = (tf->tf_YSize + 1) * 16 +
		theScreen->RastPort.Font->tf_YSize + 1 + 6;	/* + 6 button compinsation */
	window_width = xsize * (tf->tf_Flags & FPF_PROPORTIONAL ? 62 : 56);
}

	/* this function handles internationalization */

void assign_text(void)
{
#ifdef ONLY2_0
	if (LocaleBase = OpenLibrary("locale.library",36L))
	{
		static struct TagItem cat_tags[] = {
			OC_BuiltInLanguage,		NULL,
			OC_Language,			NULL,
			TAG_DONE,				NULL
		};

		cat_tags[0].ti_Data = (LONG)"english";
		cat_tags[1].ti_Data = (LONG)language;
		unless (catalog = OpenCatalogA(NULL,"sys/installer.catalog",cat_tags))
			catalog = OpenCatalogA(NULL,"installer.catalog",cat_tags);
	}
#endif

	localize_text();

	if (transcript_name[0] == 0)
		strcpy(transcript_name,def_transcript_name);
	user_level_names[0] = novice_tv;
	user_level_names[1] = average_tv;
	user_level_names[2] = expert_tv;
	pretend_mode_names[0] = GetLocalText(TX_NO);
	pretend_mode_names[1] = GetLocalText(TX_YES);
}

WORD GetDepthGadgetWidth(struct Window *window)
{	struct Gadget	*gad = window->FirstGadget;
	WORD			width = 0;

	while (gad != NULL)
	{
		if ((gad->GadgetType & 0x00f0) == GTYP_WUPFRONT) width += gad->Width;
		if ((gad->GadgetType & 0x00f0) == GTYP_WDOWNBACK) width += gad->Width;
		gad = gad->NextGadget;
	}

	return width;
}

	/* main() is defined void since I'm using exit() to return from it. */

enum {
	ERR_LIBS=1,
	ERR_ICON,
	ERR_SCREEN,
	ERR_ARGS,
	ERR_USAGE,
	ERR_MEM
};

#if 0

/* debugging code */

char *start_new_name(void)
{	struct Process		*pr;
	char				*name;

	pr = (struct Process *)FindTask(NULL);
	name = pr->pr_Task.tc_Node.ln_Name;
	pr->pr_Task.tc_Node.ln_Name = "installer";
	return name;
}

void end_new_name(char *name)
{	struct Process		*pr;

	pr = (struct Process *)FindTask(NULL);
	pr->pr_Task.tc_Node.ln_Name = name;
}
#endif

#define MAINBIT_ASSIGN		(1<<0)

void main(int argc,char **argv)
{	WORD				result = 0;
#ifndef ONLY2_0
	struct Screen		wbScreenData;
#endif
	char				*install_script = NULL,
						*name = NULL;
	WORD				i,k,w,h,dx,dy;
	BPTR				script_lock = NULL, olddir;
	BOOL				main_bits = MAINBIT_ASSIGN;
	WORD				err_mode = ERR_LIBS;

	params[0] = script_tt;
	params[1] = appname_tt;
	params[2] = minuser_tt;
	params[3] = defuser_tt;
	params[4] = logfile_tt;
	params[5] = language_tt;
	params[6] = nopretend_tt;
	params[7] = nolog_tt;
	params[8] = noprint_tt;

	if (!(IntuitionBase = OpenLibrary("intuition.library",0L))) LEAVE;
#ifdef ONLY2_0
	if (!(GadToolsBase = OpenLibrary("gadtools.library",0))) LEAVE;
#endif
	if (!(GfxBase = OpenLibrary("graphics.library",0))) LEAVE;
	if (!(LayersBase = OpenLibrary("layers.library",0))) LEAVE;
	if (!(IconBase = OpenLibrary("icon.library",0))) LEAVE;

	if (!OpenConsoleLib()) LEAVE;

	if (argc == 0)							/* WorkBench startup */
	{	struct DiskObject	*dobj;
		BPTR				lock;

		err_mode = ERR_ICON;

		wbmsg = (struct WBStartup *)argv;
		if (wbmsg->sm_NumArgs == 0) LEAVE;		/* ok, who sent this!! */

		k = (wbmsg->sm_NumArgs > 1 ? 1 : 0);

		name = (char *)wbmsg->sm_ArgList[k].wa_Name;
		if (name[0] == '\0') LEAVE;			/* if it's not a project or tool */

			/* get script parent dir and script name */
		script_lock = wbmsg->sm_ArgList[k].wa_Lock;
		templatefile = install_script = name;

		lock = CurrentDir(script_lock);
		dobj = GetDiskObject(name);
		CurrentDir(lock);
		if (dobj == NULL) LEAVE;

			/* parse tooltypes */

			/* MINUSER=NOVICE|AVERAGE|EXPERT */
		if (CheckToolValue(dobj,minuser_tt,average_tv)) minuser_level = 1;
		if (CheckToolValue(dobj,minuser_tt,expert_tv)) minuser_level = 2;

			/* DEFUSER=NOVICE|AVERAGE|EXPERT */
		if (CheckToolValue(dobj,defuser_tt,average_tv)) user_level = 1;
		if (CheckToolValue(dobj,defuser_tt,expert_tv)) user_level = 2;

			/* LOGFILE=FILENAME */
		if (name = LocateToolType(dobj,logfile_tt))
			strcpy(transcript_name,name);

			/* LOG */
		if (CheckToolValue(dobj,log_tt,false_tv)) must_log = FALSE;

			/* APPNAME=NAME */
		if (name = LocateToolType(dobj,appname_tt)) strcpy(app_name,name);

			/* PRETEND=FALSE */
		if (CheckToolValue(dobj,pretend_tt,false_tv)) no_pretend = TRUE;

			/* PRINT=FALSE */
		if (CheckToolValue(dobj,print_tt,false_tv)) no_print = TRUE;

			/* SCRIPT=NAME */
		if (name = LocateToolType(dobj,script_tt))
		{
			if (templatefile = install_script = MemAlloc(strlen(name)+1,0L))
			{
				strcpy(install_script,name);
			}
			else
			{
				err_mode = ERR_MEM;
				LEAVE;
			}
		}

			/* LANGUAGE=NAME */
		if (name = LocateToolType(dobj,language_tt))
		{
			if (language = MemAlloc(strlen(name)+1,0L))
			{
				strcpy(language,name);
			}
			else
			{
				err_mode = ERR_MEM;
				LEAVE;
			}
		}

		FreeDiskObject(dobj);

		if (err_mode != ERR_ICON) LEAVE;
	}
	else									/* CLI startup */
	{
		err_mode = 0;

		Printf("%s\n",VersionString);

		if (argc == 1 || (argc == 2 && argv[1][0] == '?'))
		{
			err_mode = ERR_USAGE;
			LEAVE;
		}

		for (i=1;i<argc;i++)				/* run through CLI parameters */
		{
			for (w=0; w<MAX_PARAMS; w++)
			{
				if ( !(paramflags & 1 << w) && !(stricmp(argv[i],params[w])) )
				{
					if (w < 6)
					{
						if (i == argc - 1) continue;
						i++;
					}
					break;
				}
			}

			if (w == MAX_PARAMS)
				for (w=0;w<6;w++) if ( !(paramflags & 1 << w) ) break;

			switch (w)
			{
				case 0:
				templatefile = install_script = argv[i];
				break;

				case 1:
				strcpy(app_name,argv[i]);
				break;

				case 2:
				if (!stricmp(argv[i],expert_tv)) minuser_level = 2;
				else if (!stricmp(argv[i],average_tv)) minuser_level = 1;
				break;

				case 3:
				if (!stricmp(argv[i],expert_tv)) user_level = 2;
				else if (!stricmp(argv[i],average_tv)) user_level = 1;
				break;

				case 4:
				strcpy(transcript_name,argv[i]);
				break;

				case 5:
				language = argv[i];
				break;

				case 6:
				no_pretend = 1;
				break;

				case 7:
				must_log = FALSE;
				break;

				case 8:
				no_print = TRUE;
				break;

				default:
				err_mode = ERR_ARGS;
				LEAVE;
			}

			paramflags |= 1 << w;
		}
	}

	if (install_script == NULL)
	{
		err_mode = ERR_ARGS;
		LEAVE;
	}

	assign_text();
	main_bits = 0;

	if (user_level < minuser_level) user_level = minuser_level;

	strcpy(xtra_trans_name,"RAM:");
	TackOn(xtra_trans_name,FileOnly(transcript_name),128);
	temp_transcript_name = xtra_trans_name;

	/* get default screen data */

	err_mode = ERR_SCREEN;

#ifndef ONLY2_0
	if (IntuitionBase->lib_Version >= 36)
#endif
	{
		theScreen = LockPubScreen(NULL);
		if (theScreen == NULL) LEAVE;
#ifdef ONLY2_0
		visinfo = GetVisualInfoA(theScreen,&null_tag);
#endif
	}
#ifndef ONLY2_0
	else
	{
		if (!GetScreenData((char *)&wbScreenData,sizeof wbScreenData,WBENCHSCREEN,NULL)) LEAVE;
		theScreen = &wbScreenData;
	}
#endif

	err_mode = 0;

#ifndef ONLY2_0
	if (IntuitionBase->lib_Version >= 36)
#endif
	{
		static struct TagItem vpe_tags[2] = {
			{ VTAG_VIEWPORTEXTRA_GET, NULL }, { TAG_DONE, }
		};
		struct ViewPortExtra *vpe;
		vpe_tags[0].ti_Data = NULL;

		VideoControl(theScreen->ViewPort.ColorMap,vpe_tags);

		if (vpe = (struct ViewPortExtra *)vpe_tags[0].ti_Data)
		{	dx = -theScreen->LeftEdge;
			dy = -theScreen->TopEdge;
			w = vpe->DisplayClip.MaxX - vpe->DisplayClip.MinX + 1;
			if (w > theScreen->Width) w = theScreen->Width;
			h = vpe->DisplayClip.MaxY - vpe->DisplayClip.MinY + 1;
			if (h > theScreen->Height) h = theScreen->Height;
		}
		else
		{	dx = dy = 0;
			w = theScreen->Width;
			h = theScreen->Height;
		}
	}
#ifndef ONLY2_0
	else
	{
		if (theScreen->ViewPort.RasInfo != NULL)
		{	dx = theScreen->ViewPort.RasInfo->RxOffset;
			dy = theScreen->ViewPort.RasInfo->RyOffset;
			w = theScreen->ViewPort.DWidth;
			h = theScreen->ViewPort.DHeight;
		}
		else
		{	dx = dy = 0;
			w = theScreen->Width;
			h = theScreen->Height;
		}
	}
#endif

#if 0		/* oh, well... it was a nice experiment */
	calc_window_size(UFont = theScreen->RastPort.Font);

		/* if the window would be too big, recalculate using def font */
	if (window_width > w || window_height > h)
#endif
	{
		calc_window_size(UFont = ((struct GfxBase *)GfxBase)->DefaultFont);
	}

		/* if the window still too big, recalculate using topaz 8 */
	if (window_width > w || window_height > h)
	{
		UFont = TFont = OpenFont(&Topaz8);
		if (UFont == NULL) 
		{	/* so where did you put topaz 8??? oh well, just exit */

			auto_err(GetLocalText(MSG_NO_FONT));
			LEAVE;
		}
		calc_window_size(UFont);
	}

	new_window.Height = window_height;
	new_window.Width = window_width;
	new_window.LeftEdge = (w - window_width) / 2 + dx;
	new_window.TopEdge = (h - window_height) / 2 + dy;

	window = OpenWindow(&new_window);

#ifndef ONLY2_0
	if (IntuitionBase->lib_Version >= 36)
#endif
	{
		UnlockPubScreen(NULL,theScreen);
	}

	if (window == NULL)
	{
		auto_err(GetLocalText(MSG_NO_WINDOW));
		LEAVE;
	}

	SetWindowTitles(window,templatefile,(char *)-1);

		/* Snoop the palette here... */
	assign_colors();

		/* make images based on snooped palette */
	make_images();

	left_edge     = window->BorderLeft + 4;
	right_edge    = window->BorderRight + 4;
	top_edge      = window->BorderTop + 1;
	bottom_edge   = window->BorderBottom + 1;
	window_width  = window->Width;
	window_height = window->Height;
	rp = window->RPort;
	SetFont(rp,UFont);

	depth_width = GetDepthGadgetWidth(window);

	if (script_lock) olddir = CurrentDir(script_lock);

	if (init_symbols())							/* init parser */
	{
		set_busy();

		if (root = compile(install_script))		/* compile script */
		{
			app_save = *app_ptr = MakeString(app_name,strlen(app_name));

			language_save = *language_ptr = MakeString(language,strlen(language));

			clear_busy();

				/* set up path to original startup icon */
			if (argc == 0)
			{
				k = (wbmsg->sm_NumArgs > 1 ? 1 : 0);

				name = (char *)wbmsg->sm_ArgList[k].wa_Name;
				if (strchr(name,':')) strcpy(format_string,name);
				else
				{	ExpandPath((void *)wbmsg->sm_ArgList[k].wa_Lock,
						format_string,MAX_STRING);
					TackOn(format_string,name,MAX_STRING);
				}

				icon_save = *icon_ptr =
					MakeString(format_string,strlen(format_string));

				if (*icon_ptr == NULL)
				{	report_message(1,no_find_icon);
					goto quit;
				}
			}
			else
			{
				/*	At the present time, '@icon' is not defined for scripts
					started from a CLI. But the install_script could be
					checked to see if it had an icon... */
			}

				/* set up default path for installation */
			defpart_save = *defpart_ptr = find_largest_partition();
			if (*defpart_ptr == NULL)
			{	report_message(1,GetLocalText(MSG_NO_PARTITION));
				goto quit;
			}

			if (welcome_state == 0 && !welcome_page(NULL)) goto quit;

#if 0
				/* put up the first options screen */
			if (get_user() == FALSE) goto quit;

				/* put up the second options screen if not in NOVICE mode */
repeat:		if (user_level != USER_NOVICE && get_options() == FALSE) goto quit;

				/* insert the options into the symbols so that script can use them */
			*pretend_ptr = (void *)pretend_flag;
			*ulevel_ptr = (void *)user_level;

				/* open the log file if option selected */
			if (user_level == USER_NOVICE && must_log) transcript_mode = 1;

			if (transcript_mode < 2)
			{
				transcript_file =
					Open(transcript_mode ? temp_transcript_name : "PRT:",
						MODE_NEWFILE);

				if (transcript_file == NULL)
				{
					report_message(1, GetLocalText(
						transcript_mode ? MSG_NO_TRANSCRIPT : MSG_NO_PRINTER));
					goto repeat;
				}

				if (Fprintf((void *)transcript_file,transcript_header,
					user_level_names[user_level],
					pretend_mode_names[pretend_flag]) < 58)
				{
					Close(transcript_file);
					transcript_file = NULL;
					report_message(1, GetLocalText(
						transcript_mode ? MSG_ACCESS_TRANSCRIPT : MSG_ACCESS_PRINTER));
					goto repeat;
				}
			}
#endif

			set_busy();
			quick_working_page();
			doroot(root->value.v);				/* interpret script */
			clear_busy();

skip:		if (ierror == 0)
			{
				if (!did_final) final_page();
			}
			else
			{
				if (ierror != ERR_HANDLED && ierror != ERR_ABORT)
					transcript("Error not Trapped!!!\n\n");
				close_transcript();
			}

quit:		de_compile(root);

			FreeString(icon_save);
			FreeString(defpart_save);
			FreeString(app_save);
			FreeString(extra_exit_text);
			FreeString(language_save);
		}
		else
		{	extern char *compile_err;
			extern long linenum;

			clear_busy();
		 	report_message(1,
				GetLocalText(linenum ? MSG_NO_COMPILE : MSG_NO_SCRIPT),
				compile_err,linenum);
		}
	}
	else report_message(1,GetLocalText(MSG_NO_INIT));

	if (script_lock) CurrentDir(olddir);
	cleanup_symbols();

exitit:
	if (TFont) CloseFont(TFont);
	if (window) CloseWindow(window);

	if (main_bits & MAINBIT_ASSIGN) assign_text();

	switch (err_mode)
	{
		case ERR_LIBS: auto_err(GetLocalText(MSG_NO_LIBRARIES)); break;
		case ERR_ICON: auto_err(no_find_icon); break;
		case ERR_SCREEN: auto_err(GetLocalText(MSG_NO_WORKBENCH)); break;
		case ERR_ARGS: Puts(GetLocalText(MSG_BAD_ARGS)); break;
		case ERR_USAGE: Puts(usage_tx); break;
	}

	CloseConsoleLib();

#ifdef ONLY2_0
	if (LocaleBase)
	{
		if (catalog) CloseCatalog(catalog);
		CloseLibrary(LocaleBase);
	}
	if (visinfo) FreeVisualInfo(visinfo);
	if (GadToolsBase) CloseLibrary(GadToolsBase);
#endif

	if (IconBase) CloseLibrary(IconBase);
	if (LayersBase) CloseLibrary(LayersBase);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);

	exit(result);
}

BOOL welcome_page(struct InstallationRecord *ir)
{
	if (welcome_state == 2) return TRUE;
	welcome_state = 2;

		/* put up the first options screen */
	if (get_user(ir) == FALSE) return FALSE;

		/* put up the second options screen if not in NOVICE mode */
repeat:
	if (user_level != USER_NOVICE && get_options(ir) == FALSE) return FALSE;

		/* insert the options into the symbols so that script can use them */
	*pretend_ptr = (void *)pretend_flag;
	*ulevel_ptr = (void *)user_level;

		/* open the log file if option selected */
	if (user_level == USER_NOVICE && must_log) transcript_mode = 1;

	if (transcript_mode < 2)
	{
		transcript_file =
			Open(transcript_mode ? temp_transcript_name : "PRT:", MODE_NEWFILE);

		if (transcript_file == NULL)
		{
			report_message(1, GetLocalText(
				transcript_mode ? MSG_NO_TRANSCRIPT : MSG_NO_PRINTER));
			goto repeat;
		}

		if (Fprintf((void *)transcript_file,transcript_header,
			user_level_names[user_level],
			pretend_mode_names[pretend_flag]) < 58)
		{
			Close(transcript_file);
			transcript_file = NULL;
			report_message(1, GetLocalText(
				transcript_mode ? MSG_ACCESS_TRANSCRIPT : MSG_ACCESS_PRINTER));
			goto repeat;
		}
	}

	return TRUE;
}

	/* if script specified alterate logfile location, move there */

BOOL copylogfile(char *src,char *dest)
{	BPTR	sfh, dfh;
	BOOL	cp = FALSE;

	if (sfh = Open(src,MODE_OLDFILE))
	{	if (dfh = Open(dest,MODE_NEWFILE))
		{
			cp = (CopyFile(sfh,dfh) == 1);
			Close(dfh);
		}
		Close(sfh);
	}

	if (cp == TRUE) DeleteFile(src);

	return cp;
}

	/* close the transcript file and possibly move elsewhere file */

char *close_transcript(void)
{	char	*name;

	if (transcript_file)
	{
		Close(transcript_file);
		transcript_file = NULL;

		if (transcript_mode == 1)
		{
			name = STR_VAL(*defpart_ptr);
			if (name[0] != 0)
			{	extern char format_string[];

				/* we want to copy the log file to the @default-dest */
				if (FileOnly(transcript_name) == transcript_name)
				{
					strcpy(format_string,name);
					TackOn(format_string,transcript_name,MAX_STRING);
				}
				else strcpy(format_string,transcript_name);

				if (!strcmp(temp_transcript_name,format_string))
					name = temp_transcript_name;
				else if (copylogfile(temp_transcript_name,format_string))
					name = format_string;
				else name = temp_transcript_name;
			}
			else name = temp_transcript_name;

			/* write an icon */
			write_log_file_icon(name);

			/* return where the log file ended up... */
			return name;
		}
	}

	return NULL;
}

	/* routines to perform gadget maintenance */

void set_gadgets(struct Gadget *gad)
{	extern struct TextZone	*tz_activate;

	first_gadget = gad;
	AddGList(window,gad,-1,-1,NULL);
	RefreshGList(gad,window,NULL,-1);
	if (tz_activate) ActivateGadget(&tz_activate->Gadget,window,NULL);
}

void disable_gadgets(struct Gadget *gad)
{	struct Gadget *ngad = gad;

	while (ngad)
	{
		if (ngad->Activation & (GADGIMMEDIATE|RELVERIFY))
			ngad->Flags |= GADGDISABLED;
		ngad = ngad->NextGadget;
	}

	RefreshGList(gad,window,NULL,-1);
}

void remove_gadgets(struct Gadget *gad)
{
	RemoveGList(window,gad,-1);
	EmptyPort(window->UserPort);
}

void clear_gadgets(struct Gadget *gad)
{
	disable_gadgets(gad);
	remove_gadgets(gad);
	first_gadget = NULL;
}

	/* handle all standard forms of input, return value of any unknown input */

struct Gadget *HandleAnyEvent(struct ListControl *ctrl)
{
	struct IntuiMessage *imsg;
	ULONG			class,newscroll;
	UWORD			code,qual,ticks,moved,temp;
	WORD			arrow = 0,
					insert = 0,
					my;
	struct Gadget 	*gad,*lgad;
	struct PropInfo *prop;
	BPTR			lock,olddir;
	LONG			i;

	while (TRUE)
	{
		moved = 0;

		while (imsg = (struct IntuiMessage *)GetMsg(window->UserPort))
		{	class = imsg->Class;
			code = imsg->Code;
			qual = imsg->Qualifier;
			gad = (struct Gadget *)imsg->IAddress;
			my = imsg->MouseY;

			if (class == RAWKEY && (temp = DoKeyConvert(imsg)))
			{
				class = VANILLAKEY;
				code = temp;
			}

			ReplyMsg(&imsg->ExecMessage);

			switch (class)
			{
				case VANILLAKEY:
				if (gad = find_key_gadget(code)) return gad;
				break;

				case RAWKEY:
				if (code == RAW_HELP)
				{
					if (gad = FindGadget(window,HELP_ID)) return gad;
				}
				if (code == RAW_ESC)
				{
					if (gad = FindGadget(window,ABORT_ID)) return gad;
				}
				if (code == RAW_UP || code == RAW_LEFT)
				{
					if (gad = FindGadget(window,UP_ID))
					{
						if (ctrl != NULL) goto as_arrow;
						return gad;
					}
				}
				if (code == RAW_DOWN || code == RAW_RIGHT)
				{
					if (gad = FindGadget(window,DN_ID))
					{
						if (ctrl != NULL) goto as_arrow;
						return gad;
					}
				}
				break;

				case GADGETDOWN:
				if (gad->GadgetID == LIST_ID)
				{
					if (ctrl->Files)
					{
						/* this is the only place where which_file is
							called. --DJ */

						i = which_file(gad,ctrl,my);
						if (ctrl->Count == i) break;
						ctrl->Hilite = i;

						if (get_entry_type(ctrl->Files[ctrl->Hilite]) != IS_FILE)
						{	hilite_file(gad,ctrl);
							if (ctrl->DrawerZone == NULL) return gad;

							if (ctrl->Lock) olddir = CurrentDir(ctrl->Lock);
							lock = Lock(STR_VAL(ctrl->Files[ctrl->Hilite]),ACCESS_READ);
							expand_zone(ctrl->DrawerZone,lock);
							if (ctrl->Lock) CurrentDir(olddir);

							if (lock != NULL)
							{	free_matches(ctrl->Files);
								UnLock(ctrl->Lock);
								ctrl->Lock = lock;

								set_busy();
								ctrl->Files = match_files(ctrl->Lock,allpat_tx,
									ctrl->Match,&ctrl->Count);
								clear_busy();

								ctrl->Start = 0;

								show_files(FindGadget(window,LIST_ID),ctrl);
								set_slider(FindGadget(window,SLIDER_ID),ctrl);	
							}
						}
						else if (ctrl->Mode != 1)
						{
							hilite_file(gad,ctrl);
							if (ctrl->FileZone == NULL) return gad;

							set_zone(ctrl->FileZone,STR_VAL(ctrl->Files[ctrl->Hilite]));
						}
					}
					else
					{	i = which_line(gad,ctrl,my);
						if (i != ctrl->Hilite)
						{	gad = FindGadget(window,LIST_ID);
							erase_text(gad,ctrl);
							ctrl->Hilite = i;
							show_text(gad,ctrl,1);
						}
						insert = 1;
						ModifyIDCMP(window,STD_FLAGS|INTUITICKS);	
					}
				}
				else if (gad->GadgetID == UP_ID || gad->GadgetID == DN_ID)
				{
					if (ctrl != NULL)
					{
						ModifyIDCMP(window,STD_FLAGS|INTUITICKS);	
as_arrow:				arrow = (gad->GadgetID == UP_ID ? -1 : 1);
						ticks = 0;

						i = ctrl->Start + arrow;
						if (i >= 0 && i <= ctrl->Count - ctrl->Visible)
						{
							ctrl->OldStart = ctrl->Start;
							ctrl->Start = i;
							if (ctrl->Files)
								show_files(FindGadget(window,LIST_ID),ctrl);
							else show_text(FindGadget(window,LIST_ID),ctrl,0);
							set_slider(FindGadget(window,SLIDER_ID),ctrl);
						}
					}
				}
				else if (gad->GadgetID < FIRSTRESV_ID &&
					(gad->GadgetType & GTYP_GTYPEMASK) == BOOLGADGET &&
					(gad->Activation & GADGIMMEDIATE) &&
					!(gad->Activation & TOGGLESELECT) &&
					!(gad->Flags & SELECTED))
				{
						/* this handles all the radio gadgets in a group */
					lgad = first_gadget;
					while (lgad != NULL)
					{	if (lgad != gad && 
							(lgad->GadgetType & GTYP_GTYPEMASK) == BOOLGADGET &&
							(lgad->Activation & GADGIMMEDIATE) &&
							!(lgad->Activation & TOGGLESELECT) &&
							lgad->UserData == gad->UserData &&
							(lgad->Flags & SELECTED))
								SelectGadget(lgad,window,0);
						lgad = lgad->NextGadget;
					}
					SelectGadget(gad,window,SELECTED);
				}
				else return gad;
				break;

				case GADGETUP:
				if (gad->GadgetID == LIST_ID)
				{	if (insert)
					{	insert = 0;
						ModifyIDCMP(window,STD_FLAGS);	
					}
				}
				else if (gad->GadgetID == UP_ID || gad->GadgetID == DN_ID)
				{
					if (ctrl == NULL) return gad;

					arrow = 0;
					ModifyIDCMP(window,STD_FLAGS);	
				}
				else if (gad->GadgetID == PARENT_ID)
				{	if (ctrl->Lock != NULL && (lock = ParentDir(ctrl->Lock)))
					{	free_matches(ctrl->Files);
						UnLock(ctrl->Lock);

						ctrl->Lock = lock;
						if (ctrl->DrawerZone)
							expand_zone(ctrl->DrawerZone,lock);

						set_busy();
						ctrl->Files = match_files(ctrl->Lock,allpat_tx,ctrl->Match,
							&ctrl->Count);
						clear_busy();

						ctrl->Start = 0;

						show_files(FindGadget(window,LIST_ID),ctrl);
						set_slider(FindGadget(window,SLIDER_ID),ctrl);	
					}
				}
				else if (gad->GadgetID == DRIVES_ID)
				{
					if (ctrl->Lock != NULL)
					{
						free_matches(ctrl->Files);

						if (ctrl->LastLock) UnLock(ctrl->LastLock);
						ctrl->LastLock = ctrl->Lock;
						ctrl->Lock = NULL;

						ctrl->Files = get_drives(&ctrl->Count);
						ctrl->Start = 0;

						show_files(FindGadget(window,LIST_ID),ctrl);
						set_slider(FindGadget(window,SLIDER_ID),ctrl);	
					}
					else if (ctrl->LastLock != NULL)
					{
						free_matches(ctrl->Files);

						ctrl->Lock = ctrl->LastLock;
						ctrl->LastLock = NULL;

						if (ctrl->DrawerZone)
							expand_zone(ctrl->DrawerZone,ctrl->Lock);

						set_busy();
						ctrl->Files = match_files(ctrl->Lock,allpat_tx,ctrl->Match,
							&ctrl->Count);
						clear_busy();

						ctrl->Start = 0;

						show_files(FindGadget(window,LIST_ID),ctrl);
						set_slider(FindGadget(window,SLIDER_ID),ctrl);	
					}
				}
				else if (gad->GadgetID == SLIDER_ID)
				{	prop = (struct PropInfo *)gad->SpecialInfo;
					if ( !(prop->Flags & KNOBHIT) )
					{	newscroll = prop->VertPot * (ctrl->Count-ctrl->Visible) / MAXBODY;
						if (newscroll != ctrl->Start)
						{
							ctrl->OldStart = ctrl->Start;
							ctrl->Start = newscroll;
							if (ctrl->Files)
								show_files(FindGadget(window,LIST_ID),ctrl);
							else show_text(FindGadget(window,LIST_ID),ctrl,0);
						}
					}
				}
				else if (gad->GadgetID == DRAWER_ID)
				{	if (ctrl->Files)
					{	if (ctrl->DrawerZone != NULL)
						{	if (lock = LockDir(ctrl->DrawerZone->Buffer,ACCESS_READ))
							{	free_matches(ctrl->Files);
								UnLock(ctrl->Lock);

								ctrl->Lock = lock;

								set_busy();
								ctrl->Files = match_files(ctrl->Lock,allpat_tx,ctrl->Match,
									&ctrl->Count);
								clear_busy();

								ctrl->Start = 0;

								show_files(FindGadget(window,LIST_ID),ctrl);
								set_slider(FindGadget(window,SLIDER_ID),ctrl);	
							}
							else
							{	DisplayBeep(window->WScreen);
								expand_zone(ctrl->DrawerZone,ctrl->Lock);
							}
						}
					}
				}
				else if (gad->GadgetID == HELP_ID ||
					gad->GadgetID == PROCEED_ID ||
					gad->GadgetID == ABORT_ID ||
					gad->GadgetID == CREATE_ID) return gad;
#ifdef ONLY2_0
				else if (gad->GadgetID < FIRSTRESV_ID &&
					(gad->GadgetType & GTYP_GTYPEMASK) == CUSTOMGADGET) return gad;
#else
				else if (gad->GadgetID < FIRSTRESV_ID &&
					(gad->GadgetType & GTYP_GTYPEMASK) == BOOLGADGET) return gad;
#endif
				break;

				case MOUSEMOVE:
				moved = 1;
				break;

				case MOUSEBUTTONS:
				if (code == SELECTUP)
				{	if (arrow != 0)
					{	arrow = 0;
						ModifyIDCMP(window,STD_FLAGS);	
					}
					if (insert)
					{	insert = 0;
						ModifyIDCMP(window,STD_FLAGS);	
					}
				}
				if (code == SELECTDOWN) return NULL;
				break;

				case DISKINSERTED:
				return (struct Gadget *)-1;

				case INTUITICKS:
				if (arrow && ++ticks == 2)
				{
					ticks = 0;
					i = ctrl->Start + arrow;
					if (i >= 0 && i <= ctrl->Count - ctrl->Visible)
					{
						ctrl->OldStart = ctrl->Start;
						ctrl->Start = i;
						if (ctrl->Files)
							show_files(FindGadget(window,LIST_ID),ctrl);
						else show_text(FindGadget(window,LIST_ID),ctrl,0);
						set_slider(FindGadget(window,SLIDER_ID),ctrl);
					}
				}
				if (insert)
				{	gad = FindGadget(window,LIST_ID);
					i = which_line(gad,ctrl,window->MouseY);
					if (i != ctrl->Hilite)
					{	erase_text(gad,ctrl);
						ctrl->Hilite = i;
						show_text(gad,ctrl,1);
					}
				}
				break;
			}
		}

		if (moved)
		{
			gad = FindGadget(window,SLIDER_ID);
			prop = (struct PropInfo *)gad->SpecialInfo;

			newscroll = prop->VertPot * (ctrl->Count-ctrl->Visible) / MAXBODY;
			if (newscroll != ctrl->Start)
			{
				ctrl->OldStart = ctrl->Start;
				ctrl->Start = newscroll;
				if (ctrl->Files)
					show_files(FindGadget(window,LIST_ID),ctrl);
				else show_text(FindGadget(window,LIST_ID),ctrl,0);
			}
		}

		Wait(1 << window->UserPort->mp_SigBit);
	}
}

/* this routine is called by those who cannot handle a -1 (DISKINSERT) value */

struct Gadget *HandleEvents(struct ListControl *ctrl)
{
	struct Gadget	*g;

	while (1)
	{
		g = HandleAnyEvent(ctrl);

		if (g != (struct Gadget *)-1) break;
	}

	return g;
}

	/* check to see if user hit the ESCAPE key or the copy abort gadget */

BOOL check_escape(void)
{
	struct IntuiMessage *imsg;
	ULONG	class;
	UWORD	code;
	struct Gadget *gad;

	while (imsg = (struct IntuiMessage *)GetMsg(window->UserPort))
	{	class = imsg->Class;
		code = imsg->Code;
		gad = (struct Gadget *)imsg->IAddress;
		ReplyMsg(&imsg->ExecMessage);

		if (class == RAWKEY && code == RAW_ESC) return TRUE;

		if (class == GADGETUP && gad->GadgetID == COPYABORT_ID) return TRUE;

		/* also could process ACTIVE/INACTIVEWINDOW and change window text */
	}

	return FALSE;
}

	/*	backsave the current page, set-up for sub-page and call function	*/
	/*	(don't ask me why I didn't use a requester...)						*/

int save_page(struct InstallationRecord *ir,char *title,
	int (*func)(struct InstallationRecord *,char *))
{	struct RastPort srp;
	struct BitMap sbm;
	struct Gadget *sgad = first_gadget;
	WORD	bcount = button_count,
		 	tcount = textzone_count,
			ccount = checkmark_count;
	int		result;

		/* allocate a place to store the display */
	if (MakeBitMap(&sbm,window->WScreen->BitMap.Depth,
		window_width - left_edge - right_edge,
		window_height - top_edge - bottom_edge) == 0) return 0;

	if (sgad) remove_gadgets(sgad);		/* remove old gadget list */

		/* save the display */
	InitRastPort(&srp);
	srp.BitMap = &sbm;
	ClipBlit(rp,left_edge,top_edge,&srp,0,0,
		window_width - left_edge - right_edge,
		window_height - top_edge - bottom_edge,0xc0);

	result = func(ir,title);

		/* restore display */
	ClipBlit(&srp,0,0,rp,left_edge,top_edge,
		window_width - left_edge - right_edge,
		window_height - top_edge - bottom_edge,0xc0);

	if (sgad)
	{		/* restore old gadgets */
		first_gadget = sgad;
		AddGList(window,sgad,-1,-1,NULL);
	}

		/* free memory */
	UnMakeBitMap(&sbm);

	button_count = bcount;
	textzone_count = tcount;
	checkmark_count = ccount;

	return result;
}

	/* handle the extra gadgets associated with the "ask_disk" page */

int do_insert_disk(struct InstallationRecord *ir,char *title)
{
	while (TRUE)
	{	struct Gadget	*gad;

		if (gad = HandleAnyEvent(NULL))
		{
			if (gad != (struct Gadget *)-1)		/* not DISKINSERTED */
			{
				if (gad->GadgetID == HELP_ID && title != NULL)
				{
					save_page(ir,title,help_page);
				}
				if (gad->GadgetID == PROCEED_ID || gad->GadgetID == ABORT_ID)
				{
					return (int)gad->GadgetID;
				}
			}

				/* comment this out to remove auto-proceed on disk insert */

			if (VolumeMounted(STR_VAL(ir->dest_val.v),
				ir->flags & IFLAG_OPTIONAL ? LDF_VOLUMES|LDF_ASSIGNS :
				LDF_VOLUMES)) return PROCEED_ID;
		}
	}
}

	/* handle the standard gadgets associated with a standard page */

int do_nothing(struct InstallationRecord *ir,char *title,LONG mask)
{
	while (TRUE)
	{	struct Gadget	*gad;

		if (gad = HandleEvents(NULL))
		{
			if (gad->GadgetID == HELP_ID && title != NULL)
			{
				save_page(ir,title,help_page);
			}
			if (gad->GadgetID == PROCEED_ID || gad->GadgetID == ABORT_ID)
			{
				return (int)gad->GadgetID;
			}
			if (mask & 1 << gad->GadgetID) return (int)gad->GadgetID;
		}
	}
}

	/* handle the extra gadgets associated with change_startup page */

int do_textlist(struct InstallationRecord *ir,char *title)
{
	char				str[120],
						*dtext;
	struct ListControl	ctrl;
	struct Gadget		*gad;

	gad = FindGadget(window,DRAWER_ID);
	dtext = (char *)((struct StringInfo *)gad->SpecialInfo)->Buffer;
	strcpy(str,dtext);

	zero(&ctrl,sizeof ctrl);
	gad = FindGadget(window,LIST_ID);
	init_text(gad,&ctrl);
	show_text(gad,&ctrl,0);
	set_slider(FindGadget(window,SLIDER_ID),&ctrl);

	while (TRUE)
	{
		if (gad = HandleEvents(&ctrl))
		{
			if (gad->GadgetID == PROCEED_ID || gad->GadgetID == ABORT_ID)
			{
				if (gad->GadgetID == PROCEED_ID)
					textloc = final_text(FindGadget(window,LIST_ID),&ctrl);

				return (int)gad->GadgetID;
			}

			if (gad->GadgetID < 10) return (int)gad->GadgetID;

			if (gad->GadgetID == HELP_ID && title != NULL)
			{
				save_page(ir,title,help_page);
			}
		}
	}
}

	/* setup code used by both directory and file display pages */

BOOL setup_list(struct ListControl *ctrl,char *path,BOOL repl)
{	BPTR 			lock1, lock2;
	char			*str,
					*end;
	BOOL			many,
					refresh = FALSE,
					drives = FALSE;
	struct Process	*pr;
	APTR			prwin;

	unless ( (str = AllocMem(MAX_STRING,0L)) ) return FALSE;

	pr = (struct Process *)FindTask(NULL);
	prwin = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR)-1;

	unless (lock2 = LockDir(path,ACCESS_READ))
	{
		strcpy(str,path);

		while (1)
		{
			end = FileOnly(str);

			many = (end-- != str);

			if (many == TRUE)
			{
				if (*end == ':')
				{
					end[1] = 0;
					many = FALSE;
				}
				else end[0] = 0;
			}

			if (lock2 = LockDir(str,ACCESS_READ))
			{
				if (repl) strcpy(path,str);
				refresh = TRUE;			
				break;
			}

			if (many == FALSE)
			{
				if (*end == ':') drives = TRUE;
				else
				{
					if (repl) strcpy(path,str);
					refresh = TRUE;
				}
				break;
			}
		}
	}

	pr->pr_WindowPtr = prwin;

	if (drives == TRUE)
	{
		ctrl->Files = get_drives(&ctrl->Count);
	}
	else
	{
		lock1 = CurrentDir(lock2);
		ctrl->Lock = DupLock(lock2 ? lock2 : lock1);

		set_busy();
		ctrl->Files = match_files(ctrl->Lock,allpat_tx,ctrl->Match,&ctrl->Count);
		clear_busy();

		CurrentDir(lock1);
		if (lock2) UnLock(lock2);
	}

	FreeMem(str,MAX_STRING);

	return refresh;
}

	/* handle extra gadgets associated with select_dir page */

int do_drawers(struct InstallationRecord *ir,char *title)
{	struct ListControl ctrl;
	struct Gadget	*gad = FindGadget(window,DRAWER_ID);
	struct String	*name,*temp;
	WORD			i;

	zero(&ctrl,sizeof ctrl);
	ctrl.Match = UNMATCH_INFO_FILES|MATCH_DIRS_ONLY;

	if (ir->flags & IFLAG_OPTIONAL)
	{
		ctrl.Files = get_drives(&ctrl.Count);
	}
	else
	{
		if ( setup_list(&ctrl,
			(char *)((struct StringInfo *)gad->SpecialInfo)->Buffer,
			!(ir->flags & IFLAG_SAFE)) )
		{
			i = RemoveGList(window,gad,1);
			AddGadget(window,gad,i);
			RefreshGList(gad,window,NULL,1L);		
			ActivateGadget(gad,window,NULL);
		}
	}

	ctrl.Start = 0;
	ctrl.OldStart = -1;
	ctrl.Mode = 1;
	ctrl.DrawerZone = (struct TextZone *)FindGadget(window,DRAWER_ID);
	ctrl.FileZone = NULL;

	show_files(FindGadget(window,LIST_ID),&ctrl);
	set_slider(FindGadget(window,SLIDER_ID),&ctrl);

	while (TRUE)
	{	struct Gadget	*gad;

		unless (gad = HandleEvents(&ctrl)) continue;

		if (gad->GadgetID == PROCEED_ID || gad->GadgetID == ABORT_ID)
		{
			if (gad->GadgetID == PROCEED_ID)
			{
				/* check if text in string gadget REALLY a drawer */
			}

			if (gad->GadgetID == ABORT_ID)
			{	extern BOOL	dir_special;	/* ichy global varaible */

				if (!dir_special && !aborterr(FUNC_ASKDIR)) continue;
			}

			free_matches(ctrl.Files);
			if (ctrl.Lock) UnLock(ctrl.Lock);
			if (ctrl.LastLock) UnLock(ctrl.LastLock);
			return (int)gad->GadgetID;
		}

		if (gad->GadgetID == HELP_ID && title != NULL)
		{
			save_page(ir,title,help_page);
		}

		if (gad->GadgetID == CREATE_ID)
		{
			gad = FindGadget(window,DRAWER_ID);
			temp = new_string((char *)((struct StringInfo *)gad->SpecialInfo)->Buffer);
			if (temp != NULL)
			{	WORD	t = ir->numchoices;

				name = temp;
				save_page(ir,(char *)&name,makedir_page);
				MemFree(temp);

				if (name != NULL)
				{	if (!P_PRETEND_MODE)
					{	if (create_dir(STR_VAL(name),ir->numchoices))
						{	BPTR lock;
						
							i = RemoveGList(window,(struct Gadget *)ctrl.DrawerZone,1);
							strcpy(ctrl.DrawerZone->Buffer,STR_VAL(name));
							if (lock = LockDir(ctrl.DrawerZone->Buffer,ACCESS_READ))
							{	free_matches(ctrl.Files);
								UnLock(ctrl.Lock);

								ctrl.Lock = lock;

								set_busy();
								ctrl.Files = match_files(ctrl.Lock,allpat_tx,ctrl.Match,
									&ctrl.Count);
								clear_busy();

								ctrl.Start = 0;

								show_files(FindGadget(window,LIST_ID),&ctrl);
								set_slider(FindGadget(window,SLIDER_ID),&ctrl);	
							}
							else
							{	DisplayBeep(window->WScreen);
								ExpandPath((void *)ctrl.Lock,ctrl.DrawerZone->Buffer,
									ctrl.DrawerZone->Info.MaxChars);
							}
							AddGadget(window,(struct Gadget *)ctrl.DrawerZone,i);
							RefreshGList((struct Gadget *)ctrl.DrawerZone,window,NULL,1L);

							transcript(trans_made_drawer,STR_VAL(name));
						}
					}
					MemFree(name);
				}
				ir->numchoices = t;
			}
		}
	}
}

	/* handle extra gadgets associated with select_file page */

int show_filelist(struct InstallationRecord *ir,char *title)
{	struct ListControl	ctrl;
	struct Gadget		*gad = FindGadget(window,DRAWER_ID);
	WORD				i;

	zero(&ctrl,sizeof ctrl);
	ctrl.Match = UNMATCH_INFO_FILES|MATCH_FILES_DIRS;

	if (ir->flags & IFLAG_OPTIONAL)
	{
		ctrl.Files = get_drives(&ctrl.Count);
	}
	else
	{
		if ( setup_list(&ctrl,
			(char *)((struct StringInfo *)gad->SpecialInfo)->Buffer,
			!(ir->flags & IFLAG_SAFE)) )
		{
			i = RemoveGList(window,gad,1);
			AddGadget(window,gad,i);
			RefreshGList(gad,window,NULL,1L);		
		}
	}

	ctrl.Start = 0;
	ctrl.OldStart = -1;
	ctrl.Mode = 0;
	ctrl.DrawerZone = (struct TextZone *)FindGadget(window,DRAWER_ID);
	ctrl.FileZone = (struct TextZone *)FindGadget(window,FILENAME_ID);

	show_files(FindGadget(window,LIST_ID),&ctrl);
	set_slider(FindGadget(window,SLIDER_ID),&ctrl);

	while (TRUE)
	{
		unless (gad = HandleEvents(&ctrl)) continue;

		if (gad->GadgetID == PROCEED_ID || gad->GadgetID == ABORT_ID)
		{
			if (gad->GadgetID == ABORT_ID && !aborterr(FUNC_ASKFILE)) continue;

			free_matches(ctrl.Files);
			if (ctrl.Lock) UnLock(ctrl.Lock);
			if (ctrl.LastLock) UnLock(ctrl.LastLock);
			return (int)gad->GadgetID;
		}

		if (gad->GadgetID == HELP_ID && title != NULL)
		{
			save_page(ir,title,help_page);
		}
	}
}

/* ================== Layout schemes for individual pages ===================== */

/* Abort Copy Sub-Page:

	|---------------------------|
	|   	Copying file:		|
	|			 file			|
	|		  To drawer:		|
	|			drawer			|
	|							|
	|	[****** gauge ******]	|
	|							|
	|			Abort			|
	|---------------------------|
*/

static struct Gadget	*abortcopy_base;
static BOOL				abortcopy_active = 0;

BOOL abortcopy_page(WORD *counts)
{	static struct GadgetDef abortcopy_def[] = {
		{ NULL, 		GTYPE_BEVEL| GBEVEL_SAVE,				0		},
		{ "",			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 		},
		{ (void *)TX_COPYING_FILE,
			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_BOLD | GFLAG_LOCAL, },
		{ "",			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 		},
		{ "",			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 		},
		{ (void *)TX_TO_DRAWER,
			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_BOLD | GFLAG_LOCAL, },
		{ "",			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 		},
		{ (void *)TX_ABORT,
		 	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL, COPYABORT_ID },
		{ NULL,			GTYPE_ADJUST | GADJUST_CENTER,				1	},
		{ NULL,			GTYPE_GAUGE | GFLAG_LAST,						}
	};
	struct Gadget	*sgad = first_gadget;
	struct Gadget	*gad = NULL;
	UWORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

	if (abortcopy_active) return FALSE;

	if (!show_gauge)
	{
		abortcopy_active = TRUE;
		return TRUE;
	}

	counts[0] = button_count;
	counts[1] = textzone_count;
	counts[2] = checkmark_count;

	if (sgad) remove_gadgets(sgad);		/* remove old gadget list */

	make_gadgets(&gad,abortcopy_def,NULL,GSET_GADGETS);

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	abortcopy_base = sgad;
	abortcopy_active = TRUE;
	return TRUE;
}

struct Gadget *abortcopy_display(char *src,char *dest)
{	static struct GadgetDef abortcopy_def2[] = {
		{ NULL, GTYPE_BEVEL| GBEVEL_SCALE,								},
		{ "",	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 				},
		{ "",	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_SKIP, 	},
		{ NULL,	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_PREWIPE,	},
		{ "",	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 				},
		{ "",	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_SKIP, 	},
		{ NULL,	GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_PATH | GFLAG_LAST, }
	};
	struct Gadget	*fgad = FindGadget(window,GAUGE_ID);
	UWORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

	/* abortcopy_def2[0].ID = fgad->TopEdge - 2; */
	abortcopy_def2[3].Text = src;
	abortcopy_def2[6].Text = dest;

		/* we use fgad merely as a flag to indicate DON't reset layout */
	make_gadgets(&fgad,abortcopy_def2,NULL,0);

	SetAPen(rp,0);
	RectFill(rp,fgad->LeftEdge,fgad->TopEdge,
		fgad->LeftEdge+fgad->Width-1,fgad->TopEdge+fgad->Height-1);
	SetAPen(rp,highlight_color);

	/* clear_busy(); */		/* commented out to reduce flashies */

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	return fgad;
}

void abortcopy_clear(WORD *counts)
{
	if (show_gauge)
	{
		clear_gadgets(first_gadget);

		if (abortcopy_base)
		{		/* restore old gadgets */
			first_gadget = abortcopy_base;
			AddGList(window,first_gadget,-1,-1,NULL);
			abortcopy_base = NULL;
		}

		button_count = counts[0];
		textzone_count = counts[1];
		checkmark_count = counts[2];
	}

	abortcopy_active = FALSE;
}

/* Make Directory Sub-Page:

	|---------------------------|
	|   	Make Directory		|
	|		makedir_help		|
	|							|
	|							|
	|		[] Make Icon		|
	|							|
	|   Enter Complete Path:	|
	|	[___________________]	|
	|							|
	| Proceed			Cancel	|
	|---------------------------|
*/

int makedir_page(struct InstallationRecord *ir,char *newdir)
{	static struct GadgetDef makedir_def[] = {
		{ NULL, 		GTYPE_BEVEL| GBEVEL_SAVE,				0			},
		{ (void *)TX_MAKEDIR, GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_BOLD | GFLAG_LOCAL, },
		{ (void *)TX_MAKEDIR_INFO,GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_LOCAL,	},
		{ (void *)TX_PROCEED, GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL, PROCEED_ID	},
		{ (void *)TX_CANCEL, GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL, ABORT_ID	},
		{ NULL,			GTYPE_ADJUST | GADJUST_BOTTOM,						},
		{ NULL, 		GTYPE_STRGAD | GPUT_LOWER | GSTRING_ACTIVE,	1		},
		{ (void *)TX_ENTER_PATH,	GTYPE_TEXT | GPUT_LOWER | GTEXT_CENTER | GTEXT_BOLD | GFLAG_LOCAL, },
		{ NULL, 		GTYPE_CHECKBOX | GFLAG_LAST,			2			}
	};
	struct MultiData data;
	struct Gadget	*sgad,
					*gad = NULL;
	struct String	*text,
					**dir = (struct String **)newdir;
	int				result;
	WORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

	makedir_def[5].ID = rp->TxHeight + 1;
	makedir_def[6].Text = STR_VAL(*dir);
	text = (struct String *)GetLocalText(STR_CREATE_ICON);
	data.Choices = &text;
	data.NumChoices = 1;
	data.DefChoice = -1;
	makedir_def[8].Text = &data;
	make_gadgets(&gad,makedir_def,ir,GSET_GADGETS);

	sgad = FindGadget(window,1);

	while (TRUE)
	{	struct Gadget *egad;

		if (egad = HandleEvents(NULL))
		{
			if (egad->GadgetID == PROCEED_ID) { result = 1; break; }
			if (egad->GadgetID == ABORT_ID) { result = 0; break; }
		}
	}

	clear_gadgets(gad);

	if (result == 0) *dir = NULL;
	else
	{	*dir = new_string((char *)((struct StringInfo *)sgad->SpecialInfo)->Buffer);
		ir->numchoices = (gad->Flags & SELECTED ? 1 : 0);
	}

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	return result;
}

/* Ask Delete Page:

	|---------------------------|
	|   	   Prompt			|
	|							|
	| Overwrite			Cancel	|
	|			Help			|
	|---------------------------|

*/

int askdelete_page(struct InstallationRecord *ir,char *file)
{	static struct GadgetDef askdelete_def[] = {
		{ NULL, 		GTYPE_BEVEL| GBEVEL_SAVE,				0			},
		{ NULL,			GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER,							},
		{ (void *)TX_DELETE, GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL, PROCEED_ID	},
		{ (void *)TX_SKIP_FILE, GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL | GFLAG_LAST, ABORT_ID	}
	};
	char			str[128];
	struct Gadget	*gad = NULL;
	int				result;
	WORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

	Sprintf(str,GetLocalText(MSG_ASKDELETE),file);

	askdelete_def[1].Text = str;
	make_gadgets(&gad,askdelete_def,NULL,GSET_GADGETS | GCLEAR_BUSY);
	clear_busy();

	while (TRUE)
	{	struct Gadget *egad;

		if (egad = HandleEvents(NULL))
		{
			if (egad->GadgetID == PROCEED_ID) { result = 1; break; }
			if (egad->GadgetID == ABORT_ID) { result = 0; break; }
		}
	}

	clear_gadgets(gad);
	set_busy();

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	return result;
}

/* Copy File Page (copyfiles):

	|---------------------------|
	|   	   Prompt			|
	|							|
	||-------------------------||
	||  [] File		  [] File  ||
	||  [] File		  [] File  ||
	||-------------------------||
	|							|
	|    Destination Drawer:	|
	|	[___________________]	|
	|							|
	| Proceed			Skip	|
	| Change Dest		Abort	|
	| 			Help			|
	|---------------------------|

    Parameters are:
        prompt
		help
        source - name of source directory or file
        dest - name of destination directory, which is created if it doesn't
			exist
        newname - if copying one file only, and file is to be renamed, this is
            the new name
        choices - a list of files/directories to be copied (optional)
        all - all files/directories in the source directory should be copied
        pattern - indicates that files/directories from the source dir
			matching a pattern should be copied
		files - only copy files. By default the installer will match and copy
			subdirectories.
        infos - switch to copy icons along with other files/directories
		confirm -  if this option is present, user will be prompted to indicate
			which files are to be copied, else the files will be copied silently.
		safe - copy files even if in PRETEND mode.
*/

BOOL copy_extra_file(struct InstallationRecord *ir, char *destdir, char *name,
	char *newname, char *ext)
{
	char	filename[160],
			xname[32];
	int		r;

	strcpy(filename,name);
	strcat(filename,ext);
	if (newname)
	{	strcpy(xname,newname);
		strcat(xname,ext);
	}

	if (!P_PRETEND_MODE)
	{
		r = copy_file(filename,destdir,newname ? xname : NULL,ir->bits & IBITS_NOPOSITION);
		if (ierror) return FALSE;
	}
	else r = 1;

	if (r) transcript(trans_copy_file,filename,destdir);

	return TRUE;
}

struct String *copy_page(struct InstallationRecord *ir)
{	static struct GadgetDef copy_def[] = {
		{ &gir_prompt,	GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER | GFLAG_PTR,	},
		{ (void *)TX_HELP, GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL,HELP_ID},
		{ (void *)TX_CHANGE_DEST, GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL, 4		},
		{ (void *)TX_ABORT,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	ABORT_ID},
		{ (void *)TX_PROCEED_COPY,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL, PROCEED_ID	},
		{ (void *)TX_SKIP,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	1		},
		{ NULL,			GTYPE_STRGAD | GPUT_LOWER | GSTRING_ACTIVE,	DRAWER_ID	},
		{ (void *)TX_COPY_DEST,GTYPE_TEXT|GPUT_LOWER|GTEXT_CENTER|GTEXT_BOLD|GFLAG_LOCAL,},
		{ NULL,		GTYPE_CHECKBOX | GCHECK_SPECIAL1,			5			},
		{ NULL,		GTYPE_BEVEL | GBEVEL_STD | GFLAG_LAST,		0			}
	};
	struct MultiData data;
	struct Gadget	*gad,*agad;
	LONG			count,num,
					first = 0;
	struct String	**choices = NULL,
					*source;
	char			destdir[160],
					*newname = NULL,
					*pattern;
	struct FileInfoBlock  *fib;
	BPTR			olddir = NULL,
					lock3 = NULL;
	int				r, what;
	UWORD			id;
	WORD			gcounts[3];

	show_gauge = !(ir->bits & IBITS_NOGAUGE);

	strcpy(destdir,STR_VAL(ir->dest_val.v));
	if (!strchr(destdir,':'))		/* no colon, must be a relative path */
	{
		r = 0;

		if (lock3 = Lock(destdir,ACCESS_READ))
		{
			r = ExpandPath((void *)lock3,destdir,160);
			UnLock(lock3);
		}

		if (r == 0)
		{
			err_msg(ehead,ERR_DOS,FUNC_COPYFILES,GetLocalText(MSG_NOEXIST_OBJECT));
			return 0;
		}
	}

	fib = MemAlloc(sizeof *fib,MEMF_CLEAR|MEMF_PUBLIC);
	if (fib == NULL)
	{	memerr();
		return 0;
	}

	unless (lock3 = Lock(STR_VAL(ir->source_val.v),ACCESS_READ))
	{
		err_msg(ehead,ERR_DOS,FUNC_COPYFILES,prob_with_source);
		MemFree(fib);
		return 0;
	}

	if (Examine(lock3,fib) == 0)
	{
		err_msg(ehead,ERR_DOS,FUNC_COPYFILES,prob_with_source);
		MemFree(fib);
		UnLock(lock3);
		return 0;
	}

	what = fib->fib_DirEntryType;
	MemFree(fib);

	if (what > 0)			/* directory */
	{
		if (ir->choices == NULL)
		{
			if (!(ir->flags & IFLAG_ALL) &&
				!(ir->pattern_val.type == TYPE_STRING))
			{
				err_msg(ehead,ERR_SCRIPT,FUNC_COPYFILES,bad_params_copy);
				UnLock(lock3);
				return 0;
			}

			olddir = CurrentDir(lock3);

			if ( !(ir->flags & IFLAG_CONFIRM) && (ir->flags & IFLAG_ALL) )
			{
				if (!P_PRETEND_MODE)
				{
						/* recursive directory copy */
					abortcopy_page(gcounts);
					local_delopts = ir->delopts;
					r = copy_tree(lock3,destdir);
					abortcopy_clear(gcounts);

					if (r != 0 && ir->flags & IFLAG_INFOS)
						create_drawer_icon(destdir);
				}

				CurrentDir(olddir);
				UnLock(lock3);

				if (r == 0) return NULL;

				transcript(trans_copy_drawer,STR_VAL(ir->source_val.v),destdir);

				return TempString(destdir, strlen(destdir), NULL);
			}

				/* pattern matching choices */
			pattern =
				((ir->flags & IFLAG_ALL) ? allpat_tx : STR_VAL(ir->pattern_val.v));

			choices = match_files(lock3,pattern,
				(ir->flags & IFLAG_INFOS ? MATCH_FREE_INFOS : 0) |
				(ir->flags & IFLAG_FONTS ? UNMATCH_FONT_FILES : 0) |
				(ir->flags & IFLAG_NODIRS ? MATCH_FILES_ONLY : MATCH_FILES_DIRS),
				&count);
		}
		else
		{
			olddir = CurrentDir(lock3);
			choices = ir->choices;		/* explicit choices */
			unless (count = ir->numchoices) goto bye;
		}
	}
	else
	{
		UnLock(lock3);
		lock3 = NULL;

		source = ir->source_val.v;
		choices = &source;
		count = 1;
		if (ir->name_val.type == TYPE_STRING) newname = STR_VAL(ir->name_val.v);
	}

	if ( !(ir->flags & IFLAG_CONFIRM) )
	{
		if (!P_PRETEND_MODE) abortcopy_page(gcounts);

		r = 0;
		while (count--)
		{
			if (!P_PRETEND_MODE)
			{
				local_delopts = ir->delopts;
				if (copy_object((char *)(choices[first]+1),destdir,newname,0)
					== 0) break;

				if (r == 0 && ir->flags & IFLAG_INFOS)
					create_drawer_icon(destdir);

				r = 1;
			}

			transcript(trans_copy_file,choices[first]+1,destdir);

			if (ir->flags & IFLAG_INFOS)
			{
				local_delopts = ir->delopts | DELOPT_OKNOSRC;
				if (!copy_extra_file(ir,destdir,(char *)(choices[first]+1),
					newname,dot_info)) break;
			}
			if (ir->flags & IFLAG_FONTS)
			{
				local_delopts = ir->delopts | DELOPT_OKNOSRC;
				if (!copy_extra_file(ir,destdir,(char *)(choices[first]+1),
					newname,dot_font)) break;
			}
			first++;
		}

		if (!P_PRETEND_MODE) abortcopy_clear(gcounts);
	}
	else while (first < count)
	{

redisplay:
		gad = NULL;

		copy_def[6].Text = destdir;
		data.DefChoice = -1;
		data.NumChoices = count - first;
		data.Choices = &choices[first];
		copy_def[8].Text = &data;
		num = make_gadgets(&gad,copy_def,ir,
			GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

input:
		r = do_nothing(ir,help_copyfiles,(1<<1)|(1<<4));

		if (r == 4)
		{	extern BOOL	dir_special;	/* ichy global variable */
			struct InstallationRecord ir2;
			char	*str;

				/* remove current gadgets */
			clear_gadgets(gad);

			ir2.prompt_val.v = (struct String *)GetLocalText(STR_NEW_DEST);
			ir2.help_val.v = (struct String *)GetLocalText(HELP_SELECT_DRAWER);
			ir2.default_val.v = new_string(destdir);
			ir2.prompt_val.type = ir2.help_val.type = 
				ir2.default_val.type = TYPE_STRING;

			dir_special = TRUE;
			str = select_dir(&ir2);
			dir_special = FALSE;

			MemFree(ir2.default_val.v);
			if (str != NULL) strcpy(destdir,str);
			goto redisplay;
		}

		agad = FindGadget(window,DRAWER_ID);
		strcpy(destdir,(char *)((struct StringInfo *)agad->SpecialInfo)->Buffer);

		if (r == 1)			/* skip gadget */
		{
			first += num;
			num = 0;
			disable_gadgets(gad);
			goto skip;
		}

		if (r == ABORT_ID)
		{	if (!aborterr(FUNC_COPYFILES)) goto input;

			RemoveGList(window,gad,-1);
			break;
		}

			/* if IFLAG_INFOS, need to copy .info files also */
		disable_gadgets(gad);
		id = 5;
		r = 0;
		if (!P_PRETEND_MODE) abortcopy_page(gcounts);
		while (num--)
		{
			agad = FindGadgetB(gad,id++);
			if (agad->Flags & SELECTED)
			{
				set_busy();

				if (!P_PRETEND_MODE)
				{
					local_delopts = ir->delopts;
					if (copy_object((char *)(choices[first]+1),destdir,newname,0)
						== 0)
					{	num = 1;
						break;
					}

					if (r == 0 && ir->flags & IFLAG_INFOS)
						create_drawer_icon(destdir);
				}

				transcript(trans_copy_file,choices[first]+1,destdir);

				if (ir->flags & IFLAG_INFOS)
				{
					local_delopts = ir->delopts | DELOPT_OKNOSRC;
					if (!copy_extra_file(ir,destdir,(char *)(choices[first]+1),
						newname,dot_info)) { num = 1; break; }
				}
				if (ir->flags & IFLAG_FONTS)
				{
					local_delopts = ir->delopts | DELOPT_OKNOSRC;
					if (!copy_extra_file(ir,destdir,(char *)(choices[first]+1),
						newname,dot_font)) { num = 1; break; }
				}
			}
			first++;
		}
		if (!P_PRETEND_MODE) abortcopy_clear(gcounts);

skip:	RemoveGList(window,gad,-1);

		if (num > 0) break;
	}

bye:
	first_gadget = NULL;

	if ((ir->flags & IFLAG_ALL) || ir->pattern_val.type == TYPE_STRING)
		free_matches(choices);

	EmptyPort(window->UserPort);
	if (lock3)
	{
		if (olddir) CurrentDir(olddir);
		UnLock(lock3);
	}

	set_busy();

	return TempString(destdir, strlen(destdir), NULL);
}

	/* this set of gadget definitions is used by thenext few pages */

static struct GadgetDef many_def[] = {
	{ &gir_prompt,	GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_PTR,		},
	{ (void *)TX_HELP,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL,HELP_ID	},
	{ (void *)TX_PROCEED,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	PROCEED_ID	},
	{ (void *)TX_ABORT,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	ABORT_ID	},
	{ NULL,			GTYPE_NONE,									2			},
	{ NULL,			GTYPE_BEVEL | GBEVEL_STD | GFLAG_LAST,		0			}
};

/* Radio Page (askchoice):

	|---------------------------|
	|   	   Prompt			|
	|							|
	||-------------------------||
	||       [] Choice		   ||
	||       [] Choice		   ||
	||-------------------------||
	|							|
	| Proceed			Abort	|
	| 			Help			|
	|---------------------------|

    Parameters:
        prompt
		help
        choices - a list of choice strings, such as "ok" "cancel", etc.
        default - the number of the default choice (defaults to 0)
*/

int radio_page(struct InstallationRecord *ir)
{	struct Gadget		*radio,
						*gad = NULL;
	struct MultiData	data;
	int					r, num;

	data.DefChoice = (long)ir->default_val.v;
	if ((data.NumChoices = ir->numchoices) == 0)
	{
		/* english */

		err_msg(ehead,ERR_SCRIPT,FUNC_ASKCHOICE,"No choices offered");		
	}

	data.Choices = (struct String **)ir->choices;
	many_def[4].Text = &data;
	many_def[4].Flags = GTYPE_RADIO;
	num = make_gadgets(&gad,many_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GCLEAR_BUSY|GSET_GADGETS);

	radio = gad;

	while (TRUE)
	{	r = do_nothing(ir,help_with_choice,0);

		if (r == ABORT_ID && !aborterr(FUNC_ASKCHOICE)) continue;
		break;
	}

	clear_gadgets(gad);

	if (r != ABORT_ID)
	{
		r = final_radio(radio,num);
		if (r == -1)
		{
			/* english */

			err_msg(ehead,ERR_DATA,FUNC_ASKCHOICE,"No choices selected");
		}
		else
		{
			transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
			transcript(trans_ask_choice,ir->choices[r] + 1);
		}
	}

	set_busy();

	return r;
}

/* Checkbox Page (askoption):

	|---------------------------|
	|   	   Prompt			|
	|							|
	||-------------------------||
	||       [] Option		   ||
	||       [] Option		   ||
	||-------------------------||
	|							|
	| Proceed			Abort	|
	| 			Help			|
	|---------------------------|

	Parameters:
        prompt
		help - as above
        options - a list of option strings
        default - a bit mask of the buttons to be checked (defaults to -1)
*/

int checkbox_page(struct InstallationRecord *ir)
{	struct Gadget	*checks,
					*gad = NULL;
	struct MultiData	data;
	int				r, num;
	WORD			i;

	data.DefChoice = (long)ir->default_val.v;
	if ((data.NumChoices = ir->numchoices) == 0)
	{
		/* english */

		err_msg(ehead,ERR_SCRIPT,FUNC_ASKCHOICE,"No choices offered");		
	}

	data.Choices = (struct String **)ir->choices;
	many_def[4].Text = &data;
	many_def[4].Flags = GTYPE_CHECKBOX;
	num = make_gadgets(&gad,many_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GCLEAR_BUSY|GSET_GADGETS);

	checks = gad;

	while (TRUE)
	{	r = do_nothing(ir,help_with_options,0);

		if (r == ABORT_ID && !aborterr(FUNC_ASKOPTIONS)) continue;
		break;
	}

	clear_gadgets(gad);

	if (r != ABORT_ID)
	{
		r = final_checkboxes(checks,num);
		transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		if (r == 0) transcript(trans_no_options);
		else
		{
			transcript(trans_ask_options);
			for (i=0;i<32;i++)
				if (r & 1 << i) transcript("  %s\n",STR_VAL(ir->choices[i]));
		}
	}

	set_busy();

	return r;
}

	/* these gadget definitions are used by the next few pages */

static struct GadgetDef askstd_def[] = {
	{ &gir_prompt,	GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_PTR,		},
	{ (void *)TX_HELP,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL,HELP_ID		},
	{ (void *)TX_PROCEED,GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	PROCEED_ID	},
	{ (void *)TX_ABORT,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	ABORT_ID	},
	{ NULL,			GTYPE_ADJUST | GADJUST_CENTER,				1			},
	{ &gir_deftool,	GTYPE_STRGAD | GFLAG_PTR | GSTRING_ACTIVE | GFLAG_LAST,	1	}
};

/* AskString Page (askstring):

	|---------------------------|
	|   	   Prompt			|
	|							|
	|  [_______string________]  |
	|							|
	| Proceed			Abort	|
	| 			Help			|
	|---------------------------|

	Parameters:
        prompt
		help
        default - the default text string.
*/

char *askstring_page(struct InstallationRecord *ir)
{	struct Gadget	*agad,
					*gad = NULL;
	char 			*str;
	int				r;

	askstd_def[5].Flags = GTYPE_STRGAD | GFLAG_PTR | GSTRING_ACTIVE | GFLAG_LAST;
	make_gadgets(&gad,askstd_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GCLEAR_BUSY|GSET_GADGETS);

	while (TRUE)
	{
		r = do_nothing(ir,help_ask_string,0);

		if (r == ABORT_ID && !aborterr(FUNC_ASKSTRING)) continue;
		break;
	}

	agad = FindGadget(window,1);

	clear_gadgets(gad);

	if (r == ABORT_ID) str = NULL;
	else
	{
		str = (char *)((struct StringInfo *)agad->SpecialInfo)->Buffer;
		transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		transcript(trans_ask_string,str);
	}

	set_busy();

	return str;
}

/* AskNumber Page (asknumber):

	|---------------------------|
	|   	   Prompt			|
	|							|
	|  [_______number________]  |
	|							|
	| Proceed			Abort	|
	| 			Help			|
	|---------------------------|

	Parameters:
        prompt
		help
        default - default value
*/

long asknumber_page(struct InstallationRecord *ir)
{	struct Gadget	*agad,
					*gad = NULL;
	char			text[80];
	long			val;
	int				r;

	askstd_def[5].Flags = GTYPE_INTGAD | GSTRING_ACTIVE | GFLAG_LAST;
	make_gadgets(&gad,askstd_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GCLEAR_BUSY|GSET_GADGETS);

	agad = FindGadget(window,1);

	if (ir->maxval != (LONG)0x7fffffff)
	{
		place_top = agad->TopEdge + agad->Height + 4;
		Sprintf(text,valid_range,ir->minval,ir->maxval);
		layout_text_length(text, strlen(text), 0, TEXT_CENTER, 0);
	}

	while (TRUE)
	{	r = do_nothing(ir,help_ask_number,0);

		if (r == ABORT_ID && !aborterr(FUNC_ASKNUMBER)) continue;

		val = ((struct StringInfo *)agad->SpecialInfo)->LongInt;
		if (val < ir->minval || val > ir->maxval)
		{
			DisplayBeep(NULL);
			set_integer_gadget(agad,(LONG)ir->deftool_val.v);
			continue;
		}

		break;
	}

	clear_gadgets(gad);

	if (r == ABORT_ID) val = 0;
	else
	{
		transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		transcript(trans_ask_number,val);
	}

	set_busy();

	return val;
}

/* AskDisk Page (askdisk):

	|---------------------------|
	|   	   Prompt			|
	|							|
	|							|
	| Proceed			Abort	|
	| 			Help			|
	|---------------------------|

	Parameters:
        prompt
		help
		dest - the volume name of the disk to be inserted
        newname - a name to assign to the disk for future reference. This
			assignment is done even in Dry Run mode -- it is considered
			"safe"
*/

long askdisk_page(struct InstallationRecord *ir)
{
	struct Gadget	*gad = NULL;
	int				r;
	BOOL			warned = FALSE;
	short			top;

	if (VolumeMounted(STR_VAL(ir->dest_val.v),
		ir->flags & IFLAG_OPTIONAL ? LDF_VOLUMES|LDF_ASSIGNS : LDF_VOLUMES))
			return 1;

	askstd_def[5].Flags = GTYPE_NONE | GFLAG_LAST;
	make_gadgets(&gad,askstd_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	top = place_top;
	ModifyIDCMP(window,STD_FLAGS | DISKINSERTED);

	while (TRUE)
	{	r = do_insert_disk(ir,help_insert_disk);

		if (r == ABORT_ID && !aborterr(FUNC_ASKDISK)) continue;

		if (r == PROCEED_ID && !VolumeMounted(STR_VAL(ir->dest_val.v),
			ir->flags & IFLAG_OPTIONAL ? LDF_VOLUMES|LDF_ASSIGNS : LDF_VOLUMES))
		{
			DisplayBeep(window->WScreen);
			if (warned == FALSE)
			{
				place_top = top;
				layout_text(GetLocalText(TX_PLEASE_INSERT),PUT_UPPER,TEXT_CENTER|TEXT_BOLD, 0);
				warned = TRUE;
			}
			continue;
		}

		break;
	}

	ModifyIDCMP(window,STD_FLAGS);
	clear_gadgets(gad);

	set_busy();

	return (r == PROCEED_ID);
}

/* YesNo Page (askbool):

	|---------------------------|
	|   	   Prompt			|
	|							|
	|							|
	| Positive		 Negative	|
	| Help				Abort	|
	|---------------------------|

	Parameters:
        prompt
		help
        default - 0 = no, 1 = yes
		choices - change the positive and negative text. The defaults are
			"Yes" and "No". So to change the text to "Proceed" and "Cancel"
			you would use: (choices "Proceed" "Cancel")
*/

long yesno_page(struct InstallationRecord *ir)
{	static struct GadgetDef yesno_def[] = {
		{ &gir_prompt,	GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_PTR,	},
		{ (void *)TX_HELP,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	HELP_ID	},
		{ (void *)TX_ABORT,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LOCAL,	ABORT_ID	},
		{ NULL,			GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2, 		2			},
		{ NULL,			GTYPE_BUTTON | GPUT_LOWER | GWIDTH_2 | GFLAG_LAST, 1	}
	};
	struct Gadget	*gad = NULL;
	long			val;
	int				r;
	struct String	**choices = ir->choices;

	yesno_def[3].Text = ir->numchoices > 1 ? STR_VAL(choices[0]) : GetLocalText(TX_YES);
	yesno_def[4].Text = ir->numchoices > 1 ? STR_VAL(choices[1]) : GetLocalText(TX_NO);
	make_gadgets(&gad,yesno_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	while (TRUE)
	{	r = do_nothing(ir,help_ask_yesno,(1<<1)+(1<<2));

		if (r == ABORT_ID && !aborterr(FUNC_ASKBOOL)) continue;
		break;
	}

	clear_gadgets(gad);

	if (r == ABORT_ID) val = 0;
	else
	{	val = r - 1;
		transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		transcript(trans_ask_yesno,
			val ? yesno_def[3].Text : yesno_def[4].Text);
	}

	set_busy();

	return val;
}

/* Help Sub-Page:

	|---------------------------|
	|   	Help With...		|
	|							|
	| |-----------------------| |
	| |						  |	|
	| |		  Help Text		  |	|
	| |						  |	|
	| |-----------------------|	|
	|							|
	| Back      Exit       More |
	|---------------------------|

*/

#define MAX_HELP_PAGES	16
#define HELP_MARGIN		3

void display_help_page(char *text,WORD lines)
{	WORD	len,j,off;
	char	*start;

	SetAPen(rp,text_color);

	off = 1 + ((place_bottom-place_top-4) - lines * (rp->TxHeight + 1))/2;

	init_wrap();
	for (j=0;j<lines && text != NULL;j++)
	{	start = text;
		len = find_wrap(&text,window_width-right_edge-left_edge-2*HELP_MARGIN);
		Move(rp,left_edge+HELP_MARGIN,
			place_top + off + j * (rp->TxHeight + 1) + rp->TxBaseline);
		render_text(rp,start,len);
	}
}

void erase_help_page(void)
{
	SetAPen(rp,0);
	RectFill(rp,left_edge+2,place_top+1,window_width-right_edge-3,
		place_bottom-4);
}

int help_page(struct InstallationRecord *ir,char *title)
{	static struct GadgetDef help_def[] = {
		{ NULL,			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER,				},
		{ (void *)TX_EXITHELP,
			GTYPE_BUTTON | GPUT_STAY | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL, ABORT_ID },
		{ (void *)TX_BACK,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_SIDES | GFLAG_LOCAL, UP_ID	},
		{ (void *)TX_MORE,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_SIDES | GFLAG_LOCAL, DN_ID	},
		{ NULL,			GTYPE_BEVEL | GBEVEL_STD | GFLAG_LAST,		1		}
	};
	struct Gadget	*gad = NULL;
	char			*text,*page_start[MAX_HELP_PAGES];
	WORD			j,max_lines,
					pages,thispage = 0;

	help_def[0].Text = title;
	make_gadgets(&gad,help_def,ir,GERASE_PAGE|GSET_GADGETS);

	max_lines =	(place_bottom - place_top - 4) / (rp->TxHeight + 1);

	text = STR_VAL(ir->help_val.v);
	init_wrap();
	for (pages=0;pages<MAX_HELP_PAGES && text != NULL;pages++)
	{	page_start[pages] = text;

		for (j=0;j<max_lines && text != NULL;j++)
			find_wrap(&text,window_width-right_edge-left_edge-8);
	}

	display_help_page(page_start[0],max_lines);
	ghost_gadget(UP_ID);
	if (pages == 1) ghost_gadget(DN_ID);

	while (TRUE)
	{	struct Gadget *egad = HandleEvents(NULL);

		if (egad) switch (egad->GadgetID)
		{	case ABORT_ID: goto done;

			case UP_ID:
			if (thispage > 0)
			{	erase_help_page();
				if (thispage == pages - 1)
				{	enable_gadget(DN_ID);
#ifndef ONLY2_0
					write_text(rp,FindGadget(window,DN_ID),GetLocalText(TX_MORE));
#endif
				}
				if (--thispage == 0) ghost_gadget(UP_ID);
				display_help_page(page_start[thispage],max_lines);
			}
			break;	

			case DN_ID:
			if (thispage < pages - 1)
			{	erase_help_page();
				if (thispage == 0)
				{	enable_gadget(UP_ID);
#ifndef ONLY2_0
					write_text(rp,FindGadget(window,UP_ID),GetLocalText(TX_BACK));
#endif
				}
				if (++thispage == pages - 1) ghost_gadget(DN_ID);
				display_help_page(page_start[thispage],max_lines);
			}
			break;
		}
	}

done:
	clear_gadgets(gad);

	return 1;
}

/* About Sub-Page:

	|---------------------------|
	|   	  Installer			|
	|		   Version			|
	|		   Authors			|
	|		  Copyright			|
	|							|
	|           Exit       		|
	|---------------------------|
*/

int about_page(struct InstallationRecord *ir,char *title)
{	static struct GadgetDef about_def[] = {
		{ (void *)MSG_ABOUT_INSTALLER1, GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GFLAG_LOCAL,	},
		{ NULL,						    GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER,		},
		{ (void *)MSG_ABOUT_INSTALLER2,	GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_LOCAL,	},
		{ (void *)TX_EXITABOUT,
			GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LAST | GFLAG_LOCAL, 1 }
	};
	struct Gadget	*gad = NULL;
	char			major_version[80];

	Sprintf(major_version,"Version %ld.%ld",VERSION,REVISION);
	about_def[1].Text = major_version;

	make_gadgets(&gad,about_def,ir,GERASE_PAGE|GSET_GADGETS);

	while (TRUE)
	{	struct Gadget *egad = HandleEvents(NULL);

		if (egad && egad->GadgetID == 1) break;
	}

	clear_gadgets(gad);

	return 1;
}

/* Error Sub-Page:

	Type 0:

	|---------------------------|
	|   	    Title			|
	|							|
	|		   Problem			|
	|							|
	|  Yes			      No	|
	|---------------------------|

	Type 1:

	|---------------------------|
	|   	    Title			|
	|							|
	|		   Problem			|
	|							|
	|          Proceed     		|
	|---------------------------|
*/

int error_page(struct InstallationRecord *ir,char *title)
{	static struct GadgetDef error_def[] = {
		{ NULL, 		GTYPE_BEVEL| GBEVEL_SAVE,				0			},
		{ NULL,			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_BOLD,},
		{ NULL,			GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER,				},
		{ &gir_prompt,	GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_PTR,	},
		{ (void *)TX_YES,GTYPE_NONE | GPUT_LOWER | GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_NO,GTYPE_NONE | GPUT_LOWER | GFLAG_LOCAL,	1			},
		{ (void *)TX_PROCEED,	GTYPE_NONE | GPUT_LOWER | GFLAG_LOCAL | GFLAG_LAST,	PROCEED_ID	}
	};
	struct Gadget	*gad = NULL;
	int				result;
	WORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

	error_def[2].Text = ((error_def[1].Text = title) ? " " : NULL);
	if (ir->minval)
	{	error_def[4].Flags = error_def[5].Flags =
			GTYPE_BUTTON | GWIDTH_2 | GPUT_LOWER | GFLAG_LOCAL;
		error_def[6].Flags = GTYPE_NONE | GFLAG_LAST;
	}
	else
	{	error_def[4].Flags = error_def[5].Flags = GTYPE_NONE;
		error_def[6].Flags =
			GTYPE_BUTTON | GWIDTH_HALF | GPUT_LOWER | GFLAG_LOCAL | GFLAG_LAST;
	}
	make_gadgets(&gad,error_def,ir,GSET_GADGETS);

	while (TRUE)
	{	struct Gadget *egad = HandleEvents(NULL);

		if (egad)
		{	result = (egad->GadgetID == PROCEED_ID ? 1 : 0);
			if (egad->GadgetID == PROCEED_ID ||
				egad->GadgetID == 1) break;
		}
	}

	clear_gadgets(gad);

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	return result;
}

/* SelectDir Page (askdir):

	|---------------------------|
	|   	  Prompt			|
	|							|
	| |---------------------|^| |
	| |						|#|	|
	| |		  File List		|#|	|
	| |						| |	|
	| |---------------------|.|	|
	|							|
	|	   Selected Drawer		|
	|  [_______drawer________]  |
	|							|
	| Proceed  Parent   Drives	|
	|   MakeDir		   Abort	|
	|			Help			|
	|---------------------------|

	Parameters:
        prompt
		help
        default - default name of directory to be selected.
*/

BOOL	dir_special = FALSE;

char *select_dir(struct InstallationRecord *ir)
{	static struct GadgetDef dirgads[] = {
		{	&gir_prompt,GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR,	0			},
		{	(void *)TX_HELP,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF|GFLAG_LOCAL,	HELP_ID		},
		{	(void *)TX_MAKEDIR,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	CREATE_ID	},
		{	NULL,		GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,			ABORT_ID	},
		{	(void *)TX_PROCEED,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	PROCEED_ID,	},
		{	(void *)TX_PARENT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	PARENT_ID,	},
		{	(void *)TX_DRIVES,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	DRIVES_ID,	},
		{	&gir_default,GTYPE_STRGAD|GPUT_LOWER|GFLAG_PTR|GSTRING_ACTIVE,	DRAWER_ID,	},
		{	(void *)TX_SELECTED_DRAWER,
						GTYPE_TEXT|GPUT_LOWER|GTEXT_CENTER|GTEXT_BOLD|GFLAG_LOCAL,},
		{	NULL,		GTYPE_ARROW,							0			},
		{	NULL,		GTYPE_SLIDER,							0			},
		{	NULL,		GTYPE_LIST|GFLAG_LAST,					0			}
	};
	struct Gadget	*sgad,
					*gad = NULL;
	char			*str;
	WORD			r;

	dirgads[3].Text = (void *)(dir_special ? TX_CANCEL : TX_ABORT);
	make_gadgets(&gad,dirgads,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	r = do_drawers(ir,help_select_drawer);

	sgad = FindGadget(window,DRAWER_ID);

	clear_gadgets(gad);

	if (r == ABORT_ID) str = NULL;
	else
	{	str = (char *)((struct StringInfo *)sgad->SpecialInfo)->Buffer;
		if (dir_special == FALSE)
			transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		transcript(dir_special == TRUE ? trans_dest_drawer : trans_ask_drawer,
			str);
	}

	set_busy();

	return str;
}

/* SelectFile Page (askfile):

	|---------------------------|
	|   	  Prompt			|
	|							|
	| |---------------------|^| |
	| |						|#|	|
	| |		  File List		|#|	|
	| |						| |	|
	| |---------------------|.|	|
	|							|
	|	   Current Drawer		|
	|  [_______drawer________]  |
	|	   Selected File		|
	|  [_______drawer________]  |
	|							|
	| Proceed  Parent   Drives	|
	|	   Help		  Abort		|
	|---------------------------|

	Parameters:
        prompt
		help
        default - default name of file to be selected
*/

char *select_file(struct InstallationRecord *ir)
{	static struct GadgetDef filedef[] = {
		{	&gir_prompt,GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR,	0			},
		{	(void *)TX_HELP,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	HELP_ID		},
		{	(void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	ABORT_ID	},
		{	(void *)TX_PROCEED,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	PROCEED_ID,	},
		{	(void *)TX_PARENT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	PARENT_ID,	},
		{	(void *)TX_DRIVES,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_3|GFLAG_LOCAL,	DRIVES_ID,	},
		{	NULL,		GTYPE_STRGAD|GPUT_LOWER|GSTRING_ACTIVE,		FILENAME_ID,},
		{	(void *)TX_SELECTED_FILE,
						GTYPE_TEXT|GPUT_LOWER|GTEXT_CENTER|GTEXT_BOLD|GFLAG_LOCAL,},
		{	NULL,		GTYPE_STRGAD|GPUT_LOWER,					DRAWER_ID,},
		{	(void *)TX_CURRENT_DRAWER,
						GTYPE_TEXT|GPUT_LOWER|GTEXT_CENTER|GTEXT_BOLD|GFLAG_LOCAL,},
		{	NULL,		GTYPE_ARROW,							0			},
		{	NULL,		GTYPE_SLIDER,							0			},
		{	NULL,		GTYPE_LIST|GFLAG_LAST,					0			}
	};
	struct Gadget	*sgad,
					*gad = NULL;
	char			ch,
					*patch = NULL,
					*def,
					*str = NULL;
	WORD			r;
	BPTR			lock;

	def = STR_VAL(ir->default_val.v);

	if (lock = Lock(def,ACCESS_READ))
	{	struct FileInfoBlock	*fib;

		if (fib = AllocMem(sizeof *fib,MEMF_CLEAR|MEMF_PUBLIC))
		{
			if (Examine(lock,fib))
			{
				if (fib->fib_DirEntryType > 0)
				{
					str = "";
				}
			}

			FreeMem(fib,sizeof *fib);
		}

		UnLock(lock);
	}

	if (str == NULL)
	{
		str = FileOnly(def);

		if (str == def)
		{	patch = NULL;
			def = "";
		}
		else if (str[-1] == ':')
		{	patch = str;
			ch = *patch;
			*patch = 0;
		}
		else if (str[-1] == '/')
		{	patch = str - 1;
			ch = *patch;
			*patch = 0;
		}
		else
		{
			err_msg(ehead,ERR_DATA,FUNC_ASKFILE,invalid_drawer_name); 
			return NULL;
		}
	}

	filedef[6].Text = str;
	filedef[8].Text = def;
	make_gadgets(&gad,filedef,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	r = show_filelist(ir,help_select_file);

	if (patch) *patch = ch;

	if (r == ABORT_ID) str = NULL;
	else
	{	sgad = FindGadget(window,DRAWER_ID);
		str = (char *)((struct StringInfo *)sgad->SpecialInfo)->Buffer;
		sgad = FindGadget(window,FILENAME_ID);
		TackOn(str,(char *)((struct StringInfo *)sgad->SpecialInfo)->Buffer,
			MAX_TZ_CHARS);
		transcript_wrap("\n>%s\n",STR_VAL(ir->prompt_val.v));
		transcript(trans_ask_file,str);
	}

	clear_gadgets(gad);

	set_busy();

	return str;
}

/* Message Page (message):

	|---------------------------|
	|   	    Prompt			|
	|							|
	|							|
	|  Proceed		     Abort	|
	|---------------------------|

		or...

	|---------------------------|
	|   	    Prompt			|
	|							|
	|							|
	|  Proceed		     Abort	|
	|			 Help			|
	|---------------------------|

	Parameters:
        help - optional help text

*/

void message_page(struct InstallationRecord *ir)
{	static struct GadgetDef msg_def[] = {
		{ &gir_prompt,	GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR,	0			},
		{ NULL,			GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF,			HELP_ID		},
		{ (void *)TX_PROCEED,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL|GFLAG_LAST, ABORT_ID }
	};
	struct Gadget	*gad = NULL;
	WORD			i;
	char			*help = GetLocalText(TX_HELP);

	msg_def[1].Text = (ir->help_val.type == TYPE_STRING ? help : NULL);
	make_gadgets(&gad,msg_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	while (TRUE)
	{
		i = do_nothing(ir,msg_def[1].Text ? help : NULL,0);
		if (i == ABORT_ID && !aborterr(FUNC_MESSAGE)) continue;
		break;
	}

	clear_gadgets(gad);

	set_busy();
}

/* Final Page:

	|---------------------------|
	|   Installation Complete	|
	|							|
	|	     Information		|
	|							|
	|		   Proceed			|
	|---------------------------|

*/

void final_page(void)
{	static struct GadgetDef final_def[] = {
		{ NULL,			GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ NULL,			GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ "",			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ "",			GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ (void *)TX_PROCEED,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF|GFLAG_LOCAL|GFLAG_LAST, PROCEED_ID }
	};
	struct Gadget	*gad = NULL;
	char			buffer1[256],
					buffer2[256],
					*def_dest,
					*log_name;

	log_name = close_transcript();

	if (exit_quiet == FALSE)
	{
		def_dest = STR_VAL(*defpart_ptr);

		if (def_dest[0] == 0) Sprintf(buffer1,inst_complete1,app_name);
		else Sprintf(buffer1,inst_complete2,app_name,def_dest);

		if (transcript_mode == 1)
		{
			Sprintf(buffer2,inst_logfile_name,log_name);
		}
		else buffer2[0] = 0;

		final_def[0].Text = buffer1;
		final_def[1].Text = buffer2;
		if (extra_exit_text) final_def[3].Text = STR_VAL(extra_exit_text);
		make_gadgets(&gad,final_def,NULL,
			GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

		do_nothing(NULL,NULL,0);

		clear_gadgets(gad);

		set_busy();
	}

	did_final = TRUE;
}

/* Working Page (working):

	|---------------------------|
	|  Working on installation	|
	|							|
	|		   Prompt			|
	|							|
	|---------------------------|

	Parameters:
        NONE
*/

int working_page(struct InstallationRecord *ir)
{	static struct GadgetDef work_def[] = {
		{ (void *)TX_WORKING, GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL,	0	},
		{ "",			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ &gir_prompt,	GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR|GFLAG_LAST,	0	}
	};
	struct Gadget *gad = NULL;

	make_gadgets(&gad,work_def,ir,GERASE_PAGE|GSTART_LAYOUT);

	return 1;
}

int quick_working_page(void)
{	static struct GadgetDef work_def[] = {
		{ (void *)TX_WORKING, GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL|GFLAG_LAST,	0	}
	};
	struct Gadget *gad = NULL;

	make_gadgets(&gad,work_def,NULL,GERASE_PAGE|GSTART_LAYOUT);

	return 1;
}

/* Confirm Page - (confirm) parameter:

	|---------------------------|
	|   	    Prompt			|
	|							|
	|							|
	|  Proceed		     Skip	|
	|  Help			     Abort	|
	|---------------------------|
*/

int confirm_page(struct InstallationRecord *ir)
{	static struct GadgetDef confirm_def[] = {
		{ &gir_prompt,	GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR,	0			},
		{ (void *)TX_HELP,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	HELP_ID		},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	ABORT_ID	},
		{ (void *)TX_PROCEED,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_SKIP,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL|GFLAG_LAST,	1	}
	};
	struct Gadget	*gad = NULL;
	int				r;

	if ( !(ir->flags & IFLAG_CONFIRM) ) return TRUE;

	make_gadgets(&gad,confirm_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	while (TRUE)
	{	r = do_nothing(ir,help_with_confirm,(1<<1));

		if (r == ABORT_ID && !aborterr(FUNC_CONFIRM)) continue;
		break;
	}

	clear_gadgets(gad);

	/*	Could possibly do a transcript entry such as
		"Next action confirmed..." 									*/

	set_busy();

	return (r == PROCEED_ID);
}

/* Version Page (copylib):

	|---------------------------|
	|   	    Prompt			|
	|							|
	|		New Version #		|
	|		Old Version #		|
	|							|
	|  Proceed		     Skip	|
	|  Help			     Abort	|
	|---------------------------|

    Parameters are:
        prompt
		help
        source - name of source file
        dest - name of destination directory
        newname - if copying one file only, and file is to be renamed, this is
             the new name
        infos - switch to copy icons along with other files
		confirm - user will be asked to confirm. Note that an EXPERT user
			will be able to overwrite a newer file with an older one.
		safe - copy the file even if in PRETEND mode
*/

int version_page(struct InstallationRecord *ir,long old,long new)
{	static struct GadgetDef ver_def[] = {
		{ &gir_prompt,	GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_PTR,	0			},
		{ "",			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ NULL,			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ NULL,			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ "",			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ (void *)TX_COPY_DEST,GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL,	0	},
		{ NULL,			GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ (void *)TX_HELP,		GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	HELP_ID		},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	ABORT_ID	},
		{ (void *)TX_PROCEED_COPY,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_SKIP,		GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL|GFLAG_LAST,	1	}
	};
	struct Gadget	*gad = NULL;
	char			text1[80], text2[80];
	int				r;

	Sprintf(text1,version_msg1,new >> 16,new & 0x0ffff);
	if (old != 0) Sprintf(text2,version_msg2,old >> 16,old & 0x0ffff);

	ver_def[2].Text = text1;
	ver_def[3].Text = (old != 0 ? text2 : version_msg3);
	ver_def[6].Text = STR_VAL(ir->dest_val.v);

	make_gadgets(&gad,ver_def,ir,
		GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS|GCLEAR_BUSY);

	while (TRUE)
	{	r = do_nothing(ir,help_copyfiles,(1<<1));

		if (r == ABORT_ID && !aborterr(FUNC_COPYLIB)) continue;
		break;
	}

	clear_gadgets(gad);

	set_busy();

	return (r == PROCEED_ID);
}

/* Intro/User Level Page:

	|---------------------------|
	|   	    Intro			|
	|							|
	|	Set Installation Mode	|
	|	   [] Novice User		|
	|	   [] Average User		|
	|	   [] Expert User		|
	|							|
	|  Proceed		     Abort	|
	|  About			 Help	|
	|---------------------------|
*/

struct String *make_user_help(struct InstallationRecord *ir,
	struct InstallationRecord *xir, WORD num)
{
	struct String	*help_temp = NULL;
	char			*c1, *c2, *c3;

	ir->help_val.v = (struct String *)GetLocalText(num);
	ir->help_val.type = TYPE_STRING;
	if (xir)
	{
		c1 = STR_VAL(xir->help_val.v);
		c2 = STR_VAL(ir->help_val.v);
		if (c1[0])
		{
			if ( help_temp = empty_string(strlen(c1)+1+strlen(c2)) )
			{
				strcpy(c3 = STR_VAL(help_temp),c1);
				strcat(c3,"\n");
				strcat(c3,c2);
				ir->help_val.v = help_temp;
			}
		}
	}

	return help_temp;
}

int get_user(struct InstallationRecord *xir)
{	static struct GadgetDef user_def1[] = {
		{ NULL,			GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER,			0			},
		{ (void *)TX_ABOUT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	1			},
		{ (void *)TX_HELP,		GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	HELP_ID		},
		{ (void *)TX_PROCEED_INST,GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL|GFLAG_LAST,ABORT_ID	}
	};
	static struct GadgetDef user_def2[] = {
		{ (void *)TX_INST_MODE,GTYPE_TEXT|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL,	0		},
		{ NULL,			GTYPE_RADIO,								2			},
		{ NULL,			GTYPE_BEVEL|GBEVEL_STD|GFLAG_LAST,			0			}
	};
	struct MultiData data;
	struct Gadget	*gad = NULL,
					*gad1;
	int				u,r,
					result = 0;
	struct String	*set1[3],
					*help_temp = NULL;
	struct InstallationRecord ir;
	char			buffer[200];

		/* if EXPERT is the only allowed mode of operation... */

	if (minuser_level == 2)
	{	user_level = 2;
		return TRUE;
	}

	clear_busy();

		/* The help text needs to change based on MINUSER. */

	help_temp = make_user_help(&ir,xir,HELP_USER_LEVEL);

	set1[0] = (struct String *)GetLocalText(STR_NOVICE_USER);
	set1[1] = (struct String *)GetLocalText(STR_AVERAGE_USER);
	set1[2] = (struct String *)GetLocalText(STR_EXPERT_USER);

	Sprintf(buffer,welcome_msg,app_name);

	user_def1[0].Text = buffer;
	make_gadgets(&gad,user_def1,&ir,GERASE_PAGE|GSTART_LAYOUT);

	u = (place_bottom - place_top)/8;
	place_bottom = place_top + 6 * u + 2;
	place_top += 2 * u + 2;

	radio_space_check(3);

	data.Choices = set1;
	data.NumChoices = 3;
	data.DefChoice = user_level;
	user_def2[1].Text = &data;
	make_gadgets(&gad,user_def2,&ir,GSET_GADGETS);

	gad1 = FindGadget(window,2+2);
	if (minuser_level > 0) disable_radio(gad1,0,3);
	if (minuser_level > 1) disable_radio(gad1,1,3);

	while (TRUE)
	{
		r = do_nothing(&ir,help_inst_mode,2);
		if (r == ABORT_ID) { result = FALSE; break; }
		if (r == PROCEED_ID) { result = TRUE; break; }
		if (r == 1)
		{		/* do "about" display */
			save_page(NULL,NULL,about_page);
		}
	}

	clear_gadgets(gad);

	if (result == TRUE)
	{		/* this statement depends upon the fact that USER_NOVICE == 0 */
		user_level = final_radio(gad1,3);
	}

	MemFree(help_temp);

	set_busy();

	return result;
}

/* Use Level Page:

	|---------------------------|
	|      Possible Intro		|
	|							|
	|	Installation Options	|
	|	[] Install for Real		|
	|	[] Pretend to Install	|
	|							|
	|	 Log all actions to:	|
	|	     [] Printer			|
	|	     [] Log File		|
	|	     [] None			|
	|							|
	|  Proceed		     Abort	|
	|  			Help			|
	|---------------------------|
*/

int get_options(struct InstallationRecord *xir)
{	static struct GadgetDef options_def[] = {
		{ NULL,			GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER,		0			},
		{ (void *)TX_HELP,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF|GFLAG_LOCAL,HELP_ID	},
		{ (void *)TX_PROCEED,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	ABORT_ID },
		{ NULL,			GTYPE_RADIO|GRADIO_SPECIAL1,			3			},
		{ (void *)TX_LOG_TO,	GTYPE_TEXT|GPUT_LOWER|GTEXT_BOLD|GTEXT_CENTER|GFLAG_LOCAL,	0	},
		{ NULL,			GTYPE_RADIO|GRADIO_SPECIAL2,			1			},
		{ (void *)TX_INST_OPTIONS,
			GTYPE_TEXT|GPUT_LOWER|GTEXT_BOLD|GTEXT_CENTER|GFLAG_LOCAL|GFLAG_LAST,	0	}
	};
	struct Gadget	*gad = NULL;
	struct MultiData data1, data2;
	int				r,
					result = 0;
	struct String	*set2[2],*set3[3],
					*help_temp = NULL;
	struct InstallationRecord ir;
	char			buffer[120];

	clear_busy();

	set2[0] = (struct String *)GetLocalText(STR_FOR_REAL);
	set2[1] = (struct String *)GetLocalText(STR_PRETEND);
	set3[0] = (struct String *)GetLocalText(STR_PRINTER);
	set3[1] = (struct String *)GetLocalText(STR_LOGFILE);
	set3[2] = (struct String *)GetLocalText(STR_NOLOG);

	if (minuser_level == 2)
	{
		Sprintf(buffer,welcome_msg2,app_name);
		options_def[0].Text = buffer;
	}
	else options_def[0].Text = NULL;

	help_temp = make_user_help(&ir,options_def[0].Text ? xir : NULL,HELP_INST_SETTINGS);

	data1.Choices = set3;
	data1.NumChoices = 3;
	data1.DefChoice = transcript_mode;
	options_def[4].Text = &data1;

	data2.Choices = set2;
	data2.NumChoices = 2;
	data2.DefChoice = pretend_flag;
	options_def[6].Text = &data2;

	make_gadgets(&gad,options_def,&ir,GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS);

	if (no_pretend == TRUE) disable_radio(FindGadget(window,1+1),1,2);
	if (no_print == TRUE) disable_radio(FindGadget(window,3+2),0,3);

	while (TRUE)
	{
		r = do_nothing(&ir,help_with_settings,2);
		if (r == ABORT_ID) { result = FALSE; break; }
		if (r == PROCEED_ID) { result = TRUE; break; }
	}

	if (result == TRUE)
	{
		pretend_flag = final_radio(FindGadget(window,1+1),2);
		transcript_mode = final_radio(FindGadget(window,3+2),3);
	}

	clear_gadgets(gad);

	MemFree(help_temp);

	set_busy();

	return result;
}

/* Change Startup Page (part of startup):

	|---------------------------|
	|  Modify Startup-Sequence	|
	|							|
	| |---------------------|^| |
	| |						|#|	|
	| |	    Text of File	|#|	|
	| |						| |	|
	| |---------------------|.|	|
	|							|
	|	 File being modified:	|
	|  [_______drawer________]  |
	|							|
	| Proceed             Skip	|
	| Select File	      Abort	|
	|			Help			|
	|---------------------------|

*/

BOOL change_startup(char **file,int *offset)
{	static struct GadgetDef start_def[] = {
		{ (void *)TX_MODIFY_SS,
						GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL,	0			},
		{ (void *)TX_HELP, GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF|GFLAG_LOCAL,HELP_ID		},
		{ (void *)TX_ANOTHER_FILE,
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	1			},
		{ (void *)TX_ABORT,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	ABORT_ID	},
		{ (void *)TX_PROCEED_CHANGES,
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	PROCEED_ID	},
		{ (void *)TX_SKIP,	GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	2			},
		{ NULL, 		GTYPE_STRGAD | GPUT_LOWER | GSTRING_ACTIVE,	DRAWER_ID	},
		{ (void *)TX_FILE_MODIFIED,
			GTYPE_TEXT | GPUT_LOWER | GTEXT_CENTER | GTEXT_BOLD | GFLAG_LOCAL, },
		{ NULL, 		GTYPE_ARROW, 							0			},
		{ NULL, 		GTYPE_SLIDER,							0			},
		{ NULL,			GTYPE_LIST | GFLAG_LAST,				0			}
	};
	struct InstallationRecord ir;
	BPTR 			texthandle;
	struct Gadget	*agad,
					*gad = NULL;
	char			batchname[120],
					*str;
	BOOL			ok = FALSE;
	int				r,
					size;

restart:
	clear_busy();

	size = GetFileSize(*file);
	if ( size == 0 || !(texthandle = Open(*file,MODE_OLDFILE)) )
	{
no_open:
		err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_with_file);
		return FALSE;
	}

	textbuf = MemAlloc(size+3,0L);
	if (textbuf == NULL)
	{
		memerr();
		Close(texthandle);
		return FALSE;
	}

	r = Read(texthandle,textbuf,size);
	Close(texthandle);

	if (r != size)
	{	
		err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_with_file);
		return FALSE;
	}

	str = textbuf + size - 1;
	str[1] = '\n';
	if (*str != '\n')
	{	str[2] = '\n';
		str[3] = 0;
	}
	else str[2] = 0;

	unless (textloc = *offset)
	{
		if (str = strchr(textbuf,'\n'))
			textloc = str - textbuf + 1;
	}

	strcpy(batchname,*file);
	ir.help_val.v = (struct String *)GetLocalText(HELP_CHANGE_STARTUP);
	ir.help_val.type = TYPE_STRING;

	start_def[6].Text = batchname;
	make_gadgets(&gad,start_def,&ir,GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS);

	while (TRUE)
	{	r = do_textlist(&ir,help_change_startseq);

		if (r == 2) break;	/* skip this part */

		if (r == ABORT_ID && !aborterr(FUNC_STARTUP)) continue;

		if (r == 1)		/* select new file */
		{
			clear_gadgets(gad);

			MemFree(textbuf);

			ir.prompt_val.v = (struct String *)GetLocalText(STR_FILE_TO_MODIFY);
			ir.help_val.v = (struct String *)GetLocalText(HELP_SELECT_FILE);
			unless (ir.default_val.v = new_string(*file))
			{	memerr();
				goto bye;
			}
			ir.prompt_val.type = ir.help_val.type =
				ir.default_val.type = TYPE_STRING;

			unless (str = select_file(&ir)) goto bye;

			*file = str;
			*offset = 0;
			MemFree(ir.default_val.v);
			gad = NULL;
			goto restart;
		}

		break;
	}

	agad = FindGadget(window,DRAWER_ID);

	clear_gadgets(gad);

	if (r == PROCEED_ID)
	{	*offset = textloc;
		*file = (char *)((struct StringInfo *)agad->SpecialInfo)->Buffer;
		ok = TRUE;
	}

	MemFree(textbuf);

bye:
	set_busy();
	return ok;
}

	/* this routine sets up an error message and then calls error_page */

int report_message(WORD mode,char *text,...)
{
	struct InstallationRecord	ir;
	struct String				*str;
	char						msg[256];
	int							result;

	if (mode == 0) clear_busy();

	start_layout();

	str = (struct String *)msg;
	VSprintf((char *)(str+1),text,&text + 1);
	ir.prompt_val.v = str;
	ir.prompt_val.type = TYPE_STRING;
	ir.minval = 0;			/* error report mode for error_page */

	result =
		save_page(&ir,mode == 1 ? error_msg : NULL,error_page);

	if (mode == 0) set_busy();

	return result;
}

	/* this routine sets up a simple message and then calls message_page */

void simple_message(char *text,...)
{	struct InstallationRecord	ir;
	char						msg[256];
	struct String				*str;

	str = (struct String *)msg;
	VSprintf((char *)(str+1),text,&text + 1);
	ir.prompt_val.v = str;
	ir.prompt_val.type = TYPE_STRING;
	message_page(&ir);
}

	/* this routine gets any special message script supplied for fatal error */

char *has_special(void)
{	extern void 	**specmsg_ptr;
	char			*text;

	if (!*specmsg_ptr) return NULL;

	text = STR_VAL(*specmsg_ptr);

	if (text[0] == 0) return NULL;
	return text;
}

/* Fatal Sub-Page:

	|---------------------------|
	|   	    Title			|
	|							|
	|		 Information		|
	|							|
	|		   Proceed			|
	|---------------------------|
*/

int fatal_page(struct InstallationRecord *ir,char *title)
{	static struct GadgetDef fatal_def[] = {
		{ NULL, 		GTYPE_BEVEL| GBEVEL_SAVE,				0			},
		{ NULL, 		GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER | GTEXT_BOLD,},
		{ NULL, 		GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 			},
		{ NULL, 		GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER,				},
		{ NULL, 		GTYPE_TEXT | GPUT_UPPER | GTEXT_CENTER, 			},
		{ NULL, 		GTYPE_WRAP | GPUT_UPPER | GTEXT_CENTER | GFLAG_PTR,	},
		{ (void *)TX_PROCEED,	GTYPE_BUTTON | GPUT_LOWER | GWIDTH_HALF | GFLAG_LOCAL | GFLAG_LAST, PROCEED_ID	}
	};
	struct Gadget	*gad = NULL;
	WORD			left = left_edge,
					right = right_edge,
					top = top_edge,
					bottom = bottom_edge;

		/* special resizing done by GTYPE_BEVEL GadgetDef above */

	fatal_def[2].Text = ((fatal_def[1].Text = title) ? " " : NULL);
	fatal_def[4].Text = ((fatal_def[3].Text = has_special()) ? " " : NULL);
	fatal_def[5].Text =
		(!(fatal_def[3].Text && ierror == ERR_HANDLED) ? &gir_prompt : NULL);
	make_gadgets(&gad,fatal_def,ir,GSET_GADGETS);

	while (TRUE)
	{	struct Gadget *egad = HandleEvents(NULL);

		if (egad && egad->GadgetID == PROCEED_ID) break;
	}

	clear_gadgets(gad);

	left_edge = left;
	right_edge = right;
	top_edge = top;
	bottom_edge = bottom;

	return 0;
}

	/* the functions sets up a fatal error message and calls fatal_page */

void show_error(WORD mode,char *text)
{	extern void					**ioerr_ptr;
	char 						*DosErrorText(long num);
	char						buffer[256];
	struct InstallationRecord	ir;

	clear_busy();

	if (mode ==	ERR_DOS)
	{
		buffer[0] = buffer[1] = 0;
		if (has_special()) buffer[0] = '(';
		strcat(buffer,text);
		strcat(buffer,dos_error_type);
		strcat(buffer,DosErrorText((LONG)*ioerr_ptr));
		if (has_special()) strcat(buffer,")");
		text = buffer;
	}
	else if (has_special())
	{
		buffer[0] = '(';
		strcpy(&buffer[1],text);
		strcat(buffer,")");
		text = buffer;
	}

	ir.prompt_val.v = new_string(text);
	ir.prompt_val.type = TYPE_STRING;

	save_page(&ir,sorry_msg,fatal_page);

	MemFree(ir.prompt_val.v);

	/* we don't set_busy() since only fatal errors go through here... */
}

	/* ask if the user really wants to abort, calls error_page */

int ask_abort(void)
{	long						result;
	struct InstallationRecord	ir;

	clear_busy();

	ir.prompt_val.v = (struct String *)GetLocalText(STR_SURE_ABORT);
	ir.prompt_val.type = TYPE_STRING;
	ir.minval = 1;			/* ask report mode for error_page */

	result = save_page(&ir,please_confirm,error_page);

	return result;
}

	/* completion/window title routine */

void completion(long percent)
{	static char 		title[80];
	struct IntuiText	itext;
	int					len;

	percent_complete = percent;

	if (percent == -1)
		Sprintf(title,"%s%s",templatefile,is_busy ? press_escape : "");
	else
	{
		Sprintf(title,"%s (%ld%% %s)%s",
			templatefile,percent,GetLocalText(TX_COMPLETE),
			is_busy ? press_escape : "");

		itext.LeftEdge = 0;
		itext.ITextFont = window->WScreen->Font;
		itext.IText = title;
		itext.NextText = NULL;

		len = IntuiTextLength(&itext);

		if (len > window->Width - depth_width)
		{
			Sprintf(title,"(%ld%% %s)%s",percent,GetLocalText(TX_COMPLETE),
				is_busy ? press_escape : "");
		}
	}

	SetWindowTitles(window,title,(char *)-1);
}

	/* error autorequest to use when GUI not up */

struct IntuiText
	MsgIText = { 0,1, JAM2, 10,34, &Topaz8, NULL },
	OkIText = { 0,1, JAM2, 6,3, &Topaz8, NULL };

void auto_err(char *msg)
{
	MsgIText.IText = msg;
	OkIText.IText = GetLocalText(TX_OK);
	AutoRequest(window,&MsgIText,NULL,&OkIText,0,0,360,80);
}

/*====================== Busy Pointer ======================== */

#ifndef ONLY2_0

USHORT Busy1_3[] = {
	0x0000, 0x0000, 0x0600, 0x0600, 0x0f40, 0x0f40, 0x3fe0, 0x3fe0, 
	0x7fe0, 0x7fe0, 0x7ff0, 0x61f0, 0x7ff8, 0x7bf8, 0xfff8, 0xf7f8,
	0x7ffc, 0x61fc, 0x7ffc, 0x7f0c, 0x3ffe, 0x3fde, 0x7ffc, 0x7fbc,
	0x3ffc, 0x3f0c, 0x1ff8, 0x1ff8, 0x07f0, 0x07f0, 0x01c0, 0x01c0, 
	0x0700, 0x0700, 0x0fc0, 0x0fc0, 0x0680, 0x0680, 0x0000, 0x0000, 
	0x00c0, 0x00c0, 0x00e0, 0x00e0, 0x0040, 0x0040, 0x0000, 0x0000
};

#define BUSY1_3_HEIGHT	22

#endif

UWORD Busy2_0[] =
{
    0x0000, 0x0000,

    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07E0,

    0x0000, 0x0000             /* reserved, must be NULL */
};

#define BUSY2_0_HEIGHT	16

void set_busy(void)
{	UWORD	*busy;
	long	size;

	if (is_busy == 1) return;

	if (inited_busy == FALSE)
	{
#ifdef ONLY2_0
		busy = Busy2_0;
		size = sizeof Busy2_0;
#else
		busy = (IntuitionBase->lib_Version >= 36 ? Busy2_0 : Busy1_3);
		size = (IntuitionBase->lib_Version >= 36 ? sizeof Busy2_0 : sizeof Busy1_3);
#endif

		if ( TypeOfMem((void *)busy) & MEMF_CHIP ) chip_busy = busy;
		else
		{
			if (chip_busy = MemAlloc(size,MEMF_CHIP))
				CopyMem(busy,chip_busy,size);
			else inited_fail = 1;
		}
		inited_busy = TRUE;
	}

	if (inited_fail == 1) return;

#ifdef ONLY2_0
	SetPointer(window,(void *)chip_busy,BUSY2_0_HEIGHT,15,-6,0);
#else
	SetPointer(window,(void *)chip_busy,
		IntuitionBase->lib_Version >= 36 ? BUSY2_0_HEIGHT : BUSY1_3_HEIGHT,
		15,IntuitionBase->lib_Version >= 36 ? -6 : -7,
		IntuitionBase->lib_Version >= 36 ? 0 : -10);
#endif
	is_busy = 1;
	completion(percent_complete);
}

void clear_busy(void)
{
	if (is_busy == 0) return;

	ClearPointer(window);
	is_busy = 0;
	completion(percent_complete);
}
