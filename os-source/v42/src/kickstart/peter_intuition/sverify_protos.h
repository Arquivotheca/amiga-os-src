/* Prototypes for functions defined in
sverify.c
 */

int startVerify(UWORD (* vreturn)(),
                struct Window * window,
                ULONG class,
                UWORD code);

int dVerify(void);

static int verifyAbort(int );

