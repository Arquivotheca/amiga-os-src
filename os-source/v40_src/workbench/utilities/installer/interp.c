/* ========================================================================= *
 * Interp.c - interprets the install template tree                           *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include "functions.h"
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>

#include "installer.h"
#include "wild.h"

#include "macros.h"
#include "xfunctions.h"

extern struct InstallationRecord istate;

int VSprintf(char *,char *,void *);

extern UWORD pretend_flag, user_level;

extern char	*global_error_msg,
			*string_length_msg,
			*cannot_examine_msg,
			*noexist_msg,
			*prob_with_file,
			*cannot_assign_msg,
			*divide_zero_msg,
			*bad_cat_msg,
			*err_memory_msg,
			*user_abort_msg,
			*script_line_msg1,
			*script_line_msg2;

struct Value			vstack[256],
						*vtop = vstack,
						*vmax = vstack+256,
						*vmin = vstack;

LONG					spr_args[32];
char					format_string[MAX_STRING];

UBYTE 	ierror = 0,
		in_trap = 0;

jmp_buf	exit_buf;

struct TreeNode *ehead,
				*err_head = NULL;

struct String *TempString(char *text, WORD length, struct Value *v)
{	struct String *s;

	if (s = AllocMem(length+3,0L))				/* get string, plus size + null	*/
	{	s->length = length;						/* set length					*/
		if (text) CopyMem(text,s+1,length);		/* copy string					*/
		((UBYTE *)s)[length+2] = '\0';			/* null terminate				*/
		if (v)
		{
			v->v = s;
			v->type = TYPE_STRING;
			v->storage = STORE_TEMP;
		}
	}
	else memerr();								/* else error					*/
	return s;									/* return the string			*/
}

/* =============================== Evaluator ============================ */

void doroot(struct TreeNode *head)
{
	NewList((void *)&istate.text_list);

	if (setjmp(exit_buf) == 0) doseqn(head);	/* interpret rest of list		*/

	if (ierror)
	{	popall(vmin);							/* changed by --DJ */
		if (err_head)
		{
			ierror = 0;
			transcript(global_error_msg);
			doseqn(err_head);
			popall(vmin);						/* changed by --DJ */
			ierror = ERR_HANDLED;			/* error was "trapped" (onerror)	*/
		}
	}
}

void interpret(struct TreeNode *head)
{	struct TreeNode *pc;
	struct Value	*op;
	struct funcdef *f;

	if (ierror) return;

		/* if it's a symbol, then dereference it */

	if (head->value.type == TYPE_SYMBOL)		/* if node type is symbol		*/
	{	struct ISymbol *isym;

		isym = head->value.v;					/* dereference symbol			*/
		op = &isym->value;						/* get real value of list head	*/

		if (op->type == TYPE_LIST)
		{
			head = (struct TreeNode *)op->v;
		}
	}
	else op = &head->value;						/* value of list head (lit)		*/

		/* op now contains the list head */

	if (op->type == TYPE_INTEGER)
	{	err(head,ERR_SCRIPT,"Can't execute an integer");	/* ierror, list op can't be int. */
		return;
	}
	else if (op->type == TYPE_LIST)				/* if it's a list				*/
	{	doseq(head);
	}
	else if (op->type == TYPE_STRING)
	{	struct String *s = op->v;				/* either lit or sym, so no vfree */
		short i=0;

		op = vtop;
												/* for each argument			*/
		for (pc = head->next; pc; pc = pc->next)
		{
			if (i > 32)
			{	err_msg(head,ERR_SCRIPT,"String Fmt","Too many arguments");
				return;
			}

			eval(pc);							/* evaluate the argument		*/
												/* convert to RawDoFmt arg		*/
			vtop--;
			if (vtop->type == TYPE_INTEGER)
				spr_args[i++] = (LONG)vtop->v;
			else if (vtop->type == TYPE_STRING)
				spr_args[i++] = (LONG)STR_VAL(vtop->v);
			else spr_args[i++] = 0;				/* a NULL arg					*/
			vtop++;

		}

		VSprintf(format_string,STR_VAL(s),spr_args);
		i = strlen(format_string);
		if (i > 512)
		{	err_msg(head,ERR_DATA,"String Fmt",string_length_msg);
			return;
		}

		popall(op);

		TempString(format_string,i,vtop);
		vtop++;
	}
	else if (op->type == TYPE_CONTROL)
	{	f = op->v;
		ehead = head;
		if (f->func && !ierror) (f->func)((struct Value *)head,0);
	}
	else if (op->type == TYPE_STATEMENT)
	{	struct Value *v = vtop;

		if (istate.InstallAction)
		{	err(head,ERR_SCRIPT,"Install statements nested");
			return;
		}

		f = op->v;
		istate.InstallAction = f->syntax_check;
		istate.flags = istate.bits = 0;
		istate.minval = 0;
		istate.maxval = 0x7fffffff;
		NewList((void *)&istate.text_list);

		for (pc = head->next; pc; pc = pc->next) eval(pc);

		ehead = head;
		if (f->func && !ierror)
		{
			(f->func)(v,vtop - v);
		}

		cleanup_irecord();
	}
	else if (op->type == TYPE_FUNCTION || op->type == TYPE_PARAMETER)
	{	struct Value *v = vtop;
		struct InstallationRecord saved_record;

		f = op->v;

		if (f->syntax_check && op->type == TYPE_FUNCTION)
		{	saved_record = istate;
			zero(&istate,sizeof istate);
			NewList((void *)&istate.text_list);

			istate.InstallAction = f->syntax_check;
			istate.maxval = 0x7fffffff;
		}

		for (pc = head->next; pc; pc = pc->next) eval(pc);
		ehead = head;
		if (f->func && !ierror)
		{
			(f->func)(v,vtop - v);
		}

		if (f->syntax_check && op->type == TYPE_FUNCTION)
		{	cleanup_irecord();
			istate = saved_record;
		}
	}
	else if (op->type == TYPE_NONE)
	{
		err_msg(head,ERR_DATA,"Interpreter","Executing non-function");		
	}
}

void eval(struct TreeNode *head)
{	BOOL check_escape();
	struct ISymbol *isym;
	struct Value *v;

	if (!head)
	{	vtop->type = TYPE_NONE;
		vtop++;
		return;
	}

	if (ierror) return;

	if (check_escape() && aborterr("EVAL")) return;

	v = &head->value;

	if (v->type == TYPE_LIST)					/* if it's a list				*/
		interpret(v->v);						/* then interpret as code		*/
	else if (v->type == TYPE_SYMBOL)			/* a symbol						*/
	{	isym = v->v;							/* get the symbol				*/
#if 0
		if (isym->value.type == TYPE_NONE)
		{	err(head,ERR_SCRIPT,"EVAL: symbol '%s' has no value",isym+1);
			return;
		}
#endif
		*vtop = isym->value;					/* push symbol data onto stack	*/

		/* we want to dup the string here because, since it's going on the stack, we
			know it will get consumed somehow.
		*/

		dupstring(vtop);
		vtop++;
	}
	else										/* a literal					*/
	{	*vtop++ = *v;							/* structure copy value to stack */
	}

	if (vtop >= vmax)							/* check the stack				*/
		err_msg(head,ERR_SCRIPT,"EVAL","Stack too deep");
}

	/* evaluate a conditional */

test(struct TreeNode *head)
{	struct String *s;

	eval(head);
	if (ierror) return FALSE;
	vtop--;
	if (vtop->type == TYPE_NONE) return FALSE;
	if (vtop->type == TYPE_INTEGER) return (vtop->v != 0L);
	if (vtop->type != TYPE_STRING) return TRUE;

	s = vtop->v;
	if (s->length)
	{	vfree(vtop);
		return TRUE;
	}
	vfree(vtop);
	return FALSE;
}

void vfree(struct Value *v)
{	if (v->type == TYPE_STRING && v->storage == STORE_TEMP)
	{
		FreeString(v->v);
		v->v = NULL;
	}
	v->type = TYPE_NONE;
}

void pop(void)
{	if (vtop > vmax)
	{
		Printf("Stack Overflow!!\n");
		memerr();
	}
	if (vtop < vmin)
	{
		Printf("Stack Underflow!!\n");
		memerr();
	}
	if (vtop > vmin)
	{	vtop--;
		vfree(vtop);
	}
}

long as_integer(struct Value *v)
{	long	a_to_x(char *);
	long	a_to_b(char *);
	char	*str;

	if (v->type == TYPE_INTEGER) return (long)v->v;
	if (v->type != TYPE_STRING) return 0L;

	str = STR_VAL(v->v);

	if (*str == '$') return a_to_x(str + 1);
	if (*str == '%') return a_to_b(str + 1);
	return atol(str);
}

int integer_to_string(struct Value *v)
{	char		digits[14];
	long		val;

	if (v->type == TYPE_STRING) return 1;

	val = (v->type == TYPE_INTEGER ? (long)v->v : 0);

	Sprintf(digits,"%ld",val);
	return (TempString(digits,strlen(digits),v) != NULL);
}

void dupstring(struct Value *v)
{
	if (v->type == TYPE_STRING)				/* make a copy of the string	*/
	{	struct String *s = (struct String *)v->v;

		if (s == NULL) TempString(NULL,0,v);
		else TempString((char *)(s+1),s->length,v);
	}
}

/* ============================= Control Statements ========================== */

void doseq(struct TreeNode *head)
{	while (!ierror)								/* until list end				*/
	{	eval(head);								/* evaluate the node			*/
		head = head->next;						/* get next node				*/
		if (head) pop();						/* if there is a next node, pop v */
		else break;								/* else return with val on stack */
	}
}

void doseqn(struct TreeNode *head)
{	while (head && !ierror)						/* until list end				*/
	{	eval(head);								/* evaluate the node			*/
		pop();									/* if there is a next node, pop v */
		head = head->next;						/* get next node				*/
	}
}

void doif(struct TreeNode *head)				/* an IF clause					*/
{	struct Value	*oldtop = vtop;

	head = head->next;							/* get test condition			*/
	if (!test(head)) head = head->next;			/* if false, skip TRUE clause	*/
	if (ierror) return;
	if (head) head = head->next;				/* skip 1 clause anyway			*/
	eval(head);									/* evaluate a clause			*/

	if (vtop == oldtop) eval(0L);
}

void doselect(struct TreeNode *head)			/* a SELECT clause				*/
{	struct Value	*oldtop = vtop;
	long			n;

	head = head->next;							/* get test condition			*/

	eval(head);									/* calc selection value			*/
	if (ierror) return;

	vtop--;
	if (vtop->type != TYPE_INTEGER)				/* must be an integer			*/
	{
		err_msg(ehead,ERR_SCRIPT,FUNC_SELECT,"Integer argument expected");
	}

	n = (long)vtop->v;							/* get selection value			*/

	if (n < 0) 									/* must not be negative			*/
	{
bad:	err_msg(ehead,ERR_SCRIPT,FUNC_SELECT,"Selection value out of range");			
	}
	else
	{
		while (n-- >= 0)						/* go thru n+1 nodes			*/
		{
			unless (head = head->next) goto bad;
		}

		eval(head);								/* evaluate selected node		*/ 
	}

	if (vtop == oldtop) eval(0L);
}

void dowhile(struct TreeNode *head)				/* a WHILE statement			*/
{
	head = head->next;							/* skip WHILE keyword			*/
	while (test(head) && !ierror)				/* test condition				*/
		doseqn(head->next);						/* do loop body					*/
	eval(0L);									/* push a NIL for result		*/
}

void dountil(struct TreeNode *head)				/* an UNTIL statement			*/
{	head = head->next;							/* skip UNTIL keyword			*/
	while (!test(head) && !ierror)				/* test condition				*/
		doseqn(head->next);						/* do loop body					*/
	eval(0L);									/* push a NIL for result		*/
}

struct Value *each_string(struct TreeNode *head)
{
	eval(head);
	if (ierror) return NULL;
	if (!verify_string(--vtop,FUNC_FOREACH)) return NULL;
	vtop->type = TYPE_NONE;
	return vtop;
}

void do_foreach(struct TreeNode *head)			/* a FOREACH statement			*/
{
	LONG			i,
					total;
	struct Value	*v,
					theFile,
					theDir;
	struct String	**files = NULL;
	BPTR			lock = NULL;

	unless (v = each_string(head = head->next)) return;
	theDir = *v;
	theDir.type = TYPE_STRING;
	unless (v = each_string(head = head->next)) { vfree(&theDir); goto bye; }
	theFile = *v;
	theFile.type = TYPE_STRING;

	unless ( lock = Lock(STR_VAL(theDir.v),ACCESS_READ) )
	{
		err_msg(head,ERR_DOS,FUNC_FOREACH,cannot_examine_msg);
		goto bye;
	}

	unless (files =
		match_files(lock,STR_VAL(theFile.v),MATCH_FILES_DIRS,&total))
	{
		memerr();
		goto bye;
	}

	UnLock(lock);
	lock = NULL;

	for (i=0;i<total && !ierror;i++)
	{	extern void **foreach_type,**foreach_name;

		*foreach_type = (void *)get_dos_type(files[i]);
		*foreach_name = files[i];
		doseqn(head->next);							/* do loop body					*/
	}

bye:
	if (lock) UnLock(lock);
	if (files) free_matches(files);
	vfree(&theDir);
	vfree(&theFile);
	eval(0L);									/* push a NIL for result		*/
}

void dotrap(struct TreeNode *head)				/* a TRAP statement				*/
{	struct Value	*vsave = vtop;				/* save stack pointer			*/
	UBYTE			tsave = in_trap;			/* save trap level				*/

	in_trap++;									/* increment trap level			*/
	doseq(head->next);							/* interpret rest of list		*/

	if (ierror)
	{	while (vtop > vsave) pop();				/* if ierror, restore stack		*/
#ifdef DEBUG
		Printf("Trapped at %ld\n\n",head->line);
#endif
	}

	vtop->type = TYPE_INTEGER;					/* push ierror code				*/
	vtop->storage = STORE_TEMP;
	vtop->v = (void *)ierror;
	vtop++;

	ierror = FALSE;								/* reset ierror code			*/
	in_trap = tsave;							/* reset trap level				*/
}

void doset(struct TreeNode *head)				/* a SET statement				*/
{	struct ISymbol	*isym;						/* a symbol entry				*/
	WORD			once = FALSE;

	while ((head = head->next) && !ierror)		/* if a var to set				*/
	{
		if (head->value.type != TYPE_SYMBOL)	/* check if it's a symbol		*/
		{	err_msg(head,ERR_SCRIPT,FUNC_SET,"Symbol name expected");
			return;
		}

			/* check to make sure we don't overwrite commands! */
			/* NOTE: Can't ever happen... */

#if 0
		if (head->value.type >= TYPE_STATEMENT)	/* check if it's a keyword		*/
		{	err(head,ERR_SCRIPT,"SET: Invalid symbol name");
			return;
		}
#endif

		once = TRUE;

		isym = head->value.v;					/* get the symbol address		*/
		head = head->next;
		eval(head);								/* evaluate expression			*/
		vtop--;									/* pop the stack				*/

		if (*(char *)(isym+1) == '@')			/* if assignment to '@' variable */
		{
			if (isym->value.type != vtop->type && isym->value.type != TYPE_NONE)
			{
				err(head,ERR_SCRIPT,"SET: Can't change type of an '@' symbol");
				vtop++;
				return;
			}
		}

		/* here, we only want to dupstring if it's not a temp. */
		if (vtop->storage != STORE_TEMP)		/* if not already a temp		*/
			dupstring(vtop);					/* then make it a temp			*/

		vfree(&isym->value);					/* free old contents			*/
		isym->value = *vtop;					/* move stack top to symbol		*/

		if (!head) break;
	}


	if (once)
	{
		dupstring(vtop);						/* if string, dup it			*/
		vtop++;									/* unpop the stack				*/
	}
	else eval(0L);
}

void doproc(struct TreeNode *head)				/* a PROC statement				*/
{	struct ISymbol	*isym;						/* a symbol entry				*/

	head = head->next;

	if (head->value.type != TYPE_SYMBOL)	/* check if it's a symbol		*/
	{
		err_msg(head,ERR_SCRIPT,"PROC","Symbol name expected");
		return;
	}

	/* check to make sure we don't overwrite commands! */
	/* NOTE: Can't ever happen... */

#if 0
	if (head->value.type >= TYPE_STATEMENT)	/* check if it's a keyword		*/
	{
		err(head,ERR_SCRIPT,"SET: Invalid symbol name");
		return;
	}
#endif

	isym = head->value.v;					/* get the symbol address		*/
	isym->value.type = TYPE_LIST;
	isym->value.v = (void *)head->next;

	eval(0L);									/* push a NIL for result		*/
}

void do_onerror(struct TreeNode *head)			/* an onerror clause					*/
{
	err_head = head->next;						/* save onerr statements */
	eval(0L);									/* push a NIL for result		*/
}

/* ================================= Functions ============================== */

	/* verify that the given value is a string, else return an error */

verify_string(struct Value *v, char *func)
{
	if (v->type != TYPE_STRING)
	{
		err(ehead,ERR_SCRIPT,"%s: String argument expected",func);
		return 0;
	}
	return 1;
}

/* compare two strings... */

scmp(struct Value *v1, struct Value *v2)
{	struct String *s1, *s2;

	s1 = v1->v;
	s2 = v2->v;

	return strcmp((char *)(s1+1),(char *)(s2+1));
}

	/* may need a special version of DO_CAT that takes the funcname as a param */

void do_cat_sub(struct Value *vbase, WORD args,char *between,char *caller)
{	WORD				i, b = strlen(between);
	struct Value		*v, t;
	struct String		*s, *ns;
	UBYTE				*cptr;
	LONG				total_length = 0;

	for (i=0; i<args; i++)
	{	v = vbase + i;
		if (v->type == TYPE_NONE) continue;
		if (v->type == TYPE_INTEGER) integer_to_string(v);
		if (!verify_string(v,caller)) return;
		if (v->type == TYPE_STRING)
		{	s = v->v;
			if (total_length) total_length += b;
			total_length += s->length;
		}
	}

	if (total_length > MAX_TSTRING)
	{	err(ehead,ERR_DATA,bad_cat_msg,caller);
		return;
	}

	if (!(ns = TempString(NULL,total_length,&t))) return;

	cptr = (UBYTE *)(ns + 1);

	for (i=0; i<args; i++)
	{	v = vbase + i;
		if (v->type == TYPE_STRING)
		{	s = v->v;
			if (cptr != (UBYTE *)(ns + 1))
			{	CopyMem(between,cptr,b);
				cptr += b;
			}
			CopyMem(s+1,cptr,s->length);
			cptr += s->length;
		}
		/* add concat of numbers too. */
		vfree(v);
	}

	*vbase = t;
	vtop = vbase + 1;
}

void do_cat(struct Value *vbase, WORD args)
{
	do_cat_sub(vbase,args,"",FUNC_CAT);
}

void do_tackon(struct Value *vbase, WORD args)
{	struct Value		*v1, *v2;
	struct String		*s1, *s2;

		/* make sure there are two arguments */

	if (argcheck(args,2,2,FUNC_TACKON)) return;

	v1 = vbase; v2 = vbase+1;

	if (!verify_string(v1,FUNC_TACKON)) return;
	if (!verify_string(v2,FUNC_TACKON)) return;

	s1 = v1->v;
	s2 = v2->v;

	if (s1->length + s2->length + 1 > MAX_STRING)
	{	err_msg(ehead,ERR_DATA,FUNC_TACKON,string_length_msg);
		return;
	}

	strcpy(format_string,STR_VAL(s1));
	TackOn(format_string,STR_VAL(s2),MAX_STRING);

	popall(vbase);								/* delete old strings		*/
												/* make new string			*/
	if (!TempString(format_string,strlen(format_string),vbase))
		return;
	vtop = vbase + 1;
}

void do_fileonly(struct Value *vbase, WORD args)
{	struct String		*s;
	char				*filepart;
	struct Value		t;

		/* make sure there are two arguments */

	if (argcheck(args,1,1,FUNC_FILEONLY)) return;
	if (!verify_string(vbase,FUNC_FILEONLY)) return;

	s = vbase->v;

	filepart = FileOnly((char *)(s+1));

	t = *vbase;
	if (TempString(filepart,strlen(filepart),vbase)) vfree(&t);
}

void do_pathonly(struct Value *vbase, WORD args)
{	struct String		*s;
	char				*filepart, *pathpart;
	struct Value		t;

		/* make sure there are two arguments */

	if (argcheck(args,1,1,FUNC_PATHONLY)) return;
	if (!verify_string(vbase,FUNC_PATHONLY)) return;

	s = vbase->v;

	pathpart = (char *)(s+1);
	filepart = FileOnly(pathpart);
	if (filepart > pathpart && filepart[-1] == '/') filepart--;

	t = *vbase;
	if (TempString(pathpart,filepart - pathpart,vbase)) vfree(&t);
}

void do_exists(struct Value *vbase, WORD args)
{	BPTR					lock;
	struct FileInfoBlock	*fib;
	struct Process			*pr;
	APTR					prwin;

	if (argcheck(args,1,1,FUNC_EXISTS)) return;
	if (!verify_string(vbase,FUNC_EXISTS)) return;

	if (istate.bits & IBITS_RESIDENT)
	{
		pr = (struct Process *)FindTask(NULL);
		prwin = pr->pr_WindowPtr;
		pr->pr_WindowPtr = (APTR)-1;
	}

	if (lock = Lock(STR_VAL(vbase->v),ACCESS_READ))
	{
		if ( (fib = AllocMem(sizeof *fib,MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		{
			memerr();
			UnLock(lock);
			return;
		}

		if (Examine(lock,fib) == 0)
		{
			err_msg(ehead,ERR_DOS,FUNC_EXISTS,cannot_examine_msg);
			UnLock(lock);
			FreeMem(fib, sizeof *fib);
			return;
		}

		UnLock(lock);
		popall(vbase);
		vbase->type = TYPE_INTEGER;
		vbase->v = (void *)(fib->fib_DirEntryType < 0 ? 1L : 2L);
		vtop = vbase+1;

		FreeMem(fib, sizeof *fib);
	}
	else
	{	popall(vbase);
		vbase->type = TYPE_INTEGER;
		vbase->v = (void *)0;
		vtop = vbase+1;
	}

	if (istate.bits & IBITS_RESIDENT) pr->pr_WindowPtr = prwin;
}

WORD compare_dates(struct DateStamp *date1,struct DateStamp *date2)
{
	if (date1->ds_Days < date2->ds_Days) return -1;
	if (date1->ds_Days > date2->ds_Days) return 1;
	if (date1->ds_Minute < date2->ds_Minute) return -1;
	if (date1->ds_Minute > date2->ds_Minute) return 1;
	if (date1->ds_Tick < date2->ds_Tick) return -1;
	if (date1->ds_Tick > date2->ds_Tick) return 1;
	return 0;
}

void do_earlier(struct Value *vbase, WORD args)
{	struct DateStamp	date1,
						date2;

	if (argcheck(args,2,2,FUNC_EARLIER)) return;
	if (!verify_string(vbase,FUNC_EARLIER)) return;

	if ( !( GetFileDate(STR_VAL(vbase->v),&date1) &&
		GetFileDate(STR_VAL(vbase[1].v),&date2) ) )
	{
		err_msg(ehead,ERR_DATA,FUNC_EARLIER,noexist_msg);
		return;
	}

	if (compare_dates(&date1,&date2) < 0)
	{	popall(vbase);
		vbase->type = TYPE_INTEGER;
		vbase->v = (void *)1;
		vtop = vbase+1;
	}
	else
	{	popall(vbase);
		vbase->type = TYPE_INTEGER;
		vbase->v = (void *)0;
		vtop = vbase+1;
	}
}

void do_getsize(struct Value *vbase, WORD args)
{	long result;

	if (argcheck(args,1,1,FUNC_GETSIZE)) return;
	if (!verify_string(vbase,FUNC_GETSIZE)) return;

	result = GetFileSize((char *)((struct String *)vbase->v + 1));

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase+1;
}

void do_getdiskspace(struct Value *vbase, WORD args)
{	long GetDiskSpace(char *name);
	long result;

	if (argcheck(args,1,1,FUNC_GETDISKSPACE)) return;
	if (!verify_string(vbase,FUNC_GETDISKSPACE)) return;

	result = GetDiskSpace((char *)((struct String *)vbase->v + 1));

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase+1;
}

BOOL GetChecksum(char *name,long *sum)
{	BPTR			fh = Open(name,MODE_OLDFILE);
	long			actual,
					nval;
	BOOL			result = 0;

	*sum = 0;

	if (fh != NULL)
	{
		do
		{	nval = 0;
			actual = Read(fh,&nval,4L);
			if (actual < 0) goto bad;
			if (actual > 0) *sum += nval;
		} while	(actual == 4);

		Close(fh);
		result = 1;
	}
	else
	{
bad:	err_msg(ehead,ERR_DOS,FUNC_GETSUM,prob_with_file);
	}

	return result;
}

void do_getsum(struct Value *vbase, WORD args)
{	long	result;

	if (argcheck(args,1,1,FUNC_GETSUM)) return;
	if (!verify_string(vbase,FUNC_GETSUM)) return;

		/* get file checksum */
	GetChecksum((char *)((struct String *)vbase->v + 1),&result);

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase+1;
}

/*	GetFileVersion() returns a fixed point number in the form version.revision
	where the high word has the version number.									*/

/*	note: "narrator.device" had NO romtag!! */

long GetFileVersion(char *name)
{	static char		verstr[] = "$VER: ";
	BPTR			fh = Open(name,MODE_OLDFILE);
	struct Resident *res = NULL;
	long			t, v, size,
					version = 0;
	char			*mem,
					*seg = NULL,
					*buf,*c,
					*ver_at,
					ch;

	if (fh == NULL)
	{	/* error -- what to do??? */
		return 0;
	}

/*	There are different ways to have a 'version':
		1) The new 2.0 embedded version info.
			dc.b	'$VER: programname 36.10 (date)'
		2) The id string in a library or device.
			dc.b	'libname 36.10 (date)',13,10,0
			Note that the version number should match rt_Version.
		3) if has RomTag, call InitStruct
*/

		/* look for 2.0-style embedded version info */
	mem = AllocMem(1024,0);					/* get some memory */
	if (mem == NULL) goto done;

	ver_at = verstr;						/* point at start of template */

	while (size = Read(fh,mem,1024))		/* read some of file (if any) */
	{	if (size < 0) goto done;			/* if error, exit */

		buf = mem;							/* init tracking pointer */
		while (size--)
		{	if (*buf++ == *ver_at++)		/* does character match template? */
			{	do
				{
					if (*ver_at == '\0')	/* if we hit end of template... */
					{
							/* possible match */
						Seek(fh,-size,OFFSET_CURRENT);	/* seek back */
						size = Read(fh,mem,1024);		/* read in data */
						if (size <= 0) goto done;		/* if error, exit */

						t = size;			/* init temp size & tracking pointer */
						buf = mem;
						while (t--) if ((ch = *buf++) == '\0' || ch == '\n')
						{		/* found zero at end of (possible) version string */
							while (mem != --buf)
							{	if (*buf == ')')
								{		/* skip over (date) */
									while (mem != --buf) if (*buf == '(') break;
									if (--buf <= mem) break;
								}
								else if (*buf == ' ')
								{		/* found space before (possible) version # */
									v = atol(++buf) << 16;	/* get version # */
									while ((ch = *buf++) != '\0' && ch != '\n')
										if (*buf == '.')
									{		/* found dot before revision # */
										version = v + atol(buf+1);
										goto done;		/* got it!!! */
									}
									break;
								}
							}
							break;
						}
						buf = mem;			/* failed match, continue loop */
						goto loop1;
					}
					if (*buf != *ver_at) break;
					buf++;
					ver_at++;
				} while (size--);
			}
loop1:		ver_at = verstr;
		}
	}

not2_0:
	FreeMem(mem,1024);
	mem = NULL;
	Close(fh);
	fh = NULL;

	if (seg = (char *)LoadSeg(name))
	{
			/* first, find RomTag... if not there, exit */
		buf = (char *)((long)seg << 2);
		t = *(long *)(buf - 4) << 1;
		buf += 4;
		for (v=0;v<t;v++,buf+=2)
		{
			if (*(UWORD *)buf == RTC_MATCHWORD && *(char **)(buf + 2) == buf)
			{
				res = (struct Resident *)buf;
				break;
			}
		}
		if (res == NULL) goto done;

			/* got a RomTag, so check id string */
		buf = res->rt_IdString;
		if (buf != NULL)
		{	buf += strlen(buf);
				/* search backwards in id string for version info */
			c = res->rt_IdString;
			while (--buf != c)
			{	if (*buf == ')')
				{		/* skip over (date) */
					while (c != --buf) if (*buf == '(') break;
					if (--buf <= c) break;
				}
 				else if (*buf == '.' && isdigit(buf[1]) && isdigit(buf[-1]))
				{	v = atol(buf+1);
					while (--buf != c) if (!isdigit(*buf))
					{	t = atol(buf+1);
						if (t != res->rt_Version)
						{	/* HEY, WHAT GIVES??? */ ;
						}
						version = v + (t << 16);
						goto done;
					}
				}
			}
		}

		/* method 3: InitStruct */

		if (res->rt_Flags & RTF_AUTOINIT)
		{	LONG			*table = (LONG *)res->rt_Init;
			struct Library	*base;

			if (base = AllocMem(table[0],0))
			{
				InitStruct((void *)table[2],(APTR)base,table[0]);
				version = base->lib_Version << 16 | base->lib_Revision;
				FreeMem(base,table[0]);
			}
		}

		/* total failure */
	}

done:
	if (seg) UnLoadSeg((long)seg);
	if (mem) FreeMem(mem,1024);
	if (fh) Close(fh);

	return version;		/* return version number or error (0) */
}

void do_getversion(struct Value *vbase, WORD args)
{	extern struct ExecBase *SysBase;
	long			result = 0;
	char			*name;
	struct Library	*lib;

	if (argcheck(args,0,1,FUNC_GETVERSION)) return;

	if (args == 1)	/* get version of a file */
	{
		if (!verify_string(vbase,FUNC_GETVERSION)) return;

		name = STR_VAL((struct String *)vbase->v);

		if (istate.bits & IBITS_RESIDENT)
		{
			unless (lib = (struct Library *)FindName(&SysBase->LibList,name))
				lib = (struct Library *)FindName(&SysBase->DeviceList,name);

			if (lib != NULL)
				result = lib->lib_Version << 16 | lib->lib_Revision;
		}
		else result = GetFileVersion(name);
	}
	else			/* get version of OS */
	{	extern struct ExecBase *SysBase;

		result = (SysBase->LibNode.lib_Version << 16) +
			SysBase->LibNode.lib_Revision;
	}

more:
	popall(vbase);
	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase + 1;
}

void do_expandpath(struct Value *vbase, WORD args)
{	LONG	result = 0;
	BPTR	lock;

	if (argcheck(args,1,1,FUNC_EXPANDPATH)) return;
	if (!verify_string(vbase,FUNC_EXPANDPATH)) return;

	if (lock = Lock(STR_VAL(vbase->v),ACCESS_READ))
	{
		result = ExpandPath ((void *)lock,format_string,MAX_STRING);
		UnLock(lock);	
	}

	popall(vbase);

	if ( result )
	{
		TempString(format_string,strlen(format_string),vbase);
	}
	else
	{
		TempString("",0,vbase);
	}

	vtop = vbase + 1;
}

#ifndef GFXF_AA_ALICE
#define GFXF_AA_ALICE	4
#define GFXF_AA_LISA	8
#endif

void do_database(struct Value *vbase, WORD args)
{	extern struct ExecBase *SysBase;
	extern struct GfxBase *GfxBase;
	char	*result = "unknown",
			*what;

	if (argcheck(args,1,1,FUNC_DATABASE)) return;
	if (!verify_string(vbase,FUNC_DATABASE)) return;

	what = STR_VAL(vbase->v);

	if (!stricmp(what,"vblank"))
	{
		Sprintf(result = format_string,"%ld",SysBase->VBlankFrequency);
	}
	else if (!stricmp(what,"cpu"))
	{
		if (SysBase->AttnFlags & AFF_68040) result = "68040";
		else if (SysBase->AttnFlags & AFF_68030) result = "68030";
		else if (SysBase->AttnFlags & AFF_68020) result = "68020";
		else if (SysBase->AttnFlags & AFF_68010) result = "68010";
		else result = "68000";
	}
	else if (!stricmp(what,"graphics-mem"))
	{
		Sprintf(result = format_string,"%ld",AvailMem(MEMF_CHIP));
	}
	else if (!stricmp(what,"total-mem"))
	{
		Sprintf(result = format_string,"%ld",AvailMem(0L));
	}
#ifdef ONLY2_0
	else if (!stricmp(what,"chiprev"))
	{
		if (GfxBase->ChipRevBits0 & GFXF_AA_LISA) result = "AA";
		else if (GfxBase->ChipRevBits0 & GFXF_HR_DENISE) result = "ECS";
		else result = "AGNUS";
	}
#endif

	popall(vbase);
	TempString(result,strlen(result),vbase);
	vtop = vbase + 1;
}

void do_getassign(struct Value *vbase, WORD args)
{	BOOL GetAssign(char *name,char *buffer,LONG opts);
	LONG			opts = LDF_ASSIGNS,
					result;
	char			*s;

	if (argcheck(args,1,2,FUNC_GETASSIGN)) return;
	if (!verify_string(vbase,FUNC_GETASSIGN)) return;
	if (args == 2)
	{	if (!verify_string(&vbase[1],FUNC_GETASSIGN)) return;

		opts = 0;
		s = STR_VAL(vbase[1].v);
		while (*s)
		{	switch (*s++)
			{	case 'v': case 'V': opts |= LDF_VOLUMES; break;
				case 'a': case 'A': opts |= LDF_ASSIGNS; break;
				case 'd': case 'D': opts |= LDF_DEVICES; break;
			}
		}
	}

	result = GetAssign(STR_VAL(vbase->v),format_string,opts);

	popall(vbase);

	if ( result )
	{
		TempString(format_string,strlen(format_string),vbase);
	}
	else
	{
		TempString("",0,vbase);
	}

	vtop = vbase + 1;
}

void do_makeassign(struct Value *vbase, WORD args)
{
	BPTR		lock = NULL;

	if (argcheck(args,1,2,FUNC_MAKEASSIGN)) return;
	if (!verify_string(vbase,FUNC_MAKEASSIGN)) return;
	if (args == 2)
	{	if (!verify_string(&vbase[1],FUNC_MAKEASSIGN)) return;

		lock = Lock(STR_VAL(vbase[1].v),ACCESS_READ);
		if (lock == NULL)
		{	err_msg(ehead,ERR_DOS,FUNC_MAKEASSIGN,cannot_assign_msg);
			return;
		}
	}

	if (!PRETEND_MODE)
		SetAssign(STR_VAL(vbase->v),(args == 2 ? lock : NULL));

	if (lock) UnLock(lock);

	popall(vbase);
}

void do_getdevice(struct Value *vbase, WORD args)
{	BOOL FindDOSDevice(char *name, char *buffer);
	BOOL result;

	if (argcheck(args,1,1,FUNC_GETDEVICE)) return;
	if (!verify_string(vbase,FUNC_GETDEVICE)) return;

	result = FindDOSDevice(STR_VAL(vbase->v),format_string);

	popall(vbase);

	if ( result )
	{
		TempString(format_string,strlen(format_string),vbase);
	}
	else
	{
		TempString("",0,vbase);
	}

	vtop = vbase + 1;
}

LONG GetProtect(char *file)
{	struct FileInfoBlock 	*fib;
	LONG					result;
	BPTR					lock = NULL;

	unless (fib = MemAlloc(sizeof *fib,MEMF_CLEAR|MEMF_PUBLIC))
	{
		memerr();
		return -1;
	}

	if (lock = Lock(file,ACCESS_READ))
	{
		result = Examine(lock,fib);
		UnLock(lock);

		if (result != 0) result = fib->fib_Protection;
		else result = -1;
	}
	else result = -1;

	MemFree(fib);

	return result;
}

void do_protect(struct Value *vbase, WORD args)
{
	LONG		result,
				result2;

	if (argcheck(args,1,2,FUNC_PROTECT)) return;
	if (!verify_string(vbase,FUNC_PROTECT)) return;

	result = GetProtect(STR_VAL(vbase->v));
	if (result != -1 && args == 2)
	{
		result2 = TRUE;

		if (!PRETEND_MODE)
		{
			if (vbase[1].type == TYPE_INTEGER)
			{
				result2 = SetProtection(STR_VAL(vbase->v),(LONG)vbase[1].v);
			}
			else
			{	char	ch,
						*str = STR_VAL(vbase[1].v);
				BOOL	on = TRUE;

				while (ch = *str++)
				{
					switch (ch) {

					case '+':
						on = TRUE;
						break;

					case '-':
						on = FALSE;
						break;

					case 'r': case 'R':
						if (on) result &= ~FIBF_READ;
						else result |= FIBF_READ;
						break;

					case 'w': case 'W':
						if (on) result &= ~FIBF_WRITE;
						else result |= FIBF_WRITE;
						break;

					case 'd': case 'D':
						if (on) result &= ~FIBF_DELETE;
						else result |= FIBF_DELETE;
						break;

					case 'e': case 'E':
						if (on) result &= ~FIBF_EXECUTE;
						else result |= FIBF_EXECUTE;
						break;

					case 'a': case 'A':
						if (on) result |= FIBF_ARCHIVE;
						else result &= ~FIBF_ARCHIVE;
						break;

					case 'p': case 'P':
						if (on) result |= FIBF_PURE;
						else result &= ~FIBF_PURE;
						break;

					case 's': case 'S':
						if (on) result |= FIBF_SCRIPT;
						else result &= ~FIBF_SCRIPT;
						break;
					}
				}
				
				result2 = SetProtection(STR_VAL(vbase->v),result);
			}
		}

		result = result2;
	}

	popall(vbase);
	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase + 1;
}

void do_user(struct Value *vbase, WORD args)
{
	extern void		**ulevel_ptr;
	LONG			prior = user_level;

	if (argcheck(args,1,1,FUNC_USER)) return;

	if (vbase->type != TYPE_INTEGER)
	{	err_msg(ehead,ERR_SCRIPT,FUNC_USER,"Integer argument expected");
		return;
	}

	user_level = (LONG)vbase->v;
	*ulevel_ptr = (void *)user_level;

	popall(vbase);
	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)prior;
	vtop = vbase + 1;
}

/* REM: what should we do if they try to compare
	strings with integers?
	NULL with integers?
	NULL with srtings?
*/

void do_getenv(struct Value *vbase, WORD args)
{	struct String *s;

	if (argcheck(args,1,1,FUNC_GETENV)) return;

	if (vbase->type != TYPE_STRING)
	{	err_msg(ehead,ERR_SCRIPT,FUNC_GETENV,"String argument expected");
		return;
	}

	s = vbase->v;

	if (!GetDEnv((char *)(s+1),format_string,MAX_STRING-1))
	{	err_msg(ehead,ERR_DATA,FUNC_GETENV,string_length_msg);
		return;
	}

	vfree(vbase);
	TempString(format_string,strlen(format_string),vbase);
}

	/* returns TRUE if all argument pairs obey some condition */

enum compare_types {
	CMP_EQ,
	CMP_NE,
	CMP_GT,
	CMP_GE,
	CMP_LT,
	CMP_LE
};

void do_compare(struct Value *vbase, WORD args, WORD type)
{	struct Value	*v1, *v2;
	LONG 			comparison,
					result = TRUE;
	WORD 			i, j;

	j = args & ~1;

	for (i=0; i<j - 1; i++)
	{	v1 = vbase + i;
		v2 = v1 + 1;

		if (v1->type == TYPE_INTEGER && v2->type == TYPE_INTEGER)
		{
			comparison = (long)v1->v - (long)v2->v;
		}
		else if (v1->type == TYPE_STRING && v2->type == TYPE_STRING)
		{
			comparison = scmp(v1,v2);
		}
		else
		{
			integer_to_string(v1);
			integer_to_string(v2);
			comparison = scmp(v1,v2);
		}

		switch (type) {
		case CMP_EQ: 
		case CMP_NE:		/* will be inverted later */
					 if (comparison) result = FALSE; break;
		case CMP_GT: if (comparison <= 0) result = FALSE; break;
		case CMP_GE: if (comparison < 0) result = FALSE; break;
		case CMP_LE: if (comparison > 0) result = FALSE; break;
		case CMP_LT: if (comparison >= 0) result = FALSE; break;
		}
	}

	if (type == CMP_NE) result = !result;

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->storage = STORE_TEMP;
	vbase->v = (void *)result;
	vtop = vbase + 1;
}

	/* returns TRUE if all arguments are equal */

void do_equal(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_EQ);
}

	/* returns FALSE if all arguments are equal */

void do_unequal(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_NE);
}

	/* returns TRUE if all arguments are in ascending order */

void do_greater(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_GT);
}

	/* returns TRUE if all arguments are in ascending (or equal) order */

void do_greater_equal(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_GE);
}

	/* returns TRUE if all arguments are in desccending order */

void do_lesser(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_LT);
}

	/* returns TRUE if all arguments are in descending (or equal) order */

void do_lesser_equal(struct Value *vbase, WORD args)
{
	do_compare(vbase,args,CMP_LE);
}

	/* returns 1st arg minus all remaining args  */

void do_minus(struct Value *vbase, WORD args)
{	struct Value 	*v;
	LONG 			result = 0,
					val;
	WORD 			i;

	for (i=0; i<args; i++)
	{	v = vbase + i;
		val = as_integer(v);
		if (i == 0) result = val;
		else result -= val;
		vfree(v);
	}

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->storage = STORE_TEMP;
	vbase->v = (void *)result;
	vtop = vbase + 1;
}

enum functypes {
	MFUNC_ADD,
	MFUNC_MUL,
	MFUNC_AND,
	MFUNC_OR,
	MFUNC_XOR,
	MFUNC_IN,
	MFUNC_BITAND,
	MFUNC_BITOR,
	MFUNC_BITXOR,
	MFUNC_LAST
};

void do_mathfunc(struct Value *vbase, WORD args, LONG result, WORD functype)
{	struct Value *v;
	WORD	i;
	long	mask, val;

	for (i=0; i<args; i++)
	{	v = vbase + i;
		val = as_integer(v);

		switch (functype) {
		case MFUNC_ADD: result += val; break;
		case MFUNC_MUL: result *= val; break;
		case MFUNC_AND: result = result && val; break;
		case MFUNC_OR:  result = result || val; break;
		case MFUNC_XOR: result = (result && !val) || (!result && val); break;
		case MFUNC_IN:
			if (i == 0) { result = val; mask = 0; }
			else mask |= 1 << val;
			break;
		case MFUNC_BITAND: result &= val; break;
		case MFUNC_BITOR:  result |= val; break;
		case MFUNC_BITXOR: result ^= val; break;
		}
		vfree(v);
	}

	if (functype == MFUNC_IN) result &= mask;

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->storage = STORE_TEMP;
	vbase->v = (void *)result;
	vtop = vbase + 1;
}

	/*	returns ~0 if bits indicated by 2nd and greater operands are
		set in first operand										*/

void do_in(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_IN);
}

	/* returns the arithmetic sum of all args */

void do_plus(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_ADD);
}

	/* returns the product of all args */

void do_mul(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,1,MFUNC_MUL);
}

	/* returns 1st arg minus all remaining args  */

void do_div(struct Value *vbase, WORD args)
{	LONG result, val;

	if (argcheck(args,2,2,"DIVIDE")) return;

	val = as_integer(vbase + 1);

	if (val == 0) err_msg(ehead,ERR_DATA,"DIVIDE",divide_zero_msg);
	else
	{	result = as_integer(vbase) / val;

		popall(vbase);
		vbase->type = TYPE_INTEGER;
		vbase->v = (void *)result;
		vtop = vbase+1;
	}
}

	/* returns the logical and of all args */

void do_and(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,1,MFUNC_AND);
}

	/* returns the logical or of all args */

void do_or(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_OR);
}

	/* returns the logical xor of all args */

void do_xor(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_XOR);
}

	/* returns logical not of single arg  */

void do_not(struct Value *vbase, WORD args)
{	LONG	val;

	if (argcheck(args,1,1,"NOT")) return;

	val = !as_integer(vbase);

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)val;
	vtop = vbase+1;
}

	/* returns the bitwise and of all args */

void do_bitand(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,-1,MFUNC_BITAND);
}

	/* returns the bitwise or of all args */

void do_bitor(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_BITOR);
}

	/* returns the bitwise xor of all args */

void do_bitxor(struct Value *vbase, WORD args)
{	do_mathfunc(vbase,args,0,MFUNC_BITXOR);
}

	/* returns bitwise not of single arg  */

void do_bitnot(struct Value *vbase, WORD args)
{	LONG	val;

	if (argcheck(args,1,1,"NOT")) return;

	val = ~as_integer(vbase);

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)val;
	vtop = vbase+1;
}

	/* returns the first argument shifted left by second argument */

void do_shiftleft(struct Value *vbase, WORD args)
{	long	result;

	if (argcheck(args,2,2,FUNC_SHIFTLEFT)) return;

	result = as_integer(vbase) << as_integer(vbase + 1);

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase+1;
}

	/* returns the first argument shifted right by second argument */

void do_shiftright(struct Value *vbase, WORD args)
{	long	result;

	if (argcheck(args,2,2,FUNC_SHIFTRIGHT)) return;

	result = as_integer(vbase) >> as_integer(vbase + 1);

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)result;
	vtop = vbase+1;
}

	/* returns the sub-string */

void do_substr(struct Value *vbase, WORD args)
{	long	len;

	if (argcheck(args,2,3,FUNC_SUBSTR)) return;
	if (!verify_string(vbase,FUNC_SUBSTR)) return;

	if (vbase[1].type != TYPE_INTEGER ||
		(args == 3 && vbase[2].type != TYPE_INTEGER))
	{
		err_msg(ehead,ERR_SCRIPT,FUNC_SUBSTR,"Integer argument expected");
	}
	else
	{
		len = (args == 3 ? (long)vbase[2].v : MAX_STRING-1);
		strncpy(format_string,STR_VAL(vbase->v) + (long)vbase[1].v,len);
		format_string[len] = 0;
		popall(vbase);

		TempString(format_string,strlen(format_string),vbase);
		vtop = vbase + 1;
	}
}

	/* returns the length of a string */

void do_strlen(struct Value *vbase, WORD args)
{	long	len;

	if (argcheck(args,1,1,FUNC_STRLEN)) return;
	if (!verify_string(vbase,FUNC_STRLEN)) return;

	len = strlen(STR_VAL(vbase->v));

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)len;
	vtop = vbase + 1;
}

	/* returns TRUE/FALSE based on whether string matchs an AmigaDOS pattern */

void do_patmatch(struct Value *vbase, WORD args)
{	long	result;

	if (argcheck(args,2,2,FUNC_PATMATCH)) return;
	if (!verify_string(vbase,FUNC_PATMATCH) ||
		!verify_string(vbase+1,FUNC_PATMATCH)) return;

	result = astcsma(STR_VAL(vbase[1].v),STR_VAL(vbase->v));

	popall(vbase);

	vbase->type = TYPE_INTEGER;
	vbase->v = (void *)(result ? 1 : 0);
	vtop = vbase + 1;
}

	/* prints a debugging message to console */

void do_debug(struct Value *vbase, WORD args)
{	struct Value *v;
	WORD i;

	for (i=0; i<args; i++)
	{	v = vbase + i;
		if (v->type == TYPE_INTEGER) Printf("%ld ",v->v);
		else if (v->type == TYPE_STRING) Printf("%ls ",(struct String *)(v->v)+1);
		else if (v->type == TYPE_NONE) Printf("<NIL> ");
		vfree(v);
	}
	Printf("\n");

	vbase->type = TYPE_NONE;
	vtop = vbase + 1;
}

#ifdef DEBUG
void debug_action(char *act)
{
	Puts(act);
}
#endif

/* ============================= Error Messages ========================== */

void memerr(void)
{
	err(ehead,ERR_MEMORY,err_memory_msg);
}

aborterr(char *name)
{	int 	ask_abort(void);
	int		i;

	if (i = ask_abort())
	{
		transcript(user_abort_msg);		/* transcript file notation */
		ierror = ERR_ABORT;
	}

	return i;
}

extern void **errmsg_ptr,
			**ioerr_ptr;

void err(struct TreeNode *node, WORD code, char *msg, ...)
{	void	show_error(WORD mode,char *text);
	char	*emsg = MemAlloc(MAX_STRING,0);

	if (emsg == NULL)
	{
		memerr();
		return;
	}

	if (code == ERR_DOS) *ioerr_ptr = (void *)IoErr();

	VSprintf(emsg,msg,&msg+1);

		/* call user interface */
	if (in_trap == 0) show_error(code,emsg);
	else
	{
		if (*errmsg_ptr)
		{	FreeString(*errmsg_ptr);
			*errmsg_ptr = NULL;
		}
		*errmsg_ptr = MakeString(emsg,strlen(msg));
	}

#ifdef DEBUG
		/* leave this in for now */
	if (ierror == ERR_HANDLED)
	{
		Printf("Script aborted in line %ld.\n",node->line);
	}
	else
	{
		Printf(node ? "%s in line %ld.\n" : "%s\n",emsg,node->line);
	}
#endif

		/* transcript file notation needed */
	if (ierror == ERR_HANDLED)
	{
		transcript(script_line_msg1,node->line);
	}
	else
	{
		transcript(node ? script_line_msg2 : "%s\n",emsg,node->line);
	}

	ierror = code;

	MemFree(emsg);
}

void err_msg(struct TreeNode *node, WORD code, char *sub, char *msg, ...)
{	void	show_error(WORD mode,char *text);
	char	*emsg = MemAlloc(MAX_STRING,0);

	if (emsg == NULL)
	{
		memerr();
		return;
	}

	if (code == ERR_DOS) *ioerr_ptr = (void *)IoErr();

		/* call user interface */
	strcpy(emsg,sub);
	strcat(emsg,": ");
	VSprintf(&emsg[strlen(emsg)],msg,&msg+1);

	if (in_trap == 0) show_error(code,emsg);
	else
	{
		if (*errmsg_ptr)
		{	FreeString(*errmsg_ptr);
			*errmsg_ptr = NULL;
		}
		*errmsg_ptr = MakeString(emsg,strlen(msg));
	}

#ifdef DEBUG
		/* leave this in for now */
	if (ierror == ERR_HANDLED)
	{
		Printf("Script aborted in line %ld.\n",node->line);
	}
	else
	{
		Printf(node ? "%s in line %ld.\n" : "%s\n",emsg,node->line);
	}
#endif

		/* transcript file notation needed */
	if (ierror == ERR_HANDLED)
	{
		transcript(script_line_msg1,node->line);
	}
	else
	{
		transcript(node ? script_line_msg2 : "%s\n",emsg,node->line);
	}

	ierror = code;

	MemFree(emsg);
}

argcheck(WORD argcount, WORD min, WORD max, char *name)
{	if (argcount < min) err(ehead,ERR_SCRIPT,"%s: Too few arguments",name);
	if (argcount > max) err(ehead,ERR_SCRIPT,"%s: Too many arguments",name);
	return (int)ierror;
}

void transcript(char *string, ...)
{	extern BPTR		transcript_file;

	if (transcript_file)
	{	
		if (VFprintf((void *)transcript_file,string,&string+1) <= 0)
		{
			Close(transcript_file);
			transcript_file = NULL;
		}
	}
}

void transcript_wrap(char *string,...)
{	extern BPTR		transcript_file;
	char			saved,
					*temp,
					*mark,
					*splat;

	if (transcript_file)
	{
		if ( (temp = AllocMem(2048,0)) != NULL )
		{
			VSprintf(temp,string,&string+1);
			string = temp;

			while (1)
			{
				if (strlen(string) < 73)
				{
					Fprintf((void *)transcript_file,string);
					break;
				}

				splat = &string[73];
				saved = *splat;
				*splat = 0;

				mark = strrchr(string,' ');

				*splat = saved;

				if (mark == NULL)
				{
					Fprintf((void *)transcript_file,string);
					break;				
				}

				*mark = 0;
				if (Fprintf((void *)transcript_file,"%s\n",string) <= 0)
				{
					Close(transcript_file);
					transcript_file = NULL;
					break;
				}

				string = mark + 1;
			}

			FreeMem(temp,2048);
		}
		else 
		{
			if (VFprintf((void *)transcript_file,string,&string+1) <= 0)
			{
				Close(transcript_file);
				transcript_file = NULL;
			}
		}
	}
}
