/***************************************************************************
*
* Project:	Commodore
* File:		play.c
* Date:		19 July 1993
* Version:	0.1
* Status:	Draft
*
* Description:	this module has the following tasks:
*               - control mute on/off
*               - control volume up/down
*               - jump to a requested address
*               - jump to the start of a requested track
*               - jump to the requested lead-in
*               - monitor pause mode
*               - monitor toc reading mode (keep tracking in the lead-in)
*               - monitor if requested subcode is stored in buffer
*               - prepare/restore during speed change
*
* Interface functions: void set_dac_mode(byte dac_mode)
*		       byte jump_time(struct time *pt)
*                      byte play(byte play_cmd)
*                      void execute_play_functions(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 11 December 1992	creation H.v.K.
* 19 July 1993		adapted for commodore
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

#define	IDLE_MODE			0x00  /* list of play modes */
#define	PAUSE_MODE			0x01
#define	TRACKING_MODE			0x02
#define	TOC_MODE			0x03
#define	RETURN_TO_TARGET		0x08


extern void cd6_wr(byte);
extern bit  zero_scor_counter(void);
extern void init_scor_counter(byte);
extern byte shock_detector_off(void);
extern byte shock_detector_on(void);
extern void servo_jump(int);
extern byte get_servo_process_state(void);
extern byte get_jump_status(void);
extern bit  servo_tracking(void);
extern void stop_subcode_reading(void);
extern void start_subcode_reading(void);
extern bit  is_subcode(byte);
extern int  calc_tracks(struct time *, struct time *);
extern byte compare_time(struct time *, struct time *);
extern void add_time(struct time *, struct time *, struct time *);
extern void subtract_time(struct time *, struct time *, struct time *);
extern byte hex_to_bcd(byte);
extern void hex_to_bcd_time(struct time *, struct time *);
extern void bcd_to_hex_time(struct time *, struct time *);
extern void move_abstime(struct time *);

extern struct subcode_frame	Q_buffer;
extern struct interface_field	player_interface;
extern byte			play_timer; /* timer tick is 8 msec */
extern byte			player_error;

static byte			play_process; /* high nibble is play_process_id
						 low nibble is play_function_id */
static byte			play_status; /* high nibble is BUSY=0x00, READY=0x10, ERROR=0x20
						low nibble is play_mode:
							      IDLE_MODE      0x00
							      PAUSE_MODE     0x01
							      TRACKING_MODE  0x02
							      TOC_MODE       0x03
						during speed change and return to target
						has to take place bit3 is set */
static byte			volume_level = 0x80; /* copy of volume level (to be) stored in CD-6 */
static byte			jumps_counter;
static bit			play_phase1,play_phase0;
static bit			jump_phase1,jump_phase0;
static bit			reload_play_timer1,reload_play_timer0;
static bit			track_found; /* set when during a binary search the requested track is found */
static bit			mute_on_after_search; /* if set then after search mute is still on,
							 if reset then after search mute is switched off */
bit				play_monitor; /* set after a command is executed with or without error */
bit				play_command_busy; /* set when a command is in execution,
						      cleared after ready or error is reported */

extern union {
	     struct {
		    struct subcode_frame	subframe;
		    byte		     	left;
		    byte		     	right;
		    } play_subcode;
	     struct {
		    struct time 		low_time;
		    struct time 		high_time;
		    struct time 		target_time;
		    struct time 		tmp_time;
		    } play_times;
	     } store;

/**************************************************************************/
bit drive_is_pausing(void)
{
   return (play_status & PAUSE_MODE) == PAUSE_MODE;
}


/**************************************************************************/
void set_dac_mode(byte dac_mode)
{
   if (dac_mode == 0xF0) mute_on_after_search = TRUE;
   else if (dac_mode == 0xF1) mute_on_after_search = FALSE;
   else cd6_wr(dac_mode);
}

/**************************************************************************/
void set_play_mode(byte mode)
{
   cd6_wr(mode);
}

/**************************************************************************/
void attenuate_on(void)
{
   cd6_wr(ATTENUATE);
}


/**************************************************************************/
static byte set_pause_mode(void)
{
   play_status = play_status & 0xF0 | PAUSE_MODE;
   return PROCESS_READY;
}


/**************************************************************************/
static byte set_tracking_mode(void)
{
   play_status = play_status & 0xF0 | TRACKING_MODE;
   return PROCESS_READY;
}


/**************************************************************************/
static byte set_toc_mode(void)
{
   if (player_interface.param2 == 0)
      play_status = play_status & 0xF0 | TOC_MODE; /* param2=0 -> toc_mode */
   else
      play_status = play_status & 0xF0 | IDLE_MODE; /* param2<>0 -> idle_mode */
   return PROCESS_READY;
}


/**************************************************************************/
static void set_subcode_buffer(void)
{
   /* copy Q_buffer to store.play_subcode.subframe (byte for byte to gain speed) */
   store.play_subcode.subframe.conad = Q_buffer.conad;
   store.play_subcode.subframe.tno = Q_buffer.tno;
   store.play_subcode.subframe.index = Q_buffer.index;
   store.play_subcode.subframe.r_time.min = Q_buffer.r_time.min;
   store.play_subcode.subframe.r_time.sec = Q_buffer.r_time.sec;
   store.play_subcode.subframe.r_time.frm = Q_buffer.r_time.frm;
   store.play_subcode.subframe.zero = Q_buffer.zero;
   store.play_subcode.subframe.a_time.min = Q_buffer.a_time.min;
   store.play_subcode.subframe.a_time.sec = Q_buffer.a_time.sec;
   store.play_subcode.subframe.a_time.frm = Q_buffer.a_time.frm;

   /* check which hex values have to be converted to bcd values */
   if ((byte)(store.play_subcode.subframe.conad & 0x0F) == 0x01)
   {  /* address = 1 */
      if (store.play_subcode.subframe.tno != 0xAA)
	 store.play_subcode.subframe.tno = hex_to_bcd(store.play_subcode.subframe.tno);
      hex_to_bcd_time(&store.play_subcode.subframe.r_time,&store.play_subcode.subframe.r_time);
      if (store.play_subcode.subframe.index==0xA0 || store.play_subcode.subframe.index==0xA1)
	 store.play_subcode.subframe.a_time.min = hex_to_bcd(store.play_subcode.subframe.a_time.min);
      else
      {
	 hex_to_bcd_time(&store.play_subcode.subframe.a_time,&store.play_subcode.subframe.a_time);
	 if (store.play_subcode.subframe.index != 0xA2)
	    store.play_subcode.subframe.index = hex_to_bcd(store.play_subcode.subframe.index);
      }
   }
   else if ((byte)(store.play_subcode.subframe.conad & 0x0F) == 0x05)
   {  /* address = 5 */
      if (store.play_subcode.subframe.tno == 0 &&
	  store.play_subcode.subframe.index == 0xB0 &&
	  store.play_subcode.subframe.r_time.min != 0xFF)
	 hex_to_bcd_time(&store.play_subcode.subframe.r_time,&store.play_subcode.subframe.r_time);
   }
}


/**************************************************************************/
static bit req_subc_stored_subc(void)
{
   if (player_interface.param1 == 0) return TRUE;
   if ((byte)(store.play_subcode.subframe.conad & 0x0F) == player_interface.param1)
   {
      if (store.play_subcode.subframe.index==player_interface.param2 || player_interface.param2==0xFF)
	 return TRUE;
   }
   return FALSE;
}


/**************************************************************************/
byte mute_on(void)
{
   cd6_wr(MUTE);
   return READY;
}


/**************************************************************************/
byte mute_off(void)
{
   if (!mute_on_after_search)
   {
      cd6_wr(FULL_SCALE);
   }
   return READY;
}


/**************************************************************************/
byte jump_time(struct time *pt)
{
   int	 	nr_of_grooves;
   struct time  hulp;

   /* phase = 0 ? */
   if (!jump_phase0 && !jump_phase1)
   {
      start_subcode_reading();
      play_timer = SUBCODE_TIMEOUT_VALUE;
      jumps_counter = 0;
      jump_phase0 = 1; /* set phase 1 */
   }

   /* phase = 1 ? */
   if (jump_phase0 && !jump_phase1)
   {
      if (is_subcode(ABS_TIME))
      {
	 move_abstime(&store.play_times.tmp_time);
	 nr_of_grooves = calc_tracks(&store.play_times.tmp_time,pt);
	 if (nr_of_grooves == 0)
	 {
	    jump_phase1 = 1; /* set phase 1 */
	    subtract_time(pt,&store.play_times.tmp_time,&hulp);
	    init_scor_counter(hulp.frm);
	 }
	 else
	 {
	    servo_jump(nr_of_grooves);
	    jump_phase1 = 1; jump_phase0 = 0; /* set phase 2 */
	 }
      }
      else if (is_subcode(FIRST_LEADIN_AREA))
      {
	 servo_jump(TRACKS_OUTOF_LEADIN); /* jump out of leadin */
	 jump_phase1 = 1; jump_phase0 = 0; /* set phase 2 */
      }
      else if (play_timer == 0)
      {
	 player_error = SUBCODE_TIMEOUT_ERROR;
	 jump_phase0 = 0; /* set phase 0 */
	 return ERROR;
      }
   }

   /* phase = 2 ? */
   if (!jump_phase0 && jump_phase1)
   {
      if ((byte)get_servo_process_state() == READY)
      {
	 if (get_jump_status() & NO_HF_ON_TARGET)
	 {
	    player_error = JUMP_ERROR;
	    jump_phase1 = 0; /* set phase 0 */
	    return ERROR;
	 }
	 jumps_counter++;
	 if (jumps_counter > 20)
	 {  /* more than 20 retries needed to get on target address */
	    player_error = JUMP_ERROR;
	    jump_phase1 = 0; /* set phase 0 */
	    return ERROR;
	 }
	 jump_phase1 = 0; /* set phase 0 */
      }
      else if ((byte)get_servo_process_state() == ERROR)
      {
	 jump_phase1 = 0; /* set phase 0 */
	 return ERROR;
      }
   }

   /* phase = 3 ? */
   if (jump_phase0 && jump_phase1)
   {
      if (zero_scor_counter() )
      {
	jump_phase0=0; jump_phase1=1;
	return READY;
      }
   }
   return BUSY;
}




/**************************************************************************/
byte play(byte play_cmd)
{
   if (play_process>>4 != play_cmd || !play_command_busy)
   {
      play_process = play_cmd<<4; /* also sets play_function_id = 0 */
      play_phase1 = 0; play_phase0 = 0;
      play_command_busy = TRUE; play_monitor = FALSE;
      if (play_cmd == PLAY_RESTORE_SPEED_CHANGE)
	 play_status &= 0x0F;
      else
	 play_status &= 0x07; /* clear also flag: return_to_target */

      switch (play_cmd)
      {
      case PAUSE_ON:
	 if (!servo_tracking() || play_status==TOC_MODE)
	 {
	    if (player_error == NO_ERROR) player_error = ILLEGAL_COMMAND;
	    play_status |= ERROR<<4;
	    play_monitor = TRUE;
	 }
	 else if (play_status==PAUSE_MODE)
	 {
	    play_status |= READY<<4;
	    play_monitor = TRUE;
	 }
	 break;
      case PAUSE_OFF:
	 if (!servo_tracking() || play_status!=PAUSE_MODE)
	 {
	    play_monitor = TRUE;
	    if (servo_tracking() || play_status==TRACKING_MODE)
	       play_status |= READY <<4;
	    else
	    {
	       if (player_error == NO_ERROR) player_error = ILLEGAL_COMMAND;
		  play_status |= ERROR<<4;
	    }
	 }
	 break;
      case PLAY_READ_SUBCODE:
	 if (!servo_tracking())
	 {
	    if (player_error == NO_ERROR) player_error = ILLEGAL_COMMAND;
	    play_status |= ERROR<<4;
	    play_monitor = TRUE;
	 }
	 break;
      case PLAY_SET_VOLUME:
      case PLAY_PREPARE_SPEED_CHANGE:
	 if (player_error != NO_ERROR)
	 {
	    play_status |= ERROR<<4;
	    play_monitor = TRUE;
	 }
	 break;
      case PLAY_JUMP_TRACKS:
	 if (!servo_tracking() || play_status==TOC_MODE || play_status==PAUSE_MODE)
	 {
	    if (player_error == NO_ERROR) player_error = ILLEGAL_COMMAND;
	    play_status |= ERROR<<4;
	    play_monitor = TRUE;
	 }
	 break;
      }
      if (!play_monitor) play_status |= BUSY<<4;
   }
   
   if (play_status>>4 != BUSY) play_command_busy = FALSE;
   return play_status>>4;
}


/**************************************************************************/
static byte playing_idle(void)
{
   mute_on();
   shock_detector_off();
   stop_subcode_reading();
   jumps_counter = 0;
   play_status = play_status & 0xF0 | IDLE_MODE; /* set_idle_mode */
   return PROCESS_READY;
}


/**************************************************************************/
static byte pause_on_init(void)
{
   if (!play_phase0)
   {
      start_subcode_reading();
      play_timer = SUBCODE_TIMEOUT_VALUE;
      play_phase0 = 1;
   }

   if (is_subcode(ABS_TIME))
   {
      /* store pause address in store.play_subcode.subframe (in bcd!!) */
      set_subcode_buffer();
      return READY;
   }
   else if (play_timer == 0)
   {
      player_error = SUBCODE_TIMEOUT_ERROR;
      return ERROR;
   }
   return BUSY;
}

/**************************************************************************/
static byte search_for_address(void)
{
   return jump_time(&store.play_times.target_time);
}

/**************************************************************************/
/*NOTE: bit "track_found" is used as a flag to check whether it is possible
	to read any subcode. if not possible to read any subcode within 2 sec
	then player_error=SUBCODE_TIMEOUT_ERROR is given. */
static byte monitor_subcodes(void)
{
   /* phase = 0 ? */
   if (!play_phase1 && !play_phase0)
   {
      if ((byte)(play_status & 0x0F) == PAUSE_MODE)
      {
	 if (req_subc_stored_subc())
	 {
	    player_interface.param1 = (byte)&store.play_subcode.subframe;
	    return PROCESS_READY;
	 }
	 player_error = ILLEGAL_PARAMETER;
	 return ERROR;
      }
      start_subcode_reading();
      play_timer = 255; reload_play_timer0 = 1; reload_play_timer1 = 1; /* timeout value = 8 sec */
      track_found = FALSE; /* clear flag: any subcode is found */
      play_phase0 = 1; /* set phase 1 */
   }

   /* phase = 1 ? */
   if (!play_phase1 && play_phase0)
   {
      if (play_timer == 0)
      {
	 if (!track_found)
	 {  /* can't read subcode at all !! */
	    player_error = SUBCODE_TIMEOUT_ERROR;
	    return ERROR;
	 }
	 track_found = FALSE; /* clear flag: any subcode is found */
	 play_timer = 255; /* re-init to 2 sec */
	 if (reload_play_timer1)
	 {
	    if (reload_play_timer0) reload_play_timer0 = 0;
	    else { reload_play_timer1 = 0; reload_play_timer0 = 1; }
	 }
	 else
	 {
	    if (reload_play_timer0) reload_play_timer0 = 0;
	    else { player_error = SUBCODE_NOT_FOUND; return ERROR; }
	 }
      }
      if (is_subcode(ALL_SUBCODES))
      {  /* new subcode stored in Q_buffer */
	 track_found = TRUE; /* set flag: any subcode is found */
	 if ((byte)(play_status & 0x0F)==TOC_MODE && is_subcode(PROGRAM_AREA))
	 {  /* run out of lead-in !! */
	    servo_jump(TRACKS_INTO_LEADIN); /* jump X tracks back to get into the leadin */
	    play_phase1 = 1; play_phase0 = 0; /* set phase 2 */
	 }
	 else
	 {
	    /* convert hex Q_buffer to bcd store.play_subcode.subframe */
	    set_subcode_buffer();
	    if (req_subc_stored_subc())
	    {
	       player_interface.param1 = (byte)&store.play_subcode.subframe;
	       return PROCESS_READY;
	    }
	    start_subcode_reading(); /* do new request */
	 }
      }
   }

   /* phase = 2 ? */
   if (play_phase1 && !play_phase0)
   {
      if ((byte)get_servo_process_state() == READY)
      {
	 start_subcode_reading();
	 play_phase1 = 0; play_phase0 = 1; /* set phase 1 */
      }
      else return get_servo_process_state();
   }
   return BUSY;
}


/**************************************************************************/
static byte prepare_before_speed_change(void)
{
byte	exec_status;

   /* phase = 0 ? */
   if (!play_phase1 && !play_phase0)
   {
      if ((byte)(play_status & 0x0F) != TRACKING_MODE) return READY;
      mute_on();
      play_phase0 = 1; /* set phase 1 */
   }

   /* phase = 1 ? */
   if (!play_phase1 && play_phase0)
   {
      exec_status = shock_detector_off();
      if (exec_status == READY)
      {
	 start_subcode_reading();
	 play_timer = SUBCODE_TIMEOUT_VALUE;
	 play_phase1 = 1; play_phase0 = 0; /* set phase 2 */
      }
      else return exec_status;
   }

   /* phase = 2 ? */
   if (play_phase1 && !play_phase0)
   {
      if (is_subcode(LEADIN_AREA)) return READY;
      else if (is_subcode(ABS_TIME))
      {
	 /* store address for returning to after speed change */
	 move_abstime(&store.play_times.target_time);
	 play_status |= RETURN_TO_TARGET;
	 return READY;
      }
      else if (play_timer == 0)
      {
	 player_error = SUBCODE_TIMEOUT_ERROR;
	 return ERROR;
      }
   }
   return BUSY;
}


/**************************************************************************/
static byte restore_after_speed_change(void)
{
byte	exec_status;

   /* phase = 0 ? */
   if (!play_phase1 && !play_phase0)
   {
      if ((byte)(play_status & 0x07) == TRACKING_MODE)
      {
	 if (play_status & RETURN_TO_TARGET)
	 {  /* jump back to target address */
	    play_status &= ~RETURN_TO_TARGET;
	    play_phase0 = 1; /* set phase 1 */
	 }
	 else
	 {
	    mute_off();
	    play_phase1 = 1; /* set phase 2 */
	 }
      }
      else return READY;
   }

   /* phase = 1 ? */
   if (!play_phase1 && play_phase0)
   {
      exec_status = jump_time(&store.play_times.target_time);
      if (exec_status == READY)
      {
	 mute_off();
	 play_phase1 = 1; play_phase0 = 0; /* set phase 2 */
      }
      else return exec_status;
   }

   /* phase = 2 ? */
   if (play_phase1 && !play_phase0)
   {
      return shock_detector_on();
   }
   return BUSY;
}

/**************************************************************************/
static byte restore_target_address(void)
{
   if (!play_phase0)
   {
      start_subcode_reading();
      play_timer = SUBCODE_TIMEOUT_VALUE;
      play_phase0 = 1;
   }

   if (is_subcode(ABS_TIME))
   {
      /* store pause address in store.play_subcode.subframe (in bcd!!) */
      move_abstime(&store.play_times.target_time);
      return READY;
   }
   else if (play_timer == 0)
   {
      player_error = SUBCODE_TIMEOUT_ERROR;
      return ERROR;
   }
   return BUSY;
}


/**************************************************************************/
static void monitor_pausing(void)
{
   if ((byte)get_servo_process_state() == READY)
   {
      if (is_subcode(ABS_TIME))
      {
	 /* copy r_time to a_time if pausing in the leadin (not possible in the first leadin) */
	 if (Q_buffer.tno == 0)
	 {
	    Q_buffer.a_time.min = Q_buffer.r_time.min;
	    Q_buffer.a_time.sec = Q_buffer.r_time.sec;
	    Q_buffer.a_time.frm = Q_buffer.r_time.frm;
	 }
	 /* first check which subcode is stored and then use Q_buffer.r_time as temp time for compare */
	 if (store.play_subcode.subframe.tno == 0)
	    bcd_to_hex_time(&store.play_subcode.subframe.r_time,&Q_buffer.r_time);
	 else
	    bcd_to_hex_time(&store.play_subcode.subframe.a_time,&Q_buffer.r_time);
	 if ((byte)compare_time(&Q_buffer.a_time,&Q_buffer.r_time) == BIGGER)
	 {  /* current time bigger than pause time */
	    servo_jump(calc_tracks(&Q_buffer.a_time,&Q_buffer.r_time));
	 }
	 else
	 {  /* current time smaller than or equal to pause time -> check if within window */
	    subtract_time(&Q_buffer.r_time,&Q_buffer.a_time,&Q_buffer.r_time);
	    if (Q_buffer.r_time.min!=0 || Q_buffer.r_time.sec!=0)
	    {  /* more than 1 second from pause address, so jump forward */
	       add_time(&Q_buffer.r_time,&Q_buffer.a_time,&Q_buffer.r_time);
	       servo_jump(calc_tracks(&Q_buffer.a_time,&Q_buffer.r_time));
	    }
	 }
	 start_subcode_reading();
      }
      else if (is_subcode(FIRST_LEADIN_AREA))
      {  /* gone into the leadin (because of shock?) -> jump out of leadin */
	 servo_jump(100); /* jump out of leadin */
      }
   }
}


/**************************************************************************/
static void monitor_toc_reading(void)
{
   if ((byte)get_servo_process_state() == READY)
   {
      if (is_subcode(PROGRAM_AREA))
      {
	 servo_jump(TRACKS_INTO_LEADIN); /* jump X tracks back to get into the leadin */
      }
   }
}


/**************************************************************************/
static byte jump_tracks(void)
{
#define MAX_TRACKS_ATTENUATE	100 /* mute when more than 100 tracks to jump, otherwise attenuate */
int_hl	nr_of_tracks;

   nr_of_tracks.byte_hl.high = player_interface.param1;
   nr_of_tracks.byte_hl.low = player_interface.param2;
   if (!play_phase0)
   {
      servo_jump(nr_of_tracks.int_i);
      play_phase0 = 1; /* set phase 1 */
      return BUSY;
   }
   else
      return get_servo_process_state();
}


/**************************************************************************/
static byte jump_to_toc(void)
{
byte	exec_status;

   if (!play_phase0)
   {
      store.play_times.target_time.min = 0;
      store.play_times.target_time.sec = 2;
      store.play_times.target_time.frm = 0;
      exec_status = jump_time(&store.play_times.target_time);
      if (exec_status == READY)
      {
	 servo_jump(TRACKS_INTO_LEADIN); /* jump 588 tracks back to get into the leadin */
	 play_phase0 = 1;
      }
      else return exec_status;
   }
   else
   {
      return get_servo_process_state();
   }
   return BUSY;
}

/**************************************************************************/
static byte copy_parameters(void)
{
   store.play_times.target_time.min = player_interface.param1;
   store.play_times.target_time.sec = player_interface.param2;
   store.play_times.target_time.frm = player_interface.param3;
   return READY;
}

/**************************************************************************/
static byte play_process_ready(void)
{
   return PROCESS_READY;
}


/**************************************************************************/
static byte (* const play_processes[][5])(void) =
{
/*PLAY_IDLE                =00*/ {playing_idle},
/*PLAY_STARTUP             =01*/ {play_process_ready},
/*PAUSE_ON                 =02*/ {mute_on,shock_detector_off,pause_on_init,set_pause_mode},
/*PAUSE_OFF                =03*/ {restore_target_address,shock_detector_on,set_tracking_mode},
/*JUMP_TO_ADDRESS          =04*/ {copy_parameters, search_for_address,shock_detector_on,set_tracking_mode},
/*PLAY_TRACK               =05*/ {play_process_ready},
/*PLAY_READ_SUBCODE        =06*/ {monitor_subcodes},
/*PLAY_READ_TOC            =07*/ {jump_to_toc,set_toc_mode, play_process_ready},
/*PLAY_PREPARE_SPEED_CHANGE=08*/ {prepare_before_speed_change,play_process_ready},
/*PLAY_RESTORE_SPEED_CHANGE=09*/ {restore_after_speed_change,play_process_ready},
/*PLAY_SET_VOLUME          =10*/ {play_process_ready},
/*PLAY_JUMP_TRACKS         =11*/ {shock_detector_off,jump_tracks,restore_target_address,shock_detector_on,play_process_ready}
};


/**************************************************************************/
void execute_play_functions(void)
{
   if (!play_monitor)
   {
      switch ((*play_processes[play_process>>4][play_process & 0x0F])())
      {
      case PROCESS_READY: play_status = play_status & 0x0F | READY<<4;
			  play_monitor = TRUE;
			  break;
      case READY:         play_process++; /* play_function_id = play_function_id + 1 */
			  play_phase0 = 0;
			  play_phase1 = 0;
			  break;
      case ERROR:	  play_status = play_status & 0x0F | ERROR<<4;
			  play_monitor = TRUE;
			  break;
      }
   }

   if (play_monitor)
   {
      switch (play_status)
      {
      case (READY<<4 | PAUSE_MODE):    monitor_pausing();
				       break;

      case (READY<<4 | TOC_MODE):      monitor_toc_reading();
				       break;

      case (READY<<4 | TRACKING_MODE): break;

      case (READY<<4 | IDLE_MODE):     break;
      }
   }
}
