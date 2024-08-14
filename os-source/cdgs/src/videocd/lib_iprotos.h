
/* libbase.c */
struct Library *ASM LibInit ( REG (d0 )struct LibBase *lb , REG (a0 )ULONG *seglist , REG (a6 )struct Library *sysbase );
LONG ASM LibOpen ( REG (a6 )struct LibBase *lb );
LONG ASM LibClose ( REG (a6 )struct LibBase *lb );
LONG ASM LibExpunge ( REG (a6 )struct LibBase *lb );

/* videocd.c */
LONG ASM GetCDTypeA ( REG (a0 )STRPTR path , REG (a1 )struct TagItem *attrs , REG (a6 )struct LibBase *lb );
APTR ASM ObtainCDHandleA ( REG (a0 )STRPTR pathName , REG (a1 )struct TagItem *attrs , REG (a6 )struct LibBase *lb );
struct TagItem *ASM GetVideoCDInfoA ( REG (a0 )struct VideoCDHandle *vh , REG (d0 )ULONG seqNum , REG (a1 )struct TagItem *attrs , REG (a6 )struct LibBase *lb );
void ASM FreeVideoCDInfo ( REG (a0 )struct TagItem *retTags , REG (a6 )struct LibBase *lb );
VOID ASM ReleaseCDHandle ( REG (a0 )struct VideoCDHandle *vh , REG (a6 )struct LibBase *lb );
