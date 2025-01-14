/*
 * This file is the general header file for
 * all parts of the MicroEMACS display editor. It contains
 * definitions used by everyone, and it contains the stuff
 * you have to edit to create a version of the editor for
 * a specific operating system and terminal.
 */

/* standard include file standard */

#define	V7	0			/* V7 UN*X or Coherent		*/
#define	VMS	0			/* VAX/VMS			*/
#define	CPM	0			/* CP/M-86			*/
#define	MSDOS	0			/* MS-DOS			*/
#define	AMIGA	1			/* Amiga			*/

#define	ANSI	1			/* ANSI terminal.		*/
#define	VT52	0			/* VT52 terminal (Zenith).	*/
#define	VT100	0			/* Handle VT100 style keypad.	*/
#define	LK201	0			/* Handle LK201 style keypad.	*/
#define	RAINBOW	0			/* Use Rainbow fast video.	*/
#define TERMCAP 0                       /* Use TERMCAP                  */

#define	CVMVAS	1			/* C-V, M-V arg. in screens.	*/

#if AMIGA
#include "standard.h"

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#define MAXFILEBUF 4096
#define MAXLINE 1024
#define  ID_FTXT	MAKE_ID('F','T','X','T')
#define  ID_CHRS	MAKE_ID('C','H','R','S')

extern	int	FontWidth;
extern	int	FontHeight;
extern struct FileInfoBlock *finfo;

extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;

#ifdef ADVANCED_EMACS
#define TEMPLATE "FROM/M,OPT/K,FULL/S,WINDOW/S,GOTO/K" CMDREV
#define OPT_FROM	0
#define OPT_OPT		1
#define OPT_FULL	2
#define OPT_WINDOW	3
#define OPT_GOTO	4
#define OPT_COUNT	5
#else
#define TEMPLATE "FROM/M,OPT/K,GOTO/K" CMDREV
#define OPT_FROM	0
#define OPT_OPT		1
#define OPT_GOTO	2
#define OPT_COUNT	3
#endif
#define CMDREV  "\0$VER: " VSTRING

#define  TYPE_ARGS	23		/* passing arguments     */
#define  TYPE_READ	25		/* read the clipboard	 */
#define  TYPE_REPLY	27		/* reply to a TYPE_READ  */

struct CHRSChunk {
    struct MinNode mn;
    char *text;                 /* pointer to start of text */
    char *freeme;               /* if non-zero, call FreeVec() with this pointer */
    ULONG  size;                /* total # of characters in this block */
    ULONG  flags;               /* may end up needing this at some time */
    ULONG  UserLink;
    char  data[1];              /* really 'n' # of bytes                */
};

struct cmdMsg {
    struct Message cm_Msg;
    long                cm_type;        /* type of message, and return validation */
    long                cm_error;       /* error code return */
    struct MinList      cm_Chunks;

};

#else
#define WORD	short
#define UWORD	short
#define SHORT 	short
#define UBYTE	char
#define BYTE	char
#define LONG	long
#define ULONG	long
#define VOID	void

struct IntuiText
{
UBYTE FrontPen, BackPen;
UBYTE DrawMode;
SHORT LeftEdge;
SHORT TopEdge;
UBYTE *ITextFont;
UBYTE *IText;
struct IntuiText *NextText;
};


#endif

#define	NFILEN	80			/* # of bytes, file name	*/
#define	NBUFN	16			/* # of bytes, buffer name	*/
#define	NLINE	256			/* # of bytes, line		*/
#define	NKBDM	256			/* # of strokes, keyboard macro	*/
#define	NPAT	80			/* # of bytes, pattern		*/
#define NBINDS	256			/* # of key bindings allowed	*/
#define NBINDW	512			/* # of words needed for table */
#define	HUGE	1000			/* Huge number			*/

#define	AGRAVE	0x60			/* M- prefix,   Grave (LK201)	*/
#define	METACH	0x1B			/* M- prefix,   Control-[, ESC	*/
#define	METACH8	0x9B			/* M- prefix,   Control-[, ESC	*/
#define	CTMECH	0x1C			/* C-M- prefix, Control-\	*/
#define	EXITCH	0x1D			/* Exit level,  Control-]	*/
#define	CTRLCH	0x1E			/* C- prefix,	Control-^	*/
#define	HELPCH	0x1F			/* Help key,    Control-_	*/

#define	CTRL	0x0100			/* Control flag, or'ed in	*/
#define	META	0x0200			/* Meta flag, or'ed in		*/
#define	CTLX	0x0400			/* ^X flag, or'ed in		*/

#ifndef FALSE
#define	FALSE	0			/* False, no, bad, etc.		*/
#endif

#ifndef TRUE
#define	TRUE	1			/* True, yes, good, etc.	*/
#endif

#define	ABORT	2			/* Death, ^G, abort, etc.	*/
#define CONT    3			/* great, do it, etc		*/

#define	FIOSUC	0			/* File I/O, success.		*/
#define	FIOFNF	1			/* File I/O, file not found.	*/
#define	FIOEOF	2			/* File I/O, end of file.	*/
#define	FIOERR	3			/* File I/O, error.		*/

#define	CFCPCN	0x0001			/* Last command was C-P, C-N	*/
#define	CFKILL	0x0002			/* Last command was a kill	*/

/*
 * There is a window structure allocated for
 * every active display window. The windows are kept in a
 * big list, in top to bottom screen order, with the listhead at
 * "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands
 * to guide redisplay; although this is a bit of a compromise in
 * terms of decoupling, the full blown redisplay is just too
 * expensive to run for every input character. 
 */

typedef	struct	EWINDOW {
	struct	EWINDOW *w_wndp;		/* Next window			*/
	struct	BUFFER *w_bufp;		/* Buffer displayed in window	*/
	struct	LINE *w_linep;		/* Top line in the window	*/
	struct	LINE *w_dotp;		/* Line containing "."		*/
	WORD	w_doto;			/* Byte offset for "."		*/
	struct	LINE *w_markp;		/* Line containing "mark"	*/
	WORD	w_marko;		/* Byte offset for "mark"	*/
	WORD	w_toprow;		/* Origin 0 top row of window	*/
	WORD	w_ntrows;		/* # of rows of text in window	*/
	WORD	w_force;		/* If NZ, forcing row.		*/
	BYTE	w_flag;			/* Flags.			*/
}	EWINDOW;

#define	WFFORCE	0x01			/* Window needs forced reframe	*/
#define	WFMOVE	0x02			/* Movement from line to line	*/
#define	WFEDIT	0x04			/* Editing within a line	*/
#define	WFHARD	0x08			/* Better to a full display	*/
#define	WFMODE	0x10			/* Update mode line.		*/

/*
 * Text is kept in buffers. A buffer header, described
 * below, exists for every buffer in the system. The buffers are
 * kept in a big list, so that commands that search for a buffer by
 * name can find the buffer header. There is a safe store for the
 * dot and mark in the header, but this is only valid if the buffer
 * is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with
 * a pointer to the header line in "b_linep".
 */
typedef	struct	BUFFER {
	struct	BUFFER *b_bufp;		/* Link to next BUFFER		*/
	struct	LINE *b_dotp;		/* Link to "." LINE structure	*/
	WORD	b_doto;			/* Offset of "." in above LINE	*/
	struct	LINE *b_markp;		/* The same as the above two,	*/
	WORD	b_marko;		/* but for the "mark"		*/
	struct	LINE *b_linep;		/* Link to the header LINE	*/
	UBYTE	b_nwnd;			/* Count of windows on buffer	*/
	UBYTE	b_flag;			/* Flags			*/
	UBYTE	b_mode;			/* Modes			*/
	UBYTE	b_extra;		/* I''ll need it someday	*/
	ULONG	b_protection;		/* protection bits for buffer   */
	UBYTE	b_fname[NFILEN];	/* File name			*/
	UBYTE	b_bname[NBUFN];		/* Buffer name			*/
}	BUFFER;

#define	BFTEMP	0x01			/* Internal temporary buffer	*/
#define	BFCHG	0x02			/* Changed since last write	*/

#define CMODE		0x01
#define WRAPMODE	0x02		/* word wrap mode */
#define STRIKEMODE 	0x04
#define READMODE 	0x08

/*
 * The starting position of a
 * region, and the size of the region in
 * characters, is kept in a region structure.
 * Used by the region commands.
 */
typedef	struct	{
	struct	LINE *r_linep;		/* Origin LINE address.		*/
	UWORD	r_offset;		/* Origin LINE offset.		*/
	ULONG	r_size;			/* Length in characters.	*/
}	REGION;

/*
 * All text is kept in circularly linked
 * lists of "LINE" structures. These begin at the
 * header line (which is the blank line beyond the
 * end of the buffer). This line is pointed to by
 * the "BUFFER". Each line contains a the number of
 * bytes in the line (the "used" size), the size
 * of the text array, and the text. The end of line
 * is not stored as a byte; it's implied. Future
 * additions will include update hints, and a
 * list of marks into the line.
 */

typedef	struct	LINE {
	struct	LINE *l_fp;		/* Link to the next line	*/
	struct	LINE *l_bp;		/* Link to the previous line	*/
/*	char	*l_fp;		/* Link to the next line	*/
/*	char    *l_bp;		/* Link to the previous line	*/
	WORD	l_size;			/* Allocated size		*/
	WORD	l_used;			/* Used size			*/
	UBYTE	l_text[1];		/* A bunch of characters.	*/
}	LINE;

#define	lforw(lp)	((lp)->l_fp)
#define	lback(lp)	((lp)->l_bp)
#define	lgetc(lp, n)	((lp)->l_text[(n)]&0xFF)
#define	lputc(lp, n, c)	((lp)->l_text[(n)]=(c))
#define	llength(lp)	((lp)->l_used)

/*
 * The editor communicates with the display
 * using a high level interface. A "TERM" structure
 * holds useful variables, and indirect pointers to
 * routines that do useful operations. The low level get
 * and put routines are here too. This lets a terminal,
 * in addition to having non standard commands, have
 * funny get and put character code too. The calls
 * might get changed to "termp->t_field" style in
 * the future, to make it possible to run more than
 * one terminal type.
 */  
typedef	struct	{
	WORD	t_nrow;			/* Number of rows.		*/
	WORD	t_ncol;			/* Number of columns.		*/
	int	(*t_open)(void);		/* Open terminal at the start.	*/
	int	(*t_close)(void);		/* Close terminal at end.	*/
	int	(*t_getchar)(void);		/* Get character from keyboard.	*/
	int	(*t_putchar)(int c);		/* Put character to display.	*/
	int	(*t_flush)(void);		/* Flush output buffers.	*/
	int	(*t_move)(int row, int col);		/* Move the cursor, origin 0.	*/
	int	(*t_eeol)(void);		/* Erase to end of line.	*/
	int	(*t_eeop)(void);		/* Erase to end of page.	*/
	int	(*t_beep)(void);		/* Beep.			*/
}	TERM;

extern	int	currow;			/* Cursor row			*/
extern	int	curcol;			/* Cursor column		*/
extern	int	thisflag;		/* Flags, this command		*/
extern	int	lastflag;		/* Flags, last command		*/
extern	int	curgoal;		/* Goal for C-P, C-N		*/
extern	int	mpresf;			/* Stuff in message line	*/
extern	int	sgarbf;			/* State of screen unknown	*/
extern	EWINDOW	*curwp;			/* Current window		*/
extern	BUFFER	*curbp;			/* Current buffer		*/
extern	EWINDOW	*wheadp;		/* Head of list of windows	*/
extern	BUFFER	*bheadp;		/* Head of list of buffers	*/
extern	BUFFER	*blistp;		/* Buffer for C-X C-B		*/
extern	UWORD	kbdm[];			/* Holds kayboard macro data	*/
extern	UWORD	*kbdmip;		/* Input pointer for above	*/
extern	UWORD	*kbdmop;		/* Output pointer for above	*/
extern	UBYTE	pat[];			/* Search pattern		*/

extern	UBYTE	rpat[];			/* Replace pattern		*/
extern	TERM	term;			/* Terminal information.	*/
extern	int	QueryFlag;		/* for query search & replace 	*/
extern int	MacroLevel;
extern UBYTE     *fkdmip;                /* input for above 		*/
extern UBYTE    *fkdmop;                /* output for above     	*/
extern UBYTE    *cmdmop;                /* output for commands 		*/
extern int fkinflag;
extern	int     fillcol;                /* Fill column   		*/
extern	int	LeftMargin;
extern	int	RightMargin;
extern	int	BackupBefore;		/* backup before writing flag	*/

extern	BUFFER	*bfind();		/* Lookup a buffer by name	*/
extern	EWINDOW	*wpopup();		/* Pop up window creation	*/
extern	LINE	*lalloc();		/* Allocate a line		*/

#if AMIGA
#include "memacs_protos.h"
#include "string.h"
#else
#include "stdio.h"
#endif
