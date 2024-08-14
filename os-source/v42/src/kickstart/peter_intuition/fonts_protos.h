/* Prototypes for functions defined in
fonts.c
 */

int copyTAttr(struct TextAttr * ta,
              struct TAttrBuff * tab);

struct TextFont * ISetFont(struct RastPort * rp,
                           struct TextAttr * font);

int ICloseFont(struct TextFont * fp);

ULONG IntuiTextLength(struct IntuiText * itext);

int RastPortITextLength(struct RastPort * rp,
                        struct IntuiText * itext);

int RastPortITextExtent(struct RastPort * rp,
                        struct IntuiText * itext,
                        struct TextExtent * te);

