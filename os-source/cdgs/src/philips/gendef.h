/***************************************************************************
*
* Project:      Commodore
* File:         gendef.h
* Date:         12 March 1993
* Status:       Draft
*
* Description:  contains all generally used definitions
*
* Decisions:    -
*
* HISTORY               AUTHOR COMMENTS
* 16 November 1992      creation H.v.K.
*
***************************************************************************/
#define const                          rom
        
typedef unsigned char                  byte;
typedef unsigned char                  Byte;

struct interface_field {
        byte    p_status;
        byte    a_command;
        byte    param1;
        byte    param2;
        byte    param3;
                       };

#define TRAY_OUT_OPC                    0x00
#define TRAY_IN_OPC                     0x01
#define START_UP_OPC                    0x02
#define STOP_OPC                        0x03
#define PLAY_TRACK_OPC                  0x04
#define PAUSE_ON_OPC                    0x05
#define PAUSE_OFF_OPC                   0x06
#define SEEK_OPC                        0x07
#define READ_TOC_OPC                    0x08
#define READ_SUBCODE_OPC                0x09
#define SINGLE_SPEED_OPC                0x0A
#define DOUBLE_SPEED_OPC                0x0B
#define SET_VOLUME_OPC                  0x0C
#define JUMP_TRACKS_OPC                 0x0D
#define ENTER_SERVICE_MODE_OPC          0x0E

/* ==== service mode opcodes ==== */
#define ENTER_NORMAL_MODE_OPC           0x0F
#define LASER_ON_OPC                    0x10
#define LASER_OFF_OPC                   0x11
#define FOCUS_ON_OPC                    0x12
#define FOCUS_OFF_OPC                   0x13
#define SPINDLE_MOTOR_ON_OPC            0x14
#define SPINDLE_MOTOR_OFF_OPC           0x15
#define RADIAL_ON_OPC                   0x16
#define RADIAL_OFF_OPC                  0x17
#define MOVE_SLEDGE_OPC                 0x18
#define JUMP_GROOVES_OPC                0x19
#define WRITE_CD6_OPC                   0x1A
#define WRITE_DSIC2_OPC                 0x1B
#define READ_DSIC2_OPC                  0x1C

/* ==== internal opcodes ==== */
#define IDLE_OPC                        0xFF
#define ERROR_HANDLING_ID               0x00
#define MAX_LEGAL_NORMAL_ID             0x0F
#define MAX_LEGAL_SERVICE_ID            0x1D

struct time {
        byte    min;
        byte    sec;
        byte    frm;
            };

struct subcode_frame {
        byte            conad;
        byte            tno;
        byte            index;
        struct time     r_time;
        byte            zero;
        struct time     a_time;
                     };

struct  bytes {
        byte    high;
        byte    low;
              };
union int_high_low {
        int             int_i;
        struct bytes    byte_hl;
                   };
typedef union int_high_low      int_hl;

/* definitions for the process execution states (should be the same as in defs.asm) */
#define BUSY                            0
#define READY                           1
#define ERROR                           2
#define PROCESS_READY                   3

/* definitions for the compare_time function (should be the same as in defs.asm) */
#define SMALLER                         0
#define EQUAL                           1
#define BIGGER                          2
        
/* definitions for read_subcode control */
#define LEADIN_FORMAT                   0
#define ABS_TIME                        1
#define CATALOG_NR                      2
#define ISRC_NR                         3
#define ALL_SUBCODES                    4

#define CLOSED                          0
#define OPEN                            1

#define TRUE                            1
#define FALSE                           0

/* definitions of player_error */
#define NO_ERROR                        0x00
#define ILLEGAL_COMMAND                 0x01
#define ILLEGAL_PARAMETER               0x02
#define SLEDGE_ERROR                    0x03
#define FOCUS_ERROR                     0x04
#define MOTOR_ERROR                     0x05
#define RADIAL_ERROR                    0x06
#define PLL_LOCK_ERROR                  0x07
#define SUBCODE_TIMEOUT_ERROR           0x08
#define SUBCODE_NOT_FOUND               0x09
#define TRAY_ERROR                      0x0A
#define TOC_READ_ERROR                  0x0B
#define JUMP_ERROR                      0x0C
#define HF_DETECTOR_ERROR               0x0D

