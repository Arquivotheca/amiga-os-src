/* Prototypes for functions defined in c/LRand.c */
struct LREnv *LROpen(ULONG Seed,
                     ULONG NumRegs,
                     ULONG Tap);
void LRClose(struct LREnv *lr);
ULONG LRand(struct LREnv *lr);
