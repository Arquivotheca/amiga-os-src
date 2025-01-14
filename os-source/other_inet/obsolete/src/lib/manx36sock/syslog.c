/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 * Copyright 1989 Ameristar Technology, Inc
 */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it outputs to the
 * log file instead of the standard output.  Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by syslogd(8).
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <strings.h>

#include <exec/memory.h>

static int	LogStat = 0;		/* status bits, set by openlog() */
static char	*LogTag = "syslog";	/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static struct MsgPort *LogFile;		/* Task we send messages to */

syslog(pri, fmt, a, b, c, d, e, f, g, h, i)
	int pri;
	register char *fmt;
{
	static char fmt_cpy[1024], *ctime();
	extern void *AllocMem();
	struct syslog_am *sam;
	int pid, saved_errno;
	time_t now, time();
	extern int errno;
	register int cnt;
	register char *p;

	saved_errno = errno;

	/* 
	 * see if we should just throw out this message 
	 */
	if ((u_int)LOG_FAC(pri) >= LOG_NFACILITIES ||
	    !LOG_MASK(LOG_PRI(pri)) || (pri &~ (LOG_PRIMASK|LOG_FACMASK))){
		return;
	}
	if(!LogFile){
		openlog(LogTag, LogStat | LOG_NDELAY, 0);
		if(!LogFile){
			return;
		}
	}

	/* 
	 * set default facility if none specified 
	 */
	if ((pri & LOG_FACMASK) == 0){
		pri |= LogFacility;
	}

	/* 
	 * build the message 
	 */
	sam = (struct syslog_am *)AllocMem(sizeof(*sam), MEMF_PUBLIC|MEMF_CLEAR);
	if(!sam){
		return ;
	}
	sam->sam_msg.mn_Node.ln_Type = NT_MESSAGE;
	sam->sam_msg.mn_Length = sizeof(*sam);
	sam->sam_msg.mn_ReplyPort = 0;

	(void)time(&now);
	(void)sprintf(sam->sam_buf, "<%d>%.15s ", pri, ctime(&now) + 4);
	for (p = sam->sam_buf; *p; ++p){
		;
	}
	if (LogTag) {
		(void)strcpy(p, LogTag);
		for(; *p; ++p){
			;
		}
	}
	if (LogStat & LOG_PID) {
		(void)sprintf(p, "[%d]", getpid());
		for(; *p; ++p){
			;
		}
	}
	if (LogTag) {
		*p++ = ':';
		*p++ = ' ';
	}

	/* 
	 * substitute error message for %m 
	 */
	{
		register char ch, *t1, *t2;
		char *strerror();

		for (t1 = fmt_cpy; ch = *fmt; ++fmt)
			if (ch == '%' && fmt[1] == 'm') {
				++fmt;
				for (t2 = strerror(saved_errno);
				    *t1 = *t2++; ++t1);
			}
			else
				*t1++ = ch;
		*t1 = '\0';
	}

	(void)sprintf(p, fmt_cpy, a, b, c, d, e, f, g, h, i);

	/* 
	 * output the message to the local logger 
	 */
	PutMsg(LogFile, sam);
}

/*
 * OPENLOG -- open system log
 */
openlog(ident, logstat, logfac)
	char *ident;
	int logstat, logfac;
{
	extern struct MsgPort *FindPort();

	Forbid();
	LogFile = FindPort(SYSLOGPORT);
	Permit();
}

/*
 * CLOSELOG -- close the system log
 */
closelog()
{
	LogFile = 0;
}

static int	LogMask = 0xff;		/* mask of priorities to be logged */
/*
 * SETLOGMASK -- set the log mask level
 */
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0){
		LogMask = pmask;
	}
	return (omask);
}
