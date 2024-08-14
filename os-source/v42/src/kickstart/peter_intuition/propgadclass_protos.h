/* Prototypes for functions defined in
propgadclass.c
 */

Class * initPropGadgetClass(void);

static ULONG propgadgetDispatch(Class * , Object * , Msg );

static int getPropGadgetAttrs(Class * , Object * , struct opGet * );

static int setPropGadgetAttrs(Class * , Object * , struct opSet * );

static int updatePropInfo(struct PGIData * );

static UWORD propTop(struct PGIData * );

static UWORD propBody(struct ScrollSet * );

static UWORD propPot(struct ScrollSet * );

static UWORD propHidden(struct ScrollSet * );

