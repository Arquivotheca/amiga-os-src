/***************************************************************************
*
* Project:      Commodore
* File:         player.c
* Date:         16 July 1993
* Version:      0.1
* Status:       Draft
*
* Description:  this module is the top level module for the basic player module
*               it does the basic player module initialisations
*               it contains a sequencer for controlling the servo's in the correct order
*               it converts an incoming command into a process_id (input for the sequencer)
*               it controls the software interface protocol
*               if an error is detected then this module decides how to handle it
*
* Interface functions: void player_init(void)
*                      void player(void)
*
* Decisions:    -
*
* HISTORY               AUTHOR COMMENTS
* 08 December 1992      creation H.v.K.
* 19 July               made adaptions for Commodore
***************************************************************************/
#include "defs.h"


/* externals: */
extern void reset_dsic2_cd6(void);
extern void servo_init(void);
extern void cd6_init(void);
extern void timer_init(void);
extern void init_for_new_disc(void);
extern void execute_start_stop_functions(void);
extern void execute_play_functions(void);
extern void execute_service_functions(void);
extern void subcode_module(void);
extern void servo(void);
extern bit  servo_to_service(void);
extern byte start_stop(byte);
extern byte play(byte);
extern byte service(byte);
extern void shock_recover(void);

extern bit              play_monitor;
extern bit              play_command_busy;
extern bit              start_stop_command_busy;
extern bit              service_command_busy;

struct interface_field  player_interface;
byte                    player_error;
byte                    process_id;
byte                    function_id;
static bit              service_mode;


/**************************************************************************/
void player_init(void)
{
   timer_init();
   reset_dsic2_cd6();
   servo_init();
   cd6_init();

   player_interface.a_command = IDLE_OPC;
   player_interface.p_status = READY;
   process_id = IDLE_OPC;

   init_for_new_disc();
}


/**************************************************************************/
static bit opcode_allowed(void)
{
   if (player_interface.a_command == READ_SUBCODE_OPC && process_id < 5)
      return FALSE;
   else
      return TRUE;
}


/**************************************************************************/
static byte player_module(byte cmd)
{
   if (cmd == PLAYER_IDLE) return PROCESS_READY;
   else if (cmd == SET_SERVICE_MODE)
   {
      if ( servo_to_service() )
      {  /* allowed to put servo module in service mode */
         service_mode = TRUE;
         return PROCESS_READY;
      }
      player_error = ILLEGAL_COMMAND;
      return ERROR;
   }
   else
   {  /* cmd = PLAYER_HANDLE_ERROR */
      if ( player_error==ILLEGAL_COMMAND ||
           player_error==ILLEGAL_PARAMETER ||
           player_error==SUBCODE_NOT_FOUND ||
           player_error==JUMP_ERROR)
      {  /* do not stop the disc for these errors !! */
         return PROCESS_READY;
      }
   }
   return READY; /* continue error handling sequence */
}


/**************************************************************************/
static const struct {
        byte    (*f)();
        byte    parm;
                    } processes[][5] = {
/* ERROR_HANDLING_ID -> process_id = 00 */
{player_module,PLAYER_HANDLE_ERROR, play,PLAY_IDLE, start_stop,SS_MOTOR_OFF, player_module,PLAYER_IDLE, 0,0},

/* TRAY_OUT_OPC = 00 -> process_id = 01 */
{0,0, 0,0, 0,0, 0,0, 0,0},

/* TRAY_IN_OPC = 01 -> process_id = 02 */
{0,0, 0,0, 0,0, 0,0, 0,0},

/* START_UP_OPC = 02 -> process_id = 03 */
{0,0, 0,0, 0,0, 0,0, 0,0},

/* STOP_OPC = 03 -> process_id = 04 */
{play,PLAY_IDLE, start_stop,SS_STOP, player_module,PLAYER_IDLE, 0,0, 0,0},

/* PLAY_TRACK_OPC = 04 -> process_id = 05 */
{0,0, 0,0, 0,0, 0,0, 0,0},

/* PAUSE_ON_OPC = 05 -> process_id = 06 */
{start_stop,SS_IDLE, play,PAUSE_ON, player_module,PLAYER_IDLE, 0,0, 0,0},

/* PAUSE_OFF_OPC = 06 -> process_id = 07 */
{start_stop,SS_IDLE, play,PAUSE_OFF, player_module,PLAYER_IDLE, 0,0, 0,0},

/* SEEK_OPC = 07 -> process_id = 08 */
{play,PLAY_IDLE, start_stop,SS_START_UP, play,JUMP_TO_ADDRESS, player_module,PLAYER_IDLE, 0,0},

/* READ_TOC_OPC = 08 -> process_id = 09 */
{play,PLAY_IDLE, start_stop,SS_START_UP, play,PLAY_READ_TOC, player_module,PLAYER_IDLE, 0,0},

/* READ_SUBCODE_OPC = 09 -> process_id = 10 */
{play,PLAY_READ_SUBCODE, 0,0, 0,0, 0,0, 0,0},

/* SINGLE_SPEED_OPC = 10 -> process_id = 11 */
{start_stop,SS_IDLE, play,PLAY_PREPARE_SPEED_CHANGE, start_stop,SS_SPEED_N1, play,PLAY_RESTORE_SPEED_CHANGE, player_module,PLAYER_IDLE},

/* DOUBLE_SPEED_OPC = 11 -> process_id = 12 */
{start_stop,SS_IDLE, play,PLAY_PREPARE_SPEED_CHANGE, start_stop,SS_SPEED_N2, play,PLAY_RESTORE_SPEED_CHANGE, player_module,PLAYER_IDLE},

/* SET_VOLUME_OPC = 12 -> process_id = 13 */
{0,0, 0,0, 0,0, 0,0, 0,0},

/* JUMP_TRACKS_OPC = 13 -> process_id = 14 */
{start_stop,SS_IDLE, play,PLAY_JUMP_TRACKS, player_module,PLAYER_IDLE, 0,0, 0,0},

/* ENTER_SERVICE_MODE_OPC = 14 -> process_id = 15 */
{play,PLAY_IDLE, start_stop,SS_IDLE, player_module,SET_SERVICE_MODE, 0,0, 0,0}
                                       };
/* NOTE: list is filled up with 0's because of compiler error
         length (=5) should be the same as processes_length (see equate below) !!! */


/**************************************************************************/
#pragma asm
processes_length        equ     5       ;same as list length above !!!
EXEC_FUNK       SEGMENT CODE
        RSEG    EXEC_FUNK
func_execute:
        PUSH    ACC
        PUSH    B
        RET                             ;jump to the requested function (parameter already set)
EXTRN   DATA(_start_stop_BYTE, _play_BYTE, _service_BYTE)
EXTRN   CODE(_service)
#pragma endasm

static byte call_function(void)
{  /* written in assembly because the compiler couldn't handle this in the small model */
#pragma asm
        JNB     _service_mode,normal_opcode
service_opcode:
        MOV     A,_process_id
        ADD     A,#0F0H                 ;ACC = _process_id-16
        MOV     _service_BYTE,A
        MOV     R0,#HIGH(_service)      ;R0 is service address high
        MOV     R1,#LOW(_service)       ;R1 is service address low
        SJMP    do_funk
normal_opcode:
        MOV     DPTR,#_processes        ;set pointer to processes list
        MOV     B,#3*processes_length
        MOV     A,_process_id
        MUL     AB                      ;B=high, A=low
        ADD     A,DPL
        MOV     DPL,A
        MOV     A,B
        ADDC    A,DPH
        MOV     DPH,A                   ;DPTR points to first function of process_id
        MOV     B,#3
        MOV     A,_function_id
        MUL     AB                      ;B=high, A=low
        ADD     A,DPL
        MOV     DPL,A
        MOV     A,B
        ADDC    A,DPH
        MOV     DPH,A                   ;DPTR points to requested function
        CLR     A
        MOVC    A,@A+DPTR
        MOV     R0,A                    ;R0 is function address high
        MOV     A,#1
        MOVC    A,@A+DPTR
        MOV     R1,A                    ;R1 is function address low
        MOV     A,#2
        MOVC    A,@A+DPTR
        MOV     R2,A                    ;R2 is function parameter
; determine to which function the parameter should be passed (_start_stop, _play, _player_module)
funk1:
        CJNE    R0,#HIGH(_start_stop),funk2
        CJNE    R1,#LOW(_start_stop),funk2
        MOV     _start_stop_BYTE,R2     ;function is _start_stop
        SJMP    do_funk
funk2:
        CJNE    R0,#HIGH(_play),funk3
        CJNE    R1,#LOW(_play),funk3
        MOV     _play_BYTE,R2           ;function is _play
        SJMP    do_funk
funk3:
        CJNE    R0,#HIGH(_player_module),hang_up
        CJNE    R1,#LOW(_player_module),hang_up
        MOV     _player_module_BYTE,R2  ;function is _player_module
        SJMP    do_funk
hang_up:
        SJMP    hang_up                 ;illegal function in list
do_funk:
        MOV     B,R0                    ;store high address in B
        MOV     A,R1                    ;store low address in ACC
        LCALL   func_execute
#pragma endasm
}


/**************************************************************************/
void player(void)
{
   if (player_interface.a_command != IDLE_OPC)
   {
      process_id = player_interface.a_command+1;
      player_interface.a_command = IDLE_OPC;
      player_interface.p_status = BUSY;
      function_id = 0;
      play_monitor = TRUE; /* stop all play module actions */
      play_command_busy = FALSE;
      start_stop_command_busy = FALSE;
      service_command_busy = FALSE;

      if (process_id > MAX_LEGAL_NORMAL_ID)
      {  /* service command */
         if (process_id > MAX_LEGAL_SERVICE_ID)
         {  /* illegal opcode */
            player_interface.p_status = ERROR;
            player_interface.param1 = ILLEGAL_COMMAND;
            process_id = IDLE_OPC;
          }
          else if (!service_mode)
          {
             player_error = ILLEGAL_COMMAND;
             process_id = ERROR_HANDLING_ID;
          }
       }
       else
       {  /* normal command */
          if (service_mode)
          {
             player_interface.p_status = ERROR;
             player_interface.param1 = ILLEGAL_COMMAND;
             process_id = IDLE_OPC;
          }
       }
   }

   if (process_id != IDLE_OPC)
   {
      /* switch ((*processes[process_id][function_id].f)(processes[process_id][function_id].parm)) */
      switch (call_function())
      {
      case PROCESS_READY: if (process_id == (byte)(ENTER_NORMAL_MODE_OPC+1))
                             service_mode = FALSE;
                          if (process_id != ERROR_HANDLING_ID)
                             player_interface.p_status = READY;
                          else
                          {
                             player_interface.p_status = ERROR;
                             player_interface.param1 = player_error;
                          }
                          player_error = NO_ERROR;
                          process_id = IDLE_OPC;
                          break;

      case READY:         if (process_id == 10)
                          {
                             player_interface.p_status = READY;
                             player_error = NO_ERROR;
                             process_id = IDLE_OPC;
                          }
                          else  
                              function_id++;
                          break;

      case ERROR:         if (service_mode)
                          {
                             player_interface.p_status = ERROR;
                             player_interface.param1 = player_error;
                             player_error = NO_ERROR;
                             process_id = IDLE_OPC;
                          }
                          else
                          {
                             process_id = ERROR_HANDLING_ID;
                             function_id = 0;
                          }
                          break;
      }
   }

   if (!service_mode)
   {
      execute_start_stop_functions();
      execute_play_functions();

      /* don't put any routines between servo() and subcode_module() which
         need the actual subcode, like execute_play_functions() !!!!!!!!!! */

      servo();
      subcode_module();
      shock_recover();
   }
   else
      execute_service_functions();
}
