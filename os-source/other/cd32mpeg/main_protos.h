/* Prototypes for functions defined in
main.c
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

void FlushVideoStream(struct MPEGUnit * );

void FlushAudioStream(struct MPEGUnit * );

void CMDFlush(struct IOMPEGReq * , struct MPEGUnit * );

void CMDReset(struct IOMPEGReq * , struct MPEGUnit * );

void CMDInvalid(struct IOMPEGReq * , struct MPEGUnit * );

void CMDWrite(struct IOMPEGReq * , struct MPEGUnit * );

void SetBlankMode(struct MPEGUnit * );

void DoPlay(struct MPEGUnit * );

void DoPause(struct MPEGUnit * );

void DoSlow(struct MPEGUnit * );

void FlushCDReads(struct MPEGUnit * );

ULONG StartDataPump(struct MPEGUnit * );

void StopDataPump(struct MPEGUnit * );

void CMDPlayLSN(struct IOMPEGReq * , struct MPEGUnit * );

void CMDPlay(struct IOMPEGReq * , struct MPEGUnit * );

void CMDStop(struct IOMPEGReq * , struct MPEGUnit * );

void DoScan(struct MPEGUnit * );

void DoStep(struct MPEGUnit * );

void CMDSlow(struct IOMPEGReq * , struct MPEGUnit * );

void CMDPause(struct IOMPEGReq * , struct MPEGUnit * );

void CMDStep(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSearch(struct IOMPEGReq * , struct MPEGUnit * );

void DoAccounting(struct MPEGUnit * );

void ScanComplete(struct MPEGUnit * );

void CMDGetDevInfo(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSetWindow(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSetBorder(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSetVideo(struct IOMPEGReq * , struct MPEGUnit * );

void CMDGetVideo(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSetAudio(struct IOMPEGReq * , struct MPEGUnit * );

void CMDReadFrame(struct IOMPEGReq * , struct MPEGUnit * );

extern void (* mpegCmdDispatch[24])(struct IOMPEGReq * , struct MPEGUnit * );

BOOL __asm GetNextVideoIO(register __a4 struct MPEGUnit * );

BOOL __asm GetNextAudioIO(register __a4 struct MPEGUnit * );

ULONG ProcessCL450Interrupts(ULONG , struct MPEGUnit * );

void DevTaskCEntry(void);

void NewList(struct List * );

