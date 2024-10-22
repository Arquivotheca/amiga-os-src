
/* misc.c */
VOID SetPrefsRast ( EdDataPtr ed , UWORD pen );
BOOL allocplanes ( EdDataPtr ed , struct BitMap *bm , PLANEPTR *planes , UWORD width );
VOID freeplanes ( EdDataPtr ed , struct BitMap *bm , PLANEPTR *planes , UWORD width );
BOOL allocbitmap ( EdDataPtr ed , struct BitMap *bm , UWORD depth , UWORD width , UWORD height );
VOID freebitmap ( EdDataPtr ed , struct BitMap *bm , UWORD width , UWORD height );
EdStatus allocpp ( EdDataPtr ed , struct ExtPrefs *pp );

/* pe_custom.c */
VOID ProcessArgs ( EdDataPtr ed , struct DiskObject *diskObj );
VOID CopyPrefs ( EdDataPtr ed , struct ExtPrefs *dp , struct ExtPrefs *sp );
BOOL InitDisp ( EdDataPtr ed );
EdStatus InitEdData ( EdDataPtr ed );
VOID CleanUpEdData ( EdDataPtr ed );
BOOL CreateDisplay ( EdDataPtr ed );
VOID DisposeDisplay ( EdDataPtr ed );
VOID CenterLine ( EdDataPtr ed , struct RastPort *rp , AppStringsID id , UWORD x , UWORD y , UWORD w );
VOID RefreshRepeats ( EdDataPtr ed );
VOID RenderDisplay ( EdDataPtr ed );
VOID RenderGadgets ( EdDataPtr ed );
VOID SyncTextGadgets ( EdDataPtr ed );
APTR newdtobject ( EdDataPtr ed , STRPTR name , Tag tag1 , ...);
ULONG getdtattrs ( EdDataPtr ed , Object *o , ULONG data , ...);
ULONG setdtattrs ( EdDataPtr ed , Object *o , ULONG data , ...);
EdStatus UndoFunc ( EdDataPtr ed );
EdStatus ClearFunc ( EdDataPtr ed );
ULONG ASM Filter ( REG (a0 )struct Hook *h , REG (a2 )struct FileRequester *fr , REG (a1 )struct AnchorPath *ap );
BOOL SelectSample ( EdDataPtr ed , ULONG tag1 , ...);
EdStatus TestFunc ( EdDataPtr ed );
VOID ProcessSpecialCommand ( EdDataPtr ed , EdCommand ec );
EdCommand GetCommand ( EdDataPtr ed );
VOID GetSpecialCmdState ( EdDataPtr ed , EdCommand ec , CmdStatePtr state );

/* sketchgclass.c */
Class *initsketchGClass ( EdDataPtr ed );
ULONG freesketchGClass ( EdDataPtr ed , Class *cl );
ULONG __stdargs dispatchsketchG ( Class *cl , Object *o , Msg msg );
ULONG initsketchGAttrs ( Class *cl , Object *o , struct opSet *msg );
ULONG setsketchGAttrs ( Class *cl , Object *o , struct opSet *msg );
ULONG handlesketchG ( Class *cl , Object *o , struct gpInput *msg );
VOID WriteChunkyPixel ( Class *cl , Object *o , struct RastPort *srp , UWORD x , UWORD y , UWORD mode );
ULONG rendersketchG ( Class *cl , Object *o , struct gpRender *msg , WORD x , WORD y , BOOL fast );
VOID Store ( EdDataPtr ed );
VOID Restore ( EdDataPtr ed );

/* io.c */
VOID UpdateSketch ( EdDataPtr ed );
EdStatus ReadPrefs ( EdDataPtr ed , struct IFFHandle *iff , struct ContextNode *cn );
EdStatus OpenPrefs ( EdDataPtr ed , STRPTR name );
EdStatus WritePrefs ( EdDataPtr ed , struct IFFHandle *iff , struct ContextNode *cn );
EdStatus SavePrefs ( EdDataPtr ed , STRPTR name );
VOID CopyFromDefault ( EdDataPtr ed );

/* pe_utils.c */
struct Window *OpenPrefsWindow ( EdDataPtr ed , ULONG tag1 , ...);
APTR AllocPrefsRequest ( EdDataPtr ed , ULONG type , ULONG tag1 , ...);
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
VOID ASM IntuiHook ( REG (a0 )struct Hook *hook , REG (a2 )struct FileRequester *fr , REG (a1 )struct IntuiMessage *intuiMsg );
BOOL FileRequest ( EdDataPtr ed , BOOL mode );

/* pe_iff.c */
struct IFFHandle *GetIFF ( EdDataPtr ed , STRPTR name , LONG mode , EdStatus *result );
VOID ReturnIFF ( EdDataPtr ed , struct IFFHandle *iff );
EdStatus ReadIFF ( EdDataPtr ed , STRPTR name , ULONG *stopChunks , ULONG chunkCnt , IFFFunc readFunc );
EdStatus WriteIFF ( EdDataPtr ed , STRPTR name , APTR hdr , IFFFunc writeFunc );

/* pe_strings.c */
STRPTR GetString ( struct LocaleInfo *li , AppStringsID id );

/* clipboard.c */
EdStatus EraseFunc ( EdDataPtr ed );
EdStatus CopyFunc ( EdDataPtr ed );
EdStatus CutFunc ( EdDataPtr ed );
BOOL SelectImage ( EdDataPtr ed , ULONG tag1 , ...);
EdStatus PasteFunc ( EdDataPtr ed , EdCommand ec );

/* cstubs.c */
struct Screen *OpenPrefsScreen ( EdDataPtr ed , ULONG tag1 , ...);
VOID DrawBB ( EdDataPtr ed , SHORT x , SHORT y , SHORT w , SHORT h , ULONG tags , ...);
VOID setgadgetattrs ( EdDataPtr ed , struct Gadget *gad , ULONG tags , ...);
APTR NewPrefsObject ( EdDataPtr ed , struct IClass *cl , UBYTE *name , ULONG data , ...);
