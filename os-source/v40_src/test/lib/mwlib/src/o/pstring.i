/* Prototypes for functions defined in c/PString.c */
long PString(char *dest,
             char *parm,
             char *keys,
             void *arg, ...);
long _PString(char *dest,
              char *parm,
              struct keys *keys,
              void **arr);
