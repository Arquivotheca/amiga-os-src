/*
 * $Id: proto.h,v 39.2 93/02/10 10:43:44 mks Exp $
 *
 * $Log:	proto.h,v $
 * Revision 39.2  93/02/10  10:43:44  mks
 * Cleaned up prototypes to match changes
 * 
 * Revision 39.1  93/01/11  16:54:10  mks
 * Now supports a string filter for the RENAME requester...
 * No longer needs to check for ":" or "/" after the fact.
 *
 * Revision 38.5  92/06/29  11:24:00  mks
 * Cleaned up the prototypes a bit
 *
 * Revision 38.4  92/05/30  17:30:39  mks
 * Changed some prototypes
 *
 * Revision 38.3  92/04/27  14:23:45  mks
 * The "NULL" parameters have been removed.  (Unused parameter)
 *
 * Revision 38.2  92/02/13  14:10:02  mks
 * Changed the PlaceObj() proto
 *
 * Revision 38.1  91/06/24  11:37:49  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "sysproto.h"
#include "asmproto.h"
/* Prototypes for functions defined in diskchange.c */
BPTR FindVolume(struct DeviceList *dl,
               struct ActiveDisk *ad);
int FindDevices(struct DeviceList *dl);
int UpdateVolumes(struct DeviceList *dl);
int VolumeListSearch(int (*fptr)(),
	LONG arg1, LONG arg2, LONG arg3, LONG arg4);
int ClearActive(struct ActiveDisk *ad);
void PhantomLockCleanup(struct WBObject *obj);
int RemoveOldDisks(struct ActiveDisk *ad);
int RemoveActiveDisk(struct ActiveDisk *ad);
int ProcessDisks(struct ActiveDisk *ad);
int NewDiskInfo(struct ActiveDisk *ad);
int FloppyCheck(struct ActiveDisk *ad);
int initDisks(void);
void FindDisks(void);
int VolumeToActive(struct ActiveDisk *ad,
                   long vol);
int FindActive(struct ActiveDisk *ad,
               struct WBObject *obj);
VOID RedrawGauge(struct WBObject *obj);
int TestPhysical(struct DeviceNode *dn);
struct ActiveDisk *NewActive(struct DeviceList *dl,
                             int state);
int MatchObjectToActive(struct ActiveDisk *ad,
                        struct WBObject *obj);
struct ActiveDisk *ObjectToActive(struct WBObject *obj);
/* Prototypes for functions defined in diskin.c */
int FindDeviceNode(struct ActiveDisk *ad,
                   struct ActiveDisk *volad);
int DiskInserted(struct ActiveDisk *ad);
int ManufactureDiskLock(struct ActiveDisk *ad);
void AddDiskIcon(struct ActiveDisk *ad,
                 struct WBObject *obj);
void ValidateDiskLock(struct ActiveDisk *ad);
/* Prototypes for functions defined in drawers.c */
int OpenDrawer(struct WBObject *drawer);
int LockDrawer(struct WBObject *drawerobj);
int FindDupObj(struct WBObject *obj, BPTR fl, char *name, ULONG type);

BOOL AddChildCommon(BPTR,char *,struct WBObject *,struct WBObject *,struct FileInfoBlock *);

int OpenCommon(struct WBObject *drawerobj,int flags, BOOL rescan);

BPTR CheckObjectList(struct FileInfoBlock *fib,
                    int doticon);
int TimeStampCmp(struct DateStamp *a,
                 struct DateStamp *b);
unsigned char *getline(struct LinePkt *lp,
                       unsigned char *buf);
unsigned char getbyte(struct LinePkt *lp);
int FillFromObjectList(long lock,
                       BPTR file,
                       unsigned char *buffer,
                       int bufsize,
                       struct WBObject *drawer,
                       int doticon);
int WriteObjName(struct WBObject *obj,
                 long file);
int WriteDotFile(struct WBObject *drawer,
                 long lock,
                 struct FileInfoBlock *fib,
                 int doticon);
void WriteObjectList(struct WBObject *drawer,
                     long lock,
                     struct FileInfoBlock *fib);
int UpdateDotFile(struct WBObject *drawerobj);
int CloseChild(struct WBObject *obj,
               struct WBObject *drawer);
int NoIconWin(struct WBObject *obj);
void CloseDrawer(struct WBObject *drawerobj);
void initText(void);
void initIconText(void);
void initTextText(void);
void termText(void);
void termIconText(void);
void termTextText(void);
void MinMaxDrawer(struct WBObject *drawer);
void MinMaxProp(struct Window *win,
                struct Gadget *gad,
                struct PropInfo *prop,
                long current,
                long min,
                long max,
                long size,
                int horizFlag);
int RefreshDrawer(struct WBObject *drawer);

void InfoListsCommon(struct List *,char *,struct FileInfoBlock *,struct List *);
void RemoveDupInfoEntry(struct List *,struct List *,char *name);

int OnList(struct Node *ln,char *name);

int CleanupInfoLists(void);
struct WBObject *CreateIconFromName(char *name,
		struct WBObject *parent,
		BPTR fromlock);
int CreateIconFromNode(struct Node *ln,
                       struct WBObject *parent);
void DisplayInfoList(void);
void DisplayNonInfoList(void);

/* Prototypes for functions defined in errcalls.c */
void FatalNoMem(LONG alert);
char *objscopy(struct WBObject *obj,
               char *string);

void *ObjAllocChip(struct WBObject *obj,ULONG len);
void *ObjAllocNorm(struct WBObject *obj,ULONG len);

int RemoveObjectCommon(struct WBObject *obj);
void RemoveObject(struct WBObject *obj);
char *ReturnDosErrorString(int err);
void ErrorDos(long index,char *objname);
void ErrorTitle(/*char *string,
	int a0,
	int a1,
	int a2,
	int a3*/);
int LockErrReport(long lock);
int VolumeErrReport(struct ActiveDisk *ad);
struct NewDD *AllocDrawer(struct WBObject *obj);
BOOL BuildAutoRequest(char **textArray,
                     char *posText,
                     char *negText,
                     unsigned long posflags,
                     unsigned long negflags);
void initIntuiText(struct IntuiText *it,
                  char *text,
                  struct IntuiText *itNext);

/* Prototypes for functions defined in fileops.c */
int WBMove(struct WBObject *obj,
	struct WBObject *fromdrawer,
	struct WBObject *todrawer,
	LONG curx, LONG cury);
int WBRename(struct WBObject *obj,
             char *oldname);
int WBRenameDisk(struct WBObject *obj,
                 char *oldname);
int DosMove(long fromlock,
            char *fromCfile,
            long tolock,
            char *toCfile,
            int notfoundok);
void BtoC(unsigned char *cbuf,
         unsigned char *bstr);
int ConvertCtoB(char *cstr);
int WriteProtected(struct FileLock *fl);
int AssignDOtoWBO(struct WBObject *obj,
                  struct DiskObject *dobj);
struct WBObject *GetWBIcon(char *name);
struct WBObject *GetNextWBObject(BPTR fh);
int WriteImageStruct(BPTR fh,
                     struct Image *im);
int PutNextWBObject(struct WBObject *obj,
                    BPTR fh);
/* Prototypes for functions defined in global.c */
USHORT *initPointers(void);
BPTR LockRamDisk(void);

/* Prototypes for functions defined in icons.c */
void BeginIconUpdate(struct WBObject *obj,
                    struct Window *win,
                    int ClearDamage_P);
void EndIconUpdate(struct Window *win);

int RenderIcon(struct WBObject *obj);

void RenderImage(struct RastPort *drp,
                struct Image *image,
                int xoff,
                int yoff,
                long mode,
                APTR maskplane);
void EmboseIcon(struct RastPort *rp,
		struct WBObject *obj);
void ClearDrawerWindow(struct NewDD *dd);
int AddIcon(struct WBObject *icon,
            struct WBObject *drawer);
IconBoxBackdrop(struct WBObject *icon,
                    struct WBObject *drawer);
void SetIconNames(struct WBObject *icon);
int RenderName(struct WBObject *icon);
int PrepareIcon(struct WBObject *obj);
void SetRastPort(struct RastPort *newrp,
                struct Image *im,
                struct BitMap *bitmap,
                USHORT *data);
void GetImageMode(struct WBObject *icon,
                 APTR *imagep,
                 unsigned long *modep,
                 APTR *maskp);
void ImageToBitMap(struct Image *im,
                  struct BitMap *bm,
                  UBYTE *idata);
int ChangeChildView(struct WBObject *obj,
                    struct NewDD *dd,
                    int newmode,
		    struct WBObject *drawer);
int RemoveIconsLoop(struct WBObject *obj);
int RemoveFakeIcons(struct WBObject *obj);
int ChangeShowAll(struct WBObject *drawer,
                  int newmode);
int ChangeViewBy(struct WBObject *obj,
                 int newmode);
/* Prototypes for functions defined in incoming.c */
void QuickFilter(struct IntuiMessage *);
int NotDrawer(struct WBObject *obj);
void DoOpen(void);
void DoClose2(struct WBObject *obj);
void DoClose(void);
void ProcessGadget(int nextState,
	struct IntuiMessage *NextEvent);
void HighIcon(struct WBObject *object);
void UnHighIcon(struct WBObject *object);
void BringToFront(struct WBObject *obj);
int ResetHighlight(struct WBObject *object);
void ResetPotion(void);
int ObjOnList(struct WBObject *obj1,
              struct WBObject *obj2);
int SelectObject(struct WBObject *obj,
                 int x,
                 int y);
struct WBObject *TranslateSelect(struct IntuiMessage *msg,
                                 int avoidselected);
void DoMouseMoves(struct IntuiMessage *msg);
void IncomingEvent(struct IntuiMessage *NextEvent);
int wbCleanup(struct WBObject *obj);
int wbReopen(struct WBObject *obj);
void WBOpenWorkBench(void);
void WBCloseWorkBench(void);
void ResetWB(void);

/* Prototypes for functions defined in init.c */

struct Window *OpenDiskWin(struct WBObject *obj);
int InitPotion(void);

/* Prototypes for functions defined in ioloop.c */

void ReadWBPrefs(void);
void WriteWBPrefs(void);
void ioloopinit(void);
void initDevMsg(struct DeviceMessage *dm);
void ioloop(void);
struct WBObject *WBGetWBObjectFromDobj(char *name,struct DiskObject *dobj);

void IconUpdate(struct IconMsg *im);
int BadIO(struct IOStdReq *msg);
int GotDevMsg(struct IOStdReq *msg);
void ProcessIO(struct IOStdReq *msg);
void TermIO(struct IOStdReq *msg);
ULONG MakeDevMsg(unsigned long cmd,
               unsigned long size,
               int arg);
void ProcessConsoleList(void);
short ReadFontPrefs(struct FontPref *fp,
		char *name);
BOOL FontsDifferent(struct FontPref *fp,
                   struct TextAttr *oldta);
void ChangeFonts(struct FontPref *fp);
void ProcessPrefs(struct PrefsMessage *msg);
int InitPattern(void);
void InitEmbosing(void);

/* Prototypes for functions defined in main.c */
void PathTick(void);
struct WorkbenchBase *InitFunc(struct WorkbenchBase *wb,
				APTR seglist);
int initgadget2(struct WBObject *o);

/* Prototypes for functions defined in matrix.c */
void Matrix(struct IntuiMessage *msg);
void WBFormat(struct WBObject *disk);
void DiskCopy(struct WBObject *fromdisk,
             struct WBObject *todisk);
void ObjMove(struct WBObject *obj,
            struct WBObject *olddrawer,
            struct WBObject *newdrawer,
            long curx,
            long cury);
void PostObjMove(struct WBObject *drawer,
                struct WBObject *obj,
                long newx,
                long newy,
                int existing);
short SameVolume(struct WBObject *obj1,
                 struct WBObject *obj2);
int DragObject(struct WBObject *obj,
               struct DragData *dd);

/* Prototypes for functions defined in menus.c */

long CheckForNotType(struct WBObject *obj,ULONG thetype);
long CheckForType(struct WBObject *obj,ULONG thetype);

int CreateWBMenus(void);
void FreeWBMenus(void);
void MenuCat(struct Menu *tailmenu, struct Menu *headmenu);
int LayoutWBMenus(void);
void InstallMenuStrings(struct Menu *menu);
void InstallItemString(struct MenuItem *item);
void RemoveMenus(void);
void RethinkMenus(void);
int OnOffMenus(struct WBObject *obj);
void AbleMenu(struct Window *win, UWORD menunumber, BOOL able);
void ResetMenus(void);
void ReLayoutToolMenu();
struct MenuItem *AddToolMenuItem(STRPTR text);
void DeleteToolMenuItem(long n);
int menutextlength(char *string);
int WindowOperate(struct WBObject *obj,
                  int (*func)(void),
                  long arg1,
                  long arg2);
int Snapshot(struct WBObject *obj);
void SnapshotAll(struct WBObject *drawer);
int CountItems(struct WBObject *obj,
               ULONG *filecount, ULONG *drawercount);
void Menumeration(struct IntuiMessage *msg);
int NotOpen(struct WBObject *obj);
int DoInfo(struct WBObject *obj);
int UnSelectIt(struct WBObject *obj);
int SelectIt(struct WBObject *obj);
int SelectAll(struct IntuiMessage *msg,
		struct WBObject *obj);
void reverse(char *s);
void itoa(int n,
         char *s);
void QuitCheck(void);
void DoExecute(char *buf, BPTR lock);

/* Prototypes for functions defined in objectio.c */
int PutObject(struct WBObject *obj);
int GetParentsLock(struct WBObject *obj);
int ClearWaitPointer(struct WBObject *obj);
int SetWaitPointer(struct WBObject *obj);
void SelectBracketDiskIO(int (*func)(), ULONG arg1);
void BeginDiskIO(void);
void CheckDiskIO(void);
void EndDiskIO(void);
void StripMessage(struct IntuiMessage *msg);
int MakeDiskLock(struct WBObject *disk);
/* Prototypes for functions defined in placeicon.c */
void PlaceObj(struct WBObject *obj,
	struct WBObject *parent);

int InsertByY(struct WBObject *obj,
              struct WBObject *newobj);
int FindColumn(struct WBObject *obj,
               int min,
               int max);
int PlaceVertically(struct WBObject *obj,
                    int iconheight,
                    int *currentyp,
                    int textheight);
int PlaceIconStrip(struct WBObject *icon,
                   struct WBObject *parent,
                   int base,
                   int width,
                   int currenty,
                   int maxy);

int CollideVertically(struct WBObject *,long,long,struct WBObject *);

int PlaceCollision(struct WBObject *obj);
int CleanupObj(struct WBObject *obj,
               struct WBObject *drawer);
int FindColumnWidth(struct WBObject *obj,
                    int *width);
int SetXPosition(struct WBObject *obj,
                 struct WBObject *drawer,
                 int base);
int BuildColumn(struct WBObject *obj,
                struct List *lh,
                int maxY,
                int *currentyp);
int Cleanup(struct WBObject *drawer);
struct WBObject *insertByName(struct WBObject *refobj,
                              struct WBObject *newobj);
struct WBObject *insertByDate(struct WBObject *refobj,
                              struct WBObject *newobj);
struct WBObject *insertBySize(struct WBObject *refobj,
                              struct WBObject *newobj);
int UtilityAdd(struct WBObject *obj,
               struct WBObject *drawer);
int InsertByText(struct WBObject *obj,
                 struct WBObject *drawer);
void SortByText(struct WBObject *drawer);

/* Prototypes for functions defined in recurse.c */

int Discard(struct WBObject *obj);
int EmptyTrash(struct WBObject *obj);
LONG KillFile(char *name);
LONG KillDirectory(BPTR lock);

/* Prototypes for functions defined in requesters.c */

void RenameInput(struct IntuiMessage *msg);
RenameObject(struct WBObject *obj, char *name);
int RenameStop(struct Window *win);
void RenameCleanup(struct RenameInfo *ri,
                  int error);
void RenameWindow(struct WBObject *obj);
void NoMem(void);
/* Prototypes for functions defined in runtool.c */
int PrepareArg(struct WBObject *obj,
               struct WBArg **listp,
               struct WBObject *tool,
               char *name);
int FindTool(struct WBObject *obj);
int FindDefaultTool(struct WBObject *obj);
int FindStartupIcon(struct WBObject *obj);
void PotionRunTool(void);
int StartTool(struct CreateToolMsg *ctm);

void CreateTool(struct WBObject *,char *,int,struct WBArg *);

void LoadTool(void);
int LoadToolCommon(struct CreateToolMsg *ctm);
void ExitTool(struct WBStartup *msg);
void ExitArgs(int wc,
             struct WBArg *waorig);
int CheckConsole(struct MsgPort *mp);
int atoi(char *string);

/* Prototypes for functions defined in tapeio.c */
/*
int PutTapeFile(char *name,
                struct FileInfoBlock *fib,
                struct TapeControlBlock *tcb,
                long level);
int GetTape(struct TapeControlBlock *tcb);
int GetTapeFile(struct TapeControlBlock *tcb);
void TapeWrite(struct TapeControlBlock *tcb,
              char *buf,
              int size);
int TapeWriteBlock(struct TapeControlBlock *tcb,
                   char *buf,
                   int size);
void TapeRead(struct TapeControlBlock *tcb,
             char *buf,
             int size);
int TapeReadBlock(struct TapeControlBlock *tcb,
                  char *buf,
                  int size);
*/

/* Prototypes for functions defined in timer.c */
void UpdateMemory(void);
int initTimer(void);
void TimeTick(struct timerequest *tr);
void PostLayerDemon(void);
void AbortLayerDemon(void);
int InitLayerDemon(void);
void UnInitLayerDemon(void);
void DoLayerDemon();

/* Prototypes for functions defined in windows.c */

int MoveToLink(struct WBObject *obj, struct WBObject *old, struct WBObject *new);

struct Window *OpenPWindow(struct NewWindow *nw,
                           unsigned long flags,
                           struct Gadget *gadv,
                           char *text,
                           long extraIDCMP,
			   BOOL diskwin,
			   UWORD viewmodes);
void adjustWindow(long scrsize,
                 long minpos,
                 short *posp,
                 short *sizep);
int InitScreenAndWindows(void);

VOID FormatWindowName(struct WBObject *,unsigned char **,struct Window *);

void InitGadgets(struct NewDD *dd);
int PotionOpen(struct WBObject *drawerobj);
struct Window *WhichWindow(int x,
                           int y);
int winCmp(struct WBObject *obj,
           struct Window *win);
struct WBObject *WindowToObj(struct Window *win);
UpdateWindowSize(struct WBObject *obj);
int UpdateWindow(struct WBObject *obj);
int RemIntuiMsg(struct IntuiMessage *msg,
                struct Window *win);
void ClosePWindow(struct Window *win,
		  struct NewDD *dd);
void PrintNewWindow(struct NewWindow *nw);
/* Prototypes for functions defined in gels.c */
int initWbGels(void);
struct Bob *makeBob(struct Image *im,
                    unsigned long mode,
                    unsigned short *maskplane);
int RedrawOneBob(struct BobNode *bn,
                 long dx,
                 long dy);
void RedrawBobs(struct IntuiMessage *msg);
int AddWbBob(struct WBObject *obj,
             struct Screen *screen);
int RemWbBob(struct BobNode *bn,
             struct Screen *screen);
void DrawBox(void);
void RedrawBox(struct IntuiMessage *msg);
void Redraw(struct IntuiMessage *msg);
void ClearDragging(struct IntuiMessage *msg,
                  int flag);
void SetDragging(struct IntuiMessage *msg,
                int flag);
void VisibleLayerRegion(struct Layer *layer,
                       struct Region *rgn);
void PrintRectangle(struct Rectangle *rect,
                   char *s);
void AnimateDragSelectBox(struct IntuiMessage *msg);

/* Prototypes for functions defined in app.c */
VOID DoAppWork(struct Message *);
int FindAppIcon(struct WBObject *);
int AssignMenuItemToAppMenuItem(struct WorkbenchAppInfo *, struct MenuItem **);
struct WorkbenchAppInfo *MenuToAppMenuItem(int);
int IconToAppIcon(struct WorkbenchAppInfo *, struct WBObject *);
int WindowToAppWindow(struct WorkbenchAppInfo *, struct Window *);
void SendAppMenuItemMsg(struct WorkbenchAppInfo *ami, struct IntuiMessage *);
void SendAppWindowMsg(struct WorkbenchAppInfo *, struct IntuiMessage *);
void CleanupAppMsg(struct AppMessage *);
void SendAppIconMsg(struct WorkbenchAppInfo *, struct IntuiMessage *,int);
struct MenuItem *FindLastMenuItem(struct Menu *,int *);

/* Prototypes for functions defined in showdate.c */
void ShowDate(long *v,
             int *m,
             int *d,
             int *y,
             int *w);
/* Prototypes for functions defined in startup.c */
int StartupDrawer(void);
/* Prototypes for functions defined in dos.c */
int myparentdir(long oldlock);
int myduplock(long oldlock);
int mycreatedir(char *name);
BPTR mylock(char *name,
           long accessmode);
void myunlock(BPTR lock);
int mycurrentdir(long lock);
BOOL myassignlock(char *name, BPTR lock);
/* Prototypes for functions defined in exec.c */
void *myallocmem(long size,
                 long type);
void myfreemem(void *mem,
              long size);
void myfreeentry(struct MemList *ml);

/* Prototypes for functions defined in iconlib.c */
struct WBObject *WBAllocWBObject(char *name);
void WBFreeWBObject(struct WBObject *obj);

struct WBObject *WBGetWBObject(char *);
struct WBObject *NewWBGetWBObject(char *,struct FileInfoBlock *);

int WBPutWBObject(char *name,
                struct WBObject *obj);
void *WBFreeAlloc(struct FreeList *free,
                long size,
                long type);

/* Prototypes for functions defined in viewbytext.c */
void FormatObj(struct WBObject *obj);
void PrepareViewByText(struct WBObject *obj);
int growNameImage(struct WBObject *obj);
int MakeNameGadget(struct WBObject *obj,
                   int viewmode);
/* Prototypes for functions defined in debug.c */
void PrintBString(BSTR bstr);
int PrintIDCMPFlags(unsigned long IDCMPFlags);
int PrintNewDD(struct NewDD *dd);
int PrintWBObject(struct WBObject *obj);
int PrintNode(struct Node *ln);
int PrintObj(struct WBObject *obj);
int PrintVol(struct DeviceList *dl);
int PrintAppWindow(struct WorkbenchAppInfo *aw);
int PrintAppIcon(struct WorkbenchAppInfo *ai);
int PrintAppMenuItem(struct WorkbenchAppInfo *ami);
int PrintIntuiText(struct IntuiText *it);
int PrintMenuItem(struct MenuItem *mi,
                  char *s);
int PrintMenuStrip(struct Menu *menu);
void DisplayLock(long lock,
                char *s);
struct LockNode *OnLockList(long lock);
void AddToLockList(long lock,
                  char *s);
void RemoveFromLockList(long lock,
                       char *s);
void DisplayLockList(void);
void AddToAllocList(APTR mem,
                   long size,
                   long type);
void RemoveFromAllocList(APTR mem,
                        long size);
void DisplayAllocList(void);
int OnAllocList(APTR mem,
                long size,
                long free,
                long *position);
void FreeAllocList(void);
void PrintFileopsAddresses(void);
int PrintGadget(struct Gadget *gad);
void PrintNewDDOffsets(void);
void PrintWBObjectOffsets(void);
void PrintWorkbenchBase(void);
/* Prototypes for functions defined in cstrings.c */
char *scopy(char *s);
char *suffix(char *s,
             char *suf);

/* Prototypes for functions defined in render.c */
void RenderAll(struct InfoTag *itag,
               short firsttime);
void InfoRenderIcon(struct InfoTag *itag,
                short inrefresh);
void RenderProBits(struct InfoTag *itag);
/* Prototypes for functions defined in initgadgets.c */
struct Gadget *InitAll(struct InfoTag *itag);
struct Gadget *InitPhysicalInfo(struct InfoTag *itag,
                                struct Gadget *gad);
struct Gadget *InitProBits(struct InfoTag *itag,
                           struct Gadget *gad);
struct Gadget *InitComment(struct InfoTag *itag,
                           struct Gadget *gad);
struct Gadget *InitDefaultTool(struct InfoTag *itag,
                               struct Gadget *gad);
struct Gadget *InitToolTypes(struct InfoTag *itag,
                             struct Gadget *gad);
struct Gadget *InitExitGadgets(struct InfoTag *itag,
                               struct Gadget *gad);
/* Prototypes for functions defined in misc.c */
void datetostring(unsigned char *string,
                  struct DateStamp *ds);
struct Node *AllocNode(unsigned char *name);
void EnableGadget(struct Window *win,
                  struct Gadget *gad);
void DisableGadget(struct Window *win,
                   struct Gadget *gad);
long PrintText(struct Window *win, UBYTE color, WORD x, WORD y, STRPTR string);

/* Prototypes for functions defined in window.c */
int SyncInfo(struct WBObject *wbobj);

/* Prototypes for functions defined in event.c */
void HandleTTGadget(struct Window *infowindow,
                    struct Gadget *gad,
                    UWORD code);
short HandleSave(struct Window *infowindow);
short HandleGadgetUp(struct Window *infowindow,
                     struct Gadget *gad,
                     UWORD code);
short HandleInfoEvent(struct Window *infowindow,
                      struct IntuiMessage *imsg);
/* Prototypes for functions defined in infoio.c */
short BuildToolTypes(struct InfoTag *itag,
                     struct WBObject *wbobj);
void InfoSave(struct InfoTag *itag);

/* Prototypes for functions defined in quotes.c */
char *Quote(UWORD x);
void SetQuoteStrings();

/* Prototypes for functions defined in executewindow.c */
WORD SyncExecute(STRPTR title, STRPTR greeting, STRPTR label, STRPTR buffer, struct MsgPort *intuiport, struct Hook *hook);

/* Prototypes for functions defined in executerender.c */
struct Gadget *InitExecute(struct ExecuteTag *etag, STRPTR label);
struct Gadget *InitExitGadgets(struct ExecuteTag *etag, struct Gadget *gad);
void RenderExecute(struct ExecuteTag *etag, BOOL firsttime);
WORD HandleExecuteEvent(struct Window *executewindow, struct IntuiMessage *imsg);

/* Prototypes for functions defined in backdrop.c */
void FillBackdrop(BPTR volumelock);
int LeaveOut(struct WBObject *obj, long autoPut);
int PutAway(struct WBObject *obj, long autoPut);
void FullRemoveObject(struct WBObject *obj);

/* Prototypes for functions in copy.c */
int Duplicate(struct WBObject *obj);
