/*
 * Amiga Grand Wack
 *
 * symload.c -- Symbol loader for debugging a particular file.
 *
 * $Id: symload.c,v 39.3 93/07/16 18:25:31 peter Exp $
 *
 */

#include "std.h"
#include "parse.h"
#include "symbols.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/doshunks.h>

#include "asm_proto.h"
#include "define_proto.h"
#include "io_proto.h"
#include "show_proto.h"
#include "link_proto.h"
#include "parse_proto.h"
#include "sys_proto.h"
#include "symload_proto.h"

#include "linklib_protos.h"
#include "linklib_pragmas.h"
extern APTR LinkLibBase;

extern struct Library *SysBase;
extern struct Library *DOSBase;

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define NAMESIZE 100

struct SegSem
{
    struct SignalSemaphore seg_Semaphore;
    APTR seg_Find;
    struct MinList seg_List;
};

struct SegArray
{
    ULONG seg_Address;
    ULONG seg_Size;
};

struct SegNode
{
    struct MinNode seg_Node;
    char *seg_Name;
    struct SegArray seg_Array[1];
};


#define SYMERROR_NOSEGTRACKER	0xFFFFFFFF
#define SYMERROR_NOHUNKSFOUND	0xFFFFFFFE

#define HUNKFINDER_SHOW		1
#define HUNKFINDER_BIND		2
#define HUNKFINDER_BINDFILL	3

STRPTR
bindHunks( char *arg )
{
    ULONG numhunks;
    ULONG nummodules;
    ULONG result;

    result = hunkFinder( HUNKFINDER_BIND, arg = parseRemainder( arg ), NULL,
	&nummodules, &numhunks );

    if ( result == SYMERROR_NOSEGTRACKER )
    {
	PPrintf("SegTracker not running -- can't find hunks\n");
    }
    else if ( result == SYMERROR_NOHUNKSFOUND )
    {
	PPrintf("SegTracker doesn't know any hunks");
	if ( arg )
	{
	    PPrintf(" for %s\n", arg );
	}
    }
    else
    {
	PPrintf("Bound symbols for %ld hunks of ", numhunks );
	if ( arg )
	{
	    PPrintf("%s\n", arg );
	}
	else
	{
	    PPrintf("%ld modules\n", nummodules );
	}
    }
    return( NULL );
}


STRPTR
showHunks( char *arg )
{
    ULONG result = hunkFinder( HUNKFINDER_SHOW, arg = parseRemainder( arg ), NULL, 0, 0 );
    if ( result == SYMERROR_NOSEGTRACKER )
    {
	PPrintf("SegTracker not running -- can't find hunks\n");
    }
    else if ( result == SYMERROR_NOHUNKSFOUND )
    {
	PPrintf("SegTracker doesn't know any hunks");
	if ( arg )
	{
	    PPrintf(" for %s\n", arg );
	}
    }
    return( NULL );
}


#define CHECKEOF(x) if (((ULONG)&bufferpos[x]) > ((ULONG)bufferend)) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
#define GETNEXTLONG bufferlong=*bufferpos++; CHECKEOF(0)

STRPTR
bindSymbols( char *arg )
{
    BPTR fh;
    char symbuffer[256];	/* Module name buffer and symbol buffer */
    ULONG rc;
    char taskname[ MAXTOKENLEN ];
    char symfilename[ MAXTOKENLEN ];
    char *symbolname;
    ULONG symcount = 0;

    /* The first argument is the local file containing symbols.
     * The second argument, if supplied, is the name of the
     * actual running task.  If omitted, we use the file part
     * of the symbol file name, up to the first dot.
     */
    if ( arg = parseToken( arg, symfilename ) )
    {
	PPrintf( "Loading symbols for %s...\n", symfilename );

	strcpy( symbuffer, FilePart( symfilename ) );
	symbolname = symbuffer;
	while ( *symbolname )
	{
	    if ( *symbolname == '.' )
	    {
		*symbolname = '\0';
	    }
	    else
	    {
		symbolname++;
	    }
	}

	if ( !parseToken( arg, taskname ) )
	{
	    strcpy( taskname, symbuffer );
	}
	*symbolname++ = '_';

	if ( fh = Open( symfilename, MODE_OLDFILE ) )
	{
	    ULONG bufferlong;
	    ULONG *buffer;
	    ULONG buffersize;

	    Seek( fh, 0, OFFSET_END );
	    buffersize = Seek( fh, 0, OFFSET_BEGINNING );
	    if ( buffer = AllocVec( buffersize, MEMF_PUBLIC ) )
	    {
		if ( buffersize == Read( fh, buffer, buffersize ) )
		{
		    ULONG *bufferpos;
		    ULONG *bufferend;
		    ULONG *table = NULL;
		    ULONG tableSize;
		    ULONG firstHunk;
		    ULONG lastHunk;
		    ULONG hunk;
		    ULONG hunkType;
		    ULONG flag;

		    bufferpos = buffer;
		    bufferend = (ULONG *)( ( (ULONG)buffer ) + buffersize );

		    GETNEXTLONG;	/* Check for hunk header... */
		    if ( bufferlong != HUNK_HEADER )
		    {
			rc = ERROR_FILE_NOT_OBJECT;
			goto ReadError;
		    }

		    GETNEXTLONG;	/* Check for resident lib */
		    if ( bufferlong != 0 )
		    {
			rc = ERROR_FILE_NOT_OBJECT;
			goto ReadError;
		    }

		    GETNEXTLONG;	/* Get table size */
		    tableSize = bufferlong*4;
		    if ( !( table = AllocVec( tableSize, MEMF_CLEAR ) ) )
		    {
			rc=IoErr();
			goto ReadError;
		    }
		    if ( rc = hunkFinder( HUNKFINDER_BINDFILL, taskname, table, 0, 0 ) )
		    {
			goto ReadError;
		    }

		    GETNEXTLONG;	/* Get first hunk slot */
		    firstHunk = bufferlong;

		    GETNEXTLONG;	/* Last hunk slot */
		    lastHunk = bufferlong;

		    for ( hunk = firstHunk; hunk <= lastHunk; hunk++ )
		    {
			GETNEXTLONG;	/* Get next hunk size */
		    }

		    hunk=firstHunk;
		    while ( ( !rc ) && ( hunk <= lastHunk ) )
		    {
			if ( bufferpos != bufferend )
			{
			    GETNEXTLONG;
			    hunkType = bufferlong & 0x3FFFFFFF;

			    switch ( hunkType )
			    {
			    case HUNK_CODE:
			    case HUNK_DATA:
				GETNEXTLONG;
				if ( bufferlong )
				{
				    CHECKEOF( bufferlong );
				    bufferpos = &bufferpos[ bufferlong ];
				}
				break;

			    case HUNK_RELOC32:
				flag = TRUE;
				while ( flag )
				{
				    ULONG relnum;
				    ULONG hunknum;

				    GETNEXTLONG;
				    if ( relnum = bufferlong )
				    {
					GETNEXTLONG;
					hunknum = bufferlong;
					if ( hunknum > lastHunk )
					{
					    rc = ERROR_FILE_NOT_OBJECT;
					    goto ReadError;
					}
					bufferpos = &bufferpos[ relnum ];
					CHECKEOF( 0 );
				    }
				    else
				    {
					flag = FALSE;
				    }
				}
				break;

			    case HUNK_DEBUG:
			    case HUNK_NAME:
				GETNEXTLONG;
				bufferpos = &bufferpos[ bufferlong ];
				CHECKEOF( 0 );
				break;

			    case HUNK_SYMBOL:
				while ( bufferlong )
				{
				    GETNEXTLONG;
				    if ( bufferlong )
				    {
					/*
					 * First, we check if the symbol is
					 * a "." symbol.  All "." symbols are
					 * skipped.  (These are "hidden")
					 */
					if ( ( *( (char *)bufferpos ) ) != '.' )
					{
					    /*
					     * Here we kludge a bit...
					     * We assume that there is a NULL byte
					     * after the symbol name.  Usually there
					     * must be as it is padded to the longword.
					     * But, when there is not, we assume that
					     * the symbol value will be less than 16meg
					     * such that the high byte is 0 and thus
					     * giving us the NULL we need.
					     * Our output symbols will have the
					     * NULL counted such that we always have at
					     * least one NULL in the symbol name
					     */
					    strcpy( symbolname, (STRPTR)bufferpos );
					    BindValue( symbuffer, ACT_PGMSYMBOL, bufferpos[bufferlong] + table[ hunk ] + 4 );
					    symcount++;
					}
					bufferpos = &bufferpos[ bufferlong + 1 ];
					CHECKEOF( 0 );
				    }
				}
				break;

			    case HUNK_END:
				hunk++;
				break;

			    case HUNK_BSS:
				bufferpos++;
				CHECKEOF(0);
				break;

			    default:
				PPrintf( "Unexpected hunk %lx\n",&hunkType );
				rc = ERROR_OBJECT_WRONG_TYPE;
				goto ReadError;
			    }
			}
		    }

ReadError:
		    FreeVec( table );
		}
		else
		{
		    rc = IoErr();
		}
		FreeVec( buffer );
	    }
	    else
	    {
		rc = IoErr();
	    }
	    Close( fh );
	}
	else
	{
	    rc = IoErr();
	}

	if ( rc )
	{
#define ERRORBUFFERLEN 81
	    char errorbuffer[ ERRORBUFFERLEN ];

	    if ( rc == SYMERROR_NOSEGTRACKER )
	    {
		PPrintf("SegTracker not running\n");
	    }
	    else if ( rc == SYMERROR_NOHUNKSFOUND )
	    {
		PPrintf("Could not find hunks for %s\n", taskname );
	    }
	    else
	    {
		Fault( rc, NULL, errorbuffer, ERRORBUFFERLEN );
		PPrintf( "%s", errorbuffer );
	    }
	}
	else
	{
	    PPrintf("Loaded %ld symbols\n", symcount );
	}
    }
    return( NULL );
}


ULONG
hunkFinder( ULONG option, STRPTR taskname, ULONG *table, ULONG *nummodules, ULONG *numhunks )
{
    struct ExecBase eb;
    struct SegSem *segsem = NULL;
    struct Node *node;
    struct Node *succ;
    STRPTR name;
    char buffer[NAMESIZE];
    ULONG result;

    if ( nummodules )
    {
	*nummodules = 0;
    }
    if ( numhunks )
    {
	*numhunks = 0;
    }

    result = SYMERROR_NOSEGTRACKER;

    LinkLock();
    ReadBlock( ReadPointer( ABSSYSBASE ), &eb, sizeof( struct ExecBase ) );
    for ( node = eb.SemaphoreList.lh_Head;
	succ = ReadPointer( node );
	node = succ )
    {
	if ( name = ReadPointer( &node->ln_Name ) )
	{
	    GetName( name, buffer );
	    if ( !strcmp( buffer, "SegTracker" ) )
	    {
		segsem = (struct SegSem *)node;
		break;
	    }
	}
    }

    if ( segsem )
    {
	struct SegNode *segnode;
	struct SegNode *segsucc;
	struct List *symlist;
	ULONG hunk;
	ULONG hunkaddr;
	ULONG namelen;
	ULONG tasknamelen = strlen( taskname );
	STRPTR module;

	result = SYMERROR_NOHUNKSFOUND;
	symlist = ReadPointer( &segsem->seg_List );
	for ( segnode = ReadPointer( &symlist->lh_Head );
	    segsucc = ReadPointer( segnode );
	    segnode = segsucc )
	{
	    GetName( ReadPointer( &segnode->seg_Name ), buffer );
	    module = FilePart( buffer );

	    if ( taskname )
	    {
		namelen = strlen( module );
		if ( ( namelen < tasknamelen ) ||
		    stricmp( &module[ namelen-tasknamelen ], taskname ) )
		{
		    continue;
		}
	    }

	    result = 0;
	    if ( nummodules )
	    {
		( *nummodules )++;
	    }

	    hunk = 0;

	    while ( hunkaddr = ReadLong( &segnode->seg_Array[ hunk ].seg_Address ) )
	    {
		switch ( option )
		{
		    case HUNKFINDER_SHOW:
			PPrintf("%s_Hunk%ld at %lx\n", module, hunk, hunkaddr );
			break;

		    case HUNKFINDER_BINDFILL:
			table[ hunk ] = hunkaddr;
			/* FALL THROUGH !!! */

		    case HUNKFINDER_BIND:
			{
			    char hunksymbol[ MAXTOKENLEN ];
			    sprintf( hunksymbol, "%s_Hunk%ld", module, hunk );
 			    BindValue( hunksymbol, ACT_CONSTANT, hunkaddr );
			}
			break;
		}
		hunk++;
		if ( numhunks )
		{
		    ( *numhunks )++;
		}
	    }
	}
    }
    LinkUnlock();
    return( result );
}
