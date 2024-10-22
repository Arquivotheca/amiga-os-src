
#include "JISConvert_rev.h"
//#include "JISConvert_text.h"

struct GeneralData
    {
    struct Screen *screen;
    APTR VisualInfo;
    struct Window *win;
    struct Gadget *G1_FstGadget;
    struct Gadget *G2_FstGadget;
    struct Menu *MStrip;
    struct TextFont *TextFont;
    UBYTE *inbuf;                       /* input buffer for conversion */
    UBYTE *outbuf;                      /* output buffer for conversion */
    };

struct strbuffer
    {
    char sstr_in[32] ;
    char sstr_out[32];
    char fstr_in[64];                   /* buffer for input file name */
    char fstr_out[64];                  /* buffer for output file name */
    };


/* gadgets IDs */

#define ID_FILTER	0
#define ID_INPUT	1
#define ID_OUTPUT	2
#define ID_CODE		3
#define ID_CODE_EUC	0
#define ID_CODE_SJIS	1
#define ID_CODE_NEWJIS	2
#define ID_CODE_OLDJIS	3
//#define ID_CODE_NECJIS	4
#define ID_CSTR		4
#define ID_CONVERT	5
#define ID_QUIT		6
#define ID_ISSTR	7
#define ID_OSSTR	8

#define ID_statbar	0
#define ID_fileout	1
#define ID_filein	2
#define ID_abort	3
#define ID_tostr	4
#define ID_constr	5


/* index for gadgets structure array */

#define G_FILTER	0
#define G_INPUT		1
#define G_OUTPUT	2
#define G_CODE		3
#define G_CSTR		4
#define G_CONVERT	5
#define G_QUIT		6
#define G_ISTR		7
#define G_OSTR		8

#define G_STATBAR	0
#define G_ABORT		1
#define G_TOSTR		2
#define G_CONSTR	3


/* index for array of extention */

#define	EUC	0
#define SJIS	1
#define NEWJIS	2
#define OLDJIS	3

//#define	NECJIS	4

/* window dim */

#define WINDOW_W	480
#define WINDOW_H	270
//#define WINDOW_H	182

/* ASL requester dim */

#define FR_HEIGHT       WINDOW_H
#define FR_WIDTH        250
#define FR_LEFT         0
#define FR_TOP          0

/* bevel box dim */

#define BBOX_LEFT	16
#define BBOX_TOP	180
#define BBOX_W		456
#define BBOX_H		60

#define BBOX_RIGHT	BBOX_LEFT + BBOX_W - 2
#define BBOX_BOT	BBOX_TOP + BBOX_H - 2


//#define STAT1_LEFT	BBOX_LEFT + 80
//#define STAT1_TOP	BBOX_TOP + 4
//#define STAT1_RIGHT	BBOX_LEFT + BBOX_W - 2
//#define STAT1_BOT	BBOX_TOP + BBOX_H / 2

//#define STAT2_LEFT	BBOX_LEFT + 80
//#define STAT2_TOP	BBOX_TOP + 22
//#define STAT2_RIGHT	BBOX_LEFT + BBOX_W - 2
//#define STAT2_BOT	BBOX_TOP + BBOX_H - 2


/* status bar (during conversion) dim */

#define SBAR_LEFT	80
#define SBAR_TOP	128
#define SBAR_W		320
#define SBAR_H		20

#define SBAR_RIGHT	SBAR_LEFT + SBAR_W - 2
#define SBAR_BOT	SBAR_TOP + SBAR_H - 2

/* display area (during conversion) dim */
#define WIN_LEFT	10
#define WIN_TOP		0
#define WIN_RIGHT	WINDOW_W - 2
#define WIN_BOT		WINDOW_H - 2

/* input file display (during conversion) area dim */

#define FIN_LEFT	WIN_LEFT
#define FIN_RIGHT	WIN_RIGHT
#define FIN_TOP		26
#define FIN_BOT		FIN_TOP + 40 - 2

/* output file display (during conversion) area dim */

#define FOUT_LEFT	WIN_LEFT
#define FOUT_RIGHT	WIN_RIGHT
#define FOUT_TOP	86
#define FOUT_BOT	FOUT_TOP + 40 - 2

/* input/output buffer size */

#define BUFSIZE		1024

/* error code returned by ConvertFile */

#define NOERROR		0
#define UNKWNCODE	1
#define NONTEXT		2
#define ASCIITEXT	3
#define ERRCOPY		4
#define USRABORT	5
#define ERROPENIN	6
#define ERROPENOUT	7
#define DIFFCODE	8

/* number of bytes to display in sample box */

#define LLENGTH		54

VOID DispHelp (VOID);
BOOL OpenScreenWB(struct GeneralData *);
VOID CloseScreenWB(struct GeneralData *);
VOID cleanexit(struct GeneralData *, int );
LONG TM_Request(struct Window *, UBYTE *, UBYTE *, UBYTE *, ULONG *, APTR , ...);
BOOL OpenWin(struct GeneralData *);
VOID CloseWin(struct GeneralData *);
VOID HandleEvent(struct GeneralData *);
BOOL ProcWindowSignal (struct GeneralData *);
VOID RenderBBox (struct Window *, SHORT, SHORT, SHORT, SHORT, ULONG , ...);
BOOL CreateGadgetList1 (struct GeneralData *);
BOOL CreateGadgetList2 (struct GeneralData *);
VOID DispText (struct Window *, UBYTE *, WORD, WORD);
VOID DispTextCenter (struct Window *, UBYTE *, WORD );
VOID ClearRect (struct Window *, SHORT, SHORT, SHORT, SHORT, UBYTE);
VOID DoConvertion(struct GeneralData *, struct strbuffer *, struct FileRequester *, struct FileRequester *);
UBYTE ConvertFile (struct GeneralData *, struct strbuffer *, ULONG );
BOOL CheckUsrAbort (struct GeneralData *);
LONG DetCodeSet (struct JConversionCodeSet *, BPTR, UBYTE *);
//VOID DispFSelStatus ( struct GeneralData *, struct strbuffer *, struct FileRequester *);
//VOID DispDSelStatus ( struct GeneralData *, struct strbuffer *, struct FileRequester *);
VOID DispSampleText (struct GeneralData *, UBYTE *, WORD, WORD);
VOID DispCharLine (struct GeneralData *, LONG, LONG, BPTR);

