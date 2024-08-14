/* Prototypes for functions defined in
genv.c
 */

int lockGDomain(struct GListEnv * ge);

int unlockGDomain(struct GListEnv * ge);

struct Point * GadgetMouse(struct Gadget * g,
                           struct GadgetInfo * gi,
                           struct Point * mouse);

struct Point * gadgetMouse(struct Gadget * g,
                           struct GadgetInfo * gi,
                           struct Point * mouse,
                           struct IBox * gbox);

struct Point * gadgetPoint(struct Gadget * g,
                           struct GadgetInfo * gi,
                           struct Point windowcoord,
                           struct Point * gadgetcoord,
                           struct IBox * gbox);

int gadgetInfo(struct Gadget * g,
               struct GListEnv * ge);

struct Hook * gadgetHook(struct Gadget * g);

BOOL gListDomain(struct Gadget * g,
                 struct Window * window,
                 struct GListEnv * ge);

int clearWords(UWORD * ptr,
               int numwords);

int gadgetBox(struct Gadget * g,
              struct GadgetInfo * gi,
              struct IBox * box);

int gadgetBoundingBox(struct Gadget * g,
                      struct GadgetInfo * gi,
                      struct IBox * box);

static int gadgetBoxGrunt(struct Gadget * , struct GadgetInfo * , struct IBox * );

struct Requester * findGadgetRequest(struct Gadget * gadget,
                                     struct Window * window);

struct IBox * getImageBox(struct IBox * im,
                          struct IBox * box);

