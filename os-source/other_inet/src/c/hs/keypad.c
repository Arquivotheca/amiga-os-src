/* -----------------------------------------------------------------------
 * keypad.c			handshake_src
 *
 * $Locker:  $
 *
 * $Id: keypad.c,v 1.1 91/05/09 16:27:02 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/keypad.c,v 1.1 91/05/09 16:27:02 bj Exp $
 *
 * $Log:	keypad.c,v $
 * Revision 1.1  91/05/09  16:27:02  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */


/***
*
* Keyboard definition tables table and routines.
*
***/

#include "termall.h"

/***
*
* VT100 Key string output tables
*
***/


static unsigned char function_keys[6][73] =
  {
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','2','1','~',
    
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','2','6','~',
    
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','3','1','~',
    
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','3','2','~',
    
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','3','3','~',
    
    0,4,5,68,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0x1b,'[','3','4','~'
  };
    
static unsigned char vt200_esc [] =
  {
    5,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'[','2','3','~'/* String data       */
  };

static unsigned char vt200_bs [] =
  {
    5,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'[','2','4','~'/* String data       */
  };

static unsigned char vt200_help [] =
  {
    5,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'[','2','8','~'/* String data       */
  };

static unsigned char vt200_do [] =
  {
    5,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'[','2','9','~'/* String data       */
  };

static unsigned char vt100_cursor_up [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','A',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_ansi_cursor_up [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'[','A',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_cursor_down [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','B',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_ansi_cursor_down [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'[','B',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_cursor_right [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','C',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_ansi_cursor_right [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'[','C',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_cursor_left [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','D',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_ansi_cursor_left [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'[','D',  /* String data            */
    0x1b,'[','2','5','~'/* Shifted data      */
  };

static unsigned char vt100_F1 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','P',  /* String data            */
    0x1b,'[','1','7','~'/* Shifted string data   */
  };

static unsigned char vt100_F2 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','Q',  /* String data            */
    0x1b,'[','1','8','~'/* Shifted string data   */
  };

static unsigned char vt100_F3 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','R',  /* String data            */
    0x1b,'[','1','9','~'/* Shifted string data   */
  };

static unsigned char vt100_F4 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    5,             /* Shifted bytes          */
    7,             /* Shifted offset         */
    0x1b,'O','S',  /* String data            */
    0x1b,'[','2','0','~'/* Shifted string data   */
  };

static unsigned char vt100_appl_kp_0 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','p',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_1 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','q',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_2 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','r',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_3 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','s',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_4 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','t',  /* String data            */
    0x1b,'[','4','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_5 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','u',  /* String data            */
    0x1b,'[','5','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_6 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','v',  /* String data            */
    0x1b,'[','6','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_dot [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','n',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_7 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','w',  /* String data            */
    0x1b,'[','1','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_8 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','x',  /* String data            */
    0x1b,'[','2','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_9 [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','y',  /* String data            */
    0x1b,'[','3','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_enter [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','M',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_minus [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','m',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

static unsigned char vt100_appl_kp_comma [] =
  {
    3,             /* Bytes in String        */
    4,             /* Offset to string bytes */
    4,             /* Shifted_count          */
    7,             /* Shifted offset         */
    0x1b,'O','l',  /* String data            */
    0x1b,'[','~','~'/* Shifted data          */
  };

/***
*
* VT52 Key string output tables
*
***/

static unsigned char vt52_cursor_up [] =
  {
    2,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'A'       /* String data            */
  };

static unsigned char vt52_cursor_down [] =
  {
    2,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'B'       /* String data            */
  };

static unsigned char vt52_cursor_right [] =
  {
    2,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'C'       /* String data            */
  };

static unsigned char vt52_cursor_left [] =
  {
    2,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'D'       /* String data            */
  };

static unsigned char vt52_F1 [] =
  {
    2,             /* Bytes in string        */
    2,             /* Offset to string bytes */
    0x1b,'P'       /* String data            */
  };

static unsigned char vt52_F2 [] =
  {
    2,             /* Bytes in string        */
    2,             /* Offset to string bytes */
    0x1b,'Q'       /* String data            */
  };

static unsigned char vt52_F3 [] =
  {
    2,             /* Bytes in string        */
    2,             /* Offset to string bytes */
    0x1b,'R'       /* String data            */
  };

static unsigned char vt52_F4 [] =
  {
    2,             /* Bytes in string        */
    2,             /* Offset to string bytes */
    0x1b,'S'       /* String data            */
  };

static unsigned char vt52_appl_kp_0 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','p'   /* String data            */
  };

static unsigned char vt52_appl_kp_1 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','q'   /* String data            */
  };

static unsigned char vt52_appl_kp_2 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','r'   /* String data            */
  };

static unsigned char vt52_appl_kp_3 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','s'   /* String data            */
  };

static unsigned char vt52_appl_kp_4 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','t'   /* String data            */
  };

static unsigned char vt52_appl_kp_5 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','u'   /* String data            */
  };

static unsigned char vt52_appl_kp_6 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','v'   /* String data            */
  };

static unsigned char vt52_appl_kp_dot [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','n'   /* String data            */
  };

static unsigned char vt52_appl_kp_7 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','w'   /* String data            */
  };

static unsigned char vt52_appl_kp_8 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','x'   /* String data            */
  };

static unsigned char vt52_appl_kp_9 [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','y'   /* String data            */
  };

static unsigned char vt52_appl_kp_enter [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','M'   /* String data            */
  };

static unsigned char vt52_appl_kp_minus [] =
  {
    3,             /* Bytes in String        */
    2,             /* Offset to string bytes */
    0x1b,'?','m'   /* String data            */
  };

static unsigned char vt52_appl_kp_comma [] =
  {
    3,             /* Bytes in string        */
    2,             /* Offset to string bytes */
    0x1b,'?','l'   /* String data            */
  };


/***
*
* VT100 STandard keyboard tables.
*
***/

/***
* Low tables
***/

static unsigned char no_repeat [] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

static unsigned char vt100_lo_capsable [] =
  {
    0x00,0x00,0xff,0x03,0xff,0x01,0xfe,0x00
  };

static unsigned char vt100_lo_repeatable [] =
  {
    0xff,0xbf,0xff,0xef,0xff,0xef,0xff,0xf7
  };


/***
* High tables
***/

static unsigned char vt100_hi_capsable [] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

static unsigned char vt100_hi_repeatable [] =
  {
    0xef,0xf4,0xff,0xff,0x00,0x00,0x00,0x00
  };

static unsigned char vt100_hi_map_types [] =
  {
    KCF_ALT,             /* 40 Space        */
    KCF_STRING,          /* 41 Backspace    */
    KC_NOQUAL,           /* 42 Tab          */
    KCF_STRING,          /* 43 Enter        */
    KCF_CONTROL,         /* 44 Return       */
    KCF_STRING,          /* 45 Escape       */
    KCF_CONTROL,         /* 46 Delete       */
    0,
    0,
    0,
    KCF_ALT,             /* 4a Numeric Pad -*/
    0,
    KCF_STRING+KCF_SHIFT,/* 4c Cursor Up    */
    KCF_STRING+KCF_SHIFT,/* 4d Cursor Down  */
    KCF_STRING+KCF_SHIFT,/* 4e Cursor Frwrd */
    KCF_STRING+KCF_SHIFT,/* 4f Cursor Bkwrd */
    KCF_STRING+KCF_SHIFT,/* 50 F1           */
    KCF_STRING+KCF_SHIFT,/* 51 F2           */
    KCF_STRING+KCF_SHIFT,/* 52 F3           */
    KCF_STRING+KCF_SHIFT,/* 53 F4           */
    KCF_STRING+KCF_SHIFT,/* 54 F5           */
    KCF_STRING+KCF_SHIFT,/* 55 F6           */
    KCF_STRING+KCF_SHIFT,/* 56 F7           */
    KCF_STRING+KCF_SHIFT,/* 57 F8           */
    KCF_STRING+KCF_SHIFT,/* 58 F9           */
    KCF_STRING+KCF_SHIFT,/* 59 F10          */
    KCF_STRING,          /* 5a Keypad (     */
    KCF_STRING,          /* 5b Keypad )     */
    KCF_STRING,          /* 5c Keypad /     */
    KCF_STRING,          /* 5d Keypad *     */
    KCF_STRING+KCF_SHIFT,/* 5e Keypad +     */
    KCF_STRING,          /* 5f Help         */
    KCF_NOP,             /* 60 Left Shift   */
    KCF_NOP,             /* 61 Right Shift  */
    KCF_NOP,             /* 62 Caps Lock    */
    KCF_NOP,             /* 63 Control      */
    KCF_STRING,          /* 64 Left Alt     */
    KCF_STRING,          /* 65 Right Alt    */
    KCF_NOP,             /* 66 Left Amiga   */
    KCF_NOP              /* 67 Right Amiga  */
  };

static unsigned char *vt100_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    vt200_bs,                       /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    vt100_appl_kp_enter,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    vt200_esc,                      /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    (char *) 0x0000ad2d,   /* 4a Numeric Pad - */
    0,
    vt100_cursor_up,       /* 4c Cursor Up     */
    vt100_cursor_down,     /* 4d Cursor Down   */
    vt100_cursor_right,    /* 4e Cursor Frwrd  */
    vt100_cursor_left,     /* 4f Cursor Bkwrd  */
    vt100_F1,              /* 50 F1            */
    vt100_F2,              /* 51 F2            */
    vt100_F3,              /* 52 F3            */
    vt100_F4,              /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt100_F1,              /* 5a Keypad (      */
    vt100_F2,              /* 5b Keypad )      */
    vt100_F3,              /* 5c Keypad /      */
    vt100_F4,              /* 5d Keypad *      */
    vt100_appl_kp_comma,   /* 5e Keypad +      */
    vt200_help,            /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    vt200_do,              /* 64 Left Alt      */
    vt100_appl_kp_comma,   /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };

static unsigned char *vt100_ansi_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    vt200_bs,                       /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    vt100_appl_kp_enter,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    vt200_esc,                      /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    (char *) 0x0000ad2d,   /* 4a Numeric Pad - */
    0,
    vt100_ansi_cursor_up,  /* 4c Cursor Up     */
    vt100_ansi_cursor_down,/* 4d Cursor Down   */
    vt100_ansi_cursor_right,/* 4e Cursor Frwrd */
    vt100_ansi_cursor_left,/* 4f Cursor Bkwrd  */
    vt100_F1,              /* 50 F1            */
    vt100_F2,              /* 51 F2            */
    vt100_F3,              /* 52 F3            */
    vt100_F4,              /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt100_F1,              /* 5a Keypad (      */
    vt100_F2,              /* 5b Keypad )      */
    vt100_F3,              /* 5c Keypad /      */
    vt100_F4,              /* 5d Keypad *      */
    vt100_appl_kp_comma,   /* 5e Keypad +      */
    vt200_help,            /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    vt200_do,              /* 64 Left Alt      */
    vt100_appl_kp_comma,   /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };

/***
*
* VT100 Application keybaord tables.
*
***/

/***
* Low tables
***/

static unsigned char vt100_appl_lo_capsable [] =
  {
    0x00,0x00,0xff,0x03,0xff,0x01,0xfe,0x00
  };

static unsigned char vt100_appl_lo_repeatable [] =
  {
    0xff,0xbf,0xff,0xef,0xff,0xef,0xff,0xf7
  };

static unsigned char vt100_appl_lo_map_types [] =
  {
    KCF_CONTROL+KCF_SHIFT,/* 00 `      */
    KC_VANILLA,       /* 01 1          */
    KC_VANILLA,       /* 02 2          */
    KCF_CONTROL+KCF_SHIFT,/* 04 3      */
    KCF_CONTROL+KCF_SHIFT,/* 05 4      */
    KCF_CONTROL+KCF_SHIFT,/* 06 5      */
    KCF_CONTROL+KCF_SHIFT,/* 07 6      */
    KCF_CONTROL+KCF_SHIFT,/* 08 7      */
    KC_VANILLA,       /* 08 8          */
    KC_VANILLA,       /* 09 9          */
    KC_VANILLA,       /* 0A 0          */
    KC_VANILLA,       /* 0B -          */
    KC_VANILLA,       /* 0C =          */
    KC_VANILLA,       /* 0D \          */
    KCF_NOP,
    KCF_STRING,       /* 0F Keypad 0   */
    KC_VANILLA,       /* 10 q          */
    KC_VANILLA,       /* 11 w          */
    KC_VANILLA,       /* 12 e          */
    KC_VANILLA,       /* 13 r          */
    KC_VANILLA,       /* 14 t          */
    KC_VANILLA,       /* 15 y          */
    KC_VANILLA,       /* 16 u          */
    KC_VANILLA,       /* 17 i          */
    KC_VANILLA,       /* 18 o          */
    KC_VANILLA,       /* 19 p          */
    KC_VANILLA,       /* 1a [          */
    KC_VANILLA,       /* 1b ]          */
    KCF_NOP,
    KCF_STRING,       /* 1d Keypad 1   */
    KCF_STRING,       /* 1e Keypad 2   */
    KCF_STRING,       /* 1f Keypad 3   */
    KC_VANILLA,       /* 20 a          */
    KC_VANILLA,       /* 21 s          */
    KC_VANILLA,       /* 22 d          */
    KC_VANILLA,       /* 23 f          */
    KC_VANILLA,       /* 24 g          */
    KC_VANILLA,       /* 25 h          */
    KC_VANILLA,       /* 26 j          */
    KC_VANILLA,       /* 27 k          */
    KC_VANILLA,       /* 28 l          */
    KC_VANILLA,       /* 29 ;          */
    KC_VANILLA,       /* 2a '          */
    KCF_NOP,
    KCF_NOP,
    KCF_STRING+KCF_SHIFT,/* 2d Keypad 4   */
    KCF_STRING+KCF_SHIFT,/* 2e Keypad 5   */
    KCF_STRING+KCF_SHIFT,/* 2f Keypad 6   */
    KCF_NOP,
    KC_VANILLA,       /* 31 z          */
    KC_VANILLA,       /* 32 x          */
    KC_VANILLA,       /* 33 c          */
    KC_VANILLA,       /* 34 v          */
    KC_VANILLA,       /* 35 b          */
    KC_VANILLA,       /* 36 n          */
    KC_VANILLA,       /* 37 m          */
    KC_VANILLA,       /* 38 ,          */
    KC_VANILLA,       /* 39 .          */
    KC_VANILLA,       /* 3a /          */
    KCF_NOP,
    KCF_STRING+KCF_SHIFT,/* 3c Keypad .   */
    KCF_STRING+KCF_SHIFT,/* 3d Keypad 7   */
    KCF_STRING+KCF_SHIFT,/* 3e Keypad 8   */
    KCF_STRING+KCF_SHIFT /* 3f Keypad 9   */
  };

static unsigned char *vt100_appl_lo_map [] =
  {
    (char *) 0x1e1e7e60,     /* 00 `  */
    (char *) 0xa1b12131,     /* 01 1  */
    (char *) 0xc0b24032,     /* 02 2  */
    (char *) 0x1b1b2333,     /* 03 3  */
    (char *) 0x1c1c2434,     /* 04 4  */
    (char *) 0x1d1d2535,     /* 05 5  */
    (char *) 0x1e1e5e36,     /* 06 6  */
    (char *) 0x1f1f2637,     /* 07 7  */
    (char *) 0xaab82a38,     /* 08 8  */
    (char *) 0xa8b92839,     /* 09 9  */
    (char *) 0xa9b02930,     /* 0a 0  */
    (char *) 0xdfad5f2d,     /* 0b -  */
    (char *) 0xabbd2b3d,     /* 0c =  */
    (char *) 0xfcdc7c5c,     /* 0d \  */
    0,
    vt100_appl_kp_0,         /* 0f Keypad 0 */
    (char *) 0xd1f15171,     /* 10 q  */
    (char *) 0xd7f75777,     /* 11 w  */
    (char *) 0xc5e54565,     /* 12 e  */
    (char *) 0xd2f25272,     /* 13 r  */
    (char *) 0xd4f45474,     /* 14 t  */
    (char *) 0xd9f95979,     /* 15 y  */
    (char *) 0xd5f55575,     /* 16 u  */
    (char *) 0xc9e94969,     /* 17 i  */
    (char *) 0xcfef4f6f,     /* 18 o  */
    (char *) 0xd0f05070,     /* 19 p  */
    (char *) 0xfbdb7b5b,     /* 1a [  */
    (char *) 0xfddd7d5d,     /* 1b ]  */
    (char *) 0,
    vt100_appl_kp_1,         /* 1d Keypad 1 */
    vt100_appl_kp_2,         /* 1e Keypad 2 */
    vt100_appl_kp_3,         /* 1f Keypad 3 */
    (char *) 0xc1e14161,     /* 20 a  */
    (char *) 0xd3f35373,     /* 21 s  */
    (char *) 0xc4e44464,     /* 22 d  */
    (char *) 0xc6e64666,     /* 23 f  */
    (char *) 0xc7e74767,     /* 24 g  */
    (char *) 0xc8e84868,     /* 25 h  */
    (char *) 0xcaea4a6a,     /* 26 j  */
    (char *) 0xcbeb4b6b,     /* 27 k  */
    (char *) 0xccec4c6c,     /* 28 l  */
    (char *) 0xbabb3a3b,     /* 29 ;  */
    (char *) 0xa2a72227,     /* 2a '  */
    0,
    0,
    vt100_appl_kp_4,         /* 2d Keypad 4 */
    vt100_appl_kp_5,         /* 2e Keypad 5 */
    vt100_appl_kp_6,         /* 2f Keypad 6 */
    0,
    (char *) 0xdafa5a7a,     /* 31 z  */
    (char *) 0xd8f85878,     /* 32 x  */
    (char *) 0xc3e34363,     /* 33 c  */
    (char *) 0xd6f65676,     /* 34 v  */
    (char *) 0xc2e24262,     /* 35 b  */
    (char *) 0xceee4e6e,     /* 36 n  */
    (char *) 0xcded4d6d,     /* 37 m  */
    (char *) 0xbcac3c2c,     /* 38 ,  */
    (char *) 0xbeae3e2e,     /* 39 .  */
    (char *) 0xbfaf3f2f,     /* 3a ? */
    0,
    vt100_appl_kp_dot,       /* 3c Keypad . */
    vt100_appl_kp_7,         /* 3d Keypad 7 */
    vt100_appl_kp_8,         /* 3e Keypad 8 */
    vt100_appl_kp_9          /* 3f Keypad 9 */
  };

/***
* High tables
***/

static unsigned char vt100_appl_hi_capsable [] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

static unsigned char vt100_appl_hi_repeatable [] =
  {
    0xef,0xf4,0xff,0xff,0x00,0x00,0x00,0x00
  };

static unsigned char vt100_appl_hi_map_types [] =
  {
    KCF_ALT,        /* 40 Space        */
    KCF_STRING,     /* 41 Backspace    */
    KC_NOQUAL,      /* 42 Tab          */
    KCF_STRING,     /* 43 Enter        */
    KCF_CONTROL,    /* 44 Return       */
    KCF_STRING,     /* 45 Escape       */
    KCF_CONTROL,    /* 46 Delete       */
    0,
    0,
    0,
    KCF_STRING,     /* 4a Numeric Pad -*/
    0,
    KCF_STRING+KCF_SHIFT,/* 4c Cursor Up    */
    KCF_STRING+KCF_SHIFT,/* 4d Cursor Down  */
    KCF_STRING+KCF_SHIFT,/* 4e Cursor Frwrd */
    KCF_STRING+KCF_SHIFT,/* 4f Cursor Bkwrd */
    KCF_STRING+KCF_SHIFT,/* 50 F1           */
    KCF_STRING+KCF_SHIFT,/* 51 F2           */
    KCF_STRING+KCF_SHIFT,/* 52 F3           */
    KCF_STRING+KCF_SHIFT,/* 53 F4           */
    KCF_STRING+KCF_SHIFT,/* 54 F5           */
    KCF_STRING+KCF_SHIFT,/* 55 F6           */
    KCF_STRING+KCF_SHIFT,/* 56 F7           */
    KCF_STRING+KCF_SHIFT,/* 57 F8           */
    KCF_STRING+KCF_SHIFT,/* 58 F9           */
    KCF_STRING+KCF_SHIFT,/* 59 F10          */
    KCF_STRING,          /* 5a Keypad (     */
    KCF_STRING,          /* 5b Keypad )     */
    KCF_STRING,          /* 5c Keypad /     */
    KCF_STRING,          /* 5d Keypad *     */
    KCF_STRING+KCF_SHIFT,/* 5e Keypad +     */
    KCF_STRING,          /* 5f Help         */
    KCF_NOP,             /* 60 Left Shift   */
    KCF_NOP,             /* 61 Right Shift  */
    KCF_NOP,             /* 62 Caps Lock    */
    KCF_NOP,             /* 63 Control      */
    KCF_STRING,          /* 64 Left Alt     */
    KCF_STRING,          /* 65 Right Alt    */
    KCF_NOP,             /* 66 Left Amiga   */
    KCF_NOP              /* 67 Right Amiga  */
  };

static unsigned char *vt100_appl_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    vt200_bs,                       /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    vt100_appl_kp_enter,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    vt200_esc,                      /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    vt100_appl_kp_minus,   /* 4a Numeric Pad - */
    0,
    vt100_cursor_up,       /* 4c Cursor Up     */
    vt100_cursor_down,     /* 4d Cursor Down   */
    vt100_cursor_right,    /* 4e Cursor Frwrd  */
    vt100_cursor_left,     /* 4f Cursor Bkwrd  */
    vt100_F1,              /* 50 F1            */
    vt100_F2,              /* 51 F2            */
    vt100_F3,              /* 52 F3            */
    vt100_F4,              /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt100_F1,              /* 5a Keypad (      */
    vt100_F2,              /* 5b Keypad )      */
    vt100_F3,              /* 5c Keypad /      */
    vt100_F4,              /* 5d Keypad *      */
    vt100_appl_kp_comma,   /* 5e Keypad +      */
    vt200_help,            /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    vt200_do,              /* 64 Left Alt      */
    vt100_appl_kp_comma,   /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };

static unsigned char *vt100_ansi_appl_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    vt200_bs,                       /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    vt100_appl_kp_enter,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    vt200_esc,                      /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    vt100_appl_kp_minus,   /* 4a Numeric Pad - */
    0,
    vt100_ansi_cursor_up,  /* 4c Cursor Up     */
    vt100_ansi_cursor_down,/* 4d Cursor Down   */
    vt100_ansi_cursor_right,/* 4e Cursor Frwrd  */
    vt100_ansi_cursor_left,/* 4f Cursor Bkwrd  */
    vt100_F1,              /* 50 F1            */
    vt100_F2,              /* 51 F2            */
    vt100_F3,              /* 52 F3            */
    vt100_F4,              /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt100_F1,              /* 5a Keypad (      */
    vt100_F2,              /* 5b Keypad )      */
    vt100_F3,              /* 5c Keypad /      */
    vt100_F4,              /* 5d Keypad *      */
    vt100_appl_kp_comma,   /* 5e Keypad +      */
    vt200_help,            /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    vt200_do,              /* 64 Left Alt      */
    vt100_appl_kp_comma,   /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };

/***
*
* VT52 Standard keyboard tables.
*
***/

/***
* Low tables
***/

static unsigned char vt52_lo_capsable [] =
  {
    0x00,0x00,0xff,0x03,0xff,0x01,0xfe,0x00
  };

static unsigned char vt52_lo_repeatable [] =
  {
    0xff,0xbf,0xff,0xef,0xff,0xef,0xff,0xf7
  };

static unsigned char vt52_lo_map_types [] =
  {
    KC_VANILLA,       /* 00 `          */
    KC_VANILLA,       /* 01 1          */
    KC_VANILLA,       /* 02 2          */
    KC_VANILLA,       /* 03 3          */
    KC_VANILLA,       /* 04 4          */
    KC_VANILLA,       /* 05 5          */
    KC_VANILLA,       /* 06 6          */
    KC_VANILLA,       /* 07 7          */
    KC_VANILLA,       /* 08 8          */
    KC_VANILLA,       /* 09 9          */
    KC_VANILLA,       /* 0A 0          */
    KC_VANILLA,       /* 0B -          */
    KC_VANILLA,       /* 0C =          */
    KC_VANILLA,       /* 0D \          */
    KCF_NOP,
    KC_VANILLA,       /* 0F Keypad 0   */
    KC_VANILLA,       /* 10 q          */
    KC_VANILLA,       /* 11 w          */
    KC_VANILLA,       /* 12 e          */
    KC_VANILLA,       /* 13 r          */
    KC_VANILLA,       /* 14 t          */
    KC_VANILLA,       /* 15 y          */
    KC_VANILLA,       /* 16 u          */
    KC_VANILLA,       /* 17 i          */
    KC_VANILLA,       /* 18 o          */
    KC_VANILLA,       /* 19 p          */
    KC_VANILLA,       /* 1a [          */
    KC_VANILLA,       /* 1b ]          */
    KCF_NOP,
    KC_VANILLA,       /* 1d Keypad 1   */
    KC_VANILLA,       /* 1e Keypad 2   */
    KC_VANILLA,       /* 1f Keypad 3   */
    KC_VANILLA,       /* 20 a          */
    KC_VANILLA,       /* 21 s          */
    KC_VANILLA,       /* 22 d          */
    KC_VANILLA,       /* 23 f          */
    KC_VANILLA,       /* 24 g          */
    KC_VANILLA,       /* 25 h          */
    KC_VANILLA,       /* 26 j          */
    KC_VANILLA,       /* 27 k          */
    KC_VANILLA,       /* 28 l          */
    KC_VANILLA,       /* 29 ;          */
    KC_VANILLA,       /* 2a '          */
    KCF_NOP,
    KCF_NOP,
    KC_VANILLA,       /* 2d Keypad 4   */
    KC_VANILLA,       /* 2e Keypad 5   */
    KC_VANILLA,       /* 2f Keypad 6   */
    KCF_NOP,
    KC_VANILLA,       /* 31 z          */
    KC_VANILLA,       /* 32 x          */
    KC_VANILLA,       /* 33 c          */
    KC_VANILLA,       /* 34 v          */
    KC_VANILLA,       /* 35 b          */
    KC_VANILLA,       /* 36 n          */
    KC_VANILLA,       /* 37 m          */
    KC_VANILLA,       /* 38 ,          */
    KC_VANILLA,       /* 39 .          */
    KC_VANILLA,       /* 3a /          */
    KCF_NOP,
    KC_VANILLA,       /* 3c Keypad .   */
    KC_VANILLA,       /* 3d Keypad 7   */
    KC_VANILLA,       /* 3e Keypad 8   */
    KC_VANILLA        /* 3f Keypad 9   */
  };

static unsigned char *vt52_lo_map [] =
  {
    (char *) 0xfee07e60,     /* 00 `  */
    (char *) 0xa1b12131,     /* 01 1  */
    (char *) 0xc0b24032,     /* 02 2  */
    (char *) 0xa3b32333,     /* 03 3  */
    (char *) 0xa4b42434,     /* 04 4  */
    (char *) 0xa5b52535,     /* 05 5  */
    (char *) 0xdeb65e36,     /* 06 6  */
    (char *) 0xa6b72637,     /* 07 7  */
    (char *) 0xaab82a38,     /* 08 8  */
    (char *) 0xa8b92839,     /* 09 9  */
    (char *) 0xa9b02930,     /* 0a 0  */
    (char *) 0xdfad5f2d,     /* 0b -  */
    (char *) 0xabbd2b3d,     /* 0c =  */
    (char *) 0xfcdc7c5c,     /* 0d \  */
    0,
    (char *) 0xb0b03030,     /* 0f Keypad 0 */
    (char *) 0xd1f15171,     /* 10 q  */
    (char *) 0xd7f75777,     /* 11 w  */
    (char *) 0xc5e54565,     /* 12 e  */
    (char *) 0xd2f25272,     /* 13 r  */
    (char *) 0xd4f45474,     /* 14 t  */
    (char *) 0xd9f95979,     /* 15 y  */
    (char *) 0xd5f55575,     /* 16 u  */
    (char *) 0xc9e94969,     /* 17 i  */
    (char *) 0xcfef4f6f,     /* 18 o  */
    (char *) 0xd0f05070,     /* 19 p  */
    (char *) 0xfbdb7b5b,     /* 1a [  */
    (char *) 0xfddd7d5d,     /* 1b ]  */
    (char *) 0,
    (char *) 0xb1b13131,     /* 1d Keypad 1 */
    (char *) 0xb2b23232,     /* 1e Keypad 2 */
    (char *) 0xb3b33333,     /* 1f Keypad 3 */
    (char *) 0xc1e14161,     /* 20 a  */
    (char *) 0xd3f35373,     /* 21 s  */
    (char *) 0xc4e44464,     /* 22 d  */
    (char *) 0xc6e64666,     /* 23 f  */
    (char *) 0xc7e74767,     /* 24 g  */
    (char *) 0xc8e84868,     /* 25 h  */
    (char *) 0xcaea4a6a,     /* 26 j  */
    (char *) 0xcbeb4b6b,     /* 27 k  */
    (char *) 0xccec4c6c,     /* 28 l  */
    (char *) 0xbabb3a3b,     /* 29 ;  */
    (char *) 0xa2a72227,     /* 2a '  */
    0,
    0,
    (char *) 0xb4b43434,     /* 2d Keypad 4 */
    (char *) 0xb5b53535,     /* 2e Keypad 5 */
    (char *) 0xb6b63636,     /* 2f Keypad 6 */
    0,
    (char *) 0xdafa5a7a,     /* 31 z  */
    (char *) 0xd8f85878,     /* 32 x  */
    (char *) 0xc3e34363,     /* 33 c  */
    (char *) 0xd6f65676,     /* 34 v  */
    (char *) 0xc2e24262,     /* 35 b  */
    (char *) 0xceee4e6e,     /* 36 n  */
    (char *) 0xcded4d6d,     /* 37 m  */
    (char *) 0xbcac3c2c,     /* 38 ,  */
    (char *) 0xbeae3e2e,     /* 39 .  */
    (char *) 0xbfaf3f2f,     /* 3a ? */
    0,
    (char *) 0xaeae2e2e,     /* 3c Keypad . */
    (char *) 0xb7b73737,     /* 3d Keypad 7 */
    (char *) 0xb8b83838,     /* 3e Keypad 8 */
    (char *) 0xb9b93939      /* 3f Keypad 9 */
  };

/***
* High tables
***/

static unsigned char vt52_hi_capsable [] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

static unsigned char vt52_hi_repeatable [] =
  {
    0xef,0xf4,0xff,0xff,0x00,0x00,0x00,0x00
  };

static unsigned char vt52_hi_map_types [] =
  {
    KCF_ALT,        /* 40 Space        */
    KCF_CONTROL,    /* 41 Backspace    */
    KC_NOQUAL,      /* 42 Tab          */
    KCF_ALT,        /* 43 Enter        */
    KCF_CONTROL,    /* 44 Return       */
    KCF_ALT,        /* 45 Escape       */
    KCF_CONTROL,    /* 46 Delete       */
    0,
    0,
    0,
    KCF_ALT,        /* 4a Numeric Pad -*/
    0,
    KCF_STRING,     /* 4c Cursor Up    */
    KCF_STRING,     /* 4d Cursor Down  */
    KCF_STRING,     /* 4e Cursor Frwrd */
    KCF_STRING,     /* 4f Cursor Bkwrd */
    KCF_STRING,     /* 50 F1           */
    KCF_STRING,     /* 51 F2           */
    KCF_STRING,     /* 52 F3           */
    KCF_STRING,     /* 53 F4           */
    KCF_STRING,     /* 54 F5           */
    KCF_STRING,     /* 55 F6           */
    KCF_STRING,     /* 56 F7           */
    KCF_STRING,     /* 57 F8           */
    KCF_STRING,     /* 58 F9           */
    KCF_STRING,     /* 59 F10          */
    KCF_STRING,     /* 5a Keypad (     */
    KCF_STRING,     /* 5b Keypad )     */
    KCF_STRING,     /* 5c Keypad /     */
    KCF_STRING,     /* 5d Keypad *     */
    KC_VANILLA,     /* 5e Keypad +     */
    KCF_NOP,        /* 5f Help         */
    KCF_NOP,        /* 60 Left Shift   */
    KCF_NOP,        /* 61 Right Shift  */
    KCF_NOP,        /* 62 Caps Lock    */
    KCF_NOP,        /* 63 Control      */
    KCF_NOP,        /* 64 Left Alt     */
    KC_VANILLA,     /* 65 Right Alt    */
    KCF_NOP,        /* 66 Left Amiga   */
    KCF_NOP         /* 67 Right Amiga  */
  };

static unsigned char *vt52_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    (char *) 0x0000087f,            /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    (char *) 0x00008d0d,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    (char *) 0x00009b1b,            /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    (char *) 0x0000ad2d,   /* 4a Numeric Pad - */
    0,
    vt52_cursor_up,        /* 4c Cursor Up     */
    vt52_cursor_down,      /* 4d Cursor Down   */
    vt52_cursor_right,     /* 4e Cursor Frwrd  */
    vt52_cursor_left,      /* 4f Cursor Bkwrd  */
    vt52_F1,               /* 50 F1            */
    vt52_F2,               /* 51 F2            */
    vt52_F3,               /* 52 F3            */
    vt52_F4,               /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt52_F1,               /* 5a Keypad (      */
    vt52_F2,               /* 5b Keypad )      */
    vt52_F3,               /* 5c Keypad /      */
    vt52_F4,               /* 5d Keypad *      */
    (char *) 0x2c2c2c2c,   /* 5e Keypad +      */
    0,                     /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    0,                     /* 64 Left Alt      */
    (char *) 0x2c2c2c2c,   /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };


/***
*
* VT52 Application keyboard tables.
*
***/

/***
* Low tables
***/

static unsigned char vt52_appl_lo_capsable [] =
  {
    0x00,0x00,0xff,0x03,0xff,0x01,0xfe,0x00
  };

static unsigned char vt52_appl_lo_repeatable [] =
  {
    0xff,0xbf,0xff,0xef,0xff,0xef,0xff,0xf7
  };

static unsigned char vt52_appl_lo_map_types [] =
  {
    KC_VANILLA,       /* 00 `          */
    KC_VANILLA,       /* 01 1          */
    KC_VANILLA,       /* 02 2          */
    KC_VANILLA,       /* 03 3          */
    KC_VANILLA,       /* 04 4          */
    KC_VANILLA,       /* 05 5          */
    KC_VANILLA,       /* 06 6          */
    KC_VANILLA,       /* 07 7          */
    KC_VANILLA,       /* 08 8          */
    KC_VANILLA,       /* 09 9          */
    KC_VANILLA,       /* 0A 0          */
    KC_VANILLA,       /* 0B -          */
    KC_VANILLA,       /* 0C =          */
    KC_VANILLA,       /* 0D \          */
    KCF_NOP,
    KCF_STRING,       /* 0F Keypad 0   */
    KC_VANILLA,       /* 10 q          */
    KC_VANILLA,       /* 11 w          */
    KC_VANILLA,       /* 12 e          */
    KC_VANILLA,       /* 13 r          */
    KC_VANILLA,       /* 14 t          */
    KC_VANILLA,       /* 15 y          */
    KC_VANILLA,       /* 16 u          */
    KC_VANILLA,       /* 17 i          */
    KC_VANILLA,       /* 18 o          */
    KC_VANILLA,       /* 19 p          */
    KC_VANILLA,       /* 1a [          */
    KC_VANILLA,       /* 1b ]          */
    KCF_NOP,
    KCF_STRING,       /* 1d Keypad 1   */
    KCF_STRING,       /* 1e Keypad 2   */
    KCF_STRING,       /* 1f Keypad 3   */
    KC_VANILLA,       /* 20 a          */
    KC_VANILLA,       /* 21 s          */
    KC_VANILLA,       /* 22 d          */
    KC_VANILLA,       /* 23 f          */
    KC_VANILLA,       /* 24 g          */
    KC_VANILLA,       /* 25 h          */
    KC_VANILLA,       /* 26 j          */
    KC_VANILLA,       /* 27 k          */
    KC_VANILLA,       /* 28 l          */
    KC_VANILLA,       /* 29 ;          */
    KC_VANILLA,       /* 2a '          */
    KCF_NOP,
    KCF_NOP,
    KCF_STRING,       /* 2d Keypad 4   */
    KCF_STRING,       /* 2e Keypad 5   */
    KCF_STRING,       /* 2f Keypad 6   */
    KCF_NOP,
    KC_VANILLA,       /* 31 z          */
    KC_VANILLA,       /* 32 x          */
    KC_VANILLA,       /* 33 c          */
    KC_VANILLA,       /* 34 v          */
    KC_VANILLA,       /* 35 b          */
    KC_VANILLA,       /* 36 n          */
    KC_VANILLA,       /* 37 m          */
    KC_VANILLA,       /* 38 ,          */
    KC_VANILLA,       /* 39 .          */
    KC_VANILLA,       /* 3a /          */
    KCF_NOP,
    KCF_STRING,       /* 3c Keypad .   */
    KCF_STRING,       /* 3d Keypad 7   */
    KCF_STRING,       /* 3e Keypad 8   */
    KCF_STRING        /* 3f Keypad 9   */
  };

static unsigned char *vt52_appl_lo_map [] =
  {
    (char *) 0xfee07e60,     /* 00 `  */
    (char *) 0xa1b12131,     /* 01 1  */
    (char *) 0xc0b24032,     /* 02 2  */
    (char *) 0xa3b32333,     /* 03 3  */
    (char *) 0xa4b42434,     /* 04 4  */
    (char *) 0xa5b52535,     /* 05 5  */
    (char *) 0xdeb65e36,     /* 06 6  */
    (char *) 0xa6b72637,     /* 07 7  */
    (char *) 0xaab82a38,     /* 08 8  */
    (char *) 0xa8b92839,     /* 09 9  */
    (char *) 0xa9b02930,     /* 0a 0  */
    (char *) 0xdfad5f2d,     /* 0b -  */
    (char *) 0xabbd2b3d,     /* 0c =  */
    (char *) 0xfcdc7c5c,     /* 0d \  */
    0,
    vt52_appl_kp_0,          /* 0f Keypad 0 */
    (char *) 0xd1f15171,     /* 10 q  */
    (char *) 0xd7f75777,     /* 11 w  */
    (char *) 0xc5e54565,     /* 12 e  */
    (char *) 0xd2f25272,     /* 13 r  */
    (char *) 0xd4f45474,     /* 14 t  */
    (char *) 0xd9f95979,     /* 15 y  */
    (char *) 0xd5f55575,     /* 16 u  */
    (char *) 0xc9e94969,     /* 17 i  */
    (char *) 0xcfef4f6f,     /* 18 o  */
    (char *) 0xd0f05070,     /* 19 p  */
    (char *) 0xfbdb7b5b,     /* 1a [  */
    (char *) 0xfddd7d5d,     /* 1b ]  */
    (char *) 0,
    vt52_appl_kp_1,          /* 1d Keypad 1 */
    vt52_appl_kp_2,          /* 1e Keypad 2 */
    vt52_appl_kp_3,          /* 1f Keypad 3 */
    (char *) 0xc1e14161,     /* 20 a  */
    (char *) 0xd3f35373,     /* 21 s  */
    (char *) 0xc4e44464,     /* 22 d  */
    (char *) 0xc6e64666,     /* 23 f  */
    (char *) 0xc7e74767,     /* 24 g  */
    (char *) 0xc8e84868,     /* 25 h  */
    (char *) 0xcaea4a6a,     /* 26 j  */
    (char *) 0xcbeb4b6b,     /* 27 k  */
    (char *) 0xccec4c6c,     /* 28 l  */
    (char *) 0xbabb3a3b,     /* 29 ;  */
    (char *) 0xa2a72227,     /* 2a '  */
    0,
    0,
    vt52_appl_kp_4,          /* 2d Keypad 4 */
    vt52_appl_kp_5,          /* 2e Keypad 5 */
    vt52_appl_kp_6,          /* 2f Keypad 6 */
    0,
    (char *) 0xdafa5a7a,     /* 31 z  */
    (char *) 0xd8f85878,     /* 32 x  */
    (char *) 0xc3e34363,     /* 33 c  */
    (char *) 0xd6f65676,     /* 34 v  */
    (char *) 0xc2e24262,     /* 35 b  */
    (char *) 0xceee4e6e,     /* 36 n  */
    (char *) 0xcded4d6d,     /* 37 m  */
    (char *) 0xbcac3c2c,     /* 38 ,  */
    (char *) 0xbeae3e2e,     /* 39 .  */
    (char *) 0xbfaf3f2f,     /* 3a ? */
    0,
    vt52_appl_kp_dot,        /* 3c Keypad . */
    vt52_appl_kp_7,          /* 3d Keypad 7 */
    vt52_appl_kp_8,          /* 3e Keypad 8 */
    vt52_appl_kp_9           /* 3f Keypad 9 */
  };

/***
* High tables
***/

static unsigned char vt52_appl_hi_capsable [] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

static unsigned char vt52_appl_hi_repeatable [] =
  {
    0xef,0xf4,0xff,0xff,0x00,0x00,0x00,0x00
  };

static unsigned char vt52_appl_hi_map_types [] =
  {
    KCF_ALT,        /* 40 Space        */
    KCF_CONTROL,    /* 41 Backspace    */
    KC_NOQUAL,      /* 42 Tab          */
    KCF_STRING,     /* 43 Enter        */
    KCF_CONTROL,    /* 44 Return       */
    KCF_ALT,        /* 45 Escape       */
    KCF_CONTROL,    /* 46 Delete       */
    0,
    0,
    0,
    KCF_STRING,     /* 4a Numeric Pad -*/
    0,
    KCF_STRING,     /* 4c Cursor Up    */
    KCF_STRING,     /* 4d Cursor Down  */
    KCF_STRING,     /* 4e Cursor Frwrd */
    KCF_STRING,     /* 4f Cursor Bkwrd */
    KCF_STRING,     /* 50 F1           */
    KCF_STRING,     /* 51 F2           */
    KCF_STRING,     /* 52 F3           */
    KCF_STRING,     /* 53 F4           */
    KCF_STRING,     /* 54 F5           */
    KCF_STRING,     /* 55 F6           */
    KCF_STRING,     /* 56 F7           */
    KCF_STRING,     /* 57 F8           */
    KCF_STRING,     /* 58 F9           */
    KCF_STRING,     /* 59 F10          */
    KCF_STRING,     /* 5a Keypad (     */
    KCF_STRING,     /* 5b Keypad )     */
    KCF_STRING,     /* 5c Keypad /     */
    KCF_STRING,     /* 5d Keypad *     */
    KCF_STRING,     /* 5e Keypad +     */
    KCF_NOP,        /* 5f Help         */
    KCF_NOP,        /* 60 Left Shift   */
    KCF_NOP,        /* 61 Right Shift  */
    KCF_NOP,        /* 62 Caps Lock    */
    KCF_NOP,        /* 63 Control      */
    KCF_NOP,        /* 64 Left Alt     */
    KCF_STRING,     /* 65 Right Alt    */
    KCF_NOP,        /* 66 Left Amiga   */
    KCF_NOP         /* 67 Right Amiga  */
  };

static unsigned char *vt52_appl_hi_map [] = 
  {
    (char *) 0x0000a020,            /* 40 Space         */
    (char *) 0x0000087f,            /* 41 BackSpace     */
    (char *) 0x00000009,            /* 42 Tab           */
    vt52_appl_kp_enter,            /* 43 Enter         */
    (char *) 0x00000a0d,            /* 44 Return        */
    (char *) 0x00009b1b,            /* 45 Escape        */
    (char *) 0x0000087f,            /* 46 Delete        */
    0,
    0,
    0,
    vt52_appl_kp_minus,   /* 4a Numeric Pad - */
    0,
    vt52_cursor_up,        /* 4c Cursor Up     */
    vt52_cursor_down,      /* 4d Cursor Down   */
    vt52_cursor_right,     /* 4e Cursor Frwrd  */
    vt52_cursor_left,      /* 4f Cursor Bkwrd  */
    vt52_F1,               /* 50 F1            */
    vt52_F2,               /* 51 F2            */
    vt52_F3,               /* 52 F3            */
    vt52_F4,               /* 53 F4            */
    function_keys[0],      /* 54 F5            */
    function_keys[1],      /* 55 F6            */
    function_keys[2],      /* 56 F7            */
    function_keys[3],      /* 57 F8            */
    function_keys[4],      /* 58 F9            */
    function_keys[5],      /* 59 F10           */
    vt52_F1,               /* 5a Keypad (      */
    vt52_F2,               /* 5b Keypad )      */
    vt52_F3,               /* 5c Keypad /      */
    vt52_F4,               /* 5d Keypad *      */
    vt52_appl_kp_comma,    /* 5e Keypad +      */
    0,                     /* 5f Help          */
    0,                     /* 60 Left Shift    */
    0,                     /* 61 Right Shift   */
    0,                     /* 62 Caps Lock     */
    0,                     /* 63 Control       */
    0,                     /* 64 Left Alt      */
    vt52_appl_kp_comma,    /* 65 Right Alt     */
    0,                     /* 66 Left Amiga    */
    0                      /* 67 Right Amiga   */
  };

void SetKeyboard ()
  {
    if ( nvmodes.decanm >= VT100 )
      {
        if ( tmodes.deckpm == APPLIC )
          {
            KeyMap.km_LoKeyMapTypes  = (UBYTE *) vt100_appl_lo_map_types;
            KeyMap.km_LoKeyMap       = (ULONG *) vt100_appl_lo_map;
            KeyMap.km_LoCapsable     = (UBYTE *) vt100_appl_lo_capsable;
            KeyMap.km_LoRepeatable   = (UBYTE *) vt100_appl_lo_repeatable;
            KeyMap.km_HiKeyMapTypes  = (UBYTE *) vt100_appl_hi_map_types;
            if ( tmodes.decckm == RESET )
                KeyMap.km_HiKeyMap   = (ULONG *) vt100_ansi_appl_hi_map;
            else
                KeyMap.km_HiKeyMap   = (ULONG *) vt100_appl_hi_map;
            KeyMap.km_HiCapsable     = (UBYTE *) vt100_appl_hi_capsable;
            KeyMap.km_HiRepeatable   = (UBYTE *) vt100_appl_hi_repeatable;
          }
        else /* Numeric mode */
          {
            KeyMap.km_LoKeyMapTypes  = (UBYTE *) vt100_appl_lo_map_types;
            KeyMap.km_LoKeyMap       = (ULONG *) vt100_appl_lo_map;
            KeyMap.km_LoCapsable     = (UBYTE *) vt100_lo_capsable;
            KeyMap.km_LoRepeatable   = (UBYTE *) vt100_lo_repeatable;
            KeyMap.km_HiKeyMapTypes  = (UBYTE *) vt100_hi_map_types;
            if ( tmodes.decckm == RESET )
                KeyMap.km_HiKeyMap   = (ULONG *) vt100_ansi_hi_map;
            else
                KeyMap.km_HiKeyMap   = (ULONG *) vt100_hi_map;
            KeyMap.km_HiCapsable     = (UBYTE *) vt100_hi_capsable;
            KeyMap.km_HiRepeatable   = (UBYTE *) vt100_hi_repeatable;
          }

        if ( tmodes.decarm == RESET )
          {
            KeyMap.km_LoRepeatable   = (UBYTE *) no_repeat;
            KeyMap.km_HiRepeatable   = (UBYTE *) no_repeat;
          }
        
        SetFKey ( vt100_F1 );
        SetFKey ( vt100_F2 );
        SetFKey ( vt100_F3 );
        SetFKey ( vt100_F4 );
        SetFunctionKey ( function_keys[0] );
        SetFunctionKey ( function_keys[1] );
        SetFunctionKey ( function_keys[2] );
        SetFunctionKey ( function_keys[3] );
        SetFunctionKey ( function_keys[4] );
        SetFunctionKey ( function_keys[5] );
        SetCursorKey ( vt100_cursor_up    );
        SetCursorKey ( vt100_cursor_down  );
        SetCursorKey ( vt100_cursor_right );
        SetCursorKey ( vt100_cursor_left  );
        SetAnsiCursorKey ( vt100_ansi_cursor_up    );
        SetAnsiCursorKey ( vt100_ansi_cursor_down  );
        SetAnsiCursorKey ( vt100_ansi_cursor_right );
        SetAnsiCursorKey ( vt100_ansi_cursor_left  );
        SetKPKey ( vt100_appl_kp_0, '0' );
        SetKPKey ( vt100_appl_kp_1, '1' );
        SetKPKey ( vt100_appl_kp_2, '2' );
        SetKPKey ( vt100_appl_kp_3, '3' );
        SetKPKey ( vt100_appl_kp_4, '4' );
        SetKPKey ( vt100_appl_kp_5, '5' );
        SetKPKey ( vt100_appl_kp_6, '6' );
        SetKPKey ( vt100_appl_kp_7, '7' );
        SetKPKey ( vt100_appl_kp_8, '8' );
        SetKPKey ( vt100_appl_kp_9, '9' );
        SetKPKey ( vt100_appl_kp_minus, '-' );
        SetKPKey ( vt100_appl_kp_comma, ',' );
        SetKPKey ( vt100_appl_kp_dot,   '.' );
        SetKPKey ( vt100_appl_kp_enter, '\r' );
        Set200Key ( vt200_help );
        Set200Key ( vt200_do   );
        SetESCandBsKey ( vt200_esc, '\x1b' );
        SetESCandBsKey ( vt200_bs,  '\x08' );
     }
    else /* VT52 MODE */
      {
        if ( tmodes.deckpm == APPLIC )
          {
            KeyMap.km_LoKeyMapTypes  = (UBYTE *) vt52_appl_lo_map_types;
            KeyMap.km_LoKeyMap       = (ULONG *) vt52_appl_lo_map;
            KeyMap.km_LoCapsable     = (UBYTE *) vt52_appl_lo_capsable;
            KeyMap.km_LoRepeatable   = (UBYTE *) vt52_appl_lo_repeatable;
            KeyMap.km_HiKeyMapTypes  = (UBYTE *) vt52_appl_hi_map_types;
            KeyMap.km_HiKeyMap       = (ULONG *) vt52_appl_hi_map;
            KeyMap.km_HiCapsable     = (UBYTE *) vt52_appl_hi_capsable;
            KeyMap.km_HiRepeatable   = (UBYTE *) vt52_appl_hi_repeatable;
          }
        else
          {
            KeyMap.km_LoKeyMapTypes  = (UBYTE *) vt52_lo_map_types;
            KeyMap.km_LoKeyMap       = (ULONG *) vt52_lo_map;
            KeyMap.km_LoCapsable     = (UBYTE *) vt52_lo_capsable;
            KeyMap.km_LoRepeatable   = (UBYTE *) vt52_lo_repeatable;
            KeyMap.km_HiKeyMapTypes  = (UBYTE *) vt52_hi_map_types;
            KeyMap.km_HiKeyMap       = (ULONG *) vt52_hi_map;
            KeyMap.km_HiCapsable     = (UBYTE *) vt52_hi_capsable;
            KeyMap.km_HiRepeatable   = (UBYTE *) vt52_hi_repeatable;
          }
      }

    con_kb_req.io_Command = CD_SETKEYMAP;
    con_kb_req.io_Length  = sizeof (struct KeyMap);
    con_kb_req.io_Data    = (APTR) &KeyMap;
    DoIO ( (struct IORequest *) &con_kb_req );
  }

void InitKeyboard ()
  {
    SetKeyboard ();
  }

void SaveOriginalKeyMap ()
  {
    con_kb_req.io_Command = CD_ASKKEYMAP;
    con_kb_req.io_Length  = sizeof ( struct KeyMap);
    con_kb_req.io_Data    = (APTR) &OrigKeyMap;
    DoIO ( (struct IORequest *) &con_kb_req );
  }

void UpdateFunctionKeys ()
  {
    unsigned short int i;
    unsigned char  *ptr,
                   *tmp_ptr;
    
    for ( i = 0; i < 6; i++ )
     {
       function_keys[i][0] = strlen (nvmodes.keystr[i] );
       function_keys[i][1] = 4;
       tmp_ptr = ptr = &function_keys[i][4];
       strcpy ( ptr, nvmodes.keystr[i] );
       while ( tmp_ptr < ptr + strlen ( ptr ) )
         {
           if ( *tmp_ptr == nvmodes.pfk_cr )
               *tmp_ptr = '\r';
           else if ( *tmp_ptr == nvmodes.pfk_nl )
               *tmp_ptr = '\n';
           tmp_ptr++;
         }
     }
  }

void SetFunctionKey ( ptr )
unsigned char *ptr;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[2] = 0;
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[2] = 5;
                    ptr[3] = 68;
                    ptr[68] = ESC;
                    ptr[69] = '[';
                    break;
                case 8:
                    ptr[2] = 4;
                    ptr[3] = 69;
                    ptr[69] = CSI;
                    break;
              }
            break;
      }
  }
  
void Set200Key ( ptr )
unsigned char *ptr;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[0] = 0;
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[0] = 5;
                    ptr[1] = 2;
                    ptr[2] = ESC;
                    ptr[3] = '[';
                    break;
                case 8:
                    ptr[0] = 4;
                    ptr[1] = 3;
                    ptr[3] = CSI;
                    break;
              }
            break;
      }
  }

void SetESCandBsKey ( ptr, value )
unsigned char *ptr;
unsigned char value;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[0] = 1;
            ptr[1] = 2;
            ptr[2] = value;
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[0] = 5;
                    ptr[1] = 2;
                    ptr[2] = ESC;
                    ptr[3] = '[';
                    break;
                case 8:
                    ptr[0] = 4;
                    ptr[1] = 3;
                    ptr[3] = CSI;
                    break;
              }
            break;
      }
  }

void SetKPKey ( ptr, numeric )
unsigned char *ptr;
unsigned char numeric;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[2] = 0;
            if ( tmodes.deckpm == APPLIC )
              {
                ptr[0] = 3;
                ptr[1] = 4;
                ptr[2] = 0;
                ptr[4] = ESC;
                ptr[5] = 'O';
              }
            else
              {
                ptr[0] = 1;
                ptr[1] = 4;
                ptr[2] = 0;
                ptr[4] = numeric;
              }
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    if ( tmodes.deckpm == APPLIC )
                      {
                        ptr[0] = 3;
                        ptr[1] = 4;
                        ptr[4] = ESC;
                        ptr[5] = 'O';
                      }
                    else
                      {
                        ptr[0] = 1;
                        ptr[1] = 4;
                        ptr[4] = numeric;
                      }
                    ptr[2] = 4;
                    ptr[3] = 7;
                    ptr[7] = ESC;
                    ptr[8] = '[';
                    break;
                case 8:
                    if ( tmodes.deckpm == APPLIC )
                      {
                        ptr[0] = 2;
                        ptr[1] = 5;
                        ptr[5] = SS3;
                      }
                    else
                      {
                        ptr[0] = 1;
                        ptr[1] = 4;
                        ptr[4] = numeric;
                      }
                    ptr[2] = 3;
                    ptr[3] = 8;
                    ptr[8] = CSI;
                    break;
              }
            break;
      }
  }

void SetAnsiCursorKey ( ptr )
unsigned char *ptr;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[0] = 3;
            ptr[1] = 4;
            ptr[2] = 0;
            ptr[4] = ESC;
            ptr[5] = '[';
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[0] = 3;
                    ptr[1] = 4;
                    ptr[4] = ESC;
                    ptr[5] = '[';
                    ptr[2] = 5;
                    ptr[3] = 7;
                    ptr[7] = ESC;
                    ptr[8] = '[';
                    break;
                case 8:
                    ptr[0] = 2;
                    ptr[1] = 5;
                    ptr[5] = CSI;
                    ptr[2] = 4;
                    ptr[3] = 8;
                    ptr[8] = CSI;
                    break;
              }
            break;
      }
  }

void SetCursorKey ( ptr )
unsigned char *ptr;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[0] = 3;
            ptr[1] = 4;
            ptr[2] = 0;
            ptr[4] = ESC;
            ptr[5] = 'O';
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[0] = 3;
                    ptr[1] = 4;
                    ptr[4] = ESC;
                    ptr[5] = 'O';
                    ptr[2] = 5;
                    ptr[3] = 7;
                    ptr[7] = ESC;
                    ptr[8] = '[';
                    break;
                case 8:
                    ptr[0] = 2;
                    ptr[1] = 5;
                    ptr[5] = SS3;
                    ptr[2] = 4;
                    ptr[3] = 8;
                    ptr[8] = CSI;
                    break;
              }
            break;
      }
  }

void SetFKey ( ptr )
unsigned char *ptr;
  {
    switch ( nvmodes.decanm )
      {
        case VT100:
            ptr[0] = 3;
            ptr[1] = 4;
            ptr[2] = 0;
            ptr[4] = ESC;
            ptr[5] = 'O';
            break;
        case VT200_7:
        case VT200_8:
            switch ( tmodes.ocontrols )
              {
                case 7:
                    ptr[0] = 3;
                    ptr[1] = 4;
                    ptr[4] = ESC;
                    ptr[5] = 'O';
                    ptr[2] = 5;
                    ptr[3] = 7;
                    ptr[7] = ESC;
                    ptr[8] = '[';
                    break;
                case 8:
                    ptr[0] = 2;
                    ptr[1] = 5;
                    ptr[5] = SS3;
                    ptr[2] = 4;
                    ptr[3] = 8;
                    ptr[8] = CSI;
                    break;
              }
            break;
      }
  }
  
  
void SetNormalBksp ()
  {
    vt100_hi_map_types     [1] = KCF_STRING;
    vt100_appl_hi_map_types[1] = KCF_STRING;
    vt100_hi_map_types     [6] = KCF_CONTROL;
    vt100_appl_hi_map_types[6] = KCF_CONTROL;
    
    vt100_hi_map          [1] = vt200_bs;
    vt100_ansi_hi_map     [1] = vt200_bs;
    vt100_appl_hi_map     [1] = vt200_bs;
    vt100_ansi_appl_hi_map[1] = vt200_bs;
    vt100_hi_map          [6] = (unsigned char *)0x0000087f;
    vt100_ansi_hi_map     [6] = (unsigned char *)0x0000087f;
    vt100_appl_hi_map     [6] = (unsigned char *)0x0000087f;
    vt100_ansi_appl_hi_map[6] = (unsigned char *)0x0000087f;
  }
    
void SetReverseBksp ()
  {
    vt100_hi_map_types     [1] = KCF_CONTROL;
    vt100_appl_hi_map_types[1] = KCF_CONTROL;
    vt100_hi_map_types     [6] = KCF_STRING;
    vt100_appl_hi_map_types[6] = KCF_STRING;
    
    vt100_hi_map          [1] = (unsigned char *)0x0000087f;
    vt100_ansi_hi_map     [1] = (unsigned char *)0x0000087f;
    vt100_appl_hi_map     [1] = (unsigned char *)0x0000087f;
    vt100_ansi_appl_hi_map[1] = (unsigned char *)0x0000087f;
    vt100_hi_map          [6] = vt200_bs;
    vt100_ansi_hi_map     [6] = vt200_bs;
    vt100_appl_hi_map     [6] = vt200_bs;
    vt100_ansi_appl_hi_map[6] = vt200_bs;
  }




