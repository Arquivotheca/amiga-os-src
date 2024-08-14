#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <graphics/gfxmacros.h>
#include "renderinfo.h"
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <fctype.h>
#include <stdlib.h>
#undef GLOBAL

/* Kludges to allow use of ASL under 2.0 */
extern struct Library *AslBase;
struct FileRequester	{
			BYTE	*rf_Hail;		/* Hailing text			*/
			BYTE	*rf_File;		/* Filename array (FCHARS + 1)	*/
			BYTE	*rf_Dir;		/* Directory array (DSIZE + 1)	*/
		struct	Window	*rf_Window;		/* Window requesting or NULL	*/
			UBYTE	rf_FuncFlags;		/* Set bitdef's below		*/
			UBYTE	rf_Flags2;		/* New flags...			*/
			VOID	(*rf_Function)();	/* Your function, see bitdef's	*/
			WORD	rf_LeftEdge;		/* To be used later...		*/
			WORD	rf_TopEdge;
			};

struct FileRequester * AllocFileRequest (void);
void                   FreeFileRequest(struct FileRequester *);
char *                 RequestFile(struct FileRequester *);
#pragma libcall AslBase AllocFileRequest 1e 0
#pragma libcall AslBase FreeFileRequest 24 801
#pragma libcall AslBase RequestFile 2a 801
/* ------ END OF KLUDGE ------ */

#define NOVAL     0x55555555
#define MAXSTATES 0x40
#define MAXSTRING 0x60
#define MAXBUF    100
#define MAXMSG    64
#define LISTMAX   15

union strnum {
   char *text;
   long value;
};

struct GLOBAL {
   struct Window *window;
   struct RastPort *rp;
   struct Gadget   *gadlist;
   struct Remember *gadmem;
   struct Remember *strmem;
   struct FileRequester *filereq;
   int    gadcount;
   int    reset;
   int    scrnum;
   UBYTE  highlight;
   UBYTE  shadow;
   UBYTE  textpen;
   UBYTE  backpen;
   char   message1[MAXMSG];
   char   message2[MAXMSG];
   char   message3[MAXMSG];
   WORD   left, right, top, bottom;
   struct RenderInfo *rinfo;
   BPTR   olddir;
   int    setdir;
   char   states[MAXSTATES];
   union  strnum strtab[MAXSTRING];
};

struct cycdata {
   char slot;
   char count;
   char key;
   char string[1];
};
struct chkdata {
   char slot;
   char key;
   char string[1];
};

#define MULTI        "\x01"
#define MULTI_V         1
#define BUTTON       "\x02"
#define BUTTON_V        2
#define NEWCOLUMN    "\x03"
#define NEWCOLUMN_V     3
#define CHECK        "\x04"
#define CHECK_V         4
#define LIST         "\x05"
#define LIST_V          5
#define NLIST        "\x06"
#define NLIST_V         6
#define STRING       "\x07"
#define STRING_V        7
#define NUMERIC      "\x08"
#define NUMERIC_V       8
#define ADJUST       "\x09"
#define ADJUST_V        9
#define UP_V           10
#define DOWN_V         11
#define STR_V          12
#define NEW_V          13
#define DEL_V          14
#define ITEM1_V        15
#define ITEM2_V        16
#define ITEM3_V        17
#define SCROLL_V       18
#define NUM_V          19
#define NITEM1_V       20
#define NITEM2_V       21
#define NITEM3_V       22

#define BTN_ADV      "\x01"
#define BTN_ADV_V       1
#define BTN_OBJ      "\x02"
#define BTN_OBJ_V       2
#define BTN_SAVE     "\x03"
#define BTN_SAVE_V      3
#define BTN_DEFAULT  "\x04"
#define BTN_DEFAULT_V   4
#define BTN_CANCEL   "\x05"
#define BTN_CANCEL_V    5
#define BTN_USE      "\x06"
#define BTN_USE_V       6
#define BTN_ALTCAN   "\x07"
#define BTN_ALTCAN_V    7
#define BTN_OK       "\x08"
#define BTN_OK_V        8

#define NCYCLE_V     0x37
#define NCYCLE       "\x37"



/* Prototypes for functions defined in sascopts.c */
void main(void);
void fatal(struct GLOBAL *global,
           char *str);

/* Prototypes for functions defined in sascstate.c */
void handlehit(struct GLOBAL *global,
               struct Gadget *gadget,
               int back);

void handlekey(struct GLOBAL *global,
               int key,
               int back);

void handlemenu(struct GLOBAL *global,
                int menunum,
                int itemnum);
void resetall(struct GLOBAL *global);

/* Prototypes for functions defined in sascrender.c */
void rendertext(struct RastPort *rp,
                char *text,
                int key,
                int ulx,
                int uly);
void drawbox(struct RastPort *rp,
             int ulx,
             int uly,
             int lrx,
             int lry,
             int pen1,
             int pen2);
void rendercyc(struct GLOBAL *global,
               struct Gadget *gadget);
void renderchk(struct GLOBAL *global,
               struct Gadget *gadget);
void renderbtn(struct GLOBAL *global,
               struct Gadget *gadget);
void renderbox(struct GLOBAL *global,
               struct Gadget *gadget);
void renderstrnum(struct GLOBAL *global,
                  struct Gadget *gadget);
void renderlist(struct GLOBAL *global,
                struct Gadget *gadget);
void rendernlist(struct GLOBAL *global,
                 struct Gadget *gadget);
void refresh(struct GLOBAL *global);

void render(struct GLOBAL *global,
            struct Gadget *gadget);

/* Prototypes for functions defined in sascscoll.c */
void resetscroll(struct GLOBAL *global,
                 struct Gadget *gadget);
void readscroll(struct GLOBAL *global,
                struct Gadget *gadget);

/* Prototypes for functions defined in sascsetup.c */
void *gmem(struct GLOBAL *global,
           int size);
void *strmem(struct GLOBAL *global,
           int size);
void newgad(struct GLOBAL *global,
            int x,
            int y,
            int gadtype,
            char *data,
            int width,
            int height);
void freegads(struct GLOBAL *global);
void setup(struct GLOBAL *global);
void setstate(struct GLOBAL *global,
              int state,
              int id,
              int button);

struct Gadget *findgad(struct GLOBAL *global,
                       int id,
                       int button);

int strslot(struct GLOBAL *global,
            int button);

/* Prototypes for functions defined in propgad.c */
VOID FindPropValues(USHORT total,
                    USHORT visible,
                    USHORT first,
                    USHORT *body,
                    USHORT *pot);
USHORT FindPropFirst(USHORT total,
                             USHORT visible,
                             USHORT pot);


/* Prototypes for functions defined in sascsave.c */
int saveopts(struct GLOBAL *global,
             char *name);



/* Prototypes for functions defined in sascload.c */
int agetc(BPTR fp);
int loadopts(struct GLOBAL *g, 
             char *s,
             int flag);
int opterr(struct GLOBAL *g, 
           char *s, 
           int lno);
void install_s(struct GLOBAL *global,
               int index,
               char * message,
               BPTR fp ,
               int lno);

int findopt(struct GLOBAL *global,
            int c,
            char *look,
            char *slot,
            char *msg,
            int lno);
