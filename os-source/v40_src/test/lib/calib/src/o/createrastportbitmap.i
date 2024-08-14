/* Prototypes for functions defined in c/CreateRastPortBitMap.c */
struct RastPort *CreateRastPortBitMap(long width,
                                      long height,
                                      long depth,
                                      long vectors,
                                      BOOL clip);
void FreeRastPortBitMap(struct RastPort *rp);
