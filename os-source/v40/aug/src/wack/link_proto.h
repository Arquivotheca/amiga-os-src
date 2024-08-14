/*
 * Amiga Grand Wack
 *
 * $Id: link_proto.h,v 39.9 93/11/05 15:05:17 jesup Exp $
 *
 */

LONG ValidAddress( APTR addr );

LONG ValidAddresses( APTR addr, ULONG len );

BYTE ReadByte( APTR addr );

WORD ReadWord( APTR addr );

LONG ReadLong( APTR addr );

#define ReadPointer( addr ) ((APTR)ReadLong( addr ))

void ReadBlock( APTR addr, APTR buffer, ULONG size );

ULONG ReadString( APTR addr, STRPTR buffer, ULONG maxlen );

void WriteByte( APTR addr, BYTE value );

void WriteWord( APTR addr, WORD value );

void WriteLong( APTR addr, LONG value );

void WriteBlock( APTR addr, APTR buffer, ULONG size );

#define WritePointer( addr,value ) WriteLong( addr, (LONG) value )

STRPTR ResumeExecution( void );

STRPTR CallFunction( APTR addr );

struct Node ** ObtainNodeArray( struct List *addr, ULONG size );

void ReleaseNodeArray( struct Node **array );

STRPTR ShowContext( void );

STRPTR ProtectAddresses( char *arg );

APTR OpenLinkLib( STRPTR arg );

void CloseLinkLib( APTR linkbase );

void initLinkLVO( struct LinkFakeLibrary *afl, ULONG count, APTR funcptr );

ULONG lib_stubReturn( void );
