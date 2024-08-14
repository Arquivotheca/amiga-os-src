
/* arexxhostlvo.c */
ULONG ASM ARexxHostLVO ( REG (a0 )struct RexxMsg *rm , REG (a1 )STRPTR *result , REG (a6 )struct AmigaGuideLib *ag );
LONG rShowNode ( struct AmigaGuideLib *ag , struct RexxMsg *rm , struct RetBlock *block );
LONG rLoadXRef ( struct AmigaGuideLib *ag , struct RexxMsg *rm , struct RetBlock *block );
LONG rGetXRef ( struct AmigaGuideLib *ag , struct RexxMsg *rm , struct RetBlock *block );
LONG rStub ( struct AmigaGuideLib *ag , struct RexxMsg *rm , struct RetBlock *block );

/* unlockamigaguidebaselvo.c */
VOID ASM UnlockAmigaGuideBaseLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (d0 )LONG key );

/* clientmgr.c */
struct Client *AllocClient ( struct AmigaGuideLib *ag , struct NewAmigaGuide *nag , struct TagItem *attrs );
VOID FreeClient ( struct AmigaGuideLib *ag , struct Client *cl );

/* snapshot.c */
BOOL LoadSnapShot ( struct AmigaGuideLib *ag , struct Client *cl );
BOOL SaveSnapShot ( struct AmigaGuideLib *ag , struct Client *cl );

/* main.c */
void MainLoop ( struct AmigaGuideLib *ag , struct Client *cl );
BOOL CreateMsgPorts ( struct AmigaGuideLib *ag , struct Client *cl );
struct Window *OpenAGWindow ( struct AmigaGuideLib *ag , struct Client *cl );
VOID CloseAGWindow ( struct AmigaGuideLib *ag , struct Client *cl );
ULONG ProcessStrCommand ( struct AmigaGuideLib *ag , struct Client *cl , STRPTR cmd );
LONG OpenNewData ( struct AmigaGuideLib *ag , struct Client *cl , ULONG stype );
BOOL HandleAmigaGuideMsgs ( struct AmigaGuideLib *ag , struct Client *cl , struct AmigaGuideMsg *dagm );
void HandleEvents ( struct AmigaGuideLib *ag , struct Client *cl );
VOID Navigate ( struct AmigaGuideLib *ag , struct Client *cl , LONG cmd , BOOL needvisual );
void ProcessCommand ( struct AmigaGuideLib *ag , struct Client *cl , ULONG id , struct IntuiMessage *imsg );
VOID AboutObject ( struct AmigaGuideLib *ag , struct Client *cl );

/* arexx.c */
ULONG ARexxSignal ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext );
struct RexxMsg *GetARexxMsg ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext );
void ReplyARexxMsg ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext , struct RexxMsg *rmsg , char *RString , LONG Error );
short SetARexxLastError ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext , struct RexxMsg *rmsg , char *ErrorString );
short SendARexxMsg ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext , char *RString , short StringFile );
void FreeARexx ( struct AmigaGuideLib *ag , AREXXCONTEXT RexxContext );
VOID *InitARexx ( struct AmigaGuideLib *ag , char *AppName , char *Extension , LONG unique );

/* openamigaguidelvo.c */
VOID *ASM OpenAmigaGuideALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct NewAmigaGuide *nag , REG (a1 )struct TagItem *attrs );

/* lockamigaguidebaselvo.c */
LONG ASM LockAmigaGuideBaseLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )VOID *handle );

/* misc.c */
struct Process *createnewproc ( struct AmigaGuideLib *ag , Tag tag1 , ...);
ULONG setgadgetattrs ( struct AmigaGuideLib *ag , struct Gadget *g , struct Window *w , Tag tag1 , ...);
ULONG getdtattrs ( struct AmigaGuideLib *ag , Object *o , Tag tag1 , ...);
ULONG setdtattrs ( struct AmigaGuideLib *ag , Object *o , struct Window *w , Tag tag1 , ...);
APTR newobject ( struct AmigaGuideLib *ag , Class *cl , STRPTR name , Tag tag1 , ...);
struct Window *openwindowtags ( struct AmigaGuideLib *ag , Tag tag1 , ...);
APTR newdtobject ( struct AmigaGuideLib *ag , STRPTR name , Tag tag1 , ...);
VOID setwindowpointer ( struct AmigaGuideLib *ag , struct Window *win , Tag tag1 , ...);
BOOL layoutmenus ( struct AmigaGuideLib *ag , VOID *vi , struct Menu *menus , ULONG tag1 , ...);
ULONG easyrequest ( struct AmigaGuideLib *ag , struct Window *win , struct EasyStruct *es , ULONG data , ...);
BOOL aslrequesttags ( struct AmigaGuideLib *ag , struct FileRequester *fr , Tag tag1 , ...);
ULONG setattrs ( struct AmigaGuideLib *ag , Object *o , Tag tag1 , ...);
VOID CloseWindowSafely ( struct AmigaGuideLib *ag , struct Window *win );
struct MsgPort *CreatePort ( struct AmigaGuideLib *ag , STRPTR name , BYTE pri );
VOID DeletePort ( struct AmigaGuideLib *ag , struct MsgPort *mp );
VOID *AllocPVec ( struct AmigaGuideLib *ag , void *pool , ULONG byteSize );
VOID FreePVec ( struct AmigaGuideLib *ag , void *pool , void *memoryBlock );
VOID SetBlockPointer ( struct AmigaGuideLib *ag , struct Window *win );
VOID PrintF ( struct AmigaGuideLib *ag , struct Client *cl , LONG mode , LONG id , STRPTR arg1 , ...);

/* sendamigaguidecmdlvo.c */
LONG ASM SendAmigaGuideCmdALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl , REG (d0 )STRPTR cmdLine , REG (d1 )struct TagItem *attrs );
LONG SendAmigaGuideCmd ( struct AmigaGuideLib *ag , struct Client *cl , STRPTR cmd , Tag tag1 , ...);

/* closeamigaguidelvo.c */
VOID ASM CloseAmigaGuideLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl );

/* amigaguidesignallvo.c */
ULONG ASM AmigaGuideSignalLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl );

/* getamigaguidemsglvo.c */
struct AmigaGuideMsg *ASM GetAmigaGuideMsgLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl );

/* replyamigaguidemsglvo.c */
VOID ASM ReplyAmigaGuideMsgLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct AmigaGuideMsg *agm );

/* setamigaguidecontextlvo.c */
LONG ASM SetAmigaGuideContextALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl , REG (d0 )ULONG id , REG (d1 )struct TagItem *attrs );

/* setamigaguideattrsalvo.c */
LONG ASM SetAmigaGuideAttrsALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl , REG (a1 )struct TagItem *attrs );

/* sendamigaguidecontextlvo.c */
LONG ASM SendAmigaGuideContextALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Client *cl , REG (d0 )struct TagItem *attrs );

/* getamigaguideattrlvo.c */
LONG ASM GetAmigaGuideAttrLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (d0 )ULONG tag , REG (a0 )VOID *handle , REG (a1 )ULONG *storage );

/* stubs.c */
LONG ASM LoadXRefLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR lock , REG (a1 )STRPTR name );
VOID ASM ExpungeXRefLVO ( REG (a6 )struct AmigaGuideLib *ag );
BPTR ASM AddPathEntriesLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR path , REG (d0 )STRPTR *argptr );
BPTR ASM CopyPathListLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR p );
VOID ASM FreePathListLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR p );
ULONG ASM ParsePathStringLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (d0 )STRPTR line , REG (a0 )STRPTR *argv , REG (d1 )ULONG max );
BPTR ASM LockELVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name , REG (d2 )LONG mode );
BPTR ASM OpenELVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name , REG (d2 )LONG mode );
BPTR ASM SetCurrentDirELVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name );
STRPTR ASM GetAmigaGuideStringLVO ( REG (a6 )struct AmigaGuideLib *ag , REG (d0 )LONG id );
ULONG ASM AddAmigaGuideHostALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct Hook *h , REG (d0 )STRPTR name , REG (a1 )struct TagItem *attrs );
LONG ASM RemoveAmigaGuideHostALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )VOID *handle , REG (a1 )struct TagItem *attrs );

/* openamigaguideasynclvo.c */
VOID *ASM OpenAmigaGuideAsyncALVO ( REG (a6 )struct AmigaGuideLib *ag , REG (a0 )struct NewAmigaGuide *nag , REG (d0 )struct TagItem *attrs );
VOID StartDaemonFunc ( void );

/* windowclass.c */
BOOL SetMenuItemState ( struct AmigaGuideLib *ag , struct WindowObj *wo , ULONG command , BOOL state );
void NoObjectLoaded ( struct AmigaGuideLib *ag , struct WindowObj *wo );
struct Menu *localizemenus ( struct AmigaGuideLib *ag , struct WindowObj *wo , struct EdMenu *em );
Class *initWindowClass ( struct AmigaGuideLib *ag );
ULONG freeWindowClass ( struct AmigaGuideLib *ag , Class *cl );
ULONG ASM dispatchWindowClass ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
BOOL initWindowObject ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG syncMenuItems ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct WindowObj *wo );
ULONG setWindowClassAttrs ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowClassAttr ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct opGet *msg );

/* addamigaguidehostlvo.c */

/* removeamigaguidehostlvo.c */

/* token.c */
struct AmigaGuideToken *ObtainXRefToken ( struct AmigaGuideLib *ag );

/* windowmclass.c */
ULONG notifyAttrChanges ( struct AmigaGuideLib *ag , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
Class *initWindowMClass ( struct AmigaGuideLib *ag );
ULONG freeWindowMClass ( Class *cl );
ULONG ASM dispatchWindowMClass ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG setWindowMClassAttrs ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowMClassAttr ( struct AmigaGuideLib *ag , Class *cl , Object *o , struct opGet *msg );

/* printer.c */
union printerIO *CreatePrtReq ( struct AmigaGuideLib *ag );
void DeletePrtReq ( struct AmigaGuideLib *ag , union printerIO *pio );
void PrintObject ( struct AmigaGuideLib *ag , struct Client *cl , LONG mode );
void PrintComplete ( struct AmigaGuideLib *ag , struct Client *cl );

/* filerequester.c */
BOOL FileRequest ( struct AmigaGuideLib *ag , struct Client *cl , ULONG mode , STRPTR title , STRPTR posbut , STRPTR buffer );

/* save.c */
VOID SaveObject ( struct AmigaGuideLib *ag , struct Client *cl , ULONG mode );

/* amigaguidebase.c */
struct Library *ASM LibInit ( REG (d0 )struct AmigaGuideLib *ag , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpenLVO ( REG (a6 )struct AmigaGuideLib *ag );
LONG ASM LibCloseLVO ( REG (a6 )struct AmigaGuideLib *ag );
LONG ASM LibExpungeLVO ( REG (a6 )struct AmigaGuideLib *ag );
