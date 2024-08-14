/* ========================================================================= *
 * SymTab.h - structures for generic symbol table                            *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

struct GSymbol {
	struct GSymbol		*NextSymbol;			/* pointer to next symbol		*/
	BYTE				SymbolLength;			/* length of symbol				*/
	BYTE				SymbolType;				/* for application use			*/
	/* followed by data */
	/* followed by name */
};

struct GSymbolScope {
	struct GSymbolScope	*NextScope;				/* for application use			*/
	struct GSymbol		*FirstSymbol;			/* pointer to first symbol		*/
	struct GSymbol		**HashTable;			/* hashtable, if used...		*/
/*	LONG				SymbolCount;	*/		/* number of symbols in context	*/
	WORD				HashSize;				/* size of has table			*/
	WORD				SymbolDataSize;			/* size of data after symbol	*/
	int					(*DeleteHook)();		/* for deleting extra sym data	*/
};

	/* NOTE: either FirstSymbol or HashTable must be NULL. */
	/* REM:  When to attach hash table?? */

	/* Function definitions */

struct GSymbol *FindGSymbol (
				struct GSymbolScope *scope, char *name, WORD namelength,
				struct GSymbol ***headptr );
struct GSymbol *AddGSymbol (
				struct GSymbolScope *scope, char *name, WORD namelength,
				struct GSymbol **headptr );
BOOL DeleteGSymbol (
				struct GSymbolScope *scope, struct GSymbol *symbol );
void DeleteGScope (	struct GSymbolScope *scope );
AddGHash (		struct GSymbolScope *scope, WORD hsize );
WORD HashVal (char *name, WORD length, WORD hashsize );
void *ReverseList ( void * );

#ifdef AZTEC_C
#pragma regcall( HashVal(a0,d0,d1) )
#pragma regcall( ReverseList(a0) )
#endif
