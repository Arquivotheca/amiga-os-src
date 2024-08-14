/* Copyright (C) 1990 by Commodore-Amiga */

#include <ss/socket.h>

#include <sys/time.h>
#include <exec/types.h>
#include <exec/lists.h>
#include <devices/timer.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <proto/timer.h>
#endif

struct Library *TimerBase;
//struct Library *SockBase;

gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	struct timerequest t;

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, &t, 0L)) {
		return -1;
	}

   	TimerBase = (struct Library *)t.tr_node.io_Device;
    GetSysTime(&t.tr_time);

	tp->tv_sec  = t.tr_time.tv_secs + 252460800;	/* correct to 1 Jan 70 */
    tp->tv_usec = t.tr_time.tv_micro;
	tp->tv_sec += get_tz()*60;	/* correct for timezone */
	if(tzp) {
		tzp->tz_minuteswest = get_tz();
		tzp->tz_dsttime = 0;	/* ignore daylight savings time for now */
    }
	CloseDevice(&t);

	return(0);
}

/*
main()
{
	struct timeval t;
	struct timezone tz;

	if((SockBase=OpenLibrary("inet:libs/socket.library",0L))==NULL) {
		printf("Error opening socket library\n");
		Exit(10);
	}
	setup_sockets(5,&errno);

	gettimeofday(&t,&tz);
	printf("secs: %ld   micros:%ld\n",t.tv_sec,t.tv_usec);
	printf("minutes west: %ld   dst: %ld\n",tz.tz_minuteswest,tz.tz_dsttime);
	cleanup_sockets();
	CloseLibrary(SockBase);
}
 */