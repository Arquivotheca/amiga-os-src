/*
 *		start-seq.c
 *
 *	This is	all	of the routines	involved in	parsing	and	modifying the
 *	startup-sequence and user-startup files.  These	routines existed in
 *	a number of	different modules previously, and have been	collected her
 *	in one place to	make them easier to	understand.
 *	
 *
 * Prototypes for functions defined in start-seq.c
 *
 *	extern unsigned char 	s_user_startup	[15];
 *	extern unsigned char 	best_path		[128];
 *	extern int 				best_position;
 *
 *	void 					do_startup		(struct Value * , WORD );
 *	int 					scan_scripts	(unsigned char * , int );
 *	BOOL 					change_startup	(unsigned char ** , int * );
 *	int 					do_textlist		(struct InstallationRecord * , unsigned char * );
 *	LONG 					final_text		(struct Gadget * , struct ListControl * );
 *	void 					edit_startup	(unsigned char * );
 *	
 *
 *	Revision History:
 *
 *	talin 1990:
 *		Original coding	on most	of this.
 *
 *	lwilton	07/30/93:
 *		Created	from parts of a	number of other	modules	and	major
 *		simplifications	and	cleanup.
 */


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "functions.h"
#include "xfunctions.h"
#include "macros.h"

#include "window.h"
#include "installer.h"


extern struct InstallationRecord 
						istate;
extern UWORD			pretend_flag,
						user_level;
extern char				format_string[];

char s_user_startup[] =	"s:user-startup";

extern struct Window	*window;
extern char				*textbuf;
extern int				textloc;
extern struct TreeNode	*ehead;
extern char				*prob_with_file,
						*help_change_startseq,
						*err_reading_msg,
						*trans_insert_commands,
						*prob_ss_msg;

	/* the results of the scan_startup operation will be: */

char					best_path[128];			/* name	of the file	to use	*/
int						best_position;			/* offset into that	file	*/

int	scan_scripts (char *start_file, int nest);
void edit_startup(char *appname);


/*--------------------------------------------------------------------------*/

/*	This procedure is the implementation of	the	(startup) command.	*/

void do_startup	(struct	Value *vbase, WORD args)
{
	BPTR				fh = NULL;
	long				size;
	char				*thePath;
	char				*buffer	= NULL;
	BPTR				olddir,
						boot;
	int					thePos,
						result;

	if (user_level != USER_NOVICE)		/* always confirm if not novice	*/
		istate.flags |=	IFLAG_CONFIRM;

	debug_action("Editing user_startup file");

	if (help_missing(FUNC_STARTUP,0)) 
		return;

	/* ====BUG==== Improve this	error message so it	says what is wrong!	*/
	
	if (!verify_string(vbase,FUNC_STARTUP))	
		return;									/* check argument type		*/

	/* check to	see	if command to execute "S:user-startup" exists */

	/* ====BUG==== There should	probably be	an error message here! */
	
	if ( (boot = boot_volume())	== NULL	)		/* look	for	boot disk		*/
		goto bye;								/* no disk,	so no startup	*/


	/*	Look for startup-sequence and all of the called	scripts	to see if
	 *	there is already a call	to s:user-startup.
	 *
	 *	scan_scripts will return the following values:
	 *
	 *		-1	S:user-startup found in	some script.
	 *		 0	Horrid screwup somewhere.
	 *		+n	The	"priority" of the best place to	put	a call.
	 *
	 *	It also	returns	best_path and best_pos as the name of the script
	 *	to put the call	to user-startup	in,	and	the	byte offset	to put it.
	 */
	 
	olddir = CurrentDir(boot);

	result = scan_scripts("s/Startup-Sequence", TRUE);

	CurrentDir(olddir);
	UnLock(boot);

	if (result > 0)							/* not here, found a good place	*/
	{
		/*	There is no	call to	execute	the	user-startup sequence, or at least
		 *	none that we can find.	We have	been left with a "good"	place in
		 *	the	sequence of	scripts	to put a call.	Ask	the	user if	he wants
		 *	this change	made (unless the user is a novice) and then	make the
		 *	change,	if permitted.
		 */
		 
		thePath	= best_path;				/* put 'er here, mate!          */
		thePos	= best_position;

		if (istate.flags & IFLAG_CONFIRM)	/* want	to confirm?				*/
		{
			/*	User can confirm editing on this file, or select another file. */
			
			if (!change_startup(&thePath,&thePos))	/* check for "OK"		*/
				goto edit_user_startup;		/* but do edit user-startup		*/
		}

		/*	The	user will let us add the call to user-startup.	*/
		
		transcript(trans_insert_commands,s_user_startup,thePath,thePos);

		if (pretend_flag)					/* in pretend mode we do nothing! */
			goto bye;

		/*	Read the entire	startup	file into a	buffer in one read.	 Then
		 *	open a new version of the file,	write back everything we read
		 *	up to the "good" spot, insert a	call to	the	user-startup file,
		 *	and	if there is	any	of the original	file left after	the	"good"
		 *	spot, write	that out to	the	new	file.  Then	close the file we
		 *	just created.
		 */
		 
		if ((size =	GetFileSize(thePath))			/* open	the	startup	file */
		&&	(fh	= Open(thePath,MODE_OLDFILE)))
		{
			buffer = MemAlloc(size,0L);

			if (buffer == NULL)
			{
				memerr();
				goto bye;
			}

			if (Read(fh,buffer,size) !=	size)		/* read	it up */
			{
				err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
				goto bye;
			}

			Close(fh);								/* close it	*/

			if (fh = Open(thePath,MODE_NEWFILE))	/* create a	new	version	*/
			{
				if (Write(fh,buffer,thePos)	!= thePos)
				{
					err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
					goto bye;
				}

				Fprintf((void *)fh,"if exists %s\n  execute %s\nendif\n",
					s_user_startup,	s_user_startup);

				if (size - thePos >	0)
					Write(fh,&buffer[thePos],size -	thePos);

				Close(fh);							/* close the new one */
				fh = NULL;
				MemFree(buffer);
				buffer = NULL;
			}
			else									/* couldn't make a new one */
			{
				err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
				goto bye;
			}
		}
		else			/* unable to access	startup-sequence file */
		{
			err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_ss_msg);
			goto bye;
		}
	}
	else 
	if (result == 0)							/* something terrible happened!	*/
	{
		/*	They have a	startup-sequence, but it must be weird.	What we	want
			to do is give the user a message saying	their startup-sequence
			is unusual and they	will have to manually add the command to
			execute	user-startup if	it is not already doing	so.	What should
			be done	in the case	of novices?	I don't know.

			For	now, just assume the startup-sequence is OK	and	continue.
		*/

#if	0	/* !!! old,	bad	way	*/
		err_msg(ehead,ERR_DATA,FUNC_STARTUP,prob_ss_msg);
		goto bye;
#endif
	}


	/*	If result == -1, then already have "S:user-startup"	in startup script.
	 *	Alternately, we	may	have just put the call there.  In any case,	it
	 *	is now time	to install our application into	user-startup.
	 */

	/*	The	installer controls "s:user-startup", so	we ask the user
		to confirm editing the file	ONLY if	they are experts. Pretend
		mode is	handled	inside edit_startup.									*/

edit_user_startup:
	if (user_level != USER_EXPERT || confirm_page(&istate))
		edit_startup(STR_VAL(vbase->v));

bye:
	if (buffer)	
		MemFree(buffer);
	if (fh)	
		Close(fh);
	popall(vbase);								/* pop all parameters			*/
	eval(0L);									/* always return zero? */
}



/*	This procedure scans the s:startup-sequence	and	any	related	files to
 *	determine the best place to	put	a call to s:user-startup.
 *
 *	We return the full path	to the best	file to	use	for	the	link, and the
 *	byte offset	into the file of the best place	to put the link.  We also
 *	return a value indicating the results of our search:
 *
 *		-1	We found "s:user-startup" at the supplied location.
 *		 0	Some hideous error occurred!
 *		+n	The	relative "goodness"	of the place to	put	the	call.
 *
 *	The	"goodness" is based	on an empirical	and	rather arcane calculation
 *	that tries to group	the	call with the file that	has	the	most paths,	
 *	assigns, and environment variables.
 *
 *	Note:
 *		Write-protected	files are not acceptable.
 */


#define	MATCHC(str)	(!stricmp(str,command))
#define	LINESIZE	512L

struct ScriptInfo
{
	char				line [LINESIZE];
	struct ScriptFile
	{
		BPTR			fh;						/* the file	handle				*/
		char			path[128];				/* the full	file name			*/
		int				position;				/* the current offset to EOL	*/
		int				pri;					/* the 'goodness' amount		*/
	} file [10];								/* array of	file infos			*/
};


int	scan_scripts (char *start_file, int nest)
{
	/*	The	*start_file	is the name	of the file	to start scanning from.
	 *	This will of course	always be s:startup-sequence.
	 */
	 
	struct ScriptInfo	*info;					/* start of	useful info			*/
	struct ScriptFile	*stack;					/* start of	file info array		*/
	int					level =	-1;				/* current level in	the	stack	*/
	
	char				*script_line;			/* ptr to the current line		*/
	int					actual;					/* byte	count in current line	*/

	char				*scan,					/* curr	line scan pointer		*/
						*command;				/* same	as argv[0]				*/
	char				*scriptname,			/* fileonly	of current filename	*/
						*next_file = start_file;/* current file	name			*/
						
	int					argc;					/* words on	the	current	line	*/
	char				*argv[32];				/* ptrs	to the words			*/
	
	int					if_nest	= 0;			/* 'if'	nesting	level			*/
	int					i,						/* misc	index					*/
						need_open =	1,			/* must	open the file			*/
						pri;					/* current 'goodness' value		*/
	int					best_value=0;			/* current 'goodness' rating	*/

	BPTR				flock;					/* filelock	temp				*/


	/*	Allocate some working space.  */
	
	if ((info =	AllocMem (sizeof *info,	MEMF_CLEAR)) ==	0)
		return 0;								/* allocation failed			*/

	stack =	&info->file[0];						/* point to	important parts		*/
	script_line	= &info->line[0];

	/*	Now	read the tree of startup files and look	for	a hook to user-startup.	*/
	
	while (1)
	{
		if (need_open)
		{
			need_open =	0;

			if (level >	9)
				continue;
			if (!(flock	= Lock(next_file,ACCESS_READ)))	/* lock	the	file...	*/
				continue;
			ExpandPath ((void*)flock, stack[level+1].path,	128); /* expand	*/
			UnLock(flock);							/* unlock				*/

			/* get filename	of script path */

			scriptname = FileOnly (stack[level+1].path);

			/* don't scan these special names... */

			if (!stricmp(scriptname,"shell-startup"))	continue;
			if (!stricmp(scriptname,"cli-startup"))		continue;
			if (!stricmp(scriptname,"startup-wshell"))	continue;

			/* now,	bump the level and open	the	file */

			level++;
			if (stack[level].fh	= Open(stack[level].path,MODE_OLDFILE))	/* open	the	file */
			{
				stack[level].position =	0;		/* at file start			*/
				stack[level].pri = level;		/* deeper is better!		*/
			}
			else 
				level--;						/* failed, forget the file	*/
		}

		if (level >= 0)							/* we have a file open?		*/
		{
			actual = Fgets((void*)stack[level].fh,	script_line, LINESIZE -	1);

			if (actual < 0)						/* end of the file?			*/
			{
endfile:
				Close(stack[level].fh);			/* finished	with the file	*/
				stack[level].fh	= NULL;			/* don't close it again     */
				level--;						/* back	down a level		*/
			}
		}
		if (level <	0)							/* finished	starting file?	*/
		{
			strcpy (best_path, next_file);
			best_position = 0;
			FreeMem(info, sizeof *info);		/* return resources			*/
			return best_value;					/* return results, too		*/
		}

		stack[level].position += actual	+ 1;	/* remember	EOL	location	*/

		/* now,	scan the line... */

		if (script_line[0] == '.') 
			continue;							/* dotted line...			*/

		scan = script_line;						/* init	scan pointer		*/
		script_line[actual]	= '\0';				/* use NULL	as sentinel		*/

		/*	Scan this line apart in	up to 32 tokens.  This is in general rather
		 *	a waste	of time, since we could	do the scanning	in only	the	very few
		 *	cases where	we really need to do it, not on	every line!
		 */
		 
		argc = 0;								/* set arg count to	zero	*/
		while (argc	< 32)						/* loop	through	arguments	*/
		{
			for	( ;	isspace	(*scan); *scan++ = 0);	/* skip	whitespace				*/

			if (*scan == '\0' || *scan == ';')	/* comment or sentinel ends	line */
			{
				*scan =	0;						/* just	to be safe!			*/
				break;
			}

			if (*scan == '"')                   /* a quoted argument        */
			{
				char *put;

				/*	Reformat the string	in place, replacing	*E and *N with escape
				 *	and	newline.  Stop at the terminal quote.  Do not handle
				 *	the	possibility	of doubled quotes.
				 */
				 
				argv [argc++] = ++scan;			/* start of	new	argument	*/
				put	= scan;						/* need	to reformat...		*/

				for	(;;)
				{
					if (*scan == '"' || !*scan) /* terminal quote or null?  */
						break;					/* end of quoted string		*/
					if (*scan == '*')			/* BCPL	escape string		*/
					{
						scan++;
						if (*scan == 'E' ||	*scan == 'e') 
							*put++ = 0x1b;
						else 
						if (*scan == 'N' ||	*scan == 'n') 
							*put++ = '\n';
						else 
							*put++ = *scan;
						scan++;
					}
					else						/* else	just copy characters */
						*put++ = *scan++;
				}
				*put = '\0';					/* terminate the string		*/
				scan++;							/* skip final quote 		*/
			}
			else								/* a non-quoted	argument	*/
			{
				if (*scan != '>' &&	*scan != '<')	/* redirection?					*/
					argv[argc++] = scan;		/* no, remember	arg	start	*/

				while (!(*scan == ' '  ||		/* scan	an argument			*/
						 *scan == ';'  ||
						 *scan == '\n' ||
						 *scan == '\0' ||
						 *scan == '\t' ||
						 *scan == '\r')) scan++;
			}
		}

		if (argc ==	0) 
			continue;							/* nothing on the line 		*/

		/*	We will	make a check for "s:user-startup" somewhere on this line. */
		
		for (i=0; i<argc; i++)
		{ 
            if (strstr(strlwr(argv[i]),
            		   s_user_startup))			/* user-startup anywhere?   */
            {                                   /* yes, we are finished!    */
                for (i=0;i<10;i++)              /* close all open files     */
                    if (stack[i].fh) 
                        Close(stack[i].fh);
                FreeMem(info, sizeof *info);    /* return resources         */
                return -1;                      /* that hit the spot!       */
            }
        }

		command	= argv[0] =	FileOnly (argv[0]);	/* get the command word		*/

		/*	do if-level	testing	first... 
		 *	Note that we IGNORE	anything inside	an IF /	ENDIF !
		 */

		if (MATCHC("IF"))						/* if an IF	statement...	*/
		{
			if_nest++;							/* add 1 to	nesting	level	*/
			continue;
		}
		else 
		if (MATCHC("ENDIF")	&& if_nest)			/* if an ENDIF statement	*/
		{
			if_nest--;							/* remove 1	nesting	level	*/
			continue;
		}

		if (if_nest) 
			continue;

		/*  Now, assign priorities... 
		 *	This determines	the	"best" place to	put	the call to the user-startup
		 *	file.  The deeper the files nest the "better" the place.
		 *
		 *	Note that we only check for a new script name following an
		 *	Execute or NewShell type command.  So if the user has tagged a
		 *	script with the Script bit, and is simply executing it with its
		 *	name as the first thing on the line, we will not find the script!
		 */

		pri	= -1;

		if (MATCHC("assign"))
			pri	= 5	* (++stack[level].pri);
		else 
		if (MATCHC("path") || MATCHC("setenv") || MATCHC("set"))
			pri	= 3	* (++stack[level].pri);
		else 
		if (MATCHC("copy"))
			pri	= 3;
		else 
		if (MATCHC("quit") || MATCHC("endcli") || MATCHC("kickit"))
			goto endfile;
		else 
		if (MATCHC("execute"))
		{
			next_file =	argv[1];
			need_open = 1;
		}
		else 
		if (MATCHC("newcli") 
		||	MATCHC("newshell") 
		||	MATCHC("ashell") 
		||	MATCHC("NewWSH"))
		{
			for	(i=1; i<argc; i++)
			{
				if (!stricmp(argv[i],"from") && (i+1 < argc))
				{
					next_file =	argv[i+1];
					need_open = 1;
				}
			}
		}

		if (pri	> best_value)
		{
			/*	Remember what we found,	and	where we found it.	*/
			
			best_value = pri;
			strcpy (best_path, stack[level].path);
			best_position =	stack[level].position;
		}
	}
}




/* Change Startup Page (part of	startup):
 *
 *	This procedure is (only!) called when we need to add a call to the
 *	S:user-startup file to the S:startup-sequence file!  At that time
 *	it will display the contents of the file we have selected to modify,
 *	but *without* the modifications we intend to make!
 *
 *	In general, the user does NOT have a chance to see the modifications
 *	we will make, but only sees a page indicating that something will
 *	be modified.
 

	|---------------------------|
	|  Modify Startup-Sequence	|
	|							|
	| |---------------------|^|	|
	| |						|#|	|
	| |		Text of	File	|#|	|
	| |						| |	|
	| |---------------------|.|	|
	|							|
	|	 File being	modified:	|
	|  [_______drawer________]	|
	|							|
	| Proceed			  Skip	|
	| Select File		  Abort	|
	|			Help			|
	|---------------------------|

*/

BOOL change_startup(char **file,int	*offset)
{
	static struct GadgetDef	start_def[]	= 
	{
		{ (void	*)TX_MODIFY_SS,
						GTYPE_WRAP|GPUT_UPPER|GTEXT_CENTER|GFLAG_LOCAL,	
						0 },
		{ (void	*)TX_HELP, 
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_HALF|GFLAG_LOCAL,
						HELP_ID	},
		{ (void	*)TX_ANOTHER_FILE,
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	
						1 },
		{ (void	*)TX_ABORT,	
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	
						ABORT_ID },
		{ (void	*)TX_PROCEED_CHANGES,
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	
						PROCEED_ID },
		{ (void	*)TX_SKIP,	
						GTYPE_BUTTON|GPUT_LOWER|GWIDTH_2|GFLAG_LOCAL,	
						2 },
		{ NULL,			GTYPE_STRGAD | GPUT_LOWER |	GSTRING_ACTIVE,	
						DRAWER_ID },
		{ (void	*)TX_FILE_MODIFIED,
						GTYPE_TEXT|GPUT_LOWER|GTEXT_CENTER|GTEXT_BOLD|GFLAG_LOCAL,
						 },
		{ NULL,			GTYPE_ARROW,
						0 },
		{ NULL,			GTYPE_SLIDER,							
						0 },
		{ NULL,			GTYPE_LIST | GFLAG_LAST,				
						0 }
	};
	struct InstallationRecord ir;
	BPTR			texthandle;
	struct Gadget	*agad,
					*gad = NULL;
	char			batchname[120],
					*str;
	BOOL			ok = FALSE;
	int				r,
					size;

restart:
	clear_busy();									/* clear busy pointer		*/

	size = GetFileSize(*file);
	if ( size == 0 
	|| !(texthandle = Open(*file,MODE_OLDFILE)) )	/* open the file to modify	*/
	{
no_open:
		err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_with_file, *file);
		return FALSE;
	}

	/*	NOTE that textbuf and textloc are used as implicit parameters and
	 *	function results all over the place!
	 */
	 
	textbuf	= MemAlloc(size+3,0L);					/* get buffer for file data */
	if (textbuf	== NULL)
	{
		memerr();
		Close(texthandle);
		return FALSE;
	}

	r =	Read(texthandle,textbuf,size);				/* read the file in			*/
	Close(texthandle);

	if (r != size)
	{	
		err_msg(ehead,ERR_DOS,FUNC_STARTUP,prob_with_file, *file);
		return FALSE;
	}

	str	= textbuf +	size - 1;						/* point to last char of file*/
	str[1] = '\n';									/* append a newline			*/
	str[2] = '\0';									/* new string end			*/
	if (*str !=	'\n')								/* have a newline now?		*/
	{
		str[2] = '\n';								/* no, need two newlines	*/
		str[3] = '\0';
	}

	/* ====BUG==== This code makes no sense here!	*/
	
	unless (textloc	= *offset)						/* line somewhere in file?	*/
	{
		if (str	= strchr(textbuf,'\n'))				/* find first newline in file */
			textloc	= str -	textbuf	+ 1;			/* end of first line of file  */
	}

	strcpy(batchname,*file);
	ir.help_val.v =	(struct	String *)GetLocalText(HELP_CHANGE_STARTUP);
	ir.help_val.type = TYPE_STRING;

	start_def[6].Text =	batchname;
	make_gadgets(&gad,start_def,&ir,GERASE_PAGE|GSTART_LAYOUT|GSET_GADGETS);

	while (TRUE)
	{
		r =	do_textlist(&ir,help_change_startseq);	/* get gadget type	*/

		if (r == 2)	
			break;									/* skip	this part 	*/
		if (r == ABORT_ID && !aborterr(FUNC_STARTUP)) 
			continue;
		if (r == 1)									/* select new file 	*/
		{
			clear_gadgets(gad);

			MemFree(textbuf);

			ir.prompt_val.v	= (struct String *)GetLocalText(STR_FILE_TO_MODIFY);
			ir.help_val.v   = (struct String *)GetLocalText(HELP_SELECT_FILE);
			unless (ir.default_val.v = new_string(*file))
			{	
				memerr();							/* string allocate failed */
				goto bye;
			}
			ir.prompt_val.type 
				= ir.help_val.type 
				= ir.default_val.type	
					= TYPE_STRING;

			unless (str	= select_file(&ir))	
				goto bye;

			/*	Make sure that this turkey doesn't ask us to call user-startup
			 *	from inside of user-startup!
			 */
			
			if (!strstr(strlwr(str), &s_user_startup[2]))
			{
				scan_scripts (str, FALSE);			/* look for a good place	*/
				*file =	best_path;					/* new file name			*/
				*offset	= best_position;
			}
			MemFree(ir.default_val.v);				/* free filename temp		*/
			gad	= NULL;
			goto restart;							/* display this file		*/
		}

		break;
	}

	agad = FindGadget(window,DRAWER_ID);

	clear_gadgets(gad);

	if (r == PROCEED_ID)
	{
		*offset	= textloc;
		*file =	(char *)((struct StringInfo	*)agad->SpecialInfo)->Buffer;
		ok = TRUE;
	}

	MemFree(textbuf);

bye:
	set_busy();										/* turn on busy pointer	*/
	return ok;
}


	/* handle the extra	gadgets	associated with	change_startup page	*/

int	do_textlist (struct InstallationRecord *ir, char *title)
{
	char				str[120],
						*dtext;
	struct ListControl	ctrl;
	struct Gadget		*gad;

	gad	= FindGadget (window, DRAWER_ID);
	dtext =	(char *)((struct StringInfo	*)gad->SpecialInfo)->Buffer;
	strcpy(str,dtext);

	zero(&ctrl,sizeof ctrl);
	gad	= FindGadget(window,LIST_ID);
	init_text(gad,&ctrl);
	show_text(gad,&ctrl,0);
	set_slider(FindGadget(window,SLIDER_ID),&ctrl);

	while (TRUE)
	{
		if (gad	= HandleEvents(&ctrl))
		{
			if (gad->GadgetID == PROCEED_ID)
			{
				textloc	= final_text(FindGadget(window,LIST_ID),&ctrl);
				return (int)gad->GadgetID;
			}

			if (gad->GadgetID == ABORT_ID || gad->GadgetID < 10)
				return (int)gad->GadgetID;

			if (gad->GadgetID == HELP_ID &&	title != NULL)
				save_page(ir,title,help_page);
		}
	}
}

	
LONG final_text(struct Gadget *gad, struct ListControl *ctrl)
{
	char	*at = textbuf;
	WORD	i;
	LONG	hi = ctrl->Hilite;

	for (i=0;i<hi;i++) 
		at = strchr(at,'\n') + 1;
	return at - textbuf;
}



/* an application head always goes like	this:

	;BEGIN Music-X
		<lines>
	;END Music-X

*/

/* this	function merges	the	text lines declared	in the "startup" statement with	
	the	text lines in the user-startup file. The text lines	for	that particular
	application	will be	completely replaced	by the new text	lines, but the text	
	lines for other	applications should	not	be affected.

	Possible problems: If the user has edited the app_startup file badly,
		it could conceivably cause loss	of information.

	Header for user-startup:

;		NOTE: This file	is created by the install utility. If you are going
;	to edit	this file by hand, please be aware of the following:
;		The	lines ";BEGIN" and ";END" delineate	lines used by a	particular 
;	applcation.	 The name of the application is	also listed	on these lines.	
;	When an	application	is installed, all the text between these two lines may
;	end	up being replaced by new commands. Please leave	them alone unless you
;	know what you are doing.

*/


void edit_startup (char *appname)
{
	/*	Hunt through the user-startup file looking for our application.
	 *	If we find it, delete the old stuff, and remember where we found it.
	 *	Then insert the new stuff in place of the old stuff, or at the
	 *	end of the user-startup file.
	 */
	 
	BPTR				fh;						/* filehandle for app_startup	*/
	struct MinList		temp_list;				/* temporary list for new lines	*/
	WORD				had_error =	0,			/* flag	indicating write failed	*/
						capture	= 0;			/* flag	indicating capture mode	*/
	struct TextList		*tl, *tlast;			/* used	in line	processing		*/
	BOOL				any_commands;			/* are there any commands?		*/
	LONG				filesize,
						readsize,
						namesize = strlen(appname);
	char				*filestart;
	char				*linebuf;
	char				*filenext;
	WORD				flength;


	/* transfer	the	new	text to	a temporary	list */

	TransferList((struct List *)&istate.text_list,(struct List *)&temp_list);
	NewList((void *)&istate.text_list);			/* init	the	text list			*/

	any_commands = (temp_list.mlh_Head->mln_Succ !=	NULL);

	filesize = GetFileSize (s_user_startup);
	if (filesize <=	0) 
		goto no_startup_file;

	filestart =	AllocMem(filesize +	1,MEMF_CLEAR);
	if (filestart == NULL)
	{
		memerr();
		had_error =	1;
		goto no_startup_file;
	}

	if (fh = Open (s_user_startup, MODE_OLDFILE))
	{
		readsize = Read(fh,filestart,filesize);
		Close(fh);
		if (readsize !=	filesize) 
			goto no_read;						/* failed to read startup file */
	}
	else
	{
no_read:
		err_msg(ehead,ERR_DATA,FUNC_STARTUP,err_reading_msg,s_user_startup);

		FreeMem (filestart, filesize+1);
		had_error =	1;
		goto no_startup_file;	
	}

	/*	We read the startup file, now take it apart.  */
	
	for	(linebuf=filestart; *linebuf; linebuf=filenext)
	{											/* parse a line	in buffer	*/
		if (filenext = strchr(linebuf,'\n'))	/* find a newline?			*/
		{
			flength	= filenext - linebuf;		/* remember line length		*/
			*filenext++	= 0;					/* terminate string			*/
		}
		else
		{
			flength	= strlen(linebuf);			/* length of last line		*/
			filenext = linebuf + flength;		/* where next line starts	*/
		}

		if (!strnicmp(";BEGIN ",linebuf, 7))	/* look	for	start of app		*/
		{
			if (capture	== 0 &&	!strnicmp(appname,&linebuf[7],namesize) )
			{
				if (any_commands)
				{
					new_text(linebuf,flength,TRUE);	/* capture this	line	*/

						/* now,	append all the command lines to	the	list */

					tlast =	NULL;
					while (tl =	(struct	TextList *)RemHead((void *)&temp_list))
					{
						AddTail((void *)&istate.text_list,(void	*)tl);
						tlast =	tl;
					}

					if (tlast != NULL &&
						*((char	*)(tlast+1)	+ tlast->size -	1) != '\n')
					{
						new_text("\n",1,FALSE);
					}
				}

				capture	= 1;				/* ignore further input	lines	*/
				continue;					/* go read next	line			*/
			}								/* if another application found	*/
			else 
			if (capture	== 1)				/* and not capturing it			*/
				capture	= 2;				/* then start capturing again	*/
		}

		if (capture	== 1)					/* if capture disabled			*/
		{
			if (!strnicmp(";END ",linebuf,5)) /* and we reach end of app	*/
			{
				if (any_commands)
					new_text(linebuf,flength,TRUE);	/* capture this	line	*/
				capture	= 2;				/* then	re-enable				*/
			}
		}
		else 
			new_text(linebuf,flength,TRUE);	/* else	just save the line	*/
	}

	FreeMem(filestart,filesize+1);

no_startup_file:
	fh = NULL;

    /* if app_startup never got opened, or if the application's name never
        appeared in app_startup, then append the list of commands to it.
    */

    /* add to the list anyway... */

	if (capture	== 0)						/* if lines	never got added		*/
	{
		new_text(";BEGIN ",7,FALSE);
		new_text(appname,strlen(appname),TRUE);
		tlast =	NULL;
		while (tl =	(struct	TextList *)RemHead((void *)&temp_list))
		{
			AddTail((void *)&istate.text_list,(void	*)tl);
			tlast =	tl;
		}
		if (tlast != NULL && *((char *)(tlast+1) + tlast->size - 1)	!= '\n')
		{
			new_text("\n",1,FALSE);
		}
		new_text(";END ",5,FALSE);
		new_text(appname,strlen(appname),TRUE);
	}

    /* now, write out the file.. */

    /* if the file wasn't there, then determine where to write it to,
        otherwise write to the old file...

        Also, write a series of comments at the beginning of the file explaining
        what the installer needs, and tell them if they are going to hand-edit
        this file to be careful of certain things.
    */

	if (had_error) 
		goto dealloc;

		/* first, write	to the transcript file... */

	transcript (GetLocalText (TRANS_START_US), s_user_startup);

	if (!pretend_flag)
	{
		unless (fh = Open(s_user_startup,MODE_NEWFILE))
		{
			err_msg(ehead,ERR_DOS,FUNC_STARTUP,	err_reading_msg, s_user_startup);
			return;
		}
	}

dealloc:
	while (tl=(struct TextList *)RemHead((void *)&istate.text_list))
	{
		/* do various conditions...	PRETEND_MODE, etc. */
		
		if (!pretend_flag && !had_error)
		{
			if ( Write(fh,tl+1,	tl->size) != tl->size)
			{
				had_error =	1;
				err_msg	(ehead,ERR_DOS,FUNC_STARTUP,
					GetLocalText (ERR_WRITING_US), s_user_startup);
			}
		}

		if (!had_error)	
			transcript((char *)(tl+1));

			/* but always execute this line	*/
		free_text(tl);
	}

	if (fh)	
		Close(fh);

	transcript (GetLocalText (TRANS_END_US));
}
