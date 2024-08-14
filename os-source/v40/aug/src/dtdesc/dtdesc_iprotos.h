
/* readdatatype.c */
struct DataType *ReadDataType ( struct AppInfo *ai , STRPTR name );
struct DataType *GetDataType ( struct AppInfo *ai , struct IFFHandle *iff );
struct DataType *GetDTYPForm ( struct AppInfo *ai , struct IFFHandle *iff );
struct DataType *GetDTHD ( struct AppInfo *ai , struct IFFHandle *iff );
LONG ReadFunc ( struct MyFileHandle *fh , STRPTR buff , ULONG len , struct Library *dosbase );
VOID *AllocFunc ( ULONG size , ULONG flags , struct Library *SysBase );
VOID FreeFunc ( APTR mem , ULONG size , struct Library *SysBase );

/* icon.c */

/* filerequester.c */
APTR allocaslrequest ( struct AppInfo *ai , ULONG type , ULONG tag1 , ...);
BOOL FileRequest ( struct AppInfo *ai , BOOL mode , STRPTR title , STRPTR dir , STRPTR name );

/* writedtyp.c */
BOOL WriteDTYP ( struct AppInfo *ai , STRPTR fname );
BOOL AddDTYP ( struct AppInfo *ai );

/* scan.c */
VOID SetDataType ( struct AppInfo *ai );
VOID ScanFiles ( struct AppInfo *ai );

/* open.c */
VOID AddWBArgs ( struct AppInfo *ai , struct WBArg *wbarglist , LONG numargs );

/* filenode.c */
struct Node *FindSelected ( struct AppInfo *ai , struct Node *snode , WORD num );
struct Node *FindFileNodeLock ( struct AppInfo *ai , BPTR lock );
VOID removefilenode ( struct FileNode *fn );
VOID removefunc ( struct AppInfo *ai );
VOID removeallfunc ( struct AppInfo *ai );

/* main.c */
int cmd_start ( void );
BOOL AppWindowIDCMP ( struct AppInfo *ai );
VOID SetAppAttrs ( struct AppInfo *ai );
BOOL AppWindowE ( struct AppInfo *ai , UBYTE command );
VOID AppRemoveWindow ( struct AppInfo *ai );

/* mask.c */
BOOL FindPoint ( struct AppInfo *ai );
VOID DrawPixel ( struct AppInfo *ai , WORD ch , WORD tx , WORD ty , WORD w , WORD h );
VOID ShowMask ( struct AppInfo *ai );

/* funcs.c */
void FreeDataTypeNode ( struct AppInfo *ai , struct DataType *dtn );
VOID RemoveFunc ( struct AppInfo *ai );
VOID RemoveAllFunc ( struct AppInfo *ai );
VOID NewFunc ( struct AppInfo *ai );
LONG ProcessCommand ( struct AppInfo *ai , ULONG id );

/* readdtyp.c */
struct DataType *ReadDTYP ( struct AppInfo *ai , STRPTR name );
void SetGroupID ( struct AppInfo *ai , ULONG gid );

/* writedatatype.c */
LONG WriteDataType ( struct AppInfo *ai , STRPTR name , struct DataType *dtn );
LONG PutDataType ( struct AppInfo *ai , struct IFFHandle *iff , struct DataType *dtn );
BOOL WriteDTHDChunk ( struct AppInfo *ai , struct IFFHandle *iff , struct DataType *dtn );
