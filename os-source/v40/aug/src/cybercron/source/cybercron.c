/* CyberCron.c

   Copyright © 1992 by Christopher A. Wichura (caw@miroc.chi.il.us).
   All rights reserved.
*/

struct RxsLib *RexxSysBase;
unsigned long ARexxLibCount = 0;

/*
 * here we have storage for the current crontab file, sendmail command
 * and the name of our log file
 */
UBYTE CronTabName[256];
UBYTE SendMailCmd[256];
UBYTE LogFile[256];

/*
 * these are used by the ParseEvent() routine when no priority or stack size
 * is specified.
 */
ULONG DefaultStackSize = 4096;
BYTE DefaultPriority = 0;

/*
 * this array is used to keep track of which job numbers are in use and
 * which ones are free.
 */
UBYTE Jobs[256];

/*
 * this global is the list header for all cybernodes.  we tell who added the
 * event (crontab or via a rexx command) by whether or not the CNB_CRONTAB
 * bit is set in the cn_Flags field.
 */
struct List EventList;

#define ESC "\x1B["
#define CYBERCRON ESC "1;33;42mCyberCron" ESC "0;32;40m"

#define ARG_TEMPLATE "CRONTAB/K,LOGFILE/K,SENDMAIL/K,DEFSTACK/K/N,DEFPRI/K/N,CRONPRI/K/N"
enum CmdlineReadArgs {
	ARG_CRONTAB,
	ARG_LOGFILE,
	ARG_SENDMAIL,
	ARG_STACK,
	ARG_PRI,
	ARG_CPRI,
	ARG_sizeof
};

/* extern references to our version and revision numbers */
extern ULONG __far Version;
extern ULONG __far Revision;
extern UBYTE __far VersionID[];

/* extern references to our help text and copyright */
extern UBYTE __far arg_help[];
extern UBYTE __far copyright[];

/* storage for the pointer to StdErr */
BPTR StdErr = NULL;

/* our old task priority */
WORD OldPriority = -1;

/* for our main ReadArgs call so we can free it later */
struct RDArgs *MyArgs = NULL;
struct RDArgs *ArgsPtr = NULL;
STRPTR WBArgs = NULL;

/* stuff used in launching/destroying jobs */
struct SignalSemaphore jobSema;
ULONG NumSystemJobs = 0;
ULONG NumARexxJobs = 0;

/* Semaphore to protect Log() being called under EndSystemJob() */
struct SignalSemaphore logSema;

/* stuff for our timer port */
struct MsgPort *TimerPort = NULL;
struct timerequest TimerIO;
BOOL TimerUP = FALSE;
BOOL DoingTimeRequest = FALSE;

/* stuff for our notify request */
struct NotifyRequest MyNotifyRequest;
UBYTE NotifySignal = -1;
BOOL NotifyUP = FALSE;

/* stuff for our rexx port */
struct MsgPort *RexxPort = NULL;

/* global flags */
BOOL BringerDown = FALSE;	/* trying to quit ? */
BOOL Suspended = FALSE;		/* currently suspended ? */

/* storage for our old pr_WindowPtr */
APTR OldWindowPtr;

/* specifies the maximum number of jobs for each of the queues */
struct JobQueue jobQueue[27];

/* this is our main routine */
int __regargs main(char *cmdptr, int cmdlen, struct WBStartup *WBMsg)
{
	char *ArgArray[ARG_sizeof];
	UBYTE TextBuf[256];
	ULONG NSignal, TSignal, RSignal;
	ULONG signals;
	ULONG numJobs;
	int index;
	BPTR lock;

	struct timeval tr_time;

	OldWindowPtr = ((struct Process *)FindTask(NULL))->pr_WindowPtr;
	((struct Process *)FindTask(NULL))->pr_WindowPtr = (APTR)-1;

	StdErr = ((struct Process *)FindTask(NULL))->pr_CES;
	if (StdErr == NULL)
		StdErr = Output();

	if (WBMsg) {
		if (!(WBArgs = WBtoCLIargs(WBMsg)))
			MyExit(5);
	}

	NewList(&EventList);
	InitSemaphore(&jobSema);
	InitSemaphore(&logSema);

	/* do the stuff needed to call ReadArgs to parse the command line */
	memset(ArgArray, 0, sizeof(ArgArray));

	if (!(MyArgs = (struct RDArgs *)AllocDosObject(DOS_RDARGS, TAG_DONE))) {
		ErrorMsg("Couldn't allocate RDArgs structure.\n");
		MyExit(5);
	}

	if (!(MyArgs->RDA_ExtHelp = (UBYTE *)AllocVec(strlen(arg_help) + strlen(copyright) + (2 * strlen(CYBERCRON)) + strlen(VersionID) + 1, MEMF_CLEAR))) {
		ErrorMsg("Out of memory!\n");
		MyExit(5);
	}

	MySPrintf((char *)MyArgs->RDA_ExtHelp, arg_help, CYBERCRON, VersionID, copyright, CYBERCRON);

	if (WBArgs) {
		MyArgs->RDA_Source.CS_Buffer = WBArgs;
		MyArgs->RDA_Source.CS_Length = strlen(WBArgs);
		MyArgs->RDA_Source.CS_CurChr = 0L;
	}

	/* now call ReadArgs to parse the command line */
	ArgsPtr = ReadArgs(ARG_TEMPLATE, (LONG *)&ArgArray, MyArgs);

	/* free the memory we used for this ReadArgs() call */
	FreeVec((char *)MyArgs->RDA_ExtHelp);
	FreeVec(WBArgs);
	WBArgs = NULL;

	if (!ArgsPtr) {
		Fault(IoErr(), NULL, TextBuf, sizeof(TextBuf));
		ErrorMsg("%s\n", TextBuf);
		MyExit(5);
	}

	if (ArgArray[ARG_CRONTAB])
		if (strlen(ArgArray[ARG_CRONTAB]) + 1 > sizeof(CronTabName)) {
			ErrorMsg("Crontab filename too long.\n");
			MyExit(5);
		} else
			strcpy(CronTabName, ArgArray[ARG_CRONTAB]);
	else
		strcpy(CronTabName, "S:CronTab");

	if (ArgArray[ARG_LOGFILE]) {
		if (strlen(ArgArray[ARG_LOGFILE]) + 1 > sizeof(LogFile)) {
			ErrorMsg("Log filename too long.\n");
			MyExit(5);
		} else
			strcpy(LogFile, ArgArray[ARG_LOGFILE]);
	}

	if (ArgArray[ARG_SENDMAIL]) {
		if (strlen(ArgArray[ARG_SENDMAIL]) + 1 > sizeof(SendMailCmd)) {
			ErrorMsg("SendMail command too long.\n");
			MyExit(5);
		} else
			strcpy(SendMailCmd, ArgArray[ARG_SENDMAIL]);
	}

	if (ArgArray[ARG_STACK])
		DefaultStackSize = *((LONG *)ArgArray[ARG_STACK]);
	else {
		/* if we have a cli attached to us then get the default
		   stack size out of it.  Otherwise leave it be.
		   WBtoCLIargs() will probably have set DefaultStackSize
		   for us already in such a case.  If not, the hard-coded
		   default of 4096 will be used */

		struct CommandLineInterface *cli;

		if (cli = Cli())
			DefaultStackSize = sizeof(LONG) * cli->cli_DefaultStack;
	}

	if (DefaultStackSize < 2048)
		DefaultStackSize = 2048;

	if (ArgArray[ARG_PRI])
		DefaultPriority = *((LONG *)ArgArray[ARG_PRI]) & 0xFF;

	if (ArgArray[ARG_CPRI])
		OldPriority = SetTaskPri(FindTask(NULL), *((LONG *)ArgArray[ARG_CPRI]) & 0xFF);

	/* open up our ARexx port */
	if (!(RexxPort = CreatePort("CYBERCRON", 0))) {
		ErrorMsg("Couldn't create ARexx port.\n");
		MyExit(5);
	}

	RSignal = 1L << RexxPort->mp_SigBit;

	/* open up the timer */
	if (!(TimerPort = CreatePort(NULL, 0))) {
		ErrorMsg("Couldn't create timer port.\n");
		MyExit(5);
	}

	if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&TimerIO, 0)) {
		ErrorMsg("Couldn't open %s.\n", TIMERNAME);
		MyExit(5);
	}

	TimerIO.tr_node.io_Message.mn_ReplyPort = TimerPort;
	TSignal = 1L << TimerPort->mp_SigBit;
	TimerUP = TRUE;

	if ((NotifySignal = AllocSignal(-1)) == -1) {
		ErrorMsg("Couldn't allocate notify signal.\n");
		MyExit(5);
	}

	NSignal = 1L << NotifySignal;

	memset((char *)&MyNotifyRequest, 0, sizeof(struct NotifyRequest));

	MyNotifyRequest.nr_Name = CronTabName;
	MyNotifyRequest.nr_Flags = NRF_SEND_SIGNAL;
	MyNotifyRequest.nr_stuff.nr_Signal.nr_Task = FindTask(NULL);
	MyNotifyRequest.nr_stuff.nr_Signal.nr_SignalNum = NotifySignal;

	if (StartNotify(&MyNotifyRequest) == DOSFALSE) {
		ErrorMsg("Couldn't start notification on crontab file.\n");
		MyExit(5);
	}
	NotifyUP = TRUE;

	ReadCronTab();

	/* initialize jobQueue Maximums */
	for (index = 0; index < 27; index++) {
		jobQueue[index].jq_Max = 1;
		jobQueue[index].jq_Cur = 0;
	}

	/* set queue 0 to 0xFFFFFFFF (infinite) so that jobs with no queue
	   specified will use one with no limits */
	jobQueue[0].jq_Max = 0xFFFFFFFF;

	/* print a banner to the world saying we've started */
	if (!WBMsg) {
		MySPrintf(TextBuf, "%s daemon v%s started.\n", CYBERCRON, VersionID);
		PutStr(TextBuf);
		PutStr(copyright);
		Flush(Output());
	}

	/* loop forever waiting for each minute and checking to see if we need
	   to do anything.  also look for break, notify, etc.  BringerDown
	   can be set by the ARexx SHUTDOWN command so check it as well.
	*/

	for (;BringerDown == FALSE;) {
		TimerIO.tr_node.io_Command = TR_ADDREQUEST;
		TimerIO.tr_time.tv_micro = 0;
		GetSysTime(&tr_time);
		TimerIO.tr_time.tv_secs = 60 - tr_time.tv_secs % 60;
		SetSignal(0L, TSignal);
		SendIO((struct IORequest *)&TimerIO);
		DoingTimeRequest = TRUE;

		signals = Wait(TSignal | NSignal | RSignal | SIGBREAKF_CTRL_C);

		if (signals & TSignal) {
			GetMsg(TimerPort);
			DoingTimeRequest = FALSE;
			if (Suspended == FALSE)
				ScanForJobs();
		}

		if (signals & NSignal)
			if (lock = Lock(CronTabName, ACCESS_READ)) {
				FreeEvents(TRUE);

				/* not really an error, but ReadCronTab() will send
				   messages along the lines of parse error in line #
				   if they occur so we spit this out to let them know
				   why they are getting these parse errors. */

				ErrorMsg("Crontab file modified, re-reading.\n");
				Log("Crontab file modified, re-reading.\n");

				ReadCronTab();
				UnLock(lock);
			}

		if (signals & RSignal)
			HandleRexxEvents();

		if (DoingTimeRequest) {
			DoingTimeRequest = FALSE;

			Disable();
			if (((struct Message *)&TimerIO)->mn_Node.ln_Type != NT_REPLYMSG)
				AbortIO((struct IORequest *)&TimerIO);
			Enable();

			WaitIO((struct IORequest *)&TimerIO);
		}

		if (signals & SIGBREAKF_CTRL_C)
			break;
	}

	BringerDown = TRUE;

	ObtainSemaphore(&jobSema);
	numJobs = NumSystemJobs + NumARexxJobs;
	ReleaseSemaphore(&jobSema);

	if (numJobs) {
		ErrorMsg("Waiting for all outstanding jobs to terminate.\n");

		for (;numJobs;) {
			TimerIO.tr_node.io_Command = TR_ADDREQUEST;
			TimerIO.tr_time.tv_secs = 15;
			TimerIO.tr_time.tv_micro = 0;
			SetSignal(0L, TSignal);
			SendIO((struct IORequest *)&TimerIO);
			DoingTimeRequest = TRUE;

			signals = Wait(TSignal | RSignal);

			if (signals & TSignal) {
				GetMsg(TimerPort);
				DoingTimeRequest = FALSE;
			}

			if (signals & RSignal)
				HandleRexxEvents();

			if (DoingTimeRequest) {
				DoingTimeRequest = FALSE;

				Disable();
				if (((struct Message *)&TimerIO)->mn_Node.ln_Type != NT_REPLYMSG)
					AbortIO((struct IORequest *)&TimerIO);
				Enable();

				WaitIO((struct IORequest *)&TimerIO);
			}

			ObtainSemaphore(&jobSema);
			numJobs = NumSystemJobs + NumARexxJobs;
			ReleaseSemaphore(&jobSema);
		}
	}

	MyExit(0);
}

/* loop through the event list looking for any jobs to start */

void ScanForJobs(void)
{
	struct CyberNode *cn, *tcn;

	UBYTE	DayOfWeek;
	UWORD	Month;
	ULONG	Day;
	ULONG	Hour;
	ULONG	Min;
	BOOL	WasDelayed;

	SystemTime_t st;
	static ULONG LastScan = 0;

	/* initilize the bit fields for us to do comparisons against */
	GetSystemTime(&st);

	/* figure out if the time has gone backwards.  This could happen if
	   the clock was reloaded, etc.  We use a 5 minute threshhold.  If
	   it goes back farther than that then we assume the user knows what
	   they are doing. */
	{	ULONG MSSC = st.st_tvsecs / 60;		/* MSSC stands for Minutes Since System Creation */
		if (MSSC >= (LastScan - 5) && MSSC <= LastScan)
			return;
		else
			LastScan = MSSC;
	}

	DayOfWeek = 1L << st.st_DOW;
	Month = 1L << st.st_Month;
	Day = 1L << st.st_Day;
	Hour = 1L << st.st_Hour;
	if (st.st_Min > 31)
		Min = 1L << (st.st_Min - 32);
	else
		Min = 1L << st.st_Min;

	/* loop through the list looking for events to do */
	for (cn = (struct CyberNode *)EventList.lh_Head; cn->cn_Node.ln_Succ;
		cn = (struct CyberNode *)cn->cn_Node.ln_Succ) {
			if (cn->cn_Flags & CNF_DELAYED)
				WasDelayed = TRUE;
			else
				WasDelayed = FALSE;

			if (cn->cn_DayOfWeek & DayOfWeek || WasDelayed)
				if (cn->cn_Month & Month || WasDelayed)
					if (cn->cn_Day & Day || WasDelayed)
						if (cn->cn_Hour & Hour || WasDelayed)
							if ((st.st_Min > 31 && cn->cn_HiMin & Min) ||
							    (st.st_Min < 32 && cn->cn_LoMin & Min) || WasDelayed) {
								ObtainSemaphore(&jobSema);
								if (jobQueue[cn->cn_ObeyQueue].jq_Cur < jobQueue[cn->cn_ObeyQueue].jq_Max) {
									if (cn->cn_Flags & CNF_REXX)
										StartRexxJob(cn);
									else
										StartSystemJob(cn);

									cn->cn_Flags &= ~CNF_DELAYED;

									if (cn->cn_Flags & CNF_EXECONE) {
										tcn = (struct CyberNode *)cn->cn_Node.ln_Pred;
										Remove((struct Node *)cn);
										FreeVec(cn);
										cn = tcn;
									}
								} else
									cn->cn_Flags |= CNF_DELAYED;

								ReleaseSemaphore(&jobSema);
							}
	}
}

void HandleRexxEvents(void)
{
	struct RexxMsg *msg;

	/* this is the table of ARexx commands that CyberCron knows.  the
	   format is as follows:

	   1) a short that is the length of the command name
	   2) a pointer to the command name
	   3) a short to descibe the args to pass to the function.
		value 0 = no args
		value 1 = pointer to string after command name
		value 2 = an integer
		value 3 = pointer to the current ARexx message
	   4) a short to describe the return value from the function
		value 0 = no returns, set rc to zero
		value 1 = return an argstring
		value 2 = return integer in rc
		value 3 = return an argstring already in argstring format
	   5) a pointer to the function
	*/

#define NUMRXCMDS 17
	static struct {short len; char *RxCmd; short args; short ret; APTR func;} CmdTbl[NUMRXCMDS] = {
		{8,  "SHUTDOWN", 0, 0, (APTR)&rx_Shutdown},
		{7,  "VERSION", 0, 1, (APTR)&rx_Version},
		{7,  "SUSPEND", 0, 0, (APTR)&rx_Suspend},
		{6,  "RESUME", 0, 0, (APTR)&rx_Resume},
		{14, "NEW_EVENT_FILE", 1, 2, (APTR)&rx_NewEventFile},
		{16, "CLOSE_EVENT_FILE", 0, 0, (APTR)&rx_CloseEventFile},
		{9,  "ADD_EVENT", 1, 2, (APTR)&rx_AddEvent},
		{11, "SHOW_STATUS", 0, 1, (APTR)&rx_ShowStatus},
		{17, "PURGE_REXX_EVENTS", 0, 0, (APTR)&rx_PurgeRexxEvents},
		{17, "DELETE_REXX_EVENT", 1, 2, (APTR)&rx_DeleteRexxEvent},
		{12, "DELETE_EVENT", 1, 2, (APTR)&rx_DeleteEvent},
		{11, "LIST_EVENTS", 0, 3, (APTR)&rx_ListEvents},
		{10, "SHOW_EVENT", 1, 1, (APTR)&rx_ShowEvent},
		{12, "NEW_LOG_FILE", 1, 2, (APTR)&rx_NewLogFile},
		{14, "CLOSE_LOG_FILE", 0, 0, (APTR)&rx_CloseLogFile},
		{13, "SET_QUEUE_MAX", 1, 2, (APTR)&rx_SetQueueMax},
		{13, "GET_QUEUE_MAX", 1, 2, (APTR)&rx_GetQueueMax}
	};

	/* if we can't get ahold of the rexx system library then we
	   spin emptying our message port by replying to everything.
	   shouldn't happen, but if some idiot tries sending us messages
	   when they don't have ARexx then its better safe than sorry */

	if (GetARexxLib() == FALSE) {
		ErrorMsg("Couldn't handle events on ARexx port -- no %s available!\n", RXSNAME);
		Log("Couldn't handle events on ARexx port -- no %s available!\n", RXSNAME);

		while ((msg = (struct RexxMsg *)GetMsg(RexxPort)))
			ReplyMsg((struct Message *)msg);

		return;
	}

	/* we've got the ARexx library so spin on our port looking for messages.
	   if its a reply then a command/string we launched has finished and ARexx
	   is returning its results to us.  Otherwise, it's a command we are to
	   execute so call DoMsg() to dispatch it */

	while ((msg = (struct RexxMsg *)GetMsg(RexxPort)))
		if (msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG) {
			if (!msg->rm_Args[2])
				Log("Job %ld ended.\n", (UWORD)msg->rm_Args[1]);

			Close(msg->rm_Stdout);
			Close(msg->rm_Stdin);
			FreeJobNum((UWORD)msg->rm_Args[1]);
			ObtainSemaphore(&jobSema);
			jobQueue[(int)msg->rm_Args[3]].jq_Cur--;
			ReleaseSemaphore(&jobSema);
			DeleteArgstring(msg->rm_Args[0]);
			DeleteRexxMsg(msg);
			NumARexxJobs--;
		} else
			DoMsg(msg, (APTR)&CmdTbl, NUMRXCMDS, BringerDown);

	FreeARexxLib();
}

void rx_Shutdown(void)
{
	BringerDown = TRUE;
}

STRPTR rx_Version(void)
{
	return VersionID;
}

void rx_Suspend(void)
{
	Suspended = TRUE;
}

void rx_Resume(void)
{
	Suspended = FALSE;
}

int rx_NewEventFile(STRPTR name)
{
	BPTR lock;

	if (lock = Lock(name, ACCESS_READ)) {
		if (strlen(name) + 1 > sizeof(CronTabName)) {
			ErrorMsg("Crontab filename too long.\n");
			UnLock(lock);
			return RC_WARN;
		}

		FreeEvents(TRUE);

		if (NotifyUP) {
			EndNotify(&MyNotifyRequest);
			NotifyUP = FALSE;
		}

		strcpy(CronTabName, name);

		memset((char *)&MyNotifyRequest, 0, sizeof(struct NotifyRequest));

		MyNotifyRequest.nr_Name = CronTabName;
		MyNotifyRequest.nr_Flags = NRF_SEND_SIGNAL;
		MyNotifyRequest.nr_stuff.nr_Signal.nr_Task = FindTask(NULL);
		MyNotifyRequest.nr_stuff.nr_Signal.nr_SignalNum = NotifySignal;

		if (StartNotify(&MyNotifyRequest) == DOSFALSE) {
			ErrorMsg("Couldn't start notification on crontab file.\n");
			UnLock(lock);
			strcpy(CronTabName, "<None>");
			return RC_ERROR;
		}
		NotifyUP = TRUE;

		/* again, not really an error */
		ErrorMsg("New crontab file specified, reading.\n");
		Log("New crontab file specified, reading.\n");

		ReadCronTab();
		UnLock(lock);
		return RC_OK;
	}

	return RC_WARN;
}


void rx_CloseEventFile(void)
{
	FreeEvents(TRUE);
	EndNotify(&MyNotifyRequest);
	NotifyUP = FALSE;
	strcpy(CronTabName, "<None>");
}

int rx_NewLogFile(STRPTR name)
{
	if (strlen(name) + 1 > sizeof(LogFile)) {
		ErrorMsg("Log filename too long.\n");
		return RC_WARN;
	}

	ObtainSemaphore(&logSema);
	strcpy(LogFile, name);
	ReleaseSemaphore(&logSema);

	return RC_OK;
}

void rx_CloseLogFile(void)
{
	ObtainSemaphore(&logSema);
	LogFile[0] = '\0';
	ReleaseSemaphore(&logSema);
}

STRPTR rx_ShowStatus(void)
{
	static UBYTE result[sizeof(CronTabName) + sizeof(LogFile)+ 16];

	MySPrintf(result, "%s %s %s", (Suspended ? "SUSPENDED" : "ACTIVE"), CronTabName, (LogFile[0] ? LogFile : "<None>"));

	return result;
}

void rx_PurgeRexxEvents(void)
{
	FreeEvents(FALSE);
}

int rx_AddEvent(STRPTR event)
{
	struct CyberNode *cn;

	if (cn = ParseEvent(event)) {
		AddTail(&EventList, (struct Node *)cn);
		return RC_OK;
	} else
		return RC_ERROR;
}

int rx_DeleteRexxEvent(STRPTR name)
{
	struct CyberNode *cn;

	if ((cn = FindEvent(name)) && !(cn->cn_Flags & CNF_CRONTAB)) {
		Remove((struct Node *)cn);
		FreeVec(cn);
		return RC_OK;
	}

	return RC_ERROR;
}

int rx_DeleteEvent(STRPTR name)
{
	struct CyberNode *cn;

	if (cn = FindEvent(name)) {
		Remove((struct Node *)cn);
		FreeVec(cn);
		return RC_OK;
	}

	return RC_ERROR;
}

STRPTR rx_ListEvents(void)
{
	struct CyberNode *cn;
	STRPTR	string, string2;
	ULONG	num;

	num = 0;
	for (cn = (struct CyberNode *)EventList.lh_Head; cn->cn_Node.ln_Succ;
		cn = (struct CyberNode *)cn->cn_Node.ln_Succ)
			num++;

	if (num == 0)
		return CreateArgstring("<None>", 6);

	if (!(string = CreateArgstring(NULL, num * 11)))
		return NULL;

	string2 = string;
	for (cn = (struct CyberNode *)EventList.lh_Head; cn->cn_Node.ln_Succ;
		cn = (struct CyberNode *)cn->cn_Node.ln_Succ) {
			MySPrintf(string2, "0x%08lx ", cn);
			string2 += 11;
		}

	*--string2 = '\0';

	return string;
}

UBYTE rxSE_Buf[1024 + 12];

STRPTR rx_ShowEvent(STRPTR name)
{
	struct CyberNode *cn;
	STRPTR	ptr;
	ULONG	Bits[2];

	if (!(cn = FindEvent(name)))
		return NULL;

	MySPrintf(rxSE_Buf, "0x%08lx ", cn);
	ptr = &rxSE_Buf[11];

	if (cn->cn_Name) {
		MySPrintf(ptr, ":NAME %s ", cn->cn_Name);
		ptr += strlen(ptr);
	}

	Bits[0] = cn->cn_LoMin, Bits[1] = cn->cn_HiMin;
	UnParseBits(Bits, ptr, 0, 59);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	Bits[1] = 0;

	Bits[0] = cn->cn_Hour;
	UnParseBits(Bits, ptr, 0, 23);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	Bits[0] = cn->cn_Day;
	UnParseBits(Bits, ptr, 1, 31);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	Bits[0] = cn->cn_Month;
	UnParseBits(Bits, ptr, 1, 12);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	Bits[0] = cn->cn_DayOfWeek;
	UnParseBits(Bits, ptr, 0, 6);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	if (cn->cn_Flags & CNF_EXECONE) {
		strcpy(ptr, ":EXECONCE ");
		ptr += 10;
	}

	if (cn->cn_Flags & CNF_NOLOG) {
		strcpy(ptr, ":NOLOG ");
		ptr += 7;
	}

	if (cn->cn_Flags & CNF_REXX) {
		strcpy(ptr, ":REXX ");
		ptr += 6;
	} else {
		if (cn->cn_Stack != DefaultStackSize) {
			MySPrintf(ptr, ":STACK %ld ", cn->cn_Stack);
			ptr += strlen(ptr);
		}

		if (cn->cn_Priority != DefaultPriority) {
			MySPrintf(ptr, ":PRI %ld ", cn->cn_Priority);
			ptr += strlen(ptr);
		}

		if (cn->cn_CustomShell) {
			MySPrintf(ptr, ":CUSTOMSH %s ", cn->cn_CustomShell);
			ptr += strlen(ptr);
		} else if (cn->cn_Flags & CNF_SYSSH) {
			strcpy(ptr, ":SYSSH ");
			ptr += 7;
		}
	}

	strcpy(ptr, cn->cn_Command);
	ptr += strlen(ptr);
	*ptr++ = ' ';

	if (cn->cn_ReDirIn) {
		MySPrintf(ptr, "< %s ", cn->cn_ReDirIn);
		ptr += strlen(ptr);
	}

	if (cn->cn_SendToUser && SendMailCmd[0]) {
		MySPrintf(ptr, ":MAILUSER %s ", cn->cn_SendToUser);
		ptr += strlen(ptr);
	} else {
		if (cn->cn_ReDirOut) {
			MySPrintf(ptr, "%s %s ", (cn->cn_Flags & CNF_OUTAPP ? ">>" : ">"), cn->cn_ReDirOut);
			ptr += strlen(ptr);
		}
	}

	if (cn->cn_ReDirErr) {
		MySPrintf(ptr, "2%s %s ", (cn->cn_Flags & CNF_ERRAPP ? ">>" : ">"), cn->cn_ReDirErr);
		ptr += strlen(ptr);
	}

	if (cn->cn_ObeyQueue) {
		MySPrintf(ptr, ":OBEYQUEUE %lc ", cn->cn_ObeyQueue + 'a' - 1);
		ptr += 13;
	}

	*--ptr = '\0';

	return rxSE_Buf;
}

int rx_SetQueueMax(STRPTR argline)
{
	int queueNum;

	if (isalpha(*argline)) {
		queueNum = tolower(*argline) - 'a' + 1;

		argline++;

		while (isspace(*argline++))
			;

		if (*--argline) {
			jobQueue[queueNum].jq_Max = atol(argline);
			return RC_OK;
		}
	}

	return RC_ERROR;
}

int rx_GetQueueMax(STRPTR argline)
{
	int queueNum;

	if (isalpha(*argline)) {
		queueNum = tolower(*argline) - 'a' + 1;
		return (int)jobQueue[queueNum].jq_Max;
	} else
		return -1;
}

STRPTR WBtoCLIargs(struct WBStartup *WBMsg)
{
	UBYTE *Argline, *ArglineSave, *SourcePtr;
	struct Library *IconBase;
	UBYTE tempChar;
	BOOL sawEqual;
	int index;
	ULONG size;
	BPTR oldDir;
	struct DiskObject *dob;

	if (!(IconBase = OpenLibrary("icon.library", 37)))
		return NULL;

	oldDir = CurrentDir(WBMsg->sm_ArgList[0].wa_Lock);

	if (!(dob = GetDiskObjectNew(WBMsg->sm_ArgList[0].wa_Name))) {
		CloseLibrary(IconBase);
		CurrentDir(oldDir);
		return NULL;
	}

	if (dob->do_ToolTypes)
		for (size = index = 0; dob->do_ToolTypes[index]; index++)
			size += strlen(dob->do_ToolTypes[index]) + 1;
	else
		size = 0;

	if (Argline = AllocVec(size + 2, MEMF_CLEAR)) {
		ArglineSave = Argline;

		if (dob->do_ToolTypes)
			for (index = 0; dob->do_ToolTypes[index]; index++) {
				SourcePtr = dob->do_ToolTypes[index];
				sawEqual = 0;
				while (tempChar = *SourcePtr++) {
					if (tempChar == '=' && !sawEqual) {
						tempChar = ' ';
						sawEqual = 1;
					}
					*Argline++ = tempChar;
				}
				*Argline++ = ' ';
			}

		*Argline++ = '\n';
		*Argline = '\0';
	} else
		ArglineSave = NULL;

	if (dob->do_StackSize)
		DefaultStackSize = dob->do_StackSize;

	FreeDiskObject(dob);
	CloseLibrary(IconBase);
	CurrentDir(oldDir);
	return ArglineSave;
}

void MyExit(int error)
{
	if (OldPriority != -1)
		SetTaskPri(FindTask(NULL), OldPriority);

	if (ARexxLibCount)
		CloseLibrary((struct Library *)RexxSysBase);

	FreeEvents(TRUE);
	FreeEvents(FALSE);

	if (NotifyUP)
		EndNotify(&MyNotifyRequest);

	if (NotifySignal != -1)
		FreeSignal(NotifySignal);

	if (TimerUP) {
		if (DoingTimeRequest) {
			Disable();
			if (((struct Message *)&TimerIO)->mn_Node.ln_Type != NT_REPLYMSG)
				AbortIO((struct IORequest *)&TimerIO);
			Enable();

			WaitIO((struct IORequest *)&TimerIO);
		}

		CloseDevice((struct IORequest *)&TimerIO);
	}

	if (TimerPort)
		DeletePort(TimerPort);

	if (RexxPort)
		DeletePort(RexxPort);

	if (ArgsPtr)
		FreeArgs(ArgsPtr);

	if (MyArgs)
		FreeDosObject(DOS_RDARGS, MyArgs);

	if (WBArgs)
		FreeVec(WBArgs);

	((struct Process *)FindTask(NULL))->pr_WindowPtr = OldWindowPtr;

	XCEXIT(error);
}

/*
 * this routine will read the crontab file, calling ParseEvent() to create
 * CyberNodes, and then link them into the event list.
 * 
 */

UBYTE RCT_Buf[1024];

void ReadCronTab(void)
{
	BPTR fh;
	struct CyberNode *cn;
	ULONG line = 0;
	LONG error;

	if (!(fh = Open(CronTabName, MODE_OLDFILE))) {
		ErrorMsg("Couldn't open file \"%s\"\n", CronTabName);
		Log("Couldn't open file \"%s\"\n", CronTabName);
		return;
	}

	while (FGets(fh, RCT_Buf, sizeof(RCT_Buf))) {
		line++;

		if (RCT_Buf[0] == '#' || RCT_Buf[0] == '\n')
			continue;

		if (cn = ParseEvent(RCT_Buf)) {
			cn->cn_Flags |= CNF_CRONTAB;
			AddTail(&EventList, (struct Node *) cn);
		} else {
			ErrorMsg("Error parsing line %ld in file \"%s\"\n", line, CronTabName);
			Log("Error parsing line %ld in file \"%s\"\n", line, CronTabName);
		}
	}

	error = IoErr();

	if (error) {
		Fault(error, NULL, RCT_Buf, sizeof(RCT_Buf) - 1);
		ErrorMsg("I/O Error #%ld (%s) reading line %ld in file \"%s\"\n", error, RCT_Buf, line + 1, CronTabName);
		Log("I/O Error #%ld (%s) reading line %ld in file \"%s\"\n", error, RCT_Buf, line + 1, CronTabName);
	}

	Close(fh);
}

/*
 * this routine will parse an ASCII string and make a CyberNode out of it.
 * it returns NULL if there was an error during the parse.  otherwise it
 * returns a pointer to the node.
 * 
 * note that we do something really sneaky here.  we use the ReadArgs() routine
 * to do the initial parse!.  This means that the order in which items occur
 * in a crontab entry can be virtually anything the user desires!
 */
#define PE_TEMPLATE "Event/M,</K,>/K,>>/K,2>/K,2>>/K,:NAME/K,:STACK/K/N,:PRI/K/N,:CUSTOMSH/K,:MAILUSER/K,:OBEYQUEUE/K,:SYSSH/S,:REXX/S,:NOLOG/S,:EXECONCE/S"

enum ParseEventReadArgs {
	PEARG_EVENT,
	PEARG_REDIRIN,
	PEARG_REDIROUT,
	PEARG_REDIROUT2,
	PEARG_REDIRERR,
	PEARG_REDIRERR2,
	PEARG_REXXNAME,
	PEARG_STACK,
	PEARG_PRI,
	PEARG_CUSTOMSH,
	PEARG_MAILUSER,
	PEARG_OBEYQUEUE,
	PEARG_SYSSH,
	PEARG_REXX,
	PEARG_NOLOG,
	PEARG_EXECONE,
	PEARG_sizeof
};

enum Events {
	EVENT_MINUTE,
	EVENT_HOUR,
	EVENT_DAY,
	EVENT_MONTH,
	EVENT_DOW,
	EVENT_COMMAND,
	EVENT_ARGS
};

UBYTE PE_Buf[sizeof(RCT_Buf) + 4];

struct CyberNode *ParseEvent(STRPTR event)
{
	struct CyberNode *cn;
	struct RDArgs *PArgsPtr;
	struct RDArgs *PMyArgs;
	char *ArgArray[PEARG_sizeof];

	register char **EventArgs;
	register int index;
	ULONG size;
	ULONG Bits[2];

	/* allocate our RDArgs structure */
	if (!(PMyArgs = (struct RDArgs *)AllocDosObject(DOS_RDARGS, TAG_DONE)))
		return (struct CyberNode *)NULL;

	PMyArgs->RDA_Flags |= RDAF_NOPROMPT;

	/* set up the buffer for our ReadArgs() call.  We have to copy
	   over the string and put a new line at the end of it because
	   of a limitation of ReadArgs().  sigh. */

	{
		ULONG length;

		length = strlen(event);
		if (length + 2 > sizeof(PE_Buf)) {
			FreeDosObject(DOS_RDARGS, PMyArgs);
			return (struct CyberNode *)NULL;
		}

		CopyMem(event, PE_Buf, length);

		PE_Buf[length++] = '\n';
		PE_Buf[length] = '\0';

		PMyArgs->RDA_Source.CS_Buffer = PE_Buf;
		PMyArgs->RDA_Source.CS_Length = length;
		PMyArgs->RDA_Source.CS_CurChr = 0L;
	}

    /*
     * here we walk through the event line to make sure it isnt all blank.
     */

	while (isspace(*event))
		event++;

	if (!*event) {
		FreeDosObject(DOS_RDARGS, PMyArgs);
		return (struct CyberNode *)NULL;
	}

	memset(ArgArray, 0, sizeof(ArgArray));

	/* now we call ReadArgs() */
	PArgsPtr = ReadArgs(PE_TEMPLATE, (LONG *)&ArgArray, PMyArgs);

	if (!PArgsPtr) {
		FreeDosObject(DOS_RDARGS, PMyArgs);
		return (struct CyberNode *)NULL;
	}

	/* if they specified a name to be known as via the rexx port, make
	   sure it doesn't start with 0x because that's what we use to
	   prefix a hex number for nodes with no name and we don't want
	   the user fooling around with names we consider private. */

	if (ArgArray[PEARG_REXXNAME])
		if (ArgArray[PEARG_REXXNAME][0] == '0' && tolower(ArgArray[PEARG_REXXNAME][1]) == 'x') {
			FreeArgs(PArgsPtr);
			FreeDosObject(DOS_RDARGS, PMyArgs);
			return (struct CyberNode *)NULL;
		}


    /*
     * ok, ReadArgs has parsed the event for us.  make sure that we have
     * at least 5 time specs and a command name.
     */
	EventArgs = (char **) ArgArray[PEARG_EVENT];

	for (index = EVENT_MINUTE; index < EVENT_COMMAND; index++, EventArgs++)
		if (!*EventArgs || !isdigit(*EventArgs[0]) && *EventArgs[0] != '*') {
			FreeArgs(PArgsPtr);
			FreeDosObject(DOS_RDARGS, PMyArgs);
			return (struct CyberNode *)NULL;
		}

    /*
     * we have the five time spec strings.  now check to make sure we have a
     * command name.  we will also calculate its size as well as the size of
     * any args for the command while we are at it
     */

	if (!*EventArgs) {
		FreeArgs(PArgsPtr);
		FreeDosObject(DOS_RDARGS, PMyArgs);
		return (struct CyberNode *)NULL;
	}

	size = strlen(*EventArgs++) + 1;

	while (*EventArgs)
		size += strlen(*EventArgs++) + 1;

    /*
     * now figure out the memory needed to store the other textual items for
     * this node
     */

	if (ArgArray[PEARG_REDIRIN])
		size += strlen(ArgArray[PEARG_REDIRIN]) + 1;

	if (ArgArray[PEARG_REDIROUT])
		size += strlen(ArgArray[PEARG_REDIROUT]) + 1;
	if (ArgArray[PEARG_REDIROUT2]) {
		size += strlen(ArgArray[PEARG_REDIROUT2]) + 1;
		if (ArgArray[PEARG_REDIROUT])
			size -= strlen(ArgArray[PEARG_REDIROUT]) + 1;
	}

	if (ArgArray[PEARG_REDIRERR])
		size += strlen(ArgArray[PEARG_REDIRERR]) + 1;
	if (ArgArray[PEARG_REDIRERR2]) {
		size += strlen(ArgArray[PEARG_REDIRERR2]) + 1;
		if (ArgArray[PEARG_REDIRERR])
			size -= strlen(ArgArray[PEARG_REDIRERR]) + 1;
	}

	if (ArgArray[PEARG_REXXNAME])
		size += strlen(ArgArray[PEARG_REXXNAME]) + 1;
	if (ArgArray[PEARG_CUSTOMSH])
		size += strlen(ArgArray[PEARG_CUSTOMSH]) + 1;

	if (ArgArray[PEARG_MAILUSER])
		size += strlen(ArgArray[PEARG_MAILUSER]) + 1;

	if (ArgArray[PEARG_OBEYQUEUE] && !isalpha(ArgArray[PEARG_OBEYQUEUE][0])) {
		FreeArgs(PArgsPtr);
		FreeDosObject(DOS_RDARGS, PMyArgs);
		return (struct CyberNode *)NULL;
	}

	if (!(cn = (struct CyberNode *) AllocVec(size + sizeof(struct CyberNode) + 1, MEMF_CLEAR))) {
		FreeArgs(PArgsPtr);
		FreeDosObject(DOS_RDARGS, PMyArgs);
		return (struct CyberNode *)NULL;
	}

    /*
     * now that we have got the memory for the CyberNode start filling it
     * in.  we start by testing the STACK and PRI fields of the arg list and
     * use Atol() to get their values if present.  We then test the REXX and
     * NOLOG flags and use them to set the cn_Flags element.
     */

	if (ArgArray[PEARG_STACK])
		cn->cn_Stack = *((LONG *)ArgArray[PEARG_STACK]);
	if (cn->cn_Stack < 2048)
		cn->cn_Stack = DefaultStackSize;

	if (ArgArray[PEARG_PRI])
		cn->cn_Priority = *((LONG *)ArgArray[PEARG_PRI]) & 0xFF;
	else
		cn->cn_Priority = DefaultPriority;

	if (ArgArray[PEARG_OBEYQUEUE])
		cn->cn_ObeyQueue = tolower(ArgArray[PEARG_OBEYQUEUE][0]) - 'a' + 1;

	if (ArgArray[PEARG_REXX])
		cn->cn_Flags |= CNF_REXX;

	if (ArgArray[PEARG_NOLOG])
		cn->cn_Flags |= CNF_NOLOG;

	if (ArgArray[PEARG_SYSSH])
		cn->cn_Flags |= CNF_SYSSH;

	if (ArgArray[PEARG_EXECONE])
		cn->cn_Flags |= CNF_EXECONE;

    /*
     * now prepare to copy the textual items over into memory behind the
     * CyberNode
     */

	event = (char *) cn + sizeof(struct CyberNode);

	if (ArgArray[PEARG_REXXNAME]) {
		cn->cn_Name = event;
		size = strlen(ArgArray[PEARG_REXXNAME]) + 1;
		CopyMem(ArgArray[PEARG_REXXNAME], event, size);
		event += size;
	}
	if (ArgArray[PEARG_REDIRIN]) {
		cn->cn_ReDirIn = event;
		size = strlen(ArgArray[PEARG_REDIRIN]) + 1;
		CopyMem(ArgArray[PEARG_REDIRIN], event, size);
		event += size;
	}
	if (ArgArray[PEARG_REDIROUT] && !ArgArray[PEARG_REDIROUT2]) {
		cn->cn_ReDirOut = event;
		size = strlen(ArgArray[PEARG_REDIROUT]) + 1;
		CopyMem(ArgArray[PEARG_REDIROUT], event, size);
		event += size;
	}
	if (ArgArray[PEARG_REDIROUT2]) {
		cn->cn_ReDirOut = event;
		size = strlen(ArgArray[PEARG_REDIROUT2]) + 1;
		CopyMem(ArgArray[PEARG_REDIROUT2], event, size);
		event += size;
		cn->cn_Flags |= CNF_OUTAPP;
	}
	if (ArgArray[PEARG_REDIRERR] && !ArgArray[PEARG_REDIRERR2]) {
		cn->cn_ReDirErr = event;
		size = strlen(ArgArray[PEARG_REDIRERR]) + 1;
		CopyMem(ArgArray[PEARG_REDIRERR], event, size);
		event += size;
	}
	if (ArgArray[PEARG_REDIRERR2]) {
		cn->cn_ReDirErr = event;
		size = strlen(ArgArray[PEARG_REDIRERR2]) + 1;
		CopyMem(ArgArray[PEARG_REDIRERR2], event, size);
		event += size;
		cn->cn_Flags |= CNF_ERRAPP;
	}
	if (ArgArray[PEARG_CUSTOMSH]) {
		cn->cn_CustomShell = event;
		size = strlen(ArgArray[PEARG_CUSTOMSH]) + 1;
		CopyMem(ArgArray[PEARG_CUSTOMSH], event, size);
		event += size;
	}
	if (ArgArray[PEARG_MAILUSER]) {
		cn->cn_SendToUser = event;
		size = strlen(ArgArray[PEARG_MAILUSER]) + 1;
		CopyMem(ArgArray[PEARG_MAILUSER], event, size);
		event += size;
	}

	EventArgs = (char **) ArgArray[PEARG_EVENT];
	cn->cn_Command = event;
	index = EVENT_COMMAND;
	size = strlen(EventArgs[index]);
	CopyMem(EventArgs[index++], event, size);
	event += size;
	*event++ = ' ';

	if (EventArgs[index])
		while (EventArgs[index]) {
			size = strlen(EventArgs[index]);
			CopyMem(EventArgs[index++], event, size);
			event += size;
			*event++ = ' ';
		}

	*--event = 0;

    /*
     * Now we need to convert the ASCII time values into bitmaps to store in
     * the node.  Note that we do not check to see if the time strings are
     * within range or not.  We simply logically AND away any invalid bits
     * and use whats left.
     */

	ParseBits(Bits, EventArgs[EVENT_MINUTE]);
	cn->cn_LoMin = Bits[0], cn->cn_HiMin = Bits[1] & 0xFFFFFFF;

	ParseBits(Bits, EventArgs[EVENT_HOUR]);
	cn->cn_Hour = Bits[0] & 0xFFFFFF;

	ParseBits(Bits, EventArgs[EVENT_DAY]);
	cn->cn_Day = Bits[0] & 0xFFFFFFFE;

	ParseBits(Bits, EventArgs[EVENT_MONTH]);
	cn->cn_Month = Bits[0] & 0x1FFE;

	ParseBits(Bits, EventArgs[EVENT_DOW]);
	cn->cn_DayOfWeek = Bits[0] & 0x7F;

	FreeArgs(PArgsPtr);
	FreeDosObject(DOS_RDARGS, PMyArgs);
	return cn;
}

/*
 * this will take an ASCII time string and convert it into a bitmap for
 * storage in a CyberNode
 */
void ParseBits(ULONG *bits, STRPTR tstr)
{
	register char *ptr;
	int start, end;
	int save;

	if (*tstr == '*') {
		bits[0] = bits[1] = 0xFFFFFFFF;
		return;
	} else
		bits[0] = bits[1] = 0;

	for (;;) {
		ptr = tstr;
		while (isdigit(*ptr))
			ptr++;

		save = *ptr, *ptr = NULL;
		end = start = atol(tstr);

		if (save == '-') {
			tstr = ++ptr;

			while (isdigit(*ptr))
				ptr++;

			save = *ptr, *ptr = NULL;
			end = atol(tstr);
		}
		if (start >= 0 && end >= start)
			while (start <= end) {
				if (start >= 64)
					break;

				if (start < 32)
					bits[0] |= 1L << start;
				else
					bits[1] |= 1L << (start - 32);

				start++;
			}

		if (!save)
			break;
		else
			tstr = ptr + 1;
	}
}

/* convert a bit field back into an ASCII time string */
void UnParseBits(ULONG *bits, STRPTR ptr, int lowBit, int hiBit)
{
	STRPTR tptr;
	int curBit, startBit;
	BOOL isOn, lastOn;

	/* first check to see if everything is specified and return "*" if so */
	for (curBit = lowBit; curBit <= hiBit; curBit++)
		if ((curBit < 32 && !(bits[0] & 1L << curBit)) || (curBit > 31 && !(bits[1] & 1L << (curBit - 32))))
			break;

	if (curBit == hiBit + 1) {
		strcpy(ptr, "*");
		return;
	}

	/* it's not "*" so walk through and build things the hard way */
	tptr = ptr;
	*tptr = 0;
	lastOn = FALSE;

	for (curBit = lowBit; curBit < hiBit + 2; curBit++) {
		if ((curBit < 32 && (bits[0] & 1L << curBit)) || (curBit > 31 && (bits[1] & 1L << (curBit - 32))))
			isOn = TRUE;
		else
			isOn = FALSE;

		if (isOn & !lastOn) {
			MySPrintf(tptr, "%ld", curBit);
			startBit = curBit;
		}

		if (!isOn)
			if (lastOn && startBit != curBit - 1)
				MySPrintf(tptr, "-%ld,", curBit - 1);
			else
				if (tptr > ptr && *(tptr - 1) != ',')
					strcpy(tptr, ",");

		tptr += strlen(tptr);
		lastOn = isOn;
	}

	if (tptr == ptr)
		strcpy(ptr, "*");	/* Uh oh.  Somehow we have a field with nothing specified.  Fill it in with a "*" */

	else
		*--tptr = '\0';

	return;
}

/* find a specific CyberNode by name */

struct CyberNode *FindEvent(STRPTR name)
{
	struct CyberNode *cn;
	struct CyberNode *eventAddr;

	if (!name || name[0] == '\0')
		return (struct CyberNode *)NULL;

	if (name[0] == '0' && tolower(name[1]) == 'x')
		stch_l(&name[2], (long *)&eventAddr);
	else
		eventAddr = 0;

	for (cn = (struct CyberNode *)EventList.lh_Head; cn->cn_Node.ln_Succ;
		cn = (struct CyberNode *)cn->cn_Node.ln_Succ)
			if (cn == eventAddr || (cn->cn_Name && stricmp(name, cn->cn_Name) == 0))
				return cn;

	return (struct CyberNode *)NULL;
}

/* this routine will walk through the even list and free all the nodes in
   it of a given type.  If called with TRUE it will free crontab entries,
   otherwise it will free Rexx entries
*/

void FreeEvents(BOOL DoCronTabEntries)
{
	register struct CyberNode *cn;
	register struct CyberNode *tcn;
	UBYTE Flags;

	for (cn = (struct CyberNode *)EventList.lh_Head; cn->cn_Node.ln_Succ;
		cn = (struct CyberNode *)cn->cn_Node.ln_Succ) {
			Flags = cn->cn_Flags & CNF_CRONTAB;

			if ((DoCronTabEntries && Flags) || (!DoCronTabEntries && !Flags)) {
				tcn = (struct CyberNode *)cn->cn_Node.ln_Pred;
				Remove((struct Node *)cn);
				FreeVec(cn);
				cn = tcn;
			}
	}
}

/* this allocates an output filehandle for us which output will be piped through to sendmail over */
UBYTE SSM_Buf[sizeof(SendMailCmd) * 2];
UBYTE SSM_Buf2[50];

BPTR SetupSendMail(STRPTR cmdName, STRPTR userName)
{
	BPTR pfho, pfhi, ofh;
	UBYTE pipeName[36];

	struct timeval tr_time;

	GetSysTime(&tr_time);

	MySPrintf(pipeName, "PIPE:CyberCron.%08lx.%08lx", tr_time.tv_secs, tr_time.tv_micro);

	MySPrintf(SSM_Buf2, "CyberCron daemon v%s", VersionID);
	MySPrintf(SSM_Buf, SendMailCmd, "cybercron", SSM_Buf2);

	ofh = Open("NIL:", MODE_NEWFILE);
	if (!ofh)
		return NULL;

	pfho = Open(pipeName, MODE_NEWFILE);
	if (!pfho) {
		Close(ofh);
		return NULL;
	}

	pfhi = Open(pipeName, MODE_OLDFILE);
	if (!pfhi) {
		Close(pfho);
		Close(ofh);
		return NULL;
	}

	if (SystemTags(SSM_Buf, SYS_Input, pfhi, SYS_Output, ofh, SYS_Asynch, TRUE,
	    NP_StackSize, 25000, NP_CopyVars, TRUE, NP_Cli, TRUE, TAG_DONE) == -1) {
		Close(pfho);
		Close(pfhi);
		Close(ofh);
		return NULL;
	}

	FPrintf(pfho, "To: %s\nSubject: Output of \"%s\"\n\n", userName, cmdName);
	Flush(pfho);

	return pfho;
}

/* this routine will start up a job using System() */
void StartSystemJob(struct CyberNode *cn)
{
	BPTR	ifh, ofh /*, efh */;
	struct SystemECdata *ecdata;

	struct TagItem tlist[20];
	int tlistIdx = 0;

	if (!(ecdata = AllocVec(sizeof(struct SystemECdata), MEMF_CLEAR)))
		return;

	ecdata->jobNo = GetJobNum();
	if (!ecdata->jobNo) {
		Log("Job table full trying to start \"%s\"\n", cn->cn_Command);
		FreeVec(ecdata);
		return;
	}

	if (cn->cn_ReDirIn) {
		ifh = Open(cn->cn_ReDirIn, MODE_OLDFILE);
		if (!ifh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "input", cn->cn_Command);
			FreeJobNum(ecdata->jobNo);
			FreeVec(ecdata);
			return;
		}
	} else {
		ifh = Open("NIL:", MODE_OLDFILE);
		if (!ifh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "input", cn->cn_Command);
			FreeJobNum(ecdata->jobNo);
			FreeVec(ecdata);
			return;
		}
	}

	tlist[tlistIdx].ti_Tag = SYS_Input;
	tlist[tlistIdx++].ti_Data = (ULONG)ifh;

	if (cn->cn_SendToUser && SendMailCmd[0]) {
		ofh = SetupSendMail(cn->cn_Command, cn->cn_SendToUser);
		if (!ofh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
			Close(ifh);
			FreeJobNum(ecdata->jobNo);
			FreeVec(ecdata);
			return;
		}
	} else {
		if (cn->cn_ReDirOut) {
			ofh = Open(cn->cn_ReDirOut, (cn->cn_Flags & CNF_OUTAPP ? MODE_READWRITE : MODE_NEWFILE));
			if (!ofh) {
				Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
				Close(ifh);
				FreeJobNum(ecdata->jobNo);
				FreeVec(ecdata);
				return;
			}
		} else {
			ofh = Open("NIL:", MODE_NEWFILE);
			if (!ofh) {
				Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
				Close(ifh);
				FreeJobNum(ecdata->jobNo);
				FreeVec(ecdata);
				return;
			}
		}

		if (cn->cn_Flags & CNF_OUTAPP)
			Seek(ofh, 0, OFFSET_END);
	}

	tlist[tlistIdx].ti_Tag = SYS_Output;
	tlist[tlistIdx++].ti_Data = (ULONG)ofh;

	tlist[tlistIdx].ti_Tag = SYS_Asynch;
	tlist[tlistIdx++].ti_Data = TRUE;

/*
	Sigh..  Randell tells me that StdErr is pretty much unofficially "not for use"
	under 2.0.  This is here for the day that it can be used.  All that should need
	to be done is to pull out the comments.  Oh well.

	Do not uncomment this code.  NP_Error and NP_ErrorClose are __IGNORED__ by the
	system right now so if you specify stderr redirection, it will open the file and never
	close it.

	if (cn->cn_ReDirErr) {
		efh = Open(cn->cn_ReDirErr, (cn->cn_Flags & CNF_ERRAPP ? MODE_READWRITE : MODE_NEWFILE));
		if (!efh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "error", cn->cn_Command);
			Close(ofh);
			Close(ifh);
			FreeJobNum(ecdata->jobNo);
			FreeVec(ecdata);
			return;
		}

		if (cn->cn_Flags & CNF_ERRAPP)
			Seek(efh, 0, OFFSET_END);

		tlist[tlistIdx].ti_Tag = NP_Error;
		tlist[tlistIdx++].ti_Data = (ULONG)efh;
		tlist[tlistIdx].ti_Tag = NP_CloseError;
		tlist[tlistIdx++].ti_Data = TRUE;
	} else {
		tlist[tlistIdx].ti_Tag = NP_Error;
		tlist[tlistIdx++].ti_Data = (ULONG)StdErr;
		tlist[tlistIdx].ti_Tag = NP_CloseError;
		tlist[tlistIdx++].ti_Data = FALSE;
	}
*/
	tlist[tlistIdx].ti_Tag = NP_StackSize;
	tlist[tlistIdx++].ti_Data = (ULONG)cn->cn_Stack;

	tlist[tlistIdx].ti_Tag = NP_Priority;
	tlist[tlistIdx++].ti_Data = (ULONG)cn->cn_Priority;

	if (cn->cn_CustomShell) {
		tlist[tlistIdx].ti_Tag = SYS_CustomShell;
		tlist[tlistIdx++].ti_Data = (ULONG)cn->cn_CustomShell;
	} else {
		tlist[tlistIdx].ti_Tag = SYS_UserShell;
		tlist[tlistIdx++].ti_Data = ((cn->cn_Flags & CNF_SYSSH) ? FALSE : TRUE);
	}

	tlist[tlistIdx].ti_Tag = NP_Cli;
	tlist[tlistIdx++].ti_Data = TRUE;

	tlist[tlistIdx].ti_Tag = NP_CopyVars;
	tlist[tlistIdx++].ti_Data = TRUE;

	tlist[tlistIdx].ti_Tag = NP_ExitCode;
	tlist[tlistIdx++].ti_Data = (ULONG)&EndSystemJob;

	tlist[tlistIdx].ti_Tag = NP_ExitData;
	tlist[tlistIdx++].ti_Data = (ULONG)ecdata;

	tlist[tlistIdx].ti_Tag = TAG_DONE;
	tlist[tlistIdx].ti_Data = 0;

	ecdata->queueNo = cn->cn_ObeyQueue;
	ecdata->flags = cn->cn_Flags & CNF_NOLOG;

	if (SystemTagList(cn->cn_Command, tlist) == -1) {
		Log("System() failed to start \"%s\"\n", cn->cn_Command);
/*
		See above for why this is currently commented out.

		if (cn->cn_ReDirErr)
			Close(efh);
*/
		Close(ofh);
		Close(ifh);
		FreeJobNum(ecdata->jobNo);
		FreeVec(ecdata);
		return;
	}

	if (!(cn->cn_Flags & CNF_NOLOG))
		Log("Job %ld started:  \"%s\"\n", ecdata->jobNo, cn->cn_Command);

	ObtainSemaphore(&jobSema);
	NumSystemJobs++;
	jobQueue[cn->cn_ObeyQueue].jq_Cur++;
	ReleaseSemaphore(&jobSema);
}

int __saveds __asm EndSystemJob(register __d0 int rc, register __d1 struct SystemECdata *data)
{
	if (!data->flags)
		Log("Job %ld ended.\n", data->jobNo);

	FreeJobNum(data->jobNo);

	ObtainSemaphore(&jobSema);
	NumSystemJobs--;
	jobQueue[data->queueNo].jq_Cur--;
	ReleaseSemaphore(&jobSema);

	FreeVec(data);

	return rc;
}

void StartRexxJob(struct CyberNode *cn)
{
	struct RexxMsg *msg;
	struct MsgPort *rexxServerPort;
	STRPTR	command;
	UWORD	jobNo;
	BPTR	ifh, ofh;

	if (GetARexxLib() == NULL) {
		ErrorMsg("Couldn't start \"%s\" -- no %s available!\n", cn->cn_Command, RXSNAME);
		Log("Couldn't start \"%s\" -- no %s available!\n", cn->cn_Command, RXSNAME);
		return;
	}

	jobNo = GetJobNum();
	if (!jobNo) {
		Log("Job table full trying to start \"%s\"\n", cn->cn_Command);
		FreeARexxLib();
		return;
	}

	if (cn->cn_ReDirIn) {
		ifh = Open(cn->cn_ReDirIn, MODE_OLDFILE);
		if (!ifh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "input", cn->cn_Command);
			FreeJobNum(jobNo);
			FreeARexxLib();
			return;
		}
	} else {
		ifh = Open("NIL:", MODE_OLDFILE);
		if (!ifh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "input", cn->cn_Command);
			FreeJobNum(jobNo);
			FreeARexxLib();
			return;
		}
	}

	if (cn->cn_SendToUser && SendMailCmd[0]) {
		ofh = SetupSendMail(cn->cn_Command, cn->cn_SendToUser);
		if (!ofh) {
			Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
			Close(ifh);
			FreeJobNum(jobNo);
			FreeARexxLib();
			return;
		}
	} else {
		if (cn->cn_ReDirOut) {
			ofh = Open(cn->cn_ReDirOut, (cn->cn_Flags & CNF_OUTAPP ? MODE_READWRITE : MODE_NEWFILE));
			if (!ofh) {
				Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
				Close(ifh);
				FreeJobNum(jobNo);
				FreeARexxLib();
				return;
			}
		} else {
			ofh = Open("NIL:", MODE_NEWFILE);
			if (!ofh) {
				Log("Couldn't open %s redirection for \"%s\"\n", "output", cn->cn_Command);
				Close(ifh);
				FreeJobNum(jobNo);
				FreeARexxLib();
				return;
			}
		}

		if (cn->cn_Flags & CNF_OUTAPP)
			Seek(ofh, 0, OFFSET_END);
	}

	if (cn->cn_Command[0] == '`')
		command = &cn->cn_Command[1];
	else
		command = cn->cn_Command;

	if (!(msg = CreateRexxMsg(RexxPort, "rexx", RXSDIR))) {
		Log("Couldn't create %s to launch \"%s\"\n", "RexxMsg", cn->cn_Command);
		Close(ofh);
		Close(ifh);
		FreeJobNum(jobNo);
		FreeARexxLib();
		return;
	}

	if (!(msg->rm_Args[0] = CreateArgstring(command, strlen(command)))) {
		Log("Couldn't create %s to launch \"%s\"\n", "Argstring", cn->cn_Command);
		DeleteRexxMsg(msg);
		Close(ofh);
		Close(ifh);
		FreeJobNum(jobNo);
		FreeARexxLib();
		return;
	}

	msg->rm_Action = RXCOMM;
	if (command != cn->cn_Command)
		msg->rm_Action |= RXFF_STRING;

	msg->rm_Args[1] = (STRPTR)jobNo;
	msg->rm_Args[2] = (STRPTR)(cn->cn_Flags & CNF_NOLOG);
	msg->rm_Args[3] = (STRPTR)cn->cn_ObeyQueue;

	msg->rm_Stdin = ifh;
	msg->rm_Stdout = ofh;

	Forbid();
	if (rexxServerPort = FindPort(RXSDIR))
		PutMsg(rexxServerPort, (struct Message *)msg);
	Permit();

	if (rexxServerPort) {
		if (!msg->rm_Args[2])
			Log("Job %ld started:  \"%s\"\n", jobNo, cn->cn_Command);
		ObtainSemaphore(&jobSema);
		NumARexxJobs++;
		jobQueue[cn->cn_ObeyQueue].jq_Cur++;
		ReleaseSemaphore(&jobSema);
	} else {
		Log("Couldn't connect to %s host to launch \"%s\"\n", RXSDIR, cn->cn_Command);
		DeleteArgstring(msg->rm_Args[0]);
		DeleteRexxMsg(msg);
		Close(ofh);
		Close(ifh);
		FreeJobNum(jobNo);
	}

	FreeARexxLib();
}

/*
 * this routine will attempt to get a job number for us. it returns the job
 * number or 0 if no jobs are free.
 */
UWORD GetJobNum(void)
{
	register UWORD job;
	register int index;
	register UBYTE mask;

	ObtainSemaphore(&jobSema);

	for (job = 0; job < sizeof(Jobs) * sizeof(UBYTE); job++) {
		index = job / sizeof(UBYTE);
		mask = 1L << (job - index * sizeof(UBYTE));

		if (Jobs[index] & mask)
			continue;

		Jobs[index] |= mask;
		ReleaseSemaphore(&jobSema);
		return (UWORD) (job + 1);
	}

	ReleaseSemaphore(&jobSema);
	return (UWORD) 0;
}

/* this routine will free a job number previously allocated */
void FreeJobNum(UWORD job)
{
	register int index;
	register UBYTE mask;

	if (!job || job >= sizeof(Jobs) * sizeof(UBYTE))
		return;

	job--;

	index = job / sizeof(UBYTE);
	mask = 1L << (job - index * sizeof(UBYTE));

	ObtainSemaphore(&jobSema);
	Jobs[index] &= ~mask;
	ReleaseSemaphore(&jobSema);
}

void __stdargs Log(STRPTR fmt, ...)
{
	va_list args;
	BPTR loghandle;
	struct DateTime dat;
	UBYTE Date[LEN_DATSTRING + 1];
	UBYTE Time[LEN_DATSTRING + 1];

	DateStamp(&dat.dat_Stamp);
	dat.dat_Format = FORMAT_DOS;
	dat.dat_Flags = 0;
	dat.dat_StrDay = NULL;
	dat.dat_StrDate = Date;
	dat.dat_StrTime = Time;

	memset(Date, 0, LEN_DATSTRING + 1);
	memset(Time, 0, LEN_DATSTRING + 1);
	DateToStr(&dat);

	va_start(args, fmt);
	if (LogFile[0]) {
		ObtainSemaphore(&logSema);

		if (loghandle = Open(LogFile, MODE_READWRITE)) {
			Seek(loghandle, 0, OFFSET_END);

			FPrintf(loghandle, "(%s %s) ", Date, Time);
			VFPrintf(loghandle, fmt, (LONG *)args);
			Close(loghandle);
		}

		ReleaseSemaphore(&logSema);
	}
	va_end(args);
}

/*
 * this function will open the ARexx library for us.  it handles nested
 * calls to open the library such that we only call OpenLibrary() one.  each
 * time a rexx command is run, we call this routine to open the library and
 * when the RexxMsg comes back we call FreeARexxLib() to decrement the nest
 * count.
 */
BOOL GetARexxLib(void)
{
	if (ARexxLibCount) {
		ARexxLibCount++;
		return TRUE;
	}

	if (!(RexxSysBase = (struct RxsLib *) OpenLibrary(RXSNAME, 0)))
		return FALSE;

	ARexxLibCount = 1;
	return TRUE;
}

/*
 * this routine is the opposite of GetARexxLib().  it frees a nested open
 * count for the ARexx library.  if the count goes to zero then we call
 * CloseLibrary() to free the library for real.
 */
void FreeARexxLib(void)
{
	if (!--ARexxLibCount)
		CloseLibrary((struct Library *)RexxSysBase);
}

void __stdargs ErrorMsg(char *fmt, ...)
/* we use this function to send error messages to StdErr.  We prefix all
   messages with "CyberCron:" since the message might be dumped into a stream
   with other people sending output to it */
{
	va_list args;

	if (StdErr) {
		FWrite(StdErr, "CyberCron:  ", 1, 12);

		va_start(args, fmt);
		VFPrintf(StdErr, (STRPTR)fmt, (LONG *)args);
		Flush(StdErr);
		va_end(args);
	}
}

static void __regargs MySPrintfSupp(void);

void __stdargs MySPrintf(STRPTR buf, STRPTR fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	RawDoFmt(fmt, (APTR)args, MySPrintfSupp, (APTR)buf)
	va_end(args);
}

/* this next bit is rather SAS specific and was snitched from Loren Rittle */
static void __regargs MySPrintfSupp(void)
{
	__emit(0x16C0);	/* MOVE.B D0,(A3)+ */
}

static UBYTE DayTable[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

#define BASEYEAR (1970)
#define BASEDAY  (3)
#define LEAP (ScrapA % 4 == 0 && ScrapA % 100 != 0 || ScrapA % 400 == 0)

void GetSystemTime(SystemTime_t *st)
{
	register ULONG c;
	register ULONG ScrapA;
	register ULONG ScrapB;
	register ULONG secs;
	struct timeval tr_time;

	GetSysTime(&tr_time);

	st->st_tvsecs = secs = tr_time.tv_secs;

	c = (secs / SECSINDAY) - 2251;

	/* start of code grabbed from Thomas Rokicki */
	ScrapA = (4 * c + 3) / 1461;
	c -= 1461 * ScrapA / 4;
	ScrapA += 1984;
	ScrapB = (5 * c + 2) / 153;
	st->st_Day = c - (153 * ScrapB + 2) / 5 + 1;
	ScrapB += 3;
	if (ScrapB > 12) {
		ScrapA += 1;
		ScrapB -= 12;
	}
	/* end of code grabbed from Rokicki */

	st->st_Year = ScrapA;
	st->st_Month = ScrapB;

	c = secs % SECSINDAY;
	ScrapA = c / 3600;
	c -= ScrapA * 3600;
	ScrapB = c / 60;
	c -= ScrapB * 60;

	st->st_Hour = ScrapA;
	st->st_Min = ScrapB;
	st->st_Sec = c;

	/* now figure out the day of the week */

	c = BASEDAY;

	for (ScrapA = BASEYEAR; ScrapA < st->st_Year; ScrapA++)
		c += LEAP ? 366 : 365;

	for (ScrapB = 1; ScrapB < st->st_Month; ScrapB++)
		c += DayTable[LEAP][ScrapB - 1];

	st->st_DOW = (c + st->st_Day) % 7;
}
