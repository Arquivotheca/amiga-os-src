/* ncr53c710.h */

#include <exec/types.h>

// NOTE: this creates offsets for use by big-endian processors.  Internal
// scripts use little-endian addresses, apparently (data manual, prelim,
// p10).

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

// Bit definitions

// scntl0
#define SCNTL0F_ARB1	(1<<7)
#define SCNTL0F_ARB0	(1<<6)
#define SCNTL0F_START	(1<<5)
#define SCNTL0F_WATN	(1<<4)
#define SCNTL0F_EPC	(1<<3)
#define SCNTL0F_EPG	(1<<2)
#define SCNTL0F_AAP	(1<<1)
#define SCNTL0F_TRG	(1<<0)

// scntl1
#define SCNTL1F_EXC	(1<<7)
#define SCNTL1F_ADB	(1<<6)
#define SCNTL1F_ESR	(1<<5)
#define SCNTL1F_CON	(1<<4)
#define SCNTL1F_RST	(1<<3)
#define SCNTL1F_AESP	(1<<2)
#define SCNTL1F_SND	(1<<1)
#define SCNTL1F_RCV	(1<<0)

// sien
#define SIENF_MA	(1<<7)
#define SIENF_FCMP	(1<<6)
#define SIENF_STO	(1<<5)
#define SIENF_SEL	(1<<4)
#define SIENF_SGE	(1<<3)
#define SIENF_UDC	(1<<2)
#define SIENF_RST	(1<<1)
#define SIENF_PAR	(1<<0)

// sxfer
#define SXFERF_DHP	(1<<7)
#define SXFERF_TP2	(1<<6)
#define SXFERF_TP1	(1<<5)
#define SXFERF_TP0	(1<<4)
#define SXFERF_MO3	(1<<3)
#define SXFERF_MO2	(1<<2)
#define SXFERF_MO1	(1<<1)
#define SXFERF_MO0	(1<<0)

#define SXFERB_TP0	4

// socl
#define SOCLF_REQ	(1<<7)
#define SOCLF_ACK	(1<<6)
#define SOCLF_BSY	(1<<5)
#define SOCLF_SEL	(1<<4)
#define SOCLF_ATN	(1<<3)
#define SOCLF_MSG	(1<<2)
#define SOCLF_CD	(1<<1)
#define SOCLF_IO	(1<<0)

// sbcl
#define SBCLF_REQ	(1<<7)
#define SBCLF_ACK	(1<<6)
#define SBCLF_BSY	(1<<5)
#define SBCLF_SEL	(1<<4)
#define SBCLF_ATN	(1<<3)
#define SBCLF_MSG	(1<<2)
#define SBCLF_CD	(1<<1) /* read */
#define SBCLF_IO	(1<<0) /* read */
#define SBCLF_SSCF1	(1<<1) /* write */
#define SBCLF_SSCF0	(1<<0) /* write */

// dstat
#define DSTATF_DFE	(1<<7)

#define DSTATF_BF	(1<<5)
#define DSTATF_ABRT	(1<<4)
#define DSTATF_SSI	(1<<3)
#define DSTATF_SIR	(1<<2)
#define DSTATF_WTD	(1<<1)
#define DSTATF_IID	(1<<0)

// sstat0
#define SSTAT0F_MA	(1<<7)
#define SSTAT0F_FCMP	(1<<6)
#define SSTAT0F_STO	(1<<5)
#define SSTAT0F_SEL	(1<<4)
#define SSTAT0F_SGE	(1<<3)
#define SSTAT0F_UDC	(1<<2)
#define SSTAT0F_RST	(1<<1)
#define SSTAT0F_PAR	(1<<0)

// sstat1
#define SSTAT1F_ILF	(1<<7)
#define SSTAT1F_ORF	(1<<6)
#define SSTAT1F_OLF	(1<<5)
#define SSTAT1F_AIP	(1<<4)
#define SSTAT1F_LOA	(1<<3)
#define SSTAT1F_WOA	(1<<2)
#define SSTAT1F_RST	(1<<1)
#define SSTAT1F_SDP	(1<<0)

// sstat2
#define SSTAT2F_FF3	(1<<7)
#define SSTAT2F_FF2	(1<<6)
#define SSTAT2F_FF1	(1<<5)
#define SSTAT2F_FF0	(1<<4)
#define SSTAT2F_SDP	(1<<3)
#define SSTAT2F_MSG	(1<<2)
#define SSTAT2F_CD	(1<<1)
#define SSTAT2F_IO	(1<<0)

// ctest0

#define CTEST0F_BTD	(1<<6)
#define CTEST0F_GRP	(1<<5)
#define CTEST0F_EAN	(1<<4)
#define CTEST0F_HSC	(1<<3)
#define CTEST0F_ERF	(1<<2)

#define CTEST0F_DDIR	(1<<0)

// ctest2

#define CTEST2F_SIGP	(1<<6)
#define CTEST2F_SOFF	(1<<5)
#define CTEST2F_SFP	(1<<4)
#define CTEST2F_DFP	(1<<3)
#define CTEST2F_TEOP	(1<<2)
#define CTEST2F_DREQ	(1<<1)
#define CTEST2F_DACK	(1<<0)

// ctest8




#define CTEST8F_FLF	(1<<3)
#define CTEST8F_CLF	(1<<2)
#define CTEST8F_FM	(1<<1)
#define CTEST8F_SM	(1<<0)

// istat
#define ISTATF_ABRT	(1<<7)
#define ISTATF_RST	(1<<6)
#define ISTATF_SIGP	(1<<5)

#define ISTATF_CON	(1<<3)

#define ISTATF_SIP	(1<<1)
#define ISTATF_DIP	(1<<0)

// dmode
#define DMODEF_BL1	(1<<7)
#define DMODEF_BL0	(1<<6)
#define DMODEF_FC2	(1<<5)
#define DMODEF_FC1	(1<<4)
#define DMODEF_PD	(1<<3)
#define DMODEF_FAM	(1<<2)
#define DMODEF_U0	(1<<1)
#define DMODEF_MAN	(1<<0)

// dien


#define DIENF_BF	(1<<5)
#define DIENF_ABRT	(1<<4)
#define DIENF_SSI	(1<<3)
#define DIENF_SIR	(1<<2)
#define DIENF_WTD	(1<<1)
#define DIENF_IID	(1<<0)

// dcntl
#define DCNTLF_CF1	(1<<7)
#define DCNTLF_CF0	(1<<6)
#define DCNTLF_EA	(1<<5)
#define DCNTLF_SSM	(1<<4)
#define DCNTLF_LLM	(1<<3)
#define DCNTLF_STD	(1<<2)
#define DCNTLF_FA	(1<<1)
#define DCNTLF_COM	(1<<0)

// define all the instructions for the 53c710

struct jump_inst {
	UBYTE op;	// 10XXXMCI  X=opcode, MCI=phase
	UBYTE control;	// R0C0JDPW  R=Relative, J=Jump on True/False,
			//           DP=Compare Data/Phase, W=Wait, C=Carry
	UBYTE mask;	// XXXXXXXX  X=mask for compare
	UBYTE data;	// XXXXXXXX  X=data for compare with SFBR
	ULONG addr;	// address or offset for jump
};

struct io_inst {
	UBYTE op;	// 01XXX00A  X=opcode, A=select_with_atn
	UBYTE id;	// 87654321  SCSI ID
	UBYTE io1;	// 00000CT0  C=set/clear carry, T=set/clear target
	UBYTE io2;	// 0A00N000  A=set/clear ack, N=set/clear atn
	ULONG res;	// reserved
};

struct rw_reg_inst {
	UBYTE op;	// 01XXXYYZ  X=opcode, Y=operator Z=carry
	UBYTE reg;	// 00XXXXXX  X=register addr
	UBYTE imm;	// XXXXXXXX  X=8-bit immediate
	UBYTE res;	// 00000000  reserved
	LONG res2;	// 0 - reserved
};

struct memmove_inst {
	UBYTE op;	// 11000000  Memory-to-Memory move
	UBYTE len[3];	// 24-bit length in bytes for move
	ULONG source;	// source address for move
	ULONG dest;	// dest address for move
};

struct move_inst {
	UBYTE op;	// 00ITOMCI I=Indrect, T=Table Indirect, O=Opcode,
			// MCI = phase
	UBYTE len[3];	// 24-bit length in bytes for move
	ULONG addr;	// address for move (or 24-bit offset for Table Indirect
};

// table indirect move data
struct move_data {
	ULONG len;	// 24-bit length in bytes for move (high byte 0)
	ULONG addr;	// address for move (or 24-bit offset for Table Indirect
};


// Select data, for table-indirect Select commands
struct SelectData {
	UBYTE res1,id,sync,res2;	// res1/2 == 0
};

// an individual scheduler entry
struct Sched_entry {
	struct jump_inst    next_sched;	// gets changed to NOP to schedule
	struct memmove_inst make_jump;	// changes the NOP back to JUMP
	struct memmove_inst save_dsa;	// set current_dsa
	struct memmove_inst get_dsa;	// get dsa value
	struct jump_inst    start;	// start the IO
};

// the scheduler script.  An array of scheduler entries, followed by a jump
// to wait for select/reselect/new-IO.
// These could be part of the DSA_entry, and have the last one jump to
// the get_default_dsa/jump_sleep.
struct Scheduler {
	struct Sched_entry  array[CMD_BLKS];
	struct jump_inst    jump_sleep;	// nothing to do, wait for reselect
};

// an individual reselect entry
// To use, check_lun, check_tag, address, and no_nexus must be patched.
// Address will be &(DSA_values[i]).
// One of these is added to the list for each ID at scheduling time, since
// it would be painful for the 710 to do it.  It should be carefully
// removed at completion time, in case the 710 is running.  It's safer if
// it isn't which will usually be the case.
struct Find_addr_entry {
	struct jump_inst    check_addr;	// JUMP REL(tN) IF NOT 0x01
	struct move_inst    get_lun;	// MOVE SCRATCH1 TO SFBR
	struct rw_reg_inst  set_sxfer;	// MOVE 0x00 to SXFER (patched)
	struct jump_inst    check_lun;	// JUMP no_nexus_found
};

struct Find_lun_entry {
	struct jump_inst    check_lun;	// JUMP no_nexus_found IF NOT 0x00
	struct jump_inst    get_tag;	// CALL get_tag
	struct jump_inst    check_tag;	// JUMP no_nexus_found (patched)
};

// if we're not using tag queuing, don't clear ack - it was already cleared
// in get_tag, we may lose something here if we clear it.
struct Find_nexus_entry {
	struct jump_inst    check_tag;	// JUMP no_nexus_found IF NOT 0x00
	struct io_inst      clear_ack;	// CLEAR ACK
	struct memmove_inst save_dsa;	// MOVE 4,address,g->current_dsa
	struct memmove_inst get_dsa;	// MOVE 4,address,DSA
	struct jump_inst    set_up;	// JUMP nexus_found
};

// must always have one of these in the DSA register.
struct DSA_entry {
	struct move_data    move_data;	//  0	data move
	struct move_data    save_data;	//  8	restore from this
	UBYTE 		   *final_ptr;	// 16	addr+len
	struct SelectData   select_data;// 20	selection data
	struct move_data    status_data;// 24	1 byte to status
	struct move_data    recv_msg;	// 32	1 byte to message
	struct move_data    send_msg;	// 40	N byte send
	struct move_data    command_data;// 48	command move
	UBYTE send_buf[8];		// 56 max is ident, simple_queue, sync
	UBYTE recv_buf[8];		// 64 recv_msg and extd_msg point here
	UBYTE status_buf[1];		// 72 status gets put here
	UBYTE pad[3];			// 73 to next longword
	struct Find_nexus_entry reselect_entry; // 76 identifies this nexus
	UBYTE head_tail_data[16+16+16];	// 16 head, 16 tail, 16 for alignment
};
