/* ========================================================================= *
 * Assign.c - maintains the user-startup file                                *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#ifndef EXEC_TYPES_H
/* #define __NO_PRAGMAS */
#include "functions.h"
#endif

#include <string.h>

#include "xfunctions.h"
#include "macros.h"

#include "installer.h"
#include "text.h"

/* an application head always goes like this:

	;BEGIN Music-X
		<lines>
	;END Music-X

*/

/* this function merges the text lines declared in the "startup" statement with the
	text lines in the user-startup file. The text lines for that particular
	application will be completely replaced by the new text lines, but the text lines
	for other applications should not be affected.

	Possible problems: If the user has edited the app_startup file badly,
		it could concievably cause loss of information.

	Header for user-startup:

;		NOTE: This file is created by the install utility. If you are going
;	to edit this file by hand, please be aware of the following:
;		The lines ";BEGIN" and ";END" delineate lines used by a particular applcation.
;	The name of the application is also listed on these lines. When an
;	application is installed, all the text between these two lines may
;	end	up being replaced by new commands. Please leave them alone unless you
;	know what you are doing.

*/

int str_mchi(char *,char *);

extern char	*err_reading_msg;

extern struct InstallationRecord istate;
extern struct TreeNode	*ehead;
extern UWORD	pretend_flag;
extern char		format_string[];

char app_startup_name[] = "s:user-startup";

void edit_startup(char *appname)
{	BPTR				fh;						/* filehandle for app_startup	*/
	struct MinList		temp_list;				/* temporary list for new lines	*/
	WORD				had_error = 0,			/* flag indicating write failed */
						capture = 0;			/* flag indicating capture mode	*/
	struct TextList		*tl, *tlast;			/* used in line processing		*/
	BOOL				any_commands;			/* are there any commands?		*/
	LONG				filesize,
						readsize;
	char				*filestart;
	char				*linebuf;
	char				*filenext;
	WORD				flength;


		/* transfer the new text to a temporary list */

	TransferList((struct List *)&istate.text_list,(struct List *)&temp_list);
	NewList((void *)&istate.text_list);			/* init the text list			*/

	any_commands = (temp_list.mlh_Head->mln_Succ != NULL);

	filesize = GetFileSize(app_startup_name);
	if (filesize <= 0) goto no_file;

	filestart = AllocMem(filesize + 1,MEMF_CLEAR);
	if (filestart == NULL)
	{
		memerr();
		had_error = 1;
		goto no_file;
	}

	if (fh = Open(app_startup_name,MODE_OLDFILE))
	{
		readsize = Read(fh,filestart,filesize);
		Close(fh);
		if (readsize != filesize) goto no_read;
	}
	else
	{
no_read:
		err_msg(ehead,ERR_DATA,FUNC_STARTUP,err_reading_msg,app_startup_name);

		FreeMem(filestart,filesize+1);
		had_error = 1;
		goto no_file;	
	}

	for (linebuf=filestart;*linebuf;linebuf=filenext)
	{
											/* parse a line in buffer	*/
		if (filenext = strchr(linebuf,'\n'))
		{
			flength = filenext - linebuf;
			*filenext++ = 0;
		}
		else
		{
			flength = strlen(linebuf);
			filenext = linebuf + flength;
		}

		if (str_mchi(";BEGIN ",linebuf))		/* look for start of app		*/
		{
			if (capture == 0 && str_mchi(appname,&linebuf[7]) )
			{
				if (any_commands)
				{	new_text(linebuf,flength,TRUE);	/* capture this line	*/

						/* now, append all the command lines to the list */

					tlast = NULL;
					while (tl = (struct TextList *)RemHead((void *)&temp_list))
					{	AddTail((void *)&istate.text_list,(void *)tl);
						tlast = tl;
					}

					if (tlast != NULL &&
						*((char *)(tlast+1) + tlast->size - 1) != '\n')
					{
						new_text("\n",1,FALSE);
					}
				}

				capture = 1;				/* ignore further input lines	*/
				continue;					/* go read next line			*/
			}								/* if another application found	*/
			else if (capture == 1)			/* and not capturing it			*/
				capture = 2;				/* the start capturing again	*/
		}

		if (capture == 1)					/* if capture disabled			*/
		{	if (str_mchi(";END ",linebuf))	/* and we reach end of app		*/
			{
				if (any_commands)
					new_text(linebuf,flength,TRUE);	/* capture this line	*/
				capture = 2;				/* then re-enable				*/
			}
		}
		else new_text(linebuf,flength,TRUE);	/* else just save the line	*/
	}

	FreeMem(filestart,filesize+1);

no_file:
	fh = NULL;

		/* if app_startup never got opened, or if the application's name never
			appeared in app_startup, then append the list of commands to it.
		*/

		/* add to the list anyway... */

	if (capture == 0)						/* if lines never got added		*/
	{	new_text(";BEGIN ",7,FALSE);
		new_text(appname,strlen(appname),TRUE);
		tlast = NULL;
		while (tl = (struct TextList *)RemHead((void *)&temp_list))
		{	AddTail((void *)&istate.text_list,(void *)tl);
			tlast = tl;
		}
		if (tlast != NULL && *((char *)(tlast+1) + tlast->size - 1) != '\n')
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

	if (had_error) goto dealloc;

		/* first, write to the transcript file... */

	transcript(GetLocalText(TRANS_START_US),app_startup_name);

	if (!pretend_flag)
	{
#ifdef RENAME_IT
			/* rename old "user-startup" to "user-startup.bak" */
		strcpy(format_string,app_startup_name);
		strcat(format_string,".bak");
		DeleteFile(format_string);
		Rename(app_startup_name,format_string);
#endif

		unless (fh = Open(app_startup_name,MODE_NEWFILE))
		{
			err_msg(ehead,ERR_DOS,FUNC_STARTUP,err_reading_msg,app_startup_name);
#ifdef RENAME_IT
			Rename(format_string,app_startup_name);
#endif
			return;
		}
	}

dealloc:
	while (tl=(struct TextList *)RemHead((void *)&istate.text_list))
	{
			/* do various conditions... PRETEND_MODE, etc. */
		if (!pretend_flag && !had_error)
		{
			if (Write(fh,tl+1,tl->size) != tl->size)
			{
				had_error = 1;
				err_msg(ehead,ERR_DOS,FUNC_STARTUP,
					GetLocalText(ERR_WRITING_US),app_startup_name);
#ifdef RENAME_IT
				Rename(app_startup_name,format_string);
#endif
			}
		}

		if (!had_error) transcript((char *)(tl+1));

			/* but always execute this line */
		free_text(tl);
	}

	if (fh) Close(fh);

	transcript(GetLocalText(TRANS_END_US));
}
