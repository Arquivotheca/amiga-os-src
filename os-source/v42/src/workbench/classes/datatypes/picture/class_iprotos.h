
/* classbase.c */
struct Library *ASM LibInit ( REG (d0 )struct ClassBase *cb , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct ClassBase *cb );
LONG ASM LibClose ( REG (a6 )struct ClassBase *cb );
LONG ASM LibExpunge ( REG (a6 )struct ClassBase *cb );

/* dispatch.c */
void CopyBitMap ( struct ClassBase *cb , struct BitMap *bm1 , struct BitMap *bm2 );
Class *initClass ( struct ClassBase *cb );

/* putilbm.c */
BOOL writeObject ( struct ClassBase *cb , struct IFFHandle *iff , Object *o , struct localData *lod );
