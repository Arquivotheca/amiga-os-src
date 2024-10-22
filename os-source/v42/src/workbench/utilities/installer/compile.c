/*
 *		compile.c
 *
 * ========================================================================= 
 * Compile.c - converts parsed clauses to internal representation            
 * By Talin. (c) 1990 Sylvan Technical Arts                                  
 * ========================================================================= 
 *
 *
 * Prototypes for functions defined in compile.c
 *	
 *	extern void ** 				errmsg_ptr;
 *	extern void ** 				specmsg_ptr;
 *	extern void ** 				foreach_type;
 *	extern void ** 				foreach_name;
 *	extern void ** 				ioerr_ptr;
 *	extern void ** 				execute_dir_name;
 *	extern void ** 				abort_button_text;
 *	extern struct 				TokenState tstate;
 *	extern jmp_buf 				env;
 *
 *	struct String * 			MakeString			(unsigned char * , WORD );
 *	void 						FreeString			(struct String * );
 *
 *	extern struct funcdef 		statement_syms		[17];
 *	extern struct funcdef 		control_syms		[9];
 *	extern unsigned char 		func_math_name		[5];
 *	extern unsigned char 		func_debug_name		[6];
 *	extern struct funcdef 		func_syms			[54];
 *	extern struct funcdef 		param_syms			[31];
 *	extern struct funcdef 		string_syms			[15];
 *	extern struct funcdef 		help_syms			[11];
 *	extern struct GSymbolScope 	tscope;
 *
 *	void 						sym_delete_hook		(struct GSymbolScope * , struct Value * );
 *	int 						init_symbols		(void);
 *	void 						cleanup_symbols		(void);
 *	unsigned char * 			override_text		(UWORD );
 *
 *	extern WORD 				context;
 *
 *	struct TreeNode * 			compile				(unsigned char * );
 *	void 						parse_list			(struct TreeNode * );
 *	void 						de_compile			(struct TreeNode * );
 *	extern WORD 				parse_debug;
 *
 *	extern unsigned char * 		compile_err;
 *	extern long 				linenum;
 *
 *	void 						fatal				(unsigned char * );
 *	void 						prefatal			(unsigned char * );
 *
 *	extern unsigned char 		inverse_on			[7];
 *	extern unsigned char 		inverse_off			[7];
 *	void 						offending_line		(short );
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 10/25/93:
 *		Changed compare for do_welcome function to work correctly so that
 *		user welcome message will be displayed.
 */



#include "installer.h"

#include <setjmp.h>
#include <stddef.h>
#include <string.h>

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

void prefatal 			(char *);
void fatal	  			(char *);
void offending_line		(short n);
void cleanup_irecord	(void);

extern void	**pretend_ptr, 
			**ulevel_ptr, 
			**icon_ptr, 
			**defpart_ptr, 
			**app_ptr;
			
void		**errmsg_ptr, 
			**specmsg_ptr, 
			**foreach_type, 
			**foreach_name;
			
void		**ioerr_ptr, 
			**execute_dir_name, 
			**abort_button_text;
			
extern void	**language_ptr;

struct TokenState tstate;

jmp_buf 			env;						/* setjmp for errors			*/

/* ============================ Object Management ========================= */

	/* REM: later we can replace both of these routines with ones that
		don't require so much fragmented memory.
	*/

static struct TreeNode *MakeNode (
	struct TreeNode *prevnode, 
	struct TreeNode *parent)
{
	struct TreeNode *node;
												/* allocate a tree node			*/
	if (node = AllocMem(sizeof *node, MEMF_CLEAR))
	{
		if (prevnode) 
			prevnode->next = node;
		else 
		if (parent) 
			parent->value.v = node;
		node->line = tstate.token_history[0].source_line;
		node->value.storage = STORE_LITERAL;
	}
	else 
		prefatal("Out of Memory");				/* return an error				*/
	return node;								/* return the node				*/
}

static void FreeNode(struct TreeNode *node)		/* free a tree node				*/
{
	if (node)									/* if node exists				*/
	{
		/* Q: check for string?? */
		FreeMem(node,sizeof *node);				/* free the memory				*/
	}
}

struct String *MakeString(char *text, WORD length)
{
	/*	To avoid future confusion, this routine deserves a little description.
	 *
	 *	The String structure is defined to be a UWORD.  This is the first two
	 *	bytes, which is the string length.  the string follows that, and is
	 *	terminated by a null.  The two bytes for the length plus the null
	 *	count for the "3" that is being added to the length.  Also, since
	 *	the structure as defined is 2 bytes long, the "+1" correctly points
	 *	past the length to where the string will start.
	 */
	 
	struct String *s;

	if (s = AllocMem(length + 3, 0L))			/* get string, plus size + null	*/
	{
		s->length = length;						/* set length					*/
		CopyMem(text,s+1,length);				/* copy string					*/
		((UBYTE *)s)[length+2] = '\0';			/* null terminate				*/
	}
	else 
		prefatal("Out of memory");				/* else error					*/
	return s;									/* return the string			*/
}

void FreeString(struct String *s)				/* free an allocated string		*/
{
	if (s) 
		FreeMem (s, s->length + 3);				/* see above on +3 comments		*/
}


/* ============================== Symbol Table =========================== */

	/* define initial symbols */

struct funcdef statement_syms[] = 
{
	{ "MAKEDIR",	(int (*)())do_newdir,	ACTION_NEWDIR		},
	{ "COPYFILES",	(int (*)())do_files,	ACTION_FILES		},
	{ "COPYLIB",	(int (*)())do_libs,		ACTION_LIBS			},
	{ "STARTUP",	(int (*)())do_startup,	ACTION_STARTEDIT	},
	{ "TOOLTYPE",	(int (*)())do_tooltype,	ACTION_TOOLTYPE		},
	{ "TEXTFILE",	(int (*)())do_textfile,	ACTION_TEXTFILE		},
	{ "EXECUTE",	(int (*)())do_execute,	ACTION_OTHER		},
	{ "RUN",		(int (*)())do_run,		ACTION_OTHER		},
	{ "REXX",		(int (*)())do_rexx,		ACTION_OTHER		},
	{ "RENAME",		(int (*)())do_rename,	ACTION_RENAME		},
	{ "DELETE",		(int (*)())do_delete,	ACTION_DELETE		},
	{ "ABORT",		(int (*)())do_abort,	ACTION_NONE			},
	{ "EXIT",		(int (*)())do_exit,		ACTION_OTHER		},
	{ "COMPLETE",	(int (*)())do_complete,	ACTION_NONE			},
	{ "MESSAGE",	(int (*)())do_message,	ACTION_OTHER		},
	{ "WORKING",	(int (*)())do_working,	ACTION_OTHER		},
	{ "WELCOME",	(int (*)())do_welcome,	ACTION_NONE			}
};

struct funcdef welcome_compare = 
	{ "WELCOME",	(int (*)())do_welcome,	ACTION_NONE			};

struct funcdef control_syms[] = 
{
	{ "SET",		(int (*)())doset							},
	{ "IF",			(int (*)())doif								},
	{ "WHILE",		(int (*)())dowhile							},
	{ "UNTIL",		(int (*)())dountil							},
	{ "TRAP",		(int (*)())dotrap							},
	{ "ONERROR",	(int (*)())do_onerror						},
	{ "FOREACH",	(int (*)())do_foreach						},
	{ "SELECT",		(int (*)())doselect							},
	{ "PROCEDURE",	(int (*)())doproc 							}
};

char	func_math_name[]  = "MATH",
		func_debug_name[] = "DEBUG";

struct funcdef func_syms[] = 
{
	{ "CAT",		(int (*)())do_cat,							},
	{ "TACKON",		(int (*)())do_tackon, 						},
	{ "FILEONLY",	(int (*)())do_fileonly,						},
	{ "PATHONLY",	(int (*)())do_pathonly,						},
	{ "ASKFILE",	(int (*)())do_askfile,		ACTION_ASKFILE 	},
	{ "ASKDIR",		(int (*)())do_askdir,		ACTION_ASKDIR 	},
	{ "ASKSTRING",	(int (*)())do_askstring,	ACTION_ASKSTRING },
	{ "ASKNUMBER",	(int (*)())do_asknumber,	ACTION_ASKNUM 	},
	{ "ASKCHOICE",	(int (*)())do_askchoice,	ACTION_ASKCHOICE },
	{ "ASKOPTIONS",	(int (*)())do_askoptions,	ACTION_ASKCHOICE },
	{ "ASKBOOL",	(int (*)())do_askbool,		ACTION_ASKBOOL 	},
	{ "ASKDISK",	(int (*)())do_askdisk,		ACTION_ASKDISK 	},
	{ "EXISTS",		(int (*)())do_exists,		ACTION_OTHER 	},
	{ "EARLIER",	(int (*)())do_earlier,						},
	{ "GETSIZE",	(int (*)())do_getsize,						},
	{ "GETDISKSPACE",(int (*)())do_getdiskspace,				},
	{ "GETSUM",		(int (*)())do_getsum,						},
	{ "GETVERSION",	(int (*)())do_getversion,	ACTION_OTHER 	},
	{ "GETASSIGN",	(int (*)())do_getassign,					},
	{ "MAKEASSIGN",	(int (*)())do_makeassign,	ACTION_OTHER 	},
	{ "GETENV",		(int (*)())do_getenv						},
	{ "SELECT",		NULL										},
	{ "=",			(int (*)())do_equal							},
	{ ">",			(int (*)())do_greater						},
	{ ">=",			(int (*)())do_greater_equal					},
	{ "<",			(int (*)())do_lesser						},
	{ "<=",			(int (*)())do_lesser_equal					},
	{ "<>",			(int (*)())do_unequal						},
	{ "+",			(int (*)())do_plus							},
	{ "-",			(int (*)())do_minus							},
	{ "*",			(int (*)())do_mul							},
	{ "/",			(int (*)())do_div							},
	{ "AND",		(int (*)())do_and							},
	{ "OR",			(int (*)())do_or							},
	{ "NOT",		(int (*)())do_not							},
	{ "XOR",		(int (*)())do_xor							},
	{ "IN",			(int (*)())do_in							},
	{ func_debug_name,	(int (*)())do_debug						},
	{ "TRANSCRIPT",	(int (*)())do_trans,						},
	{ "PROTECT",	(int (*)())do_protect,						},
	{ "USER",		(int (*)())do_user,							},
	{ "SHIFTLEFT",	(int (*)())do_shiftleft						},
	{ "SHIFTRIGHT",	(int (*)())do_shiftright					},
	{ "SUBSTR",		(int (*)())do_substr						},
	{ "EXPANDPATH",	(int (*)())do_expandpath					},
	{ "DATABASE",	(int (*)())do_database						},
	{ "DELOPTS",	(int (*)())do_delopts						},
	{ "STRLEN",		(int (*)())do_strlen						},
	{ "PATMATCH",	(int (*)())do_patmatch						},
	{ "GETDEVICE",	(int (*)())do_getdevice						},
	{ "BITAND",		(int (*)())do_bitand						},
	{ "BITOR",		(int (*)())do_bitor							},
	{ "BITNOT",		(int (*)())do_bitnot						},
	{ "BITXOR",		(int (*)())do_bitxor						}
};

#define OK_NEWDIR	(1L<<ACTION_NEWDIR)
#define OK_FILES	(1L<<ACTION_FILES)
#define OK_STARTUP	(1L<<ACTION_STARTEDIT)
#define OK_TOOLTYPE	(1L<<ACTION_TOOLTYPE)
#define OK_TEXTFILE	(1L<<ACTION_TEXTFILE)
#define OK_RENAME	(1L<<ACTION_RENAME)
#define OK_DELETE	(1L<<ACTION_DELETE)
#define OK_OTHER	(1L<<ACTION_OTHER)
#define OK_FUNC		(1L<<ACTION_FUNC)
#define OK_ASKFILE	(1L<<ACTION_ASKFILE)
#define OK_ASKDIR	(1L<<ACTION_ASKDIR)
#define OK_ASKSTR	(1L<<ACTION_ASKSTRING)
#define OK_ASKNUM	(1L<<ACTION_ASKNUM)
#define OK_ASKCHOI	(1L<<ACTION_ASKCHOICE)
#define OK_LIBS		(1L<<ACTION_LIBS)
#define OK_ASKBOOL	(1L<<ACTION_ASKBOOL)
#define OK_ASKDISK	(1L<<ACTION_ASKDISK)

#define OK_ASK		(OK_ASKFILE|OK_ASKDIR|OK_ASKSTR|OK_ASKNUM|OK_ASKCHOI|OK_ASKBOOL|OK_LIBS|OK_ASKDISK)
#define OK_ALL		(OK_NEWDIR|OK_FILES|OK_STARTUP|OK_TOOLTYPE|OK_TEXTFILE|OK_OTHER|OK_RENAME|OK_DELETE|OK_ASK)

struct funcdef param_syms[] = 
{
	{ "HELP",		(int (*)())do_help,			OK_ALL },
	{ "PROMPT",		(int (*)())do_prompt,		OK_ALL },
	{ "CHOICES",	(int (*)())do_choices,		OK_FILES|OK_FUNC|OK_ASKCHOI|OK_ASKBOOL },
	{ "PATTERN",	(int (*)())do_pattern,		OK_FILES },
	{ "ALL",		(int (*)())do_all,			OK_FILES },
	{ "SOURCE",		(int (*)())do_source,		OK_FILES|OK_RENAME|OK_LIBS },
	{ "DEST",		(int (*)())do_dest,			OK_FILES|OK_TOOLTYPE|OK_TEXTFILE|OK_RENAME|OK_DELETE|OK_NEWDIR|OK_LIBS|OK_ASKDISK },
	{ "NEWNAME",	(int (*)())do_newname,		OK_FILES|OK_LIBS|OK_ASKDISK },
	{ "CONFIRM",	(int (*)())do_confirm,		OK_NEWDIR|OK_FILES|OK_TEXTFILE|OK_TOOLTYPE|OK_RENAME|OK_DELETE|OK_LIBS|OK_OTHER },
	{ "SAFE",		(int (*)())do_safe,			OK_FILES|OK_NEWDIR|OK_TEXTFILE|OK_TOOLTYPE|OK_RENAME|OK_DELETE|OK_LIBS|OK_OTHER },
	{ "INFOS",		(int (*)())do_infos,		OK_NEWDIR|OK_FILES|OK_LIBS|OK_DELETE },
	{ "SETTOOLTYPE",(int (*)())do_settooltype,	OK_TOOLTYPE },
	{ "SETDEFAULTTOOL",(int (*)())do_defaulttool,	OK_TOOLTYPE },
	{ "SETSTACK",	(int (*)())do_setstack,		OK_TOOLTYPE },
	{ "NOPOSITION",	(int (*)())do_noposition,	OK_TOOLTYPE|OK_FILES|OK_LIBS },
	{ "DEFAULT",	(int (*)())do_default,		OK_ASKSTR|OK_ASKCHOI|OK_ASKNUM|OK_ASKDIR|OK_ASKFILE|OK_ASKBOOL, },
	{ "RANGE",		(int (*)())do_range,		OK_ASKNUM },
	{ "COMMAND",	(int (*)())do_append,		OK_STARTUP	},
	{ "APPEND",		(int (*)())do_append,		OK_TEXTFILE },
	{ "FILES",		(int (*)())do_filesonly,	OK_FILES },
	{ "INCLUDE",	(int (*)())do_include,		OK_TEXTFILE },
	{ "OPTIONAL",	(int (*)())do_optional,		OK_FILES|OK_LIBS|OK_DELETE|OK_ASKDIR|OK_TOOLTYPE },
	{ "FONTS",		(int (*)())do_fonts,		OK_FILES },
	{ "DISK",		(int (*)())do_disk,			OK_RENAME|OK_ASKDIR },
	{ "RESIDENT",	(int (*)())do_resident,		OK_LIBS|OK_OTHER },
	{ "SWAPCOLORS",	(int (*)())do_optional,		OK_TOOLTYPE },
	{ "NEWPATH",	(int (*)())do_safe,			OK_ASKDIR },
	{ "NOREQ",		(int (*)())do_resident,		OK_OTHER },
	{ "ASSIGNS",	(int (*)())do_optional,		OK_ASKDISK },
	{ "NOGAUGE",	(int (*)())do_nogauge,		OK_FILES|OK_LIBS },
	{ "QUIET",		(int (*)())do_resident,		OK_OTHER }
};

struct funcdef string_syms[] = 
{
	{ "@icon"		,(void *)1,		TYPE_STRING  },		/* icon path name */
	{ "@default-dest",(void *)4,	TYPE_STRING  },		/* default installation partition */
	{ "@pretend"	,(void *)2,		TYPE_INTEGER },		/* pretend mode flag */
	{ "@user-level"	,(void *)3,		TYPE_INTEGER },		/* novive mnode flag */
	{ "@error-msg"	,(void *)5,		TYPE_STRING  },		/* error msg for trap */
	{ "@special-msg",(void *)6,		TYPE_STRING  },		/* special msg for error */
	{ "@each-type"	,(void *)7,		TYPE_INTEGER },		/* type of foreach object */
	{ "@each-name"	,(void *)8,		TYPE_STRING  },		/* name of foreach object */
	{ "@ioerr"		,(void *)9,		TYPE_INTEGER },		/* number of last IoErr */
	{ "@execute-dir",(void *)10,	TYPE_STRING  },		/* change dir to here */
	{ "FALSE"		,(void *)11,	TYPE_INTEGER },
	{ "TRUE"		,(void *)12,	TYPE_INTEGER },
	{ "@language"	,(void *)13,	TYPE_STRING  },
	{ "@app-name"	,(void *)14,	TYPE_STRING  },
	{ "@abort-button",(void *)15,	TYPE_STRING  }
};

	/* the following are the default help texts available */

extern struct String

	askoptions_help, 
	askchoice_help, 
	asknumber_help,
	askstring_help, 
	askdisk_help, 
	version_help, 
	selectdir_help,
	selectfile_help, 
	copyfiles_help, 
	makedir_help, 
	startup_help;

struct funcdef help_syms[] = 
{
	{ "@askoptions-help",	(void *)HELP_ASKOPTIONS,	TYPE_STRING },
	{ "@askchoice-help",	(void *)HELP_ASKCHOICE,		TYPE_STRING },
	{ "@asknumber-help",	(void *)HELP_ASKNUMBER,		TYPE_STRING },
	{ "@askstring-help",	(void *)HELP_ASKSTRING,		TYPE_STRING },
	{ "@askdisk-help",		(void *)HELP_ASKDISK,		TYPE_STRING },
	{ "@copylib-help",		(void *)HELP_VERSION,		TYPE_STRING },
	{ "@copyfiles-help",	(void *)HELP_COPYFILES,		TYPE_STRING },
	{ "@askfile-help",		(void *)HELP_SELECT_FILE,	TYPE_STRING },
	{ "@askdir-help",		(void *)HELP_SELECT_DRAWER,	TYPE_STRING },
	{ "@makedir-help",		(void *)HELP_MAKEDIR,		TYPE_STRING },
	{ "@startup-help",		(void *)HELP_STARTUP,		TYPE_STRING }
};

struct GSymbolScope		tscope;

void sym_delete_hook (
	struct GSymbolScope *scope,
	struct Value *value)
{
	if ((value->storage == STORE_LITERAL || value->storage == STORE_TEMP) 
	&&	(value->type == TYPE_STRING || value->type == TYPE_MACRO) )
	{
		FreeString(value->v);
		value->v = NULL;
	}
}

	/* add the list of pre-defined symbols to the symbol table */

static add_symbols(struct funcdef *list, WORD total, WORD type)
{
	while (total--)								/* for each symbol in list	*/
	{
		struct GSymbol	**where_to_add;
		struct ISymbol	*isym;
		WORD			length;

		if (list->func == NULL)					/* skip deleted nodes		*/
		{
			list++;								/* next function			*/
			continue;
		}

		length = strlen(list->name);

		/*	Call FindGSymbol to figure out where to insert symbol,
		 *	but ignore return value if not debugging code...
		 */

#ifdef DEBUG
		{
			struct GSymbol	*sym;
												/* look for where to add		*/
			sym = FindGSymbol (&tscope, list->name, length, &where_to_add);

			/*	we know this can't happen, since we would not be stupid 
			 *	enough to make duplicate pre-defined symbols
			 */

			if (sym)							/* if it was already there		*/
			{
				Printf("Duplicate symbol '%s'.\n",list->name);
				return NULL;					/* then error					*/
			}									/* add the symbol				*/
		}
#else
												/* look for where to add		*/
		FindGSymbol(&tscope,list->name,length,&where_to_add);
#endif

		isym = (struct ISymbol *)AddGSymbol(&tscope,list->name,length,where_to_add);
		if (!isym)								/* if couldn't add				*/
		{
			Printf("Memory error adding symbol");
				/* REM - make this an alert or something... */
			return NULL;
		}

		if (type == TYPE_NONE)
		{
				/* default help texts */
			isym->value.type = TYPE_STRING;
			isym->value.storage = STORE_PERM;
			isym->value.v = (void *)GetLocalText((long)list->func);
		}
		else 
		if (type == TYPE_STRING)
		{	
			isym->value.type = list->syntax_check;	/* fill in type					*/
			isym->value.storage = (isym->value.type == TYPE_STRING ?
					STORE_PERM : STORE_LITERAL);	/* keep while program running */

			switch ((ULONG)(list->func)) 
			{
			case 1: isym->value.v = NULL;			/* icon name string			*/
					icon_ptr = &isym->value.v;
					break;

			case 2: pretend_ptr = &isym->value.v;	/* pretend mode flag		*/
					break;

			case 3: ulevel_ptr = &isym->value.v;	/* novice mode flag			*/
					break;

			case 4: isym->value.v = NULL;			/* partition name string	*/
					defpart_ptr = &isym->value.v;
					break;

			case 5: isym->value.v = NULL;			/* error msg text	*/
					errmsg_ptr = &isym->value.v;
					break;

			case 6: isym->value.v = NULL;			/* special msg error text	*/
					specmsg_ptr = &isym->value.v;
					break;

			case 7: foreach_type = &isym->value.v;	/* foreach type				*/
					break;

			case 8: isym->value.v = NULL;			/* foreach name				*/
					foreach_name = &isym->value.v;
					break;

			case 9: ioerr_ptr = &isym->value.v;		/* last ioerr value			*/
					break;

			case 10: isym->value.v = NULL;			/* execute dir				*/
					execute_dir_name = &isym->value.v;
					break;

			case 11: isym->value.v = (void *)0;		/* FALSE					*/
					break;

			case 12: isym->value.v = (void *)1;		/* TRUE						*/
					break;

			case 13: isym->value.v = NULL;			/* language					*/
					language_ptr = &isym->value.v;
					break;

			case 14: isym->value.v = NULL;			/* app name					*/
					app_ptr = &isym->value.v;
					break;

			case 15: isym->value.v = NULL;			/* Abort Install replacement*/
					abort_button_text = &isym->value.v;
					break;
			}
		}
		else
		{
			isym->value.type = type;			/* fill in type					*/
			isym->value.storage = STORE_PERM;	/* storage is permanent			*/
			isym->value.v = list;				/* address of function descr.	*/
		}
		list++;									/* next function				*/
	}
	return TRUE;
}

init_symbols(void)
{
	zero (&tscope, sizeof tscope);
	tscope.SymbolDataSize = sizeof (struct Value);
	tscope.DeleteHook = (int (*)()) sym_delete_hook;

	/* AddGHash(&tscope,255); */

	if (add_symbols(statement_syms,elementsof(statement_syms),TYPE_STATEMENT) 
	&&	add_symbols(control_syms,  elementsof(control_syms),  TYPE_CONTROL) 
	&&	add_symbols(func_syms,     elementsof(func_syms),     TYPE_FUNCTION) 
	&&	add_symbols(param_syms,    elementsof(param_syms),    TYPE_PARAMETER) 
	&&	add_symbols(string_syms,   elementsof(string_syms),   TYPE_STRING) 
	&&	add_symbols(help_syms,     elementsof(help_syms),     TYPE_NONE))
	{
			return TRUE;
	}
	return FALSE;
}

void cleanup_symbols(void)
{
	DeleteGScope(&tscope);
}

char *override_text(UWORD num)
{
	if (num == TX_ABORT && *abort_button_text)
	{
		return STR_VAL(*abort_button_text);
	}

	return NULL;
}

/* ============================ Parsing Routines ========================= */

/* there is a question if we want to use the default rules for symbol names */

void parse_list (struct TreeNode *parent);
void de_compile (struct TreeNode *node);

WORD context;

static is_idfirst (char c)
{	
	if (c == ',' || c == ';' || c == '\\') 
		return FALSE;
	if (c > ')') 
		return TRUE;
	return !(c <= ' ' || c >= '\'' || c == '"');
}

static is_idrest(char c)
{
	if (c > ')') 
		return TRUE;
	return !(c <= ' ' || c >= '\'' || c == '"');
}

struct TreeNode *compile (char *filename)
{
	struct TreeNode		*root,
						*lastnode;
	WORD				lerror;
	WORD				jmp;
	WORD				save_context;

		/* initialize the parser */

	begin_parse(&tstate,
				MAX_SYMBOL,
				MAX_STRING,
				SKIP_RETURN | SKIP_BLANKS | EOL_BAD_STRING,
				is_idfirst,
				is_idrest,
				";",
				NULL,
				"$",
				"%");

		/* tell the parser to start parsing the template file */

		/* REM: We need to insert error handling here... --DJ */

	lerror = push_file(&tstate,filename);
	if (lerror)	
		return NULL;
#if 0
	{	if (lerror == LPARSE_FILEERR)
		{
			;
		}
		else 
		if (lerror == LPARSE_MEMERR)
		{
			;
		}
		return NULL;
	}
#endif

		/* make the root node */

	root = MakeNode(NULL,NULL);
	root->value.type = TYPE_LIST;

		/* initialize error recovery */

	if (jmp = setjmp(env))
	{
		if (root) 
			de_compile(root);
		end_parse(&tstate);
		return NULL;
	}

		/* now, start parsing clauses */

	lastnode = NULL;
	context = ACTION_NONE;

	next_token(&tstate);

	for (;;)
	{
		switch (tstate.token_type) 
		{
		case TOKEN_PUNC:						/* if it's a punctuation		*/
			if (tstate.numeric_val == '(')		/* if start of new list			*/
			{
				struct TreeNode *newlist;

				next_token(&tstate);			/* skip to next token			*/
												/* make a new list node			*/
				lastnode = newlist = MakeNode(lastnode,root);
				newlist->value.type = TYPE_LIST;
				save_context = context;			/* save the parent type			*/
				parse_list(newlist);
				context = save_context;			/* restore parent type			*/
			}
			else 
			if (tstate.numeric_val == ')')		/* if end of list				*/
				fatal("Unmatched parenthesis");
			else 
				fatal("Unexpected character");
			break;

		case TOKEN_SYMBOL:
#if 0
				/* do #define macro definition */
				/* how to detect beginning of line? */

			if (tstate.numeric_val == 7 && !strcmp(tstate.text_val,"#define"))
			{
				WORD c, length = 0;
				char macro_def[MAX_MACRO];
				struct GSymbol	**where_to_add;
				struct ISymbol	*isym;

				next_token(&tstate);
				if (tstate.token_type != TOKEN_SYMBOL)
					fatal("#define must be followed by symbol name");

												/* capture macro string			*/
				for (length = 0; length < MAX_MACRO; length++)
				{
					c = next_char(&tstate);
					if (c == '\n' || c == '\r') 
						break;
					macro_def[length] = c;
				}
				if (length == MAX_MACRO)		/* if too long, error			*/
					fatal("Macro definition too long");

												/* look for symbol				*/
				isym = (struct ISymbol *)FindGSymbol(
					&tscope,tstate.text_val,tstate.numeric_val,&where_to_add);
				if (isym) 
					prefatal("Symbol already defined");

												/* create symbol				*/
				isym = (struct ISymbol *)AddGSymbol(
					&tscope,tstate.text_val,tstate.numeric_val,where_to_add);
				if (!isym) 
					prefatal("Memory error adding symbol");

				isym->value.type = TYPE_MACRO;	/* type of symbol is macro		*/
				isym->value.storage = STORE_LITERAL;
				isym->value.v = MakeString(macro_def,length);

				next_token(&tstate);			/* skip over macro name			*/
			}
			else
#endif
				prefatal("Symbol name not valid here");
			break;

		case TOKEN_EOF:		return root;		/* file end				*/
		case TOKEN_TERMERR: prefatal("Unterminated string!");
		case TOKEN_COMMERR:	prefatal("Unterminated comment!");
		default:			prefatal("Clause definition expected");
		}
	}
}

#if 0
struct ISymbol dummy_symbol = 
{
	{ NULL, 0, 0 },
	{ TYPE_FUNCTION, STORE_PERM, NULL }
};
#endif

/* REM: Think of extra conditions we can tack on to make the syntax mode strict
	during the parsing stage:
		1. Make sure parameters are correct for type of install statement.
		2. Can't have statement in a statement. (can have control though).
		3. Can have parameter only in a statement.
		4. Can't have function at root level.
		5. Can't have function if no-one to return value to.
		6. Can't have number as first item in a list.

	Note: control statements are also functions, somewhat.
*/

void parse_list (struct TreeNode *parent)
{
	struct TreeNode		*lastnode,
						*newnode;
	WORD				save_context;
	struct funcdef		*f;

	lastnode = 0;								/* start with no previous node	*/

	for (;;)									/* while there are list elements */
	{
		struct GSymbol	**where_to_add;
		struct ISymbol	*isym, *psym;

		switch (tstate.token_type) 
		{
		case TOKEN_SYMBOL:
												/* look for symbol				*/
			isym = (struct ISymbol *)FindGSymbol(
				&tscope,tstate.text_val,tstate.numeric_val,&where_to_add);
			if (!isym)							/* if not found					*/
			{									/* then attempt to add			*/
				isym = (struct ISymbol *)AddGSymbol(
					&tscope,tstate.text_val,tstate.numeric_val,where_to_add);
				/* isym->value.type = TYPE_NONE; */ /* implicit */
			}									/* if couldn't add, then error	*/
			if (!isym)
				prefatal("Memory error adding symbol");
												/* check if it's a macro		*/
#if 0
			if (isym->value.type == TYPE_MACRO)
			{
				struct String *macro = isym->value.v;
												/* expand the macro				*/
				push_macro(&tstate,(char *)(macro+1),macro->length,NULL,NULL);
			}
			else								/* make a node for this symbol	*/
#endif
			{
				if (lastnode)					/* if this is not the first node */
				{
					if (isym->value.type == TYPE_FUNCTION 
					||	isym->value.type == TYPE_STATEMENT 
					||	isym->value.type == TYPE_PARAMETER 
					||	isym->value.type == TYPE_CONTROL)
					{
						prefatal("Can't use statement/parameter/function as data item ");
					}

				/*	wonderful kludges for allowing subroutines, but
					maintaining other type checking						*/

					if (lastnode->value.type == TYPE_SYMBOL)
					{
						psym = lastnode->value.v;
						if (psym->value.type == TYPE_CONTROL)
						{
							f = psym->value.v;
							if (f->func == (void *)doproc)
							{
								isym->value.type = TYPE_INTEGER;
							}
						}
					}
				}
				else							/* if this is the first node	*/
				{
					f = isym->value.v;

					switch (isym->value.type) 
					{
#if 0
					case TYPE_NONE:				/* can't be non-statement */
						prefatal("not a statement");
						break;
#endif
					case TYPE_STATEMENT:		/* check for nested stmt */
						if (context)
							prefatal("Can't have nested statements");

						/*  special processing for welcome.
						 *
						 *	Note that we can't do a literal compare to
						 *	"&do_welcome" here (against f->func) since
						 *	the compiler creates a branch temp to reach
						 *	the do_welcome function, and the compare fails!
						 */

						if (f->func == welcome_compare.func)
							welcome_state = 1;		/* don't do auto-welcome */

						context = f->syntax_check;
						break;

					case TYPE_PARAMETER:		/* check for param out of stmt */
						if (!context) 
							prefatal("parameter outside of statement");

						if (!( (1<<context) & f->syntax_check))
							prefatal("invalid parameter for statement");
						break;

					case TYPE_FUNCTION:
						if (f->syntax_check) 
							context = f->syntax_check;
						break;
					}
				}

				lastnode = newnode = MakeNode(lastnode,parent);
				newnode->value.type = TYPE_SYMBOL;
				newnode->value.v = isym;		/* patch in symbol address */
			}
			next_token(&tstate);				/* skip over symbol name		*/
			break;

		case TOKEN_PUNC:
			switch (tstate.numeric_val) 
			{
			case '(':							/* beginning of new list		*/
				next_token(&tstate);			/* skip over '('				*/
				lastnode = newnode = MakeNode(lastnode,parent);
				newnode->value.type = TYPE_LIST;
				save_context = context;			/* save type of parent			*/
				parse_list(newnode);			/* create a sub-list			*/
				context = save_context;			/* restore type of parent		*/
				break;

			case ')':							/* end of this list				*/
				if (parent->value.v == NULL)
					prefatal("empty clause not allowed");
				next_token(&tstate);			/* skip over ')'				*/
				return;							/* return to parent list		*/

			default:
				prefatal("unexpected character");
			}
			break;

		case TOKEN_INTEGER:						/* an integer					*/
			if (!lastnode)
				prefatal("Integer cannot be first item in clause");
												/* make a node for the int	*/
			lastnode = newnode = MakeNode(lastnode,parent);
			newnode->value.type = TYPE_INTEGER;
			newnode->value.v = (void *)tstate.numeric_val;
			next_token(&tstate);				/* skip over integer			*/
			break;

		case TOKEN_STRING:						/* a text string				*/
		case TOKEN_TEXT:
			lastnode = newnode = MakeNode(lastnode,parent);	/* make a node for the text	*/
			newnode->value.type = TYPE_STRING;
			newnode->value.v =					/* make a string				*/
				MakeString(tstate.text_val,tstate.numeric_val);
			next_token(&tstate);				/* skip over text string		*/
			break;

		case TOKEN_EOF:			fatal("Unexpected end of file");

		case TOKEN_SYMERR:
		case TOKEN_STRINGERR:	prefatal("Symbol or String too long");

		case TOKEN_TERMERR:
		case TOKEN_COMMERR:		prefatal("Unterminated string or comment");
		}
	}
}

	/* de-allocate all the memory used in the compiled program */

void de_compile (struct TreeNode *node)
{
	struct TreeNode *next;

	while (node)								/* while nodes left in this chain */
	{	next = node->next;

		switch (node->value.type) 
		{										/* switch by node type			*/
		case TYPE_STRING:						/* if it's a string				*/
			FreeString(node->value.v);			/* free the string				*/
			break;

		case TYPE_LIST:							/* if it's a list				*/
			de_compile(node->value.v);			/* free the list (recursively)	*/
			break;
		}

		FreeNode(node);							/* and de-allocate the node		*/
		node = next;
	}
}

/* ============================= Error Recovery ========================== */

extern WORD parse_debug = 1;

#if 0
	/* print fatal global error message */

void error (char *message)
{
	if (parse_debug) 
		Printf("ERROR: %s.\n",message);
	longjmp(env,1);
}
#endif

	/* print fatal error message caused by specific line */

char *compile_err;
long linenum;

void fatal(char *message)
{
	offending_line(1);
	compile_err = message;
	if (parse_debug) 
		Printf("ERROR: %s on line %ld.\n",message,linenum);
	longjmp(env,2);
}

	/* use this if you have an error before next_token is called */

void prefatal(char *message)
{
	offending_line(0);
	compile_err = message;
	if (parse_debug) 
		Printf("ERROR: %s on line %ld.\n",message,linenum);
	longjmp(env,2);
}

#if 0
	/* print warning message */

warn(char *message)
{
	offending_line(1);
	if (parse_debug) 
		Printf("WARNING: %s on line %ld.\n",message,linenum);
}
#endif

#define CODELENGTH	(sizeof inverse_on)

char inverse_on[]  = { 0x9b,'3','3',';','4','2','m' };
char inverse_off[] = { 0x9b,'3','1',';','4','0','m' };

/* print out the line that caused the error */

void offending_line (short n)
{
	BPTR				out;
	struct TokenLine	tl;

	if (!format_token_line(&tstate,&tl,n)) 
		return;

	linenum = tl.source_line;

	if (!(out = Output())) 
		return;

	/* REM: perhaps we should write an assy routine that formats the
		line for us, then write it out as one chunk... --DJ
	 *	even better would be to call Fprintf, since we have it!
	*/

	Write (out, tl.line_text,tl.token_start);
	Write (out, inverse_on,CODELENGTH);
	Write (out, tl.line_text + tl.token_start,tl.token_stop-tl.token_start);
	Write (out, inverse_off,CODELENGTH);
	Write (out, tl.line_text + tl.token_stop, tl.line_length - tl.token_stop);
	Write (out, "\n",1);
}
