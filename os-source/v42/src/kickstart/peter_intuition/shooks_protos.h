/* Prototypes for functions defined in
shooks.c
 */

ULONG stringHook(struct Hook * h,
                 struct Gadget * g,
                 ULONG * cgp);

static int stringRender(struct Gadget * , struct gpRender * );

static int stringGoActive(struct Gadget * , struct gpInput * );

static int safeDisplayString(struct Gadget * , struct GadgetIngo * );

static int stringGoInactive(struct Gadget * , struct gpGoInactive * );

static int stringHandleInput(struct Gadget * , struct gpInput * );

