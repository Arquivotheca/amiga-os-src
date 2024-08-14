/* Prototypes for functions defined in
layout.c
 */

struct Gadget * createITGadget(struct Screen * screen,
                               struct IntuiText * gtext,
                               struct Remember ** remkey,
                               struct Gadget * prevgad,

  int id,
                               int frame);

int spreadLayoutGadgets(struct Gadget * g,
                        int gadcount,
                        struct IBox * box,
                        int totalwidth,
                        int gadspace);

int glistExtentBox(struct Gadget * g,
                   struct IBox * box,
                   int gadspace);

int ITextLayout(struct RastPort * rp,
                struct IntuiText * it,
                struct IBox * box,
                BOOL do_layout,
                int xoffset,
                int yoffset);

static int divvyUp(int , int , UWORD * );

