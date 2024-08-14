struct SPAWNINFO {
	struct Message msg;		/* ALIGNED launcher startup message */
	struct DosPacket dp;		/* ALIGNED packet header for msg */

	/* ALIGNED fake segment for CreateProc of launcher */
	BPTR nextseg;
	WORD jmp;
	VOID (*launcher)();

	struct MsgPort port;		/* port for file handler */
	struct FileHandle fh1;		/* ALIGNED fake file handle */
#if 0
	struct FileHandle fh2;		/* ALIGNED yaffh */
#endif
	LONG opencount;			/* # opens on fh */
	struct Process *proc;		/* spawned process */
	struct CommandLineInterface *CLI;	/* spawned CLI info */
	struct DosPacket *queuedpkt;	/* packet pushback */
	BOOL badoutput;			/* CLI error message output */
	UBYTE breaksig;			/* break signaled */
};

extern char *mempos __ARGS((char *, int, char));
extern VOID FreeBCPL __ARGS((long));
extern VOID ReturnPkt __ARGS((struct DosPacket *, long, long));
extern VOID AbortPkt __ARGS((struct DosPacket *, char *));
extern VOID SignalCLI __ARGS((struct SPAWNINFO *, long));
extern struct DosPacket *GetPkt __ARGS((struct SPAWNINFO *));
extern struct DosPacket *HandlePkt __ARGS((struct SPAWNINFO *,struct DosPacket *, int));
extern struct DosPacket *WaitRead __ARGS((struct SPAWNINFO *));
extern struct SPAWNINFO *SpawnCLI __ARGS((void));
extern VOID CleanupSpawn __ARGS((struct SPAWNINFO *));
extern struct DosPacket *HandleRead __ARGS((struct SPAWNINFO *,
		char *, int, int));
extern int ExecCLI __ARGS((struct SPAWNINFO *, char *));
extern VOID KillCLI __ARGS((struct SPAWNINFO *));
extern int BreakCLI __ARGS((struct SPAWNINFO *));
extern VOID LaunchTask __ARGS((void));
extern int system __ARGS((char *));

