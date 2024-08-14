/***************************************************************************
*
* Project:	BASIC PLAYER MODULE
* File:		servo.c
* Date:		06 July 1993
* Version:	0.1
* Status:	Draft
*
* Description:	this module controls all servo actions
*		It has the following tasks:
*               - control sledge movement
*		- turn laser on/off
*		- enable/disable focus search loop
*		- turn spindle motor on/off and speed change
*		- radial initialisation
*		- controlling jump tracks
*		- focus recovery when focus lost
*		- radial recovery when radial lost
*
* Interface functions: void servo_init(void)
*                      void servo_start(void)
*                      void servo_reinit_sledge(void)
*                      void servo_n1(void)
*                      void servo_n2(void)
*		       void servo_jump_sledge_cal(void)
*                      void servo_stop(void)
*                      void servo_jump(int jump_size)
*                      byte get_servo_process_state(void)
*                      byte get_jump_status(void)
*                      bit servo_tracking(void)
*		       bit servo_to_service(void)
*                      void servo(void)
*
* Interface functions for service.c:	byte switch_laser_on(void)
*                      			byte switch_focus_off(void)
*                      			byte turn_laser_focus_on(void)
*                      			byte turn_focus_laser_off(void)
*                      			byte turn_radial_off(void)
*                      			bit dsic_in_focus(void)
*                      			bit dsic_on_track(void)
*                      			void rad_hold(void)
*                      			void rad_start(void)
*                      			void sledge_off(void)
*                      			void jump_servo_state(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 16 November 1992	creation P.vd.Z.
* 30 March 1993		function "read_off_track_value" changed for correct sign bit,
*			so functions "jump_servo_state" and "check_jump_state" are changed.
* 31 March 1993		functions "servo_jump_low_speed" and "servo_jump_high_speed" added for disc scan.
* 19 April 1993		max jump speed changed for N=1.
* 22 April 1993		function "set_motor_gain" added.
* 05 May 1993		function "get_jump_status" added.
*			check on hf present during jump.
* 06 May 1993		in radial recover state no delay anymore but when
*                       sledge switch not closed immediate go to check focus.
* 07 May 1993		function "servo_restart" added.
*
* 17 May 1993		PVDZ 	switch_focus_off :laser stays on
*                               renamed laser_off_array in focus_off_array
*				function returns READY
* 17 May 1993		PVDZ    switch_focus_on : laser already on
*                               renamed laser_on_array in focus_on_array
* 17 May 1993		PVDZ    added switch_laser_on, switch_laser_off
* 17 May 1993		PVDZ    added servo_is_idle routine
* 18 May 1993           PVDZ    made PUBLIC:
*					switch_laser_on, switch focus_off
*					dsic_on_track, dsic_in_focus
*					rad_start, rad_hold, sledge_off
*					reload_servo_timer, grooves
* 24 May 1993		PVDZ    made PUBLIC motor_started, initialized
*				switch_focus_on: check if already in focus added
*
* 27 May 1993		function "servo_reinit_sledge" added.
*			function "servo_restart" deleted.
*			function "servo_is_idle" renamed and redefined to
*			"servo_to_service".
* 01 Jume 1993		check on MOT_STRT_1 instead of MOT_STRT_2 in
*			function: double_check_focus.
*			check in function "radial_recover_state" for error
*			can be a subcode timeout error.
* 02 June 1993		states upto_n2_state and downto_n1_state go via
*			wait_subcode_state to servo_monitor_state.
*			monitoring of subcode reading in servo_monitor_state.
* 04 June 1993		stopping algorithm changed.
*			new flag "motor_on_speed" replaces "motor_started".
*			flag "motor_started" is set as soon as spindle motor
* 			turned on.
* 11 June 1993		functions: rad_hold + sledge_off = turn_radial_off.
*               	functions: switch_focus_off + switch_laser_off = turn_focus_laser_off.
*               	functions: switch_laser_on + switch_focus_on = turn_laser_focus_on.
*			functions all return ready.
*             		function "status_cd6" moved from servo.c to cd6.c.
* 15 June 1993		new functions: dsic_sledge_jump, servo_jump_sledge_cal.
*			sledge calibration added.
* 16 June 1993		new variable: n2_jump_speed.
* 17 June 1993		functions "servo_jump_low_speed", "servo_jump_high_speed" and
*			flag "jump_low_speed" deleted. scan speed is set by n2_jump_speed.
* 18 June 1993		sledge speed calculation changed for: MAX < jump < BRAKE_2_DIS_MAX
*			in function "jump_servo_state".
* 22 June 1993		new function: "get_jump_speed".
* 23 June 1993		spindle motor status signals only valid when pll in lock.
* 24 June 1993		check on working of hf detector, error -> HF_DETECTOR_ERROR.
* 30 June 1993		during stopping a check for re-start is implemented.
*                       in servo_monitor_state the (dummy) check for
*			motor overflow is deleted.
* 02 July 1993		check on motor overflow when switching from
*			wait_subcode_state to servo_monitor_state
*			(new flag: monitor_motor_overflow).
* 05 July 1993          added active brake_ok ==> for games6001 added disc_size (8cm) in stop_servo
*			added top_loader and lid open detection
* 06 July 1993		compiler switch on motor high/low gain
*
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

extern void wr_dsic2(byte);
extern byte rd_dsic2(void);
extern void cd6_wr(byte);
extern bit status_cd6(byte);
extern void delay(byte);
extern bit  sledge_switch(void);
extern bit  cd6_status_pin(void);
extern bit  hf_present(void);
extern bit  door_closed(void);
extern void start_subcode_reading();
extern byte get_area(void);

extern byte			servo_timer;		/* multiple 8ms timer */
extern byte			kick_brake_timer;       /* multiple 8ms timer */
extern byte			player_error;    	/* contains type of servo error */
extern bit			n1_speed;               /* also controls DALAS */
extern bit			i_can_read_subcode;     /* set when valid subcode is read */
extern struct {
	      byte		samples;
	      unsigned short	offtrack_1;
	      unsigned short	offtrack_2;
	      struct time	tmp_time;
	      byte		counter;
	      } store; /* should be the same format as in strtstop.c !!! */

static byte			servo_exec_state;      	/* command execution-> busy,ready,error */
static byte			servo_state;	       	/* operation state */
static byte			servo_requested_state; 	/* requested operation state */
static byte			servo_retries;	       	/* contains amount of retries yet to do */
static unsigned short		off_track_value;       	/* contains last read DSIC2 offtrack value */
int_hl     			grooves;	       	/* contains request amount of grooves to be jumped */
static bit			n2_speed_req;
static bit			rad_recover_in_jump;
static bit			foc_recover_in_jump;
static bit			no_efm_in_jump;
bit				initialized;	       	/* indicates radial initializing done */
bit				motor_started;          /* set when spindle motor turned on */
bit				motor_on_speed;         /* set when spindle motor at correct speed */
bit				reload_servo_timer;
bit				disc_size;
bit				disc_size_known;
static   bit			kick, brk;

/***********************************************************************************************
 * Function  : calc_kick
 * Input     : n=1 or n=2
 * Output    : time to kick
 * Abstract  : calculate with use of speed information kick time
 * Decisions : offtrack value must be valid
*/
byte calc_kick(void)
{
   int	 	time;
   byte  	offset;

   if (get_area() == 1)
      /*JUMP TO INSIDE WITHIN AREA 1*/
      offset=6;
   else
   {
      if (get_area() == 2)
      {
	 /*AREA 2*/
	 if (off_track_value > 4000)
	     /*BIG JUMP TO AREA 1*/
	     offset=6;
	 else
	     /*JUMP WITHIN AREA 2*/
	     offset=1;
      }
      else
      {
	 /*AREA 3*/
	 if (n1_speed)
	    /* SINGLE SPEED: NO OFFSET NEEDED IN CALCULATION*/
	    offset=0;
	 else
	 {
	    /*DOUBLE SPEED: ADAPT OFFSET ACCORDING TO JUMP LENGTH*/
	    if (off_track_value > 10000)
	       /*BIG JUMP INSIDE ON DOUBLE SPEED*/
	       offset=9;
	    else
	       /*NOT SO A BIG JUMP ON DOUBLE SPEED*/
	       offset=0;
	 }
      }
   }
   /*NOW OFFSET IS ADDED TO FORMULA*/
   /* kick (3 * (|jump|) / 8) + offset
    * 	         ----
    *	          128
    *
   */

   if (n1_speed)
   {
      time=off_track_value >> 3; /* |jump| / 8 */
      time=time * 3;		 /* (3 * |jump| / 8) */
      time=time >> 7;		 /* (3 * |jump| / 8) / 128 */
      time = time + offset;      /* (3 * |jump| / 8) / 128 + 6*/
      return time;
   }
   /*DOUBLE SPEED*/
   time=off_track_value >> 3;	 /* |jump| / 8 */
   time=time * 3;		 /* (3 * |jump| / 8) */
   time=time >> 7;		 /* (3 * |jump| / 8) / 128 */
   time = time + (offset << 2);  /* (3 * |jump| / 8) / 128 + 24 */
   return time;
}

/***********************************************************************************************
 * Function  : calc_brake
 * Input     : n=1 or n=2
 * Output    : time to brake
 * Abstract  : calculate with use of speed information brake time
 * Decisions : offtrack value must be valid
*/
byte calc_brake(void)
{
   int	 time;

   if (n1_speed)
   {
      if (get_area() == 1)  		time=6;
      else if (get_area() == 2)  	time=1;
      else 				time=0;
   }
   else
   {
      /* DOUBLE SPEED*/
      if (get_area() == 1) 		time=16;
      else if (get_area() == 2)  	time=3;
      else 				time=0;
   }
   return time;
}

/***********************************************************************************************
 * Function  : active_brake_ok
 * Input     : -
 * Output    : yes / no
 * Abstract  : checks if active brake is allowed in games6001 application
 * Decisions :
*/
static bit active_brake_ok(void)
{
   if (!disc_size)
      return FALSE;
   if (!door_closed() )
      return FALSE;
   return TRUE;
}


/***********************************************************************************************
 * Function  : wr_dsic2_array
 * Input     : array_pointer,array_length
 * Output    : -
 * Abstract  : send commands to dsic2 that are in array pointed by array pointer
 * Decisions :
*/
static void wr_dsic2_array(byte const *array_pointer, byte array_length)
{
   while (array_length != 0)
   {
      wr_dsic2(*array_pointer);
      array_pointer++;
      array_length--;
   }
}

/***********************************************************************************************
 * Function  : jump_long
 * Input     : brake
 * Output    : -
 * Abstract  : send srcomm5[brake,jump_speed,HIGH(grooves),LOW(grooves),rad_stat5]
 * 	       command to DSIC2
 * Decisions :
*/
static void jump_long(signed char brake)
{
   wr_dsic2(SRCOMM5);				/* write long jump command*/
   wr_dsic2(brake);				/* write BRAKE distance*/
   wr_dsic2(SLEDGE_UOUT_JMP);			/* write SLEDGE speed*/
   wr_dsic2(grooves.byte_hl.high);		/* write high(grooves)*/
   wr_dsic2(grooves.byte_hl.low);		/* write low(grooves)*/
   wr_dsic2(RAD_STAT5);				/* write status*/
}


/***********************************************************************************************
 * Function  : jump_short
 * Input     : -
 * Output    : -
 * Abstract  : send srcomm3[HIGH(grooves),LOW(grooves),rad_stat3]
 * 	       command to DSIC2
 * Decisions :
*/
static void jump_short(void)
{
   wr_dsic2(SRCOMM3);			/* write short jump command*/
   wr_dsic2(grooves.byte_hl.high); 	/* high(grooves)*/
   wr_dsic2(grooves.byte_hl.low);	/* low(grooves)*/
   wr_dsic2(RAD_STAT3);			/* radstat after jump*/
}


/***********************************************************************************************
 * Function  : sledge_in
 * Input     : -
 * Output    : -
 * Abstract  : move sledge towards leadin
 * Decisions :
*/
static void sledge_in(void)
{
   wr_dsic2(SRSLEDGE);        	/* write sledge command*/
   wr_dsic2(SLEDGE_UOUT_IN);  	/* write sledge in*/
}


/***********************************************************************************************
 * Function  : sledge_out
 * Input     : -
 * Output    : -
 * Abstract  : move sledge towards leadout
 * Decisions :
*/
static void sledge_out(void)
{
   wr_dsic2(SRSLEDGE);        	/* write sledge command*/
   wr_dsic2(SLEDGE_UOUT_OUT); 	/* write sledge out */
}


/***********************************************************************************************
 * Function  : sledge_off
 * Input     : -
 * Output    : -
 * Abstract  : stop moving sledge
 * Decisions :
*/
void sledge_off(void)
{
   wr_dsic2(SRSLEDGE);        	/* write sledge command*/
   wr_dsic2(SLEDGE_UOUT_OFF); 	/* write sledge off*/
}


/***********************************************************************************************
 * Function  : switch_laser_on
 * Input     : -
 * Output    : READY
 * Abstract  : switch laser on
 * Decisions : as a side effect the clock divider and reference voltage are set
*/
byte switch_laser_on(void)
{
   wr_dsic2(PRESET);
   wr_dsic2(LASER_ON);
   return READY;
}


/***********************************************************************************************
 * Function  : switch_laser_off
 * Input     : -
 * Output    : -
 * Abstract  : switch laser off
 * Decisions : as a side effect the clock divider and reference voltage are set
*/
static void switch_laser_off(void)
{
   wr_dsic2(PRESET);
   wr_dsic2(LASER_OFF);
}


/***********************************************************************************************
 * Function  : dsic_in_focus
 * Input     : -
 * Output    : CY = 1: focus
 *	       CY = 0: not focus
 * Abstract  : read focus status
 * Decisions :
*/
bit dsic_in_focus(void)
{
   wr_dsic2(RSTAT1);			/* ask dsic2 for focus status */
   return (byte)(rd_dsic2() & 0xC0) == 0x80;   	/* read focus status*/
   /* select relevant bits, bit 7 = 1 and bit 6 = 0 ?, yes: in focus */
}


/***********************************************************************************************
 * Function  : switch_focus_on
 * Input     : -
 * Output    : -
 * Abstract  : check if dsic not in focus and then switch focus on
 * Decisions : laser has to be switched on
*/
static void switch_focus_on(void)
{
static byte const focus_on_array[] =
   { SFCOMM,FOC_MASK,FOC_STAT_ON,SHOCK_LEVEL };

   if ( !dsic_in_focus() )
   {  /*NOT YET IN FOCUS*/
      wr_dsic2_array(focus_on_array,sizeof(focus_on_array));
   }
}


/***********************************************************************************************
 * Function  : switch_focus_off
 * Input     : -
 * Output    : READY
 * Abstract  : switch focus off
 * Decisions : laser has to be switched off separately
*/
byte switch_focus_off(void)
{
static byte const focus_off_array[] =
   { SFCOMM,FOC_MASK,FOC_STAT_OFF,SHOCK_LEVEL };

   wr_dsic2_array(focus_off_array,sizeof(focus_off_array));
   return READY;
}


/***********************************************************************************************
 * Function  : turn_laser_focus_on
 * Input     : -
 * Output    : READY
 * Abstract  : first switch laser on and then switch focus on
 * Decisions : -
*/
byte turn_laser_focus_on(void)
{
   switch_laser_on();
   switch_focus_on();
   return READY;
}


/***********************************************************************************************
 * Function  : turn_focus_laser_off
 * Input     : -
 * Output    : READY
 * Abstract  : switch focus and laser off
 * Decisions : -
*/
byte turn_focus_laser_off(void)
{
   switch_focus_off();
   switch_laser_off();
   return READY;
}


/***********************************************************************************************
 * Function  : dsic_on_track
 * Input     : -
 * Output    : CY = 1: radial lock
 *	       CY = 0: not radial lock
 * Abstract  : read radial status, put mask over it
 * Decisions :
*/
bit dsic_on_track(void)
{
   wr_dsic2(RSTAT2);			/* ask dsic2 for radial status */
   return (byte)(rd_dsic2() & 0xB3) == 0x90;	/* read status */
   /* select relevant bits 7, 5, 4 and 1, 0; bits 7 and 4 = 1 ?, yes: radial lock */
}

/***********************************************************************************************
 * Function  : rad_hold
 * Input     : -
 * Output    : -
 * Abstract  : turn radial servo to hold mode
 * Decisions :
*/
void rad_hold(void)
{
   wr_dsic2(SRCOMM1);		/* send radial task*/
   wr_dsic2(HOLD_MODE);       	/* switch to hold mode*/
}


/***********************************************************************************************
 * Function  : rad_start
 * Input     : -
 * Output    : -
 * Abstract  : turn radial servo on
 * Decisions :
*/
void rad_start(void)
{
   static byte const prs_rad_init_table[] =
   { SRINIT,RE_OFFSET,RE_GAIN,SUM_GAIN,SRCOMM1,INIT_MODE };

   rad_hold();
   wr_dsic2_array(prs_rad_init_table,sizeof(prs_rad_init_table));
}


/***********************************************************************************************
 * Function  : turn_radial_off
 * Input     : -
 * Output    : READY
 * Abstract  : turn radial servo off
 * Decisions :
*/
byte turn_radial_off(void)
{
   rad_hold();
   sledge_off();
   return READY;
}


/***********************************************************************************************
 * Function  : read_off_track_value
 * Input     : -
 * Output    : offtrack_value of real tracks to jump (not half tracks!!)
 * Abstract  : read 4 status bytes rstat2, skip the first 2
 * Decisions :
*/
static int read_off_track_value(void)
{
#pragma asm
   MOV	   _wr_dsic2_BYTE,#0F0H
   LCALL   _wr_dsic2               	;ask dsic2 for radial status
   LCALL   _rd_dsic2               	;read rad_stat (ignore)
   LCALL   _rd_dsic2               	;read rad_error (ignore)
   LCALL   _rd_dsic2			;read offtrack_value_high
   MOV	   R6,A				;store MSB in R6
   LCALL   _rd_dsic2			;read offtrack_value_low
   MOV	   R7,A				;store LSB in R7
; now determine sign bit and set in CY
; if R6 = 1111xxxx then CY = 1
; else if R6 = 0000xxxx then CY = 0
;      else CY = bit 7 of _grooves
   CLR     C				;sign bit = 0
   MOV	   A,R6
   ANL	   A,#0F0H
   JZ      div_by_2
   CJNE    A,#0F0H,set_sign_bit
   SETB    C				;sign bit = 1
   SJMP    div_by_2
set_sign_bit:
   MOV     A,_grooves			;get grooves to jump msb
   MOV	   C,0E7H			;sign bit = bit 7 of _grooves
div_by_2:
   MOV     A,R6
   RRC     A
   MOV	   R6,A
   MOV     A,R7
   RRC     A
   MOV     R7,A
#pragma endasm
}


/***********************************************************************************************
 * Function  : set_motor_gain
 * Input     : -
 * Output    : -
 * Abstract  : sets the new spindle motor gain dependent on n1_speed and
 *             disc_size
 * Decisions :
*/
static void set_motor_gain(void)
{
   if (disc_size == (bit)TWELVE)
   {
      if (n1_speed) cd6_wr(MOT_GAIN_12CM_N1);
      else cd6_wr(MOT_GAIN_12CM_N2);
   }
   else
   {
      if (n1_speed) cd6_wr(MOT_GAIN_8CM_N1);
      else cd6_wr(MOT_GAIN_8CM_N2);
   }
}


/***********************************************************************************************
 * Function  : init_dsic2_state
 * Input     : -
 * Output    :
 * Abstract  : Reset communication with DSIC2
 *	       initialise clock divider / reference voltage / laser off
 * Decisions :
*/
static void init_dsic2_state(void)
{
   static byte const init_array[] =
   { SFCOEF1,FOC_PARM3,FOC_INT,RAMP_INC,RAMP_HEIGHT,RAMP_OFFSET,FE_START,FOC_GAIN,
     SFCOEF2,RAD_PARM_JUMP,VEL_PARM2,VEL_PARM1,FOC_PARM1,FOC_PARM2,CA_DROP,
     SRCOEFS,RAD_LENGTH_LEAD,RAD_INT,RAD_PARM_PLAY,RAD_POLE_NOISE,RAD_GAIN,SLEDGE_PARM2,SLEDGE_PARM1
   };

   wr_dsic2(PRESET);		/* send preset command*/
   delay(1);			/* wait 500 us */
   wr_dsic2(LASER_OFF);       	/* send preset parameter*/
   delay(1);                  	/* wait 500 us */

   wr_dsic2_array(init_array,sizeof(init_array));
   servo_state=INIT_SLEDGE;	/* select new state*/
}


/***********************************************************************************************
 * Function  : init_sledge_state
 * Input     : -
 * Output    : -
 * Abstract  : check the position of the sledge, and move to the leadin position
 *             which is the position at which the sledge switch is just opened 
 * Decisions :
*/
static void init_sledge_state(void)
{
   rad_hold();     /* EXTRA PROTECTION ENSURE THAT ALL RADIAL FUNCTIONS ARE SWITCHED OFF */
   if ((bit)sledge_switch()==(bit)CLOSED)
   {
      /* SLEDGE ALREADY IN */
      sledge_out();			/* send sledge outwards*/
      servo_timer=SLEDGE_OUT_TIME; 	/* max time that sledge is allowed to get outwards*/
      servo_state=CHECK_OUT_SLEDGE;	/* select next state*/
   }
   else
   {
      /* SLEDGE IS OUT */
      sledge_in();			/* send sledge in*/
      servo_timer=HALF_SLEDGE_IN_TIME; 	/* max time that sledge is allowed to reach */
      reload_servo_timer=TRUE;		/* the "sledge switch is closed" position*/
      servo_state=CHECK_IN_SLEDGE; 	/* select next state*/
   }
}


/***********************************************************************************************
 * Function  : check_in_sledge_state
 * Input     : -
 * Output    : -
 * Abstract  : checks if the sledge has already reached the leadin position within time out
 * Decisions :
*/
static void check_in_sledge_state(void)
{
   if (servo_timer==0)
   {
      if (reload_servo_timer)
      {
	 servo_timer=HALF_SLEDGE_IN_TIME;
	 reload_servo_timer=FALSE;
      }
      else
      {  /* TIMER EXPIRED */
	 /* sledge did not reach "sledge home position" */
	 sledge_off();			  /* switch sledge off */
	 servo_state=SERVO_IDLE;	  /* select next state */
	 servo_requested_state=SERVO_IDLE;
	 player_error=SLEDGE_ERROR;	  /* indicate error */
	 servo_exec_state=ERROR; 	  /* execution ended with an error */
      }
   }
   else
   {  /* NOT YET TIME OUT */
      if ((bit)sledge_switch()==(bit)CLOSED)
      {
	 /* sledge reached "sledge home position" */
	 sledge_off();
	 sledge_out();  		/* steer sledge outwards*/
	 servo_timer=SLEDGE_OUT_TIME;   /* time that sledge is sent outwards*/
	 servo_state=CHECK_OUT_SLEDGE;  /* select next state*/
      }
   }
}


/***********************************************************************************************
 * Function  : check_out_sledge_state
 * Input     : -
 * Output    : -
 * Abstract  : check if the sledge out position is reached within time-out
 * Decisions :
*/
static void check_out_sledge_state(void)
{
   if (servo_timer==0)
   {
      /* TIMER EXPIRED SLEDGE NOT "OUT" WITHIN TIME OUT */
      sledge_off();			/* switch sledge off*/
      servo_state=SERVO_IDLE;	        /* select next state*/
      servo_requested_state=SERVO_IDLE;
      player_error=SLEDGE_ERROR;        /* indicate error*/
      servo_exec_state=ERROR;      	/* servo execution ended with an error*/
   }
   else
   {
      if ((bit)sledge_switch()==(bit)OPEN)
      {
	 /* TIMER NOT EXPIRED AND SLEDGE IS "OUT" */
	 sledge_off();			/* switch sledge off*/
	 servo_state=SERVO_IDLE;   	/* select next state*/
      }
   }
}


/***********************************************************************************************
 * Function  : servo_idle_state
 * Input     : -
 * Output    : -
 * Abstract  : wait for command request to start the servo
 *	       (= request for a state change)
 * Decisions :
*/
static void servo_idle_state(void)
{
   if (servo_requested_state==START_FOCUS)
   {
      /* REQUEST FOR STARTING SERVO FUNCTIONS */
      /* START IN FIRST STATE --> START FOCUS */
	 servo_state=START_FOCUS;
	 servo_requested_state=SERVO_MONITOR;
   }
   else if (servo_requested_state==JUMP_SERVO)
   {
      servo_requested_state=SERVO_IDLE;
      servo_exec_state=ERROR;
      player_error=ILLEGAL_COMMAND;
   }
   else if (servo_exec_state!=ERROR)
   {
      servo_exec_state=READY;
   }
}


/***********************************************************************************************
 * Function  : start_focus_state
 * Input     : -
 * Output    : -
 * Abstract  : enable the laser output and startup the focus control loops
 *	       initialise number of servo trials--> maximum number of servo errors
 * Decisions :
*/
static void start_focus_state(void)
{
   turn_laser_focus_on();
   servo_timer=FOCUS_TIME_OUT;          /* time to find focus*/
   servo_state=CHECK_FOCUS;		/* check focus state*/
   servo_retries=MAX_RETRIES;           /* number of startup retries*/
}


/***********************************************************************************************
 * Function  : check_focus_state
 * Input     : -
 * Output    : -
 * Abstract  : check if within timeout focus is found
 * Decisions :
*/
static void check_focus_state(void)
{
   if (servo_timer==0)
   {
      /* FOCUS TIMER EXPIRED */
      /* NO FOCUS FOUND !! */
      turn_focus_laser_off();
      player_error=FOCUS_ERROR;		/* indicate error*/
      servo_exec_state=ERROR;		/* servo execution ended with an error*/
      servo_state=STOP_SERVO;
      servo_requested_state=SERVO_IDLE;
   }
   else
   {
      /* READ DSIC2 STATUS INFORMATION */
      if (dsic_in_focus())
      {
	 /* FOCUS FOUND */
	 servo_timer=TIME_DOUBLE_FOCUS_CHECK;
	 servo_state=DOUBLE_CHECK_FOCUS;
      }
   }
}


/***********************************************************************************************
 * Function  : double check focus
 * Input     : -
 * Output    : -
 * Abstract  : check focus after delay =>  OK: start motor
 *					   not OK recover
 * Decisions :
*/
static void double_check_focus_state(void)
{
   if (servo_timer==0)
   {
      /* DELAY IS OVER -> READ DSIC2 FOCUS STATUS INFORMATION AGAIN */
      if (dsic_in_focus())
      {
	 /* FOCUS FOUND */
	 servo_state=START_TTM;
      }
      else
	 servo_state=FOCUS_RECOVER;
   }
}


/***********************************************************************************************
 * Function  : start_ttm_state
 * Input     : -
 * Output    : -
 * Abstract  : trigger ttm process to startup the turntable motor
 * Decisions :
*/
static void start_ttm_state(void)
{
   cd6_wr(MOT_STRTM1_ACTIVE); /* start cd6 motor in mode 1; anti wind up active */
   servo_timer=SPEEDUP_TIME;  /* time that motor is accelerated in mode 1 */
   servo_state=TTM_SPEED_UP;
   motor_started=TRUE;
}


/***********************************************************************************************
 * Function  : ttm_speedup_state
 * Input     : -
 * Output    : -
 * Abstract  : monitor the time to speed up to 10 % of nominal speed
 *	       (by waiting)
 * Decisions :
*/
static void ttm_speedup_state(void)
{
   if (dsic_in_focus())
   {
      /* FOCUS STILL OK */
      if (servo_timer==0)
      {
	 /*DISK SPEED > 10 %*/
	 /*SWITCH TO MOTOR START 2*/
	 /*MONITOR DISK SPEED*/
	 cd6_wr(MOT_STRTM2_ACTIVE);      	/* cd6 motor mode 2; anti wind active*/
	 servo_state=CHECK_TTM;
      }
   }
   else
      /* FOCUS ERROR TRY TO REPAIR */
      servo_state=FOCUS_RECOVER;
}


/***********************************************************************************************
 * Function  : check_ttm_state
 * Input     : -
 * Output    : -
 * Abstract  : monitor the time to speed up to 75% of nominal speed
 *	       by testing mot_stat1 signal (status)
 * Decisions :
*/
static void check_ttm_state(void)
{
   if (dsic_in_focus())
   {
      /* FOCUS STILL OK */
      if (status_cd6(MOT_STRT_1) )
      {
	 /* MOTOR TURNING AT 75% OF NOMINAL SPEED */
	 set_motor_gain();
	 motor_on_speed=TRUE;

	 /* CHECK IF HF DETECTOR IS WORKING */
	 if (!hf_present())
	 {  /* ERROR IN HF DETECTOR CIRCUIT */
	    servo_state=STOP_SERVO;
	    servo_requested_state=SERVO_IDLE;
	    servo_exec_state=ERROR;
	    player_error=HF_DETECTOR_ERROR;
	 }
	 else
	 {  /* HF DETECTOR IS OKE */
	    servo_state=START_RADIAL;
	 }
      }
   }
   else
      /* FOCUS ERROR TRY TO REPAIR */
      servo_state=FOCUS_RECOVER;
}


/***********************************************************************************************
 * Function  : start_radial_state
 * Input     : -
 * Output    : -
 * Abstract  : double check focus and start the radial signal initialization
 * Decisions :
*/
static void start_radial_state(void)
{
   servo_timer=0;
   if (dsic_in_focus())
   {
      /* FOCUS STILL OKE */
      /* CHECK IF ALREADY RADIAL INITIALISE DONE ON THIS DISK */
      if (!initialized)
      {
	 /* RESET GAIN AND OFFSET PARAMETERS */
	 /* THIS MUST BE DONE AT LEAST ONCE FOR EVERY DISK */
	 /* TTM TURNING AND SLEDGE ON INNERSIDE FOR OFFSET */
	 rad_start();
	 servo_timer=RAD_INITIALIZE_TIME;
      }
      servo_state=INIT_RADIAL;
   }
   else
   {
      /* FOCUS ERROR OCCURED -> TRY TO REPAIR */
      servo_state=FOCUS_RECOVER;
   }
}


/***********************************************************************************************
 * Function  : init_radial_state
 * Input     : -
 * Output    : -
 * Abstract  : initialize servo gain and offsett for 300 ms
 * Decisions :
*/
static void init_radial_state(void)
{
   if (dsic_in_focus())
   {
      if (servo_timer==0)
      {
	 /* READY WITH INITIALIZING */
	 rad_hold();			/* switch off initializing*/
	 initialized=TRUE;
	 /* DUMMY JUMP COMMAND TO FIND RADIAL LOCK */
	 grooves.int_i=-10;		/* jump 10 grooves*/
	 servo_state=JUMP_SERVO;        /* select next state*/
      }
   }
   else
   /* FOCUS ERROR -> TRY TO REPAIR */
   {
      servo_state=FOCUS_RECOVER;
   }
}


/***********************************************************************************************
 * Function  : upto_n2_state
 * Input     : -
 * Output    : -
 * Abstract  : Wait till motor is turning on double speed
 * Decisions :
*/
static void upto_n2_state(void)
{
   if ( servo_timer < (byte)(UPTO_N2_TIME) )
   {
      /* speed of disk > 75% of nominal speed */
      cd6_wr(MOT_PLAYM_ACTIVE);
      start_subcode_reading();
      servo_state=WAIT_SUBCODE;
      servo_timer=SUBCODE_TIME_OUT;
   }
}


/***********************************************************************************************
 * Function  : downto_n1_state
 * Input     : -
 * Output    : -
 * Abstract  : Wait till motor is turning on single speed
 * Decisions :
*/
static void downto_n1_state(void)
{
   if (!n1_speed)
   {
      /* not yet switched to single speed mode */
      if (!status_cd6(MOT_STRT_2) || servo_timer==0)
      {
	 /* motor speed below 50 % for double speed => switch to single speed */
	 cd6_wr(SPEED_CONTROL_N1);
	 cd6_wr(MOT_STRTM2_ACTIVE); /* give motor control a chance to control speed to single speed */
	 n1_speed=TRUE;
	 set_motor_gain();
      }
   }
   else
   {
      /* already switched to single speed mode, wait for motor control to settle */
      if (status_cd6(MOT_STRT_1) )
      {
	 cd6_wr(MOT_PLAYM_ACTIVE);
	 start_subcode_reading();
	 servo_state=WAIT_SUBCODE;
	 servo_timer=SUBCODE_TIME_OUT;
      }
   }
}


/***********************************************************************************************
 * Function  : servo_monitor_state
 * Input     : -
 * Output    : -
 * Abstract  : check servo functions: - radial
 *				      - focus
 *				      - subcode reading
 *				      - motor overflow
 * Decisions :
*/
static void servo_monitor_state(void)
{
   /* IF DURING MONITORING ERROR THEN RECOVER ERROR */
   /* CHECK FOCUS STATE & RADIAL STATE & SUBCODE READING */
   if (dsic_in_focus())
   {
      if ( dsic_on_track() && (i_can_read_subcode || servo_timer!=0) )
      {
	 /* FOCUS STILL OKE & RADIAL OKE & (SUBCODE READING OKE ) */
	 if (i_can_read_subcode)
	 {  /* bit is set in subcode module when valid subcode is read */
	     i_can_read_subcode=FALSE;
	     servo_timer=SUBCODE_MONITOR_TIMEOUT;
	     servo_retries=MAX_RETRIES;
	 }
	 /* CHECK IF MODE SWITCH REQUEST */
	 if (servo_requested_state==JUMP_SERVO)
	 {
	    servo_state=JUMP_SERVO;
	    servo_requested_state=SERVO_MONITOR;
	 }
	 else if (!n1_speed && !n2_speed_req)
	 {
	    /* NEW TARGET STATE = DOWNTO_N1: SINGLE SPEED */
	    servo_state=DOWNTO_N1;
	    cd6_wr(MOT_BRM2_ACTIVE);    /* start braking for n2_to_n1_brake_time */
	    servo_timer=N2_TO_N1_BRAKE_TIME;
	 }
	 else if (n1_speed && n2_speed_req)
	 {
	    /* NEW TARGET STATE = UPTO_N2: DOUBLE_SPEED */
	    servo_state=UPTO_N2;
	    cd6_wr(SPEED_CONTROL_N2);   /* switch to double speed mode */
	    cd6_wr(MOT_STRTM2_ACTIVE);  /* accelerate disk in start mode 2 */
	    n1_speed=FALSE;
	    set_motor_gain();
	 }
	 else
	 {
	    servo_requested_state=SERVO_MONITOR;
	    servo_exec_state=READY;
	 }
      }
      else
      {
	 /* RADIAL ERROR OCCURED -> REPAIR */
	 servo_state=RADIAL_RECOVER;
      }
   }
   else
      /* FOCUS ERROR OCCURED -> TRY TO REPAIR */
      servo_state=FOCUS_RECOVER;
}


/***********************************************************************************************
 * Function  : radial_recover_state
 * Input     : -
 * Output    : -
 * Abstract  : radial error occured try to recover
 * Decisions :
*/
static void radial_recover_state(void)
{
   servo_retries--;
   if (servo_retries==0)
   {
      /* MAXIMUM NUMBER OF SERVO ERRORS OCCURED */
      if (!hf_present() )
      {
	 player_error=HF_DETECTOR_ERROR;
      }
      else if ( dsic_in_focus() && dsic_on_track() )
	 player_error=SUBCODE_TIMEOUT_ERROR; /* can't read subcode */
      else
	 player_error=RADIAL_ERROR;	     /* serious radial error */
      servo_exec_state=ERROR;                /* servo execution ended with an error */
      servo_state=STOP_SERVO;		     /* select next state: also calls rad_hold and sledge_off */
      servo_requested_state=SERVO_IDLE;
   }
   else
   {
      /* SERVO RETRIES <> 0 => TRY TO FIND RADIAL LOCK */
      turn_radial_off();    /* switch off radial servo */
      if ((bit)sledge_switch()==(bit)CLOSED)
      {
	 /* SLEDGE IN POSITION -> RADIAL ERROR CAUSED BY MIRROR: JUMP OUTSIDE */
	 sledge_out();
	 servo_timer=R_REC_OUT_SLEDGE;         /* send sledge outwards for a fixed time*/
	 servo_state=SLEDGE_OUTSIDE_RECOVER;
      }
      else
      {
	 /* SLEDGE IN POSITION */
	 servo_state=INIT_RADIAL;
	 servo_timer=0;
      }
   }
}


/***********************************************************************************************
 * Function  : focus_recover_state
 * Input     : -
 * Output    : -
 * Abstract  : focus error occured try to recover
 * Decisions :
*/
static void focus_recover_state(void)
{
   turn_radial_off();	    /* switch radial servo off*/
   cd6_wr(MOT_OFF_ACTIVE);  /* focus error: motor should be switched off*/
   servo_retries--;
   if (servo_retries==0)
   {
      /* MAXIMUM NUMBER OF SERVO ERRORS OCCURED */
      servo_exec_state=ERROR;           /* servo function ended with an error*/
      servo_state=STOP_SERVO;		/* select next state*/
      servo_requested_state=SERVO_IDLE;
      if (no_efm_in_jump)
	 player_error=HF_DETECTOR_ERROR;
      else
	 player_error=FOCUS_ERROR;
   }
   else
   {
      /* SEND SLEDGE IN PERHAPS JUMPED OUTSIDE DISK */
      servo_timer=F_REC_IN_SLEDGE;
      servo_state=SLEDGE_INSIDE_RECOVER;
      sledge_in(); 			/* steer sledge inwards*/
   }
}


/***********************************************************************************************
 * Function  : sledge_inside_recover_state
 * Input     : -
 * Output    : -
 * Abstract  : sledge is driven inside; wait till timer is expired or home switch is closed
 *             next state will be check_focus
 * Decisions :
*/
static void sledge_inside_recover_state(void)
{
   if (servo_timer==0 || (bit)sledge_switch()==(bit)CLOSED)
   {
      /* TIMER EXPIRED OR SLEDGE IS "IN" */
      sledge_off();
      servo_timer=FOCUS_TIME_OUT;
      servo_state=CHECK_FOCUS;
   }
}


/***********************************************************************************************
 * Function  : sledge_outside_recover_state
 * Input     : -
 * Output    : -
 * Abstract  : sledge is driven towards the outside of the disk; wait till timer is expired
 *	       next state will be init_radial
 * Decisions :
*/
static void sledge_outside_recover_state(void)
{
   if (dsic_in_focus())
   {
      if (servo_timer==0)
      {
	 /* TIMER EXPIRED */
	 sledge_off();
	 servo_state=INIT_RADIAL;
	 servo_timer=0;
      }
   }
   else
      servo_state=FOCUS_RECOVER;
}


/***********************************************************************************************
 * Function  : stop_servo_state
 * Input     : -
 * Output    : -
 * Abstract  : the servo is set in stop mode: next state will be check stop servo
 *	       stopping algorithm of the spindle motor:
 *	       - if motor not yet started -> turn focus/laser off,
 *					     no extra stop delay
 *	       - if motor not yet at normal speed -> turn motor/focus/laser off,
 *						     stop delay is 3*MOT_OFF_STOP_TIME (6 sec)
 *	       - if not on track -> turn all servo's off,
 *				    for N=1 stop delay is 3*MOT_OFF_STOP_TIME (6 sec)
 *				    for N=2 stop delay is 4*MOT_OFF_STOP_TIME (8 sec)
 *	       - if on track -> brake spindle motor, monitor cd6 stop signal,
 *				stop delay is 2 * time to stop motor + EXTRA_STOP_DELAY(=1sec)
 * Decisions : -
*/
static void stop_servo_state(void)
{
   servo_state=CHECK_STOP_SERVO; /* select next state */
   if (motor_started)
   {
      if (motor_on_speed && dsic_on_track() && dsic_in_focus() && active_brake_ok() )
      {
	 cd6_wr(MOT_BRM2_ACTIVE);     /* brake motor with brake mode 2; anti windup active */
	 servo_timer=STOP_TIME_OUT;   /* always stop after time out */
      }
      else
      {  /* turn servo's immediate off when not on track or not yet at correct speed */
	 turn_radial_off();
	 turn_focus_laser_off();
	 cd6_wr(MOT_OFF_ACTIVE);
	 servo_timer=MOT_OFF_STOP_TIME;
	 reload_servo_timer=TRUE;
	 initialized=FALSE;
	 motor_on_speed=~n1_speed; /* set N=1 or N=2 */
	 if ( !active_brake_ok() )
	 {
	    /*only wait EXTRA_STOP_DELAY*/
	    servo_timer=0;
	    motor_started=FALSE;
	 }
      }
   }
   else
   {  /* motor is not yet started */
      turn_focus_laser_off();
      servo_timer=0;
      reload_servo_timer=FALSE;
   }
}


/***********************************************************************************************
 * Function  : check_stop_servo_state
 * Input     : -
 * Output    : -
 * Abstract  : check if ttm spindown process has reached zero speed.
 *	       next state will be init_sledge
 * Decisions :
*/
static void check_stop_servo_state(void)
{
   if (servo_requested_state==START_FOCUS)
   {  /* restart servo's, sledge is not moved to home switch */
      servo_state=START_FOCUS;
      servo_requested_state=SERVO_MONITOR;
      /* to start from a defined state, switch radial/motor off */
      turn_radial_off();
      cd6_wr(MOT_OFF_ACTIVE);
      motor_on_speed=FALSE;
   }
   else if (motor_started)
   {
      if (initialized)
      {  /* monitor motor is stopped signal */
	 if (status_cd6(MOT_STOP) || servo_timer==0)
	 {  /* ttm spinned down speed < 12% */
	    turn_radial_off();
	    turn_focus_laser_off();
	    cd6_wr(MOT_OFF_ACTIVE);
	    servo_timer=(byte)(STOP_TIME_OUT)-servo_timer; /* give extra stop delay */
	    if (servo_timer & 0x80) servo_timer=255;
	    else servo_timer=servo_timer<<1; /* calc: 2 * time-to-stop */
	    reload_servo_timer=TRUE;
	    motor_started=FALSE;
	 }
      }
      else
      {  /* monitor stopping of motor in mot off mode (don't brake) */
	 if (servo_timer==0)
	 {
	    servo_timer=MOT_OFF_STOP_TIME;
	    if (reload_servo_timer) reload_servo_timer=FALSE;
	    else if (motor_on_speed) /* speed is N=2 */ motor_on_speed=FALSE;
		 else motor_started=FALSE;
	 }
      }
   }
   else
   {  /* monitor extra stop delay after motor is stopped signal */
      if (servo_timer==0)
      {
	 if (reload_servo_timer)
	 {
	    servo_timer=EXTRA_STOP_DELAY;
	    reload_servo_timer=FALSE;
	 }
	 else
	 {
	    servo_state=INIT_SLEDGE;  /* select next state */
	    cd6_wr(SPEED_CONTROL_N1); /* switch all cd6 settings to n=1 */
	    n1_speed=TRUE;
	    set_motor_gain();
	    initialized=FALSE;
	    motor_on_speed=FALSE;
	    if (servo_requested_state==STOP_SERVO)
	    {  /* STOP UPON EXTERNAL STOP REQUEST */
	       servo_exec_state=READY; /* servo function ended without error */
	    }
	 }
      }
   }
}


/***********************************************************************************************
 * Function  : jump_servo_state
 * Input     : -
 * Output    : -
 * Abstract  : jump request for servo convert into long/short jump command
 *             next state will be check jump
 * Decisions :
*/
void jump_servo_state(void)
{
   signed char		brake_dist;

   kick=FALSE;		
   brk=FALSE;

   off_track_value = abs(grooves.int_i);
   if (off_track_value < (unsigned short)(MAX))
   {
      /* SHORT JUMP ONLY WITH ACTUATOR */
      jump_short();
   }
   else
   {
      if (off_track_value <= (unsigned short)(BRAKE_2_DIS_MAX))
      {
	 /* "SHORT BRAKE" DISTANCE JUMP */
	 /* BRAKE DISTANCE = GROOVES DIV 2 */
	 brake_dist=(-((signed char)((off_track_value+16)>>5)))-1;
	 cd6_wr(MOT_OFF_ACTIVE);
      }
      else
      {
	 /* "FULL BRAKE" DISTANCE JUMP */
	 brake_dist=BRAKE_DIS_MAX / -16;
	 if (grooves.int_i > 0)
	 {
	    /* JUMP INSIDE: KICK*/
	    kick_brake_timer=calc_kick();
	    cd6_wr(MOT_STRTM1_ACTIVE);
	    kick=TRUE;
	 }
	 else
	 {
	    /*JUMP OUTSIDE: BRAKE*/
	    brk=TRUE;
	    kick_brake_timer=calc_brake();
	    cd6_wr(MOT_BRM2_ACTIVE);
	 }
      }
      /* PREPARE DSIC2 FOR LONG JUMP */
      jump_long(brake_dist);
   }
   servo_state=CHECK_JUMP;
   servo_timer=SKATING_DELAY_CHECK; /* start delay for sampling offtrack counter dsic2 */
}


/***********************************************************************************************
 * Function  : check_jump_state
 * Input     : -
 * Output    : -
 * Abstract  : checks if dsic2 has regained radial lock after a jump.
	       if not, checks if the offtrack counter has decreased after every sample moment.
	       if not, then skating.
 * Decisions :
*/
static void check_jump_state(void)
{
unsigned short	dsic_offtrack_value;

   if (dsic_in_focus())
   {
      if (hf_present())
      {
	 /* CHECK RADIAL STATE */
	 if ( (kick || brk) && (kick_brake_timer == 0) )
	 {
	    /* STOP KICKING OR BRAKING*/
	    cd6_wr(MOT_OFF_ACTIVE);
	    kick=FALSE;
	    brk=FALSE;
	 }
	 if (dsic_on_track())
	 {
	    /* RADIAL LOCK: TRY TO READ SUBCODE STATUS */
	    cd6_wr(MOT_PLAYM_ACTIVE);
	    start_subcode_reading();
	    servo_state=WAIT_SUBCODE;
	    servo_timer=SUBCODE_TIME_OUT;
	 }
	 else
	 {
	    /* NOT YET RADIAL LOCK */
	    if (servo_timer==0)
	    {
	       /* CHECK IF SKATING OCCURED, CAUSED BY RADIAL PROBLEMS */
	       /* PREVIOUS OFF TRACK VALUE SAVED IN off_track_value */
	       /* READ NEW OFF TRACK VALUE */
	       dsic_offtrack_value=abs(read_off_track_value());
	       if (dsic_offtrack_value <= off_track_value)
	       {
		  if (off_track_value-dsic_offtrack_value < 10)
		  {
		     /* LESS THAN 10 GROOVES JUMPED OVER THE LAST 24 MSEC */
		     servo_state=RADIAL_RECOVER;
		     rad_recover_in_jump=TRUE;
		  }
		  else
		  {
		     off_track_value=dsic_offtrack_value;
		  }
	       }
	       else if (dsic_offtrack_value > 200)
	       {
		  /* CURRENT OFFTRACK VALUE BIGGER THEN PREVIOUS OFFTRACK VALUE AND STILL A LOT TO JUMP */
		  servo_state=RADIAL_RECOVER;
		  rad_recover_in_jump=TRUE;
	       }
	       servo_timer=SKATING_SAMPLE_TIME;
	    }
	 }
      }
      else
      {
	 no_efm_in_jump=TRUE;
	 servo_state=FOCUS_RECOVER; /* this pulls the sledge in */
	 kick=FALSE;
	 brk=FALSE;
      }
   }
   else
   {
      foc_recover_in_jump=TRUE;
      servo_state=FOCUS_RECOVER;
      kick=FALSE;
      brk=FALSE;
   }
}


/***********************************************************************************************
 * Function  : wait_subcode_state
 * Input     : -
 * Output    : -
 * Abstract  : wait subcode_time for new subcode after jump
 * Decisions :
*/
static void wait_subcode_state(void)
{
   /* WAIT TILL SUBCODE READY OR TIME OUT!!! */
   if (!status_cd6(SUBCODE_READY))
   {
      /* SUBODE READY ! */
      servo_state=SERVO_MONITOR;	/* select next state */
      servo_timer=SUBCODE_MONITOR_TIMEOUT;
   }
   else if (servo_timer==0)
   {
      /* RADIAL ERROR: NO SUBCODE FOUND TRY TO RECOVER */
      servo_state=RADIAL_RECOVER;
      rad_recover_in_jump=TRUE;
   }
}


/*======================INTERFACE FUNCTION DEFINITIONS=====================*/
/***********************************************************************************************
 * Function  : servo_init
 * Input     : -
 * Output    : -
 * Abstract  : initialize state machine
 * Decisions :
*/
void servo_init(void)
{
   servo_requested_state=SERVO_IDLE;	/* state at end of servo init function*/
   servo_state=INIT_DSIC2;              /* select next state*/
   servo_exec_state=BUSY;               /* servo function in progress*/
   n1_speed=TRUE;
   disc_size = TWELVE;
#if N == 1
   n2_speed_req=FALSE;			/* default setting n=1 */
#else
   n2_speed_req=TRUE;			/* default setting n=2 */
#endif
}


/***********************************************************************************************
 * Function  : servo_start
 * Input     : -
 * Output    : -
 * Abstract  : switches requested state to start focus
 * Decisions : -
*/
void servo_start(void)
{
   servo_requested_state=START_FOCUS;
   servo_exec_state=BUSY;
   rad_recover_in_jump=FALSE;
   foc_recover_in_jump=FALSE;
   no_efm_in_jump=FALSE;
}


/***********************************************************************************************
 * Function  : servo_reinit_sledge
 * Input     : -
 * Output    : -
 * Abstract  : moves sledge to the home switch when service mode is left
 * Decisions : -
*/
void servo_reinit_sledge(void)
{
   servo_requested_state=SERVO_IDLE;
   servo_state=INIT_SLEDGE;
   servo_exec_state=BUSY;
}


/***********************************************************************************************
 * Function  : servo_n1
 * Input     : -
 * Output    : -
 * Abstract  : changes disc speed to single speed, in stop mode only clears n2_speed_req
 * Decisions :
*/
void servo_n1(void)
{
   n2_speed_req=FALSE;
   servo_exec_state=BUSY;
}


/***********************************************************************************************
 * Function  : servo_n2
 * Input     : -
 * Output    : -
 * Abstract  : changes disc speed to double speed, in stop mode only sets n2_speed_req
 * Decisions :
*/
void servo_n2(void)
{
   n2_speed_req=TRUE;
   servo_exec_state=BUSY;
}


/***********************************************************************************************
 * Function  : servo_stop
 * Input     : -
 * Output    : -
 * Abstract  : switches the requested servo state to servo idle
 * Decisions :
*/
void servo_stop(void)
{
   if (servo_requested_state!=STOP_SERVO && servo_requested_state!=SERVO_IDLE)
   {
      servo_requested_state=STOP_SERVO;
      servo_state=STOP_SERVO;
   }
   servo_exec_state=BUSY;
}


/***********************************************************************************************
 * Function  : servo_jump
 * Input     : number_of_grooves
 * Output    : -
 * Abstract  : initiates jump action
 * Decisions :
*/
void servo_jump(int jump_size)
{
   grooves.int_i=jump_size;
   grooves.int_i=-jump_size;
   servo_retries=MAX_RETRIES;
   servo_requested_state=JUMP_SERVO;    /* requested state: switched to in servo monitor*/
   servo_exec_state=BUSY;		/* servo function in progress*/
   rad_recover_in_jump=FALSE;
   foc_recover_in_jump=FALSE;
   no_efm_in_jump=FALSE;
}


/***********************************************************************************************
 * Function  : get_servo_process_state
 * Input     : -
 * Output    : servo_exec_state
 * Abstract  : returns the current servo_exec_state (busy, ready, error)
 * Decisions : -
*/
byte get_servo_process_state(void)
{
   return servo_exec_state;
}


/***********************************************************************************************
 * Function  : get_jump_status
 * Input     : -
 * Output    : bit0=1 -> no_hf_on_target
 *             bit1=1 -> radial_recovery_done
 *             bit2=1 -> focus_recovery_done
 * Abstract  : returns the current jump status
 *             when a jump is ready this function reports if during the
 *	       jump a recovery toke place; important for the scan jump
 *	       and during toc reading of a multi-session disc
 * Decisions : -
*/
byte get_jump_status(void)
{
byte	status;

   status=0;
   if (no_efm_in_jump) status |= NO_HF_ON_TARGET;
   if (rad_recover_in_jump) status |= RADIAL_RECOVERY_DONE;
   if (foc_recover_in_jump) status |= FOCUS_RECOVERY_DONE;
   return status;
}


/***********************************************************************************************
 * Function  : servo_tracking
 * Input     : -
 * Output    : CY=1 -> servo is in tracking mode
	       CY=0 -> servo is not in tracking mode
 * Abstract  : returns true if servo is in tracking mode
 *             used for deciding if play commands PAUSE_ON, PAUSE_OFF or
 *             PLAY_READ_SUBCODE are allowed
 * Decisions :
*/
bit servo_tracking(void)
{
   if (servo_requested_state==SERVO_MONITOR || servo_requested_state==JUMP_SERVO)
      return TRUE;
   else
      return FALSE;
}


/***********************************************************************************************
 * Function  : servo_to_service
 * Input     : -
 * Output    : CY=1 -> allowed to put servo module in service mode (not called anymore)
	       CY=0 -> not allowed
 * Abstract  : returns true if allowed to put servo module in service mode
 * Decisions :
*/
bit servo_to_service(void)
{
   if (servo_state==SERVO_IDLE || servo_state==SERVO_MONITOR) return TRUE;
   else return FALSE;
}

/*======================END OF INTERFACE FUNCTIONS=========================*/


/***********************************************************************************************
 * Function  : servo
 * Input     : -
 * Output    : -
 * Abstract  : Selects function that corresponds with actual state of state machine
 * Decisions :
*/
static void (* const servo_functions_array[])() =
   {
      init_dsic2_state,
      init_sledge_state,
      check_in_sledge_state,
      check_out_sledge_state,
      servo_idle_state,
      start_focus_state,
      check_focus_state,
      double_check_focus_state,
      start_ttm_state,
      ttm_speedup_state,
      check_ttm_state,
      start_radial_state,
      init_radial_state,
      servo_monitor_state,
      radial_recover_state,
      focus_recover_state,
      sledge_inside_recover_state,
      sledge_outside_recover_state,
      stop_servo_state,
      check_stop_servo_state,
      jump_servo_state,
      check_jump_state,
      wait_subcode_state,
      upto_n2_state,
      downto_n1_state
   };


void servo(void)
{
   (*servo_functions_array[servo_state])();
}
