/* Prototypes for functions defined in
drawgadgets.c
 */

int commonGadgetRender(struct RastPort * rp,
                       struct Gadget * g,
                       struct GadgetInfo * gi,
                       struct IBox * gbox,
                       LONG suppress_selected);

int commonGadgetTextAndGhost(struct RastPort * rp,
                             struct Gadget * g,
                             struct GadgetInfo * gi,
                             struct IBox * gbox);

int gadgetDrawState(struct Gadget * g,
                    struct GadgetInfo * gi,
                    int suppress_selected);

int drawGadgets(APTR ptr,
                struct Gadget * gadget,
                int numgad,
                int filter_flags);

static int drawGGrunt(struct RastPort * , struct Gadget * , struct GListEnv * , int , int );

int callGadgetHook(struct Hook * hook,
                   struct Gadget * gad,
                   ULONG method,
                   struct GadgetInfo * ginfo);

int callGadgetHookPkt(struct Hook * hook,
                      struct Gadget * gad,
                      Msg message);

int propRender(struct Gadget * g,
               struct gpRender * cgp);

int renderNewKnob(struct Gadget * g,
                  struct GadgetInfo * gi,
                  struct PGX * pgx,
                  struct RastPort * rp);

static int getContainerPens(struct Gadget * , struct GadgetInfo * , UBYTE * );

int drawKnob(struct Gadget * g,
             struct GadgetInfo * gi,
             struct RastPort * rp,
             struct PGX * pgx);

