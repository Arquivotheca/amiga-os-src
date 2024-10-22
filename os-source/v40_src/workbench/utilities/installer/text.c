/* text strings that need to be translatable */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef ONLY2_0
#include <clib/locale_protos.h>
#include <locale.p>
#include <libraries/locale.h>
#endif

#include "text.h"

#define LAST_STR		MSG_ASKDELETE

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

extern struct AppString *AppStrings;
extern LONG				NumAppStrings;

#ifdef ONLY2_0
extern struct Library	*LocaleBase;
extern struct Catalog	*catalog;
#endif

#ifdef AZTEC_C
char *GetLocalText(WORD num);
#pragma regcall( GetLocalText(d0) )
#else
/* char __asm *GetLocalText(register __d0 WORD num); */
char *GetLocalText(WORD num);
#endif

#ifdef AZTEC_C
char *GetLocalText(WORD num)
#else
/* char __asm GetLocalText(register __d0 WORD num) */
char *GetLocalText(WORD num)
#endif
{	extern char *override_text(UWORD num);
	struct AppString	*appstr = AppStrings;
	char				*str = "BAD TEXT";
	WORD				i;

	if (str = override_text(num)) ;
	else for (i=0; i<NumAppStrings; i++, appstr++)
	{
		if (appstr->as_ID == num)
		{
			str = appstr->as_Str;
			break;
		}
	}

#ifdef ONLY2_0
	if (catalog)
	{
		str = GetCatalogStr(catalog,num,str);
	}
#endif

	return str;
}

char	*minuser_tt = "MINUSER",
		*defuser_tt = "DEFUSER",
		*pretend_tt = "PRETEND",
		*script_tt = "SCRIPT",
		*appname_tt = "APPNAME",
		*logfile_tt = "LOGFILE",
		*nopretend_tt = "NOPRETEND",
		*nolog_tt = "NOLOG",
		*noprint_tt = "NOPRINT",
		*print_tt = "PRINT",
		*log_tt = "LOG",
		*language_tt = "LANGUAGE",
		*false_tv = "FALSE",
		*novice_tv = "NOVICE",
		*average_tv = "AVERAGE",
		*expert_tv = "EXPERT";

char	*dot_info = ".info",
		*dot_font = ".font";

char	*transcript_header;
char 	*def_transcript_name;
char	*usage_tx;

char	*no_find_icon;

char	*trans_made_drawer,
		*trans_copy_file,
		*trans_copy_drawer,
		*trans_ask_choice,
		*trans_no_options,
		*trans_ask_options,
		*trans_ask_string,
		*trans_ask_number,
		*trans_ask_yesno,
		*trans_dest_drawer,
		*trans_ask_drawer,
		*trans_ask_file;

char
		*prob_with_source,
		*prob_with_file,
		*help_copyfiles,
		*help_with_choice,
		*help_with_options,
		*help_ask_string,
		*help_ask_number,
		*help_insert_disk,
		*help_ask_yesno,
		*help_select_drawer,
		*help_select_file,
		*inst_complete1,
		*inst_complete2,
		*inst_logfile_name,
		*help_with_confirm,
		*version_msg1,
		*version_msg2,
		*version_msg3,
		*set_inst_mode,
		*welcome_msg,
		*help_inst_mode,
		*welcome_msg2,
		*help_with_settings,
		*help_change_startseq,
		*error_msg,
		*dos_error_type,
		*sorry_msg,
		*please_confirm,
		*press_escape,
		*valid_range;

char	*bad_params_copy,
		*invalid_drawer_name;

/* interp.c */

char	*global_error_msg,
		*string_length_msg,
		*cannot_examine_msg,
		*noexist_msg,
		*cannot_assign_msg,
		*divide_zero_msg,
		*bad_cat_msg,
		*err_memory_msg,
		*user_abort_msg,
		*script_line_msg1,
		*script_line_msg2;

/* statement.c */

char	*trans_create_drawer,
		*trans_insert_commands,
		*trans_create_textfile,
		*trans_end_textfile,
		*trans_execute,
		*trans_run,
		*trans_arexx,
		*trans_rename,
		*trans_delete,
		*trans_aborting,
		*trans_exiting;

char	*no_drawer_msg,
		*no_file_msg,
		*prob_ss_msg,
		*err_writing_msg,
		*err_unknown_msg,
		*err_reading_text;

/* action.c */

char	*err_cannot_create,
		*err_cannot_copy,
		*err_overwrite,
		*err_execute_path,
		*err_no_nil,
		*err_no_command,
		*err_no_rexx,
		*err_cannot_duplock;

/* start.c */

char	*err_reading_msg;

void localize_text(void)
{
	/* window.c */
	transcript_header = GetLocalText(TRANS_HEADER);
	def_transcript_name = GetLocalText(TRANS_DEF_NAME);
	usage_tx = GetLocalText(TX_USAGE);

	no_find_icon = GetLocalText(MSG_NO_SCRIPT_ICON);

	trans_made_drawer = GetLocalText(TRANS_USER_DRAWER);
	trans_copy_file = GetLocalText(TRANS_COPY_FILE);
	trans_copy_drawer = GetLocalText(TRANS_COPY_DRAWER);
	trans_ask_choice = GetLocalText(TRANS_ASK_CHOICE);
	trans_no_options = GetLocalText(TRANS_NO_OPTIONS);
	trans_ask_options = GetLocalText(TRANS_ASK_OPTIONS);
	trans_ask_string = GetLocalText(TRANS_ASK_STRING);
	trans_ask_number = GetLocalText(TRANS_ASK_NUMBER);
	trans_ask_yesno = GetLocalText(TRANS_ASK_BOOL);
	trans_dest_drawer = GetLocalText(TRANS_DEST_DRAWER);
	trans_ask_drawer = GetLocalText(TRANS_ASK_DRAWER);
	trans_ask_file = GetLocalText(TRANS_ASK_FILE);

	prob_with_source = GetLocalText(MSG_PROB_SOURCE);
	prob_with_file = GetLocalText(MSG_PROB_FILE);

	help_copyfiles = GetLocalText(TX_HELP_COPY);
	help_with_choice = GetLocalText(TX_HELP_ASKCHOICE);
	help_with_options = GetLocalText(TX_HELP_ASKOPTIONS);
	help_ask_string = GetLocalText(TX_HELP_ASKSTRING);
	help_ask_number = GetLocalText(TX_HELP_ASKNUMBER);
	help_insert_disk = GetLocalText(TX_HELP_ASKDISK);
	help_ask_yesno = GetLocalText(TX_HELP_ASKBOOL);
	help_select_drawer = GetLocalText(TX_HELP_SELECTDIR);
	help_select_file = GetLocalText(TX_HELP_SELECTFILE);
	inst_complete1 = GetLocalText(TX_INST_COMPLETE);
	inst_complete2 = GetLocalText(TX_INST_COMP_WHERE);
	inst_logfile_name = GetLocalText(TX_INST_LOGFILE);
	help_with_confirm = GetLocalText(TX_HELP_CONFIRM);
	version_msg1 = GetLocalText(TX_VERSION_SOURCE);
	version_msg2 = GetLocalText(TX_VERSION_DEST);
	version_msg3 = GetLocalText(TX_VERSION_NONE);
	set_inst_mode = GetLocalText(TX_INST_MODE);
	welcome_msg = GetLocalText(TX_WELCOME);
	help_inst_mode = GetLocalText(TX_HELP_INSTMODE);
	welcome_msg2 = GetLocalText(TX_WELCOME_OPT);
	help_with_settings = GetLocalText(TX_HELP_SETTINGS);
	help_change_startseq = GetLocalText(TX_HELP_CHANGE_SS);
	error_msg = GetLocalText(TX_ERROR);
	dos_error_type = GetLocalText(TX_DOSERROR);
	sorry_msg = GetLocalText(TX_SORRY);
	please_confirm = GetLocalText(TX_CONFIRM);
	press_escape = GetLocalText(TX_ESCAPE);
	valid_range = GetLocalText(TX_RANGE);

	bad_params_copy = GetLocalText(MSG_BAD_PARAMS_COPY);
	invalid_drawer_name = GetLocalText(MSG_INVALID_DRAWER_NAME);

/* interp.c */
	global_error_msg = GetLocalText(MSG_ONERROR);
	string_length_msg = GetLocalText(MSG_TOO_LONG);
	cannot_examine_msg = GetLocalText(MSG_NOEXAMINE_OBJECT);
	noexist_msg = GetLocalText(MSG_NOEXIST_OBJECT);
	cannot_assign_msg = GetLocalText(MSG_CANNOT_ASSIGN);
	divide_zero_msg = GetLocalText(MSG_DIVIDE_ZERO);
	bad_cat_msg = GetLocalText(MSG_BAD_CAT);
	err_memory_msg = GetLocalText(MSG_ERR_MEMORY);
	user_abort_msg = GetLocalText(MSG_USER_ABORT);
	script_line_msg1 = GetLocalText(MSG_ABORT_LINE);
	script_line_msg2 = GetLocalText(MSG_PROBLEM_LINE);

/* statement.c */

	trans_create_drawer = GetLocalText(TRANS_CREATE_DRAWER);
	trans_insert_commands = GetLocalText(TRANS_INSERT_COMMANDS);
	trans_create_textfile = GetLocalText(TRANS_CREATE_TEXTFILE);
	trans_end_textfile = GetLocalText(TRANS_END_TEXTFILE);
	trans_execute = GetLocalText(TRANS_EXECUTE);
	trans_run = GetLocalText(TRANS_RUN);
	trans_arexx = GetLocalText(TRANS_AREXX);
	trans_rename = GetLocalText(TRANS_RENAME);
	trans_delete = GetLocalText(TRANS_DELETE);
	trans_aborting = GetLocalText(TRANS_ABORTING);
	trans_exiting = GetLocalText(TRANS_EXITING);

	no_drawer_msg = GetLocalText(MSG_NO_DRAWER);
	no_file_msg = GetLocalText(MSG_NO_FILE);
	prob_ss_msg = GetLocalText(MSG_PROB_SS);
	err_writing_msg = GetLocalText(MSG_ERR_WRITING);
	err_unknown_msg = GetLocalText(MSG_ERR_UNKNOWN);
	err_reading_text = GetLocalText(MSG_ERR_READING);

/* action.c */

	err_cannot_create = GetLocalText(MSG_CANNOT_CREATE);
	err_cannot_copy = GetLocalText(MSG_CANNOT_COPY);
	err_overwrite = GetLocalText(MSG_OVERWRITE);
	err_execute_path = GetLocalText(MSG_EXECUTE_PATH);
	err_no_nil = GetLocalText(MSG_NO_NIL);
	err_no_command = GetLocalText(MSG_NO_COMMAND);
	err_no_rexx = GetLocalText(MSG_NO_REXX);
	err_cannot_duplock = GetLocalText(MSG_NO_DUPLOCK);

/* start.c */

	err_reading_msg = GetLocalText(ERR_READING_US);
}
