#include <exec/types.h>

/* tinymain.c protos */
void exit(int error);
void _tinymain(char *line);

/* ansi.c prototypes */
void ansimove(int row, int col);
void ansieeol(void);
void ansieeop(void);
void ansibeep(void);
void ansiparm(int n);
void ansiopen(void);

/* basic.c prototypes */
int gotobol(int f, int n);
int backchar(int f, int n);
int gotoeol(int f, int n);
int forwchar(int f, int n);
int gotobob(int f, int n);
int gotoeob(int f, int n);
int forwline(int f, int n);
int backline(int f, int n);
int getgoal(LINE *dlp);
int forwpage(int f, int n);
int backpage(int f, int n);
int setmark(int f, int n);
int swapmark(int f, int n);

/* bindkey.c prototypes */
WORD dobindkey(WORD c);
int bindkey(int f,int n);
int whichbind(int f,int n);
int unbindkey(int f,int n);
WORD upgradekey(UBYTE *string);


/* buffer.c */
int usebuffer(int f, int n);
int killbuffer(int f, int n);
int listbuffers(int f, int n);
int makelist(void);
void itoa(char *buf, int width, int num);
int addline(char *text);
int anycb(void);
BUFFER	*bfind(char *bname, int cflag, int bflag);
int bclear(BUFFER *bp);
int musebuffer(char iname[]);
int popupbuffer(char iname[]);

/* cdio.c functions */

int CDOpen(struct Window *window);
void CDClose(void);
void queue_read(void);
void CDPutChar(UBYTE character);
void CDPutStr(UBYTE *string);
void CDPutStrL(UBYTE *string,int length);
int CDMayGetChar(void);
UBYTE CDGetChar(void);

/* safeclose.c prototypes */
void CloseWindowSafely( struct Window *win );
void StripIntuiMessages( struct MsgPort *mp, struct Window *win );



/* command.c prototypes */
int ecommand(int f,int n);
int command(UBYTE *buffer,int f,int n);
int cmpcmd(UBYTE *a, UBYTE *b);
WORD FindCommand(UBYTE *string);
int nwspace(UBYTE *s);
int nextarg(UBYTE *s);
int cmdgetc(void);
void fixcstring(UBYTE *s);

/* display.c functions */
void vtinit(void);
void vtfree(void);
void vttidy(void);
void vtmove(int row, int col);
void vtputc(int c);
void vtzapc(int c);
void vteeol(void);
void update(void);
int updateline(int row, UBYTE *vline, UBYTE *pline);
void modeline(EWINDOW *wp);
void movecursor(int row, int col);
void mlerase(void);
int mlyesno(UBYTE *prompt);
int mlgetnumber(UBYTE *prompt,int def);
int mlquery(UBYTE *prompt);
int mlreply(UBYTE *prompt, UBYTE *buf, int nbuf);
int mlquiet(UBYTE *prompt);
void __stdargs mlwrite(UBYTE *fmt, ... );
void mlputs(UBYTE *s);
void mlputi(int i, int r);
void mlputli(long l, int r);
void abort(void);
int echo(int f,int n);

/* extra.c prototypes */
int InitFunctionKeys(int f,int n);
int setkey(int f, int n);
void TellAboutEmacs(void);
int MouseCursor(struct IntuiMessage *message);
void SetMyPointer(void);
int SetLace(int f, int n);
int ExecuteFile(int f,int n,UBYTE *infile);
int egetline(LONG file,UBYTE *buf, int nbuf);
int MacroXe(int f, int n);

/* fences.c protypes */
int searchparen(int f, int n);
static int searchignore(int ch, int forward);
int fencematch(UBYTE ch);

/* file.c prototypes */
int fileread(int f, int n);
int filevisit(int f, int n);
int readin(UBYTE *fname);
void makename(UBYTE *bname, UBYTE *fname);
int filewrite(int f, int n);
int filesave(int f, int n);
int writeout(UBYTE *fn);
int filename(int f, int n);
int mfilename(UBYTE *fname);
int ireadin(UBYTE *fname);
int filemodsave(int f ,int n);
int jreadin(UBYTE *fname);
int insertbuffer(int f, int n);
int justifybuffer(int f ,int n);
int fileinsert(int f, int n);

/* fileio.c */
int filexists(UBYTE *fn);
ULONG getprot(UBYTE *fn);
void setprot(UBYTE *fn,ULONG protection);
int ffropen(UBYTE *fn);
int ffwopen(UBYTE *fn);
int ffclose(void);
int ffputline(UBYTE *buf, int nbuf);
int ffgetline(UBYTE *buf, int nbuf, int *length);
int agetc(ULONG ffp);
int aputc(UBYTE ch,ULONG ffp);

/* init.c prototypes */
void aStart(int argc,UBYTE **argv);
int GetWindow(int flag);
void GetWindowSize(void);
void Cleanup(void);
void errorExit(int error);
int CXBRK(void);
void SetForWindow(void);
void NewList(struct Minlist *list);

/* keydefs.c */
void initKeys(void);
int noop(int f,int n);

/* keymap.c */
int InitKeyMap(void);
int ReadKeyMap(void);
void InitMyTable(void);
void ResetKeyPad(void);
int WriteKeyMap(struct KeyMap *keymap);
void SetKeyPad(void);
/* line.c prototypes */
LINE	*lalloc(int used);
void lfree(LINE *lp);
void lchange(int flag);
int linsert(int n, int c);
int lnewline(void);
int ldelete(int n, int kflag);
int ldelnewline(void);
void kdelete(void);
int kinsert(int c);
int kremove(int n);

/* malloc.c prototypes */
UBYTE *malloc(LONG size);
void free(UBYTE *p);

/* main.c prototypes */
int main(int argc, char *argv[]);
void edinit(char bname[]);
void edfree(void);
int execute(int c, int f, int n, int flag);
int getkey(void);
int getctl(void);
void quickexit(int f, int n);
int quit(int f, int n);
int ctlxlp(int f, int n);
int ctlxrp(int f, int n);
int ctlxe(int f, int n);
int ctrlg(int f, int n);
int ctlu(int *ch);

/* paste.c prototypes */
void StartPaste(void);
void KillPaste(void);
UBYTE GetPaste(void);
int CopyClip(int f,int n);

/* random.c prototypes */
int showcpos(int f, int n);
int getccol(int bflg);
int getcline(void);
int twiddle(int f, int n);
int quote(int f, int n);
int openline(int f, int n);
int newline(int f, int n);
int deblank(int f, int n);
int indent(int f, int n);
int getindent(LINE *line);
int doindent(int nicol);
int lmargin(void);
int forwdel(int f, int n);
int backdel(int f, int n);
int kill(int f, int n);
int yank(int f, int n);
int deleteline(int f, int n);
int gotoline(int f, int n);
int tab(int f, int n);

/* region.c prototypes */

int killregion(int f, int n);
int copyregion(int f, int n);
int lowerregion(int f, int n);
int upperregion(int f, int n);
int getregion(REGION *rp);

/* search.c prototypes */
int forwsearch(int f, int n);
int forsearch(int f,int n);
int backsearch(int f, int n);
int eq(int bc, int pc);
int readpattern(char *prompt,char *pattern);
int readrpat(char *prompt);
int qforwsearchr(int f ,int n);
int forwsearchr(int f, int n);

/* set.c prototypes */
int set(int f,int n);

/* strfuncs.c prototypes */
UBYTE *Lstrcpy(UBYTE *dest, UBYTE *source, int length);
int atoi(UBYTE *s);

/* spawn.c prototypes */
int spawncli(int f, int n);
int spawn(int f, int n);

/* termio.c prototypes */
void ttopen(void);
void ttclose(void);
void ttputc(int c);
void ttflush(void);
void ttcursoroff(void);
void ttcursoron(void);
int ttgetc(void);
int getboth(void);

/* window.c prototypes */

int gotobow(int f, int n);
int gotoeow(int f, int n);
int reposition(int f, int n);
int refresh(int f, int n);
int nextwind(int f, int n);
int prevwind(int f, int n);
EWINDOW *wfind(char *wname);
int mvdnwind(int f, int n);
int mvupwind(int f, int n);
int onlywind(int f, int n);
int splitwind(int f, int n);
int enlargewind(int f, int n);
int shrinkwind(int f, int n);
EWINDOW	*wpopup(void);
int windforwpage(int f, int n);
int windbackpage(int f, int n);

/* word.c prototypes */
int backword(int f, int n);
int forwword(int f, int n);
int upperword(int f, int n);
int lowerword(int f, int n);
int capword(int f, int n);
int invertword(int f, int n);
int delfword(int f, int n);
int delbword(int f, int n);
int inword(void);
int wrapword(int f,int n);
