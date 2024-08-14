/*
** $Header: HOG:Other/cd32mpeg/RCS/cl450.c,v 40.6 94/03/15 14:17:23 kcd Exp $
**
** Amiga MPEG Device Driver
**
** CL450 Specific Code
**
*/

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "mpeg_device.h"
#include "cl450.h"

#ifndef _GENPROTO
#include "cl450_protos.h"
#include "datapump_protos.h"
#endif

/* Binary image of the CL450 Microcode */

extern UBYTE CL450MicroCode[];

#undef SysBase
#define SysBase	mpegUnit->mu_SysBase

#if 0
#define D(x) x
#else
#define D(x)
#endif

/*
** loadSeg - Transfer a code segment into the CL450's
** DRAM and/or IMEM.
**
*/

#define AUTO_INC_KLUDGE		// Kludge to fix possible HW problem with card and/or CL450

extern ULONG ASM EndianFixLong(REG(d0) ULONG val);
extern UWORD ASM EndianFixWord(REG(d0) ULONG val);
extern VOID kprintf(STRPTR fmt,	...);

static UWORD loadSeg(UBYTE **data, ULONG imem_start, struct MPEGUnit *mpegUnit)
{
    ULONG segsize, addr;
    UWORD loadVal;
    BOOL imemcode;


    segsize = EndianFixLong(**(ULONG **)data);
    *data = (UBYTE *)((ULONG)*data + 4);

    addr = EndianFixLong(**(ULONG **)data);
    *data = (UBYTE *)((ULONG)*data + 4);

    D(kprintf("addr = %lx, imem_start = %lx, segsize = %lx\n",addr, imem_start, segsize));

    if(segsize & 3)
    	return(ERR_BAD_SEG_ALIGN);

    segsize += addr;

//    if((addr > DRAMSIZE) || (segsize > DRAMSIZE))
//    	return(ERR_BAD_TARGET_ADDR);


    if((addr >= imem_start) && (segsize < (imem_start + (UCODE_CACHE_END<<2))))
    {
    	imemcode = TRUE;
    	D(kprintf("CPU_IADDR: %lx\n",((addr - imem_start) >> 1) & WADD_MASK));
    	WriteCL450Register(CPU_IADDR, ((addr - imem_start) >> 1) & WADD_MASK);
    	D(kprintf("CPU_IADDR: Done\n"));
    }
    else
    	imemcode = FALSE;

    for(; addr < segsize; addr += 2)
    {
    	loadVal = **(UWORD **)data;
    	*data = (UBYTE *)((ULONG)*data + 2);
    	WriteCL450Dram(addr, loadVal);

    	if(imemcode)
    	{
    	    D(kprintf("CPU_IMEM: %lx\n",loadVal));
    	    WriteCL450Register(CPU_IMEM, loadVal);
    	    D(kprintf("CPU_IMEM: Done\n"));
    	}
    }
    D(kprintf("loadSeg Done.\n"));
    return(NO_ERR);
}


/*
** DownloadCL450Microcode - Download microcode image into
** the CL450's DRAM and/or IMEM.
**
*/

static UWORD DownLoadCL450Microcode(UWORD *StartAddr, struct MPEGUnit *mpegUnit)
{
    UWORD Segment, Error;
    ULONG StartIMEM;
    UBYTE *data;
    struct binary_hd *Header;

    data = (UBYTE *)((ULONG)CL450MicroCode + sizeof(struct binary_hd));
    Header = (struct binary_hd *)CL450MicroCode;

    Error = NO_ERR;

    *StartAddr = EndianFixWord(Header->init_pc);

    D(kprintf("StartAddr = $%lx\n",*StartAddr));

    StartIMEM = EndianFixWord(Header->imem_dram);

    D(kprintf("StartIMEM = $%lx\n",StartIMEM));

    Segment = EndianFixWord(Header->nseg);

    D(kprintf("Segments: %ld\n",Segment));

    for(; Segment && !Error; Segment--)
    {
    	Error = loadSeg(&data, StartIMEM,mpegUnit);
        D(kprintf("loadSeg returned %ld\n",Error));
    }
    return(Error);
}

/*
** StartCL450 - Load Microcode and start the CL450
**
*/
static UWORD StartCL450(struct MPEGUnit *mpegUnit)
{
    UWORD StartAddr, Result;

    if(!(Result = DownLoadCL450Microcode(&StartAddr,mpegUnit)))
    {
        D(kprintf("CPU_PC: %04lx\n", StartAddr & PC_MASK));
    	WriteCL450Register(CPU_PC, StartAddr & PC_MASK);
    	D(kprintf("CPU_PC: Done\n"));
    }
    return(Result);
}

/*
** InitCL450
**
*/
UWORD InitCL450(struct MPEGUnit *mpegUnit)
{
    UWORD Result;
    ULONG i;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;
    struct GfxBase *GfxBase;

    D(kprintf("InitCL450: Entry.  CL450 Registers Located @ $%lx\n",cl450Regs));

    WrCL450Register(CMEM_CONTROL, RST | CRE | CRST);
    WrCL450Register(CMEM_CONTROL, 0);
    WrCL450Register(CMEM_DMACTRL, 0);
    WrCL450Register(HOST_SCR2, CS | (DIV_MASK & (SYSTEM_CLOCK_DIVISOR(MPEG_CL450_GCLK_FREQ) << 3)));
    WrCL450Register(DRAM_REFCNT, REFCNT_MASK & REFRESH_CLOCK_COUNT(MPEG_CL450_DRAM_REFRESH_NS,
								       MPEG_CL450_GCLK_PERIOD_NS,
								       MPEG_CL450_DRAM_ROWS));
    WrCL450Register(VID_CONTROL, VID_SELMODE);
    WrCL450Register(VID_REGDATA, 1);
    WrCL450Register(HOST_RADDR1, 0x0a);
    WrCL450Register(HOST_RDATA1, 0x01);

    WrCL450Register(CPU_IADDR, 0);

    for(i = 0; i < 512; i++)
    {
        WrCL450Register(CPU_IMEM,0);
        WrCL450Register(CPU_IMEM,0);
    }

    WrCL450Register(CPU_TADDR, 0);
    for(i = 0; i < 128; i++)
    {
        WrCL450Register(CPU_TMEM,0);
    }

#if 1
    for(i = 0; i < 0x7ffff; i+=2)
    {
    	WriteCL450Dram(i, 0);
    }
#endif

    D(kprintf("InitCL450: Starting CL450\n"));

    if(!(Result = StartCL450(mpegUnit)))
    {
    	D(kprintf("InitCL450: Microcode loaded okay, starting CPU\n"));

	WrCL450Register(HOST_RADDR1, 0xf);
	WrCL450Register(HOST_RDATA1, 0xffff);

    	WrCL450Register(CPU_CONTROL, CE_N);

    	D(kprintf("Waiting for microcode initilization.\n"));

	while(1)
	{
	    WrCL450Register(HOST_RADDR1, 0xf);
	    if(RdCL450Register(HOST_RDATA1) == 0)
	        break;
	}

	D(kprintf("Microcode initialization complete.\n"));

	WrCL450Register(HOST_CONTROL, NON_VECTORED);
	WrCL450Register(CMEM_DMACTRL, _3QE);

    	SetCL450Threshold(8192,mpegUnit);
	SetCL450Border(0,0,17,17,17,mpegUnit);		// AH: enabled 4/15/94

	if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",37L))
	{
	    if(GfxBase->DisplayFlags & PAL)
	    	SetCL450VideoMode(3,mpegUnit);

	    CloseLibrary((struct Library *)GfxBase);
	}
    }
    return(Result);
}


/*
** NewCommand - Sends a new command to the CL450.
**
*/
BOOL NewCommand(UWORD commandWord, struct MPEGUnit *mpegUnit)
{
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    D(kprintf("NewCommand: Waiting for CMD semaphore\n"));

    WaitCL450Cmd(cl450Regs);

    D(kprintf("NewCommand: Got Semaphore, sending command\n"));

    if((RdCL450Register(CPU_NEWCMD) & CMD) == 0)
    {
    	WrCL450Register(HOST_RADDR1, 0);
    	WrCL450Register(HOST_RDATA1, commandWord);
    	return(TRUE);
    }
    return(FALSE);
}

/*
** ResetCL450 - Reset CL450 Microcode.
**
*/
BOOL ResetCL450(struct MPEGUnit *mpegUnit)
{
    /* Fist, we try to do this the _nice_ way by sending
       the abort command.  If this fails, we call InitCL450
       to reset the chip, as something bad has probably
       happended. We will return FALSE in this case so
       our called knows what we did. */

    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_RESET,mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD,CMD);
    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    {
    	InitCL450(mpegUnit);
    	stat=FALSE;
    }

    EndCL450Cmd();
    return(stat);
}

/*
** InquireCL450BufferFullness - Tell the CL450 to place the
** current buffer fullness value into an internal register
** that we can read.
*/
BOOL ASM InquireCL450BufferFullness(REG(a4) struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_INQUIRE_BUFFER_FULLNESS,mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD, CMD);
	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}


/*
** NewPacket - Tell the CL450 we are about to send a new
** packet of data to it, possibly with a timestamp.
**
*/
BOOL NewPacket(UWORD length, struct IOMPEGReq *iomr, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_NEW_PACKET,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
    	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, length);

#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
	WrCL450Register(HOST_RDATA1, (iomr->iomr_PTSHigh & 0x03) | (((iomr->iomr_MPEGFlags & MPEGF_VALID_PTS) && (mpegUnit->mu_PlayFlags & MPEGPF_SYNC)) ? 0x8000 : 0));
//	WrCL450Register(HOST_RDATA1, (iomr->iomr_PTSHigh & 0x03));

#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
    	WrCL450Register(HOST_RDATA1, 0x7fff & iomr->iomr_PTSMid);

#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 4);
#endif
    	WrCL450Register(HOST_RDATA1, 0x7fff & iomr->iomr_PTSLow);

    	WrCL450Register(CPU_NEWCMD, CMD);

        WaitCL450Cmd(cl450Regs);
#if 0
        WaitCL450Cmd(cl450Regs);

    	WrCL450Register(HOST_RADDR1, 0);

    	if(RdCL450Register(HOST_RDATA1) != CL450CMD_NEW_PACKET)
    	    kprintf("Length Mismatch!\n");

    	WrCL450Register(HOST_RADDR1, 1);

    	if(RdCL450Register(HOST_RDATA1) != length)
    	    kprintf("Length Mismatch!\n");

    	WrCL450Register(HOST_RADDR1, 2);

    	if(RdCL450Register(HOST_RDATA1) != ((iomr->iomr_PTSHigh & 0x03) | ((iomr->iomr_MPEGFlags & MPEGF_VALID_PTS) ? 0x8000 : 0)))
    	    kprintf("PTSHigh mismatch!\n");

    	WrCL450Register(HOST_RADDR1, 3);

    	if(RdCL450Register(HOST_RDATA1) != (0x7fff & iomr->iomr_PTSMid))
    	    kprintf("PTSMid mismatch!\n");

    	WrCL450Register(HOST_RADDR1, 4);

    	if(RdCL450Register(HOST_RDATA1) != (0x7fff & iomr->iomr_PTSLow))
    	    kprintf("PTSLow mismatch!\n");
#endif
    	stat=TRUE;
    }
    else
        stat=FALSE;


#if 0
    if(iomr->iomr_MPEGFlags & MPEGF_VALID_PTS)
    {
        ULONG pts, minutes, seconds, frames;
        pts = ((iomr->iomr_PTSHigh & 0x3) << 30) |
               ((iomr->iomr_PTSMid & 0x7fff) << 15) |
               (iomr->iomr_PTSLow & 0x7fff);

        minutes = pts / (90000*60);
        seconds = (pts - (minutes * 90000*60)) / 90000;
        frames = pts - (minutes * 90000*60) - (seconds * 90000);

	kprintf("%03ld:%02ld:%06ld\n",minutes,seconds,frames);

        mpegUnit->mu_LastPTSHi = iomr->iomr_PTSHigh & 0x3;
        mpegUnit->mu_LastPTSMid = iomr->iomr_PTSMid & 0x7fff;
        mpegUnit->mu_LastPTSLo = iomr->iomr_PTSLow & 0x7fff;
    }
    else
    	kprintf("--\n");
#endif
    EndCL450Cmd();
    return(stat);
}

/*
** SetSCR - Set the CL450's System Clock Reference value
**
*/
BOOL ASM SetSCR(REG(d2) UWORD hi,
	        REG(d3) UWORD mid,
	        REG(d4) UWORD lo,
	        REG(a4) struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();
    if(NewCommand(CL450CMD_ACCESS_SCR, mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, (hi & 0x03));
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
    	WrCL450Register(HOST_RDATA1, (mid & 0x7fff));
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
    	WrCL450Register(HOST_RDATA1, (lo & 0x7fff));
    	WrCL450Register(CPU_NEWCMD,CMD);
    	mpegUnit->mu_PlayFlags |= MPEGPF_SYNC;
//    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();

    return(stat);
}

/*
** GetSCR - Set the CL450's System Clock Reference value
**
*/
BOOL GetSCR(UWORD *hi, UWORD *mid, UWORD *lo, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_ACCESS_SCR, mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, 0x8000);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
    	WrCL450Register(HOST_RDATA1, 0);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
    	WrCL450Register(HOST_RDATA1, 0);
    	WrCL450Register(CPU_NEWCMD,CMD);

        WaitCL450Cmd(cl450Regs);

    	WrCL450Register(HOST_RADDR1,1);

    	*hi = RdCL450Register(HOST_RDATA1) & 0x3;
    	WrCL450Register(HOST_RADDR1,2);

    	*mid = RdCL450Register(HOST_RDATA1) & 0x7fff;

    	WrCL450Register(HOST_RADDR1,3);
    	*lo = RdCL450Register(HOST_RDATA1) & 0x7fff;

    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** PlayCL450 - Tells the CL450 to begin decoding and
** displaying video.
**
*/
BOOL PlayCL450(struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_PLAY, mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD, CMD);
//    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();

    do
    {
	WrCL450Register(HOST_CONTROL, NON_VECTORED);
    }
    while ((RdCL450Register(HOST_CONTROL) & 0x81) != NON_VECTORED);

    return(stat);
}

/*
** SetCL450Border - Sets the border color and display
** window location.
**
*/
BOOL SetCL450Border(UWORD windowLeft, UWORD windowTop, UWORD borderRed,
		    UWORD borderGreen, UWORD borderBlue, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_BORDER,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, windowLeft);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
    	WrCL450Register(HOST_RDATA1, windowTop);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
    	WrCL450Register(HOST_RDATA1, borderRed);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 4);
#endif
    	WrCL450Register(HOST_RDATA1, (borderGreen) << 8 | borderBlue);
    	WrCL450Register(CPU_NEWCMD, CMD);
//    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** SetCL450InterruptMask - Sets int enable bits on the CL450
**
*/
BOOL SetCL450InterruptMask(UWORD eventMask, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_INTERRUPT_MASK,mpegUnit))
    {
    	Disable();
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, eventMask);
    	WrCL450Register(CPU_NEWCMD, CMD);
    	Enable();
//	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    if(eventMask)
    {
        WrCL450Register(HOST_RADDR1, READ_INT_STATUS);
        WrCL450Register(HOST_RDATA1, 0);
    }

    EndCL450Cmd();
    return(stat);
}

/*
** SetThreshold - Sets the amount of free buffer space that will
** cause the CL450 to send us a ready for data interrupt.
**
*/
BOOL SetCL450Threshold(UWORD bufferLevel, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    ULONG i;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    D(kprintf("SetCL450Threshold: Entry\n"));

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_THRESHOLD,mpegUnit))
    {
    	D(kprintf("SetCL450Threshold: NewCommand() okay, writing parameters\n"));
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, bufferLevel);
    	WrCL450Register(CPU_NEWCMD, CMD);

    	D(kprintf("SetCL450Threshold: Waiting for command to complete.\n"));

	for(i=500000;i;i--);

//    	WaitCL450Cmd(cl450Regs);

    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();

    D(kprintf("SetCL450Threshold: Done\n"));
    return(stat);
}

/*
** SetCL450VideoMode - Set NTSC or PAL
**
*/
BOOL SetCL450VideoMode(UWORD videoMode, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_VIDEO_FORMAT,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, videoMode);
    	WrCL450Register(CPU_NEWCMD, CMD);
//    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** SetCL450Window - Set the decompression window
**
*/
BOOL SetCL450Window(UWORD windowLeft, UWORD windowTop, UWORD windowWidth,
		    UWORD windowHeight, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_WINDOW,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
    	WrCL450Register(HOST_RDATA1, windowLeft);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
    	WrCL450Register(HOST_RDATA1, windowTop);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
    	WrCL450Register(HOST_RDATA1, windowWidth);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 4);
#endif
    	WrCL450Register(HOST_RDATA1, windowHeight);
    	WrCL450Register(CPU_NEWCMD, CMD);
//	WaitCL450Cmd(cl450Regs);

    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** ScanCL450 - Put CL450 into scan mode.
**
*/
BOOL ScanCL450(struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SCAN,mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD, CMD);
//	WaitCL450Cmd(cl450Regs);
        stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** PauseCL450 - Put CL450 into pause mode.
**
*/
BOOL PauseCL450(struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_PAUSE,mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD, CMD);
        stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** FlushCL450 - Flush the CL450's bitstream buffer.
**
*/
BOOL FlushCL450(struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_FLUSH_BITSTREAM,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
        WrCL450Register(HOST_RDATA1, 0);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 2);
#endif
        WrCL450Register(HOST_RDATA1, 0);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 3);
#endif
        WrCL450Register(HOST_RDATA1, 0);
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 4);
#endif
        WrCL450Register(HOST_RDATA1, 0);

        WrCL450Register(CPU_NEWCMD, CMD);

        WaitCL450Cmd(cl450Regs);

        stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** SingleStepCL450 - Step ahead one video frame
**
*/
BOOL SingleStepCL450(struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SINGLE_STEP,mpegUnit))
    {
    	WrCL450Register(CPU_NEWCMD, CMD);
//    	WaitCL450Cmd(cl450Regs);
    	stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** SlowMotionCL450 - Enter slow motion mode
**
*/
BOOL SlowMotionCL450(UWORD speed, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SLOW_MOTION,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
        WrCL450Register(HOST_RDATA1, speed);
        WrCL450Register(CPU_NEWCMD, CMD);
//        WaitCL450Cmd(cl450Regs);
        stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** BlankCL450 - Set CL450 blanking status
**
*/
BOOL BlankCL450(ULONG blank, struct MPEGUnit *mpegUnit)
{
    BOOL stat;
    CL450Regs *cl450Regs = mpegUnit->mu_CL450;

    StartCL450Cmd();

    if(NewCommand(CL450CMD_SET_BLANK,mpegUnit))
    {
#ifdef AUTO_INC_KLUDGE
	WrCL450Register(HOST_RADDR1, 1);
#endif
        WrCL450Register(HOST_RDATA1, (blank) ? 1 : 0);
        WrCL450Register(CPU_NEWCMD, CMD);
        stat=TRUE;
    }
    else
    	stat=FALSE;

    EndCL450Cmd();
    return(stat);
}

/*
** ObtainCL450SeqSemaphore - Grab DRAM Variable Semaphore
**
*/
VOID ObtainCL450SeqSemaphore(struct MPEGUnit *mpegUnit)
{
    volatile UBYTE delay;

    Disable();
    do
    {
    	while(ReadCL450Dram(SEQ_SEM) != 0);

    	WriteCL450Dram(SEQ_SEM,HOST_CODE);

    	/* Slimy way to delay a bit */

    	delay = *(volatile UBYTE *) 0xbfe001;

    } while(ReadCL450Dram(SEQ_SEM) != HOST_CODE);
}

/*
** ReleaseCL450SeqSemaphore
**
*/
VOID ReleaseCL450SeqSemaphore(struct MPEGUnit *mpegUnit)
{
    WriteCL450Dram(SEQ_SEM,0);
    Enable();
}

