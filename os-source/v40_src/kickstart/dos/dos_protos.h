#ifndef LIBRARIES_DOSEXTENS_H
#include <libraries/dosextens.h>
#endif
extern struct DosLibrary *DOSBase;

/************************************************************************/
/************************************************************************/
/***             Packet Level routines                               ****/
/************************************************************************/
/************************************************************************/
struct DosPacket *CreatePkt(VOID);
VOID DeletePkt(struct DosPacket *dp);
LONG DoPkt(struct MsgPort *port, LONG action,
                           LONG arg1, LONG arg2, LONG arg3, LONG arg4);
LONG DoPkt4(struct MsgPort *port, LONG action,
                            LONG arg1, LONG arg2, LONG arg3, LONG arg4);
LONG DoPkt3(struct MsgPort *port, LONG action,
                            LONG arg1, LONG arg2, LONG arg3);
LONG DoPkt2(struct MsgPort *port, LONG action, LONG arg1, LONG arg2);
LONG DoPkt1(struct MsgPort *port, LONG action, LONG arg1);
LONG DoPkt0(struct MsgPort *port, LONG action);
VOID SendPkt(struct MsgPort *port, struct DosPacket *dp);
 /* Waits for dos packet at pr_MsgPort, calls pr_PktWait if needed */
struct DosPacket *WaitPkt(VOID);
VOID ReplyPkt(struct DosPacket *dp, LONG res1, LONG res2);
VOID AbortPkt(struct MsgPort *port);

/************************************************************************/
/************************************************************************/
/***             Unbuffered File I/O                                 ****/
/************************************************************************/
/************************************************************************/
BPTR Open(UBYTE *, LONG);
VOID Close(BPTR);
LONG Read(BPTR, UBYTE *, LONG);
LONG Write(BPTR, UBYTE *, LONG);
BPTR Input(VOID);
BPTR Output(VOID);
LONG Seek(BPTR, LONG, LONG);
LONG IsInteractive(BPTR);
LONG WaitForChar(BPTR, LONG);
LONG VPrintf(UBYTE *, LONG[]);
LONG VFPrintf(BPTR, UBYTE *, LONG[]);
LONG VSPrintf(LONG, UBYTE *, LONG[], UBYTE *);
LONG SetFileSize(BPTR fh, LONG pos, LONG mode);
ULONG LockRecord(BPTR,ULONG,ULONG,ULONG,ULONG);
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
LONG ReadChar(VOID);
BOOL WriteChar(UBYTE ch);
ULONG UnReadChar(ULONG num);
LONG ReadBytes(UBYTE * buf, ULONG buflen);
LONG WriteBytes(UBYTE * buf, ULONG buflen);
LONG ReadLn(UBYTE * buf, ULONG buflen);
LONG WriteStr(UBYTE *str);
VOID BPrintf(UBYTE *format, LONG *t);
VOID Flush (BPTR fh);
ULONG BufferedSeek(BPTR fh,LONG,LONG);

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
/******* dos.library/SplitName ************
*
*   NAME
*	SplitName -- splits out a component of a pathname into a buffer
*
*   SYNOPSIS
*	newpos = SplitName(name, separator, buf, oldpos, size)
*	D0                  D1      D2      D3     D4     D5
*
*	WORD SplitName(UBYTE *, UBYTE, UBYTE *, WORD, LONG)
*
*   FUNCTION
*	This routine splits out the next piece of a name from a given file
*	name.  Each piece is copied into the buffer, truncating at size-1
*	characters.  The new position is then returned so that it may be
*	passed in to the next call to splitname.  If the separator is not
*	found within 'size' characters, then size-1 characters plus a null will
*	be put into the buffer, and the position of the next separator will
*	be returned.
*	If a a separator cannot be found, -1 is returned (but the characters
*	from the old position to the end of the string are copied into the
*	buffer, up to a maximum of size-1 characters).  Both strings are
*	null-terminated.
*
*   INPUTS
*	name      - Filename being parsed.
*	separator - Separator charactor to split by.
*	buf       - Buffer to hold separated name.
*	oldpos    - Current position in the file.
*	size 	  - Size of buf in bytes (including null termination);
*
*   RESULT
*	newpos    - New position for next call to splitname.  -1 for last one.
*/
WORD SplitName(UBYTE *name, UBYTE seperator, BYTE *buf, WORD oldpos, LONG size);
LONG SameLock(BPTR lock1, BPTR lock2);
LONG SetMode(BPTR fh, LONG mode);
VOID ExAll(BPTR lock, ); /* This needs to be thought out */

/************************************************************************/
/************************************************************************/
/***             Error Handling                                      ****/
/************************************************************************/
/************************************************************************/
LONG IoErr(VOID);
VOID SetIoErr(LONG result);
BOOL Fault(LONG code, UBYTE *buffer, LONG len);
LONG ErrorReport(LONG code, LONG type, ULONG arg1, struct MsgPort *device);
 /*    Bool := requestor( line1, line2, line3 ) */
 /* The three lines are displayed within an intuition requestor */
 /* box. Two gadgets are also presented, labelled Retry and Cancel */
 /* This call requires about 2K of stack, which is obtained dynamically */
 /* from the free pool. If there is no stack left it returns FALSE anyway. */
 /* Flags are which IDCMP flags will cause an automatic RETRY. */
LONG Requestor(UBYTE *s1, UBYTE *s2, UBYTE *s3, ULONG flags);

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
struct Process *CreateNewProc(struct NewProcess *newproc);
 /*                                                                      */
 /* rc := runcommand( seg, stacksize, paramptr, paramlen )               */
 /* BCPL:             BPTR   LONGS      BPTR      LONG                   */
 /* C:                BPTR   LONGS      CPTR      LONG                   */
 /*                                                                      */
 /* This code runs a command under TRIPOS in a language independent      */
 /* fashion. A stack of specified size is constructed, and the entry     */
 /* point of the code is assumed to be the first executable word of the  */
 /* first hunk in the segment list (which is a list of code blocks       */
 /* linked by BCPL pointers). All TRIPOS registers are saved, and the    */
 /* code is entered with the SP at the base of the new stack, having     */
 /* pushed a suitable return address, the size of the stack and the      */
 /* caller's stack onto it first of all. In addition the previous return */
 /* address for STOP is saved, and a new one installed.                  */
 /*                                                                      */
 /* Thus a program running under Tripos will find a stack as follows     */
 /*                                                                      */
 /* ReturnAddress    StackSize   CallerStack                             */
 /*                                                                      */
 /* The registers are set up as follows (any register may be corrupted   */
 /* except SP).                                                          */
 /*                                                                      */
 /* D0  Size of parameter string in bytes                                */
 /* A0  Pointer to parameter string area                                 */
 /*                                                                      */
 /* The routine returns                                                  */
 /* -1  if the program could not be loaded (result2() has more info)     */
 /* 0   if the program ran and did not set a return code                 */
 /* >0  if the program set a return code (result2() normally has more    */
 /*      info)                                                           */
 /*                                                                      */
LONG RunCommand(BPTR seg, LONG stack, UBYTE *paramptr, LONG paramlen);
struct MsgPort * ConsoleTask(VOID);
struct MsgPort * FileSystemTask(VOID);
LONG FindCli(LONG num);
LONG MaxCli(VOID);
BOOL SetCurrentDirName(UBYTE *name);
BOOL GetCurrentDirName(UBYTE *buf, LONG len);
BOOL SetProgramName(UBYTE *name);
BOOL GetProgramName(UBYTE *buf, LONG len);
BOOL SetPrompt(UBYTE *name);
BOOL GetPrompt(UBYTE *buf, LONG len);
BPTR GetProgramDir(VOID);
LONG System(UBYTE *command, BPTR input, BPTR output);

/************************************************************************/
/************************************************************************/
/***             Device List Management                              ****/
/************************************************************************/
/************************************************************************/
struct MsgPort *DeviceProc(UBYTE *);
LONG Assign(UBYTE *name, BPTR lock);
BOOL AssignLate(UBYTE *name, UBYTE *path);
BOOL AssignPath(UBYTE *name, UBYTE *path);
BOOL AssignAdd(UBYTE *name, BPTR Lock);
VOID GetDeviceProc(UBYTE *name,struct DevProc *dp);
VOID FreeDeviceProc(struct DevProc *dp);
struct DosList *LockDosList(ULONG flags);
VOID UnlockDosList(ULONG flags);
struct DosList *AttemptLockDosList(ULONG flags);
BOOL RemDosEntry(struct DosList *);
 /* should be called under forbid, so caller can set things up */
 /* creates a device of given name, type DLT_DEVICE, all other fields 0 */
struct DosList *AddDosEntry(UBYTE *name);
struct DosList *FindDosEntry(struct DosList *dlist, UBYTE *name, ULONG flags);
struct DosList *NextDosEntry(struct DosList *dlist, ULONG flags);
struct DosList *FindDosDevice(struct DosList *dlist, UBYTE *name, ULONG flags);
BOOL IsFileSystem(UBYTE *name);

/************************************************************************/
/************************************************************************/
/***             Handler Interface                                   ****/
/************************************************************************/
/************************************************************************/
BOOL Format(UBYTE *filesystem);
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

/************************************************************************/
/************************************************************************/
/***             Image Management                                    ****/
/************************************************************************/
/************************************************************************/
BPTR LoadSeg(UBYTE *name);
VOID UnLoadSeg(BPTR seglist);
BPTR InternalLoadSeg(BPTR fh, BPTR table, LONG *funcarray, LONG *stack);
VOID InternalUnLoadSeg(BPTR seglist, void (*freefunc)(UBYTE *, ULONG));
BPTR NewLoadSeg(UBYTE *file, LONG *stack);
 /* aka Resident().  Adds a node to the resident list. */
VOID Resident(UBYTE *name, BPTR seg, LONG system);

/************************************************************************/
/************************************************************************/
/***             Command Support                                     ****/
/************************************************************************/
/************************************************************************/
LONG CheckBreak(LONG mask);
LONG * RdArgs(UBYTE *keys,LONG *argv,LONG size, LONG isbcpl);
 /*  FINDARG - Cribbed from TRIPOS BLIB (BSTR->CSTR)	*/
 /*  Routine which searches the KEYS supplied		*/
 /*  to RDARGS to find whether an input WORD (w) 	*/
 /*  is part of the specified argument list in KEYS.	*/
 /*  If it is, it returns the argument number, otherwise */
 /*  -1 is returned.					*/
LONG FindArg(UBYTE *keys,UBYTE *w,LONG isbcpl);
 /* read a parameter in from a string into commandname   */
 /* returns -2    "=" Symbol                             */
 /*         -1    error                                  */
 /*          0    *N, ;, endstreamch                     */
 /*          1    unquoted item                          */
 /*          2    quoted item                            */
LONG RdItem(UBYTE *commandname,LONG maxlongs, LONG isbcpl);
LONG Atol(UBYTE *str);
BOOL FindFirst(UBYTE *pat, struct Anchor **anchor);
BOOL FindNext(struct Anchor **anchor);
VOID FreeAnchorChain(struct Anchor **anchor);
BOOL PreParse(UBYTE *pat, UBYTE *buf);
BOOL PatternMatch(UBYTE *pat, UBYTE *str);
VOID *AllocVec(LONG longwords);
VOID *AllocVecType(LONG longwords, LONG type);
VOID FreeVec(VOID *mem);

/************************************************************************/
/************************************************************************/
/***             Notification                                        ****/
/************************************************************************/
/************************************************************************/
BOOL StartNotify(struct NotifyRequest *notify);
VOID EndNotify(struct NotifyRequest *notify);

/************************************************************************/
/************************************************************************/
/***             32 Bit Math functions                               ****/
/************************************************************************/
/************************************************************************/
LONG Mult32(LONG num1, LONG num2);
LONG Div32(LONG dividend, LONG divisor);
LONG Rem32(LONG dividend, LONG divisor);

/************************************************************************/
/************************************************************************/
/***             Environment Variable functions                      ****/
/************************************************************************/
/************************************************************************/
BOOL SetEnvVar(UBYTE *name, UBYTE *buffer, LONG size, LONG flags);
LONG GetEnvVar(UBYTE *name, UBYTE *buffer, LONG size, LONG flags);


