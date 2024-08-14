/* Prototypes for functions defined in m/FLockercise.c */
void _main(void);
struct FLockMessage *CreateFLockMessage(struct MsgPort *ReplyToMe);
void DoFLockMessage(struct GlobalEnvironment *ge,
                    long tasknum,
                    enum Command cmd,
                    void *arg,
                    char *string);
void DestroyFLockMessage(struct GlobalEnvironment *ge,
                         long tasknum,
                         struct FLockMessage *m);
void __saveds __asm Child(void);
void DisplayMessage(struct GlobalEnvironment *ge,
                    struct RastPort *rp,
                    ULONG tasknum,
                    enum Command cmd,
                    void *v1,
                    void *v2, ...);
void DisplayLockStatus(struct RastPort *rp,
                       long tasknum,
                       BOOL locked);
void ChildRender(struct RastPort *rp,
                 ULONG tasknum,
                 ULONG rend);
BOOL CreateInterface(struct GlobalEnvironment *ge);
void ClearStrings(struct GlobalEnvironment *ge);
void DestroyInterface(struct GlobalEnvironment *ge);
void DoInterface(struct GlobalEnvironment *ge);
void LoadSetup(struct GlobalEnvironment *ge);
void SaveSetup(struct GlobalEnvironment *ge);
void StepChild(struct GlobalEnvironment *ge,
               int child);
void ResetChildren(struct GlobalEnvironment *ge);
void NoLoop(struct GlobalEnvironment *ge);
int FindChildEntry(struct GlobalEnvironment *ge,
                   struct Task *task);
BOOL AnyRemainingChildren(struct GlobalEnvironment *ge);
struct Gadget *FirstGadget(struct GlobalEnvironment *ge,
                           UBYTE *text,
                           ULONG kind,
                           WORD left,
                           WORD top,
                           WORD width,
                           WORD height,
                           ULONG flags,
                           UWORD gid,
                           APTR uid,
                           ULONG taglist, ...);
struct Gadget *NextGadget(struct GlobalEnvironment *ge,
                          UBYTE *text,
                          ULONG kind,
                          WORD dleft,
                          WORD dtop,
                          ULONG flags,
                          ULONG gid,
                          APTR uid,
                          ULONG taglist, ...);
void ShutDown(void);
void StateSave(void);
