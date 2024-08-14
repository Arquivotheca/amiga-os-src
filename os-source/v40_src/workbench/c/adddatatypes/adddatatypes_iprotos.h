
/* adddatatypes.c */
LONG cmd_start ( void );
BOOL GetDateStamp ( struct GlobalData *gd );
ULONG ScanDir ( struct GlobalData *gd , STRPTR path , LONG *opts );
VOID PrintF ( struct GlobalData *gd , AppStringsID str , STRPTR arg1 , ...);
struct Token *ObtainDataTypesToken ( struct GlobalData *gd );

/* texttable.c */
STRPTR GetString ( struct LocaleInfo *li , AppStringsID id );

/* misc.c */
struct DataType *InitList ( struct GlobalData *gd , STRPTR name , struct List *mlist , struct List *list , UWORD type , ULONG gid , ULONG id );
WORD MaskCmp ( struct DataType *dtn1 , struct DataType *dtn2 );
VOID EnqueueAlphaMask ( struct GlobalData *gd , struct List *list , struct DataType *dtn );
VOID EnqueueAlpha ( struct GlobalData *gd , struct List *list , struct Node *new );

/* readdatatype.c */
struct DataType *ASM ReadDataType ( REG (a6 )struct GlobalData *gd , REG (d1 )STRPTR name );
struct DataType *ASM GetDataType ( REG (a6 )struct GlobalData *gd , REG (a0 )struct IFFHandle *iff );
struct DataType *GetDTYPForm ( struct GlobalData *gd , struct IFFHandle *iff );
struct DataType *GetDTHD ( struct GlobalData *gd , struct IFFHandle *iff );
LONG ASM ReadFunc ( REG (d1 )struct MyFileHandle *fh , REG (d2 )STRPTR buff , REG (d3 )ULONG len , REG (a6 )struct Library *dosbase );
VOID *ASM AllocFunc ( REG (d0 )ULONG size , REG (d1 )ULONG flags , REG (a6 )struct Library *SysBase );
VOID ASM FreeFunc ( REG (a1 )APTR mem , REG (d0 )ULONG size , REG (a6 )struct Library *SysBase );

/* readlist.c */
struct DataType *ASM ReadDataTypeList ( REG (a6 )struct GlobalData *gd , REG (d0 )STRPTR name );

/* adddatatype.c */
LONG AddDataTypeA ( struct GlobalData *gd , struct DataType *dtn , struct TagItem *attrs );

/* removedatatype.c */
LONG ASM RemoveDataType ( REG (a6 )struct GlobalData *gd , REG (a0 )struct DataType *dtn );
