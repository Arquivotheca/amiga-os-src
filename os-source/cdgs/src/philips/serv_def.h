/***************************************************************************
*
* Project:      BASIC PLAYER MODULE
* File:         serv_def.h
* Date:         06 July 1993
* Version:      0.1
* Status:       Draft
*
* Description:  contains all servo definitions
*
* Decisions:    -
*
* HISTORY               AUTHOR COMMENTS
* 16 November 1992      creation P.vd.Z.
*
***************************************************************************/

/* SERVO STATES */
#define INIT_DSIC2                      0
#define INIT_SLEDGE                     1
#define CHECK_IN_SLEDGE                 2
#define CHECK_OUT_SLEDGE                3
#define SERVO_IDLE                      4
#define START_FOCUS                     5
#define CHECK_FOCUS                     6
#define DOUBLE_CHECK_FOCUS              7
#define START_TTM                       8
#define TTM_SPEED_UP                    9
#define CHECK_TTM                       10
#define START_RADIAL                    11
#define INIT_RADIAL                     12
#define SERVO_MONITOR                   13
#define RADIAL_RECOVER                  14
#define FOCUS_RECOVER                   15
#define SLEDGE_INSIDE_RECOVER           16
#define SLEDGE_OUTSIDE_RECOVER          17
#define STOP_SERVO                      18
#define CHECK_STOP_SERVO                19
#define JUMP_SERVO                      20
#define CHECK_JUMP                      21
#define WAIT_SUBCODE                    22
#define UPTO_N2                         23
#define DOWNTO_N1                       24

/* CD-6 STATUS PIN SELECTION */
#define SUBCODE_READY                   0x20
#define MOT_STRT_1                      0x21
#define MOT_STRT_2                      0x22
#define MOT_STOP                        0x23
#define PLL_LOCK                        0x24
#define MOTOR_OVERFLOW                  0x27

/* CD-6 MOTOR MODES */
#define MOT_OFF_ACTIVE                  0x18
#define MOT_BRM1_ACTIVE                 0x19
#define MOT_BRM2_ACTIVE                 0x1A
#define MOT_STRTM1_ACTIVE               0x1B
#define MOT_STRTM2_ACTIVE               0x1C
#define MOT_JMPM_ACTIVE                 0x1D
#define MOT_JMPM1_ACTIVE                0x1E
#define MOT_PLAYM_ACTIVE                0x1F

/* DISC SIZES */
#define EIGHT                           0
#define TWELVE                          1

/* FOR FUNCTION: get_jump_status */
#define NO_HF_ON_TARGET                 0x01
#define RADIAL_RECOVERY_DONE            0x02
#define FOCUS_RECOVERY_DONE             0x04
#define MOTOR_NOT_ON_SPEED              0x08

#define N                               1   /* select default single speed or double speed */

/* CD-6 SETTINGS */
#define SPEED_CONTROL_N1                0xB3 /* single speed & operating on 33 MHz */
#define SPEED_CONTROL_N2                0xBB /* double speed & operating on 33 MHz */
#define MOT_GAIN_8CM_N1                 0x41 /* motor gain G=4.0 */
#define MOT_GAIN_12CM_N1                0x44 /* motor gain G=12.8 */
#define MOT_GAIN_8CM_N2                 0x43 /* motor gain G=8.0 */
#define MOT_GAIN_12CM_N2                0x46 /* motor gain G=25.6 */
#define MOT_BANDWIDTH                   0x55 /* f3=1.71Hz & f4=0.7Hz */
#define DAC_OUTPUT_MODE                 0x3E /* 1fs 16 bit philips (cd-rom mode=0x3A) */
#define MOT_OUTPUT_MODE                 0x67 /* motor power 100% PWM Mode */
#define EBU_OUTPUT_MODE                 0xAA /* EBU */
#define AUDIO                           0x81
#define ROM                             0x89

#define MUTE                            0x00
#define FULL_SCALE                      0x01
#define ATTENUATE                       0x02
#define KILL_ON                         0xC6
#define KILL_OFF                        0xC2

#define SFCOEF1                         0x17 /* Focus command SFCOEF1 */
#define FOC_PARM3                       0x2B /* Focus lead length */
#define FOC_INT                         0x01 /* Focus integrator */
#define RAMP_INC                        0x1E
#define RAMP_HEIGHT                     0x14
#define RAMP_OFFSET                     0x01
#define FE_START                        0x18 /* Focus error start level */
#define FOC_GAIN                        0x51 /* Focus gain values */

#define SFCOEF2                         0x26 /* Focus command SFCOEF2 */
#define RAD_PARM_JUMP                   0x5A /* RP delay settings */
#define VEL_PARM2                       0x04 /* Velocity setpoint */
#define VEL_PARM1                       0x48 /* Velocity proportional part */
#define FOC_PARM1                       0x1E /* OTD selection */
#define FOC_PARM2                       0x23 /* Focus detector arrangement */
#define CA_DROP                         0x10 /* CA drop values */

#define SRCOEFS                         0x57 /* Radial command SRCOEF */
#define RAD_LENGTH_LEAD                 0x10 /* Radial lead length */
#define RAD_INT                         0x27 /* Radial integrator */
#define RAD_PARM_PLAY                   0x81 /* RP delay settings */
#define RAD_POLE_NOISE                  0x03 /* Radial control parameters */
#define RAD_GAIN                        0x6C /* Radial gain */
#define SLEDGE_PARM2                    0x64 /* Sledge gain */
#define SLEDGE_PARM1                    0x40 /* Sledge integrator */

#define PRESET                          0x81 /* Command PRESET */
#define LASER_ON                        0x8B /* clock divided by 4 */
#define LASER_OFF                       0x83

#define SRSLEDGE                        0xB1 /* Radial command SRSLEDGE */
#define SLEDGE_UOUT_IN                  0xA6 /* Sledge output power inside */
#define SLEDGE_UOUT_OFF                 0x00 /* Sledge output power off */
#define SLEDGE_UOUT_OUT                 0x59 /* Sledge output power outside */
#define SLEDGE_UOUT_JMP                 0x59 /* Sledge output at jump */

#define SFCOMM                          0x33 /* Focus command SFCOMM */
#define FOC_MASK                        0xF0 /* Focus mask */
#define FOC_STAT_OFF                    0x00 /* Focus status off */
#define FOC_STAT_ON                     0xE0 /* Focus status on */
#define FOC_MASK_DET                    0x00 /* Focus mask for shock detector off/on */
#define FOC_STAT_DET                    0x00 /* Focus status for shock detector off/on */
#define SHOCK_LEVEL                     0x7F /* Shock level */
#define SHOCK_LEVEL_ON                  0x20 /* Shock level for shock detector on */

#define SRCOMM1                         0xC1 /* Radial command SRCOMM : 1 BYTE */
#define HOLD_MODE                       0x1C /* Hold mode */
#define INIT_MODE                       0x3C /* Init mode */

#define SRCOMM3                         0xC3 /* Radial command SRCOMM : 3 BYTE -> SHORT JUMP */
#define RAD_STAT3                       0xF3 /* Radial Status for srcomm3 */

#define SRCOMM5                         0xC5 /* Radial command SRCOMM : 5 BYTE -> LONG JUMP */
#define RAD_STAT5                       0x43 /* Radial Status for srcomm5 */

#define SRINIT                          0x93 /* Radial command SRINIT */
#define RE_OFFSET                       0x00 /* Radial error offset */
#define RE_GAIN                         0xC4 /* Radial error gain */
#define SUM_GAIN                        0x9D /* Sum gain */

#define RSTAT1                          0x70 /* Focus command RSTAT1 */
#define RSTAT2                          0xF0 /* Radial command RSTAT2 */

#define SLEDGE_OUT_TIME                 19   /* 150 msec */
#define SLEDGE_IN_TIME                  500  /* 4000 msec 4000/8=500 */
#define HALF_SLEDGE_IN_TIME             250
#define MAX_RETRIES                     6    /* maximum number of servo errors */
#define TIME_DOUBLE_FOCUS_CHECK         3    /* after 24 msec check focus again */
#define FOCUS_TIME_OUT                  210  /* 1680 msec */
#define RAD_INITIALIZE_TIME             38   /* 38*8msec=304msec initialise time radial servo */
#define SKATING_DELAY_CHECK             12   /* 100 msec delay for first check */
#define F_REC_IN_SLEDGE                 12   /* 100 msec sledge in recover time focus */
#define R_REC_OUT_SLEDGE                12   /* 100 msec sledge out recover time radial */
#define MAX                             40   /* selection between short & long jump */
#define BRAKE_DIS_MAX                   176  /* maximum brake grooves border power */
#define BRAKE_2_DIS_MAX                 2*BRAKE_DIS_MAX
#define SKATING_SAMPLE_TIME             3    /* every 24 msec check skating */
#define SUBCODE_TIME_OUT                62   /* 500 msec subcode time out during finding of radial lock */
#define SUBCODE_MONITOR_TIMEOUT         25   /* it should be possible to read subcode every 200 msec */
#define SUBCODE_TIMEOUT_VALUE           62   /* wait 500 msec for new subcode */
#define TRACKS_INTO_LEADIN              -175 /* jump 175 tracks inward to get into the leadin */
#define TRACKS_OUTOF_LEADIN             300  /* jump 300 tracks forward to get out of the leadin */
#define SPEEDUP_TIME                    18   /* 150 msec */
#define NOMINAL_SPEED_TIME              255  /* max 2 seconds to reach nominal speed */
#define STOP_TIME_OUT                   188  /* after 1.5 second the disc motor is always switched off */
#define MOT_OFF_STOP_TIME               255  /* in mot off mode stop delay is X * 2 sec */
#define EXTRA_STOP_DELAY                125  /* give an extra delay of 1 second after disc motor switched off */
#define UPTO_N2_TIME_OUT                255  /* 2 seconds timeout for spin up from n=1 to n=2 */
#define UPTO_N2_TIME                    UPTO_N2_TIME_OUT-100  /* wait min 800 msec in start mode 2 */
#define N2_TO_N1_BRAKE_TIME             50   /* brake max 500 msec before switching to single speed */
#define N2_TO_N1_JUMP_TIME_OUT          188  /* 1.5 seconds timeout for spin down from n=2 to n=1 */
#define N2_TO_N1_JUMP_TIME              N2_TO_N1_JUMP_TIME_OUT-13  /* wait min 100 msec in start mode 2 */
#define TIME_8_CM                       9
#define SCOR_COUNT                      10

