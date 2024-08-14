/* Prototypes for functions defined in
prop.c
 */

int containerBumpPots(struct Gadget * g,
                      struct GadgetInfo * gi,
                      struct Point coord,
                      struct IBox * knobbox);

static UWORD containerIncrement(USHORT );

static int dimensionNewKnobForBodys(struct Gadget * , struct GadgetInfo * , struct PGX * );

int positionNewKnobForPots(struct Gadget * g,
                           struct GadgetInfo * gi,
                           struct PGX * pgx);

static USHORT potInRange(LONG );

BOOL dragNewKnob(struct Gadget * g,
                 struct GadgetInfo * gi,
                 struct Point mousept,
                 struct IBox * gbox);

static int setPotsForKnobPosition(struct Gadget * , struct GadgetInfo * , struct Point , struct PGX * );

int syncKnobs(struct Gadget * g,
              struct GadgetInfo * gi,
              struct PGX * pgx);

int borderlessProp(struct Gadget * g);

int setupPGX(struct Gadget * g,
             struct GadgetInfo * gi,
             struct PGX * pgx);

