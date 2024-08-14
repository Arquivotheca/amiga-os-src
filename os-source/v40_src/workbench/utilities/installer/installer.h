/* ========================================================================= *
 * installer.h - structure definitions for generic installer                 *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

/* #define __NO_PRAGMAS */

#ifndef EXEC_TYPES_H
#include <exec/lists.h>
#include "functions.h"
#endif

#include "xfunctions.h"

#include "lparse.h"
#include "symtab.h"

#ifdef DEBUG
#define debug_action(a)		debug_action(a)
#else
#define debug_action(a)
#endif

#define MAX_SYMBOL	32
#define MAX_STRING	512
#define MAX_TSTRING	10000
#define MAX_MACRO	128

	/* representation of a string */

struct String {
	UWORD				length;					/* length of string				*/
	/* followed by text */
};

#define STR_VAL(x)		((char *)( (struct String *)(x) + 1 ))

	/* value of node or symbol */

struct Value {
	UBYTE				type;					/* type of node					*/
	UBYTE				storage;				/* temporary or permanent		*/
	void				*v;						/* pointer to string, list, or num */
};

	/* an installer symbol */

struct ISymbol {
	struct GSymbol		sym;					/* a generic symbol				*/
	struct Value		value;					/* value of this symbol			*/
};

	/* a node in the compiled tree */

struct TreeNode {								/* a node in the parse tree		*/
	struct TreeNode		*next;					/* next node in the parse tree	*/
	UWORD				line;					/* line number (for errors		*/
	struct Value		value;					/* value of this node			*/
};

	/* types of nodes */

enum NodeTypes {
	TYPE_NONE=0,
	TYPE_INTEGER,
	TYPE_STRING,
	TYPE_SYMBOL,								/* nodes only					*/
	TYPE_LIST,									/* nodes only					*/
	TYPE_STATEMENT,								/* symbols only					*/
	TYPE_FUNCTION,								/* symbols only					*/
	TYPE_CONTROL,								/* symbols only					*/
	TYPE_PARAMETER,								/* symbols only					*/
	TYPE_MACRO,									/* symbols only					*/
	TYPE_USERFUNC
};

enum StoreTypes {
	STORE_TEMP,
	STORE_LITERAL,
	STORE_PERM
};

	/* a tool type */

struct ToolType {
	struct ToolType		*next;
	struct String		*tooltype;
	struct String		*toolval;
};

	/* installation parameters */

#define deftool_val default_val

struct InstallationRecord {
	UBYTE				InstallAction;			/* type of installation action	*/
	UBYTE				flags;
	BYTE				completion;
	BYTE				bits;
	struct Value		help_val,				/* text for help				*/
						prompt_val,				/* text for prompt				*/
						pattern_val,			/* text for search pattern		*/
						source_val,				/* source dir or file			*/
						dest_val,				/* dest dir or file				*/
						name_val,				/* new name of file				*/
						default_val;			/* default tool					*/
	struct MinList		text_list;				/* text to stuff to text file	*/
	struct ToolType		*tooltypes;				/* tooltypes to modify			*/
	struct String		**choices;				/* choice strings				*/
	LONG				minval,maxval;			/* minimum and maximum num vals */
	WORD				numchoices;				/* number of "choices"			*/
	UBYTE				delopts;				/* delete options				*/
	UBYTE				pad;
};

enum i_actions {
	ACTION_NONE,								/* no install action in progress */
	ACTION_NEWDIR,
	ACTION_FILES,
	ACTION_STARTEDIT,
	ACTION_TOOLTYPE,
	ACTION_ICON,
	ACTION_TEXTFILE,
	ACTION_OTHER,
	ACTION_DELETE,
	ACTION_RENAME,
	ACTION_ASKDIR,
	ACTION_ASKSTRING,
	ACTION_ASKNUM,
	ACTION_ASKCHOICE,
	ACTION_ASKFILE,
	ACTION_LIBS,
	ACTION_FUNC,								/* used for syntax checking		*/
	ACTION_ASKBOOL,
	ACTION_ASKDISK
};

/* flags for actions... */

#define	ACTIONF_NONE		(1L<<ACTION_NONE)
#define	ACTIONF_NEWDIR		(1L<<ACTION_NEWDIR)
#define	ACTIONF_FILES		(1L<<ACTION_FILES)
#define	ACTIONF_STARTEDIT	(1L<<ACTION_STARTEDIT)
#define	ACTIONF_TOOLTYPE	(1L<<ACTION_TOOLTYPE)
#define	ACTIONF_ICON		(1L<<ACTION_ICON)
#define	ACTIONF_TEXTFILE	(1L<<ACTION_TEXTFILE)
#define	ACTIONF_OTHER		(1L<<ACTION_OTHER)
#define	ACTIONF_DELETE		(1L<<ACTION_DELETE)
#define	ACTIONF_RENAME		(1L<<ACTION_RENAME)
#define	ACTIONF_ASKDIR		(1L<<ACTION_ASKDIR)
#define	ACTIONF_ASKSTRING	(1L<<ACTION_ASKSTRING)
#define	ACTIONF_ASKNUM		(1L<<ACTION_ASKNUM)
#define	ACTIONF_ASKCHOICE	(1L<<ACTION_ASKCHOICE)
#define	ACTIONF_ASKFILE		(1L<<ACTION_ASKFILE)
#define	ACTIONF_LIBS		(1L<<ACTION_LIBS)
#define	ACTIONF_FUNC		(1L<<ACTION_FUNC)
#define	ACTIONF_ASKBOOL		(1L<<ACTION_ASKBOOL)
#define	ACTIONF_ASKDISK		(1L<<ACTION_ASKDISK)

#define IFLAG_ALL		(1<<0)					/* "all" option					*/
#define IFLAG_CONFIRM	(1<<1)					/* "confirm" options			*/
#define IFLAG_INFOS		(1<<2)					/* "infos" option				*/
#define IFLAG_DDIR		(1<<3)					/* destdr as opposed to dest	*/
#define IFLAG_SAFE		(1<<4)					/* call even in pretend			*/
#define IFLAG_NODIRS	(1<<5)					/* "files" option				*/
#define IFLAG_OPTIONAL	(1<<6)					/* "optional" option			*/
#define IFLAG_FONTS		(1<<7)					/* don't show .font, but copy	*/

#define IBITS_NOPOSITION	(1<<0)				/* tooltype parameter			*/
#define IBITS_RESIDENT		(1<<1)				/* copylib/getversion parameter	*/
#define IBITS_NOGAUGE		(1<<2)				/* copy file with no gauge		*/
#define IBITS_DEBUG_ALL		(1<<7)				/* special debugging			*/

#define DELOPT_FAIL			(1<<0)
#define DELOPT_NOFAIL		(1<<1)
#define DELOPT_OKNODELETE	(1<<2)
#define DELOPT_FORCE		(1<<3)
#define DELOPT_ASKUSER		(1<<4)
#define DELOPT_OKNOSRC		(1<<5)

struct funcdef {
	char				*name;
	int					(*func)(struct Value *,WORD);
	LONG				syntax_check;
};

enum error_types {
	ERR_NONE=0,
	ERR_ABORT,
	ERR_MEMORY,
	ERR_SCRIPT,
	ERR_DOS,
	ERR_DATA,
	ERR_HANDLED,
	ERR_LAST
};

	/* structure to hold text file contents */

struct TextList {
	struct MinNode		node;					/* list node					*/
	long				size;					/* number of characters			*/
};

	/* user levels */

enum {
	USER_NOVICE=0,
	USER_AVERAGE,
	USER_EXPERT
};

#define PRETEND_MODE		(pretend_flag && !(istate.flags & IFLAG_SAFE))
#define P_PRETEND_MODE		(pretend_flag && !(ir->flags & IFLAG_SAFE))
#define NOVICE_MODE			(user_level == USER_NOVICE)
#define EXPERT_MODE			(user_level == USER_EXPERT)
#define NOCONFIRM			(user_level <= USER_AVERAGE)

extern struct funcdef
	statement_syms[],
	control_syms[],
	func_syms[],
	param_syms[];
extern char func_math_name[];

#define	FUNC_MAKEDIR	statement_syms[0].name
#define	FUNC_COPYFILES	statement_syms[1].name
#define	FUNC_COPYLIB	statement_syms[2].name
#define	FUNC_STARTUP	statement_syms[3].name
#define	FUNC_TOOLTYPE	statement_syms[4].name
#define	FUNC_TEXTFILE	statement_syms[5].name
#define	FUNC_EXECUTE	statement_syms[6].name
#define	FUNC_RUN		statement_syms[7].name
#define	FUNC_REXX		statement_syms[8].name
#define	FUNC_RENAME		statement_syms[9].name
#define	FUNC_DELETE		statement_syms[10].name
#define	FUNC_ABORT		statement_syms[11].name
#define	FUNC_EXIT		statement_syms[12].name
#define	FUNC_COMPLETE	statement_syms[13].name
#define	FUNC_MESSAGE	statement_syms[14].name
#define	FUNC_WORKING	statement_syms[15].name
#define	FUNC_SET		control_syms[0].name
#define	FUNC_IF			control_syms[1].name
#define	FUNC_WHILE		control_syms[2].name
#define	FUNC_UNTIL		control_syms[3].name
#define	FUNC_TRAP		control_syms[4].name
#define	FUNC_ONERROR	control_syms[5].name
#define	FUNC_FOREACH	control_syms[6].name
#define	FUNC_CAT		func_syms[0].name
#define	FUNC_TACKON		func_syms[1].name
#define	FUNC_FILEONLY	func_syms[2].name
#define	FUNC_PATHONLY	func_syms[3].name
#define	FUNC_ASKFILE	func_syms[4].name
#define	FUNC_ASKDIR		func_syms[5].name
#define	FUNC_ASKSTRING	func_syms[6].name
#define	FUNC_ASKNUMBER	func_syms[7].name
#define	FUNC_ASKCHOICE	func_syms[8].name
#define	FUNC_ASKOPTIONS	func_syms[9].name
#define	FUNC_ASKBOOL	func_syms[10].name
#define	FUNC_ASKDISK	func_syms[11].name
#define	FUNC_EXISTS		func_syms[12].name
#define	FUNC_EARLIER	func_syms[13].name
#define	FUNC_GETSIZE	func_syms[14].name
#define	FUNC_GETDISKSPACE	func_syms[15].name
#define	FUNC_GETSUM		func_syms[16].name
#define	FUNC_GETVERSION	func_syms[17].name
#define	FUNC_GETASSIGN	func_syms[18].name
#define	FUNC_MAKEASSIGN	func_syms[19].name
#define	FUNC_GETENV		func_syms[20].name
#define	FUNC_SELECT		func_syms[21].name
#define FUNC_DEBUG		func_debug_name
#define	FUNC_MATH		func_math_name
#define FUNC_TRANS		func_syms[38].name
#define FUNC_PROTECT	func_syms[39].name
#define FUNC_USER		func_syms[40].name
#define FUNC_SHIFTLEFT	func_syms[41].name
#define FUNC_SHIFTRIGHT	func_syms[42].name
#define FUNC_SUBSTR		func_syms[43].name
#define FUNC_EXPANDPATH	func_syms[44].name
#define FUNC_DATABASE	func_syms[45].name
#define FUNC_DELOPTS	func_syms[46].name
#define FUNC_STRLEN		func_syms[47].name
#define FUNC_PATMATCH	func_syms[48].name
#define FUNC_GETDEVICE	func_syms[49].name
#define FUNC_HELP		param_syms[0].name
#define FUNC_PROMPT		param_syms[1].name
#define FUNC_CHOICES	param_syms[2].name
#define FUNC_PATTERN	param_syms[3].name
#define FUNC_ALL		param_syms[4].name
#define FUNC_SOURCE		param_syms[5].name
#define FUNC_DEST		param_syms[6].name
#define FUNC_NEWNAME	param_syms[7].name
#define FUNC_CONFIRM	param_syms[8].name
#define FUNC_SAFE		param_syms[9].name
#define FUNC_INFOS		param_syms[10].name
#define FUNC_SETTOOLTYPE	param_syms[11].name
#define FUNC_SETDEFAULTTOOL	param_syms[12].name
#define FUNC_SETSTACK		param_syms[13].name
#define FUNC_NOPOSITION	param_syms[14].name
#define FUNC_DEFAULT	param_syms[15].name
#define FUNC_RANGE		param_syms[16].name
#define FUNC_COMMAND	param_syms[17].name
#define FUNC_APPEND		param_syms[18].name
#define FUNC_FILES		param_syms[19].name
#define FUNC_INCLUDE	param_syms[20].name
#define FUNC_OPTIONAL	param_syms[21].name
#define FUNC_FONTS		param_syms[22].name
#define FUNC_DISK		param_syms[23].name

#include "window.h"

	/* include prototype declaration file */

#include "installer.p"
