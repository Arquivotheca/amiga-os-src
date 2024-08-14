/* Prototypes for functions defined in c/GelTools.c */
void border_dummy(void);
int ReadyGels(struct Remember **k,
              struct GelsInfo *g,
              struct RastPort *r);
struct VSprite *MakeVSprite(struct Remember **k,
                            struct Image *i,
                            WORD *colorset,
                            SHORT flags);
struct Bob *MakeBob(struct Remember **k,
                    struct Image *i,
                    SHORT flags);
struct Bob *DoubleBob(struct Remember **k,
                      struct Bob *bob);
