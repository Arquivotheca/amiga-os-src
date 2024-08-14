/* -----------------------------------------------------------------------
 *
 *
 * test program to develop efficient screen/window opening. In this case
 * it's especially for rlogin but it could/should be standardized to work
 * for anything under 2.0
 *
 *------------------------------------------------------------------------
 */



#ifdef DEBUG
	#define DB1(x)  printf(x)
#else
	#define DB1(x) ;
#endif

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>

#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <graphics/rastport.h>
#include <exec/ports.h> 
#include <ctype.h>
#include <string.h>

#include <libraries/gadtools.h>
#include <utility/tagitem.h>

#include <ios1.h>
#include <proto/all.h>

#ifdef min
   #undef min
#endif

#define MAXSOCKS 10L

#include <stdio.h>   /* after the #undef min */
#include <errno.h>
 
#include "rlglobal.h"

 /* Messages sent via out-of-band data. */
#define TIOCPKT_FLUSHREAD    0x01    /* flush pending kybd data    */
#define TIOCPKT_FLUSHWRITE   0x02    /* flush all data to mark     */
#define TIOCPKT_STOP         0x04
#define TIOCPKT_START        0x08
#define TIOCPKT_NOSTOP       0x10
#define TIOCPKT_DOSTOP       0x20
#define TIOCPKT_WINDOW       0x80    /* request window size        */

#define ASKEXIT(x) (AskExit((LONG)(20 + 20 +(((x)->rl_fontwide) * 29)), 50L ))
#define ESC_SEQ_TERMINATOR(x) (((x) >= 0x40) && ((x) <= 0x7e)) 

	/* if using a sole '9b' as the escape then these values */
	/* need to be reduced by one */

#define CON_CLOSEGAD(b) (((b)[2] == 0x31 && (b)[3] == 0x31))
#define CON_RESIZE(b)   (((b)[2] == 0x31 && (b)[3] == 0x32))
#define CON_GOTMENU(b)  (((b)[2] == 0x31 && (b)[3] == 0x30))
#define CONCLIP_TERM    0x76  /* 'v'  */
#define CONCLIP_PENDING(b) (((b)[2] == 0x30 && (b)[3] == 0x20 && (b)[4] == 0x76))

	/* -------------------------------------------------------*/
	/* for gethostent - using this local version rather than  */
	/* the libray version.  Library version was crashing on   */
	/* 68000 machines. This was caused by the                 */
	/*                                                        */
	/*          char ghe_hostaddr[MAXADDRSIZE];               */
	/*                                                        */
	/* array being NOT word-aligned. When gethostent() tried  */
	/* write a LONG value into (LONG *)ghe_hostaddr, BANG!    */
	/* Solved (for now) by making the ghe_hostaddr[] array a  */
	/* global for RLOGIN.  The question remains as to why the */
	/* compiler is not aligning this.                         */
	/* -------------------------------------------------------*/

#define	MAX_ALIASES	35
#define	MAXADDRSIZE	14

#define OUTPUT(x) PutStr(x)

#define CSI               0x9b
#define CON_IDCMP_REPORT  0x7c
#define CON_OPTION_ON     0x68  /* lower case 'h' */
#define CON_OPTION_OFF    0x6c  /* lower case 'l' */
#define CON_SPECIAL_KEY   0x7e  /* tilde */
#define TILDE             0x7e  /* tilde */

UBYTE menu_on[]         = { '\x9b', '1','0','{', } ;
UBYTE closegad_on[]     = { '\x9b', '1','1','{','\0' } ;
UBYTE closegad_off[]    = { '\x9b', '1','1','}','\0' } ;
UBYTE gimme_resize[]    = { '\x9b', '1','2','{','\0' } ;

UBYTE linewrap_on[]     = { '\x9b', '\x3f', '\x37', '\x68' } ;
UBYTE linewrap_off[]    = { '\x9b', '\x3f', '\x37', '\x6c' } ;
UBYTE lfmode_on[]       = { '\x9b', '\x32', '\x30', '\x68' } ; 
UBYTE lfmode_off[]      = { '\x9b', '\x32', '\x30', '\x6c' } ; 
UBYTE cursor_off[]      = { '\x9b', '\x30','\x20', '\x70' } ;
UBYTE cursor_on[]       = { '\x9b', '\x20', '\x70'} ;

BYTE *con_stream_delim = ";\n\r\t" ;

