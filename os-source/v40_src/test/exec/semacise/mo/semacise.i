/* Prototypes for functions defined in m/Semacise.c */
void main(void);
struct SemaMessage *CreateSemaMessage(struct MsgPort *ReplyToMe);
void DoSemaMessage(struct GlobalEnvironment *ge,
                   ULONG tasknum,
                   enum Command cmd,
                   void *arg,
                   long index);
void DestroySemaMessage(struct SemaMessage *m);
__saveds void Child(void);
void DisplayMessage(struct RastPort *rp,
                    ULONG tasknum,
                    enum Command cmd,
                    long index);
void ChildRender(struct RastPort *rp,
                 ULONG tasknum,
                 ULONG rend);
BOOL CreateInterface(struct GlobalEnvironment *ge);
void ClearStrings(struct GlobalEnvironment *ge);
void DestroyInterface(struct GlobalEnvironment *ge);
void DoInterface(struct GlobalEnvironment *ge);
void StepChild(struct GlobalEnvironment *ge,
               int child);
void ResetChildren(struct GlobalEnvironment *ge);
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
