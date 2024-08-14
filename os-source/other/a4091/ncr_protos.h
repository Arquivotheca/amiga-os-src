/* Prototypes for functions defined in
ncr.c
 */

extern ULONG SCRIPT[];

extern char * External_Names[];

extern ULONG E__SCRATCH_Used[];

extern ULONG E__DSA_Used[];

extern ULONG E__SCRATCH3_Used[];

extern ULONG E_current_dsa_Used[];

extern ULONG E_default_dsa_Used[];

extern ULONG E_default_send_buf_Used[];

extern ULONG E_modify_data_Used[];

extern ULONG E_Scheduler_Used[];

extern ULONG E_sync_buf_Used[];

extern ULONG E_zero_Used[];

extern ULONG Rel_Patches[];

extern ULONG R_move_data_Used[];

extern ULONG R_select_data_Used[];

extern ULONG R_status_data_Used[];

extern ULONG R_recv_msg_Used[];

extern ULONG R_send_msg_Used[];

extern ULONG R_command_data_Used[];

extern char * Absolute_Names[];

extern ULONG A_io_complete_Used[];

extern ULONG A_scatter_dma_Used[];

extern ULONG A_sync_received_Used[];

extern ULONG A_modify_pointer_fetched_Used[];

extern ULONG A_unusual_message_in_Used[];

extern ULONG A_extended_not_len_3_Used[];

extern ULONG A_extended_not_sync_Used[];

extern ULONG A_target_rejected_msg_Used[];

extern ULONG A_data_eaten_Used[];

extern ULONG A_unknown_phase_Used[];

extern ULONG A_no_msg_after_status_Used[];

extern ULONG A_not_command_complete_Used[];

extern ULONG A_bad_reselect_Used[];

extern ULONG A_bad_address_Used[];

extern ULONG A_selected_as_target_Used[];

extern ULONG A_big_error_1_Used[];

extern ULONG A_big_error_2_Used[];

extern ULONG A_reselect_during_select_Used[];

extern ULONG A_in_wait_Used[];

extern ULONG LABELPATCHES[];

extern ULONG INSTRUCTIONS;

extern ULONG PATCHES;

static void poll_cia(ULONG );

static void loop(struct SCSIGlobals * );

static void HandleCommands(struct SCSIGlobals * );

extern ULONG clear_ack_inst;

extern ULONG my_nop_inst;

static void Schedule(struct SCSIGlobals * , struct HDUnit * , struct CommandBlock * , UBYTE );

static void UnSchedule(struct SCSIGlobals * , struct CommandBlock * , LONG );

static void CleanUp(struct SCSIGlobals * , struct CommandBlock * , struct HDUnit * );

extern struct DSA_entry standard_DSA_entry;

extern ULONG jump_rel_inst;

extern struct Sched_entry standard_sched_entry;

extern struct jump_inst standard_scheduler_jump;

extern struct Find_lun_entry standard_lun_entry;

extern struct Find_nexus_entry standard_nexus_entry;

static void init_structs(struct SCSIGlobals * , struct IOTask_Globals * );

static void init_dsa(struct SCSIGlobals * , struct DSA_entry * , LONG );

static void init_chip(struct SCSIGlobals * );

void add_unit(struct SCSIGlobals * , struct HDUnit * );

void remove_unit(struct SCSIGlobals * , struct HDUnit * );

static void addsync(struct CommandBlock * , LONG , LONG );

void abort_all(struct SCSIGlobals * , LONG );

void set_sxfer(struct SCSIGlobals * , struct HDUnit * , struct CommandBlock * , UBYTE );

static void HandleInts(struct SCSIGlobals * );

void __stdargs SCSITask(struct SCSIGlobals * , volatile struct ncr710 * , struct Task * , struct MsgPort * , LONG , struct HDDevice * );

