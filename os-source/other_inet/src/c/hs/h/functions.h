/* -----------------------------------------------------------------------
 * functions.h      handshake_src
 *
 * $Locker:  $
 *
 * $Id: functions.h,v 1.1 91/05/09 14:29:28 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/functions.h,v 1.1 91/05/09 14:29:28 bj Exp $
 *
 * $Log:	functions.h,v $
 * Revision 1.1  91/05/09  14:29:28  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Handshake function Prototypes
*
***/

int __regargs DoColorWindow ( struct Screen *, int, int, int, int );

/*
* aboutreq.c
*/
void ShowAboutRequester ( void );
int  WaitForAbout ( void );
void RemoveAboutRequester ( void );
/*
* beep.c
*/
void MakeBeep ( unsigned long int, unsigned long int );
static void CleanUp ( void );
/*
*blink.c
*/
void __saveds BlinkTask ( void );
/*
* device_init.c
*/
struct MsgPort *InitSerialIO  ( struct IOExtSer *, unsigned char * );
struct MsgPort *InitConsoleIO ( struct IOStdReq *, unsigned char * );
struct MsgPort *InitTimer ( struct timerequest *, unsigned char   * );
int SetParams ( struct IOExtSer *, unsigned short int );
/*
* dial.c
*/
int DialRequest ( void );
void Dtr (unsigned char );
void HangUp ( void );
unsigned short int Dputc ( unsigned char );
unsigned char Dgetc ( void );
unsigned short int DialPuts ( unsigned char * );
void ReDial ( void );
void HangUpPhone ( void );
void ReDialDelay ( void );
unsigned short int Connected ( void );
void ConnectBeep ( void );
unsigned short int Dial ( unsigned char * );
void SetDialMessage ( unsigned char * );
void SetDialAttempts ( unsigned int );
void SetRetryDelay ( unsigned int );
void SetAnswerTimeout ( unsigned int );
void SetPhoneNumber ( unsigned char * );
unsigned char *StrToUpper ( unsigned char * );
unsigned short int DialInitialNumber ( unsigned char   * );
/*
* error.c
*/
int Error ( unsigned char * );
/*
* escape.c
*/
unsigned short int ESCOver (  char *, char * );
unsigned short int IsTerminator ( char );
void Escape ( char * );
void VT100ESC ( char * );
void VT52ESC ( char * );
void AnsiESC ( char   * );
void ChstESC ( char * );
void SC1TESC ( char * );
void MiscESC ( char * );
void Dcs ( unsigned char * );
void Cup ( void );
void Cuf ( void );
void Cud ( void );
void Cub ( void );
void Cuu ( void );
void Cpl ( void );
void Cnl ( void );
void DecTst ( void );
void DecSC ( void );
void DecRC ( void );
void Il ( void );
void Dl ( void );
void Ech ( void );
void Ich ( void );
void Dch ( void );
void Ind ( void );
void Nel ( void );
void Ri ( void );
void DecSCA ( unsigned char * );
void Tbc ( void );
void Hts ( void );
void Ris ( void );
void PrintIt ( unsigned short int, unsigned short int, unsigned char );
void Mc ( char * );
void VT52Mc ( char * );
void Dsr ( char * );
void DecLL ( void );
void Cpr ( void );
void DecReqtParm ( void );
void DecReptParm ( void );
void SetFontAttributes ( void );
void Sgr ( char * );
void Ed (unsigned char *);
void El (unsigned char *);
void DecSED ( void );
void DecSEL ( void );
void Ca ( char * );
void BigLetters ( char * );
void DecALN ( void );
void DecSTBM ( void );
void DecSCL (unsigned char *);
void ModeSet ( char * , int);
void PnParse ( char * , unsigned short int * );
void DefaultP (int, int, int);
void ResetP ( void );
void Reverse ( char * );
void Respond ( unsigned char * );
void SwitchKeyboard ( void );
unsigned short int CSTCDI ( char * , int );
/*
* fonts.c
*/
void FontInit ( void );
void CloseFonts ( void );
void FreeMainFont ( struct TextFont * );
void FreeFont ( struct TextFont * );
void BuildDoubleWidthFont ( unsigned char *, struct TextFont *,
                                                struct TextFont * );
void BuildDoubleHeightFont ( unsigned char *,struct TextFont *,
                                                struct TextFont *,
                                                unsigned short int );
void BuildNarrowWidthFont ( unsigned char *,struct TextFont *,
                                                struct TextFont * );
void BuildDSCSFont ( struct TextFont *, struct TextFont * );
void BuildSoftFont ( unsigned char * );
void SetSoftChar ( unsigned short int, unsigned char * );
void ClearSoftFont ( void );

/*
* function_keys.c
*/
int SetFunctionKeys ( void );
/*
* handshake.c
*/
void ProcessWorkbenchArguments ( char * );
void __stdargs main ( int, char ** );
void ProcessWorkbenchArguments ( char * );
void ProcessCommandLine ( int, char **, char * );
void ProcessUntil ( unsigned short int );
void ShutDown ( void );
int  MakeVtWindow ( unsigned short int );
void NVMInit ( void );
void ScreenInit ( unsigned short int, unsigned short int );
void TermInit ( void );
void OpenSerialPort ( void );
void ConfigureScreen ( void );
void SleepTasks ( void );
void WakeTasks ( void );
void ConfigureSerialPorts ( void );
void StartSerialTasks ( void );
void StartBlinkTask ( void );
void StopSerialTasks ( void );
void StopBlinkTask ( void );
char *HSAllocMem ( long, long );
void HSFreeMem   ( char *, long );
unsigned short int ToPrinter ( unsigned short int, unsigned short int,
                               unsigned char );
void ControlPrint ( unsigned char );
/*
* kermit.c
*/
unsigned short int __stdargs BeginKermitReceive ( unsigned int, unsigned char * );
unsigned short int __stdargs BeginKermitSend ( unsigned int, unsigned char * );
void Sendsw ( void );
char Sinit ( void );
char Sfile ( void );
char Sdata ( void );
char SEOF ( void );
char Sbreak ( void );
void Recsw ( void );
char Rinit ( void );
char Rfile ( void );
char Rdata ( void );
void Connect ( void );
char ToChar ( char );
char UnChar ( char );
char Ctl ( char );
void Spack ( char ,int , int, char * );
int  Rpack ( int *, int *, char * );
int  Bufill ( char * );
void Bufemp ( char * , int );
int  GetFil ( void );
int  GNxtFl ( void );
void ShortName ( unsigned char *);
void Spar ( char * );
void Rpar ( char * );
int  ReadSer ( char *, int );
int  WriteSer ( char *, int );
void SetKermitParameters ( void );
void RestoreKermitParameters ( void );
void DisplayPacket ( unsigned char *, unsigned short int );
/*
* keypad.c
*/
void SetKeyboard ( void );
void InitKeyboard ( void );
void SaveOriginalKeyMap ( void );
void UpdateFunctionKeys ( void );
void SetFKey ( unsigned char * );
void SetFunctionKey ( unsigned char * );
void SetCursorKey ( unsigned char * );
void SetAnsiCursorKey ( unsigned char * );
void SetKPKey ( unsigned char *, unsigned char );
void Set200Key ( unsigned char * );
void SetESCandBsKey ( unsigned char *, unsigned char );
void SetNormalBksp ( void );
void SetReverseBksp ( void );
/*
* lockreq.c
*/
void HandleLockedLine ( struct IOExtSer * );
/*
menus.c
*/
void InitMenu ( void );
void SetScreen24 ( void );
void SetScreen48 ( void );
void ProcIDCMPReq ( unsigned long, unsigned short );
void UncheckMenu ( struct MenuItem * );
void UncheckMenuStrip ( void );
void CheckFreq ( void );
void CheckPrinter ( void );
void CheckIcons ( void );
void CheckBaudRate ( void );
void CheckDataBits ( void );
void CheckStopBits ( void );
void CheckParity ( void );
void CheckHandshake ( void );
void CheckFlow ( void );
void CheckPort ( void );
void CheckMode ( void );
void CheckWrap ( void );
void CheckEcho ( void );
void CheckScroll ( void );
void CheckCursor ( void );
void CheckCharSet ( void );
void CheckNewLine ( void );
void CheckScreen ( void );
void CheckColumns ( void );
void CheckInterlace ( void );
void CheckBkspDel ( void );
void CheckBellType ( void );
void CheckMarBell ( void );
void CheckDialMode ( void );
void CheckDialTime ( void );
void CheckDialDelay ( void );
void CheckHangUp ( void );
void CheckTxCR ( void );
void CheckTxNL ( void );
void CheckRxCR ( void );
void CheckRxNL ( void );
void CheckProtocol ( void );
void CheckBitPlane ( void );
void CheckMenuStrip ( void );
void RefreshMenu ( void );
int LoadNVR ( void );
int SaveNVR ( void );
/*
* serial_in.c
*/
void ShutDownInput ( void );
void Vputc ( unsigned char );
void ScrFlush ( void );
unsigned char Lgetc ( void );
unsigned char *Lgets ( void );
void RexxMatch (  char );
/*
* serial_out.c
*/
void ShutDownOutput ( void );
void Lputc ( unsigned char );
/*
*stringreq.o
*/
int GetStringFromUser ( unsigned char *, unsigned char *, unsigned int );
/*
* tabreq.c
*/
int  TabRequest ( unsigned char * );
void TabSet ( void );
/*
* task_starts.c
*/
void __saveds SerialOutput ( void );
void __saveds SerialInput ( void );
/*
* transfer.c
*/
void Binit ( void );
int  Bflush ( void );
int  Bwrite ( unsigned char *, short int );
unsigned char **FileList ( unsigned char * );
void CrcTblI ( void );
void FTCBInit ( void );
void FTCBClose ( void );
int  OpenTransferWindow ( unsigned char *, unsigned short int, 
                                             unsigned short int );
void CloseTransferWindow ( void );
void SetSendProtocol ( unsigned char * );
void SetReceiveProtocol ( unsigned char * );
void SetMessage ( unsigned char * );
void SetErrorCounts ( unsigned int, unsigned int );
void SetBytes ( unsigned long int );
void SetSize ( unsigned long int );
void SetReceived ( unsigned long int );
void SetBlock ( unsigned int );
void SetByteCount ( unsigned long int );
void SetFile ( unsigned char * );
int  CheckForWindowAbort ( void );
void TransferMessage ( unsigned char * );
unsigned short int BeginASCIIReceive ( unsigned char * );
void EndASCIIReceive ( void );
unsigned short int BeginASCIISend ( unsigned char * );
int  Aputc ( unsigned char );
void Capture ( unsigned char );
void Fstore ( unsigned char );
void FlushCaptureBuffer ( void );
void SetXmodemParameters ( void );
void RestoreParameters ( void );
void CompleteBeep ( void );
void FailedBeep ( void );
void SendCANs ( void );
unsigned short int __stdargs BeginXmodemSend ( unsigned char * );
unsigned short int __stdargs BeginYmodemSend ( unsigned char * );
void StartTimer ( struct timerequest *, unsigned int );
void StartRead ( unsigned char *, unsigned int );
unsigned char GetOneByteFromSet ( char * );
void SendHeaderBlock ( unsigned char *, unsigned long int );
void SendBlock ( void );
unsigned short int __stdargs BeginXmodemReceive ( unsigned short int, unsigned char * );
unsigned short int __stdargs BeginYmodemReceive ( unsigned short int, unsigned char * );
void SendChar ( unsigned char );
int  GetBlock ( void );
int  VerifyBlock ( void );
void DrainLine ( void );
void ClrCRC ( void );
void UpdCRC ( unsigned char *, unsigned short int );
unsigned short int TransmitFile ( unsigned short int, unsigned char * );
unsigned short int ReceiveFile ( unsigned short int, unsigned char * );
/*
*video.c
*/
void VCBInit ( unsigned short int, unsigned short int );
void DrawLogo ( void );
void Beep ( void );
void VscrollUpAttr ( short int, short int,short int );
void VscrollDownAttr ( short int, short int, short int );
void VscrollRightAttr ( short int );
void VscrollLeftAttr ( short int );
void VsetLAttr ( int );
void VsetCAttr ( unsigned char );
unsigned short int FontSize ( void );
void Vsetcup ( void );
void PlaceCursor ( void );
void RemoveCursor ( void );
void VwipeEOL ( void );
void VwipeSOL ( void );
void Vcls ( int, int );
void VswipeEOL ( void );
void VswipeSOL ( void );
void Vscls ( int, int );
void VscrollUp ( void );
void VscrollDn ( void );
void ChangeFont ( struct TextFont * );
void VtDspCh ( unsigned char );
void Vtty ( unsigned char );
void Vlocate ( unsigned int, unsigned int );
void InsChar ( void );
void SaveOrigFgBg ( void );
void SetOrigFgBg ( void );
void SwapFgBg ( void );
void SetColors ( void );
void SaveColors ( void );
unsigned short int ScreenSafe ( void );
/*
* Filereq.c
*/
void SetPattern ( BYTE * );
ULONG __stdargs FRFunction ( ULONG, CPTR );
void BuildFullName ( unsigned char *, unsigned char * );
void FileReqDefaults ( unsigned char * );
unsigned short int FileReq ( unsigned char *, unsigned char * );

/* Prototypes for functions defined in rexx.c */
int __stdargs RexxFunc(struct RexxMsg *rmsg,
                       struct rexxCommandList *rc);
int __stdargs RexxComm(struct RexxMsg *rmsg,
                       struct rexxCommandList *rc);
void StartRexx(void);
void StopRexx(void);
long upRexxPort(char *s,
               char *exten );
static void closeRexxLib(void);
void dnRexxPort(void);
void dispRexxPort(void);
static int cmdcmp(register char *c,
                  register char *m);
static int openRexxLib(void);
struct RexxMsg *sendRexxCmd(long type,
                            char *s,
                            int (*f)(void),
                            unsigned char *p1,
                            unsigned char *p2,
                            unsigned char *p3);
struct RexxMsg *asyncRexxCmd(long type,
                             char *s);
static void __stdargs replytoit(register struct RexxMsg *msg);
struct RexxMsg *syncRexxCmd(long type,
                            char *s,
                            struct RexxMsg *msg);
int replyRexxCmd(register struct RexxMsg *msg,
                 register long primary,
                 register long secondary,
                 register char *string);

void DeleteArgstring ( void * );
void DeleteRexxMsg   ( struct RexxMsg * );
struct RexxMsg *CreateRexxMsg   ( struct MsgPort *, char *, char * );
void *CreateArgstring ( char *, long );
struct RexxMsg *AddFuncHost(char *s, char *priority);

/*
* xprotocol.c
*/
/* Prototypes for functions defined in xprotocol.c */
void InitializeXProtocol ( void );
void XProtocolParams ( void );
void ShutXProtocol ( void );
long XprHostMon ( char *, int, int );
long XprUserMon ( char *, int, int );
unsigned short __stdargs BeginXProtocolSend(unsigned char *source);
unsigned short __stdargs BeginXProtocolReceive(unsigned char *destination);
long __asm __stdargs __saveds xpr_fopen(register __a0 char * filename,
                                        register __a1 char * accessmode);
long __asm __stdargs __saveds xpr_fclose(register __a0 struct _iobuf * filepointer);
long __asm __stdargs __saveds xpr_fread(register __a0 char * buffer,
                                        register __d0 long size,
                                        register __d1 long count,
                                        register __a1 struct _iobuf * filepointer);
long __asm __stdargs __saveds xpr_fwrite(register __a0 char * buffer,
                                         register __d0 long size,
                                         register __d1 long count,
                                         register __a1 struct _iobuf * filepointer);
long __asm __stdargs __saveds xpr_sread(register __a0 char * buffer,
                                        register __d0 long size,
                                        register __d1 long timeout);
long __asm __stdargs __saveds xpr_swrite(register __a0 char * buffer,
                                         register __d0 long size);
long __asm __stdargs __saveds xpr_sflush(void);
long __asm __stdargs __saveds xpr_update(register __a0 struct XPR_UPDATE * updatestruct);
long __asm __stdargs __saveds xpr_chkabort(void);
long __asm __stdargs __saveds xpr_chkmisc(void);
long __asm __stdargs __saveds xpr_gets(register __a0 char * prompt,
                                       register __a1 char * buffer);
long __asm __stdargs __saveds xpr_ffirst(register __a0 char * buffer,
                                         register __a1 char * pattern);
long __asm __stdargs __saveds xpr_fnext(register __d0 long oldstate,
                                        register __a0 char * buffer,
                                        register __a1 char * pattern);
long __asm __stdargs __saveds xpr_finfo(register __a0 char * filename,
                                        register __d0 long typeofinfo);
long __asm __stdargs __saveds xpr_fseek(register __a0 struct _iobuf * filepointer,
                                        register __d0 long offset,
                                        register __d1 long origin);
void xpr_setup(struct XPR_IO *xpr_io);
void SayTransfer ( int x, int y, unsigned char *control, unsigned char *strarg,
                   long longarg1, long longarg2 );
