
/* clipview.c */
int cmd_start ( void );
struct GlobalData *InitializeSystem ( void );
VOID ShutdownSystem ( struct GlobalData *gd );
void ProcessCommand ( struct GlobalData *gd , ULONG id , struct IntuiMessage *imsg );
VOID AboutObject ( struct GlobalData *gd );
void HandleEvents ( struct GlobalData *gd );
VOID SetBlockPointer ( struct GlobalData *gd , struct Window *win );
ULONG frameobject ( struct GlobalData *gd );
ULONG ASM notifyHook ( REG (a0 )struct Hook *h , REG (a2 )VOID *o , REG (a1 )struct ClipHookMsg *msg );
void StartCBNotify ( struct GlobalData *gd );
void EndCBNotify ( struct GlobalData *gd );

/* filerequester.c */
BOOL FileRequest ( struct GlobalData *gd , ULONG mode , STRPTR title , STRPTR posbut , STRPTR buffer );

/* printer.c */
union printerIO *CreatePrtReq ( struct GlobalData *gd );
void DeletePrtReq ( struct GlobalData *gd , union printerIO *pio );
void PrintObject ( struct GlobalData *gd );
void PrintComplete ( struct GlobalData *gd );

/* save.c */
VOID SaveObject ( struct GlobalData *gd , ULONG mode );

/* snapshot.c */
struct IBox *LoadSnapShot ( struct GlobalData *gd , struct IBox *box );
BOOL SaveSnapShot ( struct GlobalData *gd , struct Window *win );

/* strings.c */
STRPTR GetString ( struct GlobalData *gd , LONG id );
VOID PrintF ( struct GlobalData *gd , LONG mode , LONG id , STRPTR arg1 , ...);

/* stubs.c */
ULONG easyrequest ( struct GlobalData *gd , struct EasyStruct *es , ULONG data , ...);
BOOL aslrequesttags ( struct GlobalData *gd , struct FileRequester *fr , Tag tag1 , ...);
ULONG setgadgetattrs ( struct GlobalData *gd , struct Gadget *g , struct Window *w , Tag tag1 , ...);
ULONG setdtattrs ( struct GlobalData *gd , Object *o , struct Window *w , Tag tag1 , ...);
ULONG setattrs ( struct GlobalData *gd , Object *o , Tag tag1 , ...);
ULONG getdtattrs ( struct GlobalData *gd , Object *o , Tag tag1 , ...);
APTR newobject ( struct GlobalData *gd , Class *cl , STRPTR name , Tag tag1 , ...);
struct Screen *openscreentags ( struct GlobalData *gd , Tag tag1 , ...);
struct Window *openwindowtags ( struct GlobalData *gd , Tag tag1 , ...);
APTR newdtobject ( struct GlobalData *gd , STRPTR name , Tag tag1 , ...);
struct Menu *createmenus ( struct GlobalData *gd , struct NewMenu *nm , Tag tag1 , ...);
struct Menu *layoutmenus ( struct GlobalData *gd , struct Menu *menu , Tag tag1 , ...);
struct Menu *layoutmenuitems ( struct GlobalData *gd , struct MenuItem *mi , Tag tag1 , ...);
VOID setwindowpointer ( struct GlobalData *gd , struct Window *win , Tag tag1 , ...);

/* windowclass.c */
struct Menu *localizemenus ( struct GlobalData *gd , struct EdMenu *em );
BOOL SetMenuItemState ( struct GlobalData *gd , struct WindowObj *wo , ULONG command , BOOL state );
void NoObjectLoaded ( struct GlobalData *gd , struct WindowObj *wo );
Class *initWindowClass ( struct GlobalData *gd );
ULONG freeWindowClass ( Class *cl );
ULONG ASM dispatchWindowClass ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
BOOL initWindowObject ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG syncMenuItems ( struct GlobalData *gd , Class *cl , Object *o , struct WindowObj *wo );
ULONG setWindowClassAttrs ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowClassAttr ( struct GlobalData *gd , Class *cl , Object *o , struct opGet *msg );

/* windowmclass.c */
ULONG notifyAttrChanges ( struct GlobalData *gd , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
Class *initWindowMClass ( struct GlobalData *gd );
ULONG freeWindowMClass ( Class *cl );
ULONG ASM dispatchWindowMClass ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG setWindowMClassAttrs ( struct GlobalData *gd , Class *cl , Object *o , struct opSet *msg );
ULONG getWindowMClassAttr ( struct GlobalData *gd , Class *cl , Object *o , struct opGet *msg );
