/* Prototypes for functions defined in
ggclass.c
 */

Class * initGGClass(void);

static ULONG ggDispatch(Class * , Object * , Msg );

static int translateCGPInput(struct Gadget * , struct Gadget * , struct gpInput * );

static int propagateHit(Object * , struct GGData * , struct gpHitTest * );

static int setGGAttrs(Class * , Object * , struct opSet * );

