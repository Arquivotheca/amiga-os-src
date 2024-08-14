/* Prototypes for functions defined in
ihooks.c
 */

int initHook(struct Hook * hook,
             ULONG (* dispatch)());

extern hookfunc_t myhookfuncs[4];

int InitIIHooks(void);

