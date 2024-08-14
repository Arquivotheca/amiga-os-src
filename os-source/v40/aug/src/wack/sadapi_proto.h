/*
 * Prototypes for all of the important SADAPI functions.
 * $id$
 *
 */

void SADSetVersion( struct SADAPIVars *g );
extern struct SADAPIVars *SADInit(STRPTR devicename, ULONG bpsrate, ULONG unitnumber);
extern void SADDeinit(struct SADAPIVars *BasePtr);
extern void NewProcTask(void);
BOOL SADCmd(struct SADAPIVars *g, void  *CommandString, ULONG CommandLength,
            BOOL WaitForAck, BOOL WaitForDone, UWORD TimeoutForDone,
            void *ResultPtr, ULONG ResultLength,
	    void *OutputPtr, ULONG OutputLength);
BOOL SADReadLong(struct SADAPIVars *g, void *pointer, ULONG *result);
BOOL SADReadWord(struct SADAPIVars *g, void *pointer, UWORD *result);
BOOL SADReadByte(struct SADAPIVars *g, void *pointer, UBYTE *result);
BOOL SADReadArray(struct SADAPIVars *g, void *pointer, UBYTE *resultptr, ULONG length);

ULONG SADSendDelete(struct SADAPIVars *g, ULONG TimeToWait);
ULONG SADWaitForPrompt(struct SADAPIVars *g, ULONG TimeToWait);

BOOL SADWriteByte(struct SADAPIVars *g, void *pointer, UBYTE byte);
BOOL SADWriteArray(struct SADAPIVars *g, void *pointer, UBYTE *dataptr, ULONG datalength);
BOOL SADWriteWord(struct SADAPIVars *g, void *pointer, UWORD data);
BOOL SADWriteLong(struct SADAPIVars *g, void *pointer, ULONG data);
BOOL SADGetContextFrame(struct SADAPIVars *g, ULONG *resultptr);
BOOL SADReturnToSystem(struct SADAPIVars *g);
BOOL SADCallFunction(struct SADAPIVars *g, APTR addr);
BOOL SADNOP(struct SADAPIVars *g);
BOOL SADAllocateMemory(struct SADAPIVars *g, ULONG length, ULONG type, ULONG *result);
BOOL SADReleaseMemory(struct SADAPIVars *g, void *ptr);
BOOL SADCallAddress(struct SADAPIVars *g, void *address, ULONG timeout);
BOOL SADReset(struct SADAPIVars *g);
BOOL SADTurnOffSingleStepping(struct SADAPIVars *g, ULONG vector);
BOOL SADTurnOnSingleStepping(struct SADAPIVars *g, ULONG *ptr);
