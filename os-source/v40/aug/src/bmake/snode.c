/*	snode.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <ctype.h>

#include "make.h"

struct string_node *
new_snode( const char *str )
{
	struct string_node *new;
	
	if( new = malloc( sizeof(*new) + strlen( str ) )) {
		strcpy( new->data, str );
		new->node.ln_Name = new->data;
		new->node.ln_Type = (UBYTE)NT_USER;
		new->node.ln_Pri = 0;	/* Priority, for sorting */
	}
	return( new );
}

struct string_node *
renew_snode( const struct string_node *old, const char *str )
{
	struct string_node *new;
	
	if( new = realloc( old, sizeof(*new) + strlen( str ) )) {
		strcpy( new->data, str );
		new->node.ln_Name = new->data;
	}
	return( new );
}

struct string_node *
delete_snode( struct string_node *old )
{
	free( old );
	return( NULL );
}

void
delete_slist( struct List *list )
{
	for_list( list, delete_snode );
	NewList( list );
}

#if DEBUG
int
print_snode( struct string_node *old )
{
	printf( "snode: %s\n", old->data );
	return( 0 );
}

void
print_slist( struct list *list )
{
	for_list( list, print_snode );
}
#endif
