/* Prototypes for functions defined in c/RequesterTools.c */
void SetReqBackFill(int n);
void SetIdleRoutine(void (*id)(void));
int DoRequest(struct Remember **pkey,
              struct BitMap *bmap,
              struct RTGList *rlist,
              struct ITList *ilist,
              struct Window *window,
              int left,
              int top,
              int width,
              int height,
              void *udata);
int UpdateStringEntry(struct RTGList *r);
void AllocateReqStrBuffs(struct Remember **pkey,
                         struct RTGList *r);
void ClearSelect(struct RTGList *rtg);
void ClearGadgets(struct RTGList *rtg);
int _MutualExclude(struct RTGList *rlist,
                   struct RTGList *r,
                   struct Window *win,
                   struct Requester *req);
void DoRugTopRequest(struct Remember **pkey,
                     struct RTGList *rlist,
                     struct ITList *ilist,
                     struct Window *win,
                     struct Requester *req);
void RugPullRequest(struct RTGList *rlist,
                    struct Window *win,
                    struct Requester *req);
void UpdateRTGList(struct RTGList *r);
short RegisterCGC(short class,
                  BOOL (*create)(void),
                  BOOL (*handler)(void),
                  BOOL (*remove)(void),
                  BOOL (*refresh)(void),
                  BOOL (*update)(void),
                  BOOL (*idle)(void));
