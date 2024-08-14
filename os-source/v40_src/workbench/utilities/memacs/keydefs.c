#include "ed.h"

extern int nkeytab;

#define BLUE 0
#define WHITE 1


typedef	struct	{
	short	k_code;			/* Key code			*/
	int	(*k_fp)();		/* Routine to handle it		*/
	struct IntuiText  *k_string;
}	KEYTAB;

extern struct IntuiText projectext[];
extern struct IntuiText editext[];
extern struct IntuiText windowtext[];
extern struct IntuiText movetext[];
extern struct IntuiText linetext[];
extern struct IntuiText wordtext[];
extern struct IntuiText searchtext[];
extern struct IntuiText macrotext[];
extern struct IntuiText cmdtext[];

#if 0
extern	int	bindkey();
extern	int	unbindkey();
extern	int	whichbind();
extern	int	set();
/* extern	void	echo(); */
extern	int	noop();
extern	int	ctrlg();		/* Abort out of things		*/
extern	int	quit();			/* Quit				*/
extern	int	ctlxlp();		/* Begin macro			*/
extern	int	ctlxrp();		/* End macro			*/
extern	int	ctlxe();		/* Execute macro		*/
extern	int	fileread();		/* Get a file, read only	*/
extern	int	filevisit();		/* Get a file, read write	*/
extern	int	fileinsert();		/* Insert a file, read write	*/
extern	int	filewrite();		/* Write a file			*/
extern	int	filesave();		/* Save current file		*/
extern	int	filename();		/* Adjust file name		*/
extern	int	gotobol();		/* Move to start of line	*/
extern	int	forwchar();		/* Move forward by characters	*/
extern	int	gotoeol();		/* Move to end of line		*/
extern	int	backchar();		/* Move backward by characters	*/
extern	int	forwline();		/* Move forward by lines	*/
extern	int	backline();		/* Move backward by lines	*/
extern	int	forwpage();		/* Move forward by pages	*/
extern	int	backpage();		/* Move backward by pages	*/
extern	int	gotobob();		/* Move to start of buffer	*/
extern	int	gotoeob();		/* Move to end of buffer	*/
extern	int	setmark();		/* Set mark			*/
extern	int	swapmark();		/* Swap "." and mark		*/
extern	int	forwsearch();		/* Search forward		*/
extern	int	backsearch();		/* Search backwards		*/
extern	int	showcpos();		/* Show the cursor position	*/
extern	int	nextwind();		/* Move to the next window	*/
extern  int	prevwind();		/* Move to the previous window	*/
extern	int	onlywind();		/* Make current window only one	*/
extern	int	splitwind();		/* Split current window		*/
extern	int	mvdnwind();		/* Move window down		*/
extern	int	mvupwind();		/* Move window up		*/
extern	int	enlargewind();		/* Enlarge display window.	*/
extern	int	shrinkwind();		/* Shrink window.		*/
extern	int	listbuffers();		/* Display list of buffers	*/
extern	int	usebuffer();		/* Switch a window to a buffer	*/
extern	int	killbuffer();		/* Make a buffer go away.	*/
extern	int	reposition();		/* Reposition window		*/
extern	int	refresh();		/* Refresh the screen		*/
extern	int	twiddle();		/* Twiddle characters		*/
extern	int	tab();			/* Insert tab			*/
extern	int	newline();		/* Insert CR-LF			*/
extern	int	indent();		/* Insert CR-LF, then indent	*/
extern	int	openline();		/* Open up a blank line		*/
extern	int	deblank();		/* Delete blank lines		*/
extern	int	quote();		/* Insert literal		*/
extern	int	backword();		/* Backup by words		*/
extern	int	forwword();		/* Advance by words		*/
extern	int	forwdel();		/* Forward delete		*/
extern	int	backdel();		/* Backward delete		*/
extern	int	kill();			/* Kill forward			*/
extern	int	yank();			/* Yank back from killbuffer.	*/
extern	int	upperword();		/* Upper case word.		*/
extern	int	lowerword();		/* Lower case word.		*/
extern	int	upperregion();		/* Upper case region.		*/
extern	int	lowerregion();		/* Lower case region.		*/
extern	int	capword();		/* Initial capitalize word.	*/
extern	int	delfword();		/* Delete forward word.		*/
extern	int	delbword();		/* Delete backward word.	*/
extern	int	killregion();		/* Kill region.			*/
extern	int	copyregion();		/* Copy region to kill buffer.	*/
extern	int	spawncli();		/* Run CLI in a subjob.		*/
extern	int	spawn();		/* Run a command in a subjob.	*/
extern	int	quickexit();		/* low keystroke style exit.	*/
extern	int	setfillcol();		/* Set fill column.		*/
extern	int	getccol();		/* Get current column		*/
extern	int	gotoline();		/* go to line n 		*/
extern	int	forwsearchr();		/* forward search & replace	*/
extern	int	qforwsearchr();		/* forw. query search & replace	*/
extern	int	deleteline();		/* kill entire line		*/
extern	int	windforwpage();		/* scroll other window forward	*/
extern	int	windbackpage();		/* scroll other window backward */
extern	int	InitFunctionKeys();	/* load function key macros	*/
extern	int	ExecuteFile();		/* execute a command file */
extern	int	MacroXe();		/* execute macro		*/
extern	int	gotobow();		/* goto beginning of window	*/
extern	int	gotoeow();		/* goto end of window		*/
extern	int	filemodsave();		/* save all modified buffers	*/
extern	int	insertbuffer();		/* insert buffer into current	*/
extern	int	invertword();		/* invert case of a word	*/
extern	int	justifybuffer();
extern	int	setkey();		/* set function key		*/
extern  int	ecommand();
extern  int     searchparen();
extern  int	fencematch();
#endif

/*
 * Command table.
 * This table  is *roughly* in ASCII
 * order, left to right across the characters
 * of the command. This explains the funny
 * location of the control-X commands.
 */

KEYTAB	keytab[] = {
	CTRL|'@',(int)setmark,		&editext[2],
	CTRL|'A',(int)gotobol,		&linetext[3],
	CTRL|'B',(int)backchar,		&cmdtext[5],
	CTRL|'C',(int)quit,			&projectext[10],
	CTRL|'D',(int)forwdel,		&cmdtext[6],
	CTRL|'E',(int)gotoeol,		&linetext[4],
	CTRL|'F',(int)forwchar,		&cmdtext[7],
	CTRL|'G',(int)ctrlg,			&editext[15],
	CTRL|'H',(int)backdel,		&cmdtext[8],
	CTRL|'I',(int)tab,			&cmdtext[4],
	CTRL|'J',(int)indent,		&editext[13],
	CTRL|'K',(int)kill,			&linetext[2],
	CTRL|'L',(int)refresh,		&editext[11],
	CTRL|'M',(int)newline,		&cmdtext[9],
	CTRL|'N',(int)forwline,		&linetext[5],
	CTRL|'O',(int)openline,		&linetext[0],
	CTRL|'P',(int)backline,		&linetext[6],
	CTRL|'Q',(int)quote,			&editext[12],
	CTRL|'R',(int)backsearch,		&searchtext[1],
	CTRL|'S',(int)forwsearch,		&searchtext[0],
	CTRL|'T',(int)twiddle,		&editext[14],
	CTRL|'U',(int)noop,			&macrotext[0],
	CTRL|'V',(int)forwpage,		&movetext[6],
	CTRL|'W',(int)killregion,		&editext[0],
	CTRL|'Y',(int)yank,			&editext[1],
	CTRL|'Z',(int)mvdnwind,		&movetext[10],
	CTRL|'_',(int)spawncli,		&projectext[8],
	CTLX|CTRL|'B',(int)listbuffers,	&editext[6],
	CTLX|CTRL|'C',(int)quit,		&projectext[10],
	CTLX|CTRL|'D',(int)deleteline,	&linetext[1],
	CTLX|CTRL|'F',(int)quickexit,	&projectext[7],
	CTLX|CTRL|'G',(int)gotoline,		&movetext[4],
	CTLX|CTRL|'I',(int)fileinsert,	&projectext[3],
	CTLX|CTRL|'K',(int)setkey,		&macrotext[5],
	CTLX|CTRL|'L',(int)lowerregion,	&editext[5],
	CTLX|CTRL|'M',(int)filemodsave,	&projectext[6],
	CTLX|CTRL|'O',(int)deblank,		&linetext[8],
	CTLX|CTRL|'R',(int)fileread,		&projectext[1],
	CTLX|CTRL|'S',(int)filesave,		&projectext[4],
	CTLX|CTRL|'U',(int)upperregion,	&editext[4],
	CTLX|CTRL|'V',(int)filevisit,	&projectext[2],
	CTLX|CTRL|'W',(int)filewrite,	&projectext[5],
	CTLX|CTRL|'X',(int)swapmark,		&movetext[5],
	CTLX|CTRL|'Z',(int)shrinkwind,	&windowtext[5],
	CTLX|'!',(int)spawn,			&projectext[9],
	CTLX|'=',(int)showcpos,		&linetext[9],
	CTLX|'(',(int)ctlxlp,		&macrotext[2],
	CTLX|')',(int)ctlxrp,		&macrotext[3],
	CTLX|'1',(int)onlywind,		&windowtext[0],
	CTLX|'2',(int)splitwind,		&windowtext[1],
	CTLX|'B',(int)usebuffer,		&editext[7],
	CTLX|'E',(int)ctlxe,			&macrotext[4],
	CTLX|'F',(int)filename,		&projectext[0],
	CTLX|'J',(int)justifybuffer,		&editext[10],
	CTLX|'K',(int)killbuffer,		&editext[9],
	CTLX|'N',(int)nextwind,		&windowtext[2],
	CTLX|'P',(int)prevwind,		&windowtext[3],
	CTLX|'Q',(int)quote,			&editext[12],
	CTLX|'R',(int)backsearch,		&searchtext[1],
	CTLX|'S',(int)forwsearch,		&searchtext[0],
	CTLX|'V',(int)windbackpage,		&windowtext[7],
	CTLX|'Z',(int)enlargewind,		&windowtext[4],
	META|CTRL|'[',(int)ecommand,		&macrotext[8],
	META|CTRL|'B',(int)bindkey,		&cmdtext[1],
	META|CTRL|'C',(int)quit,		&projectext[10],
	META|CTRL|'D',(int)whichbind,	&cmdtext[3],
	META|CTRL|'E',(int)echo,		&cmdtext[0],
	META|CTRL|'F',(int)searchparen,	&searchtext[4],
	META|CTRL|'U',(int)unbindkey,	&cmdtext[2],
	META|CTRL|'V',(int)windforwpage,	&windowtext[6],
	META|CTRL|'Y',(int)insertbuffer,	&editext[8],
	META|'!',(int)reposition,		&linetext[7],
	META|',',(int)gotobow,		&movetext[2],
	META|'-',(int)setmark,		&editext[2],
	META|'.',(int)gotoeow,		&movetext[3],
	META|'>',(int)gotoeob,		&movetext[1],
	META|'<',(int)gotobob,		&movetext[0],
	META|'B',(int)backword,		&movetext[9],
	META|'C',(int)capword,		&wordtext[4],
	META|'D',(int)delfword,		&wordtext[0],
	META|'E',(int)ExecuteFile,		&macrotext[7],
	META|'F',(int)forwword,		&movetext[8],
	META|'H',(int)delbword,		&wordtext[1],
	META|'K',(int)InitFunctionKeys,	&macrotext[6],
	META|'L',(int)lowerword,		&wordtext[3],
	META|'Q',(int)qforwsearchr,		&searchtext[3],
	META|'R',(int)forwsearchr,		&searchtext[2],
	META|'S',(int)set,			&macrotext[1],
	META|'U',(int)upperword,		&wordtext[2],
	META|'V',(int)backpage,		&movetext[7],
	META|'W',(int)copyregion,		&editext[3],
	META|'Z',(int)mvupwind,		&movetext[11],
	META|'^',(int)invertword,		&wordtext[5],
	META|'~',(int)MacroXe,		NULL,
	META|0x7F,delbword,		&wordtext[1],
	0x7F,forwdel,			&cmdtext[5]
};

#define	NKEYTAB	(sizeof(keytab)/sizeof(keytab[0]))

#if	LK201
/*
 * Mapping table for all of the funny
 * keys with the numeric parameters on the LK201.
 * Indexed by the code, which is between 0 (unused) and
 * 34 (F20). An entry of 0 means no mapping. The map
 * goes to command keys. If I had a "special" bit,
 * I could use the code in the escape sequence as a
 * key code, and return (for example) "do" as
 * SPECIAL + 29. Then the dispatch would be
 * done by the default keymap. This is probably a
 * better way to go.
 */
short	lkmap[]	= {
	0,
	CTRL|'S',			/* 1	Find			*/
	CTRL|'Y',			/* 2	Insert here		*/
	CTRL|'W',			/* 3	Remove			*/
	CTRL|'@',			/* 4	Select			*/
	META|'V',			/* 5	Previous screen		*/
	CTRL|'V',			/* 6	Next screen		*/
	0,
	0,
	0,
	0,				/* 10	Compose			*/
	0,
	0,				/* 12	Print screen		*/
	0,
	0,				/* 14	F4			*/
	0,
	0,
	0,				/* 17	F6			*/
	0,				/* 18	F7			*/
	0,				/* 19	F8			*/
	0,				/* 20	F9			*/
	0,				/* 21	F10			*/
	0,
	0,
	0,
	0,
	0,				/* 26	F14			*/
	0,
	0,				/* 28	Help			*/
	CTLX|'E',			/* 29	Do	C-X E		*/
	0,
	CTLX|'P',			/* 31	F17	C-X P		*/
	CTLX|'N',			/* 32	F18	C-X N		*/
	CTLX|'Z',			/* 33	F19	C-X Z		*/
	CTLX|CTRL|'Z'			/* 34	F20	C-X C-Z		*/
};
#endif

void initKeys()
{
nkeytab=NKEYTAB;
}

int noop(f,n)
int f,n;
{
return(FALSE);
}
