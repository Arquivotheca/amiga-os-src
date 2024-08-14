#define	LIBNAME appshell.library
#define	BASE	AppShellBase
#define	VERSION	36
#define	REVISION 179
#define	LIBID	appshell 36.179 (02-Jul-90)

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/* apsh_main.c */
BOOL HandlerFuncA ( struct AppInfo *ai , struct TagItem *tl );
APTR HandlerDataA ( struct AppInfo *ai , struct TagItem *tl );
STRPTR GetText ( struct AppInfo *ai , ULONG base , ULONG tid , STRPTR def );
STRPTR PrepText ( struct AppInfo *ai , ULONG base , ULONG tid , char *, ...);
STRPTR BuildParseLine (STRPTR line, ULONG *argc, STRPTR *argv);
VOID FreeParseLine (STRPTR);
ULONG ParseLine ( STRPTR line , STRPTR *argv );
STRPTR FindType ( STRPTR *array , STRPTR name , STRPTR defvalue );
BOOL MatchValue ( STRPTR type , STRPTR value );
BOOL QStrCmpI ( STRPTR, STRPTR );
VOID RemoveMsgPort (struct MsgPort *);
BOOL NotifyUser ( struct AppInfo *, STRPTR, struct TagItem *);
VOID APSHSetWaitPointer (struct AppInfo *, struct TagItem *);
VOID APSHClearPointer (struct AppInfo *, struct TagItem *);

/* apsh_dispatch.c */
BOOL PerfFunc (struct AppInfo * ai, ULONG fid, STRPTR cmd, struct TagItem * tl);

/* apsh_projmng.c */
struct ProjNode *NewProject (struct AppInfo *, STRPTR, struct TagItem *);
BOOL RemoveProject (struct AppInfo * ai, STRPTR cmd, struct TagItem * tl);
BOOL AddProjects ( struct AppInfo *, LONG, struct WBArg *, struct TagItem *);
struct ProjNode *GetProjNode ( struct AppInfo *ai , LONG cnt , struct TagItem *);
VOID SwapProjNodes ( struct AppInfo *, struct ProjNode *, struct ProjNode *, struct TagItem *);
struct DiskObject *IconFromWBArg ( struct WBArg *arg );
VOID FreeProjects (struct AppInfo * ai, struct TagItem *);
VOID FreeProject (struct ProjNode *);

/* apsh_functable.c */
BOOL AddFuncEntry (struct AppInfo * ai, struct Funcs * sfe);
VOID FreeFuncEntry (struct AppInfo * ai, struct FuncEntry *fe);
BOOL AddFuncEntries (struct AppInfo * ai, struct Funcs * fels);
VOID FreeFuncEntries (struct AppInfo *ai);
struct FuncEntry *GetFuncEntry (struct AppInfo *, STRPTR, ULONG);
ULONG GetFuncID (struct AppInfo * ai, STRPTR anchor);
STRPTR GetFuncName (struct AppInfo * ai, ULONG id);

/* apsh_idcmp.c */
BOOL APSHGetGadgetInfo(struct AppInfo *, STRPTR, STRPTR, ULONG *, ULONG *);
BOOL APSHGetWindowInfo (struct AppInfo * ai, STRPTR wname, ULONG * window);

/* apsh_sipc.c */
BOOL SendSIPCMessage ( STRPTR name , ULONG type , VOID *data );
BOOL SendSIPCMessageP (struct MsgPort *, struct MsgPort *, ULONG, VOID *);
APTR OpenSIPC (struct AppInfo * ai, STRPTR name, struct TagItem * tl);
APTR SIPCPrintf (APTR ash, APTR data,...);
APTR CloseSIPC (APTR ash, struct TagItem * tl);

/* apsh_misc.asm */
int stricmpn (char *, char *, int);
struct Node * __asm FindNameI(register __a0 struct List *, register __a1 STRPTR);
