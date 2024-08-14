/* Prototypes for functions defined in TstMgr_tm.c */
LONG TM_Request(struct Window *Window,
                char *Title,
                char *TextFormat,
                char *GadgetFormat,
                ULONG *IDCMP_ptr,
                APTR Arg1, ...);
struct TMData *TM_Open(ULONG *error);
void TM_Close(struct TMData *TMData);
void TM_EventLoop(struct TMData *TMData);
BOOL OpenScreen_TESTMANA(struct TMData *TMData);
void CloseScreen_TESTMANA(struct TMData *TMData);
BOOL OpenWindow_ERRWINDO(struct TMData *TMData);
void CloseWindow_ERRWINDO(struct TMData *TMData);
void DisableWindow_ERRWINDO(struct TMData *TMData);
void EnableWindow_ERRWINDO(struct TMData *TMData);
static BOOL TM_WindowSignal(struct TMData *TMData);
static BOOL WindowIDCMP_ERRWINDO(struct TMData *TMData,
                                 struct IntuiMessage *imessage);
static void TM_RemoveWindow(struct TMWindowInfo *TMWindowInfo);
