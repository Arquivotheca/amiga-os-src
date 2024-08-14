/* Prototypes for functions defined in
rect.c
 */

extern struct Rectangle nullrect;

int eraseRect(struct RastPort * rp,
              struct Rectangle * r);

int eraseRemnants(struct RastPort * rp,
                  struct IBox * oldbox,
                  struct IBox * newbox,
                  struct Point offset,
                  UBYTE pen,
                  UBYTE patternpen);

int fillAround(struct RastPort * rp,
               struct Rectangle * yesrect,
               struct Rectangle * norect,
               UBYTE pen,
               UBYTE patternpen);

int boxModernize(int (* func)(),
                 struct RastPort * rp,
                 struct IBox * box);

