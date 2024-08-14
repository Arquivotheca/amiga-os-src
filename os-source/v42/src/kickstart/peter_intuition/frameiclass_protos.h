/* Prototypes for functions defined in
frameiclass.c
 */

Class * initFrameIClass(void);

extern ULONG frameiPackTable[5];

extern UBYTE StrokeWidth[4];

extern UBYTE InsetBox[4];

extern UBYTE WidthPad[4];

extern UBYTE HeightPad[4];

static ULONG dispatchFrameI(Class * , Object * , Msg );

static int frameBox(Class * , Object * , struct impFrameBox * );

static int drawFrameI(Class * , Object * , struct impDraw * );

int interiorBox(struct RastPort * rp,
                struct IBox * b,
                int xw,
                int yw,
                int pen);

