/* Prototypes for functions defined in
ism.c
 */

int doISM(enum ITCommand command,
          CPTR obj1,
          CPTR obj2,
          ULONG subcommand);

int sendISM(int command,
            int obj1,
            int obj2,
            int subcommand);

int sendISMNoQuick(int command,
                   int obj1,
                   int obj2,
                   int subcommand);

struct InputToken * preSendISM(enum ITCommand command,
                               CPTR obj1,
                               CPTR obj2,
                               ULONG subcommand);

int beginToken(struct InputToken * it);

static int dispatchToken(struct InputToken * );

struct InputToken * newToken(void);

static int disposeToken(struct InputToken * );

int deferToken(void);

int reuseToken(void);

static int notifyToken(struct InputToken * );

int initTokens(void);

