#include <stdio.h>
#include <fcntl.h>
#include <ios1.h>
#include <string.h>
#include <stdlib.h>
#include <workbench/startup.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>

#define ANDY

#ifndef ANDY
#define FATAL 1000
#else
#define FATAL 20
#endif

#ifndef READARGS
#define MAXARG 256	/* maximum command line arguments */
#define QUOTE  '"'
#define ESCAPE '*'
#define ESC '\027'
#define NL '\n'

#define isspace(c) ((c == ' ')||(c == '\t') || (c == '\n'))
#endif /* !READARGS */

#define MAXWINDOW 45

#ifndef TINY
extern int _fmode,_iomode;
extern int (*_ONBREAK)();
extern int CXBRK();
#endif /* !TINY */

extern void main(int argc, char **argv);

extern char *_ProgramName;
extern struct UFB _ufbs[];
extern struct WBStartup *WBenchMsg;
static char window[MAXWINDOW+18];
static int argc = 0;		/* arg count */
static char **targv;

#ifndef READARGS
static char *str1 = "Invalid argument to ";

static void badarg(char *program)
	{
	_dwrite(_ufbs[2].ufbfh, str1, strlen(str1));
	_dwrite(_ufbs[2].ufbfh, program, strlen(program));
	_dwrite(_ufbs[2].ufbfh, "\n", 1);

	exit(FATAL);
	}
#endif /* !READARGS */

/*
 * name		_xmain - process command line, open files, and call "main"
 *
 * synopsis	_xmain(line, length);
 *		char *line;	argument string to command
 *		int  length	length of argument string
 *
 * description	This function performs the standard pre-processing for
 *		the main module of a C program. It accepts a command
 *		line of the form
 *
 *			pgmname arg1 arg2 ...
 *
 *		and builds a list of pointers to each argument. The first
 *		pointer is to the program name. It calls the function "main"
 *		with argc equal to the number of entries in the list and argv
 *		as the address of the first entry of the array of pointers
 *		to the arguments.
 *
 *		If the program is started from WorkBench then argc will be
 *		zero and argv is the address of the WorkBench message.
 *
 * options	If NOWB is defined at compile time, then the program will
 *		exit immediatly with a return value of FATAL without calling
 *		"main" when started from WorkBench.
 *
 *		If TINY is defined at compile time, then the stdio streams
 *		will not be initialized.
 *
 *		If READARGS is defined at compile time, then the standard
 *		UNIX pre-processing of command line arguments is not
 *		performed. Argc will be the length of the argument string
 *		and argv will be its address. WorkBench startup is not
 *		affected by this switch.
 */
void _xmain(char *line, int length)
	{
#ifndef TINY
	int x;
#endif /* !TINY */

	/*
	 *
	 * Open standard files
	 *
	 */
#ifndef TINY
	if (line)		/* running under CLI */
		{
		_ufbs[0].ufbfh = Input();
		_ufbs[1].ufbfh = Output();
		_ufbs[2].ufbfh = Open("*", MODE_OLDFILE);
		x = UFB_NC;                     /* do not close CLI defaults    */
		}
	else			/* running under workbench */
#ifndef NOWB
		{
		struct Process *process;
		struct FileHandle *handle;

		strcpy(window, "con:10/10/320/80/");
		strncat(window, WBenchMsg->sm_ArgList->wa_Name, MAXWINDOW);
		strncat(window, "/auto", MAXWINDOW);
		_ufbs[0].ufbfh  = Open(window,MODE_NEWFILE);
		_ufbs[1].ufbfh  = _ufbs[0].ufbfh;
		_ufbs[1].ufbflg = UFB_NC;
		_ufbs[2].ufbfh  = _ufbs[0].ufbfh;
		_ufbs[2].ufbflg = UFB_NC;
		handle = (struct FileHandle *)(_ufbs[0].ufbfh << 2);
		process = (struct Process *)FindTask(0);
		process->pr_ConsoleTask = (APTR)handle->fh_Type;
		x = 0;
		}
#else /* !NOWB */
		{
		_exit(FATAL);
		}
#endif /* !NOWB */

	_ufbs[0].ufbflg |= UFB_RA | O_RAW | x;
	_ufbs[1].ufbflg |= UFB_WA | O_RAW | x;
	_ufbs[2].ufbflg |= UFB_RA | O_RAW | UFB_WA;

	stdin->_file  = 0;
	stdout->_file = 1;
	stderr->_file = 2;

	x = (_fmode) ? 0 : _IOXLAT;
	stdin->_flag  = _IOREAD | x;
	stdout->_flag = _IOWRT  | x;
	stderr->_flag = _IORW   | x;

	/* establish control-c handler */

#ifndef READARGS
	_ONBREAK = CXBRK;
#endif /* !RAEDARGS */
#endif /* !TINY */

	if (line)
#ifndef READARGS
		{
		char **argv;		/* arg pointers */
		char *argbuf;

		if (!(argv = (char **) malloc(MAXARG*sizeof(char *))))
			{
			_exit(FATAL);
			}

		if (!(argbuf = malloc(_ProgramName[-1] + length + 2)))
			{
			_exit(FATAL);
			}

		argv[argc++] = argbuf;
		strncpy(argbuf, _ProgramName, _ProgramName[-1]);
		argbuf += _ProgramName[-1];
		*argbuf++ = '\0';

		/*
		 *
		 * Build argument pointer list
		 *
		 */
		while (argc < MAXARG)
			{
			while (isspace(*line) && length)
				{
				line++;
				length--;
				}
			if (!length)
				{
				break;
				}
			argv[argc++] = argbuf;
			if (*line == QUOTE)
				{
				line++;
				length--;
				while (length && (*line != QUOTE))
					{
					if (*line == ESCAPE)
						{
						line++;
						length--;
						if (!length)
							{
							badarg(*argv);
							}
						else
							{
							switch (*line)
								{
								case 'E':
									*argbuf++ = ESC;
									break;
								case 'N':
									*argbuf++ = NL;
									break;
								default:
									*argbuf++ = *line;
								}
							length--;
							line++;
							}
						}
					else
						{
						*argbuf++ = *line++;
						length--;
						}
					}

				if (!length)
					{
					badarg(*argv);
					}

				line++;
				length--;
				*argbuf++ = '\0';	/* terminate arg */
	
				if (length && !isspace(*line))
					{
					badarg(*argv);
					}
				}
			else  /* non-quoted arg */
				{
				while (length && (!isspace(*line)))
					{
					*argbuf++ = *line++;
					length--;
					}
				if (!length)
					{
					*argbuf = '\0';
					break;
					}
				else
					{
					*argbuf++ = '\0';	/* terminate arg */
					}
				}
			}  /* while */
		targv = (char **)&argv[0];
		}
#else /* !READARGS */
		{
		argc = length;
		targv = (char **)line;
		}
#endif /* !READARGS */

#ifndef NOWB
	else
		{
		targv = (char **)WBenchMsg;
		}
#endif /* !NOWB */

	/*
	 *
	 * Call user's main program
	 *
	 */

	main(argc, targv);	/* call main function */
#ifndef TINY
	exit(0);
#else /* !TINY */
	_exit(0);
#endif /* !TINY */
	}
