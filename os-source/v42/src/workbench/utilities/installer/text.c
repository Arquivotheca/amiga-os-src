/*
 *		text.c
 *
 *	This module contains pointer to the localized text strings used throuought
 *	the reaminder of the Installer utility.  The actual text strings in English
 *	are in an assembler module created by CatComp.
 *
 *
 * Prototypes for functions defined in text.c
 *
 *	unsigned 		char * GetLocalText(WORD );
 *
 *	extern unsigned char * minuser_tt;
 *	extern unsigned char * defuser_tt;
 *	extern unsigned char * pretend_tt;
 *	extern unsigned char * script_tt;
 *	extern unsigned char * appname_tt;
 *	extern unsigned char * logfile_tt;
 *	extern unsigned char * nopretend_tt;
 *	extern unsigned char * nolog_tt;
 *	extern unsigned char * noprint_tt;
 *	extern unsigned char * print_tt;
 *	extern unsigned char * log_tt;
 *	extern unsigned char * language_tt;
 *	extern unsigned char * false_tv;
 *	extern unsigned char * novice_tv;
 *	extern unsigned char * average_tv;
 *	extern unsigned char * expert_tv;
 *	extern unsigned char * dot_info;
 *	extern unsigned char * dot_font;
 *	extern unsigned char * transcript_header;
 *	extern unsigned char * def_transcript_name;
 *	extern unsigned char * usage_tx;
 *	extern unsigned char * no_find_icon;
 *	extern unsigned char * trans_made_drawer;
 *	extern unsigned char * trans_copy_file;
 *	extern unsigned char * trans_copy_drawer;
 *	extern unsigned char * trans_ask_choice;
 *	extern unsigned char * trans_no_options;
 *	extern unsigned char * trans_ask_options;
 *	extern unsigned char * trans_ask_string;
 *	extern unsigned char * trans_ask_number;
 *	extern unsigned char * trans_ask_yesno;
 *	extern unsigned char * trans_dest_drawer;
 *	extern unsigned char * trans_ask_drawer;
 *	extern unsigned char * trans_ask_file;
 *	extern unsigned char * prob_with_source;
 *	extern unsigned char * prob_with_file;
 *	extern unsigned char * help_copyfiles;
 *	extern unsigned char * help_with_choice;
 *	extern unsigned char * help_with_options;
 *	extern unsigned char * help_ask_string;
 *	extern unsigned char * help_ask_number;
 *	extern unsigned char * help_insert_disk;
 *	extern unsigned char * help_ask_yesno;
 *	extern unsigned char * help_select_drawer;
 *	extern unsigned char * help_select_file;
 *	extern unsigned char * inst_complete1;
 *	extern unsigned char * inst_complete2;
 *	extern unsigned char * inst_logfile_name;
 *	extern unsigned char * help_with_confirm;
 *	extern unsigned char * version_msg1;
 *	extern unsigned char * version_msg2;
 *	extern unsigned char * version_msg3;
 *	extern unsigned char * set_inst_mode;
 *	extern unsigned char * welcome_msg;
 *	extern unsigned char * help_inst_mode;
 *	extern unsigned char * welcome_msg2;
 *	extern unsigned char * help_with_settings;
 *	extern unsigned char * help_change_startseq;
 *	extern unsigned char * error_msg;
 *	extern unsigned char * dos_error_type;
 *	extern unsigned char * sorry_msg;
 *	extern unsigned char * please_confirm;
 *	extern unsigned char * press_escape;
 *	extern unsigned char * valid_range;
 *	extern unsigned char * bad_params_copy;
 *	extern unsigned char * invalid_drawer_name;
 *	extern unsigned char * global_error_msg;
 *	extern unsigned char * string_length_msg;
 *	extern unsigned char * cannot_examine_msg;
 *	extern unsigned char * noexist_msg;
 *	extern unsigned char * cannot_assign_msg;
 *	extern unsigned char * divide_zero_msg;
 *	extern unsigned char * bad_cat_msg;
 *	extern unsigned char * err_memory_msg;
 *	extern unsigned char * user_abort_msg;
 *	extern unsigned char * script_line_msg1;
 *	extern unsigned char * script_line_msg2;
 *	extern unsigned char * trans_create_drawer;
 *	extern unsigned char * trans_insert_commands;
 *	extern unsigned char * trans_create_textfile;
 *	extern unsigned char * trans_end_textfile;
 *	extern unsigned char * trans_execute;
 *	extern unsigned char * trans_run;
 *	extern unsigned char * trans_arexx;
 *	extern unsigned char * trans_rename;
 *	extern unsigned char * trans_delete;
 *	extern unsigned char * trans_aborting;
 *	extern unsigned char * trans_exiting;
 *	extern unsigned char * no_file_msg;
 *	extern unsigned char * prob_ss_msg;
 *	extern unsigned char * err_writing_msg;
 *	extern unsigned char * err_unknown_msg;
 *	extern unsigned char * err_reading_text;
 *	extern unsigned char * err_cannot_create;
 *	extern unsigned char * err_cannot_copy;
 *	extern unsigned char * err_overwrite;
 *	extern unsigned char * err_execute_path;
 *	extern unsigned char * err_no_nil;
 *	extern unsigned char * err_no_command;
 *	extern unsigned char * err_no_rexx;
 *	extern unsigned char * err_cannot_duplock;
 *	extern unsigned char * err_reading_msg;
 *	
 *	void localize_text(void);
 *	
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */


/* text strings that need to be translatable */

#include <exec/types.h>

#ifdef ONLY2_0
#include <clib/locale_protos.h>
#include <locale.p>
#include <libraries/locale.h>
#endif

#define CATCOMP_STRINGS
#define CATCOMP_NUMBERS
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



/* char __asm *GetLocalText(register __d0 WORD num); */

char *GetLocalText(WORD num);



/* char __asm GetLocalText(register __d0 WORD num) */

char *GetLocalText(WORD num)
{
	extern char *override_text(UWORD num);
	struct AppString	*appstr = AppStrings;
	char				*str = "BAD TEXT";
	WORD				i;

	if (!(str = override_text(num)))
	{
        for (i=0; i<NumAppStrings; i++, appstr++)
        {
            if (appstr->as_ID == num)
            {
                str = appstr->as_Str;
                break;
            }
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

char	*minuser_tt 	= "MINUSER",
		*defuser_tt 	= "DEFUSER",
		*pretend_tt 	= "PRETEND",
		*script_tt 		= "SCRIPT",
		*appname_tt 	= "APPNAME",
		*logfile_tt 	= "LOGFILE",
		*nopretend_tt 	= "NOPRETEND",
		*nolog_tt 		= "NOLOG",
		*noprint_tt 	= "NOPRINT",
		*print_tt 		= "PRINT",
		*log_tt 		= "LOG",
		*language_tt 	= "LANGUAGE",
		*false_tv 		= "FALSE",
		*novice_tv 		= "NOVICE",
		*average_tv 	= "AVERAGE",
		*expert_tv 		= "EXPERT";

char	*dot_info 		= ".info",
		*dot_font 		= ".font";

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

char	*no_file_msg,
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
	
	transcript_header 		= GetLocalText(TRANS_HEADER);
	def_transcript_name 	= GetLocalText(TRANS_DEF_NAME);
	usage_tx 				= GetLocalText(TX_USAGE);

	no_find_icon 			= GetLocalText(MSG_NO_SCRIPT_ICON);

	trans_made_drawer 		= GetLocalText(TRANS_USER_DRAWER);
	trans_copy_file 		= GetLocalText(TRANS_COPY_FILE);
	trans_copy_drawer 		= GetLocalText(TRANS_COPY_DRAWER);
	trans_ask_choice 		= GetLocalText(TRANS_ASK_CHOICE);
	trans_no_options 		= GetLocalText(TRANS_NO_OPTIONS);
	trans_ask_options 		= GetLocalText(TRANS_ASK_OPTIONS);
	trans_ask_string 		= GetLocalText(TRANS_ASK_STRING);
	trans_ask_number 		= GetLocalText(TRANS_ASK_NUMBER);
	trans_ask_yesno 		= GetLocalText(TRANS_ASK_BOOL);
	trans_dest_drawer 		= GetLocalText(TRANS_DEST_DRAWER);
	trans_ask_drawer 		= GetLocalText(TRANS_ASK_DRAWER);
	trans_ask_file 			= GetLocalText(TRANS_ASK_FILE);

	prob_with_source 		= GetLocalText(MSG_PROB_SOURCE);
	prob_with_file 			= GetLocalText(MSG_PROB_FILE);

	help_copyfiles 			= GetLocalText(TX_HELP_COPY);
	help_with_choice 		= GetLocalText(TX_HELP_ASKCHOICE);
	help_with_options 		= GetLocalText(TX_HELP_ASKOPTIONS);
	help_ask_string 		= GetLocalText(TX_HELP_ASKSTRING);
	help_ask_number 		= GetLocalText(TX_HELP_ASKNUMBER);
	help_insert_disk 		= GetLocalText(TX_HELP_ASKDISK);
	help_ask_yesno 			= GetLocalText(TX_HELP_ASKBOOL);
	help_select_drawer 		= GetLocalText(TX_HELP_SELECTDIR);
	help_select_file 		= GetLocalText(TX_HELP_SELECTFILE);
	inst_complete1 			= GetLocalText(TX_INST_COMPLETE);
	inst_complete2 			= GetLocalText(TX_INST_COMP_WHERE);
	inst_logfile_name 		= GetLocalText(TX_INST_LOGFILE);
	help_with_confirm 		= GetLocalText(TX_HELP_CONFIRM);
	version_msg1 			= GetLocalText(TX_VERSION_SOURCE);
	version_msg2 			= GetLocalText(TX_VERSION_DEST);
	version_msg3 			= GetLocalText(TX_VERSION_NONE);
	set_inst_mode 			= GetLocalText(TX_INST_MODE);
	welcome_msg 			= GetLocalText(TX_WELCOME);
	help_inst_mode 			= GetLocalText(TX_HELP_INSTMODE);
	welcome_msg2 			= GetLocalText(TX_WELCOME_OPT);
	help_with_settings 		= GetLocalText(TX_HELP_SETTINGS);
	help_change_startseq 	= GetLocalText(TX_HELP_CHANGE_SS);
	error_msg 				= GetLocalText(TX_ERROR);
	dos_error_type 			= GetLocalText(TX_DOSERROR);
	sorry_msg 				= GetLocalText(TX_SORRY);
	please_confirm 			= GetLocalText(TX_CONFIRM);
	press_escape 			= GetLocalText(TX_ESCAPE);
	valid_range 			= GetLocalText(TX_RANGE);

	bad_params_copy 		= GetLocalText(MSG_BAD_PARAMS_COPY);
	invalid_drawer_name 	= GetLocalText(MSG_INVALID_DRAWER_NAME);

/* interp.c */
	global_error_msg 		= GetLocalText(MSG_ONERROR);
	string_length_msg 		= GetLocalText(MSG_TOO_LONG);
	cannot_examine_msg 		= GetLocalText(MSG_NOEXAMINE_OBJECT);
	noexist_msg 			= GetLocalText(MSG_NOEXIST_OBJECT);
	cannot_assign_msg 		= GetLocalText(MSG_CANNOT_ASSIGN);
	divide_zero_msg 		= GetLocalText(MSG_DIVIDE_ZERO);
	bad_cat_msg 			= GetLocalText(MSG_BAD_CAT);
	err_memory_msg 			= GetLocalText(MSG_ERR_MEMORY);
	user_abort_msg 			= GetLocalText(MSG_USER_ABORT);
	script_line_msg1 		= GetLocalText(MSG_ABORT_LINE);
	script_line_msg2 		= GetLocalText(MSG_PROBLEM_LINE);

/* statement.c */

	trans_create_drawer 	= GetLocalText(TRANS_CREATE_DRAWER);
	trans_insert_commands 	= GetLocalText(TRANS_INSERT_COMMANDS);
	trans_create_textfile 	= GetLocalText(TRANS_CREATE_TEXTFILE);
	trans_end_textfile 		= GetLocalText(TRANS_END_TEXTFILE);
	trans_execute 			= GetLocalText(TRANS_EXECUTE);
	trans_run				= GetLocalText(TRANS_RUN);
	trans_arexx 			= GetLocalText(TRANS_AREXX);
	trans_rename 			= GetLocalText(TRANS_RENAME);
	trans_delete 			= GetLocalText(TRANS_DELETE);
	trans_aborting 			= GetLocalText(TRANS_ABORTING);
	trans_exiting 			= GetLocalText(TRANS_EXITING);

	no_file_msg 			= GetLocalText(MSG_NO_FILE);
	prob_ss_msg 			= GetLocalText(MSG_PROB_SS);
	err_writing_msg 		= GetLocalText(MSG_ERR_WRITING);
	err_unknown_msg 		= GetLocalText(MSG_ERR_UNKNOWN);
	err_reading_text 		= GetLocalText(MSG_ERR_READING);

/* action.c */

	err_cannot_create 		= GetLocalText(MSG_CANNOT_CREATE);
	err_cannot_copy 		= GetLocalText(MSG_CANNOT_COPY);
	err_overwrite 			= GetLocalText(MSG_OVERWRITE);
	err_execute_path 		= GetLocalText(MSG_EXECUTE_PATH);
	err_no_nil 				= GetLocalText(MSG_NO_NIL);
	err_no_command 			= GetLocalText(MSG_NO_COMMAND);
	err_no_rexx 			= GetLocalText(MSG_NO_REXX);
	err_cannot_duplock 		= GetLocalText(MSG_NO_DUPLOCK);

/* start.c */

	err_reading_msg 		= GetLocalText(ERR_READING_US);
}
