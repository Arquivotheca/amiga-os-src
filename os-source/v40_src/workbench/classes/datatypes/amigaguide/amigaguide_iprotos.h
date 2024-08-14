
/* input.c */
ULONG propagateHit ( struct AGLib *ag , Class *cl , Object *o , struct gpHitTest *msg );
ULONG goinactive ( struct AGLib *ag , Class *cl , Object *o , struct gpInput *msg );
ULONG handleInput ( struct AGLib *ag , Class *cl , Object *o , struct gpInput *msg );

/* print.c */
ULONG dumpNode ( struct AGLib *ag , ULONG mode , APTR oh , Object *o , struct ClientData *cd );
BOOL writeObject ( struct AGLib *ag , APTR handle , Object *o , struct ClientData *cd , struct GadgetInfo *gi , LONG mode );
ULONG writeMethod ( struct AGLib *ag , Class *cl , Object *o , struct dtWrite *msg );
ULONG copyMethod ( struct AGLib *ag , Class *cl , Object *o , struct dtGeneral *msg );

/* lvogetagstring.c */
STRPTR ASM LVOGetAGString ( REG (a6 )struct AGLib *ag , REG (d0 )LONG id );

/* render.c */
VOID prepareRastPorts ( struct AGLib *ag , Class *cl , Object *o );
LONG r_renderLink ( struct AGLib *ag , struct ClientData *cd , struct Line *line , LONG x , LONG y , UWORD rmarg );
void r_makeNewActive ( struct AGLib *ag , Class *cl , Object *o , struct ClientData *cd , struct Line *oln , struct Line *nln , BOOL scroll );
struct Line *r_activateLink ( struct AGLib *ag , Class *cl , Object *o , struct ClientData *cd , LONG dir );
ULONG renderMethod ( struct AGLib *ag , Class *cl , Object *o , struct gpRender *msg );

/* nddispatch.c */
Class *initHNClass ( struct AGLib *ag );
ULONG ASM HNDispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
BOOL initializeHN ( struct AGLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG setHNAttrs ( struct AGLib *ag , Class *cl , Object *o , struct opSet *msg );

/* misc.c */
APTR newdtobject ( struct AGLib *ag , STRPTR name , Tag tag1 , ...);
struct NamedObject *ano ( struct AGLib *ag , STRPTR name , Tag tag1 , ...);
ULONG dogadgetmethod ( struct AGLib *ag , struct Gadget *g , struct Window *w , struct Requester *r , ULONG data , ...);
ULONG notifyAttrChangesDM ( struct AGLib *ag , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
ULONG notifyAttrChanges ( struct AGLib *ag , Object *o , struct Window *w , struct Requester *r , ULONG flags , ULONG tag1 , ...);
struct Process *createnewproc ( struct AGLib *ag , Tag tag1 , ...);
ULONG setattrs ( struct AGLib *ag , Object *o , Tag tag1 , ...);
ULONG getdtattrs ( struct AGLib *ag , Object *o , Tag tag1 , ...);
Object *newobject ( struct AGLib *ag , Class *cl , UBYTE *name , Tag tag1 , ...);
ULONG sam ( struct AGLib *ag , struct ClientData *cd , Msg msg , ULONG size );
void drawbevel ( struct AGLib *ag , Object *o , struct RastPort *rp , struct DrawInfo *dri , WORD x , WORD y , WORD w , WORD h , BOOL raised );
WORD GetLabelsExtent ( struct AGLib *ag , struct RastPort *rp , LONG min , LONG max , struct IBox *box );
VOID *allocpvec ( struct AGLib *ag , void *pool , ULONG byteSize , ULONG attributes );
VOID freepvec ( struct AGLib *ag , void *pool , void *memoryBlock );
void Strncpy ( char *dst , char *src , int num );
void StrToUpper ( struct AGLib *ag , char *dst );
WORD m_binSearch ( STRPTR token , UWORD tokenLen , const struct Keyword *keywords , WORD numKeywords );
LONG m_sendCmd ( struct AGLib *ag , STRPTR cmd );

/* mdispatch.c */
ULONG notifyChanges ( Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
ULONG setgattrs ( Object *o , VOID *ginfo , ULONG tag1 , ...);
Class *initModelClass ( struct AGLib *ag );
ULONG ASM ModelDispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG getModelAttr ( struct AGLib *ag , Class *cl , Object *o , struct opGet *msg );
ULONG setModelAttrs ( struct AGLib *ag , Class *cl , Object *o , struct opSet *msg );

/* parser.c */
ULONG ParseString ( struct AGLib *ag , STRPTR line , STRPTR *argv , ULONG max );

/* lvoopene.c */
BPTR ASM LVOOpenE ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name , REG (d2 )LONG mode );

/* xrefmgr.c */
LONG loadxref ( struct AGLib *ag , BPTR lock , STRPTR fileName , LONG mode );
char GetCh ( struct AGLib *ag , struct XRefWorkData *gd );
BOOL GetToken ( struct AGLib *ag , struct XRefWorkData *gd );
VOID LoadXRefFiles ( struct AGLib *ag );
struct XRefFile *AllocXRefFile ( struct AGLib *ag , BPTR lock , STRPTR name );
VOID FreeXRefFile ( struct AGLib *ag , struct XRefFile *xrf );
VOID UnLoadXRefFiles ( struct AGLib *ag );
struct XRef *AddXRef ( struct AGLib *ag , STRPTR name , STRPTR file , LONG line , UBYTE type );

/* lvocopypathlist.c */
BPTR ASM LVOCopyPathList ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR p );

/* lvolocke.c */
BPTR ASM LVOLockE ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name , REG (d2 )LONG mode );

/* lvoaddpathentries.c */
BPTR ASM LVOAddPathEntries ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR path , REG (d0 )STRPTR *argptr );

/* lvoparsepathstring.c */
ULONG ASM LVOParsePathString ( REG (a6 )struct AGLib *ag , REG (d0 )STRPTR line , REG (a0 )STRPTR *argv , REG (d1 )ULONG max );

/* lvofreepathlist.c */
VOID ASM LVOFreePathList ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR p );

/* lvoloadxref.c */
LONG ASM LVOLoadXRef ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR lock , REG (a1 )STRPTR name );

/* lvoexpungexref.c */
VOID ASM LVOExpungeXRef ( REG (a6 )struct AGLib *ag );

/* lvosetcurrentdire.c */
BPTR IsFileThere ( struct AGLib *ag , STRPTR path );
BPTR ASM LVOSetCurrentDirE ( REG (a6 )struct AGLib *ag , REG (a0 )BPTR p , REG (d1 )STRPTR name );

/* token.c */
struct AmigaGuideToken *ObtainXRefToken ( struct AGLib *ag );

/* amigaguidebase.c */
Class *ASM ObtainAGEngine ( REG (a6 )struct AGLib *ag );
struct Library *ASM LibInit ( REG (d0 )struct AGLib *ag , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct AGLib *ag );
BOOL closestuff ( struct AGLib *ag );
LONG ASM LibClose ( REG (a6 )struct AGLib *ag );
LONG ASM LibExpunge ( REG (a6 )struct AGLib *ag );

/* findnode.c */
Object *ObtainNode ( struct AGLib *ag , Class *cl , Object *o , STRPTR name , LONG line , LONG column );
VOID ReleaseNode ( struct AGLib *ag , Class *cl , Object *o , Object *node );

/* process.c */
void ObjectHandler ( void );

/* lvonodehost.c */
ULONG ASM LVOAddAGHostA ( REG (a6 )struct AGLib *ag , REG (a0 )struct Hook *h , REG (d0 )STRPTR name , REG (a1 )struct TagItem *attrs );
LONG ASM LVORemoveAGHostA ( REG (a6 )struct AGLib *ag , REG (a0 )VOID *handle , REG (a1 )struct TagItem *attrs );

/* arexx.c */
struct MsgPort *CreatePort ( struct AGLib *ag , STRPTR name , BYTE pri );
VOID DeletePort ( struct AGLib *ag , struct MsgPort *mp );
ULONG ARexxSignal ( struct AGLib *ag , struct ARexxContext *rc );
struct RexxMsg *GetARexxMsg ( struct AGLib *ag , struct ARexxContext *rc );
void ReplyARexxMsg ( struct AGLib *ag , struct ARexxContext *rc , struct RexxMsg *rmsg , char *RString , LONG Error );
short SetARexxLastError ( struct AGLib *ag , struct ARexxContext *rc , struct RexxMsg *rmsg , char *ErrorString );
short SendARexxMsg ( struct AGLib *ag , struct ARexxContext *rc , char *RString , short StringFile );
void FreeARexx ( struct AGLib *ag , struct ARexxContext *rc );
VOID *InitARexx ( struct AGLib *ag , char *AppName , char *Extension , LONG unique );

/* drawbox.c */
VOID DrawBox ( struct AGLib *ag , Class *cl , Object *o , struct RastPort *rp , struct IBox *box , struct IBox *sel , LONG status );

/* findline.c */
struct Line *FindLine ( struct AGLib *ag , Class *cl , Object *o , struct gpInput *msg , struct RastPort *rp , WORD mode , struct IBox *box , LONG status );

/* dispatch.c */
BOOL initializeAG ( struct AGLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG getAGAttr ( struct AGLib *ag , Class *cl , Object *o , struct opGet *msg );
ULONG setAGAttrs ( struct AGLib *ag , Class *cl , Object *o , struct opSet *msg );
ULONG frameMethod ( struct AGLib *ag , Class *cl , Object *o , struct dtFrameBox *msg );
ULONG ASM Dispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
Class *initClass ( struct AGLib *ag );

/* layout.c */
ULONG layoutMethod ( struct AGLib *ag , Class *cl , Object *o , ULONG initial );

/* dbdispatch.c */
Class *initDBClass ( struct AGLib *ag );

/* asyncio.c */
VOID RecordSyncFailure ( struct AGLib *ag , struct AsyncFile *file );
struct AsyncFile *OpenAsync ( struct AGLib *ag , const STRPTR fileName , UBYTE accessMode , LONG bufferSize );
LONG CloseAsync ( struct AGLib *ag , struct AsyncFile *file );
LONG ReadAsync ( struct AGLib *ag , struct AsyncFile *file , APTR buffer , LONG numBytes );
LONG ReadCharAsync ( struct AGLib *ag , struct AsyncFile *file );
LONG WriteAsync ( struct AGLib *ag , struct AsyncFile *file , APTR buffer , LONG numBytes );
LONG WriteCharAsync ( struct AGLib *ag , struct AsyncFile *file , UBYTE ch );
LONG SeekAsync ( struct AGLib *ag , struct AsyncFile *file , LONG position , BYTE mode );

/* rlayout.c */
void rl_layoutRecurse ( struct AGLib *ag , struct ClientData *cd , struct DatabaseData *dbd , struct NodeData *nd , struct WorkLayout *wl , STRPTR buffer , LONG size );
