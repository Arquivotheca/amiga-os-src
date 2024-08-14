#ifndef CLIB_APPSHELL_IPROTOS_H
#define	CLIB_APPSHELL_IPROTOS_H

/* internal function prototypes */

/* apsh_projmng.c */
BOOL __asm ExpandPattern ( register __a1 struct AppInfo *ai , register __a2 STRPTR cmd , register __a3 struct TagItem *tl );
BOOL GetBaseInfo ( struct AppInfo *ai , int argc , char **argv , struct WBStartup *wbm );
struct ProjNode *NewProject ( struct AppInfo *ai , STRPTR anchor , struct TagItem *tl );
VOID RenumProjects ( struct AppInfo *ai , STRPTR cmd , struct TagItem *tl );
BOOL RemoveProject ( struct AppInfo *ai , STRPTR cmd , struct TagItem *tl );
BOOL AddProjects ( struct AppInfo *ai , LONG numargs , struct WBArg *args , struct TagItem *tl );
VOID FreeProject ( struct ProjNode *pn );
LONG __asm UpdateProject ( register __a1 struct AppInfo *ai , register __a2 struct ProjNode *pn , register __a3 struct WBArg *arg );
VOID FreeProjects ( struct AppInfo *ai , struct TagItem *tl );
struct ProjNode *GetProjNode ( struct AppInfo *ai , LONG cnt , struct TagItem *tl );
VOID SwapProjNodes ( struct AppInfo *ai , struct ProjNode *sn1 , struct ProjNode *sn2 , struct TagItem *tl );

/* apsh_main.c */
BOOL HandleApp ( int argc , char **argv , struct WBStartup *wbm , struct TagItem *anchor );
BOOL HandleAppAsync ( struct TagItem *anchor , struct MsgPort *sipc );
BOOL HandlerFunc ( struct AppInfo *ai , ULONG tags , ...);
APTR HandlerData ( struct AppInfo *ai , ULONG tags , ...);
BOOL HandlerFuncA ( struct AppInfo *ai , struct TagItem *tl );
APTR HandlerDataA ( struct AppInfo *ai , struct TagItem *tl );
LONG __asm LockAppInfo ( register __a1 struct AppInfo *ai );
VOID __asm UnlockAppInfo ( register __d0 LONG key );

/* apsh_idcmp.c */
struct MsgHandler *setup_idcmpA ( struct AppInfo *ai , struct TagItem *tl );
BOOL handle_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *ptl );
BOOL shutdown_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID CloseWindowSafely ( struct Window *win );
BOOL setup_key_array ( struct MsgHandler *mh , struct KeyboardCMD *KeyArray );
VOID shutdown_key_array ( struct MsgHandler *mh );
LONG DeadKeyConvert ( struct IntuiMessage *msg , UBYTE *kbuffer , LONG kbsize , struct KeyMap *kmap );
VOID HandleKeyEvent ( struct AppInfo *ai , struct MsgHandler *mh , struct IntuiMessage *msg , struct TagItem *tl );
VOID bobDrawGList ( struct RastPort *rport , struct ViewPort *vport );
VOID MoveBob ( struct RastPort *rp , struct ViewPort *vp , struct Bob *Bob , SHORT X , SHORT Y );
int dumpTagList ( UBYTE *str , struct TagItem *tags );
VOID SendIcon ( struct AppInfo *ai , struct Window *dwin , struct ObjectNode *on , WORD x , WORD y );
VOID HandleGadgEvent ( struct AppInfo *ai , struct MsgHandler *mh , struct IntuiMessage *msg , struct TagItem *tl );
VOID HandleMenuEvent ( struct AppInfo *ai , struct MsgHandler *mh , struct IntuiMessage *msg , struct TagItem *tl );
BOOL APSHGetGadgetInfo ( struct AppInfo *ai , STRPTR wname , STRPTR gname , ULONG *window , ULONG *gadget );
BOOL APSHGetWindowInfo ( struct AppInfo *ai , STRPTR wname , ULONG *window );

/* animtools.c */
#if 0
struct GelsInfo *setupGelSys ( struct RastPort *rPort , BYTE reserved );
VOID cleanupGelSys ( struct GelsInfo *gInfo , struct RastPort *rPort );
struct VSprite *makeVSprite ( NEWVSPRITE *nVSprite );
struct Bob *makeBob ( NEWBOB *nBob );
struct Bob *makeImageBob ( NEWIMAGEBOB *nIBob );
VOID freeVSprite ( struct VSprite *vsprite );
VOID freeBob ( struct Bob *bob , LONG rasdepth );
#endif

/* apsh_misc.c */
LONG WhichOption ( struct AppInfo *ai , STRPTR word );
STRPTR GetText ( struct AppInfo *ai , ULONG baseid , ULONG tid , STRPTR def );
STRPTR PrepText ( struct AppInfo *ai , ULONG baseid , ULONG tid , int va_alist );
ULONG ParseLine ( STRPTR line , STRPTR *argv );
STRPTR BuildParseLine ( STRPTR line , ULONG *argc , STRPTR *argv );
VOID FreeParseLine ( STRPTR clone );
STRPTR FindType ( STRPTR *array , STRPTR name , STRPTR defvalue );
BOOL MatchValue ( STRPTR type , STRPTR value );
BOOL QStrCmpI ( STRPTR str1 , STRPTR str2 );
VOID RemoveMsgPort ( struct MsgPort *mp );
BOOL NotifyUser ( struct AppInfo *ai , STRPTR msg , struct TagItem *tl );
VOID SetPPointer ( struct Window *win , struct PointerPref *pp );
VOID APSHSetWaitPointer ( struct AppInfo *ai , struct TagItem *tl );
VOID APSHClearPointer ( struct AppInfo *ai , struct TagItem *tl );
int atoi ( char *ptr );
char *strupr ( char *str );
LONG LMatchFirst ( struct List *list , STRPTR pat , struct AnchorList *al );
LONG LMatchNext ( struct AnchorList *al );
VOID LMatchEnd ( struct AnchorList *al );
void AlphaEnqueue ( struct List *list , struct Node *node );
VOID HandleList (struct List * list, VOID (*func)(struct Node *, ULONG, void *, ...), void *data,...);
ULONG __asm APSHSignal ( register __a1 struct AppInfo *ai , register __d0 LONG bit );
STRPTR __regargs GT_Lin(struct AppInfo *, STRPTR *, ULONG, STRPTR);

/* apsh_stdfuncs.c */
VOID AliasFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID able_func ( struct AppInfo *ai , struct FuncEntry *fe , BOOL stat );
VOID AbleFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl , BOOL stat );
VOID DisableFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID EnableFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID ExecMacroFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID FaultFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *ti );
VOID GetFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID GroupFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID HelpFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID LearnFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID LoadMacroFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID PriorityFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID SaveMacroFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID SelectFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID SetFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID StatusFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID StubFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID SystemFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID WhyFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );

/* wbarg.c */
struct DiskObject *IconFromWBArg ( struct WBArg *arg );

/* apsh_functable.c */
BOOL AddFuncEntry ( struct AppInfo *ai , struct Funcs *sfe );
VOID FreeFuncEntry ( struct AppInfo *ai , struct FuncEntry *fe );
BOOL AddFuncEntries ( struct AppInfo *ai , struct Funcs *fels );
VOID FreeFuncEntries ( struct AppInfo *ai );
struct FuncEntry *GetFuncEntry ( struct AppInfo *ai , STRPTR anchor , ULONG fid );
ULONG GetFuncID ( struct AppInfo *ai , STRPTR anchor );
STRPTR GetFuncName ( struct AppInfo *ai , ULONG id );

/* apsh_msghandle.c */
BOOL AddMsgHandlerA ( struct AppInfo *ai , struct TagItem *tl );
BOOL AddIntMsgHandlerA ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID DelMsgHandlers ( struct AppInfo *ai );

/* apsh_lib.c */
BOOL OpenLibraries ( struct AppInfo *ai , struct TagItem *ObjAttrs );
VOID CloseLibraries ( struct AppInfo *ai , struct TagItem *ObjAttrs );

/* apsh_tool.c */
struct MsgHandler *setup_toolA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_tool ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL handle_tool ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_tool ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_tool ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );

/* apsh_sipc.c */
struct MsgHandler *setup_sipcA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL handle_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *ptl );
BOOL close_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL send_sipc_command ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL SafePutToPort ( struct Message *message , STRPTR name );
BOOL SendSIPCMessage ( STRPTR name , ULONG type , VOID *data );
BOOL SendSIPCMessageP ( struct MsgPort *port , struct MsgPort *reply , ULONG type , VOID *data );
APTR OpenSIPC ( struct AppInfo *ai , STRPTR name , struct TagItem *tl );
APTR SIPCPrintf ( APTR ash , APTR data , ...);
APTR CloseSIPC ( APTR ash , struct TagItem *tl );

/* appshellc.c */
struct ExtLibrary *CInit ( struct ExtLibrary *lib );
struct ExtLibrary *LibOpen ( struct ExtLibrary *lib , LONG version );
int LibClose ( struct ExtLibrary *lib );
LONG LibExpunge ( struct ExtLibrary *lib );

/* snapshot.c */
struct NewWindow *LoadSnapShot ( struct NewWindow *nw , BPTR prefs_drawer , STRPTR name );
BOOL SaveSnapShot ( struct Window *win , BPTR prefs_drawer , STRPTR name );

/* apsh_wb.c */
struct MsgHandler *setup_wbA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_wb ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID StepThruList ( struct AppInfo *ai , struct WBNode *wbn , struct AppMessage *msg );
BOOL handle_wb ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_wb ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_wb ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );

/* apsh_winnode.c */
VOID doObjList ( struct ObjectNode *on , ULONG cnt , struct HandleObj *ho );
struct ObjectInfo *NewObjList ( ULONG tags , ...);
struct TagItem *MakeNewTagList ( ULONG data , ...);
struct WinNode *GetWinNode ( struct AppInfo *ai , struct TagItem *tl );
VOID SetCurrentWindowA ( struct AppInfo *ai , struct WinNode *wn , struct TagItem *attrs );
VOID SetupMenuDisable (struct AppInfo * ai,struct WinNode * wn,STRPTR cmd,struct TempMenu *tm);
VOID SetupDisable ( struct AppInfo *ai , struct WinNode *wn );
VOID SetCurrentWindow ( struct AppInfo *ai , struct WinNode *wn , ULONG data , ...);
struct TempMenu *CreateMenuItem ( struct List *list , STRPTR tname , UBYTE type , STRPTR label , STRPTR cmdkey , STRPTR cmd );
VOID DeleteMenuItem ( struct TempMenu *tm );
VOID FreeTextTableMenu ( struct AppInfo *ai , STRPTR cmd , struct TagItem *attrs );
VOID AllocTextTableMenuA ( struct AppInfo *ai , STRPTR cmd , struct TagItem *attrs );
VOID AllocTextTableMenu ( struct AppInfo *ai , STRPTR cmd , ULONG data , ...);
struct WinNode *AllocWinNode ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *attrs );
BOOL make_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL open_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL FreeWinNode ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *attrs );
BOOL delete_idcmp ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );

/* apsh_stdidcmp.c */
VOID ActivateFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID DeActivateFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID HotKeyFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID movewin ( struct AppInfo *ai , STRPTR args , struct TagItem *tl , BOOL dir );
VOID ToBackFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID ToFrontFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID lock_window ( struct AppInfo *ai , struct WinNode *wn );
VOID windowfunc ( struct AppInfo *ai , STRPTR name , struct WinNode *wn , struct Funcs *f );
VOID WindowFunc ( struct AppInfo *ai , STRPTR cmd , struct TagItem *tl );

/* apsh_deftext.c */

/* apsh_arexx.c */
struct MsgHandler *setup_arexxA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_arexx ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL handle_arexx ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_arexx ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_arexx ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL send_rexx_command ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID execute_command ( struct RexxMsg *rmsg , struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID reply_rexx_command ( struct RexxMsg *rmsg , struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID RXFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );
VOID RXSFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );

/* apsh_dos.c */
struct MsgHandler *setup_dosA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_dos ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL handle_dos ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_dos ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_dos ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID DisplayPrompt ( struct MsgHandler *mh );
VOID send_read_packet ( struct StandardPacket *dos_message , BPTR console_fh , struct MsgPort *dos_reply_port , UBYTE *buff );
struct Window *GetConsoleWindow ( BPTR cons );
ULONG ParseCmdLine ( STRPTR line , STRPTR *argv );
VOID InitWinPos ( struct AppInfo *ai , struct DOSInfo *md );
STRPTR PrepareWindowSpec ( struct AppInfo *ai , struct DOSInfo *md );
VOID CMDShellFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );

/* apsh_dispatch.c */
struct AppInfo *ProcessMsgs ( struct AppInfo *ai );
BOOL PerfFunc ( struct AppInfo *ai , ULONG fid , STRPTR cmd , struct TagItem *tl );

/* apsh_clonesipc.c */
struct MsgHandler *setup_apsh_sipcA ( struct AppInfo *ai , struct TagItem *tl );
BOOL handle_apsh_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_apsh_sipc ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );

/* apsh_pref.c */
struct MsgHandler *setup_prefA ( struct AppInfo *ai , struct TagItem *tl );
BOOL open_pref ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL handle_pref ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL close_pref ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
BOOL shutdown_pref ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID FreshenUser ( struct AppInfo *ai , struct Prefs *p );
BOOL lockscreen ( struct AppInfo *ai , struct IDCMPInfo *md , STRPTR name );
BOOL OpenEnvironment ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tls );
BOOL CloseEnvironment ( struct AppInfo *ai , struct MsgHandler *mh , struct TagItem *tl );
VOID FreshenScreenMode ( struct AppInfo *ai , struct ScreenModePref *smp );
VOID FreshenPalette ( struct AppInfo *ai , struct PalettePref *pp );
VOID FreshenPointer ( struct AppInfo *ai , struct PointerPref *pp );

/* apsh_version.c */
VOID VersionFunc ( struct AppInfo *ai , STRPTR args , struct TagItem *tl );

/* apsh_object.c */
APTR __asm NewAPSHObject ( register __a0 struct AppInfo *ai , register __d0 LONG kind , register __a1 struct TagItem *tl );
VOID __asm DisposeAPSHObject ( register __a0 struct AppInfo *ai , register __a1 APTR data );
ULONG setAppInfoAttrs ( struct AppInfo *ai , struct TagItem *tl );
ULONG __asm SetAPSHAttr ( register __a0 struct AppInfo *ai , register __a1 APTR data , register __a2 struct TagItem *tl );
ULONG getAppInfoAttrs ( struct AppInfo *ai , ULONG attrID , ULONG *storage );
ULONG __asm GetAPSHAttr ( register __a0 struct AppInfo *ai , register __d0 LONG attr , register __a1 APTR data , register __a2 APTR storage );

/* misc. functions */
int stricmpn(char *, char *, int);
struct Node * __asm FindNameI(register __a0 struct List *, register __a1 STRPTR);
VOID AlphaEnqueue(struct List *, struct Node *);

/* hyper.c functions */
BOOL handle_hyper (struct AppInfo *, struct MsgHandler *, struct TagItem *);
BOOL shutdown_hyper (struct AppInfo *, struct MsgHandler *, struct TagItem *);
struct MsgHandler *setup_hyperA (struct AppInfo *, struct TagItem *);

ULONG __saveds __asm
dispatchRegs (
	register __a2 VOID (*func)(struct AppInfo *, STRPTR, struct TagItem *),
	register __a0 struct AppInfo *ai,
	register __d0 STRPTR,
	register __a1 struct TagItem *);

/* apsh_locale.c */
struct AppLocContext *OpenLocaleEnv( struct TagItem * );
VOID CloseLocaleEnv( struct AppLocContext * );
STRPTR __regargs GT_Loc(struct AppInfo *, ULONG, LONG);

#endif /* CLIB_APPSHELL_IPROTOS_H */
