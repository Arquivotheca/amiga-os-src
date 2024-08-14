
/* classbase.c */
Class *ASM ObtainAnimEngine ( REG (a6 )struct ClassBase *cb );
struct Library *ASM LibInit ( REG (d0 )struct ClassBase *cb , REG (a0 )BPTR seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct ClassBase *cb );
LONG ASM LibClose ( REG (a6 )struct ClassBase *cb );
LONG ASM LibExpunge ( REG (a6 )struct ClassBase *cb );

/* dispatch.c */
Class *initClass ( struct ClassBase *cb );

/* animr.c */
LONG LoadDelta ( struct ClassBase *cb , struct IFFHandle *iff , WORD *ytable , struct BitMap *bm , struct AnimHeader *ah , LONG *deltalong );
WORD *AllocYTable ( struct ClassBase *cb , struct BitMap *bm );
VOID FreeYTable ( struct ClassBase *cb , WORD *ytable );
