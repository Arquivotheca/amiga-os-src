
#include <proto/all.h>

/* VOID printf(char *,...); */
VOID kprintf(char *,...);
VOID sprintf(char *,char *,...);
#define printf kprintf
#ifdef DEBUG
#define D(x) x
#define D2(x) ;
#else
#define D(x) ;
#define D2(x) ;
#endif

#include "list.h"

/* cfunctions.c */
VOID  strcpy(UBYTE *d, UBYTE *s);
ULONG strlen(UBYTE *s);
VOID  strcat(UBYTE *to, UBYTE *from);
ULONG stricmp(UBYTE *s, UBYTE *t);
ULONG stricmpn(UBYTE *s, UBYTE *t, ULONG n);
VOID  movmem(UBYTE *s, UBYTE *d, ULONG len);
VOID  setmem(UBYTE *s, ULONG len, BYTE val);
VOID  basecopy(UBYTE *dest,UBYTE *source);
VOID  restcat(UBYTE *to, UBYTE *from);
VOID  strzap(UBYTE *to);

/* devreq.c */
VOID  DoGadgetsHilight(struct Gadget *G,struct RastPort *rp,ULONG flag);
VOID  DoGadgetsSelect(struct Gadget *G,struct RastPort *rp,struct List *list,UBYTE *buf);
VOID  MakeDevRequest(struct List *list,UBYTE *devbuf, UBYTE *drbuf,struct Window *w,UBYTE *title);
LONG  DevRequest(struct List *list,UBYTE *devbuf,UBYTE *drbuf,struct Window *w);
VOID  Notice(struct Window *w,UBYTE *string1, UBYTE *string2);

/* exdir.c */
VOID AddFNode(FILELIST *list,struct FileInfoBlock *fib,LONG flag);
VOID RemoveFNode(FILENODE *FNode);
LONG ExamineDir(FILELIST *list,UBYTE *name,LONG flag);
LONG DoExamineDir(FILELIST *list,UBYTE *name,LONG flag);
VOID EmptyDirList(FILELIST *list);

/* gadgets.c */

/* getdisks.c */
VOID btoc(UBYTE *bstring);
struct Node *GetNode(UBYTE *name,UBYTE type,UBYTE pri);
VOID FreeNode(struct Node *mynode);
VOID GetDisks(struct List *dlist);
VOID FreeDisks(struct List *dlist);

/* global.c */

/* images.c */

/* init.c */
VOID BuildAbsolutePath(UBYTE *name,UBYTE *buf,ULONG len);
LONG StartUp(int argc,UBYTE *argv[],UBYTE *installbuf[],UBYTE *defbuf,UBYTE *lastbuf,UBYTE *basebuf);
LONG Init(int argc,UBYTE *argv[],UBYTE *installbuf[],UBYTE *defbuf,UBYTE *lastbuf,UBYTE *basebuf);
VOID Bye(long err);

/* install.c */
LONG GetDiskInfo(struct InfoData *info,UBYTE *name);
VOID RemoveLastDir(UBYTE *path,LONG *depth);
LONG FixPath(FILENODE *node,UBYTE *path,LONG *depth);
LONG DeleteFlaggedFiles(FILELIST *list,UBYTE *dir,LONG flag);
LONG ReadLine(struct FileHandle *fh,UBYTE *buf);
VOID ShutDown(LONG s,LONG err);
VOID InformMsg(LONG Required);
LONG CheckWriteProtect(UBYTE *s);
LONG DoExecute(UBYTE *command);
VOID WaitLock(UBYTE *name1,UBYTE *name2);
LONG ScanQuote(UBYTE *string);
VOID PlusRename(UBYTE *name);
LONG delete(UBYTE *name);

/* list.c */
VOID AddFNode(FILELIST *list,struct FileInfoBlock *fib,LONG n);
VOID RemoveFNode(FILENODE *FNode);
VOID EmptyDirList(FILELIST *list);

/* prompt.c */
LONG UnpackNode(FILENODE *node,UBYTE *buf);
VOID DisplayNode(FILENODE *node);
VOID RefreshDisplay(FILENODE *node);
VOID SelectUp(VOID);
VOID SelectDown(VOID);
VOID DoProp(struct Gadget *Gadget);
LONG DoGadgets(struct IntuiMessage *Msg);
VOID DoMouse(struct IntuiMessage *Msg,LONG *Selected, LONG *Required, LONG Flag);
LONG Prompt(FILELIST *list,LONG *Selected,LONG *Required,LONG Flag);
VOID ResetProp(VOID);

/* status.c */
VOID Status(struct Window *w,LONG Selected, LONG Required, LONG sw);
VOID PrintMsg(UBYTE *msg);
VOID ClearMsg(VOID);
