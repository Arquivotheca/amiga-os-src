/* Prototypes for functions defined in c/ListTools.c */
struct LTList *InitLTHead(struct LTList *h);
struct LTList *AddLTHead(struct LTList *h,
                         struct LTList *l);
struct LTList *AddLTTail(struct LTList *h,
                         struct LTList *l);
struct LTList *DeleteLT(struct LTList *l);
void JoinLTLists(struct LTList *h1,
                 struct LTList *h2);
void FSplitLTLists(struct LTList *h1,
                   struct LTList *h2);
