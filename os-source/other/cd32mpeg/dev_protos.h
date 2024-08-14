/* Prototypes for functions defined in
dev.c
 */

void __asm DevInit(register __a5 struct MPEGDevice * , register __a6 struct ExecBase * );

ULONG __asm DevOpen(register __a1 struct IOMPEGReq * , register __d0 ULONG , register __d1 ULONG , register __a6 struct MPEGDevice * );

BPTR __asm DevClose(register __a1 struct IOMPEGReq * , register __a6 struct MPEGDevice * );

BPTR __asm DevExpunge(register __a6 struct MPEGDevice * );

void __asm DevBeginIO(register __a1 struct IOMPEGReq * , register __a6 struct MPEGDevice * );

extern struct FakeMemList TaskMemTemplate;

struct Task * IntCreateTask(char * name,
                            UBYTE pri,
                            APTR initPC,
                            ULONG stackSize,
                            struct MPEGDevice * mpegDevice);

struct MPEGUnit * InitMPEGUnit(ULONG , struct MPEGDevice * );

ULONG __asm DevAbortIO(register __a1 struct IOMPEGReq * , register __a6 struct MPEGDevice * );

ULONG __asm AbortReq(register __a2 struct MinList * , register __a3 struct IOMPEGReq * , register __a6 struct MPEGDevice * );

void __asm TermIO(register __a1 struct IOMPEGReq * , register __a6 struct MPEGDevice * );

BOOL __asm DevSetSCR(register __a0 struct MPEGUnit * , register __d0 ULONG , register __a6 struct MPEGDevice * );

ULONG __asm DevGetSCR(register __a0 struct MPEGUnit * );

ULONG __asm DevGetSector(register __a0 struct MPEGUnit * );

void ExpungeUnit(struct MPEGUnit * , struct MPEGDevice * );

