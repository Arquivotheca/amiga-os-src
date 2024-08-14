#ifndef	DP_PROTOS_H
#define DP_PROTOS_H 1

/*
** Prototypes for functions defined in datapump.asm
**
*/

extern ULONG CmpSCR(struct IOMPEGReq *r1, struct IOMPEGReq *r2);
extern ULONG WaitCL450Cmd(struct CL450Regs *cl450);

extern ULONG ASM FeedCL450(REG(d2) ULONG StatFlags,
			   REG(a5) struct MPEGUnit *mpegUnit,
			   REG(a6) struct MPEGDevice *mpegDevice);

extern ULONG ASM FeedL64111(REG(d2) ULONG StatFlags,
			   REG(a5) struct MPEGUnit *mpegUnit,
			   REG(a6) struct MPEGDevice *mpegDevice);


extern ASM ReadLumaBuffer(REG(a0) UBYTE *base,
			  REG(a1) UBYTE *dest,
			  REG(d0) ULONG width,
			  REG(d1) ULONG height);

extern ASM ReadChromaBuffer(REG(a0) UBYTE *base,
			    REG(a1) UBYTE *dest,
			    REG(d0) ULONG width,
			    REG(d1) ULONG height);

#endif	/* DP_PROTOS_H */
