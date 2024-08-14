/*
 * Amiga Grand Wack
 *
 * $Id: show_proto.h,v 39.4 93/08/19 16:36:02 peter Exp $
 *
 */

unsigned long GetString( APTR addr, char *buffer, unsigned long maxlen );

unsigned long GetBSTR( APTR addr, char *buffer, unsigned long maxlen );

void GetName( APTR nameAddr, char *nameStr );

void ShowNode( struct Node *n );

STRPTR ShowNodes( char *arg );

void dumpTaskSummary( struct Task *task );

STRPTR ShowTasks( void );

void ShowDeviceHead( struct Node *device );

STRPTR ShowDeviceList( void );

STRPTR ShowResourceList( void );

STRPTR ShowLibraryList( void );

void ShowPortHead( struct Node *port );

STRPTR ShowPortList( void );

STRPTR ShowIntVects( void );

void ShowRegion( struct MemHeader *region );

STRPTR ShowRegionList( void );

void ShowMemory( struct MemRegion *region );

STRPTR ShowMemoryList( void );

void ShowModule( struct Resident *m );

STRPTR ShowResModules( void );

long compareTaskName( struct Task *task, char *arg );

struct Task *findTask( char *arg );

void dumpTask( struct Task *task );

STRPTR ShowTask( char *arg );

void dumpProcess( struct Process *proc );

STRPTR ShowProcess( char *arg );

STRPTR ShowExecBase( void );
