/* Prototypes for functions defined in
fillrectclass.c
 */

Class * initFillRectClass(void);

static ULONG dispatchFillRect(Class * , Object * , Msg );

static int drawFillRect(Class * , Object * , struct impDraw * );

static int setFillRectAttrs(Class * , Object * , struct opSet * );

