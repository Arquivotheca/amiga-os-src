
/* scale.c */
VOID ScaleBitMap ( struct AppInfo *ai );

/* ilbm.c */
ILBM *ReadILBM ( BPTR drawer , STRPTR name , UWORD depth );
ILBM *GetILBM ( struct IFFHandle *iff , UWORD depth );
BOOL GetBMHD ( struct IFFHandle *iff , struct BitMapHeader *bmhd );
WORD GetColors ( struct IFFHandle *iff , struct ColorRegister *cmap );
VOID GetHotSpot ( struct IFFHandle *iff , struct Point2D *grab , WORD width , WORD height );
LONG GetBody ( struct IFFHandle *iff , struct BitMapHeader *bmhd , struct BitMap *bitmap );
BOOL unpackrow ( BYTE **pSource , BYTE **pDest , WORD srcBytes0 , WORD dstBytes0 );
LONG loadbody2 ( struct IFFHandle *iff , struct BitMap *bitmap , BYTE *mask , struct BitMapHeader *bmhd , BYTE *buffer , ULONG bufsize );

/* auxbitmap.c */
VOID TrimExtraPlanes (ILBM *ir, UWORD depth);
BOOL AllocBM ( struct BitMap *bm , UWORD depth , LONG width , LONG height );
VOID FreeMyBitMap ( struct BitMap *bm , LONG width , LONG height );
BOOL AllocSecondaryBM ( struct AppInfo *ai );
VOID FreeSecondaryBM (struct AppInfo * ai);
VOID FreeILBM ( ILBM *ir );

/* bitmap.c */
VOID ClearDisplay ( struct AppInfo *ai );
BOOL ScaleImage ( struct AppInfo *ai );
VOID UpdateDisplay ( struct AppInfo *ai );
VOID LoadPicture ( struct AppInfo *ai , BPTR dir , STRPTR name );

/* appwindow.c */
VOID HandleAppMessage ( struct AppInfo *ai );

/* main.c */
LONG main ( int argc , char **argv );
struct Library *openlibrary ( STRPTR name , ULONG version );
BOOL OpenLibraries ( struct AppInfo *ai );
VOID CloseLibraries ( struct AppInfo *ai );
LONG DeadKeyConvert ( struct IntuiMessage *msg , STRPTR kbuffer , LONG kbsize , struct KeyMap *kmap );
STRPTR GetWDisplayString ( struct AppInfo *ai , LONG id );
VOID CLIMsg ( STRPTR msg );
VOID NotifyUser ( struct AppInfo *ai , LONG id );

/* environment.c */
BOOL OpenEnvironment ( struct AppInfo *ai );
VOID CloseEnvironment ( struct AppInfo *ai );

/* window.c */
struct Window *OpenDocWindow ( struct AppInfo *ai );
VOID CloseDocWindow ( struct AppInfo *ai );

/* boundbox.c */
BOOL BoundCheckMouse (struct IntuiMessage * msg);
struct Cursor *AllocAnts ( VOID );
VOID FreeAnts ( struct Cursor *cs );
VOID AbortCut ( struct Window *win , struct Cursor *cs );
struct IBox *DoButtons ( struct IntuiMessage *msg , struct Cursor *cs );
struct IBox *DoMouseMove ( struct IntuiMessage *msg , SHORT xadj , SHORT yadj , struct Cursor *cs );

/* funcs.c */
VOID SetWindowTitle ( struct AppInfo *ai , STRPTR title , LONG id );
LONG SetDisplayMode ( struct AppInfo *ai );
LONG FullSizeFunc ( struct AppInfo *ai );
LONG ScaleSizeFunc ( struct AppInfo *ai );
LONG StatusFunc ( struct AppInfo *ai );
LONG VScrollBarFunc ( struct AppInfo *ai );
LONG HScrollBarFunc ( struct AppInfo *ai );

/* snapshot.c */
struct NewWindow *LoadSnapShot ( struct AppInfo *, struct NewWindow *nw , BPTR prefs_drawer , STRPTR name );
BOOL SaveSnapShot ( struct AppInfo *, struct Window *win , BPTR prefs_drawer , STRPTR name );

/* idcmp.c */
VOID HandleIDCMP ( struct AppInfo *ai );

/* sprites.c */
VOID SetWaitPointer ( struct Window *win );
VOID SetHandPointer ( struct Window *win );
VOID SetBlockPointer ( struct Window *win );
VOID SetBlockPointer ( struct Window *win );

/* readargs.c */
VOID readargs ( struct AppInfo *ai , int argc , char **argv );

/* 24to12.c */
VOID TrimTrueColor (struct AppInfo *ai);

/* misc.c */
VOID CloseWindowSafely (struct Window * win);

/* about.c */
LONG OpenAboutWindow (struct AppInfo * ai);
VOID CloseAboutWindow (struct AppInfo *ai);
VOID UpdateAboutWindow (struct AppInfo *ai);

