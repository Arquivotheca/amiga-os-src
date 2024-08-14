/* Prototypes for functions defined in
strgadclass.c
 */

Class * initStrGadgetClass(void);

static ULONG strgadgetDispatch(Class * , Object * , Msg );

static int getStrGadgetAttrs(Class * , Object * , struct opGet * );

extern ULONG stringPackTable[17];

static int setStrGadgetAttrs(Class * , Object * , struct opSet * );

