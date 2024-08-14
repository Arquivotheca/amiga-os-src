
/* classbase.c */
Class *ASM ObtainILBMEngine ( REG (a6 )struct ClassBase *cb );
struct Library *ASM LibInit ( REG (d0 )struct ClassBase *cb , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct ClassBase *cb );
LONG ASM LibClose ( REG (a6 )struct ClassBase *cb );
LONG ASM LibExpunge ( REG (a6 )struct ClassBase *cb );

/* dispatch.c */
ULONG setdtattrs ( struct ClassBase *cb , Object *o , ULONG data , ...);
ULONG getdtattrs ( struct ClassBase *cb , Object *o , ULONG data , ...);
Class *initClass ( struct ClassBase *cb );
ULONG ASM Dispatch ( REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )Msg msg );

/* getilbm.c */
BOOL ASM GetILBM ( REG (a6 )struct ClassBase *cb , REG (a0 )Class *cl , REG (a2 )Object *o , REG (a1 )struct TagItem *attrs );

/* getbody.c */
LONG GetBody (struct ClassBase *cb ,
              struct IFFHandle *iff ,
              struct BitMapHeader *bmhd ,
              struct BitMap *bitmap );
