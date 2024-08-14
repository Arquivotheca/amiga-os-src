/* enable or disable debugging globally */
#ifdef VERBOSE
void Debug0(char *);
void Debug1(char *, );
void Debug2(char *, ...);
#else
  #define DebugInit()
  #define DebugEnd()
  #define Debug0(x) 
  #define Debug1(x, y)
  #define Debug2(x, y, z)
#endif
/*
   Inhibiting before and after switching disk makes sure AmigaDOS does actually
   notice that the disk has been removed. It seems a bit slow sometimes.
   On the other hand, inhibiting costs a lot of time too. And it gets in the
   way when other activities happen while DOS is logging the disk.
*/
#ifndef DO_INHIBIT
  #define inhibit(filesystem, what)
  #define NO_INHIBIT
#endif
/* defines for the share project */

#define SHARENAME "Janus_Floppy_Port"

#define VERSION "Version 2.1"
#define MAKEDATE  __DATE__

#define DISK_UNKNOWN 4
#define DISK_PC 2
#define DISK_AMIGA 1
#define DISK_EMPTY 0

#define SELECT_PC 2
#define SELECT_AMIGA 1
#define SELECT_UNKNOWN 0

#define MODE_MANUAL 0x80
#define MODE_AUTO 0x40
#define MODE_QUIT 0x20
#define MODE_QUIT_DAMMIT 0x10

#define PC_A    1
#define PC_B    2
#define AMIGA_DF0 0
#define AMIGA_DF1 1

#define NO_ERROR 0
#define PARAMETER_ERROR 0x10
#define JANUS_ERROR 0x20
#define JANUS_NOTSHARED 0x21
#define DISK_OPEN_ERROR 0x40
#define DISK_CHANGE_ERROR 0x80
#define WINDOW_ERROR 0x100

#define BUFFERSIZE 512

#ifdef MAIN
  #define COMMON 
#else
  #define COMMON extern
#endif

#ifdef MAIN
char pc_a[] = "A:";
char pc_b[] = "B:";
char amiga_df0[] = "DF0:";
char amiga_df1[] = "DF1:";
char window_spec[] = "window=";
#endif

COMMON int currentmode, currentdisk, currentselect;
COMMON int owner, switchstate;

COMMON int xposition, yposition;
COMMON int going;

COMMON char *amiga_name, *pc_name;
COMMON int amiga_number, pc_number;

COMMON int user_mask, port_mask, disk_mask, wait_mask;

COMMON int window_mode;

#ifdef WB_SUPPORT

typedef struct OPTION {
    char *option;
    int (*handleoption) (char *);
} OPTION;

int pc_drive(char *);
int amiga_drive(char *);
int flipper_mode(char *);
int window_position(char *);

#ifdef STARTUP
char mode[] = "MODE=";
char position[] = "WINDOW=";

OPTION options[] = {
    { mode, flipper_mode},
    { position, window_position},
    { NULL, NULL}
};
#endif

#endif
