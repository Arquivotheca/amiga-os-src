/*
 * Amiga Grand Wack
 *
 * $Id: special_proto.h,v 39.3 93/06/02 15:51:52 peter Exp $
 *
 */

APTR FindBase( char *name );

void ShowAddress( char *s, APTR x );

ULONG WalkList( struct List *list, int nodesize, ULONG (*nodefunc)(), BOOL newline );

ULONG doNode( struct Node *node, int nodesize, ULONG (*nodefunc)( struct Node *, APTR, char * ) );

ULONG WalkMinList( struct MinList *list, int nodesize, ULONG (*nodefunc)(), BOOL newline );

ULONG doMinNode( struct MinNode *node, int nodesize, ULONG (*nodefunc)( struct MinNode *, APTR ) ) ;

ULONG WalkSimpleList( APTR firstnode, int nodesize, ULONG (*nodefunc)(), BOOL newline ) ;
