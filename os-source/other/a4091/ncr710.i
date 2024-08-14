; ncr710.i

	INCLUDE "exec/types.i"

; NOTE: this creates offsets for use by big-endian processors.  Internal
; scripts use little-endian addresses, apparently (data manual, prelim,
; p10).

 STRUCTURE ncr710,0
	UBYTE sien	; SCSI interrupt enable
	UBYTE sdid	; SCSI destination ID
	UBYTE scntl1	; SCSI control reg 1
	UBYTE scntl0	; SCSI control reg 0
	UBYTE socl	; SCSI Output Control Latch
	UBYTE sodl	; SCSI Output Data Latch
	UBYTE sxfer	; SCSI Transfer reg
	UBYTE scid	; SCSI Chip ID
	UBYTE sbcl	; SCSI Bus Control Lines
	UBYTE sbdl	; SCSI Bus Data Lines		(read only)
	UBYTE sidl	; SCSI Input Data Latch	(read only)
	UBYTE sfbr	; SCSI First Byte Received
	UBYTE sstat2	; SCSI Status Register 2	(read only)
	UBYTE sstat1	; SCSI Status Register 1	(read only)
	UBYTE sstat0	; SCSI Status Register 0	(read only)
	UBYTE dstat	; DMA Status			(read only)
	ULONG dsa	; Data Structure Address
	UBYTE ctest3	; Chip Test Register 3		(read only)
	UBYTE ctest2	; Chip Test Register 2		(read only)
	UBYTE ctest1	; Chip Test Register 1		(read only)
	UBYTE ctest0	; Chip Test Register 0
	UBYTE ctest7	; Chip Test Register 7
	UBYTE ctest6	; Chip Test Register 6
	UBYTE ctest5	; Chip Test Register 5
	UBYTE ctest4	; Chip Test Register 4
	ULONG temp	; Temporary Stack
	UBYTE lcrc	; Longitudinal Parity
	UBYTE ctest8	; Chip Test Register 8
	UBYTE istat	; Interrupt Status
	UBYTE dfifo	; DMA FIFO
	UBYTE dcmd	; DMA Command
	STRUCT dbc,3	; DMA Byte Count (actually 3 bytes!)
	ULONG dnad	; DMA Next Address
	ULONG dsp	; DMA SCRIPTs Pointer
	ULONG dsps	; DMA SCRIPTs Pointer Save
	ULONG scratch	; General Purpose Scratch Pad
	UBYTE dcntl	; DMA Control
	UBYTE dwt	; DMA Watchdog Timer
	UBYTE dien	; DMA Interrupt Enable
	UBYTE dmode	; DMA Mode
	ULONG adder	; Sum output of internal adder	(read only)
 LABEL ncr_SIZEOF

; define all the instructions for the 53c710
 STRUCTURE jump_inst,0
	UBYTE jp_op	; 10XXXMCI  X=opcode, MCI=phase
	UBYTE jp_control ; R000JDPW  R=Relative, J=Jump on True/False,
			;           DP=Compare Data/Phase, W=Wait
	UBYTE jp_mask	; XXXXXXXX  X=mask for compare
	UBYTE jp_data	; XXXXXXXX  X=data for compare with SFBR
	ULONG jp_addr	; address or offset for jump
 LABEL jump_inst_SIZEOF

 STRUCTURE io_inst,0
	UBYTE io_op	; 01XXX00A  X=opcode, A=select_with_atn
	UBYTE io_id	; 87654321  SCSI ID
	UBYTE io_io1	; 00000CT0  C=set/clear carry, T=set/clear target
	UBYTE io_io2	; 0A00N000  A=set/clear ack, N=set/clear atn
	ULONG io_res	; reserved
 LABEL io_inst_SIZEOF

 STRUCTURE rw_reg_inst,0
	UBYTE rw_op	; 01XXXYY0  X=opcode, Y=operator
	UBYTE rw_reg	; 00XXXXXX  X=register addr
	UBYTE rw_imm	; XXXXXXXX  X=8-bit immediate
	UBYTE rw_res	; 00000000  reserved
	ULONG rw_res2	; 0 - reserved
 LABEL rw_reg_inst_SIZEOF

 STRUCTURE memmove_inst,0
	UBYTE mm_op	; 11000000  Memory-to-Memory move
	STRUCT mm_len,3	; 24-bit length in bytes for move
	ULONG mm_source	; source address for move
	ULONG mm_dest	; dest address for move
 LABEL memmove_inst_SIZEOF

 STRUCTURE move_inst,0
	UBYTE move_op	; 00ITOMCI I=Indrect, T=Table Indirect, O=Opcode,
			; MCI = phase
	STRUCT move_len,3 ; 24-bit length in bytes for move
	ULONG move_addr	; address for move (or 24-bit offset for Table Indirect
 LABEL move_inst_SIZEOF


; table indirect move data
 STRUCTURE move_data,0
	ULONG md_len	; 24-bit length in bytes for move (high byte 0)
	ULONG md_addr	; address for move (or 24-bit offset for Table Indirect
 LABEL move_data_SIZEOF



; Select data, for table-indirect Select commands
 STRUCTURE SelectData,0
	UBYTE sel_res1
	UBYTE sel_id
	UBYTE sel_sync
	UBYTE sel_res2	; res1/2 == 0
 LABEL SelectData_SIZEOF


; an individual scheduler entry
 STRUCTURE Sched_entry,0
	STRUCT se_next_sched,jump_inst_SIZEOF	; gets changed to NOP to schedule
	STRUCT se_make_jump,memmove_inst_SIZEOF	; changes the NOP back to JUMP
	STRUCT se_save_dsa,memmove_inst_SIZEOF	; set current_dsa
	STRUCT se_get_dsa,memmove_inst_SIZEOF	; get dsa value
	STRUCT se_start,jump_inst_SIZEOF	; start the IO
 LABEL Sched_entry_SIZEOF


; the scheduler script.  An array of scheduler entries, followed by a jump
; to wait for select/reselect/new-IO.
; These could be part of the DSA_entry, and have the last one jump to
; the get_default_dsa/jump_sleep.
 STRUCTURE Scheduler,0
	STRUCT sch_array,Sched_entry_SIZEOF*CMD_BLKS;
	STRUCT sch_jump_sleep,jump_inst_SIZEOF	; nothing to do, wait for reselect
 LABEL Scheduler_SIZEOF


; an individual reselect entry
; To use, check_lun, check_tag, address, and no_nexus must be patched.
; Address will be &(DSA_values[i]).
; One of these is added to the list for each ID at scheduling time, since
; it would be painful for the 710 to do it.  It should be carefully
; removed at completion time, in case the 710 is running.  It's safer if
; it isn't which will usually be the case.
 STRUCTURE Find_addr_entry,0
	STRUCT fa_check_addr,jump_inst_SIZEOF	; JUMP REL(tN) IF NOT 0x01
	STRUCT fa_get_lun,move_inst_SIZEOF	; MOVE SCRATCH(1) TO SFBR
	STRUCT fn_set_sxfer,rw_reg_inst_SIZEOF	; MOVE 0x00 to SXFER (patched)
	STRUCT fa_check_lun,jump_inst_SIZEOF	; JUMP no_nexus_found
 LABEL Find_addr_entry_SIZEOF


 STRUCTURE Find_lun_entry,0
	STRUCT fl_check_lun,jump_inst_SIZEOF	; JUMP no_nexus_found IF NOT 0x00
	STRUCT fl_get_tag,jump_inst_SIZEOF	; CALL get_tag
	STRUCT fl_check_tag,jump_inst_SIZEOF	; JUMP no_nexus_found (patched)
 LABEL Find_lun_entry_SIZEOF


; if we're not using tag queuing, don't clear ack - it was already cleared
; in get_tag, we may lose something here if we clear it.
 STRUCTURE Find_nexus_entry,0
	STRUCT fn_check_tag,jump_inst_SIZEOF	; JUMP no_nexus_found IF NOT 0x00
	STRUCT fn_clear_ack,io_inst_SIZEOF	; CLEAR ACK
	STRUCT fn_save_dsa,memmove_inst_SIZEOF	; MOVE 4,address,g->current_dsa
	STRUCT fn_get_dsa,memmove_inst_SIZEOF	; MOVE 4,address,DSA
	STRUCT fn_set_up,jump_inst_SIZEOF	; JUMP nexus_found
 LABEL Find_nexus_entry_SIZEOF
 ; 56 bytes

 ; must always have one of these in the DSA register.
 STRUCTURE DSA_entry,0
	STRUCT dsa_move_data,move_data_SIZEOF	;  0	data move
	STRUCT dsa_save_data,move_data_SIZEOF	;  8	restore from this
	APTR   dsa_final_ptr			; 16	addr+len
	STRUCT dsa_select_data,SelectData_SIZEOF ; 20	selection data
	STRUCT dsa_status_data,move_data_SIZEOF	; 24	1 byte to status
	STRUCT dsa_recv_msg,move_data_SIZEOF	; 32	1 byte to message
	STRUCT dsa_send_msg,move_data_SIZEOF	; 40	N byte send
	STRUCT dsa_command_data,move_data_SIZEOF ; 48	command move
	STRUCT dsa_send_buf,8		; 56 max is ident, simple_queue, sync
	STRUCT dsa_recv_buf,8		; 64 recv_msg and extd_msg point here
	UBYTE  dsa_status_buf		; 72 status gets put here
	STRUCT dsa_pad,3		; 73 to next longword
	STRUCT dsa_reselect_entry,Find_nexus_entry_SIZEOF ; 76 identifies this nexus
	STRUCT head_tail_data,16+16+16	; 16 head, 16 tail, 16 for alignment
 LABEL DSA_entry_SIZEOF


