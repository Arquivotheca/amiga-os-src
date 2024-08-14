
/* textbase.c */
Class *ASM ObtainTextEngine ( REG (a6 )struct TextLib *txl );
struct Library *ASM LibInit ( REG (d0 )struct TextLib *txl , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct TextLib *txl );
LONG ASM LibClose ( REG (a6 )struct TextLib *txl );
LONG ASM LibExpunge ( REG (a6 )struct TextLib *txl );

/* dispatch.c */
Class *initClass ( struct TextLib *txl );
ULONG getdtattrs ( struct TextLib *txl , Object *o , ULONG data , ...);
ULONG setdtattrs ( struct TextLib *txl , Object *o , ULONG data , ...);
ULONG notifyAttrChanges ( struct TextLib *txl , Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
void PrepareFont ( struct TextLib *txl , Object *o , struct localData *lod , struct TextAttr *tattr );
ULONG ASM Dispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );
ULONG getTextDTAttr ( struct TextLib *txl , Class *cl , Object *o , struct opGet *msg );
ULONG setTextDTAttrs ( struct TextLib *txl , Class *cl , Object *o , struct opSet *msg );
VOID RenderLine ( struct TextLib *txl , struct localData *lod , UWORD x , UWORD y , UWORD w , STRPTR text , ULONG len );
ULONG renderMethod ( struct TextLib *txl , Class *cl , Object *o , struct gpRender *msg );
ULONG printMethod ( struct TextLib *txl , Class *cl , Object *o , struct dtPrint *msg );
BOOL writeObject ( struct TextLib *txl , APTR handle , Object *o , struct localData *lod , struct GadgetInfo *gi , LONG mode );
ULONG copyMethod ( struct TextLib *txl , Class *cl , Object *o , struct dtGeneral *msg );
ULONG writeMethod ( struct TextLib *txl , Class *cl , Object *o , struct dtWrite *msg );

/* drawbox.c */
VOID DrawBox ( struct TextLib *txl , Class *cl , Object *o , struct RastPort *rp , struct IBox *box , struct IBox *sel , LONG status );

/* input.c */
ULONG goinactive ( struct TextLib *txl , Class *cl , Object *o , struct gpInput *msg );
ULONG handleInput ( struct TextLib *txl , Class *cl , Object *o , struct gpInput *msg );
VOID scrollMethod ( struct TextLib *txl , Class *cl , Object *o , struct gpInput *msg , struct RastPort *rp , struct IBox *box );

/* findline.c */
struct Line *FindLine ( struct TextLib *txl , Class *cl , Object *o , struct gpInput *msg , struct RastPort *rp , WORD mode , struct IBox *box , LONG status );
