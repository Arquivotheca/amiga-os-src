/* Prototypes for functions defined in
thing.c
 */

struct Thing * delinkThing(struct Thing * thing,
                           struct Thing * prevthing);

struct Thing * previousThing(struct Thing * thing,
                             struct Thing * prevthing,
                             short * pos);

struct Thing * lastThing(struct Thing * thing);

struct Thing * nthThing(struct Thing * thing,
                        int n,
                        short * realn);

struct Thing * previousLink(struct Thing * first,
                            struct Thing * current);

