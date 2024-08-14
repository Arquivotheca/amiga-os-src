/* Prototypes for functions defined in
strings.c
 */

extern UBYTE nchar;

int displayString(struct RastPort * rp,
                  struct Gadget * sg,
                  struct GadgetInfo * gi,
                  struct IBox * gbox,
                  struct StringExtend * sex);

struct StringExtend * getSEx(struct Gadget * g);

