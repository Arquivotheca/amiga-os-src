/* -----------------------------------------------------------------------
 * gettimeofday.c   for as225's FTP  R2
 *
 * $Locker:  $
 *
 * $Id: gettimeofday.c,v 2.1 92/10/30 11:22:08 bj Exp $
 *
 * $Revision: 2.1 $
 *
 * $Log:	gettimeofday.c,v $
 * Revision 2.1  92/10/30  11:22:08  bj
 * 
 * Binary 2.3
 * 
 * Removed extra copyright notice. Registerized args.
 * 
 *
 * $Header: AS225:src/c/ftp/RCS/gettimeofday.c,v 2.1 92/10/30 11:22:08 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/* Copyright (C) 1990 by Commodore-Amiga */
#include <ss/socket.h>
#include <sys/time.h>
#include <config.h>
#include <exec/types.h>
#include <exec/lists.h>
#include <devices/timer.h>

#ifdef LATTICE
#include <proto/exec.h>
#endif
extern struct ExecBase *SysBase;


gettimeofday(REGISTER struct timeval *tp, REGISTER struct timezone *tzp)
{
    static struct timerequest t;    /* made static MMH */
    static struct timezone tz;
    short offset;

    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&t, 0L)) 
    {
        return -1;
    }

    t.tr_node.io_Message.mn_ReplyPort = CreateMsgPort();
    t.tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IORequest *)&t);
    DeleteMsgPort(t.tr_node.io_Message.mn_ReplyPort);
    CloseDevice((struct IORequest *)&t);
    t.tr_time.tv_secs += 252460800;    /* correct to 1 Jan 70 */
    *tp =  t.tr_time;

    if(( offset = get_tz() ) > 0)
    {
        tz.tz_minuteswest = offset;
        tz.tz_dsttime = 0;    /* ignore daylight savings time for now */
        t.tr_time.tv_secs += offset*60;    /* correct for timezone */
    } 
    else 
    {
        tz.tz_minuteswest = 0;
        tz.tz_dsttime = 0;
    }

    return(0);
}

