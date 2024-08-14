/* Prototypes for functions defined in
cl450.c
 */

UWORD InitCL450(struct MPEGUnit * );

BOOL NewCommand(UWORD , struct MPEGUnit * );

BOOL ResetCL450(struct MPEGUnit * );

BOOL __asm InquireCL450BufferFullness(register __a4 struct MPEGUnit * );

BOOL NewPacket(UWORD , struct IOMPEGReq * , struct MPEGUnit * );

BOOL __asm SetSCR(register __d2 UWORD , register __d3 UWORD , register __d4 UWORD , register __a4 struct MPEGUnit * );

BOOL GetSCR(UWORD * , UWORD * , UWORD * , struct MPEGUnit * );

BOOL PlayCL450(struct MPEGUnit * );

BOOL SetCL450Border(UWORD , UWORD , UWORD , UWORD , UWORD , struct MPEGUnit * );

BOOL SetCL450InterruptMask(UWORD , struct MPEGUnit * );

BOOL SetCL450Threshold(UWORD , struct MPEGUnit * );

BOOL SetCL450VideoMode(UWORD , struct MPEGUnit * );

BOOL SetCL450Window(UWORD , UWORD , UWORD , UWORD , struct MPEGUnit * );

BOOL ScanCL450(struct MPEGUnit * );

BOOL PauseCL450(struct MPEGUnit * );

BOOL FlushCL450(struct MPEGUnit * );

BOOL SingleStepCL450(struct MPEGUnit * );

BOOL SlowMotionCL450(UWORD , struct MPEGUnit * );

BOOL BlankCL450(ULONG , struct MPEGUnit * );

void ObtainCL450SeqSemaphore(struct MPEGUnit * );

void ReleaseCL450SeqSemaphore(struct MPEGUnit * );

