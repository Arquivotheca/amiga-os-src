/* Prototypes for functions defined in
sdmrtimeout.c
 */

int startDMRTimeout(UWORD (* dmrreturn)(),
                    int okcode);

int dDMRTimeout(void);

static int tooFarDMR(void);

