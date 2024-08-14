/* Prototypes for functions defined in
modclass.c
 */

Class * initModelClass(void);

static ULONG modDispatch(Class * , Object * , Msg );

static int notifyMod(Class * , Object * , struct opUpdate * , struct ModData * );

