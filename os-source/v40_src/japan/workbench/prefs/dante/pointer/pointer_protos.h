/* pe_custom.c */
VOID ProcessArgs ( EdDataPtr ed , struct DiskObject *diskObj );
EdStatus InitEdData ( EdDataPtr ed );
VOID CopyPrefs ( EdDataPtr ed , struct PointerPrefs *sp , struct PointerPrefs *dp );
BOOL InitDisp ( EdDataPtr ed );
VOID CleanUpEdData ( EdDataPtr ed );
BOOL CreateDisplay ( EdDataPtr ed );
VOID DisposeDisplay ( EdDataPtr ed );
VOID RenderDisplay ( EdDataPtr ed );
VOID SetSliders ( EdDataPtr ed );
VOID RenderGadgets ( EdDataPtr ed );
VOID SyncTextGadgets ( EdDataPtr ed );
EdStatus TestFunc ( EdDataPtr ed );
VOID UpdateColor ( EdDataPtr ed );
VOID ProcessSpecialCommand ( EdDataPtr ed , EdCommand ec );
EdCommand GetCommand ( EdDataPtr ed );
VOID GetSpecialCmdState ( EdDataPtr ed , EdCommand ec , CmdStatePtr state );
VOID RefreshRepeats (EdDataPtr ed);

/* clipboard.c */
struct IFFHandle *OpenClip ( EdDataPtr ed );
VOID CloseClip ( EdDataPtr ed , struct IFFHandle *iff );
LONG QueryClip ( EdDataPtr ed , struct IFFHandle *iff , ULONG *clipType , ULONG *clipID );
EdStatus EraseFunc ( EdDataPtr ed );
EdStatus CopyFunc ( EdDataPtr ed );
EdStatus CutFunc ( EdDataPtr ed );
EdStatus PasteFunc ( EdDataPtr ed );

/* ilbm.c */
UWORD ConvertPP ( EdDataPtr ed , struct PointerPrefs *sp , struct PointerPrefs *dp , WORD adj );
EdStatus ReadPrefs ( EdDataPtr ed , struct IFFHandle *iff );
EdStatus WritePrefs ( EdDataPtr ed , struct IFFHandle *iff );
BOOL GetBMHD ( EdDataPtr ed , struct IFFHandle *iff , struct BitMapHeader *bmhd );
BOOL PutBMHD ( EdDataPtr ed , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct PointerPrefs *pp );
WORD GetColors ( EdDataPtr ed , struct IFFHandle *iff , struct ColorRegister *cmap );
BOOL PutColors ( EdDataPtr ed , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct ColorRegister *cmap );
void GetHotSpot ( EdDataPtr ed , struct IFFHandle *iff , struct Point2D *grab , WORD width , WORD height );
BOOL PutHotSpot ( EdDataPtr ed , struct IFFHandle *iff , struct Point2D *grab );
BOOL GetBody ( EdDataPtr ed , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct BitMap *bm );
BOOL GetLine ( EdDataPtr ed , struct IFFHandle *iff , UBYTE *buf , WORD wide , WORD deep , UBYTE cmptype );
BOOL PutBody ( EdDataPtr ed , struct IFFHandle *iff , struct BitMapHeader *bmhd , struct BitMap *bitmap );
VOID FreeILBM ( EdDataPtr ed , struct PointerPrefs *pp );

/* sketchgclass.c */
Class *initsketchGClass ( EdDataPtr ed );
ULONG freesketchGClass ( EdDataPtr ed , Class *cl );
ULONG __stdargs dispatchsketchG ( Class *cl , Object *o , Msg msg );
ULONG initsketchGAttrs ( Class *cl , Object *o , struct opSet *msg );
ULONG setsketchGAttrs ( Class *cl , Object *o , struct opSet *msg );
ULONG getsketchGAttrs ( Class *cl , Object *o , struct opGet *msg );
ULONG handlesketchG ( Class *cl , Object *o , struct gpInput *msg );
VOID WriteChunkyPixel ( Class *cl , Object *o , struct RastPort *srp , UWORD x , UWORD y , UWORD mode );
ULONG rendersketchG ( Class *cl , Object *o , struct gpRender *msg , WORD x , WORD y , BOOL fast );
VOID Store (EdDataPtr ed);
VOID Restore (EdDataPtr ed);

/* io.c */
VOID CopyFromSketchBM ( EdDataPtr ed );
VOID UpdateSketch ( EdDataPtr ed );
EdStatus OpenPrefs ( EdDataPtr ed , STRPTR name );
EdStatus SavePrefs ( EdDataPtr ed , STRPTR name );
void UpdateSamples ( EdDataPtr ed , WORD x , WORD y , WORD c );
void InitSamples ( EdDataPtr ed );
VOID CopyFromDefault ( EdDataPtr ed );

/* misc.c */
BOOL allocplanes ( EdDataPtr ed , struct BitMap *bm , PLANEPTR *planes , UWORD width );
VOID freeplanes ( EdDataPtr ed , struct BitMap *bm , PLANEPTR *planes , UWORD width );
BOOL allocbitmap ( EdDataPtr ed , struct BitMap *bm , UWORD depth , UWORD width , UWORD height );
VOID freebitmap ( EdDataPtr ed , struct BitMap *bm , UWORD width , UWORD height );
EdStatus allocpp ( EdDataPtr ed , struct PointerPrefs *pp );

/* pe_utils.c */
struct Window *OpenPrefsWindow ( EdDataPtr ed , ULONG tag1 , ...);
BOOL RequestPrefsFile ( EdDataPtr ed , ULONG tag1 , ...);
BOOL LayoutPrefsMenus ( EdDataPtr ed , struct Menu *menus , ULONG tag1 , ...);
struct Menu *CreatePrefsMenus ( EdDataPtr ed , struct EdMenu *em );
struct Gadget *DoPrefsGadget ( EdDataPtr ed , struct EdGadget *eg , struct Gadget *gad , ULONG tags , ...);
VOID SetGadgetAttr ( EdDataPtr ed , struct Gadget *gad , ULONG tags , ...);
VOID ShowError1 ( EdDataPtr ed , AppStringsID es );
VOID ShowError2 ( EdDataPtr ed , EdStatus es );

/* pe_main.c */
int main ( int argc , char *argv );
VOID OpenLib ( EdDataPtr ed , struct Library **libBase , STRPTR libName , AppStringsID error );
UBYTE DoDiskObj ( EdDataPtr ed , STRPTR name );
VOID ProcessCommand ( EdDataPtr ed , EdCommand ec );
VOID GetCmdState ( EdDataPtr ed , EdCommand ec , CmdStatePtr state );
VOID ProcessItem ( EdDataPtr ed , struct MenuItem *item );
VOID EventLoop ( EdDataPtr ed );
BOOL CreateDisp ( EdDataPtr ed );
VOID DisposeDisp ( EdDataPtr ed );
VOID RenderGadg ( EdDataPtr ed );
VOID WriteIcon ( EdDataPtr ed , STRPTR name );
struct IntuiMessage *__stdargs DoFRRefresh ( ULONG mask , struct IntuiMessage *intuiMsg , struct FileRequester *freq );
BOOL FileRequest ( EdDataPtr ed , BOOL mode );

/* pe_strings.c */
STRPTR GetString ( struct LocaleInfo *li , AppStringsID id );

/* cstubs.c */
struct Screen *OpenPrefsScreen ( EdDataPtr ed , ULONG tag1 , ...);
VOID DrawBB ( EdDataPtr ed , SHORT x , SHORT y , SHORT w , SHORT h , ULONG tags , ...);
VOID setgadgetattrs ( EdDataPtr ed , struct Gadget *gad , ULONG tags , ...);
APTR NewPrefsObject ( EdDataPtr ed , struct IClass *cl , UBYTE *name , ULONG data , ...);
