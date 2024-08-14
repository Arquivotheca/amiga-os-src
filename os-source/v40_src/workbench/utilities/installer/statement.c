/* ========================================================================= *
 * Statement.c - executes various statements for installation                *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#include <stddef.h>
#include <setjmp.h>
#include <string.h>

#include "macros.h"

#include "installer.h"
#include "text.h"

struct InstallationRecord istate;
extern struct Value		*vtop;
extern UBYTE			ierror;
extern struct TreeNode	*ehead;
extern UWORD pretend_flag, user_level;

extern char	*trans_create_drawer,
			*trans_copy_file,
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

extern char	*no_drawer_msg,
			*no_file_msg,
			*prob_ss_msg,
			*err_writing_msg,
			*err_unknown_msg,
			*err_reading_text;

UBYTE		global_delopts = DELOPT_FAIL,
			local_delopts = DELOPT_FAIL;
BOOL		show_gauge;

void cleanup_irecord(void)
{	struct TextList		*tl;
	struct ToolType		*tt;

	vfree( &istate.help_val );
	vfree( &istate.prompt_val );
	vfree( &istate.source_val );
	vfree( &istate.dest_val );
	vfree( &istate.name_val );
	vfree( &istate.default_val );
	vfree( &istate.deftool_val );
	vfree( &istate.pattern_val );

	if (istate.choices)
	{	WORD i;
		for (i=0; i<istate.numchoices; i++)
		{	FreeString(istate.choices[i]);
		}
		FreeMem(istate.choices,istate.numchoices * sizeof (LONG));
		istate.choices = NULL;
		istate.numchoices = 0;
	}

		/* free text list */

	while (tl = (struct TextList *)RemHead((void *)&istate.text_list))
		free_text(tl);

	while (tt = istate.tooltypes)
	{	istate.tooltypes = tt->next;

		FreeString(tt->tooltype);
		FreeString(tt->toolval);
		FreeMem(tt,sizeof *tt);
	}

	istate.InstallAction = 0;
	istate.minval = 0x80000000;
	istate.maxval = 0x7fffffff;
	istate.flags = istate.bits = 0;

	istate.delopts = global_delopts;
}

void popall(struct Value *vbase)
{	while (vtop > vbase)
	{	vtop--;
		vfree(vtop);
	}
}

	/* general routine to take care of single-string parameters... */

void do_string_param(struct Value *v,WORD args, struct Value *d, char *funcname, long flags)
{	if (argcheck(args,1,1,funcname)) return;	/* check argument count			*/

	if ((1 << istate.InstallAction) & flags)
	{	if (v->type != TYPE_STRING)
		{	err(ehead,ERR_SCRIPT,"%s: String argument expected",funcname);
			return;
		}

		if (d->type)
		{	err(ehead,ERR_SCRIPT,"%s: Parameter already defined",funcname);
			return;
		}

		*d = *v;
		vtop = v;
	}
	else popall(v);
}

help_missing(char *me,BOOL confirmed)
{
	if (!confirmed || (istate.flags & IFLAG_CONFIRM))
	{	if (!istate.prompt_val.type) err(ehead,ERR_SCRIPT,"%s: No prompt specified",me);
		if (!istate.help_val.type) err(ehead,ERR_SCRIPT,"%s: No help text specified",me);
	}
	return (int)ierror;
}


/* =============================== Ask Actions ============================ */

void do_askfile(struct Value *vbase, WORD args)
{
	char *fname;

	debug_action(FUNC_ASKFILE);

	popall(vbase);								/* pop all parameters		*/

	if (help_missing(FUNC_ASKFILE,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_file;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_file,STR_VAL(vbase->v));
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	/* verify parameters */

	if (istate.default_val.type != TYPE_STRING)
		err_msg(ehead,ERR_SCRIPT,FUNC_ASKFILE,"No default specified");

	fname = select_file(&istate);

	if (ierror) eval(0);
	else
	{	TempString(fname,strlen(fname),vbase);
		vtop++;
	}
}

void do_askdir(struct Value *vbase, WORD args)
{
	char *dir;

	debug_action(FUNC_ASKDIR);

	popall(vbase);								/* pop all parameters		*/

		/* NOTE: We should probably do some sanity checking here... */

	if (help_missing(FUNC_ASKDIR,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_drawer;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_drawer,STR_VAL(vbase->v));
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

		/* verify parameters */

	if (!istate.default_val.type)
	{
		err_msg(ehead,ERR_SCRIPT,FUNC_ASKDIR,"No default specified");
		return;
	}

	dir = select_dir(&istate);

	if (ierror) eval(0);
	else
	{	TempString(dir,strlen(dir),vbase);
		vtop++;
	}
}

void do_askstring(struct Value *vbase, WORD args)
{
	char *string;

	debug_action(FUNC_ASKSTRING);

	popall(vbase);								/* pop all parameters		*/

		/* The DEFAULT default is a NULL string */

	if (!istate.default_val.type)
		TempString("",0,&istate.default_val);

		/* verify parameters */

	if (help_missing(FUNC_ASKSTRING,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_string;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_string,STR_VAL(vbase->v));
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	string = askstring_page(&istate);

	if (ierror) eval(0);
	else
	{	TempString(string,strlen(string),vbase);
		vtop++;
	}
}

void do_asknumber(struct Value *vbase, WORD args)
{	long				result;

	debug_action(FUNC_ASKNUMBER);

	popall(vbase);								/* pop all parameters		*/

		/* The DEFAULT default is a NULL string */

	if (istate.default_val.type == TYPE_NONE)
	{	istate.default_val.type = TYPE_INTEGER;
		istate.default_val.v = 0;
	}

	istate.default_val.v =
		(void *)clamp(istate.minval,(LONG)istate.default_val.v,istate.maxval);

		/* verify parameters */

	if (help_missing(FUNC_ASKNUMBER,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_number;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_number,vbase->v);
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	result = asknumber_page(&istate);

	if (ierror) eval(0);
	else
	{	vtop->type = TYPE_INTEGER;
		vtop->v = (void *)result;
		vtop++;
	}
}

void do_askchoice(struct Value *vbase, WORD args)
{	int result = 0;

	debug_action(FUNC_ASKCHOICE);

	popall(vbase);								/* pop all parameters			*/

		/* set up a DEFAULT default... :-) */

	if (istate.default_val.type == TYPE_NONE)
	{	istate.default_val.type = TYPE_INTEGER;
		istate.default_val.v = 0;
	}

	if (help_missing(FUNC_ASKCHOICE,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_choice;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_choice,istate.choices[(LONG)vbase->v] + 1);
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	if (!istate.choices)
		err_msg(ehead,ERR_SCRIPT,FUNC_ASKCHOICE,"No choices specified");

		/* clip default to number of choices */

	istate.default_val.v =
		(void *)clamp(0,(long)istate.default_val.v,istate.numchoices);

	if (!ierror) result = radio_page(&istate);

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;					/* from the radio page		*/
	vtop++;
}

void do_askoptions(struct Value *vbase, WORD args)
{	int		result = 0;
	LONG	r;
	WORD	i;

	debug_action(FUNC_ASKOPTIONS);

	popall(vbase);								/* pop all parameters			*/

		/* set up a DEFAULT default... :-) */

	if (istate.default_val.type == TYPE_NONE)
	{	istate.default_val.type = TYPE_INTEGER;
		istate.default_val.v = (void *)-1;
	}

	if (help_missing(FUNC_ASKOPTIONS,0)) return;

	if (NOVICE_MODE)
	{	extern char 	*trans_ask_options;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript(trans_ask_options);
		r = (LONG)vbase->v;
		for (i=0;i<istate.numchoices;i++)
			if (r & 1 << i) transcript("  %s\n",STR_VAL(istate.choices[i]));
		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	if (!istate.choices)
		err_msg(ehead,ERR_SCRIPT,FUNC_ASKOPTIONS,"No choices specified");

	if (!ierror) result = checkbox_page(&istate);

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;					/* from the radio page		*/
	vtop++;
}

void do_askbool(struct Value *vbase, WORD args)
{	long yesno_page(struct InstallationRecord *ir);
	int result = 0;

	debug_action(FUNC_ASKBOOL);

	popall(vbase);								/* pop all parameters			*/

		/* set up a DEFAULT default... :-) */

	if (istate.default_val.type == TYPE_NONE)
	{	istate.default_val.type = TYPE_INTEGER;
		istate.default_val.v = 0;
	}

	if (help_missing(FUNC_ASKBOOL,0)) return;

	if (NOVICE_MODE)
	{	extern char		*trans_ask_yesno;

		*vbase = istate.default_val;
		transcript_wrap("\n>%s\n",STR_VAL(istate.prompt_val.v));
		transcript( trans_ask_yesno, (LONG)vbase->v ?
			(istate.numchoices > 1 ? STR_VAL(istate.choices[0]) : GetLocalText(TX_YES)) :
			(istate.numchoices > 1 ? STR_VAL(istate.choices[1]) : GetLocalText(TX_NO)) );

		dupstring(vbase);
		vtop = vbase + 1;
		return;
	}

	if (!ierror) result = yesno_page(&istate);

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;					/* from the radio page		*/
	vtop++;
}

void do_askdisk(struct Value *vbase, WORD args)
{	long askdisk_page(struct InstallationRecord *ir);
	int result = 0;

	debug_action(FUNC_ASKDISK);

	popall(vbase);								/* pop all parameters			*/

	if (help_missing(FUNC_ASKDISK,0)) return;
	if (!verify_string(&istate.dest_val,FUNC_ASKDISK)) return;

	if (!ierror) result = askdisk_page(&istate);

	if (result && istate.name_val.type == TYPE_STRING)
	{	long MatchAssign(char *new,char *old,long opts);

		MatchAssign(STR_VAL(istate.name_val.v),
			STR_VAL(istate.dest_val.v),LDF_VOLUMES);
	}

	eval(0L);
}

/* =========================== Install Statements ========================= */

void do_newdir(struct Value *vbase, WORD args)
{	BOOL create_dir(char *path,BOOL icon);
	int				result = 0;

	if (argcheck(args,1,1,FUNC_MAKEDIR)) return;	/* check argument count			*/
	if (!verify_string(vbase,FUNC_MAKEDIR)) return;/* check argument type			*/
	if (help_missing(FUNC_MAKEDIR,1)) return;

	if (confirm_page(&istate))
	{
		transcript(trans_create_drawer,STR_VAL(vbase->v));

		if (!PRETEND_MODE)
 		{	if (create_dir(STR_VAL(vbase->v),istate.flags & IFLAG_INFOS))
				result = 1;
		}
		else result = 1;
	}

	popall(vbase);								/* pop all parameters			*/

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;					/* from the radio page		*/
	vtop++;
}

void do_files(struct Value *vbase, WORD args)
{	struct String	*result;
	WORD			i = 0;

	debug_action(FUNC_COPYFILES);

	popall(vbase);								/* pop all parameters			*/

	if (help_missing(FUNC_COPYFILES,1)) return;

	if (!verify_string(&istate.source_val,FUNC_COPYFILES)) return;
	if (!verify_string(&istate.dest_val,FUNC_COPYFILES)) return;

	/* verify that we have the proper parameters... */

	/*
		must have choices, params, or "all"
		source, dest should be filled in.
		- infos may be set
		- name may be set, if there is only 1 file
	*/

	if (istate.flags & IFLAG_ALL) i++;
	if (istate.pattern_val.type == TYPE_STRING) i++;
	if (istate.choices != NULL) i++;

	if (i > 1)
	{
		err_msg(ehead,ERR_SCRIPT,FUNC_COPYFILES,
			"All, Pattern and Choices are mutually exclusive");
		return;		
	}

	result = copy_page(&istate);

	if (ierror) eval(0);
	else
	{
		vbase->v = result;
		vbase->type = (result ? TYPE_STRING : TYPE_NONE);
		vbase->storage = STORE_TEMP;
		vtop++;
	}
}

/* (copylib (source-path) (dest-dir) (prompt) (help) (info) (confirm)) */

void do_libs(struct Value *vbase, WORD args)
{	LONG			r, oldver = 0,
					newver;
	char			filename[256],
					newname[40],
					*basename,
					*dir,
					*name;
	BPTR			lock,
					flock,
					oldlock = NULL;

	debug_action(FUNC_COPYLIB);

	popall(vbase);								/* pop all parameters			*/

	if (help_missing(FUNC_COPYLIB,1)) return;
	if (!verify_string(&istate.source_val,FUNC_COPYLIB)) return;
	if (!verify_string(&istate.dest_val,FUNC_COPYLIB)) return;

	dir = STR_VAL(istate.dest_val.v);
	name = STR_VAL(istate.source_val.v);
	basename = (istate.name_val.type == TYPE_STRING ?
		STR_VAL(istate.name_val.v) : FileOnly(name));

	lock = Lock(dir,ACCESS_READ);

	flock = Lock(name,ACCESS_READ);
	if (flock == NULL)
	{	err_msg(ehead,ERR_DOS,FUNC_COPYLIB,no_file_msg);
		goto bye;
	}

	newver = GetFileVersion(name);
	/* if (newver == 0) newver = 1; */
	UnLock(flock);

	if (lock == NULL)
	{
		oldver = 0;			/* no dir, so no older version */
	}
	else
	{
		oldlock = CurrentDir(lock);

		if (flock = Lock(basename,ACCESS_READ))
		{
			oldver = GetFileVersion(basename);
			/* if (oldver == 0) oldver = 1; */

			UnLock(flock);
		}

		CurrentDir(oldlock);
	}

	if (oldver == newver) goto bye;

	if (oldver > newver)
	{
		if ( !(istate.flags & IFLAG_CONFIRM) ) goto bye;

		if (!version_page(&istate,oldver,newver)) goto bye;

	}
	else
	{		/* source version newer */
		if ( (istate.flags & IFLAG_CONFIRM) &&
			!version_page(&istate,oldver,newver) ) goto bye;
	}

	show_gauge = !(istate.bits & IBITS_NOGAUGE);

	if (!PRETEND_MODE)
	{
		local_delopts = istate.delopts;
		r = copy_file(name,dir,basename,0);
	}
	else r = 1;

	if (r) transcript(trans_copy_file,name,dir);

	if (istate.flags & IFLAG_INFOS)
	{
		strcpy(filename,name);
		strcat(filename,".info");
		strcpy(newname,basename);
		strcat(newname,".info");

		if (!PRETEND_MODE)
		{
			local_delopts = istate.delopts | DELOPT_OKNOSRC;
			r = copy_file(filename,dir,newname,istate.bits & IBITS_NOPOSITION);
		}
		else r = 1;

		if (r) transcript(trans_copy_file,filename,dir);
	}

bye:
	if (lock) UnLock(lock);

	eval(0L);
}

/* #ifdef RENAME_IT */

void do_startup(struct Value *vbase, WORD args)
{
	extern char			app_startup_name[],
						best_path[],
						format_string[];
	extern int			best_position;
	BPTR				fh = NULL;
	long				size;
	char				*thePath;
	char 				*buffer = NULL;
	BPTR				olddir,
						boot;
	int					thePos,
						result;

	if (user_level != USER_NOVICE)		/* always confirm if not novice */
		istate.flags |= IFLAG_CONFIRM;

	debug_action("Editing user_startup file");

	if (help_missing(FUNC_STARTUP,0)) return;

	if (!verify_string(vbase,FUNC_STARTUP)) return;	/* check argument type	*/

		/* check to see if command to execute "S:user-startup" exists */

		/* look for boot disk */
	if ( (boot = boot_volume()) == NULL ) goto bye;

	olddir = CurrentDir(boot);

		/* look for startup-sequence */
	result = scan_scripts("s/Startup-Sequence",TRUE);

	CurrentDir(olddir);
	UnLock(boot);

	if (result > 0)
	{
		thePath = best_path;
		thePos = best_position;

		if (istate.flags & IFLAG_CONFIRM)
		{
			if (!change_startup(&thePath,&thePos))
				goto edit_user;		/* but do edit user-startup */
		}

		transcript(trans_insert_commands,app_startup_name,thePath,thePos);

		if (!pretend_flag)
		{
			if ( (size = GetFileSize(thePath)) &&
				(fh = Open(thePath,MODE_OLDFILE)) )
			{
				buffer = MemAlloc(size,0L);

				if (buffer == NULL)
				{	memerr();
					goto bye;
				}

				if (Read(fh,buffer,size) != size)
				{
					err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
					goto bye;
				}

				Close(fh);

#ifdef RENAME_IT
		/* rename old "startup-sequence" to "startup-sequence.bak" */
				strcpy(format_string,thePath);
				strcat(format_string,".bak");
				DeleteFile(format_string);
				Rename(thePath,format_string);
#endif

				if (fh = Open(thePath,MODE_NEWFILE))
				{
					if (Write(fh,buffer,thePos) != thePos)
					{
						err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
						Close(fh);
						fh = NULL;
#ifdef RENAME_IT
						Rename(format_string,thePath);
#endif
						goto bye;
					}

					Fprintf((void *)fh,"if exists %s\n  execute %s\nendif\n",
						app_startup_name,app_startup_name);

					if (size - thePos > 0)
						Write(fh,&buffer[thePos],size - thePos);

					Close(fh);
					fh = NULL;
					MemFree(buffer);
					buffer = NULL;
				}
				else
				{
					err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
#ifdef RENAME_IT
					Rename(format_string,thePath);
#endif
					goto bye;
				}
			}
			else
			{
				err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
				goto bye;
			}
		}
		else /* pretend */
		{
			goto bye;
		}
	}
	else if (result == 0)
	{
		/*	They have a startup-sequence, but it must be weird. What we want
			to do is give the user a message saying their startup-sequence
			is unusual and they will have to manually add the command to
			execute user-startup if it is not already doing so. What should
			be done in the case of novices? I don't know.

			For now, just assume the startup-sequence is OK and continue.
		*/

#if 0	/* !!! old, bad way */
		err_msg(ehead,ERR_DATA,FUNC_STARTUP,prob_ss_msg);
		goto bye;
#endif
	}

	/* if result == -1, then already have "S:user-startup" in startup script */

	/*	The installer controls "s:user-startup", so we ask the user
		to confirm editing the file ONLY if they are experts. Pretend
		mode is handled inside edit_startup.									*/

edit_user:
	if (user_level != USER_EXPERT || confirm_page(&istate))
		edit_startup(STR_VAL(vbase->v));

bye:
	if (buffer)	MemFree(buffer);
	if (fh) Close(fh);
	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_tooltype(struct Value *vbase, WORD args)
{
	debug_action(FUNC_TOOLTYPE);

	if (help_missing(FUNC_TOOLTYPE,1)) return;
	if (!verify_string(&istate.dest_val,FUNC_TOOLTYPE)) return;

	if (confirm_page(&istate))
	{
		/* modify the icon */
		if (!PRETEND_MODE)
		{
			if (istate.maxval == 0x7fffffff) istate.maxval = 0;

			 modify_icon(STR_VAL(istate.dest_val.v),istate.tooltypes,
				(istate.default_val.type != TYPE_NONE ? STR_VAL(istate.default_val.v) : NULL),
				istate.maxval,istate.bits & IBITS_NOPOSITION,istate.flags & IFLAG_OPTIONAL);
		}
	}

	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_textfile(struct Value *vbase, WORD args)
{	struct TextList		*tl;
	BPTR				fh;

	if (istate.dest_val.type != TYPE_STRING)
		err_msg(ehead,ERR_SCRIPT,FUNC_TEXTFILE,"Bad destination specifier");

	if (help_missing(FUNC_TEXTFILE,1)) return;
	if (!verify_string(&istate.dest_val,FUNC_TEXTFILE)) return;

	if (confirm_page(&istate))
	{
		/* Q: allow them to change name of text file as part of confirm?? */
		/* NOTE: If error opening or writing file, should give option to retry?
		perhaps somewhere else?
		*/

		transcript(trans_create_textfile, STR_VAL(istate.dest_val.v));

		if (!PRETEND_MODE)
		{	if (fh = Open(STR_VAL(istate.dest_val.v),MODE_NEWFILE))
			{	for (tl=GetHead(&istate.text_list); tl; tl = NextNode(tl))
				{	if (Write(fh,tl+1,tl->size) != tl->size)
					{
						err(ehead,ERR_DOS,err_writing_msg);
						goto bye;
					}
					transcript((char *)(tl+1));
				}
				Close(fh);	/* check for error?? */
			}
			else
			{	err(ehead,ERR_DOS,no_file_msg);
			}
		}

bye:	transcript(trans_end_textfile);
	}
	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_execute(struct Value *vbase, WORD args)
{	int		result = 1;
	char	command[256],
			*v;

	do_cat_sub(vbase,args," ",FUNC_EXECUTE);

	v = STR_VAL(vbase->v);

	if (help_missing(FUNC_EXECUTE,1)) return;

	if (confirm_page(&istate))
	{		/* clean up this line when we merge args */
		transcript(trans_execute,v);

		if (!PRETEND_MODE)
		{
				/* execute a dos script */
			strcpy(command,"Execute ");
			strncat(command,v,248L);
			if (strcmp(command+8,v))
			{
				err_msg(ehead,ERR_SCRIPT,FUNC_EXECUTE,"Command too long");
			}
			else result = execute_command(command);
		}
	}

	popall(vbase);								/* pop all parameters			*/

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;
	vtop++;
}

void do_run(struct Value *vbase, WORD args)
{	int		result = 1;
	char	*v;

	do_cat_sub(vbase,args," ",FUNC_RUN);

	v = STR_VAL(vbase->v);

	if (help_missing(FUNC_RUN,1)) return;

	if (confirm_page(&istate))
	{	transcript(trans_run,v);

		if (!PRETEND_MODE)
		{
				/* execute a program */
			result = execute_command(v);
		}
	}

	popall(vbase);								/* pop all parameters			*/

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;					/* from the radio page		*/
	vtop++;
}

void do_rexx(struct Value *vbase, WORD args)
{	int		result = 1;
	char	*v;

	do_cat_sub(vbase,args," ",FUNC_REXX);

	v = STR_VAL(vbase->v);

	if (help_missing(FUNC_REXX,1)) return;

	if (confirm_page(&istate))
	{	transcript(trans_arexx,v);

		if (!PRETEND_MODE)
		{
				/* execute a rexx script */
			result = send_rexx(v);
		}
	}

	popall(vbase);								/* pop all parameters			*/

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;
	vtop++;
}

void do_rename(struct Value *vbase, WORD args)
{	struct Value		*v2 = vbase+1;
	char				*name = STR_VAL(v2->v);
	LONG				result = TRUE;

	if (argcheck(args,2,2,FUNC_RENAME)) return;	/* check argument count			*/
	if (!verify_string(vbase,FUNC_RENAME)) return;
	if (!verify_string(v2,FUNC_RENAME)) return;

	if (help_missing(FUNC_RENAME,1)) return;
	if (*name == 0)
		err_msg(ehead,ERR_SCRIPT,FUNC_RENAME,"Zero length path name");

	if (confirm_page(&istate))
	{
		if (!PRETEND_MODE)
		{
			if (istate.flags & IFLAG_OPTIONAL)	/* actually a disk relabel */
				result = RelabelDisk(STR_VAL(vbase->v),name);
			else result = Rename(STR_VAL(vbase->v),STR_VAL(v2->v));
		}

		transcript(trans_rename,STR_VAL(vbase->v),STR_VAL(v2->v));
	}

	popall(vbase);								/* pop all parameters			*/

	vtop->type = TYPE_INTEGER;					/* return an integer		*/
	vtop->v = (void *)result;
	vtop++;
}

static BOOL fdelete(char *name)
{	BPTR					lock;
	struct FileInfoBlock	*fib;
	LONG					protection = 0,
							result = 1;

	if (!PRETEND_MODE)
	{
		if (lock = Lock(name,ACCESS_READ))
		{
			if (fib = AllocMem(sizeof *fib,MEMF_PUBLIC|MEMF_CLEAR))
			{
				if (Examine(lock,fib))
				{
					protection =  fib->fib_Protection;
				}
				else
				{
					FreeMem(fib,sizeof *fib);
					UnLock(lock);
					return FALSE;
				}

				FreeMem(fib,sizeof *fib);
				UnLock(lock);
			}
			else
			{
				UnLock(lock);
				return FALSE;
			}
		}

		if (protection & (FIBF_WRITE | FIBF_DELETE))
		{
			if ( !(local_delopts & (DELOPT_ASKUSER | DELOPT_FORCE)) )
				return FALSE;

			if (user_level == USER_NOVICE && !(local_delopts & DELOPT_FORCE))
				return FALSE;

			if ( (user_level != USER_NOVICE) && (local_delopts & DELOPT_ASKUSER) &&
				!save_page(NULL,name,askdelete_page) ) return FALSE;

			SetProtection(name,0);	/* trying to delete, so other flags unimportant */
		}

		result = DeleteFile(name);
	}

	if (result) transcript(trans_delete,name);

	return TRUE;
}

void do_delete(struct Value *vbase, WORD args)
{	WORD			i;
	struct Value	*v;

	if (help_missing(FUNC_DELETE,1)) return;

	if (confirm_page(&istate))
	{	for (i=0; i<args; i++)
		{	v = vbase+i;
			if (v->type == TYPE_STRING)
			{	char *str = STR_VAL(v->v);

				local_delopts = istate.delopts;

				if (fdelete(str) && istate.flags & IFLAG_INFOS)
				{	char	name[256];

					strcpy(name,str);
					strcat(name,".info");
					fdelete(name);
				}
			}
		}
	}

	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_abort(struct Value *vbase, WORD args)
{	extern jmp_buf	exit_buf;
	char			*str;

	if (args == 0) str = err_unknown_msg;
	else
	{
		do_cat_sub(vbase,args,"",FUNC_ABORT);
		str = STR_VAL(vbase->v);
	}

	transcript(trans_aborting);
	ierror = ERR_HANDLED;
	err(ehead,ERR_ABORT,str);
	longjmp(exit_buf,1);
}

void do_exit(struct Value *vbase, WORD args)
{	extern jmp_buf			exit_buf;
	extern struct String	*extra_exit_text;
	extern BOOL				exit_quiet;

	if (args > 0 && vbase->type == TYPE_STRING)
	{
		do_cat_sub(vbase,args,"",FUNC_EXIT);
		extra_exit_text = vbase->v;
		vbase->v = NULL;
		vbase->type = TYPE_NONE;
	}

	if (istate.bits & IBITS_RESIDENT) exit_quiet = TRUE;

	transcript(trans_exiting);
	ierror = 0;
	longjmp(exit_buf,1);
}

void do_complete(struct Value *vbase, WORD args)
{	extern void completion(long);

	completion((long)vbase->v);
	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_message(struct Value *vbase, WORD args)
{	do_cat_sub(vbase,args,"",FUNC_MESSAGE);		/* concatenate all strings (??) */

	if (!ierror && !NOVICE_MODE)
	{	istate.prompt_val = *vbase;
		vtop = vbase;
		message_page(&istate);
	}

	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_working(struct Value *vbase, WORD args)
{
	do_cat_sub(vbase,args,"",FUNC_WORKING);		/* concatenate all strings (??) */

	if (!ierror)	/* don't check for novice, always prints messages */
	{	istate.prompt_val = *vbase;
		vtop = vbase;
		working_page(&istate);
	}

	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

void do_welcome(struct Value *vbase, WORD args)
{
	do_cat_sub(vbase,args,"","WELCOME");		/* concatenate all strings (??) */

	if (!ierror)	/* don't check for novice, always prints messages */
	{
		istate.help_val = *vbase;
		vtop = vbase;
		if (!welcome_page(&istate)) ierror = ERR_ABORT;
	}

	popall(vbase);								/* pop all parameters			*/
	eval(0L);
}

/* =============================== Parameters ============================= */

void do_merged_params(struct Value *vbase, WORD args, struct Value *d, char *text)
{
	if (!istate.InstallAction)
	{	popall(vbase);
		return;
	}

	if (d->type)
	{	err(ehead,ERR_SCRIPT,"%s text already defined",text);
		return;
	}

	do_cat_sub(vbase,args,"","PARAMS");		/* concatenate all strings (??) */

	if (!ierror)
	{	*d = *vbase;
		vtop = vbase;
	}
}

	/* get all the help strings, merge them, and pass to the install record. */

void do_help(struct Value *vbase, WORD args)
{	do_merged_params(vbase,args,&istate.help_val,"HELP: Help");
}

	/* get all the prompt strings, merge them, and pass to the install record. */

void do_prompt(struct Value *vbase, WORD args)
{	do_merged_params(vbase,args,&istate.prompt_val,"PROMPT: Prompt");
}

void do_trans(struct Value *vbase, WORD args)
{
	do_cat_sub(vbase,args,"","TRANSCRIPT"); /* concatinate strings			*/
	transcript("%s\n",STR_VAL(vbase->v));	/* print string to transcript	*/
	popall(vbase);							/* pop all parameters			*/
	eval(0L);
}

void do_choices(struct Value *vbase, WORD args)
{	WORD i;
	struct String **list;

	if ( !((1<<istate.InstallAction) &
		(ACTIONF_ASKCHOICE | ACTIONF_ASKBOOL | ACTIONF_FUNC | ACTIONF_FILES)) )
	{
		popall(vbase);
		return;
	}

	/* get the list of choice strings... */

	list = AllocMem(args*sizeof (LONG),MEMF_CLEAR);
	if (!list) { memerr(); return; }

	for (i=0; i<args; i++)
	{	struct Value *v = vbase + i;
		if (v->type == TYPE_INTEGER) integer_to_string(v);
		if (v->type == TYPE_STRING)
		{	
			if (v->storage != STORE_TEMP)		/* if not temporary			*/
				dupstring(v);					/* then duplicate			*/
			list[i] = v->v;						/* and consume				*/
		}
		else
		{	err_msg(ehead,ERR_SCRIPT,FUNC_CHOICES,"Incompatible type");
			return;
		}
	}

	istate.choices = list;
	istate.numchoices = args;
	vtop = vbase;								/* all strings captured, no dealloc */
}

void do_defaulttool(struct Value *vbase, WORD args)
{	do_string_param(
		vbase, args, &istate.deftool_val, "DEFAULT TOOL",ACTIONF_TOOLTYPE );
}

void do_pattern(struct Value *vbase, WORD args)
{	do_string_param(
		vbase, args, &istate.pattern_val, FUNC_PATTERN,ACTIONF_FILES );
}

void do_source(struct Value *vbase, WORD args)
{	do_string_param(
		vbase, args, &istate.source_val, FUNC_SOURCE,
			ACTIONF_FILES | ACTIONF_LIBS | ACTIONF_RENAME );
}

void do_dest(struct Value *vbase, WORD args)
{	do_string_param(
		vbase, args, &istate.dest_val, FUNC_DEST,
			ACTIONF_FILES  | ACTIONF_TOOLTYPE | ACTIONF_LIBS   |
			ACTIONF_NEWDIR | ACTIONF_TEXTFILE | ACTIONF_DELETE |
			ACTIONF_RENAME | ACTIONF_ASKDISK );
}

void do_newname(struct Value *vbase, WORD args)
{
	do_string_param(
		vbase, args, &istate.name_val, FUNC_NEWNAME,
		ACTIONF_FILES | ACTIONF_LIBS | ACTIONF_ASKDISK );
}

void do_filesonly(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) & ACTIONF_FILES)
			istate.flags |= IFLAG_NODIRS;

	popall(vbase);
}

void do_default(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) &
			( ACTIONF_ASKSTRING | ACTIONF_ASKFILE | ACTIONF_ASKDIR) )
	{	do_string_param(vbase,args,&istate.default_val,FUNC_DEFAULT,-1);
	}
	else if ((1L<<istate.InstallAction) &
			( ACTIONF_ASKNUM | ACTIONF_ASKBOOL | ACTIONF_ASKCHOICE) )
	{
		if (argcheck(args,1,1,FUNC_DEFAULT)) return;	/* check argument count			*/
		if (vbase->type == TYPE_INTEGER)
		{	istate.default_val = *vbase;
		}
		popall(vbase);
	}
	else
	{	popall(vbase);
	}
}

	/* parameters that just set flags */

void do_all(struct Value *vbase, WORD args)
{
	if (istate.InstallAction == ACTION_FILES)
		istate.flags |= IFLAG_ALL;
	popall(vbase);
}

void do_infos(struct Value *vbase, WORD args)
{
	if (
		(1L<<istate.InstallAction) &
			( ACTIONF_FILES  | ACTIONF_NEWDIR   | ACTIONF_LIBS |
			  ACTIONF_DELETE) )
			istate.flags |= IFLAG_INFOS;
	popall(vbase);
}

void do_fonts(struct Value *vbase, WORD args)
{
	if (istate.InstallAction == ACTION_FILES)
		istate.flags |= IFLAG_FONTS;
	popall(vbase);
}

void do_confirm(struct Value *vbase, WORD args)
{
	WORD confirm_level =
		(args == 0 ? USER_EXPERT :
			(!stricmp(STR_VAL(vbase->v),"expert") ? USER_EXPERT : USER_AVERAGE));

	if (user_level != USER_NOVICE &&
		confirm_level <= user_level &&
		((1L<<istate.InstallAction) &
			( ACTIONF_FILES  | ACTIONF_LIBS     | ACTIONF_OTHER    |
			  ACTIONF_NEWDIR | ACTIONF_TOOLTYPE | ACTIONF_TEXTFILE |
			  ACTIONF_RENAME | ACTIONF_DELETE) ) )
			istate.flags |= IFLAG_CONFIRM;

	popall(vbase);
}

void do_safe(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) &
			( ACTIONF_FILES  | ACTIONF_LIBS     | ACTIONF_OTHER    |
			  ACTIONF_NEWDIR | ACTIONF_TOOLTYPE | ACTIONF_TEXTFILE |
			  ACTIONF_RENAME | ACTIONF_DELETE   | ACTIONF_ASKDIR ) )
			istate.flags |= IFLAG_SAFE;
	popall(vbase);
}

void do_nogauge(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) & (ACTIONF_FILES | ACTIONF_LIBS))
		istate.bits |= IBITS_NOGAUGE;
	popall(vbase);
}

char *delopt_names[] = {
	"FAIL",
	"NOFAIL",
	"OKNODELETE",
	"FORCE",
	"ASKUSER"
};

UBYTE build_delopts(struct Value *vbase, WORD args)
{	WORD	i,j;
	UBYTE	opts = 0;

	for (i=1;i<=args;i++,vbase++) if (vbase->type == TYPE_STRING)
	{
		for (j=0;j<5;j++) if (!stricmp(STR_VAL(vbase->v),delopt_names[j]))
		{
			opts |= 1 << j;
		}
	}

	if ( !(opts & (DELOPT_FAIL | DELOPT_NOFAIL | DELOPT_OKNODELETE)) )
	{
		opts |= DELOPT_FAIL;
	}

	return opts;
}

void do_optional(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) &
		(ACTIONF_ASKDIR | ACTIONF_TOOLTYPE | ACTIONF_ASKDISK))
	{
		istate.flags |= IFLAG_OPTIONAL;
	}
	else if ((1L<<istate.InstallAction) &
		(ACTIONF_FILES | ACTIONF_LIBS | ACTIONF_DELETE))
	{
		istate.flags |= IFLAG_OPTIONAL;

		if (args > 0) istate.delopts = build_delopts(vbase,args);
		else istate.delopts = DELOPT_NOFAIL;
	}

	popall(vbase);
}

void do_delopts(struct Value *vbase, WORD args)
{
	if (args > 0) istate.delopts = global_delopts = build_delopts(vbase,args);

	popall(vbase);
}

void do_resident(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) & (ACTIONF_LIBS | ACTIONF_OTHER))
		istate.bits |= IBITS_RESIDENT;

	popall(vbase);
}

	/* special parameters */

void do_range(struct Value *vbase, WORD args)
{		/* check to make sure not already used... ?? */

	if (istate.InstallAction == ACTION_ASKNUM)
	{	if (argcheck(args,2,2,FUNC_RANGE)) return;	/* check argument count			*/
		if (vbase[0].type != TYPE_INTEGER ||
			vbase[1].type != TYPE_INTEGER)
				err(ehead,ERR_SCRIPT,"Integer argument expected");
		else
		{		/* REM - should also sort, but later... */
			istate.minval = (long)(vbase[0].v);
			istate.maxval = (long)(vbase[1].v);
		}
	}

	popall(vbase);
}

	/* special parameters */

void do_setstack(struct Value *vbase, WORD args)
{		/* check to make sure not already used... ?? */

	if (istate.InstallAction == ACTION_TOOLTYPE)
	{	if (argcheck(args,1,1,FUNC_SETSTACK)) return;	/* check argument count			*/
		if (vbase[0].type != TYPE_INTEGER)
			err(ehead,ERR_SCRIPT,"Integer argument expected");
		else
		{		/* overloaded meaning... */
			/* set a flag indicating stack size was set... */
			istate.maxval = (long)(vbase[0].v);
		}
	}

	popall(vbase);
}

void do_noposition(struct Value *vbase, WORD args)
{
	if ((1L<<istate.InstallAction) &
		( ACTIONF_TOOLTYPE | ACTIONF_FILES | ACTIONF_LIBS ))
			istate.bits |= IBITS_NOPOSITION;
	popall(vbase);
}

void do_settooltype(struct Value *vbase, WORD args)
{	struct ToolType		*tt;

	if (argcheck(args,1,2,FUNC_SETTOOLTYPE)) return; /* check argument count			*/

	if (tt = AllocMem(sizeof *tt,MEMF_CLEAR))
	{	struct String *s;

		if (args == 2)
		{	if (!verify_string(vbase+1,FUNC_SETTOOLTYPE)) return;
			s = (struct String *)vbase[1].v;
			tt->toolval = TempString((char *)(s+1),s->length,NULL);
		}
		if (!verify_string(vbase,FUNC_SETTOOLTYPE)) return;
		s = (struct String *)vbase[0].v;
		tt->tooltype = TempString((char *)(s+1),s->length,NULL);

		tt->next = istate.tooltypes;
		istate.tooltypes = tt;
	}
	else memerr();

	popall(vbase);
}

void do_disk(struct Value *vbase, WORD args)
{
	if ((1L <<istate.InstallAction) & (ACTION_RENAME | ACTION_ASKDIR))
		istate.flags |= IFLAG_OPTIONAL;			/* overload OPTIONAL */
	popall(vbase);
}

	/* appends new text to the text_list */

	/* modification was made to support the "command" directive. */
	/* the only difference is that the "command" appends a newline at the end
		of the text as well.
	*/

struct TextList *new_text(char *text, LONG length, WORD flag)
{	struct TextList		*tl;
	char				*str;

	if (flag) length++;

	if (tl = AllocMem(sizeof(*tl)+length+1,MEMF_CLEAR))
	{	tl->size = length;
		str = (char *)(tl+1);
		if (text) CopyMem(text,tl+1,length);
		if (flag) str[length-1] = '\n';
		AddTail((void *)&istate.text_list,(void *)&tl->node);
	}
	else memerr();

	return tl;
}

void free_text(struct TextList *tl)
{
	FreeMem(tl,sizeof *tl + tl->size + 1);
}

	/* serves double-duty for both "append" and "command" */

void do_append(struct Value *vbase, WORD args)
{
	if (istate.InstallAction != ACTION_TEXTFILE &&
		istate.InstallAction != ACTION_STARTEDIT)
	{
		popall(vbase);
		return;
	}

	if (args)
	{
		do_cat_sub(vbase,args,"",FUNC_APPEND);		/* concatenate all strings (??) */

		if (!ierror)
		{	struct String *s = vbase->v;

			new_text((char *)(s+1),s->length,FALSE);

		}
	}

	popall(vbase);
}

void do_include(struct Value *vbase, WORD args)
{	WORD				i;
	struct Value		*v;

	if (istate.InstallAction != ACTION_TEXTFILE)
	{	popall(vbase);
		return;
	}

	for (i=0; i<args; i++)
	{	v = vbase + i;
		if (v->type == TYPE_STRING)
		{	ULONG 	size = GetFileSize(STR_VAL(v->v));
			BPTR	fh;

			if (fh = Open(STR_VAL(v->v),MODE_OLDFILE))
			{	struct TextList		*tl;

				if (tl = new_text(NULL,size,FALSE))
				{
					if (Read(fh,(char *)(tl+1),size) != size) 
						err(ehead,ERR_DOS,err_reading_text);
				}

				Close(fh);
			}
			else err(ehead,ERR_DOS,err_reading_text);
		}
		else err_msg(ehead,ERR_SCRIPT,FUNC_INCLUDE,"Argument #%d Not a String",i+1);

		vfree(v);
	}
	vtop = vbase;
}
