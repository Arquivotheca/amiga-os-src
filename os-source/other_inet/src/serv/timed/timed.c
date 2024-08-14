#include <ss/socket.h>
#include <dos/dos.h>
#include <proto/all.h>
#include <clib/alib_stdio_protos.h>
#include <string.h>
#include <sys/syslog.h>
#include "timed_rev.h"

#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME/A,SOCKPTR/A/N,ID/A/N,ARGS/F" CMDREV
#define OPT_NAME	0
#define OPT_SOCKPTR	1
#define OPT_ID		2
#define OPT_ARGS	3
#define OPT_COUNT	4

void machtime_dg(int);
void daytime_dg (int);
void machtime_stream(int);
void daytime_stream(int);

struct Library *SockBase ;
BPTR debugfh;

struct inetmsg {
	struct Message	msg;
	ULONG	id;
};

void main(void)
{
	int errno, s;
	long opts[OPT_COUNT];
	struct RDargs *rdargs;
	struct inetmsg inet_message;
	struct MsgPort *msgport, *replyport;

	memset((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts, NULL);

	if ((SockBase = OpenLibrary( "inet:libs/socket.library", 1L ))==NULL) {
		goto exit1;
	}
	setup_sockets(5,&errno);

	/* now get our socket */ 
	s = s_inherit((void *)*(long *)opts[OPT_SOCKPTR]);
	if(opts[OPT_ARGS] && *(char *)opts[OPT_ARGS]=='U') {
		if(stricmp((char *)opts[OPT_NAME],"time")) 
			daytime_dg(s);
		else
			machtime_dg(s);
	} else {
		if(stricmp((char *)opts[OPT_NAME],"time"))
			daytime_stream(s);
		else
			machtime_stream(s);
	} 
	s_close(s);

	/* if id was nonzero, then we have to inform inetd that we are done */
	if(inet_message.id = *(long *)opts[OPT_ID]) {
		replyport = CreateMsgPort();
		if(replyport==NULL) {
			s_syslog(LOG_ERR,"TIMED: Couldn't create reply port\n");
			goto exit3;
		}
		inet_message.msg.mn_Node.ln_Type = NT_MESSAGE;
		inet_message.msg.mn_Length = sizeof(struct inetmsg);
		inet_message.msg.mn_ReplyPort = replyport;

		Forbid();
		msgport = FindPort("inetd");
		if(msgport==NULL)  {
			Permit();
			s_syslog(LOG_ERR,"TIMED: Couldn't find inetd port\n");
			DeleteMsgPort(replyport);
			goto exit3;
		}
		PutMsg(msgport,(struct Message *)&inet_message);
		Permit();
		/* we can't exit until we received a reply */
		(void)WaitPort(replyport);
		DeleteMsgPort(replyport);
	}	

exit3:
	cleanup_sockets();
exit2:
	CloseLibrary(SockBase);
exit1:
	Close(debugfh);
	if(rdargs)
		FreeArgs(rdargs);
}


void daytime(char *str)
{
	struct DateTime dt;
	char day[20], date[20], time[20];

	dt.dat_Format = FORMAT_DOS;
	dt.dat_Flags = 0;
	dt.dat_StrDay = day;
	dt.dat_StrTime = time;
	dt.dat_StrDate = date;
	(void)DateStamp(&dt.dat_Stamp);
	(void)DateToStr(&dt);
	sprintf(str,"%s %s %s\r\n",day,date,time);
}


/* Return human-readable time of day */
void daytime_stream(int s)
{
	char buffer[256];
	daytime(buffer);
	(void)send(s, buffer, strlen(buffer),0);
}

void daytime_dg(int s)
{
	char buffer[256];
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	daytime(buffer);
	(void)sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}


/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since DateStamp
 * returns the number of seconds since midnight, Jan 1, 1978,
 * we must add 246144960 seconds.
 */


long machtime(void)
{
	struct DateStamp ds;
	long t;

	(void)DateStamp(&ds);
	t = ds.ds_Days*86400 + ds.ds_Minute*60 + (long)(ds.ds_Tick/50);
	t += 2461449600;
	return( t );
}


void machtime_stream(int s)
{
	long result;

	result = machtime();
	(void) send(s, (char *) &result, sizeof(result),0);
}

void machtime_dg(int s)
{
	long result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}
