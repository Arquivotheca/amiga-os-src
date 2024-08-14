/***************************************************************************
*
* Project:	Commodore
* File:		strtstop.c
* Date:		20 July 1993
* Status:	Draft
*
* Description:	this module starts the spin up/spin down process of the spindle motor
*               it also contains some interface functions for getting disc information
*
* Interface functions: struct time * get_leadout_time(void)
*                      void init_for_new_disc(void)
*                      byte start_stop(byte cmd)
*                      void execute_start_stop_functions(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 09 December 1992	creation H.v.K.
* 20 JULY 1993		adapted for Commodore
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

extern void servo_start(void);
extern void servo_stop(void);
extern void servo_n1(void);
extern void servo_n2(void);
extern void servo_jump(int);
extern byte jump_time(struct time *);
extern byte get_servo_process_state(void);
extern void stop_subcode_reading(void);
extern void start_subcode_reading(void);
extern bit is_subcode(byte);

extern struct subcode_frame	Q_buffer;
extern bit			disc_size_known;
extern byte			play_timer;
extern byte			player_error;
extern union {
	     struct {
		    byte		status1;
		    byte		status2;
			byte		counter;
			byte		last_read_tno;
		    byte		first_mode5_pointer;
		    struct time		tmp_time;
		    struct time		start_next_leadin_area;
		    } toc_info;
	     struct {
		    int			tracks;
		    struct time		time1;
		    struct time		time2;
		    struct time		time3; /* time3.min = retry counter !!! */
		    } scan_info;
		 struct {
		    byte		samples;
		    unsigned short	offtrack_1;
		    unsigned short	offtrack_2;
		    struct time		tmp_time;
		    byte		counter; /* retry counter for disc parameters and sledge calibration */
		    unsigned short	ref_value;
		    } sledge_cal_info; /* should be the same format as in servo.c !!! */
		 } store; /* union in another way defined as in play module !! */



static byte			start_stop_process; /* STOP=0, START_UP=1, SPEED_N1=2, SPEED_N2=3 */
static byte			start_stop_status;  /* high nibble is BUSY=0x00, READY=0x10, ERROR=0x20
						      low nibble is start_stop_phase */
bit				start_stop_command_busy;
static bit			disc_type_known;
bit				cd_disc;	    /* 1:cd_disc 0:cdr_disc



/* =================== DISK INFO INTERFACE FUNCTIONS ==================== */
bit is_cd_disc(void)
{
   return cd_disc;
}
/* ================= END DISK INFO INTERFACE FUNCTIONS ================== */


/**************************************************************************/
void init_for_new_disc(void)
{
   disc_size_known=FALSE;
   disc_type_known=FALSE;
   cd_disc=TRUE;
}

static byte get_disk_type(void)
{
   byte	exec_status;

   if (disc_type_known) return READY;
   switch (start_stop_process & 0x0F)
   {
   case 0: start_subcode_reading();
	   play_timer = SUBCODE_TIMEOUT_VALUE;
	   start_stop_process++;
   break;

   case 1: /* check if in first leadin */
	   if (is_subcode(FIRST_LEADIN_AREA) )
	   {
	      /*goto reading TOC*/
	      start_stop_process = start_stop_process & 0xF0 | 0x03;
	   }
	   else if (is_subcode(ABS_TIME) )
	   {
	      start_stop_process++;
	   }
	   else if (play_timer == 0)
	   {
	      player_error = SUBCODE_TIMEOUT_ERROR;
	      return ERROR;
	   }
   break;

   case 2: /* first return to the first leadin */
	   store.toc_info.tmp_time.min = 0;
	   store.toc_info.tmp_time.sec = 2;
	   store.toc_info.tmp_time.frm = 0;
	   exec_status = jump_time(&store.toc_info.tmp_time);
	   if (exec_status == READY)
	   {
	      start_stop_process++;
	      start_subcode_reading();
	      play_timer = SUBCODE_TIMEOUT_VALUE;
	   }
	   else return exec_status;

   case 3: /* init all variables for new toc reading */
	   store.toc_info.counter = 5; /* jump max 5 times back into leadin */
	   play_timer = SUBCODE_TIMEOUT_VALUE;
	   start_stop_process++;

   case 4: /* read toc information */
	   if (is_subcode(FIRST_LEADIN_AREA) )
	   {
	      if (Q_buffer.r_time.min > 90)
		 /*CDR disk*/
		  cd_disc=FALSE;
	      else
		  cd_disc=TRUE;
	      disc_type_known=TRUE;
	      return READY;
	   }
	   else if (is_subcode(PROGRAM_AREA) )
	   {
	      servo_jump(TRACKS_INTO_LEADIN);
	      start_stop_process++;
	   }
	   else if (play_timer == 0)
	   {
	      player_error = SUBCODE_TIMEOUT_ERROR;
	      return ERROR;
	   }
   break;

   case 5: /* monitor jump back into leadin */
	   exec_status = get_servo_process_state();
	   if (exec_status == READY)
	   {
	      store.toc_info.counter--;
	      if (store.toc_info.counter==0)
	      {
		 player_error = TOC_READ_ERROR;
		 return ERROR;
	      }
	      else
	      {
		 start_subcode_reading();
		 play_timer = SUBCODE_TIMEOUT_VALUE;
		 start_stop_process--;
	      }
	   }
	   else return exec_status;
   break;
   }

   return BUSY;
}



/**************************************************************************/
byte start_stop(byte cmd)
{
   if (start_stop_process >> 4 != cmd || !start_stop_command_busy)
   {
      start_stop_process = cmd << 4; /*also set ss_subphase 0*/
      start_stop_status = BUSY<<4; /* set busy status and phase 0 */
      start_stop_command_busy = TRUE;
   }
   if (start_stop_status>>4 != BUSY) start_stop_command_busy = FALSE;
   return start_stop_status>>4;
}


/**************************************************************************/
void execute_start_stop_functions(void)
{
   byte   exec_status;

   if (start_stop_process>>4 == SS_IDLE)
   {
      if (!(start_stop_status & 0x0F))
      {
	 start_stop_status = READY<<4 | 0x01; /* set ready status and phase 1 */
      }
   }
   else if (start_stop_process>>4 == SS_START_UP)
   {
      switch (start_stop_status & 0x0F)
      {
      case 0: servo_start();
	      start_stop_status++;
      case 1: exec_status = get_servo_process_state();
	      if (exec_status == READY)
	      {
		 start_subcode_reading();
		 start_stop_process &= 0xF0;
		 start_stop_status++;
	      }
	      else
	      {
		 if (exec_status == ERROR)
		    start_stop_status = ERROR<<4 | 0x04; /* set error status and phase 4 */
		 break;
	      }

      case 2: exec_status=get_disk_type();
	      if (exec_status == READY)
		 start_stop_status = READY<<4 | 0x03; /* set ready status and phase 3 */
	      else
	      {
		 if (exec_status == ERROR)
		    start_stop_status = ERROR<<4 | 0x03; /* set error status and phase 3 */
		 break;
	      }
      case 3: /* do nothing */
	      break;
      }
   }
   else
   {  /* start_stop_process = SS_STOP or SS_SPEED_N1 or SS_SPEED_N2 or SS_MOTOR_OFF */
      switch (start_stop_status & 0x0F)
      {
      case 0: if (start_stop_process>>4 == SS_SPEED_N1) servo_n1();
	      else if (start_stop_process>>4 == SS_SPEED_N2) servo_n2();
	      else /* SS_STOP or SS_MOTOR_OFF */ servo_stop();
	      if (start_stop_process>>4 == SS_MOTOR_OFF)
		 /* don't wait for servo process is ready, only switch off spindle motor */
		 start_stop_status = READY<<4 | 0x02; /* set ready status and phase 2 */
	      else
		 start_stop_status++;
	      break;
      case 1: exec_status = get_servo_process_state();
	      if (exec_status == READY)
	      {
		 start_stop_status = READY<<4 | 0x02; /* set ready status and phase 2 */
	      }
	      else
	      {
		 if (exec_status == ERROR)
		    start_stop_status = ERROR<<4 | 0x02; /* set error status and phase 2 */
		 break;
	      }
      case 2: /* do nothing */
	      break;
      }
   }
}