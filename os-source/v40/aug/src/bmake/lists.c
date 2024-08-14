/*	lists.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <clib/exec_protos.h>

#include "make.h"


void *
for_list( struct List *list, long (*node_fn)(void *))
{
	struct Node *ln, *succ;

	for( ln = list->lh_Head; ln->ln_Succ; ln = succ ) {
		succ = ln->ln_Succ;
		if( (*node_fn)( ln ))
			return( ln );
	}
	return( NULL );
}

void
attach_list( struct List *newlist, struct List *oldlist )
{
	struct Node *ln;

	while( ln = RemHead( oldlist ))
		AddTail( newlist, ln );
}

