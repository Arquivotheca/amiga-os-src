#define MAXSIZE 510
#define MAXOSIZE 2040

#define IDS (('S' << 24) | ('P' << 16) | ('K' << 8))

/* Phil's version of BADDR() has no problems with casting */
#undef  BADDR
#define BADDR(x)	((APTR)((long)x << 2))

#define ACTION_FIND_INPUT       1005
#define ACTION_FIND_OUTPUT      1006
/*#define ACTION_END              1007*/

#define DOS_TRUE	-1

extern struct IntuitionBase *IntuitionBase;

extern LONG Translate();
extern struct MsgPort  *CreatePort();
extern struct narrator_rb *CreateExtIO();

extern struct Process   *GetProcess();
extern VOID             returnpkt();
extern struct DosPacket *taskwait();

typedef struct VARS {
	ULONG TalkDevice;
	int OpenCount;
	UBYTE *buffer;
	UBYTE *phoneme;
	int optionMode;
	int delimit;
	int aMode;
	struct MsgPort *clientport;  /* senders taskid */
	struct   MsgPort *devport;
	struct   narrator_rb    *iow;
};
