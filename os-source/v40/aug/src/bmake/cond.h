/*	cond.h
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#define STATE_IF_T	1
#define STATE_IF_F	2	/* skip lines until ELSE mode */
#define STATE_EL_T	3	/* skip lines until ENDIF mode */
#define STATE_EL_F	4

struct mstate {
	struct Node node;
	int state;
};

struct drctvs {
	char *directive;	/* the name of the directive */
	int (*call)(char *,...);	/* function call */
};

struct drctvs *find_drctvs( struct drctvs *array, int array_size,
	char *name );
int push_state( struct List *stack, long state );
int pop_state( struct List *stack );
void clear_stack( struct List *stack );
int get_directive_state( struct List *stack );

#define MAX_CDRCTVS 6

extern struct drctvs carray[ MAX_CDRCTVS ];
extern struct List Mstack;

int drctv_if( char *string, struct List *stack );
int drctv_else( char *string, struct List *stack );
int drctv_endif( char *string, struct List *stack );

