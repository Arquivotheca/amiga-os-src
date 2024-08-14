/* Prototypes for functions defined in
icclass.c
 */

Class * initICClass(void);

static ULONG icDispatch(Class * , Object * , Msg );

static int getICAttrs(Class * , Object * , struct opGet * , struct ICData * );

static int setICAttrs(Class * , Object * , struct opSet * );

