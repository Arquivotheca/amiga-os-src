/* appobjects_protos.h
 *
 */

struct ObjectInfo *NewObjListA (struct TagItem *);
VOID DisposeObjList (struct ObjectInfo *);
struct ObjectNode *LookupKeystroke (struct ObjectInfo *,struct IntuiMessage *);
VOID AbortKeystroke (struct ObjectInfo * oi, struct Window * win);
BOOL IsGadToolObj (struct Object *o);
ULONG AddObjList (struct ObjectInfo *, struct Window *, struct TagItem *);
ULONG RemoveObjList (struct ObjectInfo *, struct Window *, struct TagItem *);
ULONG RefreshObjList (struct ObjectInfo *, struct Window *, struct TagItem *);


