/* Prototypes for functions defined in
pools.c
 */

static struct Node * createPoolNode(struct List * );

int poolInit(struct List * lh,
             int size,
             int initnum);

struct Node * poolGet(struct List * lh);

int poolReturn(struct Node * ln);

