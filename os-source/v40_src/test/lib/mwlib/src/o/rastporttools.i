/* Prototypes for functions defined in c/RastPortTools.c */
BOOL ClipRastPort(struct RastPort *rp,
                  short left,
                  short top,
                  short width,
                  short height);
void UnclipRastPort(struct RastPort *rp,
                    short left,
                    short top);
