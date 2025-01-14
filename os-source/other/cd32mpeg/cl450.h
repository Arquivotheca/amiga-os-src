/*****************************************************************************
*
* This file consists of the CL450 specific information and macros needed to
* for control of CL450 for MPEG video.
*
******************************************************************************/

#ifndef CL450_CHIP_H
#define CL450_CHIP_H

/* The CL450 registers are word address. */

#if 0 /* Don't remove this #if ! */

#define VID_YDATA    (0x00 * 2)
#define CMEM_DATA    (0x01 * 2)
#define VID_UVDATA   (0x05 * 2)
#define CPU_CONTROL  (0x10 * 2)
#define CPU_PC       (0x11 * 2)
#define CPU_INTENB   (0x13 * 2)
#define CPU_TADDR    (0x1C * 2)
#define CPU_IADDR    (0x1f * 2)
#define CPU_IMEM     (0x21 * 2)
#define CPU_TMEM     (0x23 * 2)
#define CPU_INT      (0x2A * 2)
#define CPU_NEWCMD   (0x2b * 2)
#define CMEM_CONTROL (0x40 * 2)
#define CMEM_STATUS  (0x41 * 2)
#define CMEM_DMACTRL (0x42 * 2)
#define HOST_RADDR1  (0x44 * 2)
#define HOST_RDATA1  (0x46 * 2)
#define HOST_CONTROL (0x48 * 2)
#define HOST_SCR0    (0x49 * 2)
#define HOST_SCR1    (0x4a * 2)
#define HOST_SCR2    (0x4b * 2)
#define HOST_INTVECW (0x4C * 2)
#define HOST_INTVECR (0x4E * 2)
#define DRAM_REFCNT  (0x56 * 2)
#define VID_CONTROL  (0x76 * 2)
#define VID_REGDATA  (0x77 * 2)

#endif

/* Slightly more useful version of above */

struct CL450Regs
{
	volatile UWORD	VID_YDATA;	/* 0x00 */
	volatile UWORD	CMEM_DATA;	/* 0x02 */
	volatile UWORD	Private0[3];
	volatile UWORD	VID_UVDATA;	/* 0x0a */
	volatile UWORD	Private1[10];
	volatile UWORD	CPU_CONTROL;	/* 0x20 */
	volatile UWORD	CPU_PC;		/* 0x22 */
	volatile UWORD	Private2;
	volatile UWORD	CPU_INTENB;	/* 0x26 */
	volatile UWORD	Private3[8];
	volatile UWORD	CPU_TADDR;	/* 0x38 */
	volatile UWORD	Private4[2];
	volatile UWORD	CPU_IADDR;	/* 0x3e */
	volatile UWORD	Private5;
	volatile UWORD	CPU_IMEM;	/* 0x42 */
	volatile UWORD	Private6;
	volatile UWORD	CPU_TMEM;	/* 0x46 */
	volatile UWORD	Private7[6];
	volatile UWORD	CPU_INT;	/* 0x54 */
	volatile UWORD	CPU_NEWCMD;	/* 0x56 */
	volatile UWORD	Private8[20];
	volatile UWORD	CMEM_CONTROL;	/* 0x80 */
	volatile UWORD	CMEM_STATUS;	/* 0x82 */
	volatile UWORD	CMEM_DMACTRL;	/* 0x84 */
	volatile UWORD	Private9;
	volatile UWORD	HOST_RADDR1;	/* 0x88 */
	volatile UWORD	Private10;
	volatile UWORD	HOST_RDATA1;	/* 0x8c */
	volatile UWORD	Private11;
	volatile UWORD	HOST_CONTROL;	/* 0x90 */
	volatile UWORD	HOST_SCR0;	/* 0x92 */
	volatile UWORD	HOST_SCR1;	/* 0x94 */
	volatile UWORD	HOST_SCR2;	/* 0x96 */
	volatile UWORD	HOST_INTVECW;	/* 0x98 */
	volatile UWORD	Private12;
	volatile UWORD	HOST_INTVECR;	/* 0x9c */
	volatile UWORD	Private13[7];
	volatile UWORD	DRAM_REFCNT;	/* 0xac */
	volatile UWORD	Private14[31];
	volatile UWORD	VID_CONTROL;	/* 0xec */
	volatile UWORD	VID_REGDATA;	/* 0xee */
};

typedef struct CL450Regs CL450Regs;


/* Bit definitions for CPU_CONTROL. */
#define CE_N 0x0001

/* Bit definitions for CPU_PC. */
#define IE   0x0200
#define PC_MASK 0x01FF

/* Bit defintions for CPU_INTENB. */
#define NCE  0x0010
#define VSE  0x0002

/* Bit definitions for CPU_TADDR3. */
#define TADDR_MASK 0x007F

/* Bit definitions for CPU_IADDR. */
#define WADD_MASK 0x03FE

/* Bit definitions for CPU_INIT. */
#define NCS  NCE
#define VSS  NCE

/* Bit definitions for CPU_NEWCMD. */
#define CMD  0x0001

/* Bit definitions for CMEM_CONTROL. */
#define RST  0x0040
#define BS   0x0020
#define CR   0x0004
#define CRE  0x0002
#define CRST 0x0001

/* Bit definitions for CMEM_STATUS. */
#define CDCTR_MASK 0x1F00
#define CWCTR_MASK 0x00F0
#define CRCTR_MASK 0x000F

/* Bit definitions for CMEM_DMACTRL. */
#define _1Q  0x0100
#define _2Q  0x0180
#define _3Q  0x01C0
#define _4Q  0x01E0
#define _1QE 0x0010
#define _2QE 0x0008
#define _3QE 0x0004
#define _4QE 0x0002
#define DE   0x0001

/* Bit definitions for HOST_RADDR1. */
#define RADD_MASK 0x000F

/* Possible values for HOST_RADDR1 when reading. */
#define READ_CURRENT_CMD 0x0008
#define READ_CMD_STATUS  0x0009
#define READ_INT_STATUS  0x000A
#define READ_BS_BUF_FULL 0x000B

/* Bit definitions for HOST_CONTROL. */
#define NON_VECTORED 0x81
#define VECTORED_NONCLEAR 0x4081
#define VECTORED_CLEAR 0xc081

/* Bit definitions for HOST_SCR2. */
#define CS   0x1000
#define DIV_MASK 0x0FF8
#define SYSCLKHIGH_MASK 0x0007

/* Formula for System Clock Divisor. */
#define SYSTEM_CLOCK_DIVISOR(clk_freq) ((unsigned int) (clk_freq / 90000.0))

/* Bit definitions for HOST_SCR1. */
#define SYSCLKMID_MASK 0x7FFF

/* Bit definitions for HOST_SRC0. */
#define SYSCLKLOW_MASK 0x7FFF

/* Bit definitions for HOST_INTVECW and HOST_INTVECR. */
#define IPID_MASK 0x0700
#define IVECT_MASK 0x00FF

/* Bit definitions for DMA_REFCNT. */
#define REFCNT_MASK 0x0FFF

/* Formula for Refresh Clock Count. */
#define REFRESH_CLOCK_COUNT(r_ns,clk_ns,rows) ((unsigned int) (((r_ns - 512.0 * clk_ns)/(rows + 2))/clk_ns))

/* Bit definitions for VID_CONTROL. */

#define VID_SELK0K1     0x0000
#define VID_SELK2K3     0x0002
#define VID_SELACTIVE   0x0010
#define VID_SELMODE     0x000E
#define VID_SELLEFTBOR  0x0012
#define VID_SELBORDERR  0x0014
#define VID_SELBORDERGB 0x0016
#define VID_SELAUX      0x0018

#define DRAMSIZE        0x7FFF

/* The following are some VOODOO from the MSDOS code. */
#define  Toverlay_size     0x3d
#define  Tstart_overlay    0x3e

#define  CODE_LOADER_BEG   0x4f
#define  UCODE_CACHE_BEG   0x5a	/* ucode starts at 0x5a in Imem */
#define  UCODE_CACHE_END   0x200/* 512 entries in Imem */

#define  CACHE_SIZE_I      (UCODE_CACHE_END-UCODE_CACHE_BEG)	/* size of inst cache   */

/*
 * Header of microcode file.  Needed for parsing and loading microcode to
 * CL450.  Start address of microcode is contained in this header
 */


struct binary_hd
{
    UWORD magic;		/* the magic number; see below */
    UBYTE rev_hi;		/* format revision, high byte */
    UBYTE rev_lo;		/* format revision, low byte */
    UWORD isCD_I;		/* flag indicating CL450 vs. CL450i */
    UWORD init_pc;		/* initial microcode PC value */
    UWORD imem_dram;		/* load imem at 0 starting at this DRAM
				 * address */
    ULONG len;			/* total (file) length of microcode module */
    UWORD nseg;			/* number of segments defined */
    ULONG entry_pt;		/* entrypoint local segment # (first cache
				 * address) */
    char comment[80];		/* any additional note (copyright etc.) */
};

#define CL450_MAGIC     0xC3C3

#define REV_HI          1
#define REV_LO          0xf2

#define ENDIAN_REVERSAL_WORD(x) (((UWORD)((((x)>>8) & 0x00FF) | (((x) & 0xFF) << 8))))
#define ENDIAN_REVERSAL_LONG(x) (ENDIAN_REVERSAL_WORD(((x)>>16) & 0xFFFF) | (ENDIAN_REVERSAL_WORD((x) & 0xFFFF) << 16))

#define CL450_UPPER_THRESHOLD 43008

#define MPEG_CL450_OFFSET 0x0050000L
#define MPEG_CL450_GCLK_FREQ 40000000.0
#define MPEG_CL450_GCLK_PERIOD_NS (1000000000.0/MPEG_CL450_GCLK_FREQ)
#define MPEG_CL450_DRAM_REFRESH_NS 4000000
#define MPEG_CL450_DRAM_ROWS 512

enum
{
    NO_ERR = 0,
    ERR_BAD_START_ADDR,
    ERR_CMD_TIMEOUT,
    ERR_UCODE_FILE_NOT_FOUND,
    ERR_BAD_MAGIC,
    ERR_BAD_REV_NO,
    ERR_BAD_UCODE_HEADER,
    ERR_BAD_UCODEFILE,
    ERR_BAD_SEG_SIZE,
    ERR_BAD_TARGET_ADDR,
    ERR_BAD_START_IMEM,
    ERR_BAD_SEG_ALIGN
};

/* 2.0 Microcode Commands. */

#define CL450CMD_ACCESS_SCR			0x8312
#define CL450CMD_DISPLAY_STILL			0x000c
#define CL450CMD_FLUSH_BITSTREAM		0x8102
#define CL450CMD_INQUIRE_BUFFER_FULLNESS	0x8001
#define CL450CMD_NEW_PACKET			0x0408
#define CL450CMD_PAUSE				0x000e
#define CL450CMD_PLAY				0x000d
#define CL450CMD_RESET				0x8000
#define CL450CMD_SCAN				0x000a
#define CL450CMD_SET_BLANK			0x030f
#define CL450CMD_SET_BORDER			0x0407
#define CL450CMD_SET_COLOR_MODE			0x0111
#define CL450CMD_SET_INTERRUPT_MASK		0x0104
#define CL450CMD_SET_THRESHOLD			0x0103
#define CL450CMD_SET_VIDEO_FORMAT		0x0105
#define CL450CMD_SET_WINDOW			0x0406
#define CL450CMD_SINGLE_STEP			0x000b
#define CL450CMD_SLOW_MOTION			0x0109

/* CL450 Interrupt Mask Bits */

#define CL450INTB_DER	0	/* Bitstream Data Error */
#define CL450INTB_PIC	1	/* New picture display */
#define CL450INTB_GOP	2	/* First I-Picture display after group_start_code */
#define CL450INTB_SOS	3	/* First I-Picture display after sequence_header_code */
#define CL450INTB_LPD	4	/* Sequence_end_code found (VSYNC) */
#define CL450INTB_EOS	5	/* Sequence_end_code found (Decode-Time) */
#define CL450INTB_DEC	6	/* New picture decoded */
#define CL450INTB_BUF	8	/* Bitstream Buffer Underflow Error */
#define CL450INTB_NIS	9	/* sequence_header_code found */
#define CL450INTB_RDY	10	/* Ready for data */
#define CL450INTB_SCN	11	/* Picture decode complete in Scan() */

#define CL450INTF_DER	(1L << CL450INTB_DER)
#define CL450INTF_PIC	(1L << CL450INTB_PIC)
#define CL450INTF_GOP	(1L << CL450INTB_GOP)
#define CL450INTF_SOS	(1L << CL450INTB_SOS)
#define CL450INTF_LPD	(1L << CL450INTB_LPD)
#define CL450INTF_EOS	(1L << CL450INTB_EOS)
#define CL450INTF_DEC	(1L << CL450INTB_DEC)
#define CL450INTF_BUF	(1L << CL450INTB_BUF)
#define CL450INTF_NIS	(1L << CL450INTB_NIS)
#define CL450INTF_RDY	(1L << CL450INTB_RDY)
#define CL450INTF_SCN	(1L << CL450INTB_SCN)

#define CL450INTF_ALL	(CL450INTF_DER|CL450INTF_PIC|CL450INTF_GOP|CL450INTF_SOS|CL450INTF_LPD|CL450INTF_EOS|CL450INTF_DEC|CL450INTF_BUF|CL450INTF_NIS|CL450INTF_SCN)

#define HOST_CODE	0x4242

/* DRAM Variables */

#define	SEQ_SEM		0x20
#define	SEQ_CONTROL	0x22
#define HORIZONTAL_SIZE	0x24
#define VERTICAL_SIZE	0x26
#define	PICTURE_RATE	0x28
#define FLAGS		0x2a


#endif	/* CL450_CHIP_H */
