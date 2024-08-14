#include <exec/types.h>
#include <exec/exec.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <libraries/configvars.h>
#include <libraries/configregs.h>
#include <stdio.h>
#include <dos.h>


struct ncr710 {
	UBYTE sien;	// SCSI interrupt enable
	UBYTE sdid;	// SCSI destination ID
	UBYTE scntl1;	// SCSI control reg 1
	UBYTE scntl0;	// SCSI control reg 0
	UBYTE socl;	// SCSI Output Control Latch
	UBYTE sodl;	// SCSI Output Data Latch
	UBYTE sxfer;	// SCSI Transfer reg
	UBYTE scid;	// SCSI Chip ID
	UBYTE sbcl;	// SCSI Bus Control Lines
	UBYTE sbdl;	// SCSI Bus Data Lines		(read only)
	UBYTE sidl;	// SCSI Input Data Latch	(read only)
	UBYTE sfbr;	// SCSI First Byte Received
	UBYTE sstat2;	// SCSI Status Register 2	(read only)
	UBYTE sstat1;	// SCSI Status Register 1	(read only)
	UBYTE sstat0;	// SCSI Status Register 0	(read only)
	UBYTE dstat;	// DMA Status			(read only)
	ULONG dsa;	// Data Structure Address
	UBYTE ctest3;	// Chip Test Register 3		(read only)
	UBYTE ctest2;	// Chip Test Register 2		(read only)
	UBYTE ctest1;	// Chip Test Register 1		(read only)
	UBYTE ctest0;	// Chip Test Register 0
	UBYTE ctest7;	// Chip Test Register 7
	UBYTE ctest6;	// Chip Test Register 6
	UBYTE ctest5;	// Chip Test Register 5
	UBYTE ctest4;	// Chip Test Register 4
	ULONG temp;	// Temporary Stack
	UBYTE lcrc;	// Longitudinal Parity
	UBYTE ctest8;	// Chip Test Register 8
	UBYTE istat;	// Interrupt Status
	UBYTE dfifo;	// DMA FIFO
//	union {
//		UBYTE dcmd;	// DMA Command
		ULONG dbc;	// DMA Byte Count (actually 3 bytes!)
//	} dma;
	ULONG dnad;	// DMA Next Address
	ULONG dsp;	// DMA SCRIPTs Pointer
	ULONG dsps;	// DMA SCRIPTs Pointer Save
	ULONG scratch;	// General Purpose Scratch Pad
	UBYTE dcntl;	// DMA Control
	UBYTE dwt;	// DMA Watchdog Timer
	UBYTE dien;	// DMA Interrupt Enable
	UBYTE dmode;	// DMA Mode
	ULONG adder;	// Sum output of internal adder	(read only)
};


#include "test/siop.h"

struct	Library *ExpansionBase;
extern	struct Library *OpenLibrary();
extern	struct ConfigDev *FindConfigDev();



void	init_siop();								/* prototypes */
void	init_script();
void	write_long_reg(int reg, ULONG addr);
ULONG	*AllocAligned(ULONG size,ULONG chunk);

#define EMPTY ""
#define	XFER_SIZE 32

UBYTE	dest[XFER_SIZE];
UBYTE	source[XFER_SIZE] = {
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
		25,26,27,28,29,30,31,32
	};

ULONG SCRIPT[] = {
    0xc0000000l | XFER_SIZE,							/* Move */
      (ULONG) source, (ULONG) dest,						/*   src dest */
    0x98080000l, 0x00000000l,							/* Int end-of-script */
    0x00000000l, 0x00000000l,							/* Nop  */
    0x00000000l, 0x00000000l,							/* Nop  */
    0x00000000l, 0x00000000l,							/* Nop  */
    0x00000000l, 0x00000000l,							/* Nop  */
    0x00000000l, 0x00000000l,							/* Nop  */
    0x00000000l									/* Nop  */
};

ULONG	*script;
UBYTE	*dma_src, *dma_dest;

struct	ConfigDev *dev = NULL;
UWORD	Manuf = 0X202;
UBYTE	Product = 152, *base;

UBYTE	istat, dstat, sstat;
int	npass = 0, pass = 0, bad = 0;
int	i, fail = 0;
char	ans;

volatile struct ncr710 *siop;

main()										/* begin main */
{
   char string[80];

   printf("A3090 Test8c\n");

   ExpansionBase = OpenLibrary("expansion.library",33);
   if (ExpansionBase == NULL)  exit(100);

another:
   if (dev = FindConfigDev(dev,Manuf,Product))					/* Look for the board */
	{
	base = (*dev).cd_BoardAddr;
	siop = (void *) (((ULONG) base) + SIOP_ADDRESS);

	printf("Board:  $%08x    Use (y/n)?",siop);
	if (gets(string) != NULL && strcmp(string,EMPTY) != 0 && strcmp(string,"y") != 0)  goto another;
	
loop:	printf("\nNumber of passes?");
	scanf("%d",&npass); ans = getchar();


/* INITIALIZE */

	pass = 0;
	while (npass-- > 0)
   {
/*	printf("\n");
	init_script();
*/
	printf("\nBoard:  $%08x\n",siop);
	printf("Script: $%08x\n",script=SCRIPT);

	dma_src  = (UBYTE *) AllocAligned(XFER_SIZE, 0x0f); *(script+1) = (ULONG) dma_src;
	dma_dest = (UBYTE *) AllocAligned(XFER_SIZE, 0x0f); *(script+2) = (ULONG) dma_dest;
	memcpy(dma_src, source, XFER_SIZE);
	memcpy(dma_dest, dest,  XFER_SIZE);

	printf("Source: $%08x\n", dma_src);
	printf("Dest:   $%08x\n", dma_dest);


/* EXECUTE SCRIPT */

	init_siop();

	siop->dsp = (ULONG) script;		/* load & execute script */

	i = 0;
	while ((istat = siop->istat & 0x81) == 0)				/* wait for INT(end-of-script) */
		{
//		if ((i++ & 0x16) == 16) sstat = siop[0x0c0004];		/* a signal to trigger analyzer */
		}
		

/* REPORT RESULTS */

	printf("\n");
	fail = 0;
	for (i = 0; i < XFER_SIZE; i++)
		{
		printf(" %02x", dma_dest[i]);
		if (dma_dest[i] != (i+1))		fail = fail + 1;
		if ((i+1)%16 == 0)			printf("\n");
		}
	if (fail == 0)
		{
		printf("Pass %d: \33[2mGOOD\33[0m",++pass);
		} else	{
//		sstat = siop[0x0c0000];					/* a signal to trigger analyzer */
		printf("Pass %d: \33[1mBAD \33[0m",++pass);
		bad = bad + 1;
		}
	printf("   Summary: Good(%d)  Bad(%d)\n",pass-bad,bad);

	printf("\n");
	printf("istat = $%02x\n", siop->istat);
	printf("dstat = $%02x\n", siop->dstat);
	printf("dcmd/dbc  = $%08x\n", siop->dbc);
	printf("dsp   = $%08x\n", siop->dsp);
	printf("dsps  = $%08x\n", siop->dsps);

	if (fail != 0)
		{
		printf("\nPress return to continue...\07");
		while ((ans = getchar()) != '\n');
		printf("\n");
		}


/* CLEAN UP & LEAVE */

	Forbid();
	FreeMem(dma_src,XFER_SIZE);
	FreeMem(dma_dest,XFER_SIZE);
/*	FreeMem(script,sizeof(SCRIPT));	*/
	Permit();
   }										/* endwhile */
	if (pass > 0)	goto loop;
	exit (0);
   }										/* endif */
	
   else
 	{
	printf("\n\33[1;3mA3090 SCSI Host Adaptor not found!\33[0m\n\n");
	Delay(150);
 	}

}										/* endmain */


/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/
void init_siop()	/* Reset NCR chip and set up a few things */
{
	siop->istat  |= 0x80;						/* abort current operation	*/
	Delay(1);
	siop->istat  |= 0x40;						/* software reset		*/
	siop->istat  ^= 0x40;
	Delay(1);

	siop->dcntl   = 0xc1;						/* SCLK=66MHz, 710	*/
	siop->dmode   = 0xa0;						/* 4-xfer burst, FC2-0 = 101	*/
	siop->ctest7 |= 0x80;						/* disable cache line bursts	*/

										/* clear any pending interrupts	*/
	while ((istat = siop->istat & 0x03) != 0)
		{
		if (istat & 2) sstat = siop->sstat0;
		Delay(1);							/* See DATA manual, pp 2-14,15	*/
		if (istat & 1) dstat = siop->dstat;
		Delay(1);
		}
}


/*----------------------------------------------------------------------------------*/
/*void init_script()	/* Make sure script starts on a 32-bit boundary */
/*{
/*	if (script = (ULONG *) AllocMem(sizeof(SCRIPT)+3,MEMF_PUBLIC))
/*		{
/*		Forbid();
/*		FreeMem(script,sizeof(SCRIPT)+3);
/*		script = (ULONG *) AllocAbs(sizeof(SCRIPT),(APTR)((((ULONG)script)+3)&(~3)));
/*		Permit();
/*		}
/*	memcpy(script, (ULONG *) SCRIPT, sizeof(SCRIPT));
/*}
*/


/*----------------------------------------------------------------------------------*/
ULONG *AllocAligned(ULONG size,ULONG chunk)
{
ULONG *data;


	if (data = (ULONG *) AllocMem(size+chunk, MEMF_PUBLIC))
		{
		Forbid();
		FreeMem(data,size+chunk);
		data = (ULONG *) AllocAbs(size,(APTR)((((ULONG)data)+chunk)&(~chunk)));
		Permit();
		}
	return(data);
}

volatile UBYTE *siop_reg;

/*----------------------------------------------------------------------------------*/
/* This function is used to copy a 32-bit value into a register. The AT only
 * allows 8 or 16-bit transfers and the c710 only allows 8 or 32-bit
 * transfers, so the value is copied in one byte at a time. */

void write_long_reg(int reg, ULONG addr)
{
int i;
	printf("\n");
	for (i = 3; i >= 0; i--)
		{
	/*	printf("reg= $%08x  val= $%02x\n", siop+reg+i, (addr>>8 * (3-i)) & 0xff);	*/
		siop_reg[reg+i] = (addr>>(8 * (3-i))) & 0xff;
		}
}
