/* Prototypes for functions defined in c/ImageTools.c */
BOOL _AllZeroBits(UBYTE *p,
                  int s);
BOOL _AllOneBits(UBYTE *p,
                 int s);
struct Image *BitMapToImage(struct Remember **key,
                            struct BitMap *bm,
                            int left,
                            int top);
