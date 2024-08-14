/* Prototypes for functions defined in
gadclass.c
 */

Class * initGadgetClass(void);

static ULONG gadgetDispatch(Class * , Object * , Msg );

extern ULONG gadgetPackTable[52];

static int setGadgetAttrs(Class * , Object * , struct opSet * );

static int setGHigh(struct Gadget * , int );

