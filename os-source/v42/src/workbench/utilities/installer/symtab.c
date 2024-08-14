/*
 *		symtab.c
 *
 * =========================================================================
 * SymTab.c - functions for generic symbol table                             
 * By Talin. (c) 1990 Sylvan Technical Arts                                  
 * ========================================================================= 
 *	
 * Prototypes for functions defined in SymTab.c
 *
 *	struct GSymbol * 	FindGSymbol		(struct GSymbolScope * , unsigned char * , WORD , struct GSymbol *** );
 *	struct GSymbol * 	AddGSymbol		(struct GSymbolScope * , unsigned char * , WORD , struct GSymbol ** );
 *	void 				DeleteGScope	(struct GSymbolScope * );
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */



#define NO_HASH								/* no hash tabe */

#include "functions.h"

#include "symtab.h"
#include "xfunctions.h"

	/* Added by DJ for installer... --DJ */

struct Value;
sym_delete_hook(struct GSymbolScope *scope,struct Value *value);

	/* NOTES:
		We need to modify this do do "word-length" comparisons.
		Also, need to modify this for case-insensitivity.
	*/


struct GSymbol *FindGSymbol(
	struct GSymbolScope	*scope,
	char				*name,
	WORD				namelength,
	struct GSymbol		***headptr)
{
	struct GSymbol		**head;

#ifndef NO_HASH
	if (scope->HashTable)						/* if there is a hashtable		*/
		head = scope->HashTable +				/* compute address of head ptr	*/
			HashVal(name,namelength,scope->HashSize);
	else
#endif
		head = &scope->FirstSymbol;				/* else just use single chain	*/

	if (*head)									/* if there are any symbols here */
	{
		WORD			offset;
		struct GSymbol	*search;
												/* calc offset of name			*/
		offset = sizeof *search + scope->SymbolDataSize;
												/* search the list				*/
		for (search = *head; search; search = search->NextSymbol)
		{
			if (search->SymbolLength == namelength &&
				CmpMemI((UBYTE *)search + offset,name,namelength) )
					return search;				/* if a match, return found node */
		}
	}

	if (headptr) 
		*headptr = head;						/* dive back where to add new sym */
	return NULL;
}



	/* You must already have called FindSymbol to use this routine. */

struct GSymbol *AddGSymbol(
	struct GSymbolScope	*scope,
	char				*name,
	WORD				namelength,
	struct GSymbol		**headptr)
{
	struct GSymbol		*newsymbol;				/* new symbol					*/
	WORD				offset;					/* offset to name field			*/

	offset = sizeof *newsymbol + scope->SymbolDataSize;

	/* Q: do we really want to do MEMF_CLEAR?? */

	if (newsymbol = AllocMem(offset + namelength,MEMF_CLEAR))
	{
		newsymbol->NextSymbol = *headptr;
		newsymbol->SymbolLength = namelength;
		CopyMem(name,(UBYTE *)newsymbol + offset,namelength);
		*headptr = newsymbol;
	}
	return newsymbol;
}



	/* commented out because not used in installer... */

#if 0
	/* NOTE: This is not neccessarily a fast routine... */

BOOL DeleteGSymbol(
	struct GSymbolScope	*scope,
	struct GSymbol		*symbol)
{
	struct GSymbol		**head;

#ifndef NO_HASH
	if (scope->HashTable)						/* if there is a hashtable		*/
		head = scope->HashTable +				/* compute address of head ptr	*/
			HashVal((BYTE *)(symbol + 1) + scope->SymbolDataSize,
					symbol->SymbolLength,
					scope->HashSize);
	else
#endif
		head = &scope->FirstSymbol;			/* else just use single chain	*/

	while (*head)								/* if there are any symbols here */
	{
		if (*head == symbol)
		{
			if (scope->DeleteHook)
			{
				scope->DeleteHook(scope,symbol+1);
			}
			*head = symbol->NextSymbol;
			FreeMem(symbol,sizeof *symbol + scope->SymbolDataSize + symbol->SymbolLength);
			return TRUE;
		}
		*head = (*head)->NextSymbol;
	}
	return FALSE;
}
#endif

void DeleteGScope( struct GSymbolScope *scope )
{
	struct GSymbol		*sym, *next;

	for (sym = scope->FirstSymbol; sym; sym = next)
	{
		next = sym->NextSymbol;
#if 0
		if (scope->DeleteHook)
		{
			(scope->DeleteHook)(scope,symbol+1);
		}
#else
		sym_delete_hook(scope,(struct Value *)(sym+1));
#endif
		FreeMem(sym,sizeof *sym + scope->SymbolDataSize + sym->SymbolLength);
	}
	scope->FirstSymbol = NULL;

#ifndef NO_HASH
	if (scope->HashTable)
	{
		WORD				i;
		for (i=0; i<scope->HashSize; i++)
		{
			for (sym = scope->HashTable[i]; sym; sym = next)
			{
				next = sym->NextSymbol;

				if (scope->DeleteHook)
				{
					(scope->DeleteHook)(scope,sym+1);
				}
				FreeMem(sym,sizeof *sym + scope->SymbolDataSize + sym->SymbolLength);
			}
		}
		FreeMem(scope->HashTable,scope->HashSize * sizeof (struct GSymbol *));
		scope->HashTable = NULL;
	}
#endif
}



	/* commented out because not used in installer */

#if 0

AddGHash(struct GSymbolScope *scope, WORD hsize)
{
	struct GSymbol		*newlist,
						*next,
						**newhash;

	if (newhash = AllocMem(hsize * sizeof (struct GSymbol *),MEMF_CLEAR))
	{
		scope->HashTable = newhash;
		scope->HashSize = hsize;

		newlist = ReverseList(scope->FirstSymbol);
		for (;newlist; newlist = next)
		{
			struct GSymbol **head;
			next = newlist->NextSymbol;

			head = newhash +				/* compute address of head ptr	*/
				HashVal((BYTE *)(newlist+1)+scope->SymbolDataSize,newlist->SymbolLength,hsize);

			newlist->NextSymbol = *head;
			*head = newlist;
		}
		return TRUE;
	}
	return NULL;
}

#endif
