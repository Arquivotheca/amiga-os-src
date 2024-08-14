/*	touch.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 *	Update the modification time of a file to the present time.
 */

#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

extern struct MsgPort *CreatePort( UBYTE *name, long pri );
extern void DeletePort( struct MsgPort *io );

#include <time.h>

#include "make.h"

/* return the current time */
time_t
nowtime( void )
{
	struct DateStamp t;
	time_t outtime = 0L;

	DateStamp( &t );
	outtime = t.ds_Tick/TICKS_PER_SECOND + 60*t.ds_Minute + 86400*t.ds_Days;
	return( outtime );
}

static long
dos_packet( struct MsgPort *port, long type, 
long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7 )
{
	struct StandardPacket *sp;
	struct MsgPort *rp;
	long ret;

	if( !( rp = CreatePort( NULL, 0L )))
		return( 1 );
	if( !( sp = AllocMem( sizeof(*sp), MEMF_PUBLIC|MEMF_CLEAR))) {
		DeletePort( rp );
		return( 1 );
	}
	sp->sp_Msg.mn_Node.ln_Name = (char *)&sp->sp_Pkt;
	sp->sp_Pkt.dp_Link = &sp->sp_Msg;
	sp->sp_Pkt.dp_Port = rp;
	sp->sp_Pkt.dp_Type = type;
	sp->sp_Pkt.dp_Arg1 = arg1;
	sp->sp_Pkt.dp_Arg2 = arg2;
	sp->sp_Pkt.dp_Arg3 = arg3;
	sp->sp_Pkt.dp_Arg4 = arg4;
	sp->sp_Pkt.dp_Arg5 = arg5;
	sp->sp_Pkt.dp_Arg6 = arg6;
	sp->sp_Pkt.dp_Arg7 = arg7;
	PutMsg( port, &sp->sp_Msg );
	WaitPort( rp );
	GetMsg( rp );
	ret = sp->sp_Pkt.dp_Res1;
	FreeMem( (void *)sp, sizeof(*sp) );
	DeletePort(rp);
	return( ret );
}

/*	update the modification time of a file */
void
touch( const char *filename )
{
	struct MsgPort *task = NULL;
	BPTR lock = (BPTR)0L;
	BPTR plock = (BPTR)0L;
	char *bcplstring = NULL;
	struct DateStamp dateStamp;

	if( !( bcplstring = (char *)AllocMem(MAXPATHNAME, MEMF_PUBLIC)))
		goto death;
	if( !( task = (struct MsgPort *)DeviceProc( filename )))
		goto death;
	if( !( lock = Lock( filename, SHARED_LOCK )))
		goto death;
	plock = ParentDir( lock );
	UnLock( lock ); lock = NULL;

	/* Strip pathnames first */
	strcpy( bcplstring + 1, basename( filename ));
	*bcplstring = (char)strlen(bcplstring + 1);

	dos_packet( task, ACTION_SET_DATE, NULL, plock, (long)bcplstring >> 2,
		(long) DateStamp( &dateStamp ), 0L, 0L, 0L);

death:
	if( lock )
		UnLock( lock );
	if( plock )
		UnLock( plock );
	FreeMem((void *) bcplstring, 64L );
}
