#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "/mpeg.h"
#include "/cl450.h"
#include "/l64111.h"

ULONG OpenTest(VOID)
{
    struct Library *SysBase = *(struct Library **)4L;
    struct Library *DOSBase;
    struct CL450Regs *cl450Regs = NULL;
    struct L64111Regs *l64111Regs = NULL;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	Printf("VID_YATA    = %2lx\n",&cl450Regs->VID_YDATA);
    	Printf("CMEM_DATA   = %2lx\n",&cl450Regs->CMEM_DATA);
    	Printf("VID_UVDATA  = %2lx\n",&cl450Regs->VID_UVDATA);
    	Printf("CPU_CONTROL = %2lx\n",&cl450Regs->CPU_CONTROL);
    	Printf("CPU_PC      = %2lx\n",&cl450Regs->CPU_PC);
    	Printf("CPU_INTENB  = %2lx\n",&cl450Regs->CPU_INTENB);
    	Printf("CPU_TADDR   = %2lx\n",&cl450Regs->CPU_TADDR);
    	Printf("CPU_IADDR   = %2lx\n",&cl450Regs->CPU_IADDR);
    	Printf("CPU_IMEM    = %2lx\n",&cl450Regs->CPU_IMEM);
    	Printf("CPU_TMEM    = %2lx\n",&cl450Regs->CPU_TMEM);
    	Printf("CPU_INT     = %2lx\n",&cl450Regs->CPU_INT);
    	Printf("CPU_NEWCMD  = %2lx\n",&cl450Regs->CPU_NEWCMD);
    	Printf("CMEM_CONTROL= %2lx\n",&cl450Regs->CMEM_CONTROL);
    	Printf("CMEM_STATUS = %2lx\n",&cl450Regs->CMEM_STATUS);
    	Printf("CMEM_DMACTRL= %2lx\n",&cl450Regs->CMEM_DMACTRL);
    	Printf("HOST_RADDR1 = %2lx\n",&cl450Regs->HOST_RADDR1);
    	Printf("HOST_RDATA1 = %2lx\n",&cl450Regs->HOST_RDATA1);
    	Printf("HOST_CONTROL= %2lx\n",&cl450Regs->HOST_CONTROL);
    	Printf("HOST_SCR0   = %2lx\n",&cl450Regs->HOST_SCR0);
    	Printf("HOST_SCR1   = %2lx\n",&cl450Regs->HOST_SCR1);
    	Printf("HOST_SCR2   = %2lx\n",&cl450Regs->HOST_SCR2);
    	Printf("HOST_INTVECW= %2lx\n",&cl450Regs->HOST_INTVECW);
    	Printf("HOST_INTVECR= %2lx\n",&cl450Regs->HOST_INTVECR);
    	Printf("DRAM_REFCNT = %2lx\n",&cl450Regs->DRAM_REFCNT);
    	Printf("VID_CONTROL = %2lx\n",&cl450Regs->VID_CONTROL);
    	Printf("VID_REGDATA = %2lx\n",&cl450Regs->VID_REGDATA);

	Printf("Data		= %2lx\n",&l64111Regs->Data);
	Printf("Control_1	= %2lx\n",&l64111Regs->Control_1);
	Printf("Control_2	= %2lx\n",&l64111Regs->Control_2);
	Printf("Control_3	= %2lx\n",&l64111Regs->Control_3);
	Printf("Status_Int_1	= %2lx\n",&l64111Regs->Status_Int_1);
	Printf("Status_Int_2	= %2lx\n",&l64111Regs->Status_Int_2);
	Printf("Timer_CountDown	= %2lx\n",&l64111Regs->Timer_CountDown);
	Printf("Timer_OffsetHi	= %2lx\n",&l64111Regs->Timer_OffsetHi);
	Printf("Timer_OffsetLo	= %2lx\n",&l64111Regs->Timer_OffsetLo);
	Printf("Parametric_I	= %2lx\n",&l64111Regs->Parametric_I);
	Printf("Parametric_II	= %2lx\n",&l64111Regs->Parametric_II);
	Printf("Parametric_III	= %2lx\n",&l64111Regs->Parametric_III);
	Printf("Presentation_I	= %2lx\n",&l64111Regs->Presentation_I);
	Printf("Presentation_II	= %2lx\n",&l64111Regs->Presentation_II);
	Printf("Presentation_III= %2lx\n",&l64111Regs->Presentation_III);
	Printf("Presentation_IV	= %2lx\n",&l64111Regs->Presentation_IV);
	Printf("Presentation_V	= %2lx\n",&l64111Regs->Presentation_V);
	Printf("DataFIFO	= %2lx\n",&l64111Regs->DataFIFO);
	Printf("Channel_Stat	= %2lx\n",&l64111Regs->Channel_Stat);
	Printf("Channel_ReadCtr	= %2lx\n",&l64111Regs->Channel_ReadCtr);
	Printf("Channel_WriteCtr= %2lx\n",&l64111Regs->Channel_WriteCtr);
    	CloseLibrary(DOSBase);
    }
    return(0L);
}