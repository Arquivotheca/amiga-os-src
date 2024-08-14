/* 		            Commodore
 *			    ---------
 * 
 * Copyright Philips Consumer Electronics / Key Modules Group - hORDe
 * All rights  are reserved. Reproduction in whole or part is prohibited 
 * without the prior written consent of the copyright owner
 *
 * Company Confidential
 *
 * File		: cmd_hndl.c
 *
 *
 * Rev		Date			Author Comments
 * 01	 	930311 PCM vd Zande.    Creation
*/
#include "gendef.h"
#include "cmd_hndl.h"
#include "serv_def.h"
#include "sts_q_id.h"

static byte		 idat		error_condition;               /* holds type of error for host */
static byte		 idat		host_cmd;		       /* command received from host*/
static byte		 idat		return_cmd;		       /* all commands but subcode led_cntrl*/
static byte		 idat		non_player_cmd;		       /* holds present non player command*/
static byte		 idat		int_cmd;		       /* internal representation of host command*/
static byte		 idat		player_cmd;		       /* holds present player_command */
static byte		 idat		prev_player_cmd;	       /* holds previous command for player*/
static byte		 idat		state;			       /* holds present state: (open ... play)*/
static byte		 idat		cmd_function_id;	       /* indicates present executed command */
static struct time	 idat		start_play;		       /* start time for playing */
static struct time	 idat		stop_play;		       /* stop time for playing */
static struct time	 idat		start_last_leadout;	       /* start time of last leadout present on disk*/	 
static struct subcode_frame idat	subcode_buffer;
static byte		 idat		service_opc;

static bit  	           		cmd_phase0;		       /* phase in player command execution*/
bit					accept_cmd;		       /* true ==> handler accepts new commands*/
static bit				cmd_received;                  /* ready to read new command from buffer */
static bit				priority_player_cmd;	       /* internal player command has to be executed*/
static bit				play_monitor;		       /* check play functions */
static bit				search_bwd;		       /* do every 500ms a jump of -200 grooves */
static bit				search_fwd;		       /* do every 500ms a jump of 200 grooves */
static bit				pause;			       /* copy of pause bits in MODE */
static bit				subcode_reading;               /* indicates busy waiting for subcode*/
								       /* before any host command*/
static bit				double_speed;		       /* set means double speed*/
static bit				autoq;		               /* auto report subcode yes or no */
static bit				playing_in_first_leadin;       /* indicates what is says */
static bit				speed_is_single;	       /* indicates the running on single speed*/
static bit				rom_data;		       /* rom to play */
static bit				subcode_read;		       /* TRUE subcode_buffer is valid*/



extern struct interface_field		player_interface;
extern byte				search_timer;

extern struct time			*get_leadout_time(void);
extern void				bcd_to_hex_time(struct time *, struct time *);
extern byte				bcd_to_hex(byte);
extern byte				GET_BUFFER(byte);
extern byte				compare_time(struct time *, struct time *);
extern void				led_0_on(void);
extern void				led_0_off(void);
extern void				led_1_on(void);
extern void				led_1_off(void);
extern void				set_dac_mode(byte);
extern void				set_play_mode(byte);
extern void				attenuate_on(void);
extern bit				is_cd_disc(void);
extern bit 				door_closed(void);
extern bit 				drive_is_pausing(void);
extern void				init_for_new_disc(void);
extern byte				mute_off(void);
extern byte				mute_on(void);
extern void				set_level_meter_mode(byte);
extern byte				give_peak_level_low(void);
extern byte				give_peak_level_high(void);

static byte const command_conversion_table[][4] =
	 /* OPEN_S	  STOP_S        PLAY_S      PAUSE_S */
{
/* RESEND   0 */  { RESEND,       RESEND,       RESEND,     RESEND      },
/* STOP     1 */  { S_DISK_ERR,   S_STAT,       STOP_C,     STOP_C      },
/* PAUSE    2 */  { S_DISK_ERR,   START_PAUSE,  PLAY_PAUSE, S_STAT      },
/* PLAY	    3 */  { S_DISK_ERR,   START_PLAY,   S_STAT,     PAUSE_PLAY  },
/* SET_PLAY 4 */  { S_DISK_ERR,   MODIFY_PLAY,  NEW_PLAY,   NEW_PLAY    },
/* ACT_LED  5 */  { LED_CNTRL,    LED_CNTRL,    LED_CNTRL,  LED_CNTRL   },
/* SENDQ    6 */  { SEND_Q,       SEND_Q,       SEND_Q,     SEND_Q      },
/* SENDID   7 */  { S_ID,         S_ID,         S_ID,       S_ID        },
/* SEEK     8 */  { S_DISK_ERR,   SEEK_STOP,    SEEK_PLAY,  SEEK_PLAY,  },
/* ABORT    9 */  { S_STAT,       S_STAT,       PLAY_PAUSE, S_STAT      },
/* DISC     10 */ { 0,            0,            0,          0,          },
/* RESV     11 */ { 0,            0,            0,          0,          },
/* ENT_DIA  12 */ { ENT_DIA,      ENT_DIA,      S_CMD_ERR,  S_CMD_ERR   },
/* DIA      13 */ { DIA,       	  DIA,       	S_CMD_ERR,  S_CMD_ERR   },
};


/***********************************************************************************************
 * Function  : read_new_command
 * Input     :
 * Output    : -
 *
 * Abstract  : checks if command handler is allowed to read a new command
 *
 *
 * Decisions :
*/
static bit read_new_command(void)
{
   if (cmd_received && non_player_cmd == IDLE && !priority_player_cmd )
   {
      if ( (byte)(GET_BUFFER(0) & 0x0F) == 0x06 && !subcode_read )
	 return FALSE;
      cmd_received=FALSE;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : convert_to_int_cmd
 * Input     : host command
 * Output    : internal command
 *
 * Abstract  : tranforms host command to internal command depending on the state
 *
 *
 * Decisions :
*/
static byte convert_to_int_cmd(byte command)
{
   return command_conversion_table[command][state];
}

/***********************************************************************************************
 * Function  : non_player_command
 * Input     : internal command
 * Output    : true false
 *
 * Abstract  : if non_player_command return true else false
 *
 *
 * Decisions :
*/
static bit non_player_command(byte command)
{
   return (command & 0x80);
}

/***********************************************************************************************
 * Function  : convert_error_code
 * Input     : code of player module
 * Output    : code of host
 *
 * Abstract  : translates player error code to host error code
 *
 *
 * Decisions :
*/
static byte convert_error_code(byte code)
{
   switch (code)
   {
      case ILLEGAL_COMMAND:	    return BAD_COMMAND;
      case ILLEGAL_PARAMETER:	    return INVALID_START_ADDRESS;
      case SLEDGE_ERROR:	    return SLED_ERROR;
      case FOCUS_ERROR:		    return DISK_UNREADABLE;
      case MOTOR_ERROR:		    return SPINDLE_SERVO_ERROR;
      case RADIAL_ERROR:	    return ABNORMAL_TRACK_JUMP;	
      case PLL_LOCK_ERROR:	    return DISK_UNREADABLE;
      case SUBCODE_TIMEOUT_ERROR:   return ABNORMAL_SEEK;
      case SUBCODE_NOT_FOUND:	    return ABNORMAL_SEEK;	
      case TOC_READ_ERROR:	    return DISK_UNREADABLE;
      case JUMP_ERROR:		    return ABNORMAL_TRACK_JUMP;	
      case HF_DETECTOR_ERROR:	    return DISK_UNREADABLE;	
   }
}

/***********************************************************************************************
 * Function  : set_error
 * Input     : -
 * Output    : -
 *
 * Abstract  : error; notify host, reset variables
 *
 *
 * Decisions :
*/
void set_error()
{
   error_condition=convert_error_code(player_interface.param1);
   non_player_cmd=SET_ERROR_STATUS; 	/* send player error to host*/

   player_cmd=IDLE;
   cmd_function_id=0;
   priority_player_cmd=FALSE;

   accept_cmd=TRUE;

   play_monitor=FALSE;
   search_bwd=FALSE;
   search_fwd=FALSE;
   pause=FALSE;
}

/***********************************************************************************************
 * Function  : ac
 * Input     : -
 * Output    : READY
 *
 * Abstract  : this function indicates that a new command may be placed in the command buffer
 *
 *
 * Decisions :
*/
byte ac(void)
{
   accept_cmd=TRUE;
   return READY;
}

/*--------------------------------------------------------------------------------
 *
 *			PLAYER COMMANDS
 *
 *--------------------------------------------------------------------------------*/

/***********************************************************************************************
 * Function  : player_command
 * Input     : a_command, parm1, parm2, parm3
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions : error_condition is updated in case of an error situation
*/

byte player_command(byte cmd, byte par1, byte par2, byte par3)
{
   if (cmd != prev_player_cmd || !cmd_phase0)
   {
      /*PHASE 0: ISSUE COMMAND TO PLAYER */
      prev_player_cmd=cmd;
      player_interface.p_status=BUSY;
      player_interface.a_command=cmd;
      player_interface.param1=par1;
      player_interface.param2=par2;
      player_interface.param3=par3;
      cmd_phase0=TRUE;
      return BUSY;
   }

   /*PHASE 1: WAIT*/
   if (player_interface.p_status != BUSY)
   {
      cmd_phase0=FALSE;
      if (player_interface.p_status == ERROR)
      {
	 /*PLAYER INDICATES THAT ERROR HAS OCCURED*/
	 set_error();
	 return ERROR;
      }
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : player_read_q
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command for reading subcode is issued to the player
 *             the result of the command is return to the caller
 *             if the result is READY, interface.param1 contains pointer to a subcode frame
 *
 *
 * Decisions :
*/
static byte player_read_q(void)
{
   return player_command(READ_SUBCODE_OPC,00,0xFF,0);
}

/***********************************************************************************************
 * Function  : player_stop
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to stop is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_stop(void)
{
   return player_command(STOP_OPC,00,00,00);
}

/***********************************************************************************************
 * Function  : player_service_on
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to get into service mode is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_service_on(void)
{
   return player_command(ENTER_SERVICE_MODE_OPC,00,00,00);
}

/***********************************************************************************************
 * Function  : player_service_cmd
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to get into service mode is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_service_cmd(void)
{
   return player_command(service_opc,00,00,00);
}

/***********************************************************************************************
 * Function  : store_service_opc
 * Input     : -
 * Output    : READY
 *
 * Abstract  : copy opcode to service_opc
 *
 *
 * Decisions : buffer is released
*/
static byte store_service_opc(void)
{
   service_opc=GET_BUFFER(1);
   ac();
   return READY;
}


/***********************************************************************************************
 * Function  : player_seek
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to seek to a location is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_seek(void)
{
   if (start_play.min == 0xFF)
      /*NO SEEK NESSECARY*/
      return READY;
   if (start_play.min == 0 && start_play.sec == 0 && start_play.frm  == 0)
      /* READING OF TOC WHISHED
       * BUT NO AUTO JUMP BACK AFTER END OF LEAD IN
      */
      return player_command(READ_TOC_OPC,00,00,00);
   /* START_PLAY <> 00:00:00*/
   return player_command(SEEK_OPC,start_play.min,start_play.sec,start_play.frm);
}

/***********************************************************************************************
 * Function  : player_pause_on
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to pause is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_pause_on(void)
{
   return player_command(PAUSE_ON_OPC,00,00,00);
}

/***********************************************************************************************
 * Function  : player_pause_off
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : a player command to contine playing is issued to the player
 *             the result of the command is returned to the caller
 *
 *
 * Decisions :
*/
static byte player_pause_off(void)
{
   return player_command(PAUSE_OFF_OPC,00,00,00);
}

/***********************************************************************************************
 * Function  : player_jmp_grvs
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : jump 25 grooves in direction depending on serach_fwd | search_bwd
 *
 *
 * Decisions :
*/
static byte player_jmp_grvs(void)
{
    if (search_fwd)
	return player_command(JUMP_TRACKS_OPC,00,25,00);
    else
	return player_command(JUMP_TRACKS_OPC,0xFF,0xE7,00);
}

/*--------------------------------------------------------------------------------
 *
 *			SEND TO HOST COMMANDS
 *
 *--------------------------------------------------------------------------------*/

/***********************************************************************************************
 * Function  : set_progress_status
 * Input     : progress
 * Output    : BUSY, READY
 *
 * Abstract  : As long as the status buffer is occupied, the function returns BUSY.
 *             if the buffer is free, the progress of the player execution and the status is copied
 *             into the buffer ready to send to the host
 *
 *
 * Decisions :
*/
static byte set_progress_status(byte progress)
{
   if (Get_update_status() == NO_UPDATE)
   {
      /*BUFFER IS FREE FOR STORING */
      Store_command(return_cmd);
      Store_cmd_progress(progress);
      Store_drive_status(state);
      Store_update_status(STATUS_UPDATE);
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : set_error_status
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : As long as the status buffer is occupied, the function returns BUSY.
 *             if the buffer is free, the error condition of the player execution and the status is copied
 *             into the buffer ready to send to the host
 *
 *
 * Decisions :
*/
static byte set_error_status()
{
   if (Get_update_status() == NO_UPDATE)
   {
      /*BUFFER IS FREE FORE STORING */
      Store_command(return_cmd);
      Store_error_condition(error_condition);
      Store_drive_status(state);
      Store_update_status(STATUS_UPDATE);
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : set_seek
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status seek to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_seek(void)
{
   return set_progress_status(SEEKING);
}

/***********************************************************************************************
 * Function  : set_open
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send open status seek to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_open(void)
{
   state=OPEN_S;
   if (Get_update_status() == NO_UPDATE)
   {
      /*BUFFER IS FREE FOR STORING */
      Store_command(0x0A);		/*DISK CHANGED*/
      Store_cmd_progress(COMPLETE);
      Store_drive_status(state);
      Store_update_status(STATUS_UPDATE);
      init_for_new_disc();
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : set_closed
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send close status seek to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_closed(void)
{
   if (Get_update_status() == NO_UPDATE)
   {
      /*BUFFER IS FREE FOR STORING */
      Store_command(0x0A);		/*DISK CHANGED*/
      Store_cmd_progress(COMPLETE);
      Store_drive_status(state);
      Store_update_status(STATUS_UPDATE);
      return READY;
   }
   return BUSY;
}


/***********************************************************************************************
 * Function  : set_stopping
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status spinning down to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_stopping(void)
{
   return set_progress_status(SPINNING_DOWN) == READY;
}

/***********************************************************************************************
 * Function  : set_pause
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status pausing to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_pause(void)
{
   state=PAUSE_S;
   return set_progress_status(COMPLETE) == READY;
}

/***********************************************************************************************
 * Function  : set_ready
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status pausing to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_ready(void)
{
   return set_progress_status(COMPLETE) == READY;
}

/***********************************************************************************************
 * Function  : check_area
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status playing the requested sectors to host: done ready else busy
 *
 *
 * Decisions :
*/
static bit check_area(void)
{
   if ( ( ( (byte)(subcode_buffer.conad & 0x40) == 0x40) && rom_data) ||
	( ( (byte)(subcode_buffer.conad & 0x40) != 0x40) && !rom_data) ||
	  ( (byte)(subcode_buffer.tno) == 0)
      )
   {
      return TRUE;
   }
   else
   {
      player_cmd=AREA_ERROR;   /* execute pause command */
      cmd_function_id=0;       /* first function of stop command */
      priority_player_cmd=TRUE;/* accept no commands from host till this
			       /* command has been executed */
      play_monitor=FALSE;
      error_condition=WRONG_TYPE_OF_DATA;
      state=PAUSE_S;
      return FALSE;
   }
}

/***********************************************************************************************
 * Function  : set_play
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status playing the requested sectors to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_play(void)
{
      state=PLAY_S;
      play_monitor=TRUE;
      subcode_read=FALSE;
      return set_progress_status(PLAYING_REQ_SECTORS) == READY;
}

/***********************************************************************************************
 * Function  : set_stopped
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send progress status stopped: done ready else busy
 *
 *
 * Decisions :
*/
static byte set_stopped(void)
{
   state=STOP_S;
   return set_progress_status(COMPLETE) == READY;
}

/***********************************************************************************************
 * Function  : s_disk_error
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send error condition disk_error (not able to read disk) to host done: ready else busy
 *
 *
 * Decisions :
*/
static byte s_disk_error(void)
{
    error_condition=DISK_UNREADABLE;
    return set_error_status() == READY;
}

/***********************************************************************************************
 * Function  : s_cmd_error
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send error condition command_error to host done: ready else busy
 *
 *
 * Decisions :
*/
static byte s_cmd_error(void)
{
    error_condition=BAD_COMMAND;
    return set_error_status() == READY;
}

/***********************************************************************************************
 * Function  : s_id
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send id package to host done: ready else busy
 *
 *
 * Decisions :
*/
static byte s_id()
{
   if (Get_update_status() == NO_UPDATE)
   {
      /*READY TO UPDATE BUFFER*/
      Store_command(0x07);
      Store_q_id(NAME,  'P');
      Store_q_id(NAME+1,'H');
      Store_q_id(NAME+2,'I');
      Store_q_id(NAME+3,'L');
      Store_q_id(NAME+4,'I');
      Store_q_id(NAME+5,'P');
      Store_q_id(NAME+6,'S');
      Store_q_id(NAME+7,' ');
      Store_q_id(NUM,  'C');
      Store_q_id(NUM+1,'D');
      Store_q_id(NUM+2,'M');
      Store_q_id(NUM+3,'1');
      Store_q_id(NUM+4,'2');
      Store_q_id(NUM+5,'N');
      Store_q_id(NUM+6,'2');
      Store_q_id(NUM+7,' ');
      Store_q_id(REV,  '3');
      Store_q_id(REV+1,'1');
      Store_update_status(ID_READY);
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : led_cntrl
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : drive leds
 *
 *
 * Decisions :
*/
static byte led_cntrl(void)
{
   if ( ( (byte)(GET_BUFFER(1) & 0x01) ) == 0x01 )
      led_0_on();
   else
      led_0_off();

   if ( ( (byte)(GET_BUFFER(1) & 0x02) ) == 0x02 )
      led_1_on();
   else
      led_1_off();

   if ( (GET_BUFFER(1) & 0x80) == 0x80 )
   {
      if (Get_update_status() == NO_UPDATE)
      {
	 /*BUFFER IS FREE FOR STORING */
	 Store_command(0x05);		/*DISK CHANGED*/
	 Store_cmd_progress(COMPLETE);
	 Store_drive_status(state);
	 Store_update_status(STATUS_UPDATE);
	 return READY;
      }
      return BUSY;
   }
   return READY;
}

/***********************************************************************************************
 * Function  : resend
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : resend  package to host done: ready else busy
 *
 *
 * Decisions :
*/
static byte resend()
{
   byte	 status;

   if (Get_update_status() == NO_UPDATE)
   {
      status=Get_last_status_update();
      if (status != Q_READY)  
      {
	 Store_update_status(status);
	 return READY;
      }
      /*RESEND OF SUBCODE RETURN ==> BAD COMMAND*/
      non_player_cmd=S_CMD_ERR;
      return BUSY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : send_q
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send  Q package to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte send_q(byte cmd)
{
   if (Get_update_status() == NO_UPDATE && subcode_read)
   {
      Store_command(cmd);
      if (state <= STOP_S)
	 Store_error_condition(BAD_COMMAND);
      Store_q_id(ERR,0);
      Store_q_id(ADRCTL, subcode_buffer.conad);
      Store_q_id(TNO,   subcode_buffer.tno);
      Store_q_id(INDEX, subcode_buffer.index);
      Store_q_id(MIN,   subcode_buffer.r_time.min);
      Store_q_id(SEC,   subcode_buffer.r_time.sec);
      Store_q_id(FRM,   subcode_buffer.r_time.frm);
      Store_q_id(ZERO,  subcode_buffer.zero);
      Store_q_id(AMIN,  subcode_buffer.a_time.min);
      Store_q_id(ASEC,  subcode_buffer.a_time.sec);
      Store_q_id(AFRM,  subcode_buffer.a_time.frm);
      Store_q_id(LEVEL_D,give_peak_level_high() );
      Store_q_id(LEVEL_E,give_peak_level_low() );
      Store_update_status(Q_READY);
      return READY;
   }
   return BUSY;
}

/***********************************************************************************************
 * Function  : send_auto_q
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send  Q package to host done: ready else busy
 *
 *
 * Decisions :
*/
static byte send_auto_q(void)
{
   return send_q(AUTO_Q);
}

/***********************************************************************************************
 * Function  : send_req_q
 * Input     : -
 * Output    : BUSY, READY
 *
 * Abstract  : send Q package to host: done ready else busy
 *
 *
 * Decisions :
*/
static byte send_req_q(void)
{
   return send_q(host_cmd);
}

static byte s_status(void)
{

   if (Get_update_status() == NO_UPDATE)
   {
      Store_command(host_cmd);
      Store_update_status(STATUS_UPDATE);
      return READY;
   }
   return BUSY;
}


/*--------------------------------------------------------------------------------
 *
 *			OTHER COMMANDS
 *
 *--------------------------------------------------------------------------------*/


/***********************************************************************************************
 * Function  : modify_start
 * Input     : -
 * Output    : READY
 *
 * Abstract  : the start parmameters are updated
 *
 *
 * Decisions : 3 possibilities:
 *                    start_play contains new start time in hex: meaning seek to that time
 *                    start_play contains 00:00:00: meaning seek for leadin
 *                    start_play contains FF:FF:FF: meaning no seek nessecary
 *
*/
static byte modify_start(void)
{
   
   start_play.min=GET_BUFFER(SMIN);
   start_play.sec=GET_BUFFER(SSEC);
   start_play.frm=GET_BUFFER(SBLK);

   if (start_play.min != 0xFF)
   {
      playing_in_first_leadin=FALSE;
      bcd_to_hex_time(&start_play,&start_play);
      return READY;
   }
   /*S_MIN, S_SEC, S_FRM = 0XFF*/
   else if (state == STOP_S)
   {
      /*GOTO PLAYING FOR READING TOC IN FIRST LEADIN*/
      start_play.min=00;
      start_play.sec=00;
      start_play.frm=00;
      playing_in_first_leadin=TRUE;

   }
   else
   {
      /*PLAYING: NO CHANGE IN ADDRESS*/
      start_play.min=0xFF;
      start_play.sec=0xFF;
      start_play.frm=0xFF;
   }
   return READY;
}

/***********************************************************************************************
 * Function  : modify_stop
 * Input     : -
 * Output    : READY
 *
 * Abstract  : the stop parmameters are updated
 *
 *
 * Decisions :
*/
static byte modify_stop(void)
{
   stop_play.min=GET_BUFFER(EMIN);
   stop_play.sec=GET_BUFFER(ESEC);
   stop_play.frm=GET_BUFFER(EBLK);
   if (GET_BUFFER(EMIN) == 0xFF)
   {
      /* STOP TIME SHOULD BE LEADOUT*/
      stop_play.min=start_last_leadout.min;
      stop_play.sec=start_last_leadout.sec;
      stop_play.frm=start_last_leadout.frm;
   }
   else
   {
      /*STOP TIME DOES NOT EQUAL LEADOUT*/
      bcd_to_hex_time(&stop_play,&stop_play);
   }
   return READY;
}

/***********************************************************************************************
 * Function  : modify_mode
 * Input     : -
 * Output    : READY
 *
 * Abstract  : the mode connected flags are updated
 *
 *
 * Decisions :
*/
static byte modify_mode(void)
{
   byte	  mode, qmode, peak_level;

   mode=GET_BUFFER(MODE);
   qmode=GET_BUFFER(QMODE);

   double_speed=GET_BUFFER(SPEED) & 0x40;     	/* speed n1 or n2*/
   if ((mode & 0x03) == 0x03)                   /* mute yes / no */
   {
      set_dac_mode(0xF0);                       /* meaning do not release mute*/
      mute_on();
   }
   else
      set_dac_mode(0xF1);

   if ((mode & 0x80) == 0x80)                   /* rom or audio mode */
   {
      rom_data=TRUE;
      set_play_mode(ROM);
   }
   else 
   {
      rom_data=FALSE;
      set_play_mode(AUDIO);
   }
   autoq=!(qmode & 0x04);                  	/* auto report subcode yes or no */

   peak_level = qmode & 0x03;
   set_level_meter_mode(peak_level);

   pause = (byte)(mode & 0x0C) == 0xC0;

   search_fwd=(mode & 0x04);
   search_bwd=(mode & 0x08);

   search_timer=0;				/* If search start with first jump*/

   return READY;
}

/***********************************************************************************************
 * Function  : modify_speed
 * Input     : -
 * Output    : BUSY, READY, ERROR
 *
 * Abstract  : depending on: double_speed (0: n=1; 1: n=2)
 *             the player module is asked to modify the speed
 *             the result of the command is return to the caller
 *
 *
 * Decisions :
*/
static byte modify_speed(void)
{
   if (double_speed  && !speed_is_single) return READY;
   if (!double_speed && speed_is_single)  return READY;

   if (double_speed)
   {
      if (player_command(DOUBLE_SPEED_OPC,00,00,00) == READY)
      {
	 speed_is_single=FALSE;
	 return READY;
      }
      return BUSY;
   }
   else                 
   {
      if (player_command(SINGLE_SPEED_OPC,00,00,00) == READY)
      {
	  speed_is_single=TRUE;
	  return READY;
      }
      return BUSY;
   }
}

/***********************************************************************************************
 * Function  : modify_play
 * Input     : -
 * Output    : READY
 *
 * Abstract  : the play parmameters are updated
 *
 *
*/
static byte modify_play(void)
{
   modify_start();
   modify_stop();
   modify_mode();
   return READY;
}


/***********************************************************************************************
 * Function  : pps
 * Input     : -
 * Output    : READY
 *
 * Abstract  : this function switches between rows in the player_functions_array.
 *             depending on the pause flag, the player is set to play or
 *             pause
 *
 *
 * Decisions :
*/
byte pps(void)
{
    if (pause)
    {
       player_cmd=PLAY_PAUSE;
       cmd_function_id=0;
    }
    else
    {
       /*LINEAIR PLAY OR FFWD / FREV */
       player_cmd=START_PLAY;
       cmd_function_id=2;
    }
    return BUSY;
}
/***********************************************************************************************
 * Function  : pr
 * Input     : -
 * Output    : PROCESS_READY
 *
 * Abstract  : this function indicates that the end of the function array is reached
 *
 *
 * Decisions :
*/
byte pr(void)
{
   return PROCESS_READY;
}



/***********************************************************************************************
 * Function  : copy_subcode
 * Input     : -
 * Output    : READY
 *
 * Abstract  : copies just read subcode into subcode_buffer
 *
 *
 * Decisions :
*/
static byte copy_subcode(void)
{
   union {byte parm; struct subcode_frame *p;}  tmp;

   subcode_read=TRUE;
   tmp.parm=player_interface.param1;
   subcode_buffer.conad=(*tmp.p).conad;
   subcode_buffer.tno=(*tmp.p).tno;
   subcode_buffer.index=(*tmp.p).index;
   subcode_buffer.r_time.min=(*tmp.p).r_time.min;
   subcode_buffer.r_time.sec=(*tmp.p).r_time.sec;
   subcode_buffer.r_time.frm=(*tmp.p).r_time.frm;
   subcode_buffer.zero=(*tmp.p).zero;
   subcode_buffer.a_time.min=(*tmp.p).a_time.min;
   subcode_buffer.a_time.sec=(*tmp.p).a_time.sec;
   subcode_buffer.a_time.frm=(*tmp.p).a_time.frm;
   if (subcode_buffer.index == 0xA2)
   {
      start_last_leadout.min=bcd_to_hex(subcode_buffer.a_time.min);
      start_last_leadout.sec=bcd_to_hex(subcode_buffer.a_time.sec);
      start_last_leadout.frm=bcd_to_hex(subcode_buffer.a_time.frm);
   }
   return READY;
}

/***********************************************************************************************
 * Function  : read_a_time
 * Input     : *struct time
 * Output    : yes / no
 *
 * Abstract  : this function tests if the subcode frame read by the player module is
 *             a mode 1 type if so the hex format of the a_time is copied
 *             into the position indicatedby the pointer and true is returned. In all
 *             other cases false is returned.
 *
 *
 * Decisions : subcode_buffer should hold valid subcode
 *             time is returned (if available) in hex!
*/
static bit read_a_time(struct time *a_tijd)
{
   if ((byte)(subcode_buffer.conad & 0x0F)==0x01)
   {
      /*MODE 1*/
      if (subcode_buffer.tno !=0 )
      {
	 /*NOT IN LEADIN OF A SESSION*/
	 a_tijd->min = bcd_to_hex(subcode_buffer.a_time.min);
	 a_tijd->sec = bcd_to_hex(subcode_buffer.a_time.sec);
	 a_tijd->frm = bcd_to_hex(subcode_buffer.a_time.frm);
	 return TRUE;
      }
      /*TNO == 0*/
      if ( subcode_buffer.r_time.min > 90 || is_cd_disc() )
	  return FALSE;
      a_tijd->min = bcd_to_hex(subcode_buffer.r_time.min);
      a_tijd->sec = bcd_to_hex(subcode_buffer.r_time.sec);
      a_tijd->frm = bcd_to_hex(subcode_buffer.r_time.frm);
      return TRUE;
   }
   return FALSE;
}


/***********************************************************************************************
 * Function  : monitor_borders
 * Input     : -
 * Output    : yes / no
 *
 * Abstract  : this function tests whether the requested sectors have been played
 *
 *
 * Decisions : valid subcode should be present in subcode buffer
*/
static bit monitor_borders()
{
   struct time 		pos;
   byte			abs_time;

   abs_time=read_a_time(&pos);

   if ( 
	( abs_time && (compare_time(&pos,&stop_play) == BIGGER) && !search_bwd)
      ||
	( abs_time && (compare_time(&pos,&stop_play) == SMALLER) && search_bwd)

      )
   {
      player_cmd=PLAY_PAUSE;   /* execute pause command */
      cmd_function_id=0;       /* first function of stop command */
      priority_player_cmd=TRUE;/* accept no commands from host till this
			       /* command has been executed */
      play_monitor=FALSE;
      return TRUE;
   }
   return FALSE;
}
/***********************************************************************************************
 * Function  : prepare_pause_position_change
 * Input     : -
 * Output    : -
 *
 * Abstract  : set varaibles needed to go pausing at 00:02:30
 *
 *
 * Decisions :
*/
void prepare_pause_position_change(void)
{
   start_play.min=00;       /* actual position not usable as pause position*/
   start_play.sec=02;       /* adapt pause position */
   start_play.frm=30;
   player_cmd=SEEK_PAUSE;   /* execute seek command to new pause position*/
   cmd_function_id=0;       /* first function of seek command */
   play_monitor=FALSE;
}

/***********************************************************************************************
 * Function  : decide_pause_position
 * Input     : -
 * Output    : ready busy
 *
 * Abstract  : if in first lead in area execute a seek by selecting seek pause
 *             else go in pause immediatly
 *
 *
 * Decisions : valid subcode should be present subcode_buffer
*/
byte decide_pause_position(void)
{
   if (playing_in_first_leadin)
   {
      /* PLAYER IS PLAYING IN THE FIRST LEAD IN AREA OR JUST OUTSIDE THIS AREA:
       * PROBLEMS PAUSING TO BE EXPECTED: TO CLOSE TO LEADIN AREA
      */
      prepare_pause_position_change();
      return BUSY;
   }
   if ( player_read_q() == READY)
   {
     /*SUBCODE AVAILABLE*/
     copy_subcode();

     if ((byte)(subcode_buffer.conad & 0x0F) != 0x01 )
      /*WAIT FOR MODE 1*/
      return BUSY;

     if ((byte)subcode_buffer.tno == 0)
     {
	/*IN A LEADIN AREA*/
	if (subcode_buffer.r_time.min > 90 || is_cd_disc() )
	{
	   /*FIRST LEADIN AREA*/
	  prepare_pause_position_change();
	  return BUSY;
	}
     }
     /*NOT IN LEADIN AREA*/
     return READY;
   }
   /*WAIT FOR SUBCODE*/
   return BUSY;
}




static byte (* const non_player_functions[10])(void) =

   {resend, s_status, s_cmd_error, s_id, led_cntrl, set_error_status, send_auto_q, send_req_q, s_disk_error, set_closed};


static byte (* const player_functions[][6])(void) =
{
/*START_PAUSE      =0*/  {set_seek, player_seek, player_pause_on, set_pause, pr},
/*START_PLAY       =1*/  {set_seek, player_seek, set_play, pr},
/*MODIFY_PLAY      =2*/  {modify_play, modify_speed, set_ready, pr},
/*STOP_C           =3*/  {set_stopping, player_stop, set_stopped, pr},
/*PLAY_PAUSE       =4*/  {set_pause, decide_pause_position, player_pause_on, pr},
/*NEW_PLAY         =5*/  {player_pause_off, modify_play, modify_speed, set_seek, player_seek, pps},
/*SEEK_PLAY        =6*/  {modify_start, player_pause_off, set_seek, player_seek, set_play, pr},
/*SEEK_STOP	   =7*/  {modify_start, set_ready, pr},
/*PAUSE_PLAY	   =8*/  {player_pause_off, set_play, pr},
/*ENT_DIA	   =9*/  {player_service_on, set_ready, pr},
/*DIA		   =10*/ {store_service_opc, player_service_cmd, set_ready, pr},

/*INTERNAL COMMANDS*/
/*SEEK_PAUSE	   =11*/ {player_seek, player_pause_on, set_pause,  pr},
/*OPEN_C           =12*/ {set_error_status, set_open, player_stop, pr},
/*AREA_ERROR       =13*/ {player_pause_on, set_error_status, pr},

};

/***********************************************************************************************
 * Function  : command_handler
 * Input     : -
 * Output    : -
 *
 * Abstract  : execute one cycle of the command handler
 *
 * Decisions :
*/
void command_handler(void)
{
   byte  status;

   if (non_player_cmd == IDLE)
   {
      /* NOT BUSY SENDING TO HOST
       * FIRST CHECK STATE OPEN: DOOR SWITCH
      */
      if ( state == OPEN_S)
      {
	 if (door_closed() && player_cmd != OPEN_C)
	 {
	    /* DOOR IS CLOSED
	     * SEND HOST DISK CHANGE PACKET
	    */
	    state=STOP_S;
	    non_player_cmd=S_CLOSED;
	 }
      }
      else
      {
	 /*STATE <> OPEN_S CHECK DOOR*/
	 if ( !door_closed() && !(player_cmd == OPEN_C ))
	 {
	    /* DOOR HAS BEEN OPENED
	     * ISSUE A PRIORITY STOP COMMAND TO THE PLAYER
	    */
	    priority_player_cmd=TRUE;	/* accept no commands from host till this */
	    if ( (state >= STOP_S && player_cmd != IDLE) || play_monitor || drive_is_pausing() )
	    {
	       /*COMMAND IN PROGRESS OR PLAYING OR PAUSING*/
	       state=OPEN_S;
	       error_condition=DRIVE_ERROR;
	       player_cmd=OPEN_C;          /* execute stop command */
	       cmd_function_id=0;          /* first function of stop command */
	    }
	    else
	    {
	       cmd_function_id=1;          /* only second function of stop command */
	       player_cmd=OPEN_C;          /* execute stop command */
	    }
	    play_monitor=FALSE;	 	/* leave play monitor*/
	 }
      }

      if (play_monitor)
      {

	 /* NOW WE ARE PLAYING SECTORS
	  * READ Q TO
	  * CHECK IF BEYOND STOP ADDRESS: IF SO SEND DONE AND
	  *                                     ISSUE A PAUSE COMMAND
	  * IF AUTO_Q SEND SUBCODE
	  * IF SEARCH ACTIVE: JUMP 25 GROOVES EVERY 80 MS
	 */
	 if (search_timer == 0 && (search_bwd || search_fwd) && subcode_read )
	 {
	    /*CHECK IF JUMP HAS TO BE MADE*/
	    attenuate_on();
	    if (player_jmp_grvs() == READY)
	       search_timer = MS_80;
	 }
	 else
	 {
	    status = player_read_q();
	    if (status == READY)
	    {
	       /*NEW SUBCODE READ*/
	       copy_subcode();
	       if (!monitor_borders() )
	       {
		  /*CHECK IF TRACKING IN RIGHT AREA*/
		  if ( check_area() )
		  {
		     /*AREA OK :RELEASE MUTE IF ALLOWED*/
		     if ( !(search_bwd  || search_fwd) )
			/*NO SEARCH ACTIVE*/
			mute_off();

		    /*NEW SUBCODE READ WITHOUT ERROR SO CHECK IF HOST HAS TO BE NOTIFIED*/
		     if (autoq)
		     {
			/*SEND EVERY SUBCODE FRAME TO HOST*/
			non_player_cmd=SEND_AUTO_Q;
		     }
		  }
	       }
	    }
	 }
      }
   }

   if (read_new_command())
   {
      /* NEW COMMAND IN BUFFER
       * READ COMMAND
      */
      host_cmd=GET_BUFFER(0);


      /* CONVERT HOST COMMAND TO INTERNAL COMMAND
	 DEPENDING ON ACTUAL STATE (= OPEN_S, STOP_S, PLAY_S, PAUSE_S )
      */
      int_cmd=convert_to_int_cmd(host_cmd & 0x0F);

      if ( (int_cmd != SEND_Q) && (int_cmd != LED_CNTRL) && int_cmd != S_ID)
	  /* A COMMAND THAT RETURNS PROGRESSION*/
	  return_cmd=host_cmd;

      /* CHECK IF ONLY INFORMATION HAS TO BE SENT TO HOST
	 IN THAT CASE MOST SIGNIFICANT BIT OF INT_CMD IS SET
      */
      if (non_player_command(int_cmd))
      {
	  non_player_cmd=(int_cmd & 0x7F);
      }
      else
      {
	  /* NEW COMMAND THAT NEEDS PLAYER ACTIONS
	   * SELECT FUNCTION FROM FUNCTIONS ARRAY
	  */
	  player_cmd=int_cmd;
	  cmd_function_id=0; 	/* first function of stop command */
	  play_monitor=FALSE;   /* leave play monitor*/
      }
   }

   if (non_player_cmd != IDLE)
   {
      /* BUSY WITH NON PLAYER COMMAND*/
      if ( (*non_player_functions[ (non_player_cmd & 0x7F) ])() == READY )
      {
	  non_player_cmd=IDLE;		/*non player command = idle */
	  if (!cmd_received) ac();
      }
   }

   else if (player_cmd != IDLE)
   {
      /*
       * BUSY EXECUTING A COMMAND HANDLER PROCESS
      */
      switch ((*player_functions[player_cmd][cmd_function_id])())
      {
	 case PROCESS_READY:  player_cmd=IDLE;		/* no command in execution */
			      cmd_function_id=0;
			      priority_player_cmd=FALSE;
			      if (!cmd_received) ac();
			      break;

	 case READY:          cmd_function_id++;
			      break;

	 case ERROR:          break;
      }
   }
}
/*			INTERFACE OPERATIONS           */
/***********************************************************************************************
 * Function  : Cmd_acception_status
 * Input     : -
 * Output    : yes / no
 *
 * Abstract  : if command handler accepts a new command, true is returned else false
 *
 *
 * Decisions :
*/
byte Cmd_acception_status(void)
{
   return accept_cmd;
}

/***********************************************************************************************
 * Function  : New_command
 * Input     : -
 * Output    : yes / no
 *
 * Abstract  : if command handler accepts a new command, true is returned else false
 *
 *
 * Decisions :
*/

byte New_command()
{
   if (accept_cmd)
   {
      /* COMMAND HANDLER ACCEPTS NEW COMMAND
      */
      cmd_received=TRUE;			/* new command is received */
      accept_cmd=FALSE;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Init_command_handler
 * Input     : -
 * Output    : -
 *
 * Abstract  : decide initial state (=OPEN_S || STOP_S;  set all variables
 *
 *
 * Decisions :
*/
void Init_command_handler(void)
{
   /*INITIALIZE STATE TO OPEN_S OR STOP_S DEPENDING ON DOOR */
   if (door_closed())
   {
      /*DOOR CLOSED*/
      Store_status(STOPPED_CLOSED);
      state=STOP_S;
   }
   else
   {
      /*DOOR OPEN*/
      Store_status(STOPPED_OPEN);
      state=OPEN_S;
   }
   player_cmd=IDLE;        		/* no command for player module*/
   cmd_function_id=0;                   /* init- */
   cmd_phase0=0;                        /*       player function - */
   priority_player_cmd=FALSE;           /* player_cmd does not contain a high priority cmd */
   non_player_cmd=IDLE;                 /* send status to host */

   /* DEFAULT PLAY LEADIN*/
   start_play.min=0; start_play.sec=0; start_play.frm=0;
   stop_play.min=0; stop_play.sec=0; stop_play.frm=0;

   host_cmd=NO_CMD;			/* not yet a host command received*/
   return_cmd=NO_CMD;
   accept_cmd=TRUE;			/* command handler ready to accept a new command */
   cmd_received=FALSE;			/* not a command received*/
   double_speed=FALSE;                  /* start om single speed */
   speed_is_single=TRUE;
   autoq=TRUE;				/* auto send of subcode */
   led_0_off();				/* switch access led off */
   led_1_off();				/* switch auxillary led off */


}

