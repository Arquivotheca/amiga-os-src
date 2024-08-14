/* Prototypes for functions defined in c/GadgetTools.c */
void SetGadgetIDCounter(int n);
struct Gadget *MakeBGadget(struct Remember **k,
                           struct Image *i,
                           struct Image *a,
                           int LeftEdge,
                           int TopEdge,
                           USHORT flags,
                           USHORT activation);
struct Gadget *MakeCRBGadget(struct Remember **k,
                             int left,
                             int top,
                             int width,
                             int height,
                             ULONG flags,
                             ULONG activation);
struct Gadget *MakeRTGList(struct Remember **key,
                           struct RTGList *list,
                           struct CustomGadgetClass *cgc);
void RefreshRTGList(struct RTGList *list,
                    struct Window *win,
                    struct Requester *req);
void RefreshRGList(struct RTGList *list,
                   struct Window *win,
                   struct Requester *req);
struct Gadget *MakePropGadget(struct Remember **key,
                              struct Image *knob_image,
                              int left,
                              int top,
                              int width,
                              int height,
                              ULONG flags,
                              ULONG prop_flags,
                              ULONG activation);
struct Gadget *MakeStrGadget(struct Remember **key,
                             int LeftEdge,
                             int TopEdge,
                             int Width,
                             int Height,
                             int MaxChars,
                             UBYTE *buf,
                             UBYTE *undo,
                             ULONG Flags,
                             ULONG Activation,
                             ULONG Type);
struct Border *_MakeABorder(struct Remember **key,
                            short vectors,
                            short left,
                            short top,
                            short front,
                            short back,
                            short draw);
int MakeRTGBorders(struct Remember **key,
                   struct RTGList *rlist);
int GadgetCount(struct Gadget *g);
