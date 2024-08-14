/* Prototypes for functions defined in
bhooks.c
 */

ULONG boolHook(struct Hook * h,
               struct Gadget * g,
               ULONG * cgp);

static int boolHitTest(struct Gadget * , struct gpHitTest * );

static int boolRender(struct Gadget * , struct gpRender * );

static int boolGoActive(struct Gadget * , struct gpInput * );

static int boolGoInactive(struct Gadget * , struct gpGoInactive * );

static int boolHandleInput(struct Gadget * , struct gpInput * );

static int boolVisuals(struct RastPort * , struct Gadget * , struct GadgetInfo * , int , struct IBox * );

static int toggleBoolGadget(struct Gadget * , struct GadgetInfo * , struct IBox * );

static int toggleBoolHighlight(struct RastPort * , struct Gadget * , struct GadgetInfo * , struct IBox * );

static int xormask(struct RastPort * , struct IBox * , UWORD * );

