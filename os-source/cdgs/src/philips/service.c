/***************************************************************************
*
* Project:	BASIC PLAYER MODULE
* File:		service.c
* Date:		06 July 1993
* Version:	0.1
* Status:	Draft
*
* Description:	this module executes all service commands.
*               if an error occurs during service mode then this is
*               handled also in this module, without interference of the
*		top level (only error is reported).
*
* Interface functions: byte service(byte)
*                      void execute_service_functions(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 26 May 1993		creation P.vd.Z.
* 27 May 1993		error recovery for service errors handled locally.
*			re-init sledge when entering normal mode again.
*			function "monitor_focussing" added in focus-on sequence.
* 04 June 1993		for WEARNES application redefinition of
*			HALF_SERVICE_JUMP_TIME to 1 sec.
*			new spindle motor flag "motor_on_speed" and flag
*			"motor_started" gets new function.
* 11 June 1993		functions: rad_hold + sledge_off = turn_radial_off.
*               	functions: switch_focus_off + switch_laser_off = turn_focus_laser_off.
*               	functions: switch_laser_on + switch_focus_on = turn_laser_focus_on.
*			functions are located in servo.c and all return ready.
*
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

#define HALF_SERVICE_JUMP_TIME	       250   /* max time for jump tracks is 4 sec */
#define RAD_ON_TIMEOUT		       500/8 /* wait max 500 msec for finding radial lock at "radial_on" */

extern bit status_cd6(byte);
extern void cd6_wr(byte);
extern void wr_dsic2(byte);
extern byte rd_dsic2(void);
extern byte mute_on(void);
extern void jump_servo_state(void);
extern bit dsic_on_track(void);
extern bit dsic_in_focus(void);
extern void servo_reinit_sledge(void);
extern void rad_hold(void);
extern void rad_start(void);
extern void sledge_off(void);
extern bit sledge_switch(void);
extern byte switch_laser_on(void);
extern byte switch_focus_off(void);
extern byte turn_radial_off(void);
extern byte turn_focus_laser_off(void);
extern byte turn_laser_focus_on(void);

extern struct interface_field	player_interface;
extern int_hl  	    grooves;
extern byte			servo_timer;
extern byte			player_error;
extern bit			motor_started;
extern bit			motor_on_speed;
extern bit 			initialized;
extern bit			reload_servo_timer;

static bit			service_active;
static bit              	service_phase1,service_phase0;
bit				service_command_busy;

static byte		service_process; /* high nibble service_process_id,
					    low nible service_function_id */
static byte		service_status;	 /* BUSY=0x00, READY=0x01, ERROR=0x02 */



/***********************************************************************************************
 * Function  : write_cd6
 * Input     : player_interface.param1
 * Output    : READY
 * Abstract  : send player_interface.param1 to CD6
 * Decisions :
*/
static byte write_cd6(void)
{
   cd6_wr(player_interface.param1);
   return READY;
}


/***********************************************************************************************
 * Function  : write_dsic2
 * Input     : player_interface.param1
 * Output    : READY
 * Abstract  : send player_interface.param1 to DSIC2
 * Decisions :
*/
static byte write_dsic2(void)
{
   wr_dsic2(player_interface.param1);
   return READY;
}


/***********************************************************************************************
 * Function  : read_dsic2
 * Input     : -
 * Output    : READY
 * Abstract  : read one DSIC2 byte store it in player_interface.param1
 * Decisions :
*/
static byte read_dsic2(void)
{
   player_interface.param1=rd_dsic2();
   return READY;
}


/***********************************************************************************************
 * Function  : radial_on
 * Input     : -
 * Output    : BUSY / READY / ERROR
 * Abstract  : do an initalization then jump 10 tracks to gain radial lock
 * Decisions :
*/
static byte radial_on(void)
{
   if (!service_phase1 && !service_phase0)
   {  /* SERVICE PHASE 0 */
      turn_radial_off();
      if (!initialized)
      {
	 servo_timer=RAD_INITIALIZE_TIME;
	 rad_start();
      }
      else
	 /* NO INITIALIZING NEEDED */
	 servo_timer=0;
      service_phase0=1;	/* goto phase 1 */
   }

   if (!service_phase1 && service_phase0)
   {  /* SERVICE PHASE 1 */
      if (servo_timer == 0)
      {
	 /* INITIALIZING READY: JUMP 10 TRACKS */
	 rad_hold();
	 initialized=TRUE;
	 wr_dsic2(SRCOMM3);
	 wr_dsic2(0xFF);
	 wr_dsic2(0xF6);
	 wr_dsic2(RAD_STAT3);
	 servo_timer=RAD_ON_TIMEOUT;
	 service_phase1=1; service_phase0=0; /* goto phase 2 */
      }
   }

   if (service_phase1 && !service_phase0)
   {  /* SERVICE PHASE 2 */
      if (servo_timer == 0)
      {
	 /* NOT IN LOCK ON TIME!!! */
	 player_error=RADIAL_ERROR;
	 return ERROR;
      }
      if ( dsic_on_track() )
      {
	 cd6_wr(MOT_PLAYM_ACTIVE);
	 return READY;
      }
   }
   return BUSY;
}


/***********************************************************************************************
 * Function  : start_motor
 * Input     : -
 * Output    : BUSY / READY / ERROR
 * Abstract  : check if in FOCUS then go from start mode 1 to start mode 2
 * Decisions :
*/
static byte start_motor(void)
{
   if (motor_on_speed)
      return READY;
   else
   {
      /* MOTOR SPEED NOT YET OK */
      if (!service_phase1 && !service_phase0)
      {  /* SERVICE PHASE 0 */
	 servo_timer=FOCUS_TIME_OUT;
	 service_phase0=1; /* goto phase 1 */
      }

      if (!service_phase1 && service_phase0)
      {  /* SERVICE PHASE 1 */
	 if (servo_timer == 0)
	 {
	    /* NOT IN FOCUS */
	    player_error=FOCUS_ERROR;
	    return ERROR;
	 }
	 if ( dsic_in_focus() )
	 { 
	    /* FOCUS FOUND */
	    servo_timer=SPEEDUP_TIME;
	    cd6_wr(MOT_STRTM1_ACTIVE);
	    motor_started=TRUE;
	    service_phase1=1; service_phase0=0;	/* goto phase 2 */
	 }
      }

      if (service_phase1 && !service_phase0)
      {  /* SERVICE PHASE 2 */
	 if (servo_timer == 0)
	 {
	    /* MOTOR SPEED > 10% OF NOM. SPEED GOTO START MODE 2 */
	    servo_timer=NOMINAL_SPEED_TIME;
	    cd6_wr(MOT_STRTM2_ACTIVE);
	    service_phase0=1; /* goto phase 3 */
	 }
      }

      if (service_phase1 && service_phase0)
      {  /* SERVICE PHASE 3 */
	 if (servo_timer == 0)
	 {
	    /* MOTOR SPEED < 75% OF NOM. SPEED MOTOR ERROR */
	    player_error=MOTOR_ERROR;
	    return ERROR;
	 }
	 if ( status_cd6(MOT_STRT_1) )
	 {
	    /* SPEED > 75 %*/
	    motor_on_speed=TRUE;
	    return READY;
	 }
      }
      return BUSY;
   }
}


/***********************************************************************************************
 * Function  : stop_motor
 * Input     : -
 * Output    : READY
 * Abstract  : send stop motor to CD6
 * Decisions :
*/
static byte stop_motor(void)
{
   cd6_wr(MOT_OFF_ACTIVE);
   motor_started=FALSE;
   motor_on_speed=FALSE;
   initialized=FALSE;
   return READY;
}


/***********************************************************************************************
 * Function  : set_normal_mode
 * Input     : -
 * Output    : PROCESS_READY
 * Abstract  : re-init servo sledge (and enable normal mode commands: done at top level)
 * Decisions :
*/
static byte set_normal_mode(void)
{
   servo_reinit_sledge();
   return PROCESS_READY;
}


/***********************************************************************************************
 * Function  : monitor_focussing
 * Input     : -
 * Output    : BUSY / READY
 * Abstract  : wait indefinitely until focus is found
 * Decisions :
*/
static byte monitor_focussing(void)
{
   if ( dsic_in_focus() ) return READY;
   else return BUSY;
}


/***********************************************************************************************
 * Function  : ready_to_jump
 * Input     : -
 * Output    : yes / no
 * Abstract  : checks if a jump can be executed
 * Decisions : if answer is no, player_error is updated.
*/
static bit ready_to_jump(void)
{
   if ( !dsic_in_focus() )
   {
      player_error=FOCUS_ERROR;
      return FALSE;
   }
   /* IN FOCUS */
   if ( !status_cd6(MOT_STRT_1) )
   {
      player_error=MOTOR_ERROR;
      return FALSE;
   }
   /* IN FOCUS & MOTOR OK */
   if ( !dsic_on_track() )
   {
      player_error=RADIAL_ERROR;
      return FALSE;
   }
   /* IN FOCUS & MOTOR OK & RADIAL_OK */
   return TRUE;
}


/***********************************************************************************************
 * Function  : service_process_ready
 * Input     : -
 * Output    : PROCES_READY
 * Abstract  : end of process indicator
 * Decisions :
*/
static byte service_process_ready(void)
{
   return PROCESS_READY;
}


/***********************************************************************************************
 * Function  : move_sledge
 * Input     : player_interface.param1: power; player_interface.param2: time
 * Output    : BUSY / READY
 * Abstract  : move sledge with power during time
 * Decisions :
*/
static byte move_sledge(void)
{
   if (!service_phase0)
   {  /* SERVICE PHASE 0 */
      servo_timer=player_interface.param2;
      wr_dsic2(SRSLEDGE);
      wr_dsic2(player_interface.param1);
      service_phase0=1;
   }
   if (servo_timer==0 || ((bit)sledge_switch()==(bit)CLOSED && (player_interface.param1&0x80)))
   {  /* TIME OUT OR SLEDGE STEERED BEYOND HOME POSITION */
      sledge_off();
      return READY;
   }
   return BUSY;
}


/***********************************************************************************************
 * Function  : service_jump
 * Input     : player_interface.param1: tracks MSB; player_interface.param2: tracks LSB
 * Output    : BUSY / READY / ERROR
 * Abstract  : jump number of grooves as indicated by player_interface param1 and 2
 * Decisions :
*/
static byte service_jump(void)
{
   if (!service_phase0)
   {  /* SERVICE PHASE 0 */
      if (ready_to_jump())
      {
	 /* RADIAL OK!! */
	 grooves.byte_hl.high=player_interface.param1;
	 grooves.byte_hl.low=player_interface.param2;
	 grooves.int_i=-grooves.int_i;

	 /* CALCULATION OF DSIC2 COEFF IS DONE IN SERVO ROUTINE */
	 jump_servo_state();

	 service_phase0=1; /* goto phase 1 */
	 servo_timer=HALF_SERVICE_JUMP_TIME;
	 reload_servo_timer=TRUE;
	 return BUSY;
      }
      else
	 return ERROR; /* player_error has been updated by ready to jump */
   }

   /* MONITOR JUMP */
   if ( dsic_on_track() )
   {
      cd6_wr(MOT_PLAYM_ACTIVE);
      return READY;
   }
   if (servo_timer == 0)
   {
      if (reload_servo_timer)
      {
	 servo_timer=HALF_SERVICE_JUMP_TIME;
	 reload_servo_timer=FALSE;
	 return BUSY;
      }
      /* TIME OUT: SHUT OFF SERVO HANDLED IN ERROR RECOVERY */
      player_error=RADIAL_ERROR;
      return ERROR;
   }
}


static byte (* const service_functions[][5])(void) =
{
/* ENTER_NORMAL_OPC 0 */  {mute_on,/*seven_nop,*/turn_radial_off,stop_motor,turn_focus_laser_off,set_normal_mode},
/* LASER_ON_OPC     1 */  {switch_laser_on,service_process_ready},
/* LASER_OFF_OPC    2 */  {turn_radial_off,stop_motor,turn_focus_laser_off,service_process_ready},
/* FOCUS_ON_OPC     3 */  {turn_laser_focus_on,monitor_focussing,service_process_ready},
/* FOCUS_OFF_OPC    4 */  {turn_radial_off,stop_motor,switch_focus_off,service_process_ready},
/* MOTOR_ON_OPC     5 */  {turn_laser_focus_on,start_motor,service_process_ready},
/* MOTOR_OFF_OPC    6 */  {turn_radial_off,stop_motor,service_process_ready},
/* RADIAL_ON_OPC    7 */  {turn_laser_focus_on,start_motor,radial_on,service_process_ready},
/* RADIAL_OFF_OPC   8 */  {turn_radial_off,service_process_ready},
/* MOVE_SLEDGE_OPC  9 */  {turn_radial_off,move_sledge,service_process_ready},
/* JUMP_GROOVES_OPC 10 */ {service_jump,service_process_ready},
/* WRITE_CD6_OPC    11 */ {write_cd6,service_process_ready},
/* WRITE_DSIC2_OPC  12 */ {write_dsic2,service_process_ready},
/* READ_DSIC2_OPC   13 */ {read_dsic2,service_process_ready}
};


/**************************************************************************/
byte service(byte service_cmd)
{
   if (service_process>>4 != service_cmd || !service_command_busy)
   {
      service_process=service_cmd<<4; /* also sets service_function_id = 0 */
      service_phase1=0; service_phase0=0;
      service_active=TRUE;
      service_status=BUSY; service_command_busy=TRUE;
   }
   if (service_status!=BUSY) service_command_busy=FALSE;
   return service_status;
}


/**************************************************************************/
void execute_service_functions(void)
{
   if (service_active)
   {
      switch ((*service_functions[service_process>>4][service_process & 0x0F])())
      {
      case PROCESS_READY: service_status=PROCESS_READY;
			  service_active=FALSE;
			  break;
      case READY:         service_process++; /* service_function_id = service_function_id + 1 */
			  service_phase1=0; service_phase0=0;
			  break;
      case ERROR:         service_status=ERROR;
			  service_active=FALSE;
			  if (player_error == RADIAL_ERROR)
			  {
			     turn_radial_off();
			  }
			  else if (player_error == MOTOR_ERROR)
			  {
			     turn_radial_off(); stop_motor();
			  }
			  else if (player_error == FOCUS_ERROR)
			  {
			     turn_radial_off(); stop_motor(); switch_focus_off();
			  }
			  break;
      }
   }
}
