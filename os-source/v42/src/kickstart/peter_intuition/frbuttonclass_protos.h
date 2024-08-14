/* Prototypes for functions defined in
frbuttonclass.c
 */

Class * initFramedButtonClass(void);

static ULONG dispatchFramedB(Class * , Object * , Msg );

static int renderFramedB(struct RastPort * , struct GadgetInfo * , Object * , struct localObjData * , int );

static int setFramedBAttrs(Class * , Object * , struct opSet * );

static int getContentsExtent(Object * , struct DrawInfo * , struct IBox * );

static int displayContents(struct RastPort * , Object * , struct DrawInfo * , WORD , WORD , int );

