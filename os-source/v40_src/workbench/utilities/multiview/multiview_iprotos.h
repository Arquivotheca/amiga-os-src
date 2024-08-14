
/* printer.c */
union printerIO *CreatePrtReq ( struct GlobalData *gd );
void DeletePrtReq ( struct GlobalData *gd , union printerIO *pio );
void PrintObject ( struct GlobalData *gd , LONG mode );
void PrintComplete ( struct GlobalData *gd );

/* windowclass.c */
Class *initWindowClass ( struct GlobalData *gd );
ULONG freeWindowClass ( Class *cl );
ULONG ASM dispatchModel ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
BOOL initWindowObject ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG setWindowClassAttrs ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowAttr ( struct GlobalData *gd , Class *cl , Object *o , struct opGet *msg );
ULONG ASM dispatchWindow ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );

/* stubs.c */
Object *AllocBlockPointer ( struct GlobalData *gd );
void FreeBlockPointer ( struct GlobalData *gd , Object *po );
VOID SetBlockPointer ( struct GlobalData *gd , struct Window *win );
ULONG easyrequest ( struct GlobalData *gd , struct EasyStruct *es , ULONG data , ...);
struct FileRequester *allocaslrequest ( struct GlobalData *gd , ULONG kind , Tag tag1 , ...);
BOOL aslrequesttags ( struct GlobalData *gd , struct FileRequester *fr , Tag tag1 , ...);
ULONG setgadgetattrs ( struct GlobalData *gd , struct Gadget *g , struct Window *w , Tag tag1 , ...);
ULONG setdtattrs ( struct GlobalData *gd , Object *o , struct Window *w , Tag tag1 , ...);
ULONG getdtattrs ( struct GlobalData *gd , Object *o , Tag tag1 , ...);
ULONG setattrs ( struct GlobalData *gd , Object *o , Tag tag1 , ...);
APTR newobject ( struct GlobalData *gd , Class *cl , STRPTR name , Tag tag1 , ...);
struct Screen *openscreentags ( struct GlobalData *gd , Tag tag1 , ...);
struct Window *openwindowtags ( struct GlobalData *gd , Tag tag1 , ...);
APTR newdtobject ( struct GlobalData *gd , STRPTR name , Tag tag1 , ...);
struct Menu *createmenus ( struct GlobalData *gd , struct NewMenu *nm , Tag tag1 , ...);
struct Menu *layoutmenus ( struct GlobalData *gd , struct Menu *menu , Tag tag1 , ...);
VOID setwindowpointer ( struct GlobalData *gd , struct Window *win , Tag tag1 , ...);
VOID CloseWindowSafely ( struct GlobalData *gd , struct Window *win );

/* snapshot.c */
BOOL LoadSnapShot ( struct GlobalData *gd );
BOOL SaveSnapShot ( struct GlobalData *gd );

/* init.c */
int cmd_start ( void );
void GetIconArgs ( struct GlobalData *gd , struct WBArg *wa , BOOL tool );
struct Screen *OpenEnvironment ( struct GlobalData *gd );
VOID CloseEnvironment ( struct GlobalData *gd , UWORD which );
VOID AboutObject ( struct GlobalData *gd );

/* windowmclass.c */
ULONG notifyAttrChanges ( struct GlobalData *gd , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
Class *initWindowMClass ( struct GlobalData *gd );
ULONG freeWindowMClass ( Class *cl );
ULONG ASM dispatchWindowMClass ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG setWindowMClassAttrs ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowMClassAttr ( struct GlobalData *gd , Class *cl , Object *o , struct opGet *msg );

/* save.c */
ULONG ASM saveasFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );

/* strings.c */
STRPTR GetString ( struct GlobalData *gd , LONG id );
VOID PrintF ( struct GlobalData *gd , LONG mode , LONG id , STRPTR arg1 , ...);

/* filerequester.c */
BOOL FileRequest ( struct GlobalData *gd , ULONG mode , STRPTR title , STRPTR posbut , STRPTR buffer );

/* multiview.c */
LONG OpenNewData ( struct GlobalData *gd , ULONG stype , ULONG unit );
ULONG frameobject ( struct GlobalData *gd );
void SetDataObjectAttrs ( struct GlobalData *gd );

/* menus.c */
BOOL LayoutPrefsMenus ( struct GlobalData *gd , struct Menu *menus , ULONG tag1 , ...);
struct Menu *CreateLocaleMenus ( struct GlobalData *gd , struct EdMenu *em );
void AddStandardMenu ( struct GlobalData *gd );
void FreeStandardMenu ( struct GlobalData *gd );

/* arexx.c */
AREXXCONTEXT InitARexx ( char *AppName , char *Extension , LONG unique );
short SetARexxStem ( AREXXCONTEXT ac , struct RexxMsg *rmsg , STRPTR name , STRPTR value );
ULONG ARexxSignal ( AREXXCONTEXT ac );
struct RexxMsg *GetARexxMsg ( AREXXCONTEXT ac );
void ReplyARexxMsg ( AREXXCONTEXT ac , struct RexxMsg *rmsg , char *RString , LONG Error );
short SetARexxLastError ( AREXXCONTEXT ac , struct RexxMsg *rmsg , char *ErrorString );
short SendARexxMsg ( AREXXCONTEXT ac , char *RString , short StringFile );
void FreeARexx ( AREXXCONTEXT ac );

/* events.c */
ULONG GetUserMenus ( struct GlobalData *gd , struct MinList *list );
struct Menu *localizemenus ( struct GlobalData *gd , struct EdMenu *em );
struct MenuItem *SetMenuItemState ( struct GlobalData *gd , ULONG command , BOOL state );
void BackdropMenus ( struct GlobalData *gd );
void NoObjectLoaded ( struct GlobalData *gd );
void PrintMenus ( struct GlobalData *gd , BOOL state );
VOID syncMenuItems ( struct GlobalData *gd , Class *cl , Object *o );
ULONG ASM openFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM reloadFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM aboutFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM snapshotFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM markFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM copyFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM printFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM clselectFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM quitFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM minFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM nomFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM maxFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM wfrontFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM wactFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM wbackFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM sfrontFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM sbackFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM sbeepFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM gettiFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM triggerFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM screenFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM pscreenFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM gcurdirFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM gfileinfoFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
ULONG ASM gobjinfoFunc ( REG (a0 )struct Hook *h , REG (a2 )struct Cmd *c , REG (a1 )Msg msg );
BOOL ProcessCommand ( struct GlobalData *gd , ULONG id , struct IntuiMessage *imsg );
VOID Navigate ( struct GlobalData *gd , LONG cmd , BOOL needvisual );
void HandleEvents ( struct GlobalData *gd );

/* cmdprocessor.c */
struct CmdHeader *AllocCmdProcessor ( struct Cmd *cmdArray , APTR data );
void FreeCmdProcessor ( struct CmdHeader *ch );
ULONG DispatchCmd ( struct CmdHeader *ch , ULONG cmdID , STRPTR cmdStr );
