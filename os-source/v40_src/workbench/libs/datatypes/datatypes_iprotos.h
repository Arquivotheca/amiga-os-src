
/* misc.c */
void strcpy ( char *dst , char *src );
int strlen ( char *src );
ULONG notifyAttrChanges ( struct DataTypesLib *dtl , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
ULONG dogadgetmethod ( struct DataTypesLib *dtl , struct Gadget *g , struct Window *w , struct Requester *r , ULONG data , ...);
ULONG notifyGAttrChanges ( struct DataTypesLib *dtl , Object *o , struct Window *w , struct Requester *r , ULONG flags , ULONG tag1 , ...);
Object *newobject ( struct DataTypesLib *dtl , Class *cl , ULONG data , ...);
ULONG setattrs ( struct DataTypesLib *dtl , Object *o , Tag tag1 , ...);

/* dispatch.c */
LONG __stdargs RLDispatch ( struct RexxMsg *rmsg , STRPTR *result );
LONG rExamineDT ( struct RexxMsg *rmsg , struct RetBlock *block , struct DataTypesLib *dtl );

/* strings.c */
STRPTR ASM GetDTStringLVO ( REG (a6 )struct DataTypesLib *dtl , REG (d0 )ULONG id );

/* examinedt.c */
struct DataType *ASM ExamineDT ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )BPTR lock , REG (a1 )struct FileInfoBlock *fib );
struct DataType *ASM ObtainDataTypeA ( REG (a6 )struct DataTypesLib *dtl , REG (d0 )ULONG type , REG (a0 )APTR handle , REG (a1 )struct TagItem *attrs );
void ASM ReleaseDataType ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )struct DataType *dtn );

/* class.c */
Object *ASM NewDTObjectA ( REG (a6 )struct DataTypesLib *dtl , REG (d0 )APTR name , REG (a0 )struct TagItem *attrs );
VOID ASM DisposeDTObject ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o );
ULONG ASM SetDTAttrsA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct Window *win , REG (a2 )struct Requester *req , REG (a3 )struct TagItem *attrs );
ULONG ASM GetDTAttrsA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a2 )struct TagItem *attrs );
LONG ASM AddDTObject ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )struct Window *win , REG (a1 )struct Requester *req , REG (a2 )Object *o , REG (d0 )LONG pos );
VOID ASM RefreshDTObjectA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct Window *win , REG (a2 )struct Requester *req , REG (a3 )struct TagItem *attrs );
LONG ASM RemoveDTObject ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )struct Window *win , REG (a1 )Object *o );
struct DTMethods *ASM GetDTTriggerMethods ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o );
ULONG *ASM GetDTMethods ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o );
ULONG ASM DoDTMethodA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct Window *win , REG (a2 )struct Requester *req , REG (a3 )Msg msg );
ULONG ASM DoAsyncLayout ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct gpLayout *gpl );
ULONG ASM PrintDTObjectA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct Window *w , REG (a2 )struct Requester *r , REG (a3 )struct dtPrint *msg );
ULONG ASM ObtainDTDrawInfoA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )struct TagItem *attrs );
ULONG ASM DrawDTObjectA ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )struct RastPort *rp , REG (a1 )Object *o , REG (d0 )LONG x , REG (d1 )LONG y , REG (d2 )LONG w , REG (d3 )LONG h , REG (d4 )LONG th , REG (d5 )LONG tv , REG (a2 )struct TagItem *attrs );
ULONG ASM ReleaseDTDrawInfo ( REG (a6 )struct DataTypesLib *dtl , REG (a0 )Object *o , REG (a1 )APTR handle );

/* datatypesbase.c */
struct Library *ASM LibInit ( REG (d0 )struct DataTypesLib *dtl , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct DataTypesLib *dtl );
BOOL ASM CloseClasses ( REG (a6 )struct DataTypesLib *dtl );
LONG ASM LibClose ( REG (a6 )struct DataTypesLib *dtl );
LONG ASM LibExpunge ( REG (a6 )struct DataTypesLib *dtl );

/* datatypesclass.c */
void CloseClipIFF ( struct DataTypesLib *dtl , struct IFFHandle *iff );
Class *ASM initDTClass ( REG (a6 )struct DataTypesLib *dtl );
