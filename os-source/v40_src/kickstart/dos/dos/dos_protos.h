#ifndef CLIB_DOS_PROTOS_H
#define CLIB_DOS_PROTOS_H

#ifndef DOS_DOSEXTENS_H
#include "dos/dosextens.h"
#endif

/***********************************************************************
/***********************************************************************
/***             Object creation/deletion                            ***
/***********************************************************************
/***********************************************************************
VOID *AllocDosObject(ULONG type,struct TagItem *tags);
VOID FreeDosObject(ULONG type,VOID *ptr);
/************************************************************************/
/************************************************************************/
/***             Packet Level routines                               ****/
/************************************************************************/
/************************************************************************/
LONG DoPkt(struct MsgPort *port, LONG action,
                           LONG arg1, LONG arg2, LONG arg3, LONG arg4);
LONG DoPkt4(struct MsgPort *port, LONG action,
                            LONG arg1, LONG arg2, LONG arg3, LONG arg4);
LONG DoPkt3(struct MsgPort *port, LONG action,
                            LONG arg1, LONG arg2, LONG arg3);
LONG DoPkt2(struct MsgPort *port, LONG action, LONG arg1, LONG arg2);
LONG DoPkt1(struct MsgPort *port, LONG action, LONG arg1);
LONG DoPkt0(struct MsgPort *port, LONG action);
VOID SendPkt(struct DosPacket *dp, struct MsgPort *port,
	     struct MsgPort *replyport);
struct DosPacket *WaitPkt(VOID);
VOID ReplyPkt(struct DosPacket *dp, LONG res1, LONG res2);
VOID AbortPkt(struct MsgPort *port);
/************************************************************************/
/************************************************************************/
/***             Record Locking                                      ****/
/************************************************************************/
/************************************************************************/
BOOL LockRecord(BPTR,ULONG,ULONG,ULONG,ULONG);
BOOL LockRecords(struct RecLock *, ULONG);
BOOL UnLockRecord(BPTR,ULONG,ULONG);
BOOL UnLockRecords(struct RecLock *);
/************************************************************************/
/************************************************************************/
/***             Buffered File I/O                                   ****/
/************************************************************************/
/************************************************************************/
BPTR SelectInput(BPTR fh);
BPTR SelectOutput(BPTR fh);
LONG FGetC(BPTR fh);
VOID FPutC(BPTR fh, UBYTE ch);
LONG UnGetC(BPTR fh, LONG character);
LONG FRead(BPTR fh,VOID *block, ULONG blocklen,ULONG number);
LONG FWrite(BPTR fh,VOID *block, ULONG blocklen,ULONG number);
LONG FGets(BPTR fh, UBYTE *buf, ULONG buflen);
LONG FPuts(BPTR fh, UBYTE *str);
VOID VFWritef(BPTR fh, UBYTE *format, LONG *argarray);
LONG VFPrintf(BPTR fh, UBYTE *format, LONG *argarray);
VOID Flush (BPTR fh);
LONG SetVBuf(BPTR fh, LONG type, ULONG size);
LONG WriteChars(UBYTE *buf, ULONG buflen);
LONG PutStr(UBYTE *str);
LONG VPrintf(UBYTE *format, LONG *argarray);
/************************************************************************/
/************************************************************************/
/***             Unbuffered File I/O                                 ****/
/************************************************************************/
/************************************************************************/
BPTR Open(UBYTE *, LONG);
LONG Close(BPTR);
LONG Read(BPTR, VOID *, LONG);
LONG Write(BPTR, VOID *, LONG);
BPTR Input(VOID);
BPTR Output(VOID);
LONG Seek(BPTR, LONG, LONG);
LONG IsInteractive(BPTR);
LONG WaitForChar(BPTR, LONG);
/************************************************************************/
/************************************************************************/
/***             Object Management                                   ****/
/************************************************************************/
/************************************************************************/
LONG DeleteFile(UBYTE *);
LONG Rename(UBYTE *, UBYTE *);
BPTR Lock(UBYTE *, LONG);
VOID UnLock(BPTR);
BPTR DupLock(BPTR);
LONG Examine(BPTR, struct FileInfoBlock *);
LONG ExNext(BPTR, struct FileInfoBlock *);
LONG Info(BPTR, struct InfoData *);
BPTR CreateDir(UBYTE *);
LONG SetComment(UBYTE *name, UBYTE *str);
LONG SetProtection(UBYTE *name, LONG protect);
BPTR ParentDir(BPTR lock);
BPTR DupLockFromFH(BPTR fh);
BPTR OpenFromLock(BPTR lock, LONG mode);
BPTR ParentOfFH(BPTR fh);
BOOL ExamineFH(BPTR fh,struct FileInfoBlock *fib);
LONG SetFileDate(UBYTE *name,struct DateStamp *date);
LONG NameFromLock(BPTR lock, UBYTE *buffer, LONG len);
BOOL NameFromFH(BPTR fh,UBYTE *, LONG);
WORD SplitName(UBYTE *name, UBYTE seperator,UBYTE *buf, WORD oldpos, LONG size);
LONG SameLock(BPTR lock1, BPTR lock2);
LONG SetMode(BPTR fh, LONG mode);
VOID ExAll(BPTR lock, struct ExAllData *buffer, LONG size, LONG data,
	   struct ExAllControl *control);
LONG ReadLink(struct MsgPort *port, BPTR lock, UBYTE *path, UBYTE *buffer,
	      ULONG size);
LONG MakeLink(UBYTE *name, LONG dest, LONG soft);
LONG ChangeMode(LONG type, BPTR fh, LONG newmode);
LONG SetFileSize(BPTR fh, LONG pos, LONG mode);

/************************************************************************/
/************************************************************************/
/***             Error Handling                                      ****/
/************************************************************************/
/************************************************************************/
LONG IoErr(VOID);
VOID SetIoErr(LONG result);
BOOL Fault(LONG code, UBYTE *header, UBYTE *buffer, LONG len);
BOOL PrintFault(LONG code, UBYTE *header);
LONG ErrorReport(LONG code, LONG type, ULONG arg1, struct MsgPort *device);
LONG Requester(UBYTE *s1, UBYTE *s2, UBYTE *s3, ULONG flags);

/************************************************************************/
/************************************************************************/
/***             Process Management                                  ****/
/************************************************************************/
/************************************************************************/
struct MsgPort *CreateProc(UBYTE *, LONG, BPTR, LONG);
LONG Execute(UBYTE *, BPTR, BPTR);
VOID Exit(LONG);
BPTR CurrentDir(BPTR);
struct CommandLineInterface *Cli(VOID);
struct Process *CreateNewProc(struct TagItem *tags);
LONG RunCommand(BPTR seg, LONG stack, UBYTE *paramptr, LONG paramlen);
struct MsgPort * GetConsoleTask(VOID);
VOID SetConsoleTask(struct MsgPort *task);
struct MsgPort * GetFileSysTask(VOID);
VOID SetFileSysTask(struct MsgPort *task);
UBYTE *GetArgStr(VOID);
BOOL SetArgStr(UBYTE *string);
struct Process * FindCliProc(LONG num);
ULONG MaxCli(VOID);
BOOL SetCurrentDirName(UBYTE *name);
BOOL GetCurrentDirName(UBYTE *buf, LONG len);
BOOL SetProgramName(UBYTE *name);
BOOL GetProgramName(UBYTE *buf, LONG len);
BOOL SetPrompt(UBYTE *name);
BOOL GetPrompt(UBYTE *buf, LONG len);
BPTR GetProgramDir(VOID);
LONG System(UBYTE *command, struct TagItem *tags);

/************************************************************************/
/************************************************************************/
/***             Device List Management                              ****/
/************************************************************************/
/************************************************************************/
struct MsgPort *DeviceProc(UBYTE *);
LONG AssignLock(UBYTE *name, BPTR lock);
BOOL AssignLate(UBYTE *name, UBYTE *path);
BOOL AssignPath(UBYTE *name, UBYTE *path);
BOOL AssignAdd(UBYTE *name, BPTR Lock);
LONG RemAssignList(UBYTE *name, BPTR lock);
struct DevProc *GetDeviceProc(UBYTE *name,struct DevProc *dp);
VOID FreeDeviceProc(struct DevProc *dp);
struct DosList *LockDosList(ULONG flags);
VOID UnlockDosList(ULONG flags);
struct DosList *AttemptLockDosList(ULONG flags);
BOOL RemDosEntry(struct DosList *);
struct DosList *AddDosEntry(struct DosList *dlist);
struct DosList *FindDosEntry(struct DosList *dlist, UBYTE *name, ULONG flags);
struct DosList *NextDosEntry(struct DosList *dlist, ULONG flags);
struct DosList *MakeDosEntry(UBYTE *name,LONG type);
VOID FreeDosEntry(struct DosList *dlist);
BOOL IsFileSystem(UBYTE *name);

/************************************************************************/
/************************************************************************/
/***             Handler Interface                                   ****/
/************************************************************************/
/************************************************************************/
BOOL Format(UBYTE *filesystem,UBYTE *volumename,ULONG dostype);
LONG Relabel(UBYTE *drive,UBYTE *newname);
LONG Inhibit(UBYTE *name, LONG onoff);
LONG AddBuffers(UBYTE *name, LONG number);

/************************************************************************/
/************************************************************************/
/***             Date Time Routines                                  ****/
/************************************************************************/
/************************************************************************/
VOID Delay(LONG);
LONG * DateStamp(LONG *);
LONG CompareDates(struct DateStamp *date1, struct DateStamp *date2);
LONG DateToStr(struct DateTime *datetime);
LONG StrToDate(struct DateTime *datetime);

/************************************************************************/
/************************************************************************/
/***             Image Management                                    ****/
/************************************************************************/
/************************************************************************/
BPTR LoadSeg(UBYTE *name);
VOID UnLoadSeg(BPTR seglist);
BPTR InternalLoadSeg(BPTR fh, BPTR table, LONG *funcarray, LONG *stack);
VOID InternalUnLoadSeg(BPTR seglist, void (*freefunc)(UBYTE *, ULONG));
BPTR NewLoadSeg(UBYTE *file, struct TagItem *tags);
LONG AddSegment(UBYTE *name, BPTR seg, LONG system);
struct Segment *FindSegment(UBYTE *name, struct Segment *seg, LONG system);
LONG RemSegment(struct Segment *seg);
/************************************************************************/
/************************************************************************/
/***             Command Support                                     ****/
/************************************************************************/
/************************************************************************/
LONG CheckSignal(LONG mask);
struct RDargs * ReadArgs(UBYTE *template, LONG *array, struct RDargs *args);
LONG FindArg(UBYTE *keyword,UBYTE *template);
LONG ReadItem(UBYTE *name,LONG maxchars,struct CHSource *CHSource);
LONG StrToLong(UBYTE *string, LONG *value);
LONG MatchFirst(UBYTE *pat, struct AnchorChain *anchor);
LONG MatchNext(struct AnchorChain *anchor);
VOID MatchEnd(struct AnchorChain *anchor);
BOOL ParsePattern(UBYTE *pat, UBYTE *buf, LONG buflen);
BOOL MatchPattern(UBYTE *pat, UBYTE *str);
BOOL MatchReplace();			/* arguments not specified yet */
VOID FreeArgs(struct RDargs *args);
UBYTE *FilePart(UBYTE *path);
UBYTE *PathPart(UBYTE *path);
BOOL AddPart(UBYTE *dirname,UBYTE *filename,ULONG size);

/************************************************************************/
/************************************************************************/
/***             Notification                                        ****/
/************************************************************************/
/************************************************************************/
BOOL StartNotify(struct NotifyRequest *notify);
VOID EndNotify(struct NotifyRequest *notify);

/************************************************************************/
/************************************************************************/
/***             Environment Variable functions                      ****/
/************************************************************************/
/************************************************************************/
BOOL SetVar(UBYTE *name, UBYTE *buffer, LONG size, LONG flags);
LONG GetVar(UBYTE *name, UBYTE *buffer, LONG size, LONG flags);
LONG DeleteVar(UBYTE *name, ULONG flags);
struct LocalVar *FindVar(UBYTE *name, ULONG type);

#endif	/* CLIB_DOS_PROTOS_H */
