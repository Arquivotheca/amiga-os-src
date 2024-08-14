/* ======================================================================== *
   			    scan.c - Scans startup-sequence and related files
							       By Talin.
				  Compiled under Manx 5.0 with small code/data
 * ======================================================================== */

/*	NOTES
		Write-protected files are not acceptable.

*/

#ifndef EXEC_TYPES_H
#include "functions.h"
#endif

#include <string.h>

#include "macros.h"
#include "xfunctions.h"

#include "installer.h"

extern char app_startup_name[];

	/* the results of this operation will be: */

char					best_path[128];
int						best_position;

	/* the stack of files we are parsing */

struct ScriptFile {
	BPTR				fh;
	char				path[128];
	int					position;
	int					pri;

	/* parsing variables... */

};

#define MATCHC(str) (!stricmp(str,command))
#define LINESIZE	512L

	/* first, open the startup sequence... (Where?) */

int scan_scripts(char *start_file, int nestable)
{	struct ScriptFile	*stack;
	int					level = -1;
	int					actual;
	char				*script_line;
	int					argc;
	char				*argv[32];
	char				*scan, *command;
	int					i, pri;
	char				*scriptname, *next_file;
	int					if_nest = 0;
	BPTR				flock;
	int					best_value=0;

	stack = AllocMem(10 * sizeof *stack,MEMF_CLEAR);
	if (stack == NULL) return 0;

	script_line = AllocMem(LINESIZE,0L);
	if (script_line == NULL)
	{
		FreeMem(stack,10 * sizeof *stack);
		return 0;
	}

	next_file = start_file;

		/* set up to scan this file... */

newfile:										/* push a new file...			*/
	if (level >= 9) goto readline;				/* limit on nesting				*/

	if (flock = Lock(next_file,ACCESS_READ))	/* lock the file...				*/
	{	ExpandPath((void *)flock,stack[level+1].path,128); /* expand */
		UnLock(flock);							/* unlock						*/
	}
	else if (level == -1)
	{
		FreeMem(stack,10 * sizeof *stack);
		FreeMem(script_line,LINESIZE);
		return best_value;	/* if lock failed, return		*/
	}
	else goto readline;							/* or continue reading previous	*/

		/* get filename of script path */

	scriptname = (void *)FileOnly((void *)stack[level+1].path);

		/* don't scan these special names... */

	if (!stricmp(scriptname,"shell-startup")) goto readline;
	if (!stricmp(scriptname,"cli-startup")) goto readline;
	if (!stricmp(scriptname,"startup-wshell")) goto readline;

		/* now, let's do a recursion check... */
		/* if the expanded path name is equal to the expanded path name of
			 a file higher up on the list, then quit this
		*/
		/* Commented out because limit on stack size achieves mostly same effect */

#if 0
	for (i=0; i<=level; i++)
	{	if (!strcmp(stack[i].path,stack[level].path)) goto readline;
	}
#endif

		/* now, bump the level and open the file */

	level++;									/* add 1 to parsing level...	*/
	if (stack[level].fh = Open(stack[level].path,MODE_OLDFILE))	/* open the file */
	{	stack[level].position = 0;
		stack[level].pri = level;
			/* need to init something else? */
	}
	else level--;

readline:
	actual = Fgets((void *)stack[level].fh,script_line,LINESIZE - 1);

	if (actual < 0)								/* end of file test */
	{
endfile:
		Close(stack[level].fh);
		stack[level].fh = NULL;
		level--;
		if (level < 0) 
		{
			FreeMem(script_line,LINESIZE);
			FreeMem(stack,10 * sizeof *stack);
			return best_value;
		}

		/* need to pop the stack... */
	}
	stack[level].position += actual + 1;

		/* now, scan the line... */

	if (script_line[0] == '.') goto readline;	/* dotted line...				*/

		/* see if app_startup_name is in string  */

	scan = script_line;							/* init scan pointer			*/
	script_line[actual] = '\0';					/* use NULL as sentinel			*/

	if (strstr(strlwr(scan),app_startup_name) )
	{
			/* close all open files */
		for (i=0;i<10;i++) if (stack[i].fh) Close(stack[i].fh);

		FreeMem(script_line,LINESIZE);
		FreeMem(stack,10 * sizeof *stack);
		return -1;		/* so they deleted their S:user-startup huh!? */
	}

	argc = 0;									/* set arg count to zero		*/
	while (argc < 32)							/* loop through arguments		*/
	{	while (*scan == ' ' ||					/* skip white space junk		*/
				*scan == '\t' ||
				*scan == '\r'||
				*scan == '\n')
					scan++;

		if (*scan == '\0' || *scan == ';')		/* comment or sentinel ends line */
			break;

		if (*scan == '"')						/* a quoted argument			*/
		{	char *put;

			argv[argc++] = ++scan;				/* start of new argument		*/
			put = (char *)scan;					/* need to reformat...			*/

			for (;;)
			{	if (*scan == '"') break;		/* end of quoted string			*/
				if (*scan == '*')				/* BCPL escape string			*/
				{	scan++;
					if (*scan == 'E' || *scan == 'e') *put++ = 0x1b;
					else if (*scan == 'N' || *scan == 'n') *put++ = '\n';
					else *put++ = *scan;
					scan++;
				}
				else							/* else just copy characters	*/
				{	*put++ = *scan++;
				}
			}
			*put = '\0';
			scan++;
		}
		else if (*scan == '>' || *scan == '<')	/* redirection					*/
		{	while (!(*scan == ' '  ||			/* skip over redirection 		*/
				*scan == ';'  ||
				*scan == '\n' ||
				*scan == '\0' ||
				*scan == '\t' ||
				*scan == '\r')) scan++;
		}
		else									/* a non-quoted argument		*/
		{	argv[argc++] = scan;

			while (!(*scan == ' '  ||			/* scan an argument				*/
				*scan == ';'  ||
				*scan == '\n' ||
				*scan == '\0' ||
				*scan == '\t' ||
				*scan == '\r')) scan++;

			if (*scan == '\0') break;
			else *scan++ = '\0';
		}
	}

	if (argc == 0) goto readline;

		/* now, strip the file path off of argv[0] */

	command = argv[0] = (void *)FileOnly((void *)argv[0]);

		/* test code.... */

#ifdef SCAN_DEBUG
	for (i=0; i<level; i++)
	{	Printf("   ",argv[i]);
	}

	for (i=0; i<argc; i++)
	{	Printf("%s ",argv[i]);
	}
	Printf("\n");
#endif

		/* do if-level testing first... */

	if (MATCHC("IF"))							/* if an IF statement...		*/
	{	if_nest++;								/* add 1 to nesting level		*/
		goto readline;							/* and read next line...		*/
	}
	else if (MATCHC("ENDIF") && if_nest)		/* if an ENDIF statement		*/
	{	if_nest--;								/* remove 1 nesting level		*/
		goto readline;							/* and read next line			*/
	}

	if (if_nest) goto readline;					/* skip if inside IF statement	*/

		/* now, assign priorities... */

	pri = -1;

	if (MATCHC("assign"))
	{	pri = 5 * (++stack[level].pri);
	}
	else if (MATCHC("path") || MATCHC("setenv") || MATCHC("set"))
	{	pri = 3 * (++stack[level].pri);
	}
	else if (MATCHC("copy"))
	{	pri = 3;
	}
	else if (MATCHC("quit") || MATCHC("endcli") || MATCHC("kickit"))
	{	goto endfile;
	}
	else if (MATCHC("execute") && nestable)
	{	next_file = argv[1];
		goto newfile;
	}
	else if (nestable && (MATCHC("newcli") || MATCHC("newshell") || MATCHC("ashell") || MATCHC("NewWSH")))
	{	for (i=1; i<argc; i++)
		{	if (!stricmp(argv[i],"from"))
			{	if (i+1 < argc)
				{	next_file = argv[i+1];
					goto newfile;
				}
			}
		}
	}

	if (pri >= 0 && pri > best_value)
	{	best_value = pri;
		strcpy(best_path,stack[level].path);
		best_position = stack[level].position;
	}
	goto readline;
}

#if 0
main()
{	if (scan_scripts("S:Startup-sequence",TRUE) == 0)
		Printf("Scanner Confused, couldn't find a good spot\n");
	else
		Printf("\nBest spot was:\nFILE: %s\nOFFSET: %ld\n",
			best_path,
			best_position);
	exit(0);
}
#endif
