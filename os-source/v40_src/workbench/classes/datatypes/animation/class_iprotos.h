
/* classbase.c */
Class *ASM ObtainAnimationEngine ( REG (a6 )struct ClassBase *cb );
Class *initClass ( struct ClassBase *cb );
struct Library *ASM LibInit ( REG (d0 )struct ClassBase *cb , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct ClassBase *cb );
LONG ASM LibClose ( REG (a6 )struct ClassBase *cb );
LONG ASM LibExpunge ( REG (a6 )struct ClassBase *cb );
struct Gadget *newobject ( struct ClassBase *cb , APTR name , Tag tag1 , ...);
ULONG setattrs ( struct ClassBase *cb , struct Gadget *g , Tag tag1 , ...);
ULONG setgadgetattrs ( struct ClassBase *cb , struct Gadget *g , struct Window *w , Tag tag1 , ...);
Object *newdtobject ( struct ClassBase *cb , APTR name , Tag tag1 , ...);
ULONG setdtattrs ( struct ClassBase *cb , Object *o , ULONG data , ...);
ULONG notifyAttrChanges ( struct ClassBase *cb , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
struct Process *createnewproc ( struct ClassBase *cb , Tag tag1 , ...);
struct Player *createplayer ( struct ClassBase *cb , Tag tag , ...);
ULONG bestmodeid ( struct ClassBase *cb , ULONG data , ...);
struct Region *installclipregion ( struct ClassBase *cb , struct Window *w , struct Layer *l , struct Region *r );

/* dispatch.c */
void SetTapeDeckAttrs ( struct ClassBase *cb , struct localData *lod , ULONG tags , ...);
void freeExtraInfo ( struct ClassBase *cb , struct localData *lod );
ULONG handleMethod ( struct ClassBase *cb , Class *cl , Object *o , struct gpInput *gpi );
ULONG setAttrsMethod ( struct ClassBase *cb , Class *cl , Object *o , struct opSet *msg , BOOL init );
ULONG updateMethod ( struct ClassBase *cb , Class *cl , Object *o , struct opSet *msg );
ULONG getAttrMethod ( struct ClassBase *cb , Class *cl , Object *o , struct opGet *msg );
ULONG frameMethod ( struct ClassBase *cb , Class *cl , Object *o , struct dtFrameBox *msg );
ULONG layoutMethod ( struct ClassBase *cb , Class *cl , Object *o , struct gpLayout *msg );
ULONG renderMethod ( struct ClassBase *cb , Class *cl , Object *o , struct gpRender *msg );
ULONG newMethod ( struct ClassBase *cb , Class *cl , Object *o , struct opSet *ops );
ULONG startMethod ( struct ClassBase *cb , Class *cl , Object *o , struct adtStart *asa );
ULONG pauseMethod ( struct ClassBase *cb , Class *cl , Object *o , Msg msg );
ULONG stopMethod ( struct ClassBase *cb , Class *cl , Object *o , Msg msg );
struct ColorMap *GetObjectColorMap ( struct ClassBase *cb , Class *cl , Object *o );
ULONG copyMethod ( struct ClassBase *cb , Class *cl , Object *o , struct dtGeneral *msg );
ULONG writeMethod ( struct ClassBase *cb , Class *cl , Object *o , struct dtWrite *msg );
ULONG printMethod ( struct ClassBase *cb , Class *cl , Object *o , struct dtPrint *msg );
ULONG ASM Dispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );

/* loadproc.c */
void LoadHandler ( void );

/* dispproc.c */
struct FrameNode *DiscardAllBut ( struct ClassBase *cb , struct localData *lod , ULONG frame , ULONG mode );
void DiscardAllFrames ( struct ClassBase *cb , struct localData *lod );
void FreeFrameLists ( struct ClassBase *cb , Object *o , struct localData *lod );
ULONG notify ( struct ClassBase *cb , Object *o , struct localData *lod , ULONG tag1 , ...);
void DisplayFrame ( struct ClassBase *cb , Object *o , struct localData *lod , struct FrameNode *fn );
void LocateFrame ( struct ClassBase *cb , Object *o , struct localData *lod , ULONG which );
ULONG ASM playerDispatch ( REG (a0 )struct Hook *h , REG (a2 )struct Player *pi , REG (a1 )Msg msg );
void DispHandler ( void );

/* putilbm.c */
LONG packrow ( BYTE **pSource , BYTE **pDest , LONG rowSize );
BOOL PutBody ( struct ClassBase *cb , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct BitMap *bm );
BOOL PutColors ( struct ClassBase *cb , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct ColorRegister *cmap , ULONG modeid );
BOOL WriteBMHD ( struct ClassBase *cb , struct IFFHandle *iff , struct BitMapHeader *bmhd );
BOOL WriteCAMG ( struct ClassBase *cb , struct IFFHandle *iff , ULONG modeid );
BOOL WriteNAME ( struct ClassBase *cb , struct IFFHandle *iff , STRPTR name );
BOOL writeObject ( struct ClassBase *cb , struct IFFHandle *iff , Object *o , struct localData *lod );
