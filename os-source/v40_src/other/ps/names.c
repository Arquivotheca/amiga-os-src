//************************************************************************
//
// Module :	Postscript "Name" administration	© Commodore-Amiga
//
// Purpose:	Manage the adding and removing of arbitrary names.
//
// Internals:	Since name look-up has to be fast, a hash table with
//			linked lists as hash "chains" is used.
//
//************************************************************************

#define SIGN(x) ( ((x) > 0) - ((x) < 0) )

#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "string.h"
#include "stdio.h"

//-----------------------------------------------------------------------
// Hash Table size: make it bigger (but keep prime) to increase performance
//-----------------------------------------------------------------------
#define	HT_SIZE	229				// a bastard of a prime...

// Structure of Hash Table (ptrs to collision trees) (dynam. allocated)
// NODEPTR hashtable[HT_SIZE];

//-----------------------------------------------------------------------
// The "contents" of the node is pointed to by string

struct list_node {	
    struct list_node *next_node;	// next and prev links
    struct list_node *prev_node;
    char		     *string;
};

typedef	struct list_node LNODE;
typedef struct list_node * NODEPTR;

//-----------------------------------------------------------------------
// Function Prototypes
//---------------------

GLOBAL BOOL	  InitNameSpace	(DPSContext);			// Call once before using module
GLOBAL char	       *AddName	(DPSContext, char *);	// Enters a string in name space

int 		count_nodes (char *p);
void	  print_distrib	(DPSContext);

char	    *chain_word	(DPSContext, LNODE **, char *);
NODEPTR	       new_link	(DPSContext, char *);

// -------------------------------------------------------------------- 
// Allocate a hashtable, initialise it and link it to the context.
// Clear out the name space by initializing the hash table chains to NULL.
// -------------------------------------------------------------------- 

BOOL InitNameSpace(DPSContext ctxt) {

	NODEPTR *htable;				// htable is a pointer to the entire table
	int i;

	htable = AllocVM(VM, HT_SIZE* sizeof(NODEPTR));	// try to get the memory
	if ( ! htable) {
//		fprintf(OP,"InitNameSpace failed!\n");
		return FALSE;
	}

	ctxt->hashtab = htable;			// let context point to it

	i = HT_SIZE;
	while (i--) *htable++ = NULL;	// clear all hashchains

	return TRUE;
}

// -------------------------------------------------------------------- 
// hash name up and add string to a hash-collision chain.
// -------------------------------------------------------------------- 

char * AddName (DPSContext ctxt, char * strptr) {

	short i;
	unsigned long hashval;
	char * str=strptr;
	char *newptr;
	register NODEPTR * chain;

//		*((uword*)0xDFF180) = 0xff0;
//		*((uword*)0xDFF180) = 0x000;

		// HASH name up to get linked list #

	i = (strlen (strptr)+1)>>1;
	hashval = 0;
	while(i--) { hashval= 17+ hashval<<4 ^ (*((short*)strptr++)); }

	i = hashval % HT_SIZE;
	chain = ((NODEPTR *) ctxt->hashtab) +i;

// fprintf(OP,"Chain slot %3d lies at %x (%x + %4x) [%s]\n",
//						i, chain, ctxt->hashtab, i*4, str);

	newptr = chain_word (ctxt, chain, str);

	return newptr;
}
// -------------------------------------------------------------------- 
// Argument "node_ptr" is the address of a pointer to a node !

// This routine traverses a linked list and adds the string (the value)
// as the tail node if that string hasn't been encountered or returns
// the existing node if it has.

char * chain_word (DPSContext ctxt, LNODE **node_ptr, char *strptr ) {

	register LNODE *nd,*new;
	
	if ( ! (nd = *node_ptr)) {
		*node_ptr = new_link(ctxt, strptr) ;
		return (*node_ptr)->string ;
	}

// ELSE
// chain_word called with a valid list pointer. Scan list.	

	for (;;) {		// FOREVER

		if (! strcmp(strptr, nd->string)) return nd->string;
		
		if (! nd->next_node) {		// if this is the last node in list,
			new = new_link(ctxt, strptr);		// alloc a new node
			nd->next_node = new;				// and link into list

			return new->string;
		}

		nd = nd->next_node;			// else continue scanning the list
	}
}

// -------------------------------------------------------------------- 
LNODE * new_link (DPSContext ctxt, char *str) {
	unsigned len;
	NODEPTR temp=NULL;
	char *dststr;

	len = strlen(str) +1;	// Room for the string incl. /0 at end 

	if (dststr = AllocVM(VM,len) ) {

	    strcpy (dststr, str);		// str -> dst

	    if (temp = AllocVM(VM,sizeof(LNODE)) ) {
			temp->next_node = NULL;
			temp->string  = dststr;
	    } else {
		  	FreeVM(VM,dststr,len);
	    }
	}
	return temp;					// either NULL or new node addr
}
// -------------------------------------------------------------------- 
int count_nodes( char *p) {
	int i=0;

	while (p) {
		p = (char*) ((LNODE *)p)->next_node;
		i++;
	}
	return i;
}
// -------------------------------------------------------------------- 
// This shows how well our hasher spreads strings...
void print_distrib(DPSContext ctxt) {

	NODEPTR * chain;
	int i,j,k=0;
	int freq[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	i = HT_SIZE; chain = ctxt->hashtab;
	while (i--) {
		j = count_nodes( (char*) *chain++);
		freq[j]++;
		//fprintf (OP,"%1d,", j);
		k += j;
	}

	//fprintf (OP,"\nTotal = %d\n",k);

	for (i=0;i<16;i++) {
		//fprintf(OP,"Number of %d-length chains:%d\n", i, freq[i]);
	}
}
// -------------------------------------------------------------------- 
