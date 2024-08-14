/*
 * Amiga Grand Wack
 *
 * symbols.h - Wack internal symbol structures.
 *
 * $Id: symbols.h,v 39.4 93/07/16 18:25:21 peter Exp $
 *
 */

#define HASHSIZE	27

#define ACT_CONSTANT	0	/* Sets current address to value1 */
#define ACT_COMMAND	1	/* Execute function pointed at by sp->value1 */
#define ACT_PGMSYMBOL	2	/* addr=value1, used by symbol loader */

struct Symbol
{
    struct Symbol *sym_Next;
    char *sym_Name;
    long sym_Type;
    union
    {
	long a1_value;
	void *a1_function;
    } sym_Arg1;
    union
    {
	long a2_value;
	char *a2_args;
    } sym_Arg2;
};

#define sym_Value1 sym_Arg1.a1_value
#define sym_Value2 sym_Arg2.a2_value
#define sym_Function sym_Arg1.a1_function
#define sym_Args sym_Arg2.a2_args

struct SymbolMap
{
    struct Symbol *head[HASHSIZE];
    struct Symbol *tail[HASHSIZE];
};

