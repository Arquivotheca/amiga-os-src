
/* main.c */
LONG main ( void );
BOOL ObtainScreen ( struct GlobalData *gd );
void ReleaseScreen ( struct GlobalData *gd );
BOOL CreatePorts ( struct GlobalData *gd );
void DeletePorts ( struct GlobalData *gd );
BOOL CreateDemoData ( struct GlobalData *gd );
void DeleteDemoData ( struct GlobalData *gd );

/* nipc.c */
ULONG ASM FindServer ( REG (a0 )struct Hook *h , REG (a2 )struct Task *tc , REG (a1 )struct TagItem *attrs );
ULONG BroadCastMsg ( struct GlobalData *gd , STRPTR msg );
ULONG BroadCastCommand ( struct GlobalData *gd , ULONG cmd , APTR msg , ULONG msize );
void SendBitMap ( struct GlobalData *gd , struct Client *c );
ULONG BroadCastPoints ( struct GlobalData *gd , struct PlotPoints *pp );
ULONG BroadCastTalk ( struct GlobalData *gd , STRPTR text );
void ShowTransError ( struct GlobalData *gd , struct Transaction *trans );
BOOL HandleNIPCEvents ( struct GlobalData *gd );

/* windowclass.c */
Class *initViewWindowClass ( void );
ULONG freeViewWindowClass ( Class *cl );
struct Window *OpenViewWindow ( Class *cl , Tag tag1 , ...);
void SetViewWindowAttrs ( Class *cl , struct Window *w , Tag tag1 , ...);
void GetCurrentTopValues ( Class *cl , struct Window *w , ULONG *toph , ULONG *topv );
void CloseViewWindow ( Class *cl , struct Window *w );

/* stubs.c */
VOID setwindowpointer ( struct GlobalData *gd , struct Window *win , Tag tag1 , ...);
struct Gadget *creategadget ( struct GlobalData *gd , ULONG kind , struct Gadget *prev , struct NewGadget *ng , ULONG data , ...);
struct Menu *createmenus ( struct GlobalData *gd , struct NewMenu *nm , ULONG data , ...);
struct Menu *layoutmenus ( struct GlobalData *gd , ULONG data , ...);
struct Transaction *alloctransaction ( struct GlobalData *gd , Tag tag1 , ...);
BOOL nipcinquiry ( struct GlobalData *gd , struct Hook *h , ULONG time , ULONG num , Tag tag1 , ...);
struct Entity *createentity ( struct GlobalData *gd , Tag tag1 , ...);
int stcgnm ( char *dest , char *src );
int stcgfp ( char *dest , char *src );
int stcgfn ( char *dest , char *src );
void lprintf ( struct GlobalData *gd , STRPTR fmt , void *arg1 , ...);

/* events.c */
void CreateMainMenu ( struct GlobalData *gd );
BOOL ProcessCommand ( struct GlobalData *gd , ULONG cmd , struct IntuiMessage *imsg );
void HandleEvents ( struct GlobalData *gd );

/* funcs.c */
void ClearFunc ( struct GlobalData *gd , UBYTE pen );
void NewSizeFunc ( struct GlobalData *gd );

/* talk.c */
void TalkFunc ( struct GlobalData *gd , STRPTR text , ULONG len );
BOOL OpenTalkWindow ( struct GlobalData *gd );
BOOL HandleTalkEvents ( struct GlobalData *gd );
void CloseTalkWindow ( struct GlobalData *gd );
VOID SendReadPacket ( struct GlobalData *gd , struct StandardPacket *sp , BPTR fh , struct MsgPort *rport , UBYTE *buff );
struct Window *GetConsoleWindow ( struct GlobalData *gd , BPTR cons );
