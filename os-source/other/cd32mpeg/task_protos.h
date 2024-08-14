/* Prototypes for functions defined in
task.c
 */

void FlushVideoStream(struct MPEGUnit * );

void FlushAudioStream(struct MPEGUnit * );

void CMDFlush(struct IOMPEGReq * , struct MPEGUnit * );

void CMDReset(struct IOMPEGReq * , struct MPEGUnit * );

void CMDInvalid(struct IOMPEGReq * , struct MPEGUnit * );

void CMDWrite(struct IOMPEGReq * , struct MPEGUnit * );

void SetBlankMode(struct MPEGUnit * );

BOOL DoPlay(struct MPEGUnit * );

BOOL DoPause(struct MPEGUnit * );

BOOL DoSlow(struct MPEGUnit * );

void FlushCDReads(struct MPEGUnit * );

ULONG StartDataPump(struct MPEGUnit * );

void StopDataPump(struct MPEGUnit * );

void CMDPlayLSN(struct IOMPEGReq * , struct MPEGUnit * );

void CMDPlay(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSeekLSN(struct IOMPEGReq * , struct MPEGUnit * );

void CMDStop(struct IOMPEGReq * , struct MPEGUnit * );

BOOL DoScan(struct MPEGUnit * );

BOOL DoStep(struct MPEGUnit * );

void CMDSlow(struct IOMPEGReq * , struct MPEGUnit * );

void CMDPause(struct IOMPEGReq * , struct MPEGUnit * );

void CMDStep(struct IOMPEGReq * , struct MPEGUnit * );

void CMDSearch(struct IOMPEGReq * , struct MPEGUnit * );

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

