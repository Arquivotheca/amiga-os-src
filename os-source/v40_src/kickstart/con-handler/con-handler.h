#include "exec/memory.h"
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/execbase.h>
#include <exec/io.h>

#include "dos/dos.h"
#include "dos/dosextens.h"
#include "dos/filehandler.h"
#include "intuition/intuition.h"
#include "intuition/intuitionbase.h"
#include "intuition/screens.h"

#include "graphics/displayinfo.h"

#include "devices/timer.h"
#include "devices/serial.h"
#include "devices/parallel.h"
#include "devices/conunit.h"

#include "utility/tagitem.h"
#include "string.h"

#include "clib/dos_protos.h"
#include "clib/exec_protos.h"
#include "clib/intuition_protos.h"
#include "clib/graphics_protos.h"
#include "clib/utility_protos.h"

#include "pragmas/exec_old_pragmas.h"
#include "pragmas/intuition_pragmas.h"
#include "pragmas/dos_pragmas.h"
#include "pragmas/graphics_pragmas.h"
#include "pragmas/utility_pragmas.h"

#define DEBUG_ON
#define ED_PASTE
#define ED_PASTE1
#define ASYNC_KLUDGE	1
#define SEARCH	1

#define ACTION_ID	1
#define ACTION_PASTE	500
#define ACTION_GET_HIST	501
#define ACTION_SET_HIST	502
#define ACTION_SIZE_HIST	503
#define ACTION_GET_VARS	504
#define ACTION_SET_VARS	505

#define ACTION_CHANGE_SIGNAL	995

#define ACTION_FORCE	2001
#define ACTION_STACK	2002
#define ACTION_QUEUE	2003
#define ACTION_DROP	2004
#define ACTION_PEEK	2005
#define ACTION_REPLACE	2006

#define  TYPE_ARGS	23		/* passing arguments     */
#define  TYPE_READ	25		/* read the clipboard	 */
#define  TYPE_REPLY	27		/* reply to a TYPE_READ  */
#undef  BADDR
#define BADDR( bptr )   (((long)bptr) << 2)
#define BTOCSTR(bstr)   ((TEXT *)(BADDR(bstr) + 1))

#define ACTION_FIND_INPUT       1005
#define ACTION_FIND_OUTPUT      1006

#define DOS_TRUE	-1

#define ID_RAWCON ((('R' << 24L) | ('A' << 16L)) | ('W' << 8L))
#define ID_CON ((('C' << 24L) | ('O' << 16L)) | ('N' << 8L))


#define ctrl_a 1
#define ctrl_b 2
#define ctrl_c 3
#define ctrl_d 4
#define ctrl_e 5
#define ctrl_f 6
#define ctrl_i 8
#define ctrl_k 11
#define ctrl_q 17
#define ctrl_r 18
#define ctrl_s 19
#define ctrl_u 21
#define ctrl_w 23
#define ctrl_x 24
#define ctrl_y 25
#define ctrl_z 26

#define eof 0x1C   /*  Actually ^\ */

#define EXECBASE (*(struct ExecBase **)4)
#define THISPROC ((struct Process *)(EXECBASE->ThisTask))


struct CHRSChunk {
    struct MinNode mn;
    char *text;			/* pointer to start of text */
    char *freeme;		/* if non-zero, call FreeVec() with this pointer */
    ULONG  size;		/* total # of characters in this block */
    ULONG  flags;		/* may end up needing this at some time */
    ULONG  UserLink;
    char  data[1];		/* really 'n' # of bytes		*/
};

struct cmdMsg {
    struct Message cm_Msg;
    long 		cm_type;	/* type of message, and return validation */
    long 		cm_error;	/* error code return */
    struct MinList	cm_Chunks;

};

#define MAXCHARS 512
/* #define bmask 0x1FF */
#define init_bmask 0x1FF
#define bmask gv->bsize

/*#define hmask 0x7FF */
#define history_size 8	/* default memory for history */
#define HISTORY_BYTES 2048
#define hmask gv->hsize

struct Global {
  /* public variables, more or less */
  LONG version;
  LONG revision;

  BOOL EOFpend;
  BOOL CLOSEpend;

  LONG AUTOflag;
  LONG waitflag;
  LONG hold;

  LONG CLOSEflag;
  LONG usecount;
  LONG raw;
  struct Window *window;

  struct MsgPort *clienttaskid;
  struct MsgPort *writetaskid;

  /* more private; the history variables */
  LONG hsize;
  LONG history;
  LONG hscan;
  LONG lasthist;
  LONG hflag;
  LONG lastarrow;
  LONG h_erase_extra;

  /* the paste variables */
  struct MinList paste; /* conclip paste storage */
  struct MinNode *node;
  LONG pcount;

  LONG kill;

  LONG readp;
  LONG tailp;
  LONG echop;
  LONG headp;
  LONG direct;
  LONG bs_direct;
  LONG BSpend;
  LONG bsize;
  LONG ecount;

  struct ConUnit *cu;
  struct DosPacket *output_pkt;
  struct DosPacket *current_input;
  struct DosPacket *current_output;
  struct DosPacket *timeoutpkt;

  LONG flag1;
  LONG search;
  LONG h_backup;
  LONG history_count;

  LONG e_flag1;
  LONG e_flag2;
  struct DosPacket *save_current_input;
  struct DosPacket *save_current_output;

  struct IntuitionBase *IntuitionBase;
  struct Library *DOSBase;
  struct Library *UtilityBase;

  UBYTE ebuf[MAXCHARS];
  UBYTE killbuffer[MAXCHARS];
  UBYTE printbuffer[MAXCHARS*4];
  UBYTE stringmem[256];
  UBYTE *hbuffer;
  UBYTE titlemem[128];
  UBYTE namemem[140];
  UBYTE buffer[MAXCHARS]; /* general purpose buffer */
  union {
      struct IOStdReq io1;
      struct IOExtPar io2;
      struct IOExtSer io3;
  } IOB;
  union {
      struct IOStdReq io1;
      struct IOExtPar io2;
      struct IOExtSer io3;
  } IOBO;
};

#define history gv->history
#define hscan gv->hscan

#define lasthist gv->lasthist
#define hflag gv->hflag
#define lastarrow gv->lastarrow
#define h_erase_extra gv->h_erase_extra

#define kill gv->kill

#define readp gv->readp
#define tailp gv->tailp
#define echop gv->echop
#define headp gv->headp
#define direct gv->direct
#define bs_direct gv->bs_direct
#define BSpend gv->BSpend

#define EOFpend gv->EOFpend
#define CLOSEpend gv->CLOSEpend
#define ecount gv->ecount
#define usecount gv->usecount
#define raw gv->raw

#define IOB ((struct IOStdReq *)&gv->IOB)
#define IOBO ((struct IOStdReq *)&gv->IOBO)

#define window gv->window

#define clienttaskid gv->clienttaskid
#define writetaskid gv->writetaskid
#define output_pkt gv->output_pkt
#define current_input gv->current_input
#define current_output gv->current_output
#define save_current_input gv->save_current_input
#define save_current_output gv->save_current_output

#define timeoutpkt gv->timeoutpkt

#define h_backup gv->h_backup
#define search gv->search

#define UtilityBase gv->UtilityBase

/* misc.c functions */
struct Process * __regargs GetProcess(struct MsgPort *taskid);
void __regargs append(struct DosPacket **plist,struct DosPacket *pkt);
struct DosPacket *  __regargs pop(struct DosPacket **plist);
void __regargs dSendIO(struct IOStdReq *iob,struct DosPacket *packet,
	LONG command, APTR buffer, LONG length);
void __regargs StartPaste(struct Global *gv);
void __regargs KillPaste(struct Global *gv);
UBYTE __regargs GetPaste(struct Global *gv,int flag);
int __regargs PutPaste(struct Global *gv,int action,UBYTE *string,LONG len);

/* window.c functions */
struct Window * __regargs ParseParam(struct Global *gv,
	UBYTE *dev, LONG unit_no, LONG flags);
void __regargs CloseCon(struct Global *gv,struct Window *wind,
	struct IOStdReq *iob);

/* ansi.c functions */
VOID __regargs numtostring(struct Global *gv, int n, int start);
int __regargs addcursor(UBYTE *v,int start);
void __regargs fixbuf(struct IOStdReq *iobo, UBYTE *str);
int __regargs ReverseChar(struct Global *gv,UBYTE ch,int count);

/* history.c functions */
void __regargs AddHistoryLine(struct Global *gv,UBYTE *string, int len);

/* con.c functions */
VOID __regargs start(void);

void __regargs XCEXIT(int);
int CXBRK(void);

/* missing Exec */
void __regargs NewList(struct List *);

#define abs(x) ((x)<0?-(x):(x))

void __stdargs kprintf(UBYTE *fmt, ... );

#define getreg __builtin_getreg
extern long __regargs getreg(int);

