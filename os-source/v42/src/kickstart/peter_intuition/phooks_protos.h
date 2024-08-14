/* Prototypes for functions defined in
phooks.c
 */

ULONG propHook(struct Hook * h,
               struct Gadget * g,
               ULONG * cgp);

static int propGoActive(struct Gadget * , struct gpInput * );

static int propGoInactive(struct Gadget * , struct gpGoInactive * );

static int propHandleInput(struct Gadget * , struct gpInput * );

static int safeDisplayProp(struct Gadget * , struct GadgetIngo * , struct PGX * );

