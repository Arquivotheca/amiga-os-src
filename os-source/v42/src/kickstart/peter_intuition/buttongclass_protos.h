/* Prototypes for functions defined in
buttongclass.c
 */

Class * initButtonGClass(void);

static ULONG dispatchButtonG(Class * , Object * , Msg );

static int renderButtonG(struct RastPort * , struct GadgetInfo * , Object * , int );

static int setButtonAttrs(Class * , Object * , struct opSet * );

