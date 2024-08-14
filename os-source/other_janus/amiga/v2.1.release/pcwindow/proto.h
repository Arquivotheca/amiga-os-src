#ifndef PROTO_H
#define PROTO_H

void kprintf(char *,...);

/*#####################*/
/* Misc                */

extern LONG (*KeyQueuer)(UBYTE);

/*#####################*/
/* protos for adjust.c */

BOOL  AdjustStart(void);
void  AdjustKeyTiming(struct Display *);

/*#####################*/
/* protos for alert.c */

VOID  AlertGrunt(UBYTE *,struct Window *);
VOID  MyAlert(USHORT,struct Display *);
VOID  Abort(USHORT,struct Display *);

/*#####################*/
/* protos for ascii2pc.c */

void  SendASCIIAsPCKeyCodes(USHORT);
void  SendKeypadDigit(UBYTE);
VOID  SendTextToPC(UBYTE *);

/*#####################*/
/* protos for autosize.c */

void  SmallSize(struct Display *);
void  FullSize(struct Display *);
void  RecordLastSmall(struct Display *,struct SuperWindow *,struct Window *);

/*#####################*/
/* protos for auxtask.c */

void  AuxToolsManager(void);
LONG  TalkWithZaphod(struct Display *,SHORT,LONG);
void  TheFatLadySings(struct MsgPort *);
void  OpenAuxTools(struct Display *);
void  CloseAuxTools(struct Display *);
void  PutNewPriority(struct Display *, SHORT);
void  KillAuxTools(void);
void  MakeAuxTools(void);
struct DisplayList *GetDisplayList(struct Display *);
BOOL  DuplicateDisplay(struct Display *);
void  GetQueueRoutine(LONG *);

VOID  ChangeWBWindowCount(LONG);

SHORT CountWBWindows(void);
void  LinkDisplay(struct Display *);
void  UnlinkDisplay(struct Display *);

/*#####################*/
/* protos for blast.asm */

void __stdargs BlastGraphicsPlanes(struct Display *);

/*#####################*/
/* protos for border.c */

void  BorderPatrol(struct Display *,SHORT,BOOL);
void  StripGadgets(struct BorderKontrol *,struct Window *);
void  RejoinGadgets(struct BorderKontrol *,struct Window *);
void  MoveSizeWait(void);

/*#####################*/
/* protos for cbio.c */

SHORT CBOpen(SHORT);
void  CBClose(void);
      CBSatisfyPost(char *,LONG);
void  writeLong(LONG *);
      CBWriteText(char *,SHORT);
SHORT CBReadSetup(void);
SHORT CBRead(UBYTE *,SHORT);
void  CBReadCleanup(void);
SHORT CBCurrentReadID(void);
SHORT CBCurrentWriteID(void);
BOOL  CBReadEqualsWrite(void);
BOOL  CBCheckSatisfy(SHORT *);
      CBCut(char *,SHORT);
SHORT CBPost(void);

/*#####################*/
/* protos for clip.c */

BOOL  OpenTextClip(void);
void  WriteTextClip(UBYTE *);
UBYTE *ExamineTextClip(void);
void  CloseTextClip(void);

/*#####################*/
/* protos for clip2.c */

void  GetLowHi(struct Display *,SHORT *,SHORT *,SHORT *,SHORT *);
void  InvertClipArea(struct Display *);
void  InvertRow(struct Display  *,SHORT,SHORT,SHORT);
SHORT RowSize(SHORT *,SHORT,SHORT,SHORT);
VOID  WriteSelectedArea(struct Display *);

/*#####################*/
/* protos for color.c */

struct Window *OpenColorWindow(struct Display *);
void  InitColorSizes(SHORT);
VOID  CloseColorWindow(BOOL,struct Display *);
VOID  ColorRange(SHORT,SHORT);
void  ColorWindowHit(SHORT,SHORT);
BOOL  ColorGadgetGotten(struct Gadget *,struct Display *);
void  ModifyColors(void);
void  GDrawBox(struct RastPort *, SHORT, SHORT, SHORT, SHORT);
void  DrawColorWindow(void);
void  SetColorProps(struct ViewPort *,struct RastPort *);
void  ColorRectFill(struct RastPort *,SHORT);
VOID  DoColorWindow(struct Display *);

/*#####################*/
/* protos for crt.c */

void  GetCRTSetUp(struct Display *,BOOL,BOOL);
UBYTE GetColorCRTSpecials(struct Display *, USHORT *);
void  GetMonoModes(struct Display *, USHORT *);
void  CopyCRTData(struct Display *, BOOL);

/*#####################*/
/* protos for cstask.c */

void  Cursor(void);
void  BlinkCursor(struct Display *);
void  CloseCursorTask(struct MsgPort *,struct IOStdReq *);

/*#####################*/
/* protos for curse.asm */
void __stdargs Curse(struct Display *);
void __stdargs NewCursings(struct Display *,LONG,LONG);

/*#####################*/
/* protos for disptask.c */

void  DisplayTask(struct Display *);
void  WindowEvent(struct Display *);
void  CloseDisplayTask(struct Display *);
VOID SetMenuState(struct Display *display);

/*#####################*/
/* protos for drawtext.asm */
void __stdargs DrawText(struct Display *);

/*#####################*/
/* protos for fileio.c */

/*BOOL  GetNextNumber(SHORT *, BYTE *, SHORT *, LONG);*/
/*BOOL  ShortGetNextNumber(SHORT *);*/
/*BOOL  ShortWrite(UBYTE *, struct Display *);*/
/*BOOL  SPrintfShortWrite(UBYTE *, ULONG,struct Display *);*/
/*BOOL  ReadSettingsGrunt(struct Display *, BOOL);*/
/*void  PrintSettings(void);*/
VOID  WriteSettingsFile(struct Display *);
BOOL  ReadSettingsFile(struct Display *, BOOL);

/*#####################*/
/* protos for help.c */

VOID  Help(SHORT, struct Display *);

/*#####################*/
/* protos for imtask.c */

void  InputMonitor(void);
void  QueuePCRawKey(struct IntuiMessage *);
void  QueueOneKey(UBYTE);
void  SendOneKey(void);
void  CloseInputMonitorTask(struct MsgPort *, struct MsgPort *, struct IOStdReq *, BOOL);
void  IrvingForcesFFDownZaphodsThroat(void);
BOOL ReadKeymap(VOID);

/*#####################*/
/* protos for init.c */

void  CopyFont(struct TextFont *, UBYTE *);
void  InitDisplayBitMap(struct Display *,struct SuperWindow *, SHORT);

/*#####################*/
/* protos for lock.c */

BOOL  MyLock(struct Lock *);
VOID  DeathLock(struct Lock *);
VOID  Unlock(struct Lock *);
VOID  LifeUnlock(struct Lock *);

/*#####################*/
/* protos for main.c */

void  main(int,char **);
/*BOOL  fndstr (UBYTE *, UBYTE *);*/
/*void  tolower(UBYTE *);*/

/*#####################*/
/* protos for menuhand.c */

void  MenuEvent(struct Display *,USHORT,LONG,LONG);
void  ProjectEvent(struct Display *,USHORT);
void  EditEvent(struct Display *,USHORT);
void  DisplayEvent(struct Display *,SHORT);

/*#####################*/
/* protos for misc.c */

struct Task *MyCreateTask(UBYTE *, UBYTE, APTR, ULONG);
void  CloseEverything(void);
void  DrainPort(struct MsgPort *);
void  FixPropPots(struct Gadget *);
ULONG ZAllocSignal(void);
void  ZFreeSignal(ULONG);
void  SetThosePrefs(void);
SHORT CreateSignalNumber(ULONG);
void  TopRightLines(struct Window *);
void  GetPCDisplay(struct Display *);
void  SetTimer(ULONG, ULONG, struct IOStdReq *);
void  DrawBox(struct RastPort *, SHORT, SHORT, SHORT, SHORT, SHORT, SHORT, SHORT);
void  PALBottomBorder(struct Window *, struct Display *);
void  MyRefreshWindowFrame(struct Window *, struct Display *);
ULONG ULongDivide(LONG, LONG);
void  ModifyDisplayProps(struct Display *);
void  WaitTOFing(SHORT);
SHORT TextXPos(struct Display *, SHORT);
SHORT TextYPos(struct Display *, SHORT);
void  XAddGadget(struct Window *, struct Gadget *, SHORT);

/*#####################*/
/* protos for newlines.asm */
void __stdargs RecalcNewLengths(struct Display *,LONG,LONG);

/*#####################*/
/* protos for open.c */

BOOL  OpenDisplay(struct Display *);
BOOL  OpenDisplayWindow(struct Display *, struct MsgPort *, ULONG,BOOL);
void  SetLeftOfCloseGadget(struct Window *);
USHORT *GetColorAddress(SHORT, SHORT);
void  SetColorColors(struct Display *, SHORT);

/*#####################*/
/* protos for recalc.c */

void RecalcDisplayParms(struct Display *);

/*#####################*/
/* protos for region.c */

void  SetMaskAndLock(struct Display *,BOOL);
void  UnmaskUnlock(struct Display *);
void  MoveAndSetRegion(struct Display *, SHORT, SHORT, SHORT, SHORT, SHORT, SHORT);
      /*AddRegion(struct Layer *, SHORT, SHORT, SHORT, SHORT);*/

/*#####################*/
/* protos for render.c */

void  RenderWindow(struct Display *,BOOL,BOOL,BOOL,BOOL);
void  RenderWindowGrunt(struct Display *, BOOL,BOOL,BOOL,BOOL,SHORT,SHORT);
void  RenderText(struct Display *, BOOL,SHORT,SHORT);
void  RenderGraphics(struct Display *);
void  RefreshDisplay(struct Display *);

/*#####################*/
/* protos for reqsupp.c */

void  DoRequest(struct ReqSupport *);

/*#####################*/
/* protos for revamp.c */

void  RevampDisplay(struct Display *,SHORT,BOOL,BOOL);

/*#####################*/
/* protos for sext.c */

struct Screen *GetExtraScreen(struct Display *, struct SuperWindow *);
void  ReleaseExtraScreen(struct Display *, struct Screen *);
void  LinkExtraScreen(struct Display *, struct Screen *);
void  UnlinkExtraScreen(struct Display *,struct Screen *);

/*#####################*/
/* protos for shared.c */

struct Screen *FindSharedScreen(struct Display *, struct NewScreen *);

/*#####################*/
/* protos for strings.c */

/*void  CopyString(UBYTE *,UBYTE *);*/
/*void  CopyStringLength(UBYTE *,UBYTE *,LONG);*/
/*SHORT StringLength(UBYTE *);*/
/*BOOL  StringsEqual(UBYTE *, UBYTE *);*/

/*#####################*/
/* protos for stuff.c */

void StuffTextBuffer(struct Display *,SHORT,SHORT);

/*#####################*/
/* protos for super.c */

VOID  InstallFirstWindow(struct Display *);
VOID  LinkSuperWindow(struct Display *, struct SuperWindow *);
void  SetPresets(struct Display *, struct NewWindow *, struct SuperWindow *,
                 SHORT, SHORT, SHORT, SHORT, SHORT, SHORT);
BOOL  GetNewSuperWindow(struct Display *, BOOL);
VOID  UnlinkSuperWindow(struct Display *, struct SuperWindow *);
VOID  SuperCloseWindow(struct Display *);
VOID  DestroySuperWindow(struct Display *);

/*#####################*/
/* protos for taskinit.c */

/*BOOL  InitDisplayTask(struct Display *);*/

/*#####################*/
/* protos for uline.asm */
void __stdargs Underliner(void);

/*#####################*/
/* protos for wait.c */

void  MakeWaitWindow(void);
void  UnmakeWaitWindow(void);
void  SetWaitPointer(SHORT, struct Window *);

/*#####################*/
/* protos for xread.c */

/*void  InitXRead(LONG);*/
/*void  XReadFill(void);*/
/*void  XReadCopy(UBYTE *,SHORT);*/
/*SHORT XRead(UBYTE *, SHORT);*/
/*SHORT XReadLine(UBYTE *, SHORT);*/

#endif
