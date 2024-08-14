void Wack_Printf( STRPTR fmt, ... );

void GetName( APTR nameAddr, char *nameStr );

APTR FindBase( char *name );

void ShowAddressN( char *s, APTR x );

void NewLine( void );

void ShowAddress( char *s, APTR x );

void dumpRect( UBYTE *s, struct Rectangle *r );

BOOL hexArgAddr( UBYTE *arg, ULONG *val );

BOOL ScanArg( UBYTE *arg, ULONG *val );

ULONG WalkList( struct List *list, int nodesize, ULONG (*nodefunc)(), BOOL newline );

ULONG doNode( struct Node *node, int nodesize, ULONG (*nodefunc)( struct Node *, APTR, char * ) );

ULONG WalkMinList( struct MinList *list, int nodesize, ULONG (*nodefunc)(), BOOL newline );

ULONG doMinNode( struct MinNode *node, int nodesize, ULONG (*nodefunc)( struct MinNode *, APTR ) );

ULONG WalkSimpleList( APTR firstnode, int nodesize, ULONG (*nodefunc)(), BOOL newline );

void BadSyntax( void );
