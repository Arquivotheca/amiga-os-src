/* Prototypes for functions defined in c/MWReadArgs.c */
struct RDArgs *MWReadArgs(UBYTE *template,
                          LONG *array,
                          struct RDArgs *rdargs);
long MWReadItem(UBYTE *buffer,
                long maxchars,
                struct CSource *cs,
                char delim);
long MWReadChar(struct CSource *cs);
BOOL MWUnReadChar(struct CSource *cs);
